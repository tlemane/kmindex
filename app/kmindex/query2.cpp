
#include "query2.hpp"

#include <iostream>
#include <kmindex/query/query.hpp>
#include <kmindex/index/index.hpp>
#include <kmindex/query/format.hpp>

#include <kmindex/threadpool.hpp>
#include <kmindex/exceptions.hpp>
#include <kseq++/seqio.hpp>

#include <algorithm>
#include <cassert>
#include <condition_variable>
#include <mutex>

#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include <atomic_queue/atomic_queue.h>

#ifdef KMINDEX_WITH_COMPRESSION
  #include <block_compressor/config.hpp>
#endif

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

  class memory_semaphore
  {
  public:
    explicit memory_semaphore(std::size_t budget) : m_budget(budget) {}
    void acquire(std::size_t bytes)
    {
      std::unique_lock<std::mutex> lock(m_mutex);
      m_cv.wait(lock, [this, bytes] { return m_budget == 0 || m_used + bytes <= m_budget; });
      m_used += bytes;
    }
    void release(std::size_t bytes)
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      assert(m_used >= bytes);
      m_used -= bytes;
      m_cv.notify_all();
    }
  private:
    std::mutex m_mutex;
    std::condition_variable m_cv;
    std::size_t m_budget {0};
    std::size_t m_used {0};
  };

  struct sem_guard
  {
    memory_semaphore& sem;
    std::size_t bytes;
    sem_guard(const sem_guard&) = delete;
    sem_guard& operator=(const sem_guard&) = delete;
    ~sem_guard() { sem.release(bytes); }
  };

  kmq_options_t kmq_query2_cli(parser_t parser, kmq_query2_options_t options)
  {
    auto cmd = parser->add_command("query2", "To be used instead of kmindex query when many sub-indexes are registered, i.e. hundreds or thousands.");

    auto is_kmq_index = [](const std::string& p, const std::string& v) -> bc::check::checker_ret_t {
      auto paths = bc::utils::split(v, ',');
      for (const auto& path : paths)
      {
        if (!fs::exists(path))
          return std::make_tuple(false, bc::utils::format_error(p, v, fmt::format("'{}' does not exist.", path)));
        if (!fs::is_directory(path))
          return std::make_tuple(false, bc::utils::format_error(p, v, fmt::format("'{}' is not a directory.", path)));
        if (!fs::exists(fmt::format("{}/index.json", path)))
          return std::make_tuple(false, bc::utils::format_error(p, v, fmt::format("'{}' is not a kmindex index (no index.json).", path)));
      }
      return std::make_tuple(true, "");
    };

    cmd->add_param("-i/--index", "Global index path. Multiple comma-separated paths are merged (sub-indexes from all paths are queried together).")
       ->meta("STR")
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

    auto format_setter = [options](const std::string& v) {
      options->format = str_to_format(v);
    };

    cmd->add_param("-f/--format", "Output format [json|matrix|json_vec|jsonl|jsonl_vec]")
       ->meta("STR")
       ->def("json")
       ->checker(bc::check::f::in("json|matrix|json_vec|jsonl|jsonl_vec"))
       ->setter_c(format_setter);

    cmd->add_param("--fast", "Keep more pages in cache (see doc for details).")
       ->as_flag()
       ->setter(options->cache);

    cmd->add_param("-u/--uncompressed", "Use uncompressed partitions (if available).")
       ->as_flag()
       ->hide()
       ->setter(options->uncompressed);

    cmd->add_param("--memory-budget", "Total memory budget for concurrent sub-index queries in MB (heap + mmap working set). 0 = no limit.")
       ->meta("INT")
       ->def("0")
       ->checker(bc::check::f::range(0, 100000000))
       ->setter(options->memory_budget);


    add_common_options(cmd, options, true, 1);

    return options;
  }

  void main_query2(kmq_options_t opt)
  {
    kmq_query2_options_t o = std::static_pointer_cast<struct kmq_query2_options>(opt);

    Timer gtime;

    spdlog::info("Loading global index: {}", o->global_index_path);
    Timer load_time;
    auto index_paths = bc::utils::split(o->global_index_path, ',');
    index global(index_paths[0]);
    for (std::size_t i = 1; i < index_paths.size(); ++i)
    {
      spdlog::info("Merging additional index: {}", index_paths[i]);
      index other(index_paths[i]);
      global.merge(other);
    }
    spdlog::info("Global index loaded ({}).", load_time.formatted());

    if (index_paths.size() > 1)
      spdlog::info("Global index: merged from {} paths", index_paths.size());
    else
      spdlog::info(
        "Global index: '{}'", fs::absolute(o->global_index_path + "/").parent_path().filename().string());

    if (o->index_names.empty())
    {
      o->index_names = global.all();
    }
    else
    {
      if (o->index_names[0][0] == '@')
      {
        std::ifstream fs(o->index_names[0].substr(1));
        o->index_names.clear();
        for (std::string line; std::getline(fs, line);)
        {
          o->index_names.push_back(line);
        }
      }
      spdlog::info("Sub-indexes to query: [{}]", fmt::join(o->index_names, ","));
    }

    for (const auto& name : o->index_names)
    {
      if (!global.has_index(name))
        throw kmq_error(fmt::format("{} subindex does not exist!", name));
    }

    if (!o->single.empty())
      spdlog::warn("--single-query: all query results are kept in memory");

    ThreadPool pool(opt->nb_threads);

    klibpp::SeqStreamIn iss(o->input.c_str());
    std::vector<klibpp::KSeq> records;
    klibpp::KSeq record;

    while (iss >> record)
    {
      records.push_back(record);
    }

    bool with_positions = o->format == format::json_with_positions || o->format == format::jsonl_with_positions;

    std::size_t budget_bytes = o->memory_budget * 1024 * 1024;
    if (budget_bytes > 0)
      spdlog::info("Memory budget: {}MB", o->memory_budget);
    memory_semaphore sem(budget_bytes);

    // Memory estimate per sub-index
    //
    // Heap: two phases exist within each task:
    //   Phase 1 (solve_batch): smers + query_response data both live on heap.
    //   Phase 2 (agg building, after free_smers): query_response data moves into
    //     query_result objects which add m_ratios (nb_samples * 8B), m_counts
    //     (nb_samples * 4B), and m_positions (nb_samples * n_smers * 1B, position
    //     formats only). For bw==1 the response data is freed during compute, so
    //     phase 2 peak is positions + ratios + counts. For bw>1 response data
    //     stays alive, so phase 2 peak is response data + positions + ratios + counts.
    //   We take the max of both phases.
    //
    std::vector<std::pair<std::string, std::size_t>> indexed_mem;
    for (const auto& name : o->index_names)
    {
      auto infos = global.get(name);
      std::size_t ns = infos.nb_samples();
      std::size_t block_size = ((ns * infos.bw()) + 7) / 8;
      constexpr std::size_t smers_pair_size = sizeof(std::pair<smer, std::uint32_t>);
      std::size_t phase1 = 0;
      std::size_t phase2 = 0;
      for (const auto& record : records)
      {
        if (record.seq.size() >= infos.smer_size() + o->z)
        {
          std::size_t n_smers = record.seq.size() - infos.smer_size() + 1;
          std::size_t response_mem = n_smers * block_size;
          std::size_t smers_mem = n_smers * smers_pair_size;
          phase1 += response_mem + smers_mem;

          std::size_t ratios_counts = ns * sizeof(double) + ns * sizeof(std::uint32_t);
          std::size_t positions_mem = with_positions ? ns * n_smers : 0;
          std::size_t response_in_agg = (infos.bw() > 1) ? response_mem : 0;
          phase2 += response_in_agg + positions_mem + ratios_counts;
        }
      }
#ifdef KMINDEX_WITH_COMPRESSION
      if (infos.is_compressed_index())
      {
        block_compressor::Config cfg(infos.get_compression_config());
        std::uint64_t bpb = cfg.get_rows_per_block();
        std::uint64_t cpr_block_size = (bpb * ns) / 8;
        phase1 += cpr_block_size;
      }
#endif

      std::size_t heap_peak = std::max(phase1, phase2);
      indexed_mem.emplace_back(name, heap_peak);
    }

    std::sort(indexed_mem.begin(), indexed_mem.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });

    for (auto& [index_name, mem] : indexed_mem)
    {
      std::size_t acquire_bytes = (budget_bytes > 0 && mem > budget_bytes) ? budget_bytes : mem;
      if (acquire_bytes == budget_bytes && budget_bytes > 0)
        spdlog::warn("Index '{}' memory requirement {:.2f}MB exceeds budget {}MB — will run alone",
                     index_name, mem / (1024.0 * 1024.0), o->memory_budget);

      pool.add_task([&o, &global, index_name, &records, with_positions, &sem, acquire_bytes](int i){
        unused(i);
        sem.acquire(acquire_bytes);
        sem_guard g{sem, acquire_bytes};
        Timer timer;
        auto infos = global.get(index_name);
        spdlog::info("Starting '{}' query ({} samples)", infos.name(), infos.nb_samples());

        batch_query b(infos.nb_samples(),
                      infos.nb_partitions(),
                      infos.smer_size(),
                      o->z,
                      infos.bw(),
                      infos.get_repartition(),
                      infos.get_hash_w(),
                      infos.minim_size()
        );

        std::size_t skip = 0;
        for (auto& record : records)
        {
          if (record.seq.size() >= infos.smer_size() + o->z)
          {
            b.add_query(record.name, record.seq);
          }
          else
          {
            skip++;
          }
        }

        if (skip)
        {
          spdlog::warn("Ignoring {} queries (min query length is {} for index '{}')", skip, o->z + infos.smer_size(), infos.name());
        }

        if (o->uncompressed)
        {
          if (infos.has_uncompressed_partitions())
          {
            spdlog::info("Using uncompressed partitions for index '{}'.", index_name);
            infos.set_compress(false);
            auto fof_bak = fmt::format("{}/kmtricks.fof.bak", infos.get_directory());
            if (fs::exists(fof_bak))
              infos.use_fof(fof_bak);
          }
          else
          {
            spdlog::warn("Index '{}' has no uncompressed partitions, using compressed ones.", index_name);
          }
        }

        kindex ki(infos, o->cache);

        ki.solve_batch(b);
        b.free_smers();

        query_result_agg agg;
        for (auto&& r : b.response())
        {
          agg.add(query_result(std::move(r), o->z, infos, with_positions));
        }
        agg.output(infos, o->output, o->format, "", o->sk_threshold);

        spdlog::info("Index '{}' processed. ({})", infos.name(), timer.formatted());
      });
    }
    pool.join_all();
    spdlog::info("Done ({}).", gtime.formatted());
  }
}
