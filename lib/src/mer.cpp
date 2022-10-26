#include <kmindex/mer.hpp>

namespace kmq {

  smer_hasher::smer_hasher(repart_type r, hw_type h, std::size_t minim_size)
    : m_r(r), m_h(h), m_msize(minim_size)
  {

  }

  smer smer_hasher::operator()(const kmer_type& k, std::uint32_t i) const
  {
    kmer_type cano = k.canonical();

    return {
      i,
      m_r->get_partition(cano.minimizer(m_msize).value()),
      XXH64(cano.get_data64(), 8, 0) % m_h->get_window_size_bits()
    };
  }

  smer_iterator::smer_iterator(const std::string_view seq,
                               std::size_t smer_size,
                               const smer_hasher* hasher)
    : m_seq(seq), m_smer_size(smer_size), m_hash(hasher)
  {
    kmer_type sk(m_smer_size);
    sk.set_polynom(&m_seq[m_current], m_smer_size);
    m_smer = (*m_hash)(sk, m_current);
  }

  smer_iterator::smer_iterator(std::size_t seqn)
    : m_current(seqn)
  {

  }

  smer_iterator& smer_iterator::operator++()
  {
    ++m_current;
    kmer_type sk(m_smer_size);
    sk.set_polynom(&m_seq[m_current], m_smer_size);
    m_smer = (*m_hash)(sk, m_current);

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

  smer_iterator smer_iterator::end()
  {
    return smer_iterator(m_seq.size() - m_smer_size + 1ULL);
  }

}