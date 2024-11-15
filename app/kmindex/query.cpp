
#include "query.hpp"

#include <iostream>
#include <kmindex/query/query.hpp>
#include <kmindex/index/index.hpp>
#include <kmindex/query/format.hpp>

#include <kmindex/threadpool.hpp>
#include <kseq++/seqio.hpp>

#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include <atomic_queue/atomic_queue.h>

namespace kmq {

  struct fastx_record {
    std::string name;
    std::string seq;
    fastx_record() noexcept {}
    fastx_record(std::string&& name, std::string&& seq) noexcept
      : name(std::move(name)), seq(std::move(seq)) {}
  };

  static constexpr bool minimize_contention = true;
  static constexpr bool maximize_throughput = true;
  static constexpr bool total_ordering = true;
  static constexpr bool spsc = false;
  static constexpr std::size_t queue_size = 2048;
  using queue_type = atomic_queue::AtomicQueue2<
    fastx_record, queue_size, minimize_contention, maximize_throughput, total_ordering, spsc
  >;

  kmq_options_t kmq_query_cli(parser_t parser, kmq_query_options_t options)
  {
    auto cmd = parser->add_command("query", "Query index.");

    auto is_kmq_index = [](const std::string& p, const std::string& v) -> bc::check::checker_ret_t {
      return std::make_tuple(fs::exists(fmt::format("{}/index.json", v)), bc::utils::format_error(p, v, fmt::format("'{}' is not an index.", v)));
    };

    cmd->add_param("-i/--index", "Global index path.")
       ->meta("STR")
       ->checker(bc::check::is_dir)
       ->checker(is_kmq_index)
       ->setter(options->global_index_path);

    auto name_setter = [options](const std::string& v) {
      if (v != "all")
        options->index_names = bc::utils::split(v, ',');
    };

    cmd->add_param("-n/--names", "Sub-indexes to query, comma separated.")
       ->meta("STR")
       ->def("all")
       ->setter_c(name_setter);

    cmd->add_param("-z/--zvalue", "Index s-mers and query (s+z)-mers (findere algorithm).")
       ->meta("INT")
       ->def("0")
       ->checker(bc::check::f::range(0, 8))
       ->setter(options->z);

    cmd->add_param("-r/--threshold", "Shared k-mers threshold. in [0.0, 1.0]")
       ->meta("FLOAT")
       ->def("0.0")
       ->checker(bc::check::f::range(0.0, 1.0))
       ->setter(options->sk_threshold);

    auto not_dir = [](const std::string& p, const std::string& v) -> bc::check::checker_ret_t {
      return std::make_tuple(!fs::exists(v), bc::utils::format_error(p, v, "Directory already exists."));
    };

    cmd->add_param("-o/--output", "Output directory.")
       ->meta("STR")
       ->def("output")
       ->checker(not_dir)
       ->setter(options->output);

    cmd->add_param("-q/--fastx", "Input fasta/q file (supports gz/bzip2) containing the sequence(s) to query.")
       ->meta("STR")
       ->checker(bc::check::is_file)
       ->checker(bc::check::f::ext(
         "fa|fq|fasta|fastq|fna|fa.gz|fq.gz|fasta.gz|fastq.gz|fna.gz|fa.bz2|fq.bz2|fasta.bz2|fastq.bz2|fna.bz2"))
       ->setter(options->input);

    cmd->add_param("-s/--single-query", "Query identifier. All sequences are considered as a unique query.")
      ->def("")
      ->meta("STR")
      ->setter(options->single);

    auto format_setter = [options](const std::string& v) {
      options->format = str_to_format(v);
    };

    cmd->add_param("-f/--format", "Output format [json|matrix|json_vec]")
       ->meta("STR")
       ->def("json")
       ->checker(bc::check::f::in("json|matrix|json_vec"))
       ->setter_c(format_setter);

    cmd->add_param("-b/--batch-size", "Size of query batches (0≈nb_seq/nb_thread).")
       ->meta("INT")
       ->def("0")
       ->checker(bc::check::is_number)
       ->setter(options->batch_size);

    cmd->add_param("-a/--aggregate", "Aggregate results from batches into one file.")
       ->as_flag()
       ->setter(options->aggregate);

    cmd->add_param("--fast", "Keep more pages in cache (see doc for details).")
       ->as_flag()
       ->setter(options->cache);

    cmd->add_param("--blob-mode", "Use Azure SDK")
       ->as_flag()
       ->setter(options->blob_mode);

    add_common_options(cmd, options, true, 1);

    return options;
  }

  void populate_queue(queue_type& q, klibpp::SeqStreamIn& fx_stream, std::size_t n, std::size_t min_size)
  {
    klibpp::KSeq record;
    while (fx_stream >> record)
    {
      if (record.seq.size() < min_size)
      {
        spdlog::warn("'{}' skipped: min size is s+z={}", record.name, min_size);
        continue;
      }
      q.push(fastx_record(std::move(record.name), std::move(record.seq)));
    }
    for (std::size_t _ = 0; _ < n; ++_)
      q.push(fastx_record("", ""));
  }

