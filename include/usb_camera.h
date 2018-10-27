#ifndef CAMERA_H
#define CAMERA_H

#include <memory>
#include <string>
#include <vector>

#include "SpinGenApi/SpinnakerGenApi.h"
#include "Spinnaker.h"

using namespace Spinnaker;
using namespace Spinnaker::GenApi;
using namespace Spinnaker::GenICam;

namespace hapi {
// camera interface to make using Spinnaker easier
class USBCamera {
 public:
  enum TriggerType { SOFTWARE, HARDWARE };

  // initialize the Spinnaker system
  static void init_sys();
  // cleans up camera and camera system
  static void cleanup();
  // returns the number of detected cameras
  static unsigned int num_cams();
  // gets the camera by index
  static std::shared_ptr<Camera> get(unsigned int id);
  // refreshes the list of cameras
  static bool update_cameras();

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
  void set_acquisition_mode(gcstring mode = "Continuous");
  // begin image acquisition
  void begin_acquisition();
  // get an acquired image, waits for one if there isn't one ready
  ImagePtr acquire_image();
  // end image acquisition
  void end_acquisition();
  // initialize the camera
  void init();
  // de-initialize the camera
  void deinit();

 private:
  USBCamera();
  ~USBCamera();

  // Spinnaker camera pointer
  CameraPtr _ptr;
  // trigger type
  TriggerType _type{TriggerType::SOFTWARE};

  // Spinnaker system pointer
  static std::shared_ptr<SystemPtr> _system;
  // Spinnaker camera list
  static std::shared_ptr<CameraList> _clist;
  // list of cameras
  static std::vector<std::shared_ptr<Camera>> _cameras;
};
}  // namespace hapi
#endif