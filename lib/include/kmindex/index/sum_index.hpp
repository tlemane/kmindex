#ifndef SUM_INDEX_HPP_1757144462
#define SUM_INDEX_HPP_1757144462

#include <cstddef>
#include <cmath>
#include <kmindex/index/index_infos.hpp>
#include <kmindex/dispatch.hpp>
#include <kmindex/utils.hpp>
#include <kmindex/index/common.hpp>

namespace kmq {

  template<typename T>
  [[nodiscard]] constexpr T lsb_mask(std::uint8_t n) noexcept
  {
    return T((1ULL << n) - 1);
  }

  template<typename T>
  [[nodiscard]] constexpr T msb_mask(std::uint8_t n) noexcept
  {
    return ~lsb_mask<T>(std::numeric_limits<T>::digits - n);
  }

  template<std::size_t Width>
  struct bit_packer
  {
    using word_type = std::uint64_t;
    using packed_type = std::uint32_t;
    using value_type = std::uint32_t;
    static constexpr std::size_t width { Width };
    static constexpr std::size_t digits { std::numeric_limits<word_type>::digits };
    static constexpr word_type mask {(word_type(1) << width) - 1};

    static inline constexpr value_type unpack(const word_type* data, std::size_t index) noexcept
    {
      if constexpr(width == digits)
      {
        return data[index];
      } 

      const std::size_t s = (index * width) / digits;
      const std::size_t e = ((index * width) + width - 1) / digits;

      if (s == e)
      {
        const std::size_t eb = ((index * width) + width - 1) % digits;
        return value_type(static_cast<packed_type>(data[s] >> (digits - (eb + 1))) & mask);
      }
      else
      {
        const std::size_t sb = (index * width) % digits;
        const std::size_t rem = width - (digits - sb);
        
        word_type w1 = data[s] & lsb_mask<word_type>(digits - sb);
        word_type w2 = (data[e] >> (digits - rem)) & lsb_mask<word_type>(rem);
        return value_type(((w1 << rem) | w2) & mask);
      }
    }
  
    static inline constexpr void pack(word_type* data, std::size_t index, value_type value) noexcept
    {
      if constexpr(width == digits)
      {
        data[index] = static_cast<word_type>(value);
        return;
      }

      const std::size_t s = (index * width) / digits;
      const std::size_t e = ((index * width) + width - 1) / digits;
      word_type v = static_cast<word_type>(value) & mask;

      const std::size_t sb = (index * width) % digits;

      if (s == e)
      {
        std::size_t eb = ((index * width) + width - 1) % digits;
        data[s] |= (v << (digits - (eb + 1)));  
      }
      else
      {
        const std::size_t rem = width - (digits - sb);
        
        data[s] |= ((v >> rem) & lsb_mask<word_type>(digits - sb));
        data[e] |= ((v & lsb_mask<word_type>(rem)) << (digits - rem));
      }
    }
  };

  template<std::size_t W>
  struct sum_packer
  {
    using bp = bit_packer<W>;

    void operator()(const unsigned char* source,
                             const std::size_t bytes_per_entry,
                             const std::size_t nb_entry,
                             const std::size_t nb_samples,
                             double correction,
                             std::uint64_t* dest) const noexcept
    {
      for (std::size_t i = 0; i < nb_entry; i++)
      {
        std::size_t j = 0;
        std::size_t c = 0;
        std::string vec;
        
        for (; j + 8 <= bytes_per_entry; j += 8)
        {
          c += __builtin_popcount(source[i * bytes_per_entry + j + 0]);
          c += __builtin_popcount(source[i * bytes_per_entry + j + 1]);
          c += __builtin_popcount(source[i * bytes_per_entry + j + 2]);
          c += __builtin_popcount(source[i * bytes_per_entry + j + 3]);
          c += __builtin_popcount(source[i * bytes_per_entry + j + 4]);
          c += __builtin_popcount(source[i * bytes_per_entry + j + 5]);
          c += __builtin_popcount(source[i * bytes_per_entry + j + 6]);
          c += __builtin_popcount(source[i * bytes_per_entry + j + 7]);
        }
        
        for (; j < bytes_per_entry; j++)
        {
          c += __builtin_popcount(source[i * bytes_per_entry + j]);
        }
      
        if (correction > 0.0)
        {
          double est = (static_cast<double>(c) - correction * static_cast<double>(nb_samples)) / (1.0 - correction);
          c = est < 0.0 ? 0 : static_cast<std::size_t>(std::llround(est));
        }

        bp::pack(dest, i, c);
      }
    }
  };

  class sum_query_response
  {
    public:
      sum_query_response(std::string name, std::size_t query_size, std::size_t width)
        : m_name(name), m_query_size(query_size), m_width(width)
      {
        m_data.resize((m_query_size * width + 63) / 64);
      }

      ~sum_query_response() = default;
    
    public:
      const std::string& name() const
      {
        return m_name;
      }

      std::size_t query_size() const
      {
        return m_query_size;
      }

      auto& storage()
      {
        return m_data;
      }

      const auto& storage() const
      {
        return m_data;
      }
      
