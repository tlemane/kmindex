#include <kmindex/exceptions.hpp>

#include <fmt/format.h>

namespace kmq {

  kmq_error::kmq_error(const std::string& msg) noexcept
    : m_msg(fmt::format("[{}]: {}", name(), msg)) {}

  const char* kmq_error::what() const noexcept
  {
    return m_msg.c_str();
  }

  std::string kmq_error::name() const noexcept
  {
    return "kmq_error";
  }

  std::string kmq_io_error::name() const noexcept { return "kmq_io_error"; }
  std::string kmq_invalid_request::name() const noexcept { return "kmq_invalid_request"; }
  std::string kmq_invalid_index::name() const noexcept { return "kmq_invalid_index"; }
}
