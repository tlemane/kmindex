#include <kmindex/query/format.hpp>
#include <kmindex/utils.hpp>

#include <iostream>

namespace kmq {

  enum format str_to_format(const std::string& f)
  {
    if (f == "matrix")
      return format::matrix;
    else if (f == "json")
      return format::json;
    else if (f == "jsonl")
      return format::jsonl;
    else if (f == "json_vec")
      return format::json_with_positions;
    else if (f == "jsonl_vec")
      return format::jsonl_with_positions;
    else
      return format::json;
  }

  std::string format_to_fext(enum format f)
  {
    switch (f)
    {
      case format::matrix:
        return "tsv";
      case format::json:
      case format::json_with_positions:
        return "json";
      case format::jsonl:
      case format::jsonl_with_positions:
        return "jsonl";
      default:
        return "json";
    }
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
    std::size_t nbq = 0;

    for (auto& qr : queries)
    {
      ++nbq;
      // nbk += qr.nbk();

      std::transform(std::begin(global),
                     std::end(global),
                     std::begin(qr.counts()),
                     std::begin(global),
                     std::plus<std::uint32_t>{}
                     // [](auto& lhs, auto& rhs) { return std::min(lhs, rhs); }
      );
    }

    return nbq;
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

  jsonl_formatter::jsonl_formatter(double threshold)
    : query_formatter_base(threshold)
  {
  }

  jsonl_formatter::~jsonl_formatter()
  {
  }

  void jsonl_formatter::format(const index_infos& infos,
                              const query_result& response,
                              std::ostream& os)
  {
    m_os = &os;
    auto jj = json({});
    jj["index"] = infos.name();
    jj["query"] = response.name();
    std::map<std::string, double> samples;

    for (std::size_t i = 0; i < infos.nb_samples(); ++i)
    {
      if (response.ratios()[i] >= this->m_threshold)
      {
        samples[infos.samples()[i]] = response.ratios()[i];
      }
    }
    jj["samples"] = std::move(samples);
    (*m_os) << jj.dump() << "\n";
  }

  json_wp_formatter::json_wp_formatter(double threshold)
    : json_formatter(threshold)
  {
    m_json = json({});
  }

  void json_wp_formatter::format(const index_infos& infos,
                              const query_result& response,
                              std::ostream& os)
  {
    m_os = &os;
    m_json[infos.name()][response.name()] = json({});

    auto& j = m_json[infos.name()][response.name()];
    for (std::size_t i = 0; i < infos.nb_samples(); ++i)
    {
      if (response.ratios()[i] >= this->m_threshold)
      {
        auto jj = json({});
        jj["R"] = response.ratios()[i];
        jj["P"] = response.positions()[i];
        j[infos.samples()[i]] = std::move(jj);
      }
    }
  }

  void json_wp_formatter::merge_format(const index_infos& infos,
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
      {
        auto jj = json::array({});
        for (auto& r : responses)
        {
          jj.push_back(r.positions()[i]);
        }
        j[infos.samples()[i]]["R"] = v;
        j[infos.samples()[i]]["P"] = std::move(jj);

      }
    }
  }

  jsonl_wp_formatter::jsonl_wp_formatter(double threshold)
    : jsonl_formatter(threshold)
  {
  }

  void jsonl_wp_formatter::format(const index_infos& infos,
                              const query_result& response,
                              std::ostream& os)
  {
    auto jj = json({});
    jj["index"] = infos.name();
    jj["query"] = response.name();
    
    std::map<std::string, json> samples;

    for (std::size_t i = 0; i < infos.nb_samples(); ++i)
    {
      if (response.ratios()[i] >= this->m_threshold)
      {
        auto jjj = json({});
        jjj["R"] = response.ratios()[i];
        jjj["P"] = response.positions()[i];
        samples[infos.samples()[i]] = std::move(jjj);
      }
    }
    jj["samples"] = std::move(samples);
    os << jj.dump() << "\n";
  }

