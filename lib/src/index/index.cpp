#include <map>
#include <unistd.h>
#include <kmindex/index/index.hpp>
#include <kmindex/index/index_infos.hpp>
#include <kmindex/exceptions.hpp>
#include <kmindex/utils.hpp>

#include <nlohmann/json.hpp>
#include <simdjson.h>
using json = nlohmann::json;
namespace kmq {

  index::index(const std::string& index_path)
    : m_index_path(fs::absolute(index_path).string())
  {
    if (fs::exists(index_path))
      init(index_path);
    else
      fs::create_directory(index_path);
  }

  void index::init(const std::string& index_path)
  {
    using namespace simdjson;

    // std::ifstream inf(fmt::format("{}/index.json", m_index_path), std::ios::in);
    // check_fstream_good(index_path, inf);
    // json data = json::parse(inf);
    //
    // for (auto& e : data["index"].items())
    // {
    //   m_indexes[e.key()] = index_infos(e.key(), data);
    // }
    //
    padded_string json =
        padded_string::load(fmt::format("{}/index.json", m_index_path));

    ondemand::parser parser;
    ondemand::document data = parser.iterate(json);

    ondemand::object index_obj = data["index"];

    for (ondemand::field e : index_obj)
    {
        std::string key{ e.unescaped_key().value() };
        m_indexes[key] = index_infos(key, e.value(), fs::absolute(m_index_path).string());
    }
  }

  void index::add_index(const std::string& name, const std::string& km_path, register_mode rm)
  {
    if (m_indexes.count(name))
      throw std::runtime_error(fmt::format("'{}' already exists.", name));
    m_indexes[name] = index_infos(name, fs::absolute(km_path).string(), rm);
  }

  bool index::has_index(const std::string& name)
  {
    return m_indexes.count(name) != 0;
  }

  void index::remove_index(const std::string& name)
  {
    if (m_indexes.count(name))
    {
      m_indexes.erase(name);
      std::string p = fmt::format("{}/{}", m_index_path, name);
      ::unlink(p.c_str());
    }
  }

  void index::save() const
  {
    nlohmann::json data;
    data["path"] = m_index_path;
    data["index"] = json({});

    for (auto& [name, i] : m_indexes)
    {
      if (!fs::exists(fmt::format("{}/{}", m_index_path, name)))
      {
        switch (i.rmode()) {
          case register_mode::symlink: {
            fs::create_directory_symlink(i.path(), fmt::format("{}/{}", m_index_path, name));
            break;
          }
          case register_mode::copy: {
            auto target = fmt::format("{}/{}", m_index_path, name);
            fs::create_directories(target);
            std::error_code ec;
            fs::copy(i.path(), target, fs::copy_options::recursive, ec);
            if (ec)
            {
              throw kmq_io_error(fmt::format("Registering (copy) failed for {}", name));
            }
            break;
          }
          case register_mode::move_or_copy:
          case register_mode::move: {
            auto target = fmt::format("{}/{}", m_index_path, name);
            std::error_code ec;
            fs::rename(i.path(), target, ec);
            if (ec)
            {
              if (i.rmode() == register_mode::move)
                throw kmq_io_error(fmt::format("Registering (move) failed for {}. Try 'move_or_copy' mode.", name));

              fs::create_directories(target);
              fs::copy(i.path(), target, fs::copy_options::recursive, ec);
              if (ec)
              {
                throw kmq_io_error(fmt::format("Registering (move_or_copy) failed for {}.", name));
              }
              fs::remove_all(i.path());
            }
            break;
          }
          default: break;
        }
      }
      data["index"][name]["nb_samples"] = i.nb_samples();
      data["index"][name]["index_size"] = i.index_size();
      data["index"][name]["bloom_size"] = i.bloom_size();
      data["index"][name]["nb_partitions"] = i.nb_partitions();
      data["index"][name]["smer_size"]  = i.smer_size();
      data["index"][name]["minim_size"] = i.minim_size();
      data["index"][name]["samples"] = i.samples();
      data["index"][name]["sha1"] = i.sha1();
      data["index"][name]["bw"] = i.bw();
      data["index"][name]["kmindex_version"] = i.km_version().to_string();
      data["index"][name]["kmtricks_version"] = i.kmt_version().to_string();
    }

    std::ofstream out(fmt::format("{}/index.json", m_index_path), std::ios::out);

    out << std::setw(4) << data << std::endl;
  }

  std::vector<std::string> index::all() const
  {
    std::vector<std::string> names;
    for (const auto& i : m_indexes)
      names.push_back(i.first);

    return names;
  }

  index::iterator index::begin()
  {
    return std::begin(m_indexes);
  }

  index::iterator index::end()
  {
    return std::end(m_indexes);
  }

  const index_infos& index::get(const std::string& name) const
  {
    if (m_indexes.count(name))
      return m_indexes.at(name);
    throw kmq_invalid_index(fmt::format("'{}' is not registered by this instance", name));
  }

  index_infos& index::get(const std::string& name)
  {
    if (m_indexes.count(name))
      return m_indexes.at(name);
    throw kmq_invalid_index(fmt::format("'{}' is not registered by this instance", name));
  }

  bool index::has(const std::string& name) const
  {
    return m_indexes.count(name) > 0;
  }


  std::vector<std::vector<std::string>> index::mergeable() const
  {
    std::vector<std::vector<std::string>> vec;

    std::map<std::string, std::vector<std::string>> m;

    for (auto& [name, i] : m_indexes)
      m[i.sha1()].push_back(name);

    for (auto& [_, v] : m)
    {
      if (vec.size() > 1)
        vec.push_back(std::move(v));
    }

    return vec;
  }

}
