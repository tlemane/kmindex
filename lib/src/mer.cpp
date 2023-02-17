#include <limits>
#include <kmindex/mer.hpp>

#include <iostream>

namespace kmq {

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

  inline std::uint32_t kmer_minimizer(std::uint64_t kval, std::size_t ksize, std::size_t msize)
  {
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


  smer_hasher::smer_hasher(repart_type r, hw_type h, std::size_t minim_size)
    : m_r(r), m_h(h), m_msize(minim_size)
  {

  }

  smer smer_hasher::operator()(const kmer_type& k, std::uint32_t i) const
  {
    return {
      i,
      m_r->get_partition(kmer_minimizer(k.get64(), kmer_type::m_kmer_size, m_msize)),
      XXH64(k.get_data64(), 8, 0) % m_h->get_window_size_bits()
    };
  }

  smer_iterator::smer_iterator(const std::string_view seq,
                               std::size_t smer_size,
                               const smer_hasher* hasher)
    : m_seq(seq), m_smer_size(smer_size), m_hash(hasher)
  {
    m_sk.set_polynom(&m_seq[m_current], m_smer_size);
    m_smer = (*m_hash)(m_sk.canonical(), m_current);
    m_mask.set_k(smer_size);
    m_mask.set64((1ULL << (smer_size * 2)) - 1);
  }

  smer_iterator& smer_iterator::operator++()
  {
    ++m_current;
    //m_sk = m_sk * 4 + km::NToB[static_cast<unsigned char>(m_seq[m_current + m_smer_size - 1])];
    m_sk = m_sk * 4 + ((m_seq[m_current + m_smer_size - 1] >> 1) & 3);
    m_sk &= m_mask;
    m_smer = (*m_hash)(m_sk.canonical(), m_current);

    return *this;
  }

  smer_iterator smer_iterator::operator++(int)
  {
    smer_iterator tmp = *this; ++(*this); return tmp;
  }

  const smer& smer_iterator::operator*() const
  {
    return m_smer;
  }

  smer_iterator smer_iterator::begin()
  {
    return *this;
  }

  smer_iterator::smer_iterator_sentinel smer_iterator::end()
  {
    return smer_iterator_sentinel(m_seq.size() - m_smer_size + 1ULL);
  }

}

