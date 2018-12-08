#ifndef HAPI_OS_UTILS_H
#define HAPI_OS_UTILS_H

namespace hapi {
void signal_handler(int sig);
// sets usb filesystem memory to 1000 megabytes
bool set_usbfs_mb();
bool initialize_signal_handlers();

bool is_root();
};  // namespace hapi

#endif
