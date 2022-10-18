#ifndef CLI_HPP_RZKATMPJ
#define CLI_HPP_RZKATMPJ

#include "common.hpp"
#include "register.hpp"
#include "query.hpp"

namespace kmq {

  class kmq_cli
  {
    public:
      kmq_cli(const std::string& name,
              const std::string& desc,
              const std::string& version);

      std::tuple<kmq_commands, kmq_options_t> parse(int argc, char* argv[]);

    private:
      parser_t m_cli_parser {nullptr};
      kmq_register_options_t m_kmq_register_opt {nullptr};
      kmq_query_options_t m_kmq_query_opt {nullptr};
  };
}

#endif /* end of include guard: CLI_HPP_RZKATMPJ */