  void jsonl_wp_formatter::merge_format(const index_infos& infos,
                                    const std::string& name,
                                    const std::vector<query_result>& responses,
                                    std::ostream& os)
  {
    auto jj = json({});
    jj["index"] = infos.name();
    jj["query"] = name;

    std::map<std::string, json> samples;
    std::vector<std::uint32_t> global(infos.nb_samples(), 0);
    std::size_t nbk = this->aggregate(responses, global);

    for (std::size_t i = 0; i < infos.nb_samples(); ++i)
    {
      double v = global[i] / static_cast<double>(nbk);
      if (v >= this->m_threshold)
      {
        auto jjj = json::array({});
        for (auto& r : responses)
        {
          jjj.push_back(r.positions()[i]);
        }
        json sample;
        sample["R"] = v;
        sample["P"] = std::move(jjj);
        samples[infos.samples()[i]] = std::move(sample);
      }
    }
    jj["samples"] = std::move(samples);
    os << jj.dump() << "\n";
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

  void jsonl_formatter::merge_format(const index_infos& infos,
                                    const std::string& name,
                                    const std::vector<query_result>& responses,
                                    std::ostream& os)
  {
    auto jj = json({});
    jj["index"] = infos.name();
    jj["query"] = name;

    std::map<std::string, double> samples;
    std::vector<std::uint32_t> global(infos.nb_samples(), 0);
    std::size_t nbk = this->aggregate(responses, global);

    for (std::size_t i = 0; i < infos.nb_samples(); ++i)
    {
      double v = global[i] / static_cast<double>(nbk);
      if (v >= this->m_threshold)
      {
        samples[infos.samples()[i]] = v;
      }
    }
    jj["samples"] = std::move(samples);
    os << jj.dump() << "\n";
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

    std::vector<std::uint32_t> global(infos.nb_samples(), 0);

    std::size_t nbq = this->aggregate_c(responses, global);

    for (auto& c : global)
      c /= nbq;

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
    m_os = &os;
    m_json[infos.name()][response.name()] = json({});

    auto& j = m_json[infos.name()][response.name()];
    for (std::size_t i = 0; i < infos.nb_samples(); ++i)
    {
      j[infos.samples()[i]] = response.counts()[i];
    }
  }

  void json_formatter_abs::merge_format(const index_infos& infos,
                                        const std::string& name,
                                        const std::vector<query_result>& responses,
                                        std::ostream& os)
  {
    m_os = &os;
    m_json[infos.name()][name] = json({});

    std::vector<std::uint32_t> global(infos.nb_samples(), 0);

    std::size_t nbq = this->aggregate_c(responses, global);

    auto& j = m_json[infos.name()][name];

    for (std::size_t i = 0; i < infos.nb_samples(); ++i)
    {
      j[infos.samples()[i]] = global[i] / nbq;
    }
  }

  jsonl_formatter_abs::jsonl_formatter_abs(double threshold)
    : jsonl_formatter(threshold)
  {

  }

  void jsonl_formatter_abs::format(const index_infos& infos,
                                  const query_result& response,
                                  std::ostream& os)
  {
    m_os = &os;
    auto jj = json({});
    jj["index"] = infos.name();
    jj["query"] = response.name();
    std::map<std::string, std::uint32_t> samples;

    for (std::size_t i = 0; i < infos.nb_samples(); ++i)
    {
      samples[infos.samples()[i]] = response.counts()[i];
    }
    jj["samples"] = std::move(samples);
    (*m_os) << jj.dump() << "\n";
  }

  void jsonl_formatter_abs::merge_format(const index_infos& infos,
                                        const std::string& name,
                                        const std::vector<query_result>& responses,
                                        std::ostream& os)
  {
    auto jj = json({});
    jj["index"] = infos.name();
    jj["query"] = name;

    std::map<std::string, std::uint32_t> samples;
    std::vector<std::uint32_t> global(infos.nb_samples(), 0);
    std::size_t nbq = this->aggregate_c(responses, global);

    for (std::size_t i = 0; i < infos.nb_samples(); ++i)
    {
      samples[infos.samples()[i]] = global[i] / nbq;
    }
    jj["samples"] = std::move(samples);
    os << jj.dump() << "\n";
  }

  json_wp_formatter_abs::json_wp_formatter_abs(double threshold)
    : json_formatter(threshold)
  {

  }

  void json_wp_formatter_abs::format(const index_infos& infos,
                                  const query_result& response,
                                  std::ostream& os)
  {
    m_os = &os;
    m_json[infos.name()][response.name()] = json({});

    auto& j = m_json[infos.name()][response.name()];
    for (std::size_t i = 0; i < infos.nb_samples(); ++i)
    {
      auto jj = json({});
      jj["C"] = response.counts()[i];
      jj["P"] = response.positions()[i];
      j[infos.samples()[i]] = std::move(jj);
    }
  }

  void json_wp_formatter_abs::merge_format(const index_infos& infos,
                                        const std::string& name,
                                        const std::vector<query_result>& responses,
                                        std::ostream& os)
  {
    m_os = &os;
    m_json[infos.name()][name] = json({});

    std::vector<std::uint32_t> global(infos.nb_samples(), 0);

    std::size_t nbq = this->aggregate_c(responses, global);

    auto& j = m_json[infos.name()][name];

    for (std::size_t i = 0; i < infos.nb_samples(); ++i)
    {
      auto jj = json::array({});
      for (auto& r : responses)
      {
        jj.push_back(r.positions()[i]);
      }

      j[infos.samples()[i]]["C"] = global[i] / nbq;
      j[infos.samples()[i]]["P"] = std::move(jj);
    }
  }

  jsonl_wp_formatter_abs::jsonl_wp_formatter_abs(double threshold)
    : jsonl_formatter(threshold)
  {

  }

  void jsonl_wp_formatter_abs::format(const index_infos& infos,
                                  const query_result& response,
                                  std::ostream& os)
  {
    m_os = &os;
    auto jj = json({});
    jj["index"] = infos.name();
    jj["query"] = response.name();
    std::map<std::string, json> samples;

    for (std::size_t i = 0; i < infos.nb_samples(); ++i)
    {
      auto jjj = json({});
      jjj["C"] = response.counts()[i];
      jjj["P"] = response.positions()[i];
      samples[infos.samples()[i]] = std::move(jjj);
    }
    jj["samples"] = std::move(samples);
    (*m_os) << jj.dump() << "\n";
  }

  void jsonl_wp_formatter_abs::merge_format(const index_infos& infos,
                                    const std::string& name,
                                    const std::vector<query_result>& responses,
                                    std::ostream& os)
  {
    auto jj = json({});
    jj["index"] = infos.name();
    jj["query"] = name;

    std::map<std::string, json> samples;
    std::vector<std::uint32_t> global(infos.nb_samples(), 0);
    std::size_t nbq = this->aggregate_c(responses, global);

    for (std::size_t i = 0; i < infos.nb_samples(); ++i)
    {
      auto jjj = json::array({});
      for (auto& r : responses)
      {
        jjj.push_back(r.positions()[i]);
      }
      json sample;
      sample["C"] = global[i] / nbq;
      sample["P"] = std::move(jjj);
      samples[infos.samples()[i]] = std::move(sample);
    }
    jj["samples"] = std::move(samples);
    os << jj.dump() << "\n";
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
      case format::jsonl:
        return bw == 1 ? std::make_shared<jsonl_formatter>(threshold)
                       : std::make_shared<jsonl_formatter_abs>(threshold);
      case format::jsonl_with_positions:
        if (bw == 1)
          return std::make_shared<jsonl_wp_formatter>(threshold);
        else
          return std::make_shared<jsonl_wp_formatter_abs>(threshold);
      case format::json_with_positions:
        if (bw == 1)
          return std::make_shared<json_wp_formatter>(threshold);
        else
          return std::make_shared<json_wp_formatter_abs>(threshold);

    }

    return nullptr;
  }


}
