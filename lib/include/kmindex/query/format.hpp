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

  enum format str_to_format(const std::string& f);

  class query_formatter_base
  {
    public:
      virtual std::string format(const std::string& index_name,
                                 const std::vector<std::string>& sample_ids,
                                 const query_result_agg& queries) = 0;

      virtual std::string merge_format(const std::string& index_name,
                                       const std::vector<std::string>& sample_ids,
                                       const query_result_agg& queries,
                                       const std::string& qname) = 0;

    protected:
      std::size_t aggregate(const query_result_agg& queries, std::vector<uint32_t>& global);
  };

  using query_formatter_t = std::shared_ptr<query_formatter_base>;

  class matrix_formatter : public query_formatter_base
  {
    private:
      void write_headers(std::stringstream& ss, const std::vector<std::string>& sample_ids);

      void write_one(std::stringstream& ss,
                     const std::string& name,
                     const std::vector<double>& ratios);

    public:
      virtual std::string format(const std::string& name,
                                 const std::vector<std::string>& sample_ids,
                                 const query_result_agg& queries) override;

      virtual std::string merge_format(const std::string& index_name,
                                       const std::vector<std::string>& sample_ids,
                                       const query_result_agg& queries,
                                       const std::string& qname) override;
  };

  class json_formatter : public query_formatter_base
  {
    private:
      void write_one(json& data,
                     const std::string& name,
                     const std::vector<double>& ratios,
                     const std::vector<std::string>& sample_ids);

    public:
      virtual std::string format(const std::string& index_name,
                                 const std::vector<std::string>& sample_ids,
                                 const query_result_agg& queries) override;

      virtual std::string merge_format(const std::string& index_name,
                                       const std::vector<std::string>& sample_ids,
                                       const query_result_agg& queries,
                                       const std::string& qname) override;

      virtual json jformat(const std::string& index_name,
                           const std::vector<std::string>& sample_ids,
                           const query_result_agg& queries);

      virtual json jmerge_format(const std::string& index_name,
                                 const std::vector<std::string>& sample_ids,
                                 const query_result_agg& queries,
                                 const std::string& qname);
  };

  query_formatter_t get_formatter(enum format f);

  void write_result(const std::string& res,
                    const std::string& index_name,
                    const std::string& output_dir,
                    enum format f);
}

#endif /* end of include guard: FORMAT_HPP_QFHCMIRK */
