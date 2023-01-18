#include <kmindex/query/format.hpp>
#include <kmindex/utils.hpp>

namespace kmq
{

  enum format str_to_format(const std::string& f)
  {
    if (f == "matrix")
      return format::matrix;
    return format::json;
  }

  std::size_t query_formatter_base::aggregate(const query_result_agg& queries,
                                              std::vector<std::uint32_t>& global)
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

  std::size_t query_formatter_base::aggregate_c(const query_result_agg& queries,
                                                std::vector<std::uint32_t>& global)
  {
    std::size_t nbk = 0;

    for (auto& qr : queries)
    {
      nbk += qr.nbk();

      std::transform(global.begin(),
                     global.end(),
                     qr.counts().begin(),
                     global.begin(),
                     [](auto& lhs, auto& rhs)
                     { return std::min(lhs, rhs); });
    }

    return nbk;
  }

  void matrix_formatter::write_headers(std::stringstream& ss, const std::vector<std::string>& sample_ids)
  {
    ss << "ID\t";

    for (auto& s : sample_ids)
    {
      ss << s << '\t';
    }

    ss.seekp(-1, ss.cur);
    ss << '\n';
  }

  void matrix_formatter::write_one(std::stringstream& ss, const std::string& name, const std::vector<double>& ratios)
  {
    ss << name << '\t';

    for (auto& r : ratios)
    {
      ss << r << '\t';
    }

    ss.seekp(-1, ss.cur);
    ss << '\n';
  }

  std::string matrix_formatter::format(const std::string& index_name,
                                       const std::vector<std::string>& sample_ids,
                                       const query_result_agg& queries)
  {
    unused(index_name);
    std::stringstream ss;
    ss << std::setprecision(2);
    write_headers(ss, sample_ids);

    for (auto& qr : queries)
    {
      write_one(ss, qr.name(), qr.ratios());
    }

    return ss.str();
  }

  std::string matrix_formatter::merge_format(const std::string& index_name,
                                             const std::vector<std::string>& sample_ids,
                                             const query_result_agg& queries,
                                             const std::string& qname)
  {
    unused(index_name);
    std::stringstream ss;
    ss << std::setprecision(2);
    write_headers(ss, sample_ids);

    std::vector<std::uint32_t> global(sample_ids.size(), 0);
    std::size_t nbk = this->aggregate(queries, global);

    ss << qname << '\t';

    for (auto& c : global)
    {
      ss << (c / static_cast<double>(nbk)) << '\t';
    }

    ss.seekp(-1, ss.cur);
    ss << '\n';

    return ss.str();
  }

  void matrix_formatter_abs::write_one(std::stringstream& ss,
                                       const std::string& name,
                                       const std::vector<std::uint32_t>& counts)
  {
    ss << name << '\t';

    for (auto& r : counts)
    {
      ss << r << '\t';
    }

    ss.seekp(-1, ss.cur);
    ss << '\n';
  }

  std::string matrix_formatter_abs::format(const std::string& index_name,
                                           const std::vector<std::string>& sample_ids,
                                           const query_result_agg& queries)
  {
    unused(index_name);
    std::stringstream ss;
    ss << std::setprecision(2);
    write_headers(ss, sample_ids);

    for (auto& qr : queries)
    {
      write_one(ss, qr.name(), qr.counts());
    }

    return ss.str();
  }

  std::string matrix_formatter_abs::merge_format(const std::string& index_name,
                                                 const std::vector<std::string>& sample_ids,
                                                 const query_result_agg& queries,
                                                 const std::string& qname)
  {
    unused(index_name);
    std::stringstream ss;
    ss << std::setprecision(2);
    write_headers(ss, sample_ids);

    std::vector<std::uint32_t> global(sample_ids.size(), std::numeric_limits<std::uint32_t>::max());

    std::size_t nbk = this->aggregate_c(queries, global);
    unused(nbk);

    ss << qname << '\t';

    for (auto& c : global)
    {
      ss << (c / nbk) << '\t';
    }

    ss.seekp(-1, ss.cur);
    ss << '\n';

    return ss.str();
  }

