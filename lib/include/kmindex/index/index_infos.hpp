#ifndef INDEX_INFOS_HPP_IOEYNAWD
#define INDEX_INFOS_HPP_IOEYNAWD

#include <string>
#include <cstdint>
#include <cassert>

#include <kmtricks/hash.hpp>
#include <kmtricks/repartition.hpp>

#include <nlohmann/json.hpp>

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
      std::vector<std::string> m_samples;

      std::shared_ptr<km::HashWindow> m_hashw {nullptr};
      std::shared_ptr<km::Repartition> m_repart {nullptr};

    public:

      index_infos() {}
      index_infos(const std::string& name, const std::string& km_dir);
      index_infos(const std::string& name, const json& jdata);


      std::shared_ptr<km::HashWindow> get_hash_w() const;
      std::shared_ptr<km::Repartition> get_repartition() const;
      std::string get_partition(std::size_t partition) const;

      std::string name() const;
      std::size_t bloom_size() const;
      std::size_t nb_partitions() const;
      std::size_t nb_samples() const;
      std::size_t smer_size() const;
      std::size_t minim_size() const;
      std::size_t index_size() const;
      std::string path() const;
      const std::vector<std::string>& samples() const;

    private:
      void init();
      void init(const json& data);
      void is_km_index() const;
  };

}

#endif /* end of include guard: INDEX_INFOS_HPP_IOEYNAWD */
