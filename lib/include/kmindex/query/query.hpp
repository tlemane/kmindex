#ifndef QUERY_HPP_HIJEZMAU
#define QUERY_HPP_HIJEZMAU

#include <cstdint>
#include <atomic>
#include <string_view>
#include <cassert>

#include <kmtricks/hash.hpp>
#include <kmtricks/repartition.hpp>
#include <kmtricks/kmer.hpp>
#define WITH_XXHASH
#include <kmtricks/kmer_hash.hpp>

#include <kmindex/utils.hpp>
#include <kmindex/mer.hpp>

namespace kmq {

  class query_response
  {
    public:
      query_response(std::size_t n, std::size_t nbits, std::size_t width = 1)
        : m_block_size(((nbits * width) + 7) / 8)
      {
        m_responses.resize(n * m_block_size, 0);
      }

      std::size_t block_size() const { return m_block_size; }

      std::uint8_t* get(std::size_t mer_pos)
      {
        return m_responses.data() + (mer_pos * m_block_size);
      }

    private:
      std::vector<std::uint8_t> m_responses;
      std::size_t m_block_size {0};
  };

  using query_response_t = std::unique_ptr<query_response>;

  class query
  {
    public:
      query(const std::string& name,
            const std::string& seq,
            std::size_t smer_size,
            std::size_t z_size,
            std::size_t nb_samples,
            double threshold,
            std::size_t width = 1)
        : m_name(name),
          m_seq(seq),
          m_size(m_seq.size() - smer_size + 1),
          m_ksize(m_seq.size() - (smer_size + z_size) + 1),
          m_zsize(z_size),
          m_threshold(threshold)
      {
        m_responses = std::make_unique<query_response>(size(), nb_samples, width);
      }

      std::uint8_t* response_block(std::size_t mer_pos)
      {
        return m_responses->get(mer_pos);
      }

      std::size_t block_size() const
      {
        return m_responses->block_size();
      }

      std::size_t size() const { return m_size; }
      std::size_t ksize() const { return m_ksize; }
      std::size_t zsize() const  { return m_zsize; }
      std::string name() const { return m_name; }
      double threshold() const { return m_threshold; }

      smer_iterator iterate(std::size_t smer_size, smer_hasher* h)
      {
        return smer_iterator(m_seq, smer_size, h);
      }

    private:
      std::string m_name;
      std::string m_seq;
      std::size_t m_size;
      std::size_t m_ksize;
      std::size_t m_zsize;
      double m_threshold;
      query_response_t m_responses;
  };

  //struct kinfos {
  //  std::size_t kmer_size {0};
  //  std::size_t minim_size {0};
  //  std::size_t smer_size {0};
  //  std::size_t nb_samples {0};
  //};

  //class kquery;

  //class smer
  //{
  //  public:
  //    smer(std::uint64_t position, std::int64_t partition, const kinfos& ks)
  //      : m_position(position), m_partition(partition)
  //    {
  //      m_response.resize((ks.nb_samples + 7) / 8, 0);
  //    }

  //    std::uint64_t position() const { return m_position; }
  //    std::uint16_t partition() const { return m_partition; }
  //    std::vector<std::uint8_t>& response() { return m_response; }

  //    bool match(std::size_t sample) const
  //    {
  //      return BITCHECK(m_response, sample);
  //    }

  //  private:
  //    std::uint64_t m_position {0};
  //    std::uint16_t m_partition {0};
  //    std::uint16_t m_nbits {0};
  //    std::vector<std::uint8_t> m_response;
  //};


  //class query;

  //class kquery {

  //  using repart_type = std::shared_ptr<km::Repartition>;
  //  using hw_type = std::shared_ptr<km::HashWindow>;
  //  using kmer_type = km::Kmer<32>;

  //  public:

  //    kquery() {}
  //    kquery(const std::string_view& kmer,
  //           const kinfos& ks,
  //           repart_type repart,
  //           hw_type hw)
  //    {
  //      for (std::size_t i = 0; i < kmer.size() - ks.smer_size + 1; ++i)
  //      {
  //        kmer_type sk(ks.smer_size); sk.set_polynom(&kmer[i], ks.smer_size);
  //        kmer_type cano = sk.canonical();
  //        std::uint32_t p = repart->get_partition(cano.minimizer(ks.minim_size).value());
  //        std::uint64_t pos = XXH64(cano.get_data64(), 8, 0) % hw->get_window_size_bits();

  //        m_smers.push_back(smer(pos, p, ks));
  //      }
  //    }

  //    auto begin() { return m_smers.begin(); }
  //    auto end() { return m_smers.end(); }

  //    bool match(std::size_t sample)
  //    {
  //      return std::all_of(begin(), end(), [&sample](const smer& s) { return s.match(sample); });
  //    }

  //  private:
  //    std::vector<smer> m_smers;
  //};

  //class query
  //{
  //  using repart_type = std::shared_ptr<km::Repartition>;
  //  using hw_type = std::shared_ptr<km::HashWindow>;
  //  using kmer_type = km::Kmer<32>;

  //  public:
  //    query(const std::string& id, const std::string& seq, const kinfos& ks, repart_type repart, hw_type hw)
  //     : m_id(id)
  //    {
  //      for (std::size_t i = 0; i < seq.size() - ks.kmer_size + 1; ++i)
  //      {
  //        m_kqueries.push_back(
  //            kquery(
  //              std::string_view(seq.data()+i, ks.kmer_size),
  //              ks,
  //              repart,
  //              hw
  //            )
  //        );
  //      }
  //    }

  //    const std::string& name() const { return m_id; }

  //      std::vector<double> shared_ratios(std::size_t nb_samples)
  //      {
  //        std::vector<std::uint32_t> hits(nb_samples, 0);
  //        for (auto& kq : m_kqueries)
  //        {
  //          for (std::size_t i = 0; i < nb_samples; ++i)
  //          {
  //            if (kq.match(i))
  //              hits[i] += 1;
  //          }
  //        }

  //        std::vector<double> ratios(nb_samples, 0);

  //        for (std::size_t i = 0; i < nb_samples; ++i)
  //        {
  //          ratios[i] = hits[i] / static_cast<double>(m_kqueries.size());
  //        }

  //        return ratios;

  //      }

  //    auto begin() { return m_kqueries.begin(); }
  //    auto end() { return m_kqueries.end(); }

  //  private:
  //    std::string m_id;
  //    std::vector<kquery> m_kqueries;
  //};
}


#endif /* end of include guard: QUERY_HPP_HIJEZMAU */
