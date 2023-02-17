#ifndef VERSION_HPP_KUCJIMKM
#define VERSION_HPP_KUCJIMKM

#include <semver.hpp>
#include <kmindex/config.hpp>

namespace kmq {

  constexpr semver::version kmindex_version {
    KMQ_VER_MAJOR, KMQ_VER_MINOR, KMQ_VER_PATCH
  };

}

#endif /* end of include guard: VERSION_HPP_KUCJIMKM */
