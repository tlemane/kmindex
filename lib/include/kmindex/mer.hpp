#ifndef MER_HPP_PYVOSQ3P
#define MER_HPP_PYVOSQ3P

#include <memory>
#include <cstdint>
#include <string_view>

#include <cassert>
#include <limits>
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

  inline bool operator<(const smer& lhs, const smer& rhs)
  {
    return lhs.h < rhs.h;
  }


  inline std::ostream& operator<<(std::ostream& o,  const smer& s)
  {
    o << "smer<" << std::to_string(s.i) << ',' << std::to_string(s.p) << ',' << std::to_string(s.h) << '>';
    return o;
  }


  inline std::uint32_t rev_comp(std::uint64_t data, std::size_t size)
  {
    uint64_t res = data;
    res = ((res >> 2 & 0x3333333333333333) | (res & 0x3333333333333333) <<  2);
    res = ((res >> 4 & 0x0F0F0F0F0F0F0F0F) | (res & 0x0F0F0F0F0F0F0F0F) <<  4);
    res = ((res >> 8 & 0x00FF00FF00FF00FF) | (res & 0x00FF00FF00FF00FF) <<  8);
    res = ((res >>16 & 0x0000FFFF0000FFFF) | (res & 0x0000FFFF0000FFFF) << 16);
    res = ((res >>32 & 0x00000000FFFFFFFF) | (res & 0x00000000FFFFFFFF) << 32);
    res = res ^ 0xAAAAAAAAAAAAAAAA;
    res >>= (2*(32-size));
    return res;
  }

  template<std::size_t MK>
  inline std::uint32_t kmer_minimizer(km::Kmer<MK> k, std::size_t msize)
  {
    return k.minimizer(msize).value();
  }

  template<>
  inline std::uint32_t kmer_minimizer<32>(km::Kmer<32> k, std::size_t msize)
  {
    std::uint64_t kval = k.get64();
    std::size_t ksize = k.m_kmer_size;
    const std::uint32_t def = (1ULL << (2 * msize)) - 1;
    const std::size_t nb_mmers = ksize - msize + 1;
    std::uint32_t minim = std::numeric_limits<std::uint32_t>::max();

    std::size_t shift = (ksize - msize) * 2;
    for (std::size_t i = 0; i < nb_mmers; i++)
    {
      std::uint32_t v = 0;
      std::uint32_t r = 0;

      v = (kval >> shift) & def;
      r = rev_comp(v, msize);

      if (r < v)
        v = r;

      if (km::is_valid_minimizer(v, msize))
      {
        if (v < minim)
          minim = v;
      }
      else
      {
        if (def < minim)
          minim = def;
      }
      shift -= 2;
    }
    return minim;
  }


  template<std::size_t MK>
  class smer_hasher
  {
    using kmer_type = km::Kmer<MK>;
    using repart_type = std::shared_ptr<km::Repartition>;
    using hw_type = std::shared_ptr<km::HashWindow>;

    public:
      smer_hasher() = default;

      smer_hasher(repart_type r, hw_type h, std::size_t minim_size)
        : m_r(r), m_h(h), m_msize(minim_size), m_window(h->get_window_size_bits())
      {
        
      }

      smer operator()(const kmer_type& k, std::uint32_t i) const
      {
        if constexpr (MK == 32)
        {
          return {
            i,
            m_r->get_partition(kmer_minimizer(k, m_msize)),
            XXH64(k.get_data64(), 8, 0) % m_window
          };
        }
        else if constexpr (MK == 64)
        {
          return {
            i,
            m_r->get_partition(kmer_minimizer(k, m_msize)),
            XXH64(k.get_data64(), 16, 0) % m_window
          };
        }
        else
        {
          return {
            i,
            m_r->get_partition(kmer_minimizer(k, m_msize)),
            XXH64(k.get_data64(), kmer_type::m_n_data, 0) % m_window
          };
        }
      }
    private:
      repart_type m_r {nullptr};
      hw_type m_h {nullptr};
      std::size_t m_msize {0};
      std::size_t m_window {0};
  };

  template<std::size_t MK>
  class smer_iterator
  {
    using repart_type = std::shared_ptr<km::Repartition>;
    using hw_type = std::shared_ptr<km::HashWindow>;
    using kmer_type = km::Kmer<MK>;

    public:

      smer_iterator(const std::string_view seq,
                    std::size_t smer_size,
                    const smer_hasher<MK>& hasher)
        : m_seq(seq), m_smer_size(smer_size), m_hash(hasher)
      {
        m_sk.set_polynom(&m_seq[m_current], m_smer_size);
        m_smer = (m_hash)(m_sk.canonical(), m_current);
        m_mask.set_k(smer_size);
        m_mask.set64((1ULL << (smer_size * 2)) - 1);
      }

    private:
      class smer_iterator_sentinel
      {
        public:
          smer_iterator_sentinel(std::size_t n)
            : m_end(n) {}

          bool is_done(const smer_iterator& it) const
          {
            return m_end == it.m_current;
          }

        private:
          std::size_t m_end {0};
      };

    public:

      smer_iterator& operator++()
      {
        ++m_current;
        m_sk = m_sk * 4 + ((m_seq[m_current + m_smer_size - 1] >> 1) & 3);
        m_sk &= m_mask;
        m_smer = (m_hash)(m_sk.canonical(), m_current);
        return *this;
      }

      smer_iterator operator++(int)
      {
        smer_iterator tmp = *this;
        ++(*this);
        return tmp;
      }

      const smer& operator*() const
      {
        return m_smer;
      }

      smer_iterator begin()
      {
        return *this;
      }

      smer_iterator_sentinel end()
      {
        return smer_iterator_sentinel(m_seq.size() - m_smer_size + 1ULL);
      }

      friend bool operator==(const smer_iterator& lhs, const smer_iterator_sentinel& rhs)
      {
        return rhs.is_done(lhs);
      }

      friend bool operator!=(const smer_iterator& lhs, const smer_iterator_sentinel& rhs)
      {
        return !rhs.is_done(lhs);
      }

    private:
      const std::string_view m_seq;
      std::size_t m_smer_size {0};
      std::size_t m_current {0};
      const smer_hasher<MK>& m_hash;
      smer m_smer;
      kmer_type m_sk;
      kmer_type m_mask;
  };

}

#endif /* end of include guard: MER_HPP_PYVOSQ3P */
