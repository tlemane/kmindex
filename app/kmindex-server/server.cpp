#include "server.hpp"

#include <iostream>
#include <kmindex/query/query.hpp>
#include <kmindex/index/index.hpp>
#include <kmindex/query/format.hpp>
#include <kmindex/exceptions.hpp>

#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include <server_http.hpp>
#include <nlohmann/json.hpp>

#include "request.hpp"
#include "compress.hpp"
#include "utils.hpp"

using json = nlohmann::json;

namespace kmq {

  kmq_server_options_t kmq_server_cli(parser_t parser, kmq_server_options_t options)
  {
    parser->add_param("-i/--index", "Index path.")
       ->meta("STR")
       ->setter(options->index_path);

    parser->add_param("-a/--address", "Address to use (empty string to bind any address)\n" \
             "                             IPv4: dotted decimal form\n" \
             "                             IPv6: hexadimal form.\n" \
             "                             default -> ")
          ->meta("STR")
          ->def("127.0.0.1")
          ->setter(options->address);

    parser->add_param("-p/--port", "Port to use.")
       ->meta("INT")
       ->def("8080")
       ->setter(options->port);

    parser->add_param("-d/--log-directory", "Directory for daily logging.")
          ->meta("STR")
          ->def("kmindex_logs")
          ->setter(options->log_directory);

    parser->add_param("-s/--no-stderr", "Disable stderr logging.")
          ->as_flag()
          ->setter(options->no_stderr);

    parser->add_group("common", "");

    parser->add_param("-t/--threads", "Max number of parallel connections.")
      ->meta("INT")
      ->def("1")
      ->setter(options->nb_threads);

    parser->add_param("-h/--help", "Show this message and exit.")
          ->as_flag()
          ->action(bc::Action::ShowHelp);

    parser->add_param("--version", "Show version and exit.")
           ->as_flag()
           ->action(bc::Action::ShowVersion);

    parser->add_param("--verbose", "Verbosity level [debug|info|warning|error].")
      ->meta("STR")
      ->def("info")
      ->checker(bc::check::f::in("debug|info|warning|error"))
      ->setter(options->verbosity);


    return options;
  }


  using http_server_t = SimpleWeb::Server<SimpleWeb::HTTP>;
  using response_t = std::shared_ptr<http_server_t::Response>;
  using request_t = std::shared_ptr<http_server_t::Request>;

  std::thread start_server(http_server_t& s, const std::string& address, std::size_t port, std::size_t n)
  {

    spdlog::info("server running on {}:{}.", address, port);
    s.config.thread_pool_size = n;
    s.config.address = address;
    s.config.port = port;
    return std::thread([&s](){
        s.start();
    });
  }

  void accept_request(response_t& response,
                      const request_t& request,
                      const std::function<std::string(const std::string&)>& callback)
  {
    std::string content = request->content.string();

    spdlog::info("POST {} from {}", request->path, request->remote_endpoint().address().to_string());

    try {
      send_response(response, request, callback(content));

    } catch (const nlohmann::detail::exception& e) {
      spdlog::info("json parsing failure -> {}", e.what());
      response->write(SimpleWeb::StatusCode::client_error_bad_request,
                      json_error("json parsing failure, check your inputs").dump());
    }
      catch (const std::exception& e) {
      spdlog::info("bad client request -> {}", e.what());
      response->write(SimpleWeb::StatusCode::client_error_bad_request,
                      json_error(e.what()).dump());
    } catch (...) {
      spdlog::warn("internal server error");
      response->write(SimpleWeb::StatusCode::server_error_internal_server_error,
                     json_error("Internal server error").dump());
    }

  }

  void accept_get_request(response_t& response,
                          const request_t& request,
                          const std::string& p)
  {
    spdlog::info("GET {} from {}", request->path, request->remote_endpoint().address().to_string());

    std::ifstream inf(fmt::format("{}/index.json", p), std::ios::in);
    json data = json::parse(inf);
    data.erase("path");

    send_response(response, request, data.dump(4));
  }

  std::string perform_query(const std::string& content, index& global)
  {
    auto j = json::parse(content);

    request rq(j);
    spdlog::info("request -> search {} in {}", j["id"], j["index"].dump());

    return rq.solve(global).dump(4);
  }

  void main_server(kmq_server_options_t opt)
  {
    index global(opt->index_path);

    http_server_t server;

    server.resource["^/kmindex/query"]["POST"] = [&](response_t response, request_t request) {

      accept_request(response, request, [&](const std::string& content) {
        return perform_query(content, global);
      });

    };

    server.resource["^/kmindex/infos"]["GET"] = [&](response_t response, request_t request) {
      accept_get_request(response, request, opt->index_path);
    };

    auto s = start_server(server, opt->address, opt->port, opt->nb_threads);
    s.join();
  }
}
