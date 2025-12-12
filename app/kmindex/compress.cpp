#ifdef KMINDEX_WITH_COMPRESSION

#include <iostream>

#include "compress.hpp"
#include <kmindex/threadpool.hpp>
#include <kmindex/index/index.hpp>
#include <kmindex/exceptions.hpp>
#include <spdlog/spdlog.h>

#include <bitmatrixshuffle.h>
#include <zstd/BlockCompressorZSTD.h>

namespace kmq {

  kmq_options_t kmq_compress_cli(parser_t parser, kmq_compress_options_t options)
  {
    auto cmd = parser->add_command("compress", "Compress index.");

    cmd->add_param("-i/--global-index", "Global index path.")
      ->meta("STR")
      ->setter(options->global_index_path);

    cmd->add_param("-n/--name", "Index name.")
       ->meta("STR")
       ->setter(options->index_name);

    cmd->add_param("-d/--delete", "Delete uncompressed index after compressing.")
       ->as_flag()
       ->setter(options->delete_old);

    cmd->add_param("--check", "Check query results after compressing.")
       ->as_flag()
       ->setter(options->check);

    auto adv = cmd->add_group("Reordering options", "");

    cmd->add_param("-b/--block-size", "Size of uncompressed blocks, in megabytes.")
       ->meta("INT")
       ->def("8")
       ->checker(bc::check::f::range(1, 1024))
       ->setter(options->block_size);

    adv->add_param("-r/--reorder", "Reorder columns before compressing.")
       ->as_flag()
       ->setter(options->reorder);

    adv->add_param("-s/--sampling", "Number of rows to sample for reordering.")
       ->meta("INT")
       ->def("20000")
       ->checker(bc::check::f::range(1000, 100000000))
       ->setter(options->sampling);

    adv->add_param("-c/--column-per-block", "Reorder columns by group of N. Should be a multiple of 8 (0=all)")
       ->meta("INT")
       ->def("0")
       ->checker(bc::check::f::range(0, 100000000))
       ->checker([](const std::string& p, const std::string& v) -> bc::check::checker_ret_t {
          auto n = std::stoul(v);
          bool ok = (n == 0 || n % 8 == 0);
          return std::make_tuple(ok, bc::utils::format_error(p, v, fmt::format("'{}' is not a multiple of 8.", v)));
       })
       ->setter(options->column_blocks);

    add_common_options(cmd, options, true);

    return options;
  }

  auto query_check(const std::string& seq, const index_infos& infos, std::size_t nb_threads)
  {
    batch_query bq(
      infos.nb_samples(),
      infos.nb_partitions(),
      infos.smer_size(),
      0ULL,
      1,
      infos.get_repartition(),
      infos.get_hash_w(),
      infos.minim_size()
    );

    bq.add_query("random", seq);

    kindex ki(infos, false);
    ThreadPool pool(nb_threads);

    for (std::size_t p = 0; p < infos.nb_partitions(); ++p)
    {
      pool.add_task([p, &bq, &ki](int){ ki.solve_one(bq, p); });
    }
    pool.join_all();

    return std::make_unique<query_result>(std::move(bq.response().front()), 0, infos, false);
  }

  constexpr std::uint64_t rev8(std::uint64_t x)
  {
      return (x | 0x7ULL) - (x & 0x7ULL);
  }

  bool epsilon_equal(double a, double b, double epsilon = 1e-6)
  {
      return std::abs(a - b) < epsilon;
  }

  bool compare_query_results(const query_result& a, const query_result& b, std::vector<std::size_t>& perms)
  {
    if (perms.size() == 0)
    {
      return std::equal(a.ratios().begin(), a.ratios().end(), b.ratios().begin());
    }
    else
    {
      for (std::size_t i = 0; i < perms.size(); ++i)
      {
        if (!epsilon_equal(a.ratios()[rev8(perms[rev8(i)])], b.ratios()[i]))
        {
          return false;
        }
      }
      return true;
    }
  }

  void immutable_filling_columns_inplace(std::vector<std::uint64_t>& order, const std::size_t SAMPLES)
  {
      const std::size_t COLUMNS = (SAMPLES + 7) / 8 * 8;

      if(COLUMNS == SAMPLES)
          return;

      const std::size_t FILL = COLUMNS - SAMPLES;
      const std::size_t A = COLUMNS - 8;
      const std::size_t B = COLUMNS - 8 + FILL - 1;

      std::size_t k = 0;

      for(std::size_t i = 0; i < COLUMNS-8; ++i)
      {
          while(order[i+k] >= A && order[i+k] <= B)
              ++k;

          order[i] = order[i+k];
      }

      k = 0;

      for(std::size_t i = COLUMNS-1; i > COLUMNS-8+FILL-1; --i)
      {
          while(order[i-k] >= A && order[i-k] <= B)
              ++k;

          order[i] = order[i-k];
      }

      for(std::uint64_t i = COLUMNS-8; i < COLUMNS+FILL-8; ++i)
          order[i] = i;
  }

  void reorder_fof(const std::string& fof_path, const std::size_t SAMPLES, const std::vector<std::size_t>& order)
  {
      std::vector<std::string> fof_lines;
      fof_lines.resize(SAMPLES);

      std::ifstream ifdfof(fof_path);

      std::size_t i = 0;
      std::string line;
      while (std::getline(ifdfof, line) && i < SAMPLES)
          fof_lines[i++] = line;

      ifdfof.close();
      fs::copy_file(fof_path, fof_path + ".bak", fs::copy_options::overwrite_existing);
      std::ofstream ofdfof(fof_path);

      for(i = 0; i < SAMPLES; ++i)
          ofdfof << fof_lines[rev8(order[rev8(i)])] << '\n';

      ofdfof.close();
  }

