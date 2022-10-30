#include <fstream>
#include <filesystem>
#include <algorithm>

#include <kmindex/exceptions.hpp>
#include <kmindex/index/index_infos.hpp>
#include <kmindex/utils.hpp>
#include <fmt/format.h>
#include <spdlog/spdlog.h>

namespace kmq {

  index_infos::index_infos(const std::string& name, const std::string& km_dir)
    : m_name(name), m_path(km_dir)
  {
    init();
  }

  index_infos::index_infos(const std::string& name, const json& jdata)
    : m_name(name)
  {
    init(jdata);
  }

  std::shared_ptr<km::HashWindow> index_infos::get_hash_w() const
  {
    return m_hashw;
  }

  std::shared_ptr<km::Repartition> index_infos::get_repartition() const
  {
    return m_repart;
  }

  std::string index_infos::get_partition(std::size_t partition) const
  {
    return fmt::format("{}/matrices/matrix_{}.cmbf", m_path, partition);
  }

  std::string index_infos::name() const
  {
    return m_name;
  }

  std::size_t index_infos::bloom_size() const
  {
    return m_bloom_size;
  }

  std::size_t index_infos::nb_partitions() const
  {
    return m_nb_partitions;
  }

  std::size_t index_infos::nb_samples() const
  {
    return m_nb_samples;
  }

  std::size_t index_infos::smer_size() const
  {
    return m_smer_size;
  }

  std::size_t index_infos::minim_size() const
  {
    return m_minim_size;
  }

  std::size_t index_infos::index_size() const
  {
    return m_index_size;
  }

  std::size_t index_infos::bw() const
  {
    return m_bw;
  }

  std::string index_infos::path() const
  {
    return m_path;
  }

  const std::vector<std::string>& index_infos::samples() const
  {
    return m_samples;
  }

  void index_infos::is_km_index() const
  {
    std::string p = fmt::format("{}/matrices/matrix_0.cmbf", m_path);

    if (!fs::exists(p))
      throw kmq_io_error(fmt::format("{} is not a kmtricks index.", m_path));
  }

  void index_infos::init()
  {
    is_km_index();

    m_hashw = std::make_shared<km::HashWindow>(fmt::format("{}/hash.info", m_path));
    auto hw = get_hash_w();

    m_bloom_size = hw->bloom_size();

    auto d = fs::directory_iterator(fmt::format("{}/matrices", m_path));
    m_nb_partitions = [&](){
      std::size_t c = 0;
      for (auto& f : d)
      {
        if (f.is_regular_file())
          ++c;
      }
      return c;
    }();

    std::ifstream in_opt(fmt::format("{}/options.txt", m_path));

    std::string line; std::getline(in_opt, line);

    auto v = split(line, ',');
    for (auto& e : v)
    {
      auto ss = split(e, '=');
      if (trim(ss[0]) == "kmer_size")
        m_smer_size = std::stoull(trim(ss[1]));
      else if (trim(ss[0]) == "minim_size")
        m_minim_size = std::stoull(trim(ss[1]));
      else if (trim(ss[0]) == "bwidth")
        m_bw = std::stoull(trim(ss[1]));
    }

    std::string kmfof = fmt::format("{}/kmtricks.fof", m_path);

    std::ifstream in(kmfof, std::ios::in);
    check_fstream_good(kmfof, in);

    for (std::string line; std::getline(in, line);)
    {
      if (!line.empty())
      {
        m_nb_samples++;
        m_samples.push_back(trim(split(line, ':')[0]));
      }
    }

    m_index_size = directory_size(fmt::format("{}/matrices", m_path));

  }

  void index_infos::init(const json& data)
  {
    m_path = fs::read_symlink(fmt::format("{}/{}", data["path"], m_name)).string();
    m_nb_partitions = data["index"][m_name]["nb_partitions"];
    m_bloom_size  = data["index"][m_name]["bloom_size"];
    m_nb_samples  = data["index"][m_name]["nb_samples"];
    m_smer_size   = data["index"][m_name]["smer_size"];
    m_minim_size  = data["index"][m_name]["minim_size"];
    m_index_size  = data["index"][m_name]["index_size"];
    m_samples     = data["index"][m_name]["samples"];
    m_bw          = data["index"][m_name]["bw"];

    m_hashw = std::make_shared<km::HashWindow>(fmt::format("{}/hash.info", m_path));
    m_repart = std::make_shared<km::Repartition>(fmt::format("{}/repartition_gatb/repartition.minimRepart", m_path));
  }

}

