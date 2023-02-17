#include "utils.hpp"

#if __APPLE__
  #include <mach-o/dyld.h>
  #include <mach/mach.h>
#endif

#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

#include <vector>
#include <stdexcept>
#include <filesystem>

#include <bcli/bcli.hpp>
#include <spdlog/spdlog.h>
#include <fmt/format.h>

#include <kmindex/utils.hpp>

namespace kmq {

  namespace fs = std::filesystem;

  std::string get_binary_dir()
  {
    std::uint32_t size = 4096;
    char buffer[4096];

    #if __APPLE__
      _NSGetExecutablePath(buffer, &size);
    #else
      auto r = ::readlink("/proc/self/exe", buffer, size);
      if (r == -1)
        throw std::runtime_error("Unable to found kmindex binary path");
    #endif
    return fs::path(buffer).parent_path();
  }

  std::string cmd_exists(const std::string& dir, const std::string& cmd)
  {
    if (!std::system(fmt::format("which {} > /dev/null 2>&1", cmd).c_str()))
      return cmd;
    else if (!std::system(fmt::format("which {}/{} > /dev/null 2>&1", dir, cmd).c_str()))
      return fmt::format("{}/{}", dir, cmd);
    else
      throw std::runtime_error(fmt::format("{} not found.", cmd));
  }

  int exec_cmd(const std::string& cmd, const std::string& args,
               const std::string& sout, const std::string& serr)
  {
    std::string bin_name = fs::path(cmd).filename().string();
    std::vector<std::string> argsv = bc::utils::split(args, ' ');
    pid_t pid;
    int p[2];
    auto _i = pipe(p); unused(_i);
    int status;
    const char* cmd_name[] = {cmd.c_str()};
    const char** argv = new const char*[argsv.size() + 2];
    for (size_t i = 1; i < argsv.size() + 1; i++)
    {
      argv[i] = argsv[i - 1].c_str();
    }

    spdlog::info(fmt::format("exec: {} {}", bin_name, args));
    argv[0] = cmd_name[0];
    argv[argsv.size() + 1] = NULL;

    pid = fork();
    if (pid == 0)
    {
      int fd1 = -1, fd2 = -1;

      if (!sout.empty())
      {
        fd1 = open(sout.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
        dup2(fd1, STDOUT_FILENO);
      }

      if (!serr.empty())
      {
        fd2 = open(serr.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
        dup2(fd2, STDERR_FILENO);
      }

      if (bool failed = execvp(cmd_name[0], (char**)argv) == -1; failed)
      {
        delete[] argv;
        auto _w = write(p[1], &failed, sizeof(failed)); unused(_w);
        throw std::runtime_error(fmt::format("Failed to run {}.", cmd_name[0]));
      }

      close(fd1);
      close(fd2);

      delete[] argv;
    }
    else if (pid > 0)
    {
      if (wait(&status) == -1)
      {
        throw std::runtime_error("wait() failed.");
      }
      fcntl(p[0], F_SETFL, fcntl(p[0], F_GETFL) | O_NONBLOCK);
      bool failed = false;
      ssize_t r = read(p[0], &failed, sizeof(failed)); unused(r);

      if (WIFEXITED(status))
        if (WEXITSTATUS(status) != 0)
          throw std::runtime_error(fmt::format("{} exit with {}.", cmd_name[0], WEXITSTATUS(status)));

      if (!failed)
      {
        spdlog::info("{} exit normally with ({}).", bin_name, WEXITSTATUS(status));
        return 0;
      }
      else
        return 1;
    }
    else
    {
      throw std::runtime_error("fork() failed.");
    }
    return 1;
  }

}
