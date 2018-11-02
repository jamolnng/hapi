#ifndef CAMERA_H
#define CAMERA_H

#include <memory>
#include <string>
#include <vector>

#include "SpinGenApi/SpinnakerGenApi.h"
#include "Spinnaker.h"

namespace hapi {
// camera interface to make using Spinnaker easier
class USBCamera {
 public:
  enum TriggerType { SOFTWARE, HARDWARE };

  USBCamera(Spinnaker::CameraPtr ptr);
  ~USBCamera();

  // returns true if the camera is initialized
  bool is_initialized();
  // configure trigger to the given type
  void configure_trigger(TriggerType type);
  // triggers the camera to capture an image
  void grab_next_image_by_trigger();
  // resets the trigger and disables it
  void reset_trigger();
  // prints the device info to the console
  void print_device_info();
  // sets the acquisition mode (SingleFrame, MultiFrame, Continuous)
  void set_acquisition_mode(
      const Spinnaker::GenICam::gcstring& mode = "Continuous");
  // begin image acquisition
  void begin_acquisition();
  // get an acquired image, waits for one if there isn't one ready
  Spinnaker::ImagePtr acquire_image();
  // end image acquisition
  void end_acquisition();
  // initialize the camera
  void init();
  // de-initialize the camera
  void deinit();

 private:
  // Spinnaker camera pointer
  Spinnaker::CameraPtr _ptr;
  // trigger type
  TriggerType _type{TriggerType::SOFTWARE};
};
}  // namespace hapi
#endif