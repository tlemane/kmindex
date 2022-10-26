#ifndef REQUEST_HPP_NZPDLF91
#define REQUEST_HPP_NZPDLF91

#include <kmindex/exceptions.hpp>
#include <kmindex/index/index.hpp>
#include <kmindex/index/kindex.hpp>
#include <kmindex/query/format.hpp>
#include <kmindex/query/query_results.hpp>

#include <nlohmann/json.hpp>

#include <spdlog/spdlog.h>

using json = nlohmann::json;

namespace kmq {

  class request
  {
    public:
      request(const json& data)
      {
        parse_json(data);
      }

      json solve(const index& gindex) const
      {
        json_formatter jformat;

        std::vector<json> responses;

        for (auto& i : m_index)
        {
          auto infos = gindex.get(i);
          kindex ki(infos);

          query_result_agg agg;

          for (auto& s : m_seq)
          {
            query q(m_name, s, infos.smer_size(), m_z, infos.nb_samples(), 0.0);
            agg.add(ki.resolve(q));
          }
          responses.push_back(jformat.jmerge_format(infos.name(), infos.samples(), agg, m_name));
        }

        json response;
        for (auto& r : responses)
          response.update(r);

        return response;
      }

    private:

      void parse_json(const json& data)
      {
        if (!data.contains("index"))
          throw kmq_invalid_request("'index' entry is missing.");
        if (!data.contains("id"))
          throw kmq_invalid_request("'id' entry is missing.");
        if (!data.contains("seq"))
          throw kmq_invalid_request("'seq' entry is missing.");
        if (!data.contains("z"))
          throw kmq_invalid_request("'z' entry is missing.");

        m_name = data["id"];

        for (auto& i : data["index"])
          m_index.push_back(i);

        for (auto& i : data["seq"])
          m_seq.push_back(i);

        m_z = data["z"];
      }

    public:
      std::string m_name;
      std::vector<std::string> m_index;
      std::vector<std::string> m_seq;
      std::size_t m_z {0};
  };

}


#endif /* end of include guard: REQUEST_HPP_NZPDLF91 */
