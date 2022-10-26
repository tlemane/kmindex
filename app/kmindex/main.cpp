#include <kmindex/config.hpp>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "cli.hpp"

int main(int argc, char* argv[])
{
  kmq::kmq_cli cli("kmindex", "Index kmtricks bf matrices", KMQ_PROJECT_TAG);

  auto [cmd, options] = cli.parse(argc, argv);

  if (options->verbosity == "debug")
    spdlog::set_level(spdlog::level::debug);
  else if (options->verbosity == "info")
    spdlog::set_level(spdlog::level::info);
  else if (options->verbosity == "warning")
    spdlog::set_level(spdlog::level::warn);
  else if (options->verbosity == "error")
    spdlog::set_level(spdlog::level::err);

  auto cerr_logger = spdlog::stderr_color_mt("kmindex");
  cerr_logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");
  spdlog::set_default_logger(cerr_logger);

  try {
    switch (cmd)
    {
      case kmq::kmq_commands::kmq_register:
        kmq::main_register(options);
        break;
      case kmq::kmq_commands::kmq_query:
        kmq::main_query(options);
        break;
    }
  } catch (const std::exception& e) {
    spdlog::error(e.what());
  }

  return 0;
}
