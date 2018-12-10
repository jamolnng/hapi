#include "routines/os_utils.h"

#include <csignal>
#include <fstream>

#include <sys/types.h>
#include <unistd.h>

#include "logger.h"

// bool that states whether the program should remain running

namespace hapi {
volatile std::atomic<bool> running{true};

bool is_root() { return getuid() == 0 && geteuid() == 0; }

void signal_handler(int sig) { running = false; }

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
    in >> mb;
    in.close();
  }
  return mb == 1000;
}

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
};  // namespace hapi
