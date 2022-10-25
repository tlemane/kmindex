#include <sstream>
#include <stdexcept>
#include <cstring>
#include "compress.hpp"

namespace kmq {

  // Source: https://panthema.net/2007/0328-ZLibString.html
  std::string compress_string(const std::string& str,
                              int compressionlevel)
  {
      z_stream zs;                        // z_stream is zlib's control structure
      memset(&zs, 0, sizeof(zs));

      if (deflateInit(&zs, compressionlevel) != Z_OK)
          throw(std::runtime_error("deflateInit failed while compressing."));

      zs.next_in = (Bytef*)str.data();
      zs.avail_in = str.size();           // set the z_stream's input

      int ret;
      char outbuffer[32768];
      std::string outstring;

      // retrieve the compressed bytes blockwise
      do {
          zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
          zs.avail_out = sizeof(outbuffer);

          ret = deflate(&zs, Z_FINISH);

          if (outstring.size() < zs.total_out) {
              // append the block to the output string
              outstring.append(outbuffer,
                               zs.total_out - outstring.size());
          }
      } while (ret == Z_OK);

      deflateEnd(&zs);

      if (ret != Z_STREAM_END) {          // an error occurred that was not EOF
          std::ostringstream oss;
          oss << "Exception during zlib compression: (" << ret << ") " << zs.msg;
          throw(std::runtime_error(oss.str()));
      }

      return outstring;
  }
}
