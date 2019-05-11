#ifndef HAPI_ACQUISITON_H
#define HAPI_ACQUISITON_H

#include <chrono>
#include <memory>
#include <string>

#include "filesystem.hpp"
#include "obis.h"
#include "usb_camera.h"

namespace hapi {
enum HAPIMode { TRIGGER, INTERVAL, TRIGGER_TEST, ALIGN, CW };
// This loops while the program is running to acquire images from the camera
// when a particle is detected.
void acquisition_loop(std::shared_ptr<USBCamera> &camera, OBISLaser &laser,
                      std::filesystem::path &out_dir, std::string &image_type,
                      std::chrono::milliseconds interval_time, HAPIMode mode);
// Acquires an image from the camera and saves it.
void acquire_image(std::shared_ptr<USBCamera> &camera,
                   std::filesystem::path &out_dir, std::string &image_type,
                   unsigned int image_count, const std::string &image_time,
                   HAPIMode mode);
// Returns true if the specified HAPI mode requires the camera.
bool use_camera(HAPIMode mode);
};  // namespace hapi

#endif
