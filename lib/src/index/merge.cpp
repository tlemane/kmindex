#include <kmindex/index/merge.hpp>
#include <bitpacker/bitpacker.hpp>
#include <nonstd/span.hpp>

namespace kmq {


  index_merger::index_merger(index* gindex,
                            const std::vector<std::string>& to_merge,
                            const std::string& new_path,
                            const std::string& new_name,
                            rename_mode mode,
                            std::size_t bw,
                            bool rm)
    : m_index(gindex),
      m_to_merge(to_merge),
      m_new_path(new_path),
      m_new_name(new_name),
      m_rename_mode(mode),
      m_bw(bw),
      m_rm(rm)
  {
    for (auto& n : m_to_merge)
    {
      m_nb_samples += gindex->get(n).nb_samples();
    }

    auto& ii = m_index->get(m_to_merge[0]);
    m_psize = ii.bloom_size() / ii.nb_partitions();
    m_nb_parts = ii.nb_partitions();
    m_buffer.resize(m_nb_parts);
  }

  void index_merger::remove_old() const
  {
    if (m_rm)
    {
      for (auto& n : m_to_merge)
      {
        fs::remove_all(fs::read_symlink(fmt::format("{}/{}", m_index->path(), n)));
      }
    }
  }

  void index_merger::remove_old(std::size_t p) const
  {
    if (m_rm)
    {
      for (auto& n : m_to_merge)
      {
        std::string path = fs::read_symlink(fmt::format("{}/{}", m_index->path(), n));
        fs::remove_all(fmt::format("{}/matrices/matrix_{}.cmbf", path, p));
      }
    }
  }

  void index_merger::copy_tree(const std::string& path) const
  {
    fs::copy(
      fmt::format("{}/options.txt", path), m_new_path, fs::copy_options::skip_existing);
    fs::copy(
      fmt::format("{}/build_infos.txt", path), m_new_path, fs::copy_options::skip_existing);
    fs::copy(
      fmt::format("{}/hash.info", path), m_new_path, fs::copy_options::skip_existing);
    fs::copy(fmt::format("{}/config_gatb", path),
             fmt::format("{}/config_gatb", m_new_path),
             fs::copy_options::skip_existing | fs::copy_options::recursive);

    fs::copy(fmt::format("{}/repartition_gatb", path),
             fmt::format("{}/repartition_gatb", m_new_path),
             fs::copy_options::skip_existing | fs::copy_options::recursive);

    {
      std::ifstream inf(fmt::format("{}/{}", path, "kmtricks.fof"));
      std::ofstream out(fmt::format("{}/{}", m_new_path, "kmtricks.fof"), std::ios::app);
      out << inf.rdbuf();
    }
  }

  void index_merger::copy_trees() const
  {
    fs::create_directory(m_new_path);

    for (auto& i : m_to_merge)
      copy_tree(fmt::format("{}/{}", m_index->path(), i));

    fs::create_directory(fmt::format("{}/matrices", m_new_path));
    fs::create_directory(fmt::format("{}/repartition_gatb", m_new_path));
    fs::create_directory(fmt::format("{}/config_gatb", m_new_path));
  }

  void index_merger::rename(const std::string& rename_string) const
  {
    auto id_files = split(rename_string, ',');
    std::size_t s = 0;

    std::stringstream ss;

    for (std::size_t i = 0; i < m_to_merge.size(); ++i)
    {
      switch (m_rename_mode)
      {
        case rename_mode::none:
          break;
        case rename_mode::file:
          m_index->get(m_to_merge[i]).rename_samples(id_files[i]);
          break;
        case rename_mode::format:
          s = m_index->get(m_to_merge[i]).rename_samples(rename_string, s);
          break;
      }
      ss << m_index->get(m_to_merge[i]).fof_str();
      m_index->remove_index(m_to_merge[i]);
    }

    {
      std::ofstream out(fmt::format("{}/{}", m_new_path, "kmtricks.fof"), std::ios::out);
      out << ss.str();
    }

    m_index->add_index(m_new_name, m_new_path);
    m_index->save();
    remove_old();
  }

  void index_merger::zero(std::size_t p) const
  {
    std::fill(m_buffer[p].begin(), m_buffer[p].end(), 0);
  }

  index_merger_pa::index_merger_pa(index* gindex,
                                   const std::vector<std::string>& to_merge,
                                   const std::string& new_path,
                                   const std::string& new_name,
                                   rename_mode mode,
                                   bool rm)
    : index_merger(gindex, to_merge, new_path, new_name, mode, 1, rm)
  {

  }


