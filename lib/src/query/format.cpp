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

  std::size_t query_formatter_base::aggregate(const std::vector<query_result>& queries, std::vector<std::uint32_t>& global)
  {
    std::size_t nbk = 0;

    for (auto& qr : queries)
    {
      nbk += qr.nbk();

      std::transform(global.begin(),
                     global.end(),
                     qr.counts().begin(),
                     global.begin(),
                     std::plus<std::uint32_t>{});

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

  query_formatter_t make_formatter(enum format f, double threshold)
  {
    switch (f)
    {
      case format::matrix:
        return std::make_shared<matrix_formatter>(threshold);
      case format::json:
        return std::make_shared<json_formatter>(threshold);
    }

    return nullptr;
  }

  //void write_result(const std::string& res,
  //                  const std::string& index_name,
  //                  const std::string& output_dir,
  //                  enum format f)
  //{
  //  fs::create_directory(output_dir);

  //  std::ofstream out;
  //  switch (f)
  //  {
  //    case format::matrix:
  //      out.open(fmt::format("{}/{}.tsv", output_dir, index_name), std::ios::out);
  //      break;
  //    case format::json:
  //      out.open(fmt::format("{}/{}.json", output_dir, index_name), std::ios::out);
  //      break;
  //  }

  //  out << res;
  //}
}
