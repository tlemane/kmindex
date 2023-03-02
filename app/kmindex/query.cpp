
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
    fastx_record() {}
    fastx_record(std::string&& name, std::string&& seq)
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

    cmd->add_param("-r/--threshold", "Shared k-mers threshold.")
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

    cmd->add_param("-f/--format", "Output format [json|matrix]")
       ->meta("STR")
       ->def("json")
       ->checker(bc::check::f::in("json|matrix"))
       ->setter_c(format_setter);

    cmd->add_param("-b/--batch-size", "Size of query batches.")
       ->meta("INT")
       ->def("0")
       ->checker(bc::check::is_number)
       ->setter(options->batch_size);

    add_common_options(cmd, options, true);

    return options;
  }

  void populate_queue(queue_type& q, klibpp::SeqStreamIn& fx_stream, std::size_t n)
  {
    klibpp::KSeq record;
    while (fx_stream >> record)
    {
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

    ki.solve(bq);

    bq.free_smers();

    if (opt->single.empty())
    {
      query_result_agg agg;
      for (auto&& r : bq.response())
        agg.add(query_result(std::move(r), opt->z, infos));

      bq.free_responses();

      std::string output;
      if (opt->batch_size > 0)
      {
        output = fmt::format("{}/batch_{}", opt->output, batch_id);
        fs::create_directories(output);
      }
      else
      {
        output = opt->output;
      }
      agg.output(infos, output, opt->format, opt->single, opt->sk_threshold);

      spdlog::info("batch_{} processed ({} sequences) dumped at {}/{}.{} ({})",
          batch_id,
          nq,
          output,
          infos.name(),
          opt->format == format::json ? "json" : "tsv",
          timer.formatted());
    }
    else
    {
      for (auto&& r : bq.response())
        aggs.add(query_result(std::move(r), opt->z, infos));

      bq.free_responses();

      spdlog::info("batch_{} processed ({} sequences) ({})", batch_id, nq, timer.formatted());
    }
  }

  void main_query(kmq_options_t opt)
  {
    kmq_query_options_t o = std::static_pointer_cast<struct kmq_query_options>(opt);

    Timer gtime;

    index global(o->global_index_path);

    spdlog::info(
      "Global index: '{}'", fs::absolute(o->global_index_path).parent_path().filename().string());

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

      kindex ki(infos);
      smer_hasher sh(infos.get_repartition(), infos.get_hash_w(), infos.minim_size());

      std::atomic<std::size_t> batch_id = 0;

      query_result_agg aggs;

      for (std::size_t c = 0; c < opt->nb_threads; ++c)
      {
        pool.add_task([&bqueue, &infos, &ki, &sh, &batch_id, &aggs, opt=o](int i){
          unused(i);
          for (;;)
          {
            Timer timer;
            batch_query bq(infos.nb_samples(),
                           infos.nb_partitions(),
                           infos.smer_size(),
                           opt->z,
                           infos.bw(),
                           &sh);

            bool end = false;
            std::size_t nq = 0;
            std::size_t id = batch_id.load();

            batch_id++;

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
                spdlog::info("process batch_{} ({} sequences)", id, nq);
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

      populate_queue(bqueue, iss, opt->nb_threads);
      pool.join_all();

      if (!o->single.empty())
      {
        spdlog::info("aggregate query results ({} sequences)", aggs.size());
        aggs.output(infos, o->output, o->format, o->single, o->sk_threshold);
        spdlog::info("query '{}' processed, dumped at {}/{}.{}",
          o->single, o->output, infos.name(), o->format == format::json ? "json" : "tsv");
      }
    }
    spdlog::info("Done ({}).", gtime.formatted());
  }
}

