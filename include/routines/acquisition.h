#ifndef HAPI_ACQUISITON_H
#define HAPI_ACQUISITON_H

#include <chrono>
#include <filesystem>
#include <memory>

#include "usb_camera.h"

namespace hapi {
enum HAPIMode { TRIGGER, INTERVAL, TRIGGER_TEST };
void acquisition_loop(std::shared_ptr<USBCamera> &camera,
                      std::filesystem::path &out_dir, std::string &image_type,
                      std::chrono::milliseconds interval_time, HAPIMode mode);
void acquire_image(std::shared_ptr<USBCamera> &camera,
                   std::filesystem::path &out_dir, std::string &image_type,
                   unsigned int image_count);
};  // namespace hapi

#endif