  void index_merger_pa::merge_one(std::size_t p) const
  {
    std::vector<std::pair<mio::mmap_source, std::size_t>> partitions;
    partitions.reserve(m_to_merge.size());

    for (auto& sub_name : m_to_merge)
    {
      auto& sub_index = m_index->get(sub_name);

      std::string index_path = fmt::format("{}/{}", m_index->path(), sub_name);
      partitions.push_back(
        std::make_pair(
          mio::mmap_source(fmt::format("{}/matrices/matrix_{}.cmbf", index_path, p), 49),
          sub_index.nb_samples()
        )
      );
    }

    std::ofstream out(fmt::format("{}/matrices/matrix_{}.cmbf", m_new_path, p));

    {
      std::uint64_t km_magic;
      std::uint32_t km_version;
      bool km_compress;

      std::uint64_t km_matrix_magic;
      std::uint32_t km_bits;
      std::uint32_t km_id;
      std::uint32_t km_part;
      std::uint64_t km_first;
      std::uint64_t km_window;

      std::string index_path = fmt::format("{}/{}", m_index->path(), m_to_merge[0]);
      std::ifstream inf(fmt::format("{}/matrices/matrix_{}.cmbf", index_path, p), std::ios::binary);

      inf.read(reinterpret_cast<char*>(&km_magic), sizeof(km_magic));
      inf.read(reinterpret_cast<char*>(&km_version), sizeof(km_version));
      inf.read(reinterpret_cast<char*>(&km_compress), sizeof(km_compress));
      inf.read(reinterpret_cast<char*>(&km_matrix_magic), sizeof(km_matrix_magic));
      inf.read(reinterpret_cast<char*>(&km_bits), sizeof(km_bits));
      inf.read(reinterpret_cast<char*>(&km_id), sizeof(km_id));
      inf.read(reinterpret_cast<char*>(&km_part), sizeof(km_part));
      inf.read(reinterpret_cast<char*>(&km_first), sizeof(km_first));
      inf.read(reinterpret_cast<char*>(&km_window), sizeof(km_window));

      out.write(reinterpret_cast<char*>(&km_magic), sizeof(km_magic));
      out.write(reinterpret_cast<char*>(&km_version), sizeof(km_version));
      out.write(reinterpret_cast<char*>(&km_compress), sizeof(km_compress));
      out.write(reinterpret_cast<char*>(&km_matrix_magic), sizeof(km_matrix_magic));
      out.write(reinterpret_cast<char*>(&km_bits), sizeof(km_bits));
      out.write(reinterpret_cast<char*>(&km_id), sizeof(km_id));
      out.write(reinterpret_cast<char*>(&km_part), sizeof(km_part));
      out.write(reinterpret_cast<char*>(&km_first), sizeof(km_first));
      out.write(reinterpret_cast<char*>(&km_window), sizeof(km_window));
    }

    m_buffer[p].resize((m_nb_samples + 7) / 8, 0);

    auto& buf = m_buffer[p];

    for (std::size_t i = 0; i < m_psize; ++i)
    {
      std::size_t bytes_per_line = (partitions[0].second + 7) / 8;
      std::size_t bits_per_line = bytes_per_line * 8;

      std::memcpy(
        &buf[0],
        partitions[0].first.begin() + (bytes_per_line * i),
        bytes_per_line
      );

      std::size_t current = partitions[0].second;

      bool prev_mul = ((current % 8) == 0);

      for (std::size_t j = 1; j < partitions.size(); ++j)
      {
        auto& p_ = partitions[j];

        bytes_per_line = (p_.second + 7) / 8;
        bits_per_line = bytes_per_line * 8;

        if (prev_mul)
        {
          std::memcpy(
            &buf[current / 8],
            p_.first.begin() + (bytes_per_line * i),
            bytes_per_line
          );

          current += bits_per_line;
          prev_mul = ((current % 8) == 0);
        }
        else
        {
          for (std::size_t k = 0; k < p_.second; ++k)
          {
            if (BITCHECK(p_.first, k + bits_per_line * i))
            {
              BITSET(buf, current);
            }
            ++current;
          }
        }
      }

      out.write(reinterpret_cast<char*>(buf.data()), buf.size());
      zero(p);
    }

    remove_old(p);
  }

  void index_merger_pa::merge(ThreadPool& pool) const
  {
    copy_trees();

    for (std::size_t p = 0; p < m_nb_parts; ++p)
    {
      pool.add_task([this, p](int i){
        unused(i);
        this->merge_one(p);
      });
    }

    pool.join_all();
  }

  index_merger_abs::index_merger_abs(index* gindex,
                                   const std::vector<std::string>& to_merge,
                                   const std::string& new_path,
                                   const std::string& new_name,
                                   rename_mode mode,
                                   bool rm,
                                   std::size_t bw)
    : index_merger(gindex, to_merge, new_path, new_name, mode, bw, rm)
  {

  }