  void json_formatter::write_one(json& data,
                                 const std::string& name,
                                 const std::vector<double>& ratios,
                                 const std::vector<std::string>& sample_ids)
  {
    data[name] = json({});

    for (std::size_t i = 0; i < sample_ids.size(); ++i)
    {
      data[name][sample_ids[i]] = ratios[i];
    }
  }

  std::string json_formatter::format(const std::string& index_name,
                                     const std::vector<std::string>& sample_ids,
                                     const query_result_agg& queries)
  {
    return jformat(index_name, sample_ids, queries).dump(4);
  }

  std::string json_formatter::merge_format(const std::string& index_name,
                                           const std::vector<std::string>& sample_ids,
                                           const query_result_agg& queries,
                                           const std::string& qname)
  {
    return jmerge_format(index_name, sample_ids, queries, qname).dump(4);
  }

  json json_formatter::jformat(const std::string& index_name,
                               const std::vector<std::string>& sample_ids,
                               const query_result_agg& queries)
  {
    json data;
    data[index_name] = json({});

    for (auto& qr : queries)
    {
      write_one(data[index_name], qr.name(), qr.ratios(), sample_ids);
    }

    return data;
  }

  json json_formatter::jmerge_format(const std::string& index_name,
                                     const std::vector<std::string>& sample_ids,
                                     const query_result_agg& queries,
                                     const std::string& qname)
  {
    std::vector<std::uint32_t> global(sample_ids.size(), 0);
    std::size_t nbk = this->aggregate(queries, global);

    json data;
    data[index_name] = json({});
    data[index_name][qname] = json({});

    for (std::size_t i = 0; i < sample_ids.size(); ++i)
    {
      data[index_name][qname][sample_ids[i]] = global[i] / static_cast<double>(nbk);
    }

    return data;
  }

  void json_formatter_abs::write_one(json& data,
                                     const std::string& name,
                                     const std::vector<std::uint32_t>& counts,
                                     const std::vector<std::string>& sample_ids)
  {
    data[name] = json({});

    for (std::size_t i = 0; i < sample_ids.size(); ++i)
    {
      data[name][sample_ids[i]] = counts[i];
    }
  }

  std::string json_formatter_abs::format(const std::string& index_name,
                                         const std::vector<std::string>& sample_ids,
                                         const query_result_agg& queries)
  {
    return jformat(index_name, sample_ids, queries).dump(4);
  }

  std::string json_formatter_abs::merge_format(const std::string& index_name,
                                               const std::vector<std::string>& sample_ids,
                                               const query_result_agg& queries,
                                               const std::string& qname)
  {
    return jmerge_format(index_name, sample_ids, queries, qname).dump(4);
  }

  json json_formatter_abs::jformat(const std::string& index_name,
                                   const std::vector<std::string>& sample_ids,
                                   const query_result_agg& queries)
  {
    json data;
    data[index_name] = json({});

    for (auto& qr : queries)
    {
      write_one(data[index_name], qr.name(), qr.counts(), sample_ids);
    }

    return data;
  }

  json json_formatter_abs::jmerge_format(const std::string& index_name,
                                         const std::vector<std::string>& sample_ids,
                                         const query_result_agg& queries,
                                         const std::string& qname)
  {
    std::vector<std::uint32_t> global(sample_ids.size(), std::numeric_limits<std::uint32_t>::max());
    std::size_t nbk = this->aggregate_c(queries, global);

    json data;
    data[index_name] = json({});
    data[index_name][qname] = json({});

    for (std::size_t i = 0; i < sample_ids.size(); ++i)
    {
      data[index_name][qname][sample_ids[i]] = global[i] / nbk;
    }

    return data;
  }

  query_formatter_t get_formatter(enum format f, std::size_t w)
  {
    switch (f)
    {
      case format::matrix:
        return w == 1 ? std::make_shared<matrix_formatter>() : std::make_shared<matrix_formatter_abs>();
      case format::json:
        return w == 1 ? std::make_shared<json_formatter>() : std::make_shared<json_formatter_abs>();
    }

    return nullptr;
  }

  void write_result(const std::string& res,
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
}  // namespace kmq
