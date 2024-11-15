
#include "query-blob.hpp"

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

  kmq_options_t kmq_queryblob_cli(parser_t parser, kmq_queryb_options_t options)
  {
    auto cmd = parser->add_command("query-blob", "Query index.");

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

  using qsmer_type = std::pair<smer, std::uint32_t>;
  using qpart_type = std::vector<qsmer_type>;
  using repart_type = std::shared_ptr<km::Repartition>;
  using hw_type = std::shared_ptr<km::HashWindow>;

  template<std::size_t MK>
  struct smer_functor
  {
    void operator()(std::vector<qpart_type>& smers,
                    const std::string& seq,
                    std::uint32_t qid,
                    std::size_t smer_size,
                    repart_type& repart,
                    hw_type& hw,
                    std::size_t msize)
    {
      smer_hasher<MK> sh(repart, hw, msize);

      for (auto& mer : smer_iterator<MK>(seq, smer_size, sh))
      {
        smers[mer.p].emplace_back(mer, qid);
      }
    }
  };

  void main_queryblob(kmq_options_t opt)
  {
    kmq_queryb_options_t o = std::static_pointer_cast<struct kmq_queryb_options>(opt);

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

    if (o->blob_mode)
    {
      ThreadPool poolL(opt->nb_threads);

      for (auto& index_name : o->index_names)
      {
        poolL.add_task([&o, &global, &index_name](int i){

          Timer timer;
          auto infos = global.get(index_name);

          spdlog::info("Starting '{}' query ({} samples)", infos.name(), infos.nb_samples());

          klibpp::SeqStreamIn iss(o->input.c_str());
          queue_type bqueue;


          kindex ki(infos, o->cache, o->blob_mode);

          klibpp::KSeq record;
          iss >> record;

          std::string connectionString = std::getenv("AZURE_STORAGE_CONNECTION_STRING");
          auto azure_client = BlobServiceClient::CreateFromConnectionString(connectionString);
          auto itp_client = BlobContainerClient(azure_client.GetBlobContainerClient("indextheplanet"));

          std::size_t n = record.seq.size() - infos.smer_size() + 1;
          std::vector<std::unique_ptr<blob_partition>> m_partitions;
          auto r = std::make_unique<query_response>(record.name, n, infos.nb_samples(), infos.bw());
          std::vector<qpart_type> m_smers (infos.nb_partitions());

          auto repart = infos.get_repartition();
          auto hw = infos.get_hash_w();
          loop_executor<MAX_KMER_SIZE>::exec<smer_functor>(infos.smer_size(), m_smers, record.seq, 0, infos.smer_size(), repart, hw, infos.minim_size());

          for (std::size_t p = 0; p < infos.nb_partitions(); ++p)
          {
            auto s = infos.get_partition(p).substr(10);
            m_partitions.push_back(std::make_unique<blob_partition>(s, infos.nb_samples(), infos.bw(), &itp_client));
          }

          for (std::size_t p = 0; p < infos.nb_partitions(); ++p)
          {
              auto& smers = m_smers[p];
              std::sort(std::begin(smers), std::end(smers));
              for (auto& [mer, qid] : smers)
              {
                m_partitions[p]->query(mer.h, r->get(mer.i));
              }
          }

          query_result_agg aggs;
          aggs.add(query_result(std::move(r), o->z, infos, false));
          aggs.output(infos, o->output, o->format, o->single, o->sk_threshold);

        });
      }
      poolL.join_all();
    }
    else
    {
      for (auto& index_name : o->index_names)
      {
        Timer timer;
        auto infos = global.get(index_name);

        spdlog::info("Starting '{}' query ({} samples)", infos.name(), infos.nb_samples());

        klibpp::SeqStreamIn iss(o->input.c_str());
        queue_type bqueue;


        kindex ki(infos, o->cache, o->blob_mode);

        klibpp::KSeq record;
        iss >> record;

        std::string connectionString = std::getenv("AZURE_STORAGE_CONNECTION_STRING");
        auto azure_client = BlobServiceClient::CreateFromConnectionString(connectionString);
        auto itp_client = BlobContainerClient(azure_client.GetBlobContainerClient("indextheplanet"));

        std::size_t n = record.seq.size() - infos.smer_size() + 1;
        std::vector<std::unique_ptr<blob_partition>> m_partitions;
        auto r = std::make_unique<query_response>(record.name, n, infos.nb_samples(), infos.bw());
        std::vector<qpart_type> m_smers (infos.nb_partitions());

        auto repart = infos.get_repartition();
        auto hw = infos.get_hash_w();
        loop_executor<MAX_KMER_SIZE>::exec<smer_functor>(infos.smer_size(), m_smers, record.seq, 0, infos.smer_size(), repart, hw, infos.minim_size());

        for (std::size_t p = 0; p < infos.nb_partitions(); ++p)
        {
          auto s = infos.get_partition(p).substr(10);
          m_partitions.push_back(std::make_unique<blob_partition>(s, infos.nb_samples(), infos.bw(), &itp_client));
        }

        ThreadPool pool(opt->nb_threads);

        for (std::size_t p = 0; p < infos.nb_partitions(); ++p)
        {
            pool.add_task([&m_smers, &m_partitions, &r, p](int i) {
            auto& smers = m_smers[p];
            std::sort(std::begin(smers), std::end(smers));
            for (auto& [mer, qid] : smers)
            {
              m_partitions[p]->query(mer.h, r->get(mer.i));
            }
          });
        }

        pool.join_all();

        query_result_agg aggs;
        aggs.add(query_result(std::move(r), o->z, infos, false));
        aggs.output(infos, o->output, o->format, o->single, o->sk_threshold);

      }

    }
  }

}
