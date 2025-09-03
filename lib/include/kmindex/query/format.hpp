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
    json,
    json_with_positions,
    jsonl,
    jsonl_with_positions
  };

  std::string format_to_fext(enum format f);
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

      virtual void write_headers(std::ostream& os, const index_infos& infos)
      {
        unused(os); unused(infos);
      }

    protected:
      std::size_t aggregate(const std::vector<query_result>& queries, std::vector<uint32_t>& global);

      std::size_t aggregate_c(const std::vector<query_result>& queries, std::vector<uint32_t>& global);
    protected:
      double m_threshold {0};
  };

  using query_formatter_t = std::shared_ptr<query_formatter_base>;

  class matrix_formatter : public query_formatter_base
  {
    public:
      matrix_formatter(double threshold);
      virtual ~matrix_formatter() = default;

    public:
      virtual void write_headers(std::ostream& ss,
                                 const index_infos& infos) override;

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

    public:
      virtual void format(const index_infos& infos,
                          const query_result& response,
                          std::ostream& os) override;

      virtual void merge_format(const index_infos&,
                                const std::string& name,
                                const std::vector<query_result>& responses,
                                std::ostream& os) override;

      const json& get_json() const;

    protected:
      std::ostream* m_os;
      json m_json;
  };

  class jsonl_formatter : public query_formatter_base
  {
    public:
      jsonl_formatter(double threshold);
      ~jsonl_formatter();

    public:
      virtual void format(const index_infos& infos,
                          const query_result& response,
                          std::ostream& os) override;
      
      virtual void merge_format(const index_infos&,
                                const std::string& name,
                                const std::vector<query_result>& responses,
                                std::ostream& os) override;
    
    protected:
        std::ostream* m_os;
  };

  class json_wp_formatter : public json_formatter
  {
    public:
      json_wp_formatter(double threshold);

    public:
      virtual void format(const index_infos& infos,
                          const query_result& response,
                          std::ostream& os) override;

      virtual void merge_format(const index_infos&,
                                const std::string& name,
                                const std::vector<query_result>& responses,
                                std::ostream& os) override;
  };

  class jsonl_wp_formatter : public jsonl_formatter
  {
    public:
      jsonl_wp_formatter(double threshold);
    
    public:
      virtual void format(const index_infos& infos,
                          const query_result& response,
                          std::ostream& os) override;

      virtual void merge_format(const index_infos&,
                                const std::string& name,
                                const std::vector<query_result>& responses,
                                std::ostream& os) override;

  };

  class matrix_formatter_abs : public matrix_formatter
  {
    public:
      matrix_formatter_abs(double threshold);

    public:
      virtual void format(const index_infos& infos,
                          const query_result& response,
                          std::ostream& os) override;

      virtual void merge_format(const index_infos&,
                                const std::string& name,
                                const std::vector<query_result>& responses,
                                std::ostream& os) override;
  };

  class json_formatter_abs : public json_formatter
  {
    public:
      json_formatter_abs(double threshold);

    public:
      virtual void format(const index_infos& infos,
                          const query_result& response,
                          std::ostream& os) override;

      virtual void merge_format(const index_infos&,
                                const std::string& name,
                                const std::vector<query_result>& responses,
                                std::ostream& os) override;
  };

  class jsonl_formatter_abs : public jsonl_formatter
  {
    public:
      jsonl_formatter_abs(double threshold);

    public:
      virtual void format(const index_infos& infos,
                          const query_result& response,
                          std::ostream& os) override;
      
      virtual void merge_format(const index_infos&,
                                const std::string& name,
                                const std::vector<query_result>& responses,
                                std::ostream& os) override;
  };

  class json_wp_formatter_abs : public json_formatter
  {
    public:
      json_wp_formatter_abs(double threshold);

    public:
      virtual void format(const index_infos& infos,
                          const query_result& response,
                          std::ostream& os) override;

      virtual void merge_format(const index_infos&,
                                const std::string& name,
                                const std::vector<query_result>& responses,
                                std::ostream& os) override;
  };

  class jsonl_wp_formatter_abs : public jsonl_formatter
  {
    public:
      jsonl_wp_formatter_abs(double threshold);
    public:
      virtual void format(const index_infos& infos,
                          const query_result& response,
                          std::ostream& os) override;  

      virtual void merge_format(const index_infos& infos,
                                const std::string& name,
                                const std::vector<query_result>& responses,
                                std::ostream& os) override;
  };

  query_formatter_t make_formatter(enum format f, double threshold, std::size_t bw = 1);
}

#endif /* end of include guard: FORMAT_HPP_QFHCMIRK */
