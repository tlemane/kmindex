#ifndef INDEX_HPP_W1NZBQZJ
#define INDEX_HPP_W1NZBQZJ

#include <map>
#include <kmindex/index/index_infos.hpp>
#include <kmindex/index/kindex.hpp>

#include <kmindex/exceptions.hpp>

namespace kmq {

  class index
  {
    public:
      index(const std::string& index_path);

      void add_index(const std::string& name, const std::string& km_path);
      void save() const;

      auto begin() { return std::begin(m_indexes); }
      auto end() { return std::end(m_indexes); }

      const index_infos& get(const std::string& name) const
      {
        if (m_indexes.count(name))
          return m_indexes.at(name);
        throw kmq_invalid_index(fmt::format("'{}' is not registered by this instance", name));
      }

    private:
      void init(const std::string& ipath);

    private:
      std::string m_index_path;
      std::map<std::string, index_infos> m_indexes;
  };

}


#endif /* end of include guard: INDEX_HPP_W1NZBQZJ */
