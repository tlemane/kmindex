#ifndef FORMAT_HPP_QFHCMIRK
#define FORMAT_HPP_QFHCMIRK

#include <sstream>
#include <iomanip>
#include <memory>

#include <nlohmann/json.hpp>
#include <kmindex/query/query.hpp>
#include <kmindex/query/query_results.hpp>
#include <kmindex/index/index_infos.hpp>

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

      query_formatter_base(double threshold);

      virtual ~query_formatter_base() = default;

      virtual void format(const index_infos& infos,
                          const query_result& response,
                          std::ostream& os) = 0;

      virtual void merge_format(const index_infos&,
                                const std::string& name,
                                const std::vector<query_result>& responses,
                                std::ostream& os) = 0;

    protected:
      std::size_t aggregate(const std::vector<query_result>& queries, std::vector<uint32_t>& global);

    protected:
      double m_threshold {0};
  };

  using query_formatter_t = std::shared_ptr<query_formatter_base>;

  class matrix_formatter : public query_formatter_base
  {
    public:
      matrix_formatter(double threshold);
      virtual ~matrix_formatter() = default;
    private:
      void write_headers(std::ostream& ss, const index_infos& infos);

    public:

      virtual void format(const index_infos& infos,
                          const query_result& response,
                          std::ostream& os) override;

      virtual void merge_format(const index_infos&,
                                const std::string& name,
                                const std::vector<query_result>& responses,
                                std::ostream& os) override;
  };

  class json_formatter : public query_formatter_base
  {
    public:
      json_formatter(double threshold);
      ~json_formatter();

    private:
      void write_one(json& data,
                     const std::string& name,
                     const std::vector<double>& ratios,
                     const std::vector<std::string>& sample_ids);

    public:
      virtual void format(const index_infos& infos,
                          const query_result& response,
                          std::ostream& os) override;

      virtual void merge_format(const index_infos&,
                                const std::string& name,
                                const std::vector<query_result>& responses,
                                std::ostream& os) override;

      const json& get_json() const;

      //virtual json jformat(const std::string& index_name,
      //                     const std::vector<std::string>& sample_ids,
      //                     const query_result_agg& queries);

      //virtual json jmerge_format(const std::string& index_name,
      //                           const std::vector<std::string>& sample_ids,
      //                           const query_result_agg& queries,
      //                           const std::string& qname);
    private:
      std::ostream* m_os;
      json m_json;
  };

  query_formatter_t make_formatter(enum format f, double threshold);
}

#endif /* end of include guard: FORMAT_HPP_QFHCMIRK */
