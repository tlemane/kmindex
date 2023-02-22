#include <kmindex/query/format.hpp>
#include <kmindex/utils.hpp>

namespace kmq {

  enum format str_to_format(const std::string& f)
  {
    if (f == "matrix")
      return format::matrix;
    return format::json;
  }

  query_formatter_base::query_formatter_base(double threshold)
    : m_threshold(threshold)
  {

  }

  std::size_t query_formatter_base::aggregate(const std::vector<query_result>& queries,
                                              std::vector<std::uint32_t>& global)
  {
    std::size_t nbk = 0;

    for (auto& qr : queries)
    {
      nbk += qr.nbk();

      std::transform(std::begin(global),
                     std::end(global),
                     std::begin(qr.counts()),
                     std::begin(global),
                     std::plus<std::uint32_t>{}
      );

    }

    return nbk;
  }

  std::size_t query_formatter_base::aggregate_c(const std::vector<query_result>& queries,
                                                std::vector<std::uint32_t>& global)
  {
    std::size_t nbk = 0;

    for (auto& qr : queries)
    {
      nbk += qr.nbk();

      std::transform(std::begin(global),
                     std::end(global),
                     std::begin(qr.counts()),
                     std::begin(global),
                     [](auto& lhs, auto& rhs) { return std::min(lhs, rhs); }
      );
    }

    return nbk;
  }

  matrix_formatter::matrix_formatter(double threshold)
    : query_formatter_base(threshold)
  {

  }

  void matrix_formatter::write_headers(std::ostream& ss, const index_infos& infos)
  {
    ss << fmt::format("{}\t{}\n", infos.name(), fmt::join(infos.samples(), "\t"));
  }

  void matrix_formatter::format(const index_infos& infos,
                                const query_result& response,
                                std::ostream& os)
  {
    os << fmt::format("{}:{}\t{}\n", infos.name(), response.name(), fmt::join(response.ratios(), "\t"));
  }

  void matrix_formatter::merge_format(const index_infos& infos,
                                      const std::string& name,
                                      const std::vector<query_result>& responses,
                                      std::ostream& os)
  {
    write_headers(os, infos);
    std::vector<std::uint32_t> global(infos.nb_samples(), 0);
    std::vector<double> ratios; ratios.reserve(infos.nb_samples());
    std::size_t nbk = this->aggregate(responses, global);

    for (auto& c : global)
      ratios.push_back(c / static_cast<double>(nbk));

    os << fmt::format("{}\t{}\n", name, fmt::join(ratios, "\t"));
  }

  json_formatter::json_formatter(double threshold)
    : query_formatter_base(threshold)
  {
    m_json = json({});
  }

  json_formatter::~json_formatter()
  {
    (*m_os) << m_json.dump(4);
  }

  void json_formatter::format(const index_infos& infos,
                              const query_result& response,
                              std::ostream& os)
  {
    m_os = &os;
    m_json[infos.name()][response.name()] = json({});

    auto& j = m_json[infos.name()][response.name()];
    for (std::size_t i = 0; i < infos.nb_samples(); ++i)
    {
      if (response.ratios()[i] >= this->m_threshold)
        j[infos.samples()[i]] = response.ratios()[i];
    }
  }

  void json_formatter::merge_format(const index_infos& infos,
                                    const std::string& name,
                                    const std::vector<query_result>& responses,
                                    std::ostream& os)
  {
    m_os = &os;
    std::vector<std::uint32_t> global(infos.nb_samples(), 0);
    std::size_t nbk = this->aggregate(responses, global);

    m_json[infos.name()] = json({});
    m_json[infos.name()][name] = json({});

    auto& j = m_json[infos.name()][name];

    for (std::size_t i = 0; i < infos.nb_samples(); ++i)
    {
      double v = global[i] / static_cast<double>(nbk);
      if (v >= this->m_threshold)
        j[infos.samples()[i]] = v;
    }
  }

  const json& json_formatter::get_json() const
  {
    return m_json;
  }

  matrix_formatter_abs::matrix_formatter_abs(double threshold)
    : matrix_formatter(threshold)
  {

  }

  void matrix_formatter_abs::format(const index_infos& infos,
                                    const query_result& response,
                                    std::ostream& os)
  {
    os << fmt::format("{}:{}\t{}\n", infos.name(), response.name(), fmt::join(response.counts(), "\t"));
  }

  void matrix_formatter_abs::merge_format(const index_infos& infos,
                                          const std::string& name,
                                          const std::vector<query_result>& responses,
                                          std::ostream& os)
  {
    this->write_headers(os, infos);

    std::vector<std::uint32_t> global(infos.nb_samples(), std::numeric_limits<std::uint32_t>::max());

    std::size_t nbk = this->aggregate_c(responses, global);

    for (auto& c : global)
      c /= nbk;

    os << fmt::format("{}:{}\t{}\n", infos.name(), name, fmt::join(global, "\t"));
  }


  json_formatter_abs::json_formatter_abs(double threshold)
    : json_formatter(threshold)
  {

  }

  void json_formatter_abs::format(const index_infos& infos,
                                  const query_result& response,
                                  std::ostream& os)
  {

  }

  void json_formatter_abs::merge_format(const index_infos& infos,
                                        const std::string& name,
                                        const std::vector<query_result>& responses,
                                        std::ostream& os)
  {

  }

  query_formatter_t make_formatter(enum format f, double threshold, std::size_t bw)
  {
    switch (f)
    {
      case format::matrix:
        return bw == 1 ? std::make_shared<matrix_formatter>(threshold)
                       : std::make_shared<matrix_formatter_abs>(threshold);
      case format::json:
        return bw == 1 ? std::make_shared<json_formatter>(threshold)
                       : std::make_shared<json_formatter_abs>(threshold);
    }

    return nullptr;
  }


}
