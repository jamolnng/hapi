#ifndef HAPI_ACQUISITON_H
#define HAPI_ACQUISITON_H

#include <chrono>
#include <memory>
#include <string>

#include "obis.h"
#include "usb_camera.h"

#if _HAS_CXX17
#include <filesystem>
#else
#include <experimental/filesystem>
namespace std {
namespace filesystem = std::experimental::filesystem;
};
#endif

namespace hapi {
enum HAPIMode { TRIGGER, INTERVAL, TRIGGER_TEST };
void acquisition_loop(std::shared_ptr<USBCamera> &camera, OBISLaser &laser,
                      std::filesystem::path &out_dir, std::string &image_type,
                      std::chrono::milliseconds interval_time, HAPIMode mode);
void acquire_image(std::shared_ptr<USBCamera> &camera,
                   std::filesystem::path &out_dir, std::string &image_type,
                   unsigned int image_count, const std::string &image_time);
};  // namespace hapi

#endif
