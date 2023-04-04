#ifndef MERGE_HPP_BRXIFGDK
#define MERGE_HPP_BRXIFGDK

#include <kmindex/index/index.hpp>
#include <kmindex/threadpool.hpp>
#include <string>
#include <vector>
#include <memory>

namespace kmq {

  enum class rename_mode : std::uint8_t
  {
    file,
    format,
    none
  };

  class index_merger
  {
    protected:
      index_merger(index* gindex,
                   const std::vector<std::string>& to_merge,
                   const std::string& new_path,
                   const std::string& new_name,
                   rename_mode mode,
                   std::size_t bw,
                   bool rm);

      virtual ~index_merger() = default;

      void remove_old() const;
      void remove_old(std::size_t p) const;

      virtual void merge_one(std::size_t p) const = 0;

      void copy_tree(const std::string& path) const;
      void copy_trees() const;

      void zero(std::size_t p) const;

    public:
      void rename(const std::string& rename_string) const;
      virtual void merge(ThreadPool& pool) const = 0;
    protected:
      index* m_index;
      std::vector<std::string> m_to_merge;
      std::string m_new_path;
      std::string m_new_name;
      rename_mode m_rename_mode {rename_mode::none};
      std::size_t m_bw {0};
      bool m_rm {false};    

      std::size_t m_psize {0};
      std::size_t m_nb_samples {0};
      std::size_t m_nb_parts {0};
      mutable std::vector<std::vector<std::uint8_t>> m_buffer;
  };

  using index_merger_t = std::shared_ptr<index_merger>;

  class index_merger_pa : public index_merger
  {
    public:
      index_merger_pa(index* gindex,
                     const std::vector<std::string>& to_merge,
                     const std::string& new_path,
                     const std::string& new_name,
                     rename_mode mode,
                     bool rm);

      void merge_one(std::size_t p) const override final;
      void merge(ThreadPool& pool) const override final;
  };

  index_merger_t make_merger(index* gindex,
                             const std::vector<std::string>& to_merge,
                             const std::string& new_path,
                             const std::string& new_name,
                             rename_mode mode,
                             bool rm,
                             bool pa);



}


#endif /* end of include guard: MERGE_HPP_BRXIFGDK */
