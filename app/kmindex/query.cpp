
#include "query.hpp"

#include <iostream>
#include <kmindex/query/query.hpp>
#include <kmindex/index/index.hpp>
#include <kmindex/query/format.hpp>

#include <kmindex/threadpool.hpp>
#include <kseq++/seqio.hpp>

#include <fmt/format.h>
#include <spdlog/spdlog.h>


namespace kmq {

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

  void main_query(kmq_options_t opt)
  {
    kmq_query_options_t o = std::static_pointer_cast<struct kmq_query_options>(opt);

    Timer gtime;

    index global(o->global_index_path);

    spdlog::info("Global index: {}", o->global_index_path);

    if (o->index_names.empty())
    {
      o->index_names = global.all();
    }

    for (auto& index_name : o->index_names)
    {
      Timer timer;
      auto infos = global.get(index_name);

      spdlog::info("Query '{}' ({} samples)", infos.name(), infos.nb_samples());

      klibpp::KSeq record;
      klibpp::SeqStreamIn iss(o->input.c_str());
      kindex ki(infos);

      smer_hasher sh(infos.get_repartition(), infos.get_hash_w(), infos.minim_size());

      batch_query bq(
        infos.nb_samples(), infos.nb_partitions(), infos.smer_size(), o->z, infos.bw(), &sh);

      while (iss >> record)
      {
        bq.add_query(record.name, record.seq);
      }

      for (std::size_t p = 0; p < infos.nb_partitions(); ++p)
      {
        ki.solve_one(bq, p);
      }

      query_result_agg agg;
      for (auto&& r : bq.response())
      {
        agg.add(query_result(std::move(r), o->z, infos));
      }

      agg.output(infos, o->output, o->format, o->single, o->sk_threshold);


      spdlog::info("Results dumped at {}/{}.{} ({})",
                 o->output,
                 infos.name(),
                 o->format == format::json ? "json" : "tsv",
                 timer.formatted());
    }

    spdlog::info("Done ({}).", gtime.formatted());
  }
}
