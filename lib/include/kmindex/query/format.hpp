#ifndef FORMAT_HPP_QFHCMIRK
#define FORMAT_HPP_QFHCMIRK

#include <sstream>
#include <iomanip>
#include <memory>

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

  inline enum format str_to_format(const std::string& f)
  {
    if (f == "matrix")
      return format::matrix;
    return format::json;
  }

  class query_formatter_base
  {
    public:
      virtual std::string format(const std::string& name,
                               const std::vector<std::string>& sample_ids,
                               const query_result_agg& queries) = 0;
  };

  class matrix_formatter : public query_formatter_base
  {
    public:
    virtual std::string format(const std::string& name,
                               const std::vector<std::string>& sample_ids,
                               const query_result_agg& queries) override
    {
      unused(name);

      std::stringstream ss;
      ss << std::setprecision(2);
      ss << "ID\t";
      for (auto& s : sample_ids)
      {
        ss << s << '\t';
      }
      ss.seekp(-1, ss.cur);
      ss << '\n';

      for (auto& qr : queries)
      {
        ss << qr.name() << '\t';

        for (auto& r : qr.ratios())
        {
          ss << r << '\t';
        }
        ss.seekp(-1, ss.cur);
        ss << '\n';
      }

      return ss.str();
    }
  };

  class json_formatter : public query_formatter_base
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

  inline std::shared_ptr<query_formatter_base> get_formatter(enum format f)
  {
    switch (f)
    {
      case format::matrix:
        return std::make_shared<matrix_formatter>();
      case format::json:
        return std::make_shared<json_formatter>();
    }

    return nullptr;
  }

  inline void write_result(const std::string& res,
                           const std::string& index_name,
                           const std::string& output_dir,
                           enum format f)
  {
    fs::create_directory(output_dir);

    std::ofstream out;
    switch (f)
    {
      case format::matrix:
        out.open(fmt::format("{}/{}.tsv", output_dir, index_name), std::ios::out);
        break;
      case format::json:
        out.open(fmt::format("{}/{}.json", output_dir, index_name), std::ios::out);
        break;
    }

    out << res;
  }


}

#endif /* end of include guard: FORMAT_HPP_QFHCMIRK */