  void index_merger_abs::merge_one(std::size_t p) const
  {
    std::vector<std::pair<mio::mmap_source, std::size_t>> partitions;
    partitions.reserve(m_to_merge.size());

    for (auto& sub_name : m_to_merge)
    {
      auto& sub_index = m_index->get(sub_name);

      std::string index_path = fmt::format("{}/{}", m_index->path(), sub_name);
      partitions.push_back(
        std::make_pair(
          mio::mmap_source(fmt::format("{}/matrices/matrix_{}.cmbf", index_path, p), 49),
          sub_index.nb_samples()
        )
      );
    }

    std::ofstream out(fmt::format("{}/matrices/matrix_{}.cmbf", m_new_path, p));

    {
      std::uint64_t km_magic;
      std::uint32_t km_version;
      bool km_compress;

      std::uint64_t km_matrix_magic;
      std::uint32_t km_bits;
      std::uint32_t km_id;
      std::uint32_t km_part;
      std::uint64_t km_first;
      std::uint64_t km_window;

      std::string index_path = fmt::format("{}/{}", m_index->path(), m_to_merge[0]);
      std::ifstream inf(fmt::format("{}/matrices/matrix_{}.cmbf", index_path, p), std::ios::binary);

      inf.read(reinterpret_cast<char*>(&km_magic), sizeof(km_magic));
      inf.read(reinterpret_cast<char*>(&km_version), sizeof(km_version));
      inf.read(reinterpret_cast<char*>(&km_compress), sizeof(km_compress));
      inf.read(reinterpret_cast<char*>(&km_matrix_magic), sizeof(km_matrix_magic));
      inf.read(reinterpret_cast<char*>(&km_bits), sizeof(km_bits));
      inf.read(reinterpret_cast<char*>(&km_id), sizeof(km_id));
      inf.read(reinterpret_cast<char*>(&km_part), sizeof(km_part));
      inf.read(reinterpret_cast<char*>(&km_first), sizeof(km_first));
      inf.read(reinterpret_cast<char*>(&km_window), sizeof(km_window));

      out.write(reinterpret_cast<char*>(&km_magic), sizeof(km_magic));
      out.write(reinterpret_cast<char*>(&km_version), sizeof(km_version));
      out.write(reinterpret_cast<char*>(&km_compress), sizeof(km_compress));
      out.write(reinterpret_cast<char*>(&km_matrix_magic), sizeof(km_matrix_magic));
      out.write(reinterpret_cast<char*>(&km_bits), sizeof(km_bits));
      out.write(reinterpret_cast<char*>(&km_id), sizeof(km_id));
      out.write(reinterpret_cast<char*>(&km_part), sizeof(km_part));
      out.write(reinterpret_cast<char*>(&km_first), sizeof(km_first));
      out.write(reinterpret_cast<char*>(&km_window), sizeof(km_window));
    }

    m_buffer[p].resize(((m_nb_samples * m_bw) + 7) / 8, 0);

    auto& buf = m_buffer[p];

    for (std::size_t i = 0; i < m_psize; ++i)
    {
      std::size_t bytes_per_line = ((partitions[0].second * m_bw) + 7) / 8;
      std::size_t bits_per_line = bytes_per_line * 8;

      std::memcpy(
        &buf[0],
        partitions[0].first.begin() + (bytes_per_line * i),
        bytes_per_line
      );

      std::size_t current = partitions[0].second * m_bw;

      for (std::size_t j = 1; j < partitions.size(); ++j)
      {
        auto& p_ = partitions[j];

        bytes_per_line = ((p_.second * m_bw) + 7) / 8;
        bits_per_line = bytes_per_line * 8;

        auto s = nonstd::span<const std::uint8_t>(
            reinterpret_cast<std::uint8_t*>(&p_.first[0]), bytes_per_line * m_psize);
        for (std::size_t k = 0; k < p_.second; ++k)
        {
          bitpacker::insert(
            buf,
            current,
            m_bw,
            bitpacker::extract<std::uint8_t>(s, (k * m_bw) + bits_per_line * i, m_bw)
          );

          current += m_bw;
        }
      }
      out.write(reinterpret_cast<char*>(buf.data()), buf.size());
      zero(p);
    }

    remove_old(p);
  }

  void index_merger_abs::merge(ThreadPool& pool) const
  {
    copy_trees();

    for (std::size_t p = 0; p < m_nb_parts; ++p)
    {
      pool.add_task([this, p](int i){
        unused(i);
        this->merge_one(p);
      });
    }

    pool.join_all();
  }


  index_merger_t make_merger(index* gindex,
                             const std::vector<std::string>& to_merge,
                             const std::string& new_path,
                             const std::string& new_name,
                             rename_mode mode,
                             bool rm,
                             std::size_t bw)
  {
    if (bw == 1)
      return std::make_shared<index_merger_pa>(
        gindex, to_merge, new_path, new_name, mode, rm
      );
    else
      return std::make_shared<index_merger_abs>(
        gindex, to_merge, new_path, new_name, mode, rm, bw
      );
  }




}
