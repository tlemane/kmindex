#include <kmindex/utils.hpp>

namespace kmq {

  std::string rtrim(const std::string& s, const std::string& v)
  {
    size_t end = s.find_last_not_of(v);
    if (end == std::string::npos)
      return "";
    return s.substr(0, end+1);
  }

  std::string ltrim(const std::string& s, const std::string& v)
  {
    size_t beg = s.find_first_not_of(v);
    if (beg == std::string::npos)
      return "";
    return s.substr(beg);
  }

  std::string trim(const std::string& s, const std::string& v)
  {
    return ltrim(rtrim(s, v), v);
  }

  std::vector<std::string> split(const std::string& s, char delim)
  {
    return split(s, delim, [](const std::string& s) -> std::string {return s;});
  }

  std::size_t directory_size(const std::string& p)
  {
    std::size_t size = 0;
    for (auto& e : fs::directory_iterator(p))
    {
      if (e.is_regular_file())
        size += fs::file_size(e);
    }

    return size / 1024 / 1024;
  }

  Timer::Timer()
  {
    start();
  }

  std::string Timer::formatted()
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

  void Timer::start()
  {
    m_running = true;
    m_start_time = std::chrono::steady_clock::now();
  }
  void Timer::end()
  {
    m_running = false;
    m_end_time = std::chrono::steady_clock::now();
  }
}