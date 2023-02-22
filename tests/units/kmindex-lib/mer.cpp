#include <cstdlib>

#include <gtest/gtest.h>
#include <fmt/format.h>
#include <kmindex/mer.hpp>
#include <kmindex/index/index_infos.hpp>

static const std::string data_path(std::getenv("KMINDEX_TEST_DATA"));

TEST(kmindex_lib_mer, smer_iterator)
{
  kmq::index_infos infos("index", fmt::format("{}/indexes/pa_index", data_path));

  auto r = infos.get_repartition();
  auto h = infos.get_hash_w();

  std::string seq("AGCAGATTAAGCATATATATCCGACGATAACACTA");

  const std::vector<std::uint32_t> parts {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 3
  };

  const std::vector<std::uint64_t> hashes {
    102658,
    123165,
    161320,
    193062,
    164937,
    169286,
    60386,
    144980,
    49698,
    4538,
    189458
  };

  EXPECT_EQ(infos.minim_size(), 10);
  EXPECT_EQ(infos.smer_size(), 25);
  EXPECT_EQ(infos.bw(), 1);

  kmq::smer_hasher hh(r, h, infos.minim_size());

  std::vector<kmq::smer> smers;
  for (auto& e : kmq::smer_iterator(seq, infos.smer_size(), &hh))
    smers.push_back(e);

  EXPECT_EQ(smers.size(), parts.size());

  for (std::size_t i = 0; i < smers.size(); ++i)
  {
    EXPECT_EQ(smers[i].p, parts[i]);
    EXPECT_EQ(smers[i].h, hashes[i]);
  }
}

