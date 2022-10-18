
#include "query.hpp"

#include <iostream>
#include <kmindex/query/query.hpp>
#include <kmindex/index/index.hpp>
#include <kmindex/query/format.hpp>

#include <kmindex/threadpool.hpp>
#include <kseq++/seqio.hpp>


namespace kmq {

  kmq_options_t kmq_query_cli(parser_t parser, kmq_query_options_t options)
  {
    auto cmd = parser->add_command("query", "query");

    cmd->add_param("--index", "index path")
       ->setter(options->global_index_path);

    cmd->add_param("--name", "sub index name")
       ->setter(options->index_name);

    cmd->add_param("--z", "")
       ->setter(options->z);

    cmd->add_param("--threshold", "")
      ->def("0")
       ->setter(options->sk_threshold);

    cmd->add_param("--output", "")
      ->def("")
       ->setter(options->output);

    cmd->add_param("--fastx", "")
       ->setter(options->input);

    add_common_options(cmd, options);

    return options;
  }

  void main_query(kmq_options_t opt)
  {
    kmq_query_options_t o = std::static_pointer_cast<struct kmq_query_options>(opt);

    //index i(o->global_index_path);

    //auto ki = i.get(o->index_name);
    //auto info = ki.infos();
    //kmq::kinfos ks{};
    //auto ids = info.get_sample_ids();
    //ks.kmer_size = info.smer_size() + o->z;
    //ks.minim_size = info.minim_size();
    //ks.smer_size = info.smer_size();
    //ks.nb_samples = info.nb_samples();

    //auto repart = info.get_repartition();
    //auto hw = info.get_hash_w();

    //std::vector<query> queries;

    //klibpp::KSeq record;
    //klibpp::SeqStreamIn iss(o->input.c_str());
    //std::mutex m_mutex;

    //ThreadPool pool(8);
    //kmq::Timer t1;

    //while (iss >> record)
    //{
    //  pool.add_task([record, &m_mutex, &queries, &ks, repart, hw](int i){
    //    auto Q = query(record.name, record.seq, ks, repart, hw);
    //    std::unique_lock<std::mutex> m_lock(m_mutex);
    //    queries.push_back(std::move(Q));
    //  });
    //  //queries.push_back(query(record.name, record.seq, ks, repart, hw));
    //}
    //pool.join_all();
    //std::cout << t1.elapsed<std::chrono::milliseconds>().count() << std::endl;

    //kmq::Timer t2;
    //ki.resolve(queries, 4);

    //std::cout << t2.elapsed<std::chrono::milliseconds>().count() << std::endl;

    //kmq::Timer t3;
    //json_formatter f;

    //std::string ok = f.format(o->index_name, ids, queries);
    //std::cout << t3.elapsed<std::chrono::milliseconds>().count() << std::endl;
    //
    //

    index global(o->global_index_path);

    auto infos = global.get(o->index_name);

    kindex ki(infos);

    query_result_agg agg;

    klibpp::KSeq record;
    klibpp::SeqStreamIn iss(o->input.c_str());

    while (iss >> record)
    {
      query q(record.name, record.seq, infos.smer_size(), o->z, infos.nb_samples(), 0.0);
      agg.add(ki.resolve(q));
    }

    json_formatter f;

    std::string resp = f.format(infos.name(), infos.samples(), agg);

    for (auto& rr : agg)
    {
      for (const auto& d : rr.ratios())
      {
        std::cout << d << " ";
      }
      std::cout << std::endl;
    }

    //std::cout << resp << std::endl;
  }
}
