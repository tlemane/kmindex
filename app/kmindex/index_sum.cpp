#include <iostream>

#include "index_sum.hpp"
#include <kmindex/index/index.hpp>
#include <kmindex/exceptions.hpp>
#include <spdlog/spdlog.h>
#include <kmindex/threadpool.hpp>
#include <kmindex/index/sum_index.hpp>

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

    add_common_options(cmd, options, true);

    return options;
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

    spdlog::info("Process index '{}'", o->index_name);
    sum_index builder(&sub);
  
    ThreadPool pool(o->nb_threads);
    for (std::size_t p = 0; p < sub.nb_partitions(); ++p)
    {
      pool.add_task([p, &builder](int) { 
        spdlog::info("Process partition {}", p);
        builder.sum_partition(p);
        spdlog::info("Done partition {}", p);
      });
    }

    pool.join_all();

    spdlog::info("Index '{}' processed. ({})", o->index_name, gtime.formatted());
  }
}
