
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
    auto cmd = parser->add_command("query", "query");

    cmd->add_param("--index", "global index path.")
       ->meta("STR")
       ->setter(options->global_index_path);

    auto name_setter = [options](const std::string& v) {
      options->index_names = bc::utils::split(v, ',');
    };

    cmd->add_param("--names", "indexes to query, comma separated.")
       ->meta("STR")
       ->setter_c(name_setter);

    cmd->add_param("--z", "size of k-mers: (s+z)-mers. ")
       ->meta("FLOAT")
       ->def("0")
       ->setter(options->z);

    cmd->add_param("--threshold", "shared k-mers threshold.")
       ->def("0")
       ->setter(options->sk_threshold);

    cmd->add_param("--output", "output directory.")
       ->meta("STR")
       ->def("output")
       ->setter(options->output);

    cmd->add_param("--fastx", "fasta/q file containing the sequence(s) to query.")
       ->meta("STR")
       ->checker(bc::check::seems_fastx)
       ->setter(options->input);

    cmd->add_param("--single-query", "query id. All sequences are considered as a unique query.")
      ->def("")
      ->meta("STR")
      ->setter(options->single);


    auto format_setter = [options](const std::string& v) {
      options->format = str_to_format(v);
    };

    cmd->add_param("--format", "output format [json|matrix]")
       ->meta("STR")
       ->def("json")
       ->checker(bc::check::f::in("json|matrix"))
       ->setter_c(format_setter);


    add_common_options(cmd, options, true);

    return options;
  }

  void main_query(kmq_options_t opt)
  {
    kmq_query_options_t o = std::static_pointer_cast<struct kmq_query_options>(opt);

    Timer gtime;

    index global(o->global_index_path);

    spdlog::info("Global index: {}", o->global_index_path);

    auto f = get_formatter(o->format);

    for (auto& index_name : o->index_names)
    {
      Timer timer;
      auto infos = global.get(index_name);

      spdlog::info("Query '{}' ({} samples)", infos.name(), infos.nb_samples());
      query_result_agg agg;

      klibpp::KSeq record;
      klibpp::SeqStreamIn iss(o->input.c_str());
      kindex ki(infos);

      while (iss >> record)
      {
        query q(record.name, record.seq, infos.smer_size(), o->z, infos.nb_samples(), 0.0);
        agg.add(ki.resolve(q));
      }
      std::string resp = f->format(infos.name(), infos.samples(), agg, o->single);
      write_result(resp, infos.name(), o->output, o->format);

      spdlog::info("Results dumped at {}/{}.{} ({})",
                 o->output,
                 infos.name(),
                 o->format == format::json ? "json" : "tsv",
                 timer.formatted());
    }

    spdlog::info("Done ({}).", gtime.formatted());
  }
}