  void solve_batch(batch_query& bq,
                   const index_infos& infos,
                   kindex& ki,
                   const kmq_query_options_t& opt,
                   std::size_t batch_id,
                   Timer& timer,
                   query_result_agg& aggs)
  {
    std::size_t nq = bq.size();

    ki.solve_batch(bq);

    bq.free_smers();

    bool wpos = opt->format == format::json_with_positions;
    if (opt->single.empty())
    {
      query_result_agg agg;
      for (auto&& r : bq.response())
        agg.add(query_result(std::move(r), opt->z, infos, wpos));

      bq.free_responses();

      std::string output;
      if (opt->batch_size > 0 || opt->nb_threads > 1)
      {
        output = fmt::format("{}/batch_{}", opt->output, batch_id);
        fs::create_directories(output);
      }
      else
      {
        output = opt->output;
      }
      agg.output(infos, output, opt->format, opt->single, opt->sk_threshold);

      spdlog::debug("batch_{} processed ({} sequences) dumped at {}/{}.{} ({})",
          batch_id,
          nq,
          output,
          infos.name(),
          opt->format == format::matrix ? "tsv" : "json",
          timer.formatted());
    }
    else
    {
      for (auto&& r : bq.response())
        aggs.add(query_result(std::move(r), opt->z, infos, wpos));

      bq.free_responses();

      spdlog::debug("batch_{} processed ({} sequences) ({})", batch_id, nq, timer.formatted());
    }
  }

  void merge_json(std::size_t n, const std::string& index_name, const std::string& output)
  {
    std::ofstream out(fmt::format("{}/{}.json", output, index_name), std::ios::out);

    json data = json({});

    for (std::size_t b = 0; b < n; ++b)
    {
      std::ifstream inf(fmt::format("{}/batch_{}/{}.json", output, b, index_name), std::ios::in);
      auto jj = json::parse(inf);
      data.merge_patch(jj);
    }
    out << std::setw(4) << data << std::endl;
  }

  void merge_tsv(std::size_t n, const std::string& index_name, const std::string& output)
  {
    std::ofstream out(fmt::format("{}/{}.tsv", output, index_name), std::ios::out);

    bool first = true;
    for (std::size_t b = 0; b < n; ++b)
    {
      std::string line;
      std::ifstream inf(fmt::format("{}/batch_{}/{}.tsv", output, b, index_name), std::ios::in);

      if (first)
      {
        std::getline(inf, line);
        out << line << '\n';
        first = false;
      }
      else
      {
        std::getline(inf, line);
      }

      while (std::getline(inf, line))
      {
        out << line << '\n';
      }
    }
  }

  void merge_results(const std::string& output, format f, const std::string& index_name)
  {
    auto it = fs::directory_iterator(output);
    std::size_t n = std::distance(it, fs::directory_iterator{});

    switch (f)
    {
      case format::json:
      case format::json_with_positions:
        merge_json(n, index_name, output);
        break;
      case format::matrix:
        merge_tsv(n, index_name, output);
        break;
    }
  }

  void main_query(kmq_options_t opt)
  {
    kmq_query_options_t o = std::static_pointer_cast<struct kmq_query_options>(opt);

    Timer gtime;

    index global(o->global_index_path);

    spdlog::info(
      "Global index: '{}'", fs::absolute(o->global_index_path + "/").parent_path().filename().string());

    if (o->index_names.empty())
    {
      o->index_names = global.all();
    }
    else

    spdlog::info("Sub-indexes to query: [{}]", fmt::join(o->index_names, ","));

    if (!o->single.empty())
      spdlog::warn("--single-query: all query results are kept in memory");

    for (auto& index_name : o->index_names)
    {
      Timer timer;
      auto infos = global.get(index_name);

      spdlog::info("Starting '{}' query ({} samples)", infos.name(), infos.nb_samples());

      klibpp::SeqStreamIn iss(o->input.c_str());
      queue_type bqueue;

      ThreadPool pool(opt->nb_threads);

      kindex ki(infos, o->cache, o->blob_mode);
      //smer_hasher sh(infos.get_repartition(), infos.get_hash_w(), infos.minim_size());

      std::atomic<std::size_t> batch_id = 0;

      query_result_agg aggs;

      for (std::size_t c = 0; c < opt->nb_threads; ++c)
      {
        pool.add_task([&bqueue, &infos, &ki, &batch_id, &aggs, opt=o](int i){
          unused(i);
          for (;;)
          {
            Timer timer;
            batch_query bq(infos.nb_samples(),
                           infos.nb_partitions(),
                           infos.smer_size(),
                           opt->z,
                           infos.bw(),
                           infos.get_repartition(),
                           infos.get_hash_w(),
                           infos.minim_size());

            bool end = false;
            std::size_t nq = 0;
            std::size_t id = batch_id.fetch_add(1);

            while (!end)
            {
              auto record = bqueue.pop();

              if (record.name.empty())
              {
                end = true;
              }
              else
              {
                bq.add_query(std::move(record.name), std::move(record.seq));
                ++nq;
              }

              if ((nq == opt->batch_size || end) && nq > 0)
              {
                spdlog::debug("process batch_{} ({} sequences)", id, nq);
                solve_batch(bq, infos, ki, opt, id, timer, aggs);
                break;
              }
            }

            nq = 0;

            if (end)
              return;
          }
        });
      }

      populate_queue(bqueue, iss, opt->nb_threads, infos.smer_size() + o->z);
      pool.join_all();

      if (!o->single.empty())
      {
        spdlog::info("aggregate query results ({} sequences)", aggs.size());
        aggs.output(infos, o->output, o->format, o->single, o->sk_threshold);
        spdlog::info("query '{}' processed, results dumped at {}/{}.{}",
          o->single, o->output, infos.name(), o->format == format::matrix ? "tsv" : "json");
      }
      else
      {
        if (o->aggregate && ((o->batch_size > 0) || (o->nb_threads > 1)))
        {
          std::string ext = o->format == format::matrix ? "tsv" : "json";

          merge_results(o->output, o->format, infos.name());

          spdlog::info("Index '{}' processed, results dumped at {}/{}.{} ({}).",
                       infos.name(),
                       o->output,
                       infos.name(),
                       ext,
                       timer.formatted());
        }
        else
        {
          spdlog::info("Index '{}' processed. ({})", infos.name(), timer.formatted());
        }
      }
    }
    spdlog::info("Done ({}).", gtime.formatted());
  }
}

