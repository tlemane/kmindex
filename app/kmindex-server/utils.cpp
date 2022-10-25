#include "compress.hpp"
#include "utils.hpp"

namespace kmq {

  json json_error(const std::string& s)
  {
    json j;
    j["error"] = s;
    return j;
  }

  void send_response(response_t& response, const request_t& request, std::string&& msg)
  {
    auto head = SimpleWeb::CaseInsensitiveMultimap(
      { { "Content-type", "application/json" } } );

    auto ec = request->header.find("Accept-Encoding");
    auto compress =
      ec != request->header.end() && ec->second.find("deflate") != std::string::npos;

    if (compress)
    {
      msg = compress_string(msg);
      head.insert(std::make_pair("Content-Encoding", "deflate"));
      head.insert(std::make_pair("Content-Length", std::to_string(msg.size())));
    }

    response->write(SimpleWeb::StatusCode::success_ok, msg, head);
  }

}
