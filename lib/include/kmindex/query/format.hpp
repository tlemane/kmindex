#ifndef FORMAT_HPP_QFHCMIRK
#define FORMAT_HPP_QFHCMIRK

#include <nlohmann/json.hpp>
#include <kmindex/query/query_results.hpp>
#include <kmindex/query/query.hpp>

using json = nlohmann::json;

namespace kmq {

  enum class format
  {
    matrix,
    json
  };


  class query_formatter_base
  {
    public:
      virtual std::string format(const std::string& name,
                               const std::vector<std::string>& sample_ids,
                               const query_result_agg& queries) = 0;
  };

  class matrix_formatter : query_formatter_base
  {
    public:
    virtual std::string format(const std::string& name,
                               const std::vector<std::string>& sample_ids,
                               const query_result_agg& queries) override
    {
      return "";
    }
  };

  class json_formatter : query_formatter_base
  {
    public:
    virtual std::string format(const std::string& name,
                               const std::vector<std::string>& sample_ids,
                               const query_result_agg& queries) override
    {
      nlohmann::json data;
      data[name] = json({});

      for (auto& qr : queries)
      {
        data[name][qr.name()] = json({});

        const auto& ratios = qr.ratios();

        for (std::size_t i = 0; i < sample_ids.size(); ++i)
        {
          data[name][qr.name()][sample_ids[i]] = ratios[i];
        }
      }

      return data.dump(4);
    }

  };
}

#endif /* end of include guard: FORMAT_HPP_QFHCMIRK */