  void main_compress(kmq_options_t opt)
  {
    Timer gtime;
    kmq_compress_options_t o = std::static_pointer_cast<struct kmq_compress_options>(opt);
    index i(o->global_index_path);

    auto sub = i.get(o->index_name);

    if (sub.is_compressed_index())
    {
      throw kmq_error(fmt::format("Index '{}' is already compressed.", o->index_name));
      return;
    }

    std::vector<std::size_t> perm_orders;

    auto block_size_bytes = o->block_size * 1024 * 1024;
    auto entry_per_block = bms::target_block_nb_rows(sub.nb_samples(), block_size_bytes);

    auto config_path = sub.get_compression_config() + ".tmp";
    {
      std::ofstream config_file(config_path, std::ios::out);
      config_file << "samples = " << sub.nb_samples() << "\n";
      config_file << "bitvectorsperblock = " << entry_per_block << "\n";
      config_file << "preset = 6" << std::endl;
    }

    auto random_query = random_sequence(100);

    spdlog::info("Index '{}' has {} partitions, {} samples.", o->index_name, sub.nb_partitions(), sub.nb_samples());

    std::unique_ptr<query_result> before {nullptr};
    if (o->check)
    {
      spdlog::info("--check: making a query before compressing...");
      before = query_check(random_query, sub, o->nb_threads);
      spdlog::info("--check: query done.");
    }

    spdlog::info("Compressing index '{}' using {}MB blocks ({} bit vectors per block).", o->index_name, o->block_size, entry_per_block);

    ThreadPool pool(o->nb_threads);

    if (o->reorder)
    {
      Timer ptime;
      spdlog::info("Reordering columns using {} sampled rows and {} column blocks.", o->sampling, o->column_blocks);
      auto mpath = sub.get_partition(1);
      spdlog::info("Compute permutation from '{}'.", mpath);
      bms::compute_order_from_matrix_columns(mpath, 49, sub.nb_samples(), sub.bloom_size() / sub.nb_partitions(), o->column_blocks, o->sampling, perm_orders);
      immutable_filling_columns_inplace(perm_orders, sub.nb_samples());
      auto opath = fmt::format("{}/permutations.bin", sub.get_directory());
      std::ofstream ofs(opath, std::ios::out | std::ios::binary);
      ofs.write(reinterpret_cast<const char*>(perm_orders.data()), perm_orders.size() * sizeof(std::uint64_t));
      spdlog::info("Permutation computed ({}), saved at '{}'.", ptime.formatted(), opath);
    }

    for (std::size_t i = 0; i < sub.nb_partitions(); ++i)
    {
      if (o->reorder)
      {
        pool.add_task([i, &sub, &config_path, &perm_orders, block_size_bytes](int) {
          spdlog::debug("Compressing partition {}.", i);
          std::string part_path = sub.get_partition(i);
          std::string out = fmt::format("{}/matrices/blocks{}", sub.get_directory(), i);
          std::string out_ef = out + ".ef";
          bms::reorder_matrix_columns_and_compress(part_path,
                                                   out,
                                                   out_ef,
                                                   config_path,
                                                   49,
                                                   sub.nb_samples(),
                                                   sub.bloom_size() / sub.nb_partitions(),
                                                   perm_orders,
                                                   block_size_bytes);
          spdlog::debug("Partition {} compressed.", i);
        });
      }
      else
      {
        pool.add_task([i, &sub, &config_path] (int) {
          spdlog::debug("Compressing partition {}.", i);
          std::string part_path = sub.get_partition(i);
          std::string out = fmt::format("{}/matrices/blocks{}", sub.get_directory(), i);
          std::string out_ef = out + ".ef";
          BlockCompressorZSTD bc(out, out_ef, config_path);
          bc.compress_file(sub.get_partition(i), 49);
          bc.close();
          spdlog::debug("Partition {} compressed.", i);
        });
      }
    }

    pool.join_all();

    fs::copy_file(config_path, sub.get_compression_config(), fs::copy_options::overwrite_existing);
    fs::remove(config_path);

    sub.set_compress(true);

    spdlog::info("Index '{}' compressed. ({})", o->index_name, gtime.formatted());

    bool valid = true;

    if (o->check)
    {
      spdlog::info("--check: checking query results after compressing.");
      auto after = query_check(random_query, sub, o->nb_threads);
      if (compare_query_results(*before, *after, perm_orders))
      {
        spdlog::info("--check: query results match before and after compressing.");
      }
      else
      {
        valid = false;
        fs::remove(sub.get_compression_config());
        spdlog::error("--check: query results do not match before and after compressing.");

        if (o->delete_old)
        {
          spdlog::info("Uncompressed index will not be deleted (--delete).");
        }
        spdlog::info("Uncompressed index remains valid.");
      }
    }


    if (o->reorder && valid)
    {
      auto fof_path = fmt::format("{}/kmtricks.fof", sub.get_directory());
      reorder_fof(fof_path, sub.nb_samples(), perm_orders);
      auto index_directory = sub.get_directory();
      i.remove_index(o->index_name);
      i.add_index(o->index_name, index_directory);
      sub = i.get(o->index_name);
      i.save();
    }

    sub.set_compress(false); // required to get the right partition path for deletion
    if (o->delete_old && valid)
    {
      spdlog::info("Deleting uncompressed matrices.", o->index_name);
      for (std::size_t i = 0; i < sub.nb_partitions(); ++i)
      {
        auto part_path = sub.get_partition(i);
        fs::remove(part_path);
      }
      spdlog::info("Uncompressed matrices deleted.", o->index_name);
    }
  }
}
#endif
