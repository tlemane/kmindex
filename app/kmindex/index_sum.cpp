#include <iostream>

#include "index_sum.hpp"
#include <kmindex/index/index.hpp>
#include <kmindex/exceptions.hpp>
#include <spdlog/spdlog.h>
#include <kmindex/threadpool.hpp>
#include <kmindex/index/sum_index.hpp>
#include <kmindex/query/query.hpp>

namespace kmq {

  kmq_options_t kmq_sum_index_cli(parser_t parser, kmq_sum_index_options_t options)
  {
    auto cmd = parser->add_command("sum-index", 
      "Make a lightweight summarized index, at query time,\n" 
      "                reports only the number samples containing each k-mer. (experimental)");

    cmd->add_param("-i/--global-index", "Global index path.")
      ->meta("STR")
      ->setter(options->global_index_path);

    cmd->add_param("-n/--name", "Index name.")
       ->meta("STR")
       ->setter(options->index_name);

    cmd->add_param("-c/--fp-correction", "False positive rate correction factor in [0.0,1.0].")
       ->meta("FLOAT")
       ->def("0.0")
       ->checker(bc::check::is_number)
       ->checker(bc::check::f::range(0.0, 1.0))
       ->setter(options->correction);

    cmd->add_param("-e/--estimate-correction", "Estimate the false positive rate correction factor.")
       ->as_flag()
       ->setter(options->estimate_correction);

    cmd->add_param("-s/--nbk", "Number of k-mers to use for estimating the correction factor (only with -e).")
       ->meta("INT")
       ->def("10000")
       ->checker(bc::check::is_number)
       ->setter(options->nbkc);
    
    add_common_options(cmd, options, true);

    return options;
  }

  double estimate_correction(const index_infos& infos, std::size_t nb_threads, std::size_t nbk)
  {
    auto seq = random_sequence(nbk);
    
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

    query_result res(std::move(bq.response().front()), 0, infos, false);
    return std::accumulate(res.ratios().begin(), res.ratios().end(), 0.0) / infos.nb_samples();
  }

  void main_sum_index(kmq_options_t opt)
  {
    Timer gtime;
    kmq_sum_index_options_t o = std::static_pointer_cast<struct kmq_sum_index_options>(opt);
    index i(o->global_index_path);
    auto sub = i.get(o->index_name);
    
    if (sub.is_compressed_index())
    {
      throw kmq_error(fmt::format("Compressed index '{}' is not supported.", o->index_name));
      return;
    }

    if (sub.bw() != 1)
    {
      throw kmq_error(fmt::format("Abundance index '{}' is not supported.", o->index_name));
      return;
    }

    spdlog::info("Process index '{}'", o->index_name);

    if (o->estimate_correction)
    {
      o->correction = estimate_correction(sub, o->nb_threads, o->nbkc);
      spdlog::info("Estimated false positive rate correction factor: {}", o->correction);
    }

    sum_index builder(&sub);
  
    ThreadPool pool(o->nb_threads);
    for (std::size_t p = 0; p < sub.nb_partitions(); ++p)
    {
      pool.add_task([p, &builder, c=o->correction](int) { 
        spdlog::info("Process partition {}", p);
        builder.sum_partition(p, c);
        spdlog::info("Done partition {}", p);
      });
    }

    pool.join_all();

    spdlog::info("Index '{}' processed. ({})", o->index_name, gtime.formatted());
  }
}
