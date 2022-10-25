#ifndef COMPRESS_HPP_YEZUVILH
#define COMPRESS_HPP_YEZUVILH

#include <string>
#include <zlib.h>

namespace kmq {

  std::string compress_string(const std::string& s, int level = Z_BEST_COMPRESSION);

}


#endif /* end of include guard: COMPRESS_HPP_YEZUVILH */
