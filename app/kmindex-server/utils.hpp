#ifndef UTILS_HPP_OEH2LBRD
#define UTILS_HPP_OEH2LBRD

#include <string>

#include <nlohmann/json.hpp>
#include <server_http.hpp>

using json = nlohmann::json;

namespace kmq {

  using http_server_t = SimpleWeb::Server<SimpleWeb::HTTP>;
  using response_t = std::shared_ptr<http_server_t::Response>;
  using request_t = std::shared_ptr<http_server_t::Request>;

  json json_error(const std::string& msg);

  void send_response(response_t& response, const request_t& request, std::string&& msg, bool json);
}

#endif /* end of include guard: UTILS_HPP_OEH2LBRD */
