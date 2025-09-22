#ifndef INDEX_INFOS_HPP_IOEYNAWD
#define INDEX_INFOS_HPP_IOEYNAWD

#include <string>
#include <cstdint>
#include <cassert>

#ifndef KMTRICKS_PUBLIC
  #define KMTRICKS_PUBLIC
#endif

#include <array>
#include <kmtricks/hash.hpp>
#include <kmtricks/repartition.hpp>

#include <nlohmann/json.hpp>
#include <semver.hpp>

using json = nlohmann::json;

namespace kmq {

  class index_infos
  {
    private:
      std::string m_name;
      std::string m_path;
      std::size_t m_nb_partitions {0};
      std::size_t m_bloom_size {0};
      std::size_t m_nb_samples {0};
      std::size_t m_smer_size {0};
      std::size_t m_minim_size {0};
      std::size_t m_index_size {0};
      std::size_t m_bw {1};
      std::vector<std::string> m_samples;

      std::string m_sha1;

      std::shared_ptr<km::HashWindow> m_hashw {nullptr};
      std::shared_ptr<km::Repartition> m_repart {nullptr};

      semver::version m_kmver;
      semver::version m_kmtver;

      bool m_is_compressed {false};

    public:

      index_infos() {}
      index_infos(const std::string& name, const std::string& km_dir);
      index_infos(const std::string& name, const json& jdata);


      std::shared_ptr<km::HashWindow> get_hash_w() const;
      std::shared_ptr<km::Repartition> get_repartition() const;
      std::string get_partition(std::size_t partition) const;
      std::string get_directory() const;
      std::string get_sum_partition(std::size_t partition) const;
      std::string get_compression_config() const;
      void use_fof(const std::string& fof_path);
      bool is_compressed_index() const;
      void set_compress(bool v = true) { m_is_compressed = v; }

      bool has_uncompressed_partitions() const;
      bool has_compressed_partitions() const;
      
      bool has_sum_index() const { return fs::exists(get_sum_partition(0)); }

      std::string name() const;
      std::size_t bloom_size() const;
      std::size_t nb_partitions() const;
      std::size_t nb_samples() const;
      std::size_t smer_size() const;
      std::size_t minim_size() const;
      std::size_t index_size() const;
      std::size_t bw() const;

      std::string path() const;
      std::string sha1() const;

      const std::vector<std::string>& samples() const;

      std::size_t rename_samples(const std::string& format, std::size_t start_id);
      std::size_t rename_samples(const std::string& new_ids);

      std::string fof_str() const;

      const semver::version& km_version() const;
      const semver::version& kmt_version() const;

    private:
      void init();
      void init(const json& data);
      void is_km_index() const;
      std::string km_sha1() const;
  };

}

#endif /* end of include guard: INDEX_INFOS_HPP_IOEYNAWD */
