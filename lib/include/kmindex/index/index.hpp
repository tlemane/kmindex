#ifndef INDEX_HPP_W1NZBQZJ
#define INDEX_HPP_W1NZBQZJ

#include <map>
#include <kmindex/index/index_infos.hpp>
#include <kmindex/index/kindex.hpp>

#include <semver.hpp>

namespace kmq {

  constexpr semver::version min_kmv_required{1, 3, 0};

  class index
  {
    using map_t = std::map<std::string, index_infos>;
    using iterator = map_t::iterator;

    public:
      index(const std::string& index_path);

      void add_index(const std::string& name, const std::string& km_path, register_mode rm = register_mode::symlink);
      void remove_index(const std::string& name);
      bool has_index(const std::string& name);

      void save() const;

      iterator begin();
      iterator end();

      const index_infos& get(const std::string& name) const;
      index_infos& get(const std::string& name);
      bool has(const std::string& name) const;

      std::vector<std::string> all() const;

      std::vector<std::vector<std::string>> mergeable() const;

      const std::string& path() const { return m_index_path; }
    private:
      void init(const std::string& ipath);

    private:
      std::string m_index_path;
      map_t m_indexes;
  };

}


#endif /* end of include guard: INDEX_HPP_W1NZBQZJ */
