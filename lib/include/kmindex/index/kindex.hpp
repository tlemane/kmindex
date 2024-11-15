#ifndef INDEX_HPP_FJYOTLJN
#define INDEX_HPP_FJYOTLJN

#include <memory>
#include <kmindex/query/query_results.hpp>
#include <kmindex/index/index_infos.hpp>
#include <kmindex/spinlock.hpp>
#include <mio/mmap.hpp>

#include <azure/core.hpp>
#include <azure/identity/default_azure_credential.hpp>
#include <azure/storage/blobs.hpp>
#include <vector>
#include <string>

using namespace Azure::Identity;
using namespace Azure::Storage::Blobs;


#include <iostream>

namespace kmq {

  class partition_interface
  {
    public:
      virtual void query(std::uint64_t pos, std::uint8_t* dest) = 0;
      virtual ~partition_interface() {};
  };

  class partition : public partition_interface
  {
    public:
      partition(const std::string& matrix_path, std::size_t nb_samples, std::size_t width);

      ~partition();

      virtual void query(std::uint64_t pos, std::uint8_t* dest);

    private:
      int m_fd {0};
      mio::mmap_source m_mapped;
      std::size_t m_nb_samples {0};
      std::size_t m_bytes {0};
  };

  class blob_partition : public partition_interface
  {
    public:
      blob_partition(const std::string& blob_name, std::size_t nb_samples, std::size_t width, BlobContainerClient* bcc)
      : m_nb_samples(nb_samples), m_bytes(((nb_samples * width) + 7) / 8)
      {
        m_client = std::make_unique<BlockBlobClient>(bcc->GetBlockBlobClient(blob_name));
      }
      ~blob_partition() {}

      virtual void query(std::uint64_t pos, std::uint8_t* dest)
      {
        Azure::Storage::Blobs::DownloadBlobToOptions options;
        Azure::Core::Http::HttpRange range;
        range.Offset = (m_bytes * pos) + 49;
        range.Length = m_bytes;
        options.Range = range;
        options.TransferOptions.InitialChunkSize = m_bytes;
        options.TransferOptions.ChunkSize = m_bytes;
        options.TransferOptions.Concurrency = 1;
        m_client->DownloadTo(dest, m_bytes, options);
      }

      private:
        std::size_t m_nb_samples {0};
        std::size_t m_bytes {0};
        std::unique_ptr<BlockBlobClient> m_client;
  };

  class kindex
  {
    public:

      kindex();
      ~kindex();
      kindex(const index_infos& i, bool cache = false, bool blob_mode = true);

      void init(std::size_t p);
      void unmap(std::size_t p);

    public:
      std::string name() const;
      std::string directory() const;

      void solve(batch_query& bq)
      {
        std::vector<std::size_t> order; order.reserve(m_infos.nb_partitions());
        for (std::size_t p = 0; p < m_infos.nb_partitions(); p++)
          order.push_back(p);
#ifndef __APPLE__
        std::random_shuffle(std::begin(order), std::end(order));
#endif
        for (auto const& p : order)
          solve_one(bq, p);
      }

      void solve_one(batch_query& bq, std::size_t p)
      {
        auto& smers = bq.partition(p);
        auto& responses = bq.response();

        std::sort(std::begin(smers), std::end(smers));

        std::unique_lock<spinlock> lock(m_mutexes[p]);
        init(p);
        for (auto& [mer, qid] : smers)
        {
          m_partitions[p]->query(mer.h, responses[qid]->get(mer.i));
        }
        m_partitions[p] = nullptr;
      }

      void solve_cache(batch_query& bq)
      {
        std::vector<std::size_t> order; order.reserve(m_infos.nb_partitions());
        for (std::size_t p = 0; p < m_infos.nb_partitions(); p++)
          order.push_back(p);
#ifndef __APPLE__
        std::random_shuffle(std::begin(order), std::end(order));
#endif
        for (auto const& p : order)
          solve_one_cache(bq, p);
      }

      void solve_one_cache(batch_query& bq, std::size_t p)
      {
        auto& smers = bq.partition(p);
        auto& responses = bq.response();

        std::sort(std::begin(smers), std::end(smers));

        std::unique_lock<spinlock> lock(m_mutexes[p]);
        for (auto& [mer, qid] : smers)
        {
          m_partitions[p]->query(mer.h, responses[qid]->get(mer.i));
        }
      }

      void solve_batch(batch_query& bq)
      {
        if (m_cache)
          solve_cache(bq);
        else
          solve(bq);
      }

      index_infos& infos();
    private:
      index_infos m_infos;
      std::vector<std::unique_ptr<partition_interface>> m_partitions;
      std::vector<spinlock> m_mutexes;
      bool m_cache {false};

      bool blob_mode {false};
      std::unique_ptr<BlobServiceClient> azure_client;
      std::unique_ptr<BlobContainerClient> itp_client;
  };
}



#endif /* end of include guard: INDEX_HPP_FJYOTLJN */
