#include <fstream>
#include <filesystem>
#include <algorithm>

#include <kmindex/exceptions.hpp>
#include <kmindex/index/index_infos.hpp>
#include <kmindex/utils.hpp>
#include <kmindex/version.hpp>

#include <fmt/format.h>
#include <sha1.hpp>

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

  std::string index_infos::sha1() const
  {
    return m_sha1;
  }

  const std::vector<std::string>& index_infos::samples() const
  {
    return m_samples;
  }

  const semver::version& index_infos::km_version() const
  {
    return m_kmver;
  }

  const semver::version& index_infos::kmt_version() const
  {
    return m_kmtver;
  }

  std::size_t index_infos::rename_samples(const std::string& format, std::size_t start_id)
  {
    std::stringstream ss;
    {
      std::ifstream fof(fmt::format("{}/kmtricks.fof", path()), std::ios::in);

      for (std::string line; std::getline(fof, line);)
      {
        ss << fmt::format(format, start_id);
        ss << ": " << trim(split(line, ':')[1]) << '\n';
        ++start_id;
      }
    }

    {
      std::ofstream fof(fmt::format("{}/kmtricks.fof", path()), std::ios::out);
      fof << ss.str();
    }

    init();

    return start_id;
  }

  std::size_t index_infos::rename_samples(const std::string& new_ids)
  {
    std::stringstream ss;
    {
      std::ifstream fof(fmt::format("{}/kmtricks.fof", path()), std::ios::in);
      std::ifstream newifs(new_ids, std::ios::in);

      for (std::string line, nid; std::getline(fof, line), std::getline(newifs, nid);)
      {
        ss << nid;
        ss << ": " << trim(split(line, ':')[1]) << '\n';
      }
    }

    {
      std::ofstream fof(fmt::format("{}/kmtricks.fof", path()), std::ios::out);
      fof << ss.str();
    }

    init();

    return 0;
  }

  std::string index_infos::fof_str() const
  {
    std::stringstream ss;
    std::ifstream in(fmt::format("{}/kmtricks.fof", path()), std::ios::in);

    for (std::string line; std::getline(in, line);)
    {
      if (trim(line).empty())
        continue;
      ss << line << '\n';
    }
    return ss.str();
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
      {
        m_smer_size = std::stoull(trim(ss[1]));
      }
      else if (trim(ss[0]) == "minim_size")
      {
        m_minim_size = std::stoull(trim(ss[1]));
      }
      else if (trim(ss[0]) == "bwidth")
      {
        m_bw = std::stoull(trim(ss[1]));
      }
      else if (trim(ss[0]) == "mode")
      {
        if (trim(ss[1]) == "bf")
          m_bw = 1;
      }
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

    m_sha1 = km_sha1();

    std::string kmb_info_path = fmt::format("{}/build_infos.txt", m_path);
    std::ifstream kmb_info(kmb_info_path, std::ios::in);
    check_fstream_good(kmb_info_path, kmb_info);

    std::string kmt_ver_str; std::getline(kmb_info, kmt_ver_str);

    m_kmtver = semver::version(trim(kmt_ver_str.substr(10)));
    m_kmver = kmindex_version;
  }

  std::string index_infos::km_sha1() const
  {
    char digest[SHA1_HEX_SIZE];

    kmq_sha1::sha1 h;

    h.add(&m_bloom_size, sizeof(m_bloom_size));
    h.add(&m_nb_partitions, sizeof(m_nb_partitions));
    h.add(&m_smer_size, sizeof(m_smer_size));
    h.add(&m_minim_size, sizeof(m_minim_size));
    h.add(&m_bw, sizeof(m_bw));

    std::ifstream repart_file(fmt::format("{}/repartition_gatb/repartition.minimRepart", m_path));

    std::uint16_t _1, _2;
    std::uint64_t _3;
    repart_file.read(reinterpret_cast<char*>(&_1), sizeof(_1));
    repart_file.read(reinterpret_cast<char*>(&_3), sizeof(_3));
    repart_file.read(reinterpret_cast<char*>(&_2), sizeof(_2));

    std::vector<std::uint16_t> buffer(_3);
    repart_file.read(reinterpret_cast<char*>(buffer.data()), sizeof(_3) * _3);

    h.add(buffer.data(), sizeof(std::uint16_t) * _3);
    h.finalize();
    h.print_hex(digest);

    return digest;
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
    m_sha1        = data["index"][m_name]["sha1"];

    m_kmver = semver::version(data["index"][m_name]["kmindex_version"].get_ref<const json::string_t&>());
    m_kmtver = semver::version(data["index"][m_name]["kmtricks_version"].get_ref<const json::string_t&>());

    m_hashw = std::make_shared<km::HashWindow>(fmt::format("{}/hash.info", m_path));
    m_repart = std::make_shared<km::Repartition>(fmt::format("{}/repartition_gatb/repartition.minimRepart", m_path));
  }

}

