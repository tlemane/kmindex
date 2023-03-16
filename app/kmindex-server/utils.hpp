#ifndef UTILS_HPP_OEH2LBRD
#define UTILS_HPP_OEH2LBRD

#include <string>

#include <nlohmann/json.hpp>
#include <server_http.hpp>
#include <kseq++/kseq++.hpp>

using json = nlohmann::json;

namespace kmq {

  struct string_iterator_wrapper
  {
    string_iterator_wrapper(const std::string& s)
      : beg(s.begin()), end(s.end()) {}
    std::string::const_iterator beg;
    std::string::const_iterator end;
  };

  inline int wrapper_read(string_iterator_wrapper* it, char* buffer, unsigned int len)
  {
    auto& beg = it->beg;
    auto& end = it->end;

    std::size_t n = 0;

    if (beg >= end)
      return 0;

    for (std::size_t i = 0; i < len; ++i)
    {
      *buffer++ = *beg++;
      ++n;

      if (beg >= end)
        return n;
    }
    return n;
  }

  class SeqStreamString
    : public klibpp::KStreamIn<string_iterator_wrapper*, int(*)(string_iterator_wrapper* it, char*, unsigned int)>
  {
    public:
      typedef klibpp::KStreamIn<string_iterator_wrapper*, int(*)(string_iterator_wrapper* it, char*, unsigned int) > base_type;

      SeqStreamString(string_iterator_wrapper* itw)
        : base_type (itw, wrapper_read, nullptr) {}
  };

  using http_server_t = SimpleWeb::Server<SimpleWeb::HTTP>;
  using response_t = std::shared_ptr<http_server_t::Response>;
  using request_t = std::shared_ptr<http_server_t::Request>;

  json json_error(const std::string& msg);

  void send_response(response_t& response, const request_t& request, std::string&& msg);
}

#endif /* end of include guard: UTILS_HPP_OEH2LBRD */
