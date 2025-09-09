#include <iostream>

#include "query_sum.hpp"
#include <kmindex/index/index.hpp>
#include <kmindex/exceptions.hpp>
#include <spdlog/spdlog.h>
#include <kmindex/threadpool.hpp>
#include <kmindex/index/sum_index.hpp>
#include <kseq++/seqio.hpp>

namespace kmq {

  template<std::size_t W>
  struct response_dump
  {
    using bp = bit_packer<W>;

    template<std::size_t... offsets>
    std::size_t window_mean(std::size_t start, const std::uint64_t* array, std::size_t n, double fp, std::index_sequence<offsets...>) const noexcept
    {
      std::size_t s = (bp::unpack(array, start + offsets) + ... );
      std::size_t obs = s / sizeof...(offsets);

      double est = (static_cast<double>(obs) - fp * n) / (1.0 - fp);
      return est < 0.0 ? 0 : static_cast<std::size_t>(est);
    }

    template<std::size_t K>
    void process_window(std::uint64_t* packed, std::size_t n, double fp, std::ostream& os) const noexcept
    {
      auto mean0 = window_mean(0, packed, n, fp, std::make_index_sequence<K>{});
      os << mean0;
    
      for (std::size_t i = 1; i + K <= n; i++)
      {
        auto mean = window_mean(i, packed, n, fp, std::make_index_sequence<K>{});
        os << ',' << mean;
      }
    }

    void operator()(std::uint64_t* packed, std::size_t n, std::size_t z, double sk, std::ostream& os) const noexcept
    {
      if (z == 0)
      {
        for (std::size_t i = 0; i < n - 1; i++)
        {
          os << bp::unpack(packed, i) << ',';
        }
        os << bp::unpack(packed, n - 1);
        return;
      }

      std::size_t k = 1 + z;
      switch (k)
      {
        case 2: process_window<2>(packed, n, sk, os); break;
        case 3: process_window<3>(packed, n, sk, os); break;
        case 4: process_window<4>(packed, n, sk, os); break;
        case 5: process_window<5>(packed, n, sk, os); break;
        case 6: process_window<6>(packed, n, sk, os); break;
        case 7: process_window<7>(packed, n, sk, os); break;
        case 8: process_window<8>(packed, n, sk, os); break;
        default:
          break;
      }
    }
  };

  kmq_options_t kmq_sum_query_cli(parser_t parser, kmq_sum_query_options_t options)
  {
    auto cmd = parser->add_command("sum-query", "Query a summarized index. (experimental)");

    cmd->add_param("-i/--global-index", "Global index path.")
      ->meta("STR")
      ->setter(options->global_index_path);

    cmd->add_param("-q/--fastx", "Input fasta/q file (supports gz/bzip2) containing the sequence(s) to query.")
       ->meta("STR")
       ->checker(bc::check::is_file)
       ->checker(bc::check::f::ext(
         "fa|fq|fasta|fastq|fna|fa.gz|fq.gz|fasta.gz|fastq.gz|fna.gz|fa.bz2|fq.bz2|fasta.bz2|fastq.bz2|fna.bz2"))
       ->setter(options->query_file);

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

    cmd->add_param("-f/--fp", "")
       ->meta("FLOAT")
       ->def("0.25")
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

    add_common_options(cmd, options, true);

    return options;
  }

  void main_sum_query(kmq_options_t opt)
  {
    kmq_sum_query_options_t o = std::static_pointer_cast<struct kmq_sum_query_options>(opt);
    index i(o->global_index_path);
    Timer gtime;

    bool all = o->index_names.empty();
    
    if (all)
    {
      o->index_names = i.all();
    }

    for (auto& index_name : o->index_names)
    {
      Timer itime;
      auto infos = i.get(index_name);
      
      if (!infos.has_sum_index())
      {
        if (!all)
        {
          spdlog::warn("Index '{}' has no summarized index, skipped.", index_name);
        }
        continue;
      }
      
      spdlog::info("Starting '{}' query", index_name);
      klibpp::SeqStreamIn fx_stream(o->query_file.c_str());

      sum_query_batch bq(
        infos.nb_samples(),
        infos.nb_partitions(),
        infos.smer_size(),
        infos.get_repartition(),
        infos.get_hash_w(),
        infos.minim_size()
      );

      sum_index si(&infos);

      klibpp::KSeq record;

      while (fx_stream >> record)
      {
        if (record.seq.size() < infos.smer_size() + 1)
        {
          spdlog::warn("'{}' skipped: min size is s+z={}", record.name, infos.smer_size() + o->z);
          continue;
        }
        bq.add_query(record.name, record.seq);
      }


      ThreadPool pool(opt->nb_threads);
      for (std::size_t p = 0; p < infos.nb_partitions(); ++p)
      {
        pool.add_task([p, &bq, &si](int) { 
          si.search_partition(p, bq);
        });
      }

      pool.join_all();

      bq.free_smers();

      fs::create_directories(o->output);
      std::ofstream out(fmt::format("{}/{}.jsonl", o->output, infos.name()), std::ios::out);

      auto& responses = bq.response();
      
      for (auto& r : responses)
      {
        out << '{' << "\"index\": \"" << infos.name() << "\","
            << "\"query\": \"" << r->name() << "\","
            << "\"abs\": [";

        runtime_dispatch<32, 1, 1, std::equal_to<std::size_t>>::execute<response_dump>(
          static_cast<int>(std::log2(infos.nb_samples())) + 1,
          r->storage().data(),
          r->query_size(),
          o->z,
          o->sk_threshold,
          out
        );

        out << "]}\n";        
      }

      spdlog::info("Index '{}' processed ({}), results dumped at {}/{}.jsonl",
        infos.name(), itime.formatted(), o->output, infos.name());
    }

    spdlog::info("Done ({}).", gtime.formatted());
  }
}
