#include <gtest/gtest.h>
#include <kmindex/mer.hpp>
#include <kmindex/index/index_infos.hpp>

TEST(kmindex_lib_mer, smer_iterator)
{
  std::string S = "AGCAGCTATACTCATCATCCATATCTACTACTACTCATCA";

  std::string p("/home/tlemane/Documents/git_repos/kmindex/tests/units/km_index");

  kmq::index_infos infos("name", p);

  auto r = infos.get_repartition();
  auto h = infos.get_hash_w();

  kmq::smer_hasher hh(r, h, 10);

  for (auto& e : kmq::smer_iterator(S, 20, &hh))
  {
    std::cout << e << std::endl;
  }

}

