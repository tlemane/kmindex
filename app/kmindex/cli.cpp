#include "cli.hpp"
#include <iostream>

namespace kmq {

  kmq_cli::kmq_cli(const std::string& name, const std::string& desc, const std::string& version)
  {
    m_cli_parser = std::make_shared<bc::Parser<1>>(name, desc, version, std::string{});
    m_kmq_register_opt = std::make_shared<struct kmq_register_options>(kmq_register_options{});
    m_kmq_query_opt = std::make_shared<struct kmq_query_options>(kmq_query_options{});
    m_kmq_build_opt = std::make_shared<struct kmq_build_options>(kmq_build_options{});
    m_kmq_infos_opt = std::make_shared<struct kmq_infos_options>(kmq_infos_options{});
    m_kmq_merge_opt = std::make_shared<struct kmq_merge_options>(kmq_merge_options{});

    kmq_build_cli(m_cli_parser, m_kmq_build_opt);
    kmq_register_cli(m_cli_parser, m_kmq_register_opt);
    kmq_query_cli(m_cli_parser, m_kmq_query_opt);
    kmq_merge_cli(m_cli_parser, m_kmq_merge_opt);
    kmq_infos_cli(m_cli_parser, m_kmq_infos_opt);
  }

  std::tuple<kmq_commands, kmq_options_t> kmq_cli::parse(int argc, char* argv[])
  {
    try
    {
      m_cli_parser->parse(argc, argv);
    }
    catch (const bc::ex::BCliError& e)
    {
      bc::utils::exit_bcli(e);
      exit(EXIT_FAILURE);
    }

    if (m_cli_parser->is("build"))
      return std::make_tuple(kmq_commands::kmq_build, m_kmq_build_opt);
    else if (m_cli_parser->is("register"))
      return std::make_tuple(kmq_commands::kmq_register, m_kmq_register_opt);
    else if (m_cli_parser->is("query"))
      return std::make_tuple(kmq_commands::kmq_query, m_kmq_query_opt);
    else if (m_cli_parser->is("index-infos"))
      return std::make_tuple(kmq_commands::kmq_index_infos, m_kmq_infos_opt);
    else if (m_cli_parser->is("merge"))
      return std::make_tuple(kmq_commands::kmq_merge, m_kmq_merge_opt);
    else
      exit(EXIT_FAILURE);
  }
}
