#include <gtest/gtest.h>
#include <kmindex/index/kindex.hpp>
#include <kmindex/query/query.hpp>
#include <kmindex/utils.hpp>

TEST(kmindex_lib_kindex, partition)
{
  std::size_t nbits = 100;
  std::size_t nbytes = (100 + 7) / 8;
  std::size_t nkmers = 50000;
  {
    std::ofstream out("mat.cmbf", std::ios::out | std::ios::binary);

    char dummy[49] = {0};
    out.write(dummy, 49);


    for (std::size_t i = 0; i < nkmers ; ++i)
    {
      std::vector<std::uint8_t> bits(nbytes, 0);
      BITSET(bits, i % nbits);
      out.write(reinterpret_cast<char*>(bits.data()), nbytes);
    }
  }

  kmq::partition pquery("mat.cmbf", nbits);

  std::vector<std::uint8_t> resp(nbytes, 0);

  std::vector<std::uint32_t> pos(100, 0);
  std::iota(pos.begin(), pos.end(), 1);
  auto rng = std::default_random_engine{};

  std::shuffle(pos.begin(), pos.end(), rng);

  kmq::Timer t1;

  for (auto& i : pos)
  {
    pquery.query(i, resp.data());

    EXPECT_TRUE(BITCHECK(resp, i % nbits));
  }

  std::cout << t1.elapsed<std::chrono::milliseconds>().count() << std::endl;
}

TEST(kmindex_lib_kindex, partition_km)
{
  std::string p("/home/tlemane/Documents/git_repos/kmindex/tests/units/km_index");

  kmq::index_infos infos("name", p);

  std::string seq("ACGACGACGACGAGACGAGACGACAGCAGACAGAGACATAATATACTATATAATATATATAGCGAGGGGGGGAGAGCCAGCAGCACCCCCAAAAAAAAA");


  kmq::kindex ki(infos);



  kmq::query Q("ID1", seq, infos.smer_size(), 3, infos.nb_samples(), 0.0);

  auto r = ki.resolve(Q);
  r.compute_ratios();

  for (auto d : r.ratios())
  {
    std::cout << d << " " << std::endl;
  }
  //for (std::size_t i = 0; i < 4; ++i)
  //{
  //  kmq::partition pquery(infos.get_partition(i), ks.nb_samples);
  //  for (auto& kq : Q)
  //  {
  //    for (auto& sq : kq)
  //    {
  //      if (sq.partition() == i)
  //        pquery.query(sq.position(), sq.response().data());
  //    }
  //  }
  //}

  //auto r = Q.shared_ratios(2);

  //for (auto& d: r)
  //  std::cout << d << " ";

}
