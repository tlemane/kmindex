#ifndef IO_HPP_HHP3E0VV
#define IO_HPP_HHP3E0VV

#include <array>
#include <vector>
#include <iostream>
#include <fstream>
#include <kmindex/config.hpp>
#include <kmindex/exceptions.hpp>

namespace kmq {

  enum class index_type : std::uint16_t {
    pa,
    abs
  };

  inline std::string index_type_str(enum index_type it)
  {
    switch (it)
    {
      case index_type::pa:
        return "pa";
      case index_type::abs:
        return "abs";
    }
  }

  enum class result_type : std::uint16_t {
    ratio,
    dist,
    ratio_and_dist
  };

  inline std::string result_type_str(enum result_type rt)
  {
    switch (rt)
    {
      case result_type::ratio:
        return "ratio";
      case result_type::dist:
        return "dist";
      case result_type::ratio_and_dist:
        return "ratio_and_dist";
    }
  }


  #pragma pack(1)
  struct header {
    header() = default;

    header(result_type rtype, std::uint32_t bitw, std::uint32_t nb_samples)
      : magic(magic_def),
        bitw(bitw),
        nb_samples(nb_samples),
        itype(bitw > 1 ? index_type::abs : index_type::pa),
        rtype(rtype) {}

    constexpr static std::array<std::uint8_t, 8> magic_def = {
      'K', 'M', 'I', 'N', 'D', 'E', 'X', '\0'
    };

    std::array<std::uint8_t, 8> magic;
    std::uint8_t version_major {KMQ_VER_MAJOR};
    std::uint8_t version_minor {KMQ_VER_MINOR};
    std::uint8_t version_patch {KMQ_VER_PATCH};
    std::uint8_t bitw {0};
    std::uint32_t nb_samples {0};
    index_type itype;
    result_type rtype;

    std::array<std::uint8_t, 44> free = {0};

    void load(std::istream& is)
    {
      is.read(reinterpret_cast<char*>(this), sizeof(*this));
    }

    void dump(std::ostream& os) const
    {
      os.write(reinterpret_cast<const char*>(this), sizeof(*this));
    }

    void sanity_check() const
    {
      if (magic != magic_def)
        throw kmq_error("Not a kmindex file.");
    }

  };
  #pragma pack()

  struct qresult {
    std::uint32_t qname_size;
    std::string qname;

    union result {
      double ratio;
      std::uint64_t abs;
    };

    std::vector<result> results;

    std::uint64_t nb_dist;
    std::vector<std::vector<std::uint8_t>> distributions;
  };

  class qres_writer
  {
    public:
      qres_writer(std::ostream& os, std::uint8_t bitw, std::uint32_t nb_samples, result_type rtype)
        : m_os(os), m_header(rtype, bitw, nb_samples)
      {
        m_header.dump(os);
      }

      void write(const std::string& query_name, const std::vector<double>& ratios)
      {
        write_query_name(query_name);
        write_ratios(ratios);
      }

      void write(const std::string& query_name, const std::vector<std::uint32_t>& counts)
      {
        write_query_name(query_name);
        write_abs(counts);
      }

      void write(const std::string& query_name,
                 const std::vector<double>& ratios,
                 const std::vector<std::vector<std::uint8_t>>& positions)
      {
        write_query_name(query_name);
        write_ratios(ratios);
        write_dist(positions);
      }

      void write(const std::string& query_name,
                 const std::vector<std::uint32_t>& counts,
                 const std::vector<std::vector<std::uint8_t>>& positions)
      {
        write_query_name(query_name);
        write_abs(counts);
        write_dist(positions);
      }

    private:
      void write_query_name(const std::string& query_name)
      {
        std::uint32_t s = query_name.size();
        m_os.write(reinterpret_cast<char*>(&s), sizeof(s));
        m_os.write(query_name.data(), query_name.size());
      }

      void write_abs(const std::vector<std::uint32_t>& counts)
      {
        std::vector<std::uint8_t> c; c.reserve(counts.size());
        for (const auto& e : counts)
        {
          c.push_back(e);
        }
        m_os.write(reinterpret_cast<const char*>(c.data()), c.size() * sizeof(std::uint8_t));
      }

      void write_ratios(const std::vector<double>& ratios)
      {
        m_os.write(reinterpret_cast<const char*>(ratios.data()), ratios.size() * sizeof(double));
      }

      void write_dist(const std::vector<std::vector<std::uint8_t>>& positions)
      {
        std::uint64_t n = 1;
        m_os.write(reinterpret_cast<char*>(&n), sizeof(n));

        std::uint64_t qs = positions[0].size();
        m_os.write(reinterpret_cast<char*>(&qs), sizeof(qs));
        for (auto& p : positions)
          m_os.write(reinterpret_cast<const char*>(p.data()), p.size() * sizeof(std::uint8_t));
      }