    private:
      std::string m_name;
      std::size_t m_query_size {0};
      std::size_t m_width {0};
      std::vector<std::uint64_t> m_data;
  };

  using sum_query_response_t = std::unique_ptr<sum_query_response>;
  
  class sum_query_batch
  {
    public:
      using qsmer_type = std::pair<smer, std::uint32_t>;
      using qpart_type = std::vector<qsmer_type>;
      using repart_type = std::shared_ptr<km::Repartition>;
      using hw_type = std::shared_ptr<km::HashWindow>;

    public:
      sum_query_batch(std::size_t nb_samples,
                      std::size_t nb_partitions,
                      std::size_t smer_size,
                      repart_type repart,
                      hw_type hw,
                      std::size_t minim_size)
        : m_nb_samples(nb_samples),
          m_nb_partitions(nb_partitions),
          m_smer_size(smer_size),
          m_repart(repart),
          m_hw(hw),
          m_msize(minim_size),
          m_smers(nb_partitions)
      {
        m_width = static_cast<int>(std::log2(m_nb_samples)) + 1;
      }

      void add_query(const std::string name,
                     const std::string& seq)
      {
        std::size_t n = seq.size() - m_smer_size + 1;

        m_responses.push_back(
          std::make_unique<sum_query_response>(std::move(name), n, m_width)
        );

        std::uint32_t qid = m_responses.size() - 1;

        loop_executor<MAX_KMER_SIZE>::exec<smer_functor>(m_smer_size, m_smers, seq, qid, m_smer_size, m_repart, m_hw, m_msize);
      }

      qpart_type& partition(std::size_t p)
      {
        return m_smers[p];
      }

      auto& response()
      {
        return m_responses;
      }

      void free_smers()
      {
        std::vector<qpart_type>().swap(m_smers); 
      }

    private:
      std::size_t m_nb_samples {0};
      std::size_t m_nb_partitions {0};
      std::size_t m_smer_size {0};
      repart_type m_repart {nullptr};
      hw_type m_hw {nullptr};
      std::size_t m_msize {0};
      std::size_t m_width {0};

      std::vector<sum_query_response_t> m_responses;
      std::vector<qpart_type> m_smers;
  };

  template<std::size_t W>
  struct sum_solver
  {
    using bp = bit_packer<W>;
    void operator()(const std::uint64_t* packed,
                    sum_query_batch::qpart_type& smers,
                    std::vector<sum_query_response_t>& responses) noexcept
    {
      for (auto& [mer, qid] : smers)
      {
        std::uint32_t c = bp::unpack(packed, mer.h);
        bp::pack(responses[qid]->storage().data(), mer.i, c);
      }
    }
  };

  class sum_index
  {
    public:
      sum_index(const index_infos* infos)
        : m_infos(infos)
      {
        m_width = static_cast<int>(std::log2(m_infos->nb_samples())) + 1;
        m_bytes_per_entry = (m_infos->nb_samples() + 7) / 8;
      }

      ~sum_index() = default;

    public:
      void sum_partition(std::size_t part_id, double correction) const
      {
        auto p_path = m_infos->get_partition(part_id);
        std::size_t bloom_size = m_infos->bloom_size() / m_infos->nb_partitions();
        std::vector<std::uint64_t> sums((bloom_size * m_width + 63) / 64, 0);
        int fd = ::open(p_path.c_str(), O_RDONLY);
        auto mapped = mio::basic_mmap_source<unsigned char>(fd, 0, mio::map_entire_file);
        posix_madvise(&mapped[0], mapped.length(), POSIX_MADV_SEQUENTIAL);
        
        runtime_dispatch<32, 1, 1, std::equal_to<std::size_t>>::execute<sum_packer>(
          m_width,
          &mapped[0] + 49,
          m_bytes_per_entry,
          bloom_size,
          m_infos->nb_samples(),
          correction,
          sums.data()
        );

        mapped.unmap();
        ::close(fd);

        std::string out = m_infos->get_sum_partition(part_id);
        {
          std::ofstream ofs(out, std::ios::binary | std::ios::out);
          ofs.write(reinterpret_cast<const char*>(sums.data()), sizeof(std::uint64_t) * sums.size());
        }
      }

      void search_partition(std::size_t part_id, sum_query_batch& bq) const
      {
        auto& smers = bq.partition(part_id);
        std::sort(std::begin(smers), std::end(smers));
        int fd = ::open(m_infos->get_sum_partition(part_id).c_str(), O_RDONLY);
        auto mapped = mio::basic_mmap_source<unsigned char>(fd, 0, mio::map_entire_file);
        posix_madvise(&mapped[0], mapped.length(), POSIX_MADV_SEQUENTIAL);

        runtime_dispatch<32, 1, 1, std::equal_to<std::size_t>>::execute<sum_solver>(
          m_width,
          reinterpret_cast<const std::uint64_t*>(mapped.data()),
          smers,
          bq.response()
        );
      }

    private:
      const index_infos* m_infos;
      std::size_t m_width {0};
      std::size_t m_bytes_per_entry {0};
  };
}

#endif /* end of include guard: SUM_INDEX_HPP_1757144462 */