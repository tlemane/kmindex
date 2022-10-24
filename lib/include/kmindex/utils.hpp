#ifndef UTILS_HPP_TKVBI1VR
#define UTILS_HPP_TKVBI1VR

#include <string>
#include <iostream>
#include <functional>
#include <chrono>
#include <sstream>
#include <fmt/format.h>

#include <filesystem>
namespace fs = std::filesystem;

#define ROUND_UP(n, m) ((n + m - 1) / m) * m
#define ROUND_UP(n, m) ((n + m - 1) / m) * m
#define NBYTES(m) (m + 7) / 8
#define BITMASK(b) (1 << ((b) % CHAR_BIT))
#define BITSLOT(b) ((b) / CHAR_BIT)
#define BITSET(a, b) ((a)[BITSLOT(b)] |= BITMASK(b))
#define BITCHECK(a, b) ((a)[BITSLOT(b)] & BITMASK(b))

namespace kmq {

  template<typename... Args>
  void unused(Args&&...) {}

  const std::string WHITECHAR = " \n\r\t\f\v";

  template<typename T>
  inline void check_fstream_good(const std::string& path, const T& stream)
  {
    if (!stream.good())
    {
      if constexpr(std::is_same_v<T, std::ofstream>)
        throw std::runtime_error(fmt::format("Unable to write at {}.", path));
      else
        throw std::runtime_error(fmt::format("Unable to read at {}.", path));
    }
  }

  inline std::string rtrim(const std::string& s, const std::string& v = WHITECHAR)
  {
    size_t end = s.find_last_not_of(v);
    if (end == std::string::npos)
      return "";
    return s.substr(0, end+1);
  }

  inline std::string ltrim(const std::string& s, const std::string& v = WHITECHAR)
  {
    size_t beg = s.find_first_not_of(v);
    if (beg == std::string::npos)
      return "";
    return s.substr(beg);
  }

  inline std::string trim(const std::string& s, const std::string& v = WHITECHAR)
  {
    return ltrim(rtrim(s, v), v);
  }

  template<typename Formatter = std::function<std::string(const std::string&)>>
  inline std::vector<std::string> split(const std::string& s, char delim, Formatter format)
  {
    std::istringstream iss(s);
    std::vector<std::string> ret;
    for (std::string tmp; std::getline(iss, tmp, delim);)
      ret.push_back(format(tmp));
    return ret;
  }

  inline std::vector<std::string> split(const std::string& s, char delim)
  {
    return split(s, delim, [](const std::string& s) -> std::string {return s;});
  }

  inline std::size_t directory_size(const std::string& p)
  {
    std::size_t size = 0;
    for (auto& e : fs::directory_iterator(p))
    {
      if (e.is_regular_file())
        size += fs::file_size(e);
    }

    return size / 1024 / 1024;
  }

  class Timer
  {
    using time_point_t = std::chrono::time_point<std::chrono::steady_clock>;
    using days = std::chrono::duration<int, std::ratio<86400>>;

   public:
    Timer() {start();}

    template <typename Unit>
    auto elapsed()
    {
      if (m_running) end();
      return std::chrono::duration_cast<Unit>(m_end_time - m_start_time);
    }

    std::string formatted()
    {
      if (m_running) end();
      std::chrono::seconds seconds(std::chrono::duration_cast<std::chrono::seconds>(m_end_time - m_start_time));
      auto d = std::chrono::duration_cast<days>(seconds); seconds -= d;
      auto h = std::chrono::duration_cast<std::chrono::hours>(seconds); seconds -= h;
      auto m = std::chrono::duration_cast<std::chrono::minutes>(seconds); seconds -= m;
      auto s = std::chrono::duration_cast<std::chrono::seconds>(seconds);

      std::stringstream ss; ss.fill('0');
      if (d.count())
        ss << std::setw(2) << d.count() << "d";
      if (h.count())
        ss << std::setw(2) << h.count() << "h";
      if (m.count())
        ss << std::setw(2) << m.count() << "m";
      ss << std::setw(2) << s.count() << "s";
      return ss.str();
    }

    template <typename Unit>
    static auto time_it(std::function<void()> func)
    {
      auto start = std::chrono::steady_clock::now();
      func();
      auto end = std::chrono::steady_clock::now();
      return std::chrono::duration_cast<Unit>(end - start).count();
    }

    void reset()
    {
      m_running = false;
      m_start_time = time_point_t{};
      m_end_time = time_point_t{};
      start();
    }

   private:
    void start()
    {
      m_running = true;
      m_start_time = std::chrono::steady_clock::now();
    }
    void end()
    {
      m_running = false;
      m_end_time = std::chrono::steady_clock::now();
    }

   private:
    time_point_t m_start_time{};
    time_point_t m_end_time{};
    bool m_running{false};
  };

}

#endif /* end of include guard: UTILS_HPP_TKVBI1VR */
