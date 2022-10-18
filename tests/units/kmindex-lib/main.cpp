#include <gtest/gtest.h>
#include <filesystem>

namespace fs = std::filesystem;

int main(int argc, char* argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  int r = RUN_ALL_TESTS();
  return r;
}
