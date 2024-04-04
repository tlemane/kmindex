#ifndef REQUEST_HPP_NZPDLF91
#define REQUEST_HPP_NZPDLF91

#include <kmindex/exceptions.hpp>
#include <kmindex/index/index.hpp>
#include <kmindex/index/kindex.hpp>
#include <kmindex/query/format.hpp>
#include <kmindex/query/query_results.hpp>

#include <nlohmann/json.hpp>

#include <spdlog/spdlog.h>

#include "utils.hpp"

using json = nlohmann::json;

namespace kmq {

  class request
  {
    public:
      request(const json& data)
      {
        parse_json(data);
      }

      std::string solve(const index& gindex) const
      {
        return m_json ? solve_json(gindex) : solve_tsv(gindex);
      }

      std::string solve_json(const index& gindex) const
      {
        std::vector<json> responses;

        for (auto& i : m_index)
        {
          auto infos = gindex.get(i);
          kindex ki(infos);
          //smer_hasher sh(infos.get_repartition(), infos.get_hash_w(), infos.minim_size());

          batch_query bq(
            infos.nb_samples(),
            infos.nb_partitions(),
            infos.smer_size(),
            m_z,
            infos.bw(),
            infos.get_repartition(),
            infos.get_hash_w(),
            infos.minim_size());

          for (auto& s : m_seq)
          {
            if (s.size() < (infos.smer_size() + m_z))
              throw kmq_invalid_request(
                  fmt::format(
                    "Sequence too small: {}, min size is {}.", s.size(), infos.smer_size() + m_z));
            bq.add_query(m_name, s);
          }

          for (std::size_t p = 0; p < infos.nb_partitions(); ++p)
            ki.solve_one(bq, p);

          query_result_agg agg;
          for (auto&& r : bq.response())
            agg.add(query_result(std::move(r), m_z, infos, (m_format == format::json) ? false: true));


          std::ofstream nullstream; nullstream.setstate(std::ios_base::badbit);

          std::shared_ptr<json_formatter> jformat =
            std::static_pointer_cast<json_formatter>(
                make_formatter(m_format, m_r, infos.bw()));

          if (m_seq.size() == 1)
            jformat->format(infos, agg.results()[0], nullstream);
          else
            jformat->merge_format(infos, m_name, agg.results(), nullstream);

          responses.push_back(jformat->get_json());
        }

        json response;
        for (auto& r : responses)
          response.update(r);

        return response.dump(4);
      }

      std::string solve_tsv(const index& gindex) const
      {
        std::stringstream ss;

        for (auto& i : m_index)
        {
          auto infos = gindex.get(i);
          kindex ki(infos);
          //smer_hasher sh(infos.get_repartition(), infos.get_hash_w(), infos.minim_size());

          batch_query bq(
            infos.nb_samples(),
            infos.nb_partitions(),
            infos.smer_size(),
            m_z,
            infos.bw(),
            infos.get_repartition(),
            infos.get_hash_w(),
            infos.minim_size());

          for (auto& s : m_seq)
            bq.add_query(m_name, s);

          for (std::size_t p = 0; p < infos.nb_partitions(); ++p)
            ki.solve_one(bq, p);

          query_result_agg agg;
          for (auto&& r : bq.response())
            agg.add(query_result(std::move(r), m_z, infos));

          auto tformat = make_formatter(format::matrix, m_r, infos.bw());
          tformat->merge_format(infos, m_name, agg.results(), ss);
          ss << '\n';
        }

        return ss.str();
      }

    private:

      void parse_json(const json& data)
      {
        if (!data.contains("index"))
          throw kmq_invalid_request("'index' entry is missing.");
        if (!data.contains("id"))
          throw kmq_invalid_request("'id' entry is missing.");
        if (!data.contains("seq") && !data.contains("fastx"))
          throw kmq_invalid_request("'seq' or 'fastx' should be specified.");
        if (data.contains("seq") && data.contains("fastx"))
          throw kmq_invalid_request("'seq' and 'fastx' are mutually exclusive.");

        if (!data.contains("z"))
          throw kmq_invalid_request("'z' entry is missing.");

        if (!data.contains("format"))
        {
          m_json = true;
        }
        else
        {
          if (data["format"] == "json" || data["format"] == "json_vec")
          {
            m_json = true;
            m_format = str_to_format(data["format"]);
          }
          else if (data["format"] == "tsv")
            m_json = false;
          else
            throw kmq_invalid_request(
              fmt::format("'format':'{}' not supported, should be 'json' or 'tsv'.", data["format"]));
        }

        if (data.contains("r"))
        {
          if (!data["r"].is_number())
            throw kmq_invalid_request(
              fmt::format("'r' should be a float in [0.0, 1.0]"));
          m_r = data["r"].get<double>();
          if (m_r < 0.0 || m_r > 1.0)
            throw kmq_invalid_request(
              fmt::format("'r': {}, should be in [0.0, 1.0]", data["r"].get<double>()));
        }

        m_name = data["id"];

        for (auto& i : data["index"])
          m_index.push_back(i);

        if (data.contains("seq"))
        {
          for (auto& i : data["seq"])
            m_seq.push_back(i);
        }
        else
        {
          klibpp::KSeq record;

          std::string fastx_str = data["fastx"];

          string_iterator_wrapper it(fastx_str);
          SeqStreamString iss(&it);

          while (iss >> record)
          {
            m_seq.emplace_back(std::move(record.seq));
          }
        }

        m_z = data["z"];
      }

    public:
      std::string m_name;
      std::vector<std::string> m_index;
      std::vector<std::string> m_seq;
      std::size_t m_z {0};
      double m_r {0.0};
      bool m_json {true};
      enum format m_format;
  };

}


#endif /* end of include guard: REQUEST_HPP_NZPDLF91 */
