#include "routines/os_utils.h"

#include <array>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <memory>

#include <sys/types.h>
#include <unistd.h>

#include "logger.h"

// bool that states whether the program should remain running

namespace hapi {
volatile std::atomic<bool> running{true};

// Returns true if the program is running as the root user.
bool is_root() { return getuid() == 0 && geteuid() == 0; }

void signal_handler(int sig) { running = false; }

// Sets USB filesystem memory to 1000 megabytes
bool set_usbfs_mb() {
  Logger &log = Logger::instance();
  // set usbfs memory
  log.info() << "Setting usbfs memory to 1000mb." << std::endl;
  std::system(
      "sudo sh -c 'echo 1000 > "
      "/sys/module/usbcore/parameters/usbfs_memory_mb'");
  std::ifstream in("/sys/module/usbcore/parameters/usbfs_memory_mb",
                   std::ios::binary);
  unsigned int mb = 0;
  if (in) {
    in >> mb >> std::ws;
    in.close();
  }
  return mb == 1000;
}

// Initializes signal handlers for SIGINT, SIGQUIT, and SIGABRT
bool initialize_signal_handlers() {
  // set signal handler
  Logger &log = Logger::instance();
  auto set_sh = [&](int sig) -> bool {
    log.info() << "Registering signal handler for signal " << sig << "."
               << std::endl;
    if (std::signal(sig, signal_handler) == SIG_ERR) {
      log.critical() << "Failed to set signal handler for signal " << sig
                     << std::endl;
      return false;
    }
    return true;
  };
  // register signal handlers
  return set_sh(SIGINT) &&
#ifdef SIGQUIT
         set_sh(SIGQUIT) &&
#endif
         set_sh(SIGABRT);
}

// Runs a terminal command and returns the result as a std::string.
std::string exec(const char *cmd) {
  std::array<char, 128> buffer;
  std::string result;
  std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
  if (!pipe) throw std::runtime_error("popen() failed!");
  while (!feof(pipe.get())) {
    if (fgets(buffer.data(), 128, pipe.get()) != NULL) result += buffer.data();
  }
  return result.substr(0, result.length() - 1);
}

// Returns the current run number
unsigned int run_number(bool increment) {
  unsigned int run_number = 0;
  std::ifstream in("/etc/hapi/run_count", std::ios::binary);
  if (in) {
    in >> run_number;
    in.close();
  } else {
    throw std::runtime_error(
        "Failed to open /etc/hapi/run_count to get run number.");
  }
  if (increment) {
    run_number++;
    std::ofstream out("/etc/hapi/run_count", std::ios::binary);
    if (out) {
      out << run_number;
      out.close();
    } else {
      throw std::runtime_error(
          "Failed to open /etc/hapi/run_count to increment run number.");
    }
  }
  return run_number;
}
};  // namespace hapi
