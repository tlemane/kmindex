#ifndef MER_HPP_PYVOSQ3P
#define MER_HPP_PYVOSQ3P

#include <memory>
#include <cstdint>
#include <string_view>

#include <cassert>
#include <kmtricks/repartition.hpp>
#include <kmtricks/hash.hpp>
#include <kmtricks/kmer.hpp>

#include <xxhash.h>

namespace kmq {

  struct smer
  {
    smer(std::uint32_t i = 0, std::uint32_t p = 0, std::uint64_t h = 0)
      : i(i), p(p), h(h) {}

    std::uint32_t i {0};
    std::uint32_t p {0};
    std::uint64_t h {0};
  };


  inline std::ostream& operator<<(std::ostream& o,  const smer& s)
  {
    o << "smer<" << std::to_string(s.i) << ',' << std::to_string(s.p) << ',' << std::to_string(s.h) << '>';
    return o;
  }

  class smer_hasher
  {
    using kmer_type = km::Kmer<32>;
    using repart_type = std::shared_ptr<km::Repartition>;
    using hw_type = std::shared_ptr<km::HashWindow>;

    public:
      smer_hasher() {}
      smer_hasher(repart_type r, hw_type h, std::size_t minim_size) : m_r(r), m_h(h), m_msize(minim_size) {}

      smer operator()(const kmer_type& k, std::uint32_t i) const
      {
        kmer_type cano = k.canonical();

        return {
          i,
          m_r->get_partition(cano.minimizer(m_msize).value()),
          XXH64(cano.get_data64(), 8, 0) % m_h->get_window_size_bits()
        };
      }
    private:
      repart_type m_r {nullptr};
      hw_type m_h {nullptr};
      std::size_t m_msize {0};
  };

  class smer_iterator
  {
    using repart_type = std::shared_ptr<km::Repartition>;
    using hw_type = std::shared_ptr<km::HashWindow>;
    using kmer_type = km::Kmer<32>;

    public:

      smer_iterator(const std::string_view seq,
                    std::size_t smer_size,
                    const smer_hasher* hasher)
        : m_seq(seq), m_smer_size(smer_size), m_hash(hasher)
      {
        kmer_type sk(m_smer_size);
        sk.set_polynom(&m_seq[m_current], m_smer_size);
        m_smer = (*m_hash)(sk, m_current);
      }

    private:
      smer_iterator(std::size_t seqn) : m_current(seqn) {}

    public:

      smer_iterator& operator++()
      {
        ++m_current;
        kmer_type sk(m_smer_size);
        sk.set_polynom(&m_seq[m_current], m_smer_size);
        m_smer = (*m_hash)(sk, m_current);

        return *this;
      }

      smer_iterator operator++(int)
      {
        smer_iterator tmp = *this; ++(*this); return tmp;
      }

      auto& operator*() const { return m_smer; }
      smer_iterator begin() { return *this; }
      smer_iterator end() { return smer_iterator(m_seq.size() - m_smer_size + 1ULL); }

      friend bool operator==(const smer_iterator& lhs, const smer_iterator& rhs)
      { return lhs.m_current == rhs.m_current; }

      friend bool operator!=(const smer_iterator& lhs, const smer_iterator& rhs)
      { return lhs.m_current != rhs.m_current; }

    private:
      const std::string_view m_seq;
      std::size_t m_smer_size {0};
      std::int64_t m_current {0};
      const smer_hasher* m_hash;

      smer m_smer;
  };

}


#endif /* end of include guard: MER_HPP_PYVOSQ3P */