    private:
      std::ostream& m_os;
      header m_header;
  };

  class qres_reader
  {
    public:
      qres_reader(const std::string& path)
        : m_is(path, std::ios::binary | std::ios::in)
      {
        m_header.load(m_is);

        m_item.results.resize(m_header.nb_samples);

        if (m_header.rtype == result_type::dist || m_header.rtype == result_type::ratio_and_dist)
          m_item.distributions.resize(m_header.nb_samples);
      }

      bool next()
      {
        auto eof = read_query_name();

        if (eof)
          return false;

        if (m_header.rtype == result_type::ratio || m_header.rtype == result_type::ratio_and_dist)
          read_rc();

        if (m_header.rtype == result_type::dist || m_header.rtype == result_type::ratio_and_dist)
          read_dist();

        return true;
      }

      const qresult& item() const
      {
        return m_item;
      }

      void to_yaml(std::ostream& os, const std::vector<std::string>& sample_names)
      {
        for (auto& e : iterator(this))
        {
          os << e.qname << ":\n";

          for (std::size_t i = 0; i < sample_names.size(); ++i)
          {
            if (m_header.rtype == result_type::ratio)
            {
              if (m_header.itype == index_type::pa)
                os << "  - " << sample_names[i] << ": " << m_item.results[i].ratio << '\n';
              else
                os << "  - " << sample_names[i] << ": " << m_item.results[i].abs << '\n';
            }
            else
            {
              os << "  - " << sample_names[i] << ":\n";

              if (m_header.itype == index_type::pa)
                os << "    - R: " << m_item.results[i].ratio << '\n';
              else
                os << "    - C: " << m_item.results[i].abs << '\n';

              os << "    - P: [";

              for (std::size_t j = 0; j < m_item.distributions[i].size() - 1; ++j)
                os << std::to_string(m_item.distributions[i][j]) << ',';
              os << std::to_string(m_item.distributions[i].back()) << "]\n";

              os << "\n";
            }
          }
        }
      }

      void to_matrix(std::ostream& os, const std::vector<std::string>& sample_names)
      {
        for (auto& e : iterator(this))
        {

        }
      }

      const header& head() const
      {
        return m_header;
      }

    private:
      bool read_query_name()
      {
        m_is.read(reinterpret_cast<char*>(&m_item.qname_size), sizeof(m_item.qname_size));

        if (!(m_is.gcount() > 0))
          return true;

        m_item.qname.resize(m_item.qname_size);

        m_is.read(m_item.qname.data(), m_item.qname_size);

        return false;

      }

      void read_rc()
      {
        if (m_header.itype == index_type::pa)
          m_is.read(reinterpret_cast<char*>(m_item.results.data()), m_header.nb_samples * sizeof(qresult::result));
        else
        {
          std::vector<std::uint8_t> buf(m_header.nb_samples);
          m_is.read(reinterpret_cast<char*>(buf.data()), m_header.nb_samples * sizeof(std::uint8_t));
          for (std::size_t i = 0; i < m_item.results.size(); ++i)
          {
            m_item.results[i].abs = static_cast<std::uint64_t>(buf[i]);
          }
        }
      }

      void read_dist()
      {
        m_is.read(reinterpret_cast<char*>(&m_item.nb_dist), sizeof(m_item.nb_dist));
        std::size_t qs = 0;
        m_is.read(reinterpret_cast<char*>(&qs), sizeof(qs));

        for (std::size_t i = 0; i < m_header.nb_samples; ++i)
        {
          m_item.distributions[i].clear(); m_item.distributions[i].resize(qs);
          m_is.read(reinterpret_cast<char*>(m_item.distributions[i].data()), sizeof(std::uint8_t) *  qs);
        }
      }

      class iterator
      {
        public:
          iterator(qres_reader* reader)
            : m_reader(reader)
          {
            m_end = !reader->next();
          }

          struct iterator_sentinel {
            bool is_done(const iterator& it) const
            {
              return it.m_end;
            }
          };

          const qresult& operator*() const
          {
            return m_reader->item();
          }

          iterator& operator++()
          {
            m_end = !m_reader->next();
            return *this;
          }

          iterator operator++(int)
          {
            iterator tmp = *this;
            ++(*this);
            return tmp;
          }

          iterator begin() const
          {
            return *this;
          }

          iterator_sentinel end() const
          {
            return iterator_sentinel{};
          }

          friend bool operator ==(const iterator& lhs, const iterator_sentinel& rhs)
          {
            return rhs.is_done(lhs);
          }

          friend bool operator !=(const iterator& lhs, const iterator_sentinel& rhs)
          {
            return !rhs.is_done(lhs);
          }

        private:
          qres_reader* m_reader {nullptr};
          bool m_end {false};
      };

    private:
      std::ifstream m_is;
      header m_header;
      qresult m_item;
  };

}

#endif /* end of include guard: IO_HPP_HHP3E0VV */
