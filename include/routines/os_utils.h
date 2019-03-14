#ifndef HAPI_OS_UTILS_H
#define HAPI_OS_UTILS_H
#include <atomic>
#include <string>

namespace hapi {
extern volatile std::atomic<bool> running;

void signal_handler(int sig);
// sets usb filesystem memory to 1000 megabytes
bool set_usbfs_mb();
bool initialize_signal_handlers();

bool is_root();
std::string exec(const char *cmd);
};  // namespace hapi

#endif
