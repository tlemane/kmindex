#include <kmindex/config.hpp>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/dist_sink.h>

#include "server.hpp"

int main(int argc, char* argv[])
{
  auto options = std::make_shared<struct kmq::kmq_server_options>();
  auto cli_parser = std::make_shared<bc::Parser<0>>(
      "kmindex-server", "kmindex REST server", KMQ_PROJECT_TAG, "");
  kmq::kmq_server_cli(cli_parser, options);

  try {
    cli_parser->parse(argc, argv);
  }
  catch (const bc::ex::BCliError& e)
  {
    bc::utils::exit_bcli(e);
    exit(EXIT_FAILURE);
  }

  if (options->verbosity == "debug")
    spdlog::set_level(spdlog::level::debug);
  else if (options->verbosity == "info")
    spdlog::set_level(spdlog::level::info);
  else if (options->verbosity == "warning")
    spdlog::set_level(spdlog::level::warn);
  else if (options->verbosity == "error")
    spdlog::set_level(spdlog::level::err);

  auto daily_sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(
      fmt::format("{}/kmindex", options->log_directory), 0, 0);

  auto cerr_sink = std::make_shared<spdlog::sinks::stderr_color_sink_mt>();

  auto dist_sink = std::make_shared<spdlog::sinks::dist_sink_st>();

  dist_sink->add_sink(cerr_sink);
  dist_sink->add_sink(daily_sink);

  spdlog::flush_every(std::chrono::seconds(5));
  auto logger = std::make_shared<spdlog::logger>("kmindex", dist_sink);
  logger->set_pattern("[kms:%t]-[%D %H:%M:%S] [%^%l%$] %v");

  spdlog::set_default_logger(logger);

  try {
    kmq::main_server(options);
  } catch (const std::exception& e) {
    spdlog::error(e.what());
  }

  return 0;
}
