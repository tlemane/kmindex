#include "cli.hpp"
#include <iostream>

namespace kmq {

  kmq_cli::kmq_cli(const std::string& name, const std::string& desc, const std::string& version)
  {
    m_cli_parser = std::make_shared<bc::Parser<1>>(name, desc, version, std::string{});
    m_kmq_register_opt = std::make_shared<struct kmq_register_options>(kmq_register_options{});
    m_kmq_query_opt = std::make_shared<struct kmq_query_options>(kmq_query_options{});

    kmq_register_cli(m_cli_parser, m_kmq_register_opt);
    kmq_query_cli(m_cli_parser, m_kmq_query_opt);
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

    if (m_cli_parser->is("register"))
      return std::make_tuple(kmq_commands::kmq_register, m_kmq_register_opt);
    else if (m_cli_parser->is("query"))
      return std::make_tuple(kmq_commands::kmq_query, m_kmq_query_opt);
    else
      exit(EXIT_FAILURE);
  }
}
