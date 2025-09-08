#ifndef COMMON_HPP_1757164505
#define COMMON_HPP_1757164505

#include <kmindex/mer.hpp>
#include <kmtricks/hash.hpp>
#include <kmtricks/repartition.hpp>
#include <kmtricks/kmer.hpp>
#define WITH_XXHASH
#include <kmtricks/kmer_hash.hpp>

namespace kmq {

  template<std::size_t MK>
  struct smer_functor
  {
    using qsmer_type = std::pair<smer, std::uint32_t>;
    using qpart_type = std::vector<qsmer_type>;
    using repart_type = std::shared_ptr<km::Repartition>;
    using hw_type = std::shared_ptr<km::HashWindow>;

    void operator()(std::vector<qpart_type>& smers,
                    const std::string& seq,
                    std::uint32_t qid,
                    std::size_t smer_size,
                    repart_type& repart,
                    hw_type& hw,
                    std::size_t msize)
    {
      smer_hasher<MK> sh(repart, hw, msize);

      for (auto& mer : smer_iterator<MK>(seq, smer_size, sh))
      {
        smers[mer.p].emplace_back(mer, qid);
      }
    }
  };

}

#endif /* end of include guard: COMMON_HPP_1757164505 */