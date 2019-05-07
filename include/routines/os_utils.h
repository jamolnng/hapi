#ifndef HAPI_OS_UTILS_H
#define HAPI_OS_UTILS_H
#include <atomic>
#include <string>

namespace hapi {
extern volatile std::atomic<bool> running;

// Sets USB filesystem memory to 1000 megabytes
bool set_usbfs_mb();
// Initializes signal handlers for SIGINT, SIGQUIT, and SIGABRT
bool initialize_signal_handlers();

// Returns true if the program is running as the root user.
bool is_root();
// Runs a terminal command and returns the result as a std::string.
std::string exec(const char *cmd);
};  // namespace hapi

#endif
