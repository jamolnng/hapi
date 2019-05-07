#ifndef HAPI_CAMERA_H
#define HAPI_CAMERA_H

#include <map>
#include <memory>
#include <string>
#include <vector>

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
  std::map<std::string, std::string> get_device_info();
  // sets the acquisition mode (SingleFrame, MultiFrame, Continuous)
  void set_acquisition_mode(const Spinnaker::AcquisitionModeEnums mode);
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
  // set auto exposure on/off
  void set_auto_exposure(Spinnaker::ExposureAutoEnums a);
  // set exposure mode
  void set_exposure_mode(Spinnaker::ExposureModeEnums mode);
  // set exposure time
  void set_exposure(double microseconds);
  // set auto gain on/off
  void set_auto_gain(Spinnaker::GainAutoEnums a);
  // set gain
  void set_gain(double gain);
  // set image format
  void set_pixel_format(Spinnaker::PixelFormatEnums fmt);

 private:
  // Spinnaker camera pointer
  Spinnaker::CameraPtr _ptr;
  // trigger type
  TriggerType _type{TriggerType::SOFTWARE};
};
}  // namespace hapi
#endif
