#ifndef UTILS_HPP_NLUODCAF
#define UTILS_HPP_NLUODCAF

#include <string>

namespace kmq {

  std::string get_binary_dir();

  std::string cmd_exists(const std::string& dir, const std::string& cmd);

  int exec_cmd(const std::string& cmd, const std::string& args,
               const std::string& sout = "", const std::string& serr = "");
}

#endif /* end of include guard: UTILS_HPP_NLUODCAF */
