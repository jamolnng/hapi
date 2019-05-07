#include "usb_camera.h"

#include <iostream>

#include "SpinGenApi/SpinnakerGenApi.h"

using namespace hapi;

using namespace Spinnaker;
using namespace Spinnaker::GenApi;
using namespace Spinnaker::GenICam;

USBCamera::USBCamera(CameraPtr ptr) : _ptr(ptr) {}

USBCamera::~USBCamera() {}

bool USBCamera::is_initialized() { return _ptr->IsInitialized(); }

void USBCamera::configure_trigger(USBCamera::TriggerType type) {
  _ptr->TriggerMode.SetValue(Spinnaker::TriggerModeEnums::TriggerMode_Off);
  if (type == TriggerType::SOFTWARE) {
    _ptr->TriggerSource.SetValue(
        Spinnaker::TriggerSourceEnums::TriggerSource_Software);
  } else {
    _ptr->TriggerSource.SetValue(
        Spinnaker::TriggerSourceEnums::TriggerSource_Line0);
  }
  _ptr->TriggerMode.SetValue(Spinnaker::TriggerModeEnums::TriggerMode_On);
  _type = type;
}

void USBCamera::grab_next_image_by_trigger() {
  if (_type == USBCamera::TriggerType::SOFTWARE) {
    _ptr->TriggerSoftware.Execute();
  }
}

void USBCamera::reset_trigger() {
  _ptr->TriggerMode.SetValue(Spinnaker::TriggerModeEnums::TriggerMode_Off);
}

std::map<std::string, std::string> USBCamera::get_device_info() {
  std::map<std::string, std::string> device_info;
  INodeMap& nmap = _ptr->GetTLDeviceNodeMap();

  FeatureList_t features;
  CCategoryPtr category = nmap.GetNode("DeviceInformation");
  if (IsAvailable(category) && IsReadable(category)) {
    category->GetFeatures(features);

    FeatureList_t::const_iterator it;
    for (it = features.begin(); it != features.end(); ++it) {
      CNodePtr feature_node = *it;
      CValuePtr value = (CValuePtr)feature_node;
      std::string node = std::string(feature_node->GetName().c_str());
      std::string val =
          (IsReadable(value) ? std::string(value->ToString().c_str())
                             : "Node not readable");

      device_info[node] = val;
    }
  } else {
    throw std::runtime_error("Device control information not available.");
  }
  return device_info;
}

void USBCamera::set_acquisition_mode(
    const Spinnaker::AcquisitionModeEnums mode) {
  _ptr->AcquisitionMode.SetValue(mode);
}

void USBCamera::begin_acquisition() { _ptr->BeginAcquisition(); }

ImagePtr USBCamera::acquire_image() {
  try {
    grab_next_image_by_trigger();
  } catch (const std::exception& ex) {
  }
  return _ptr->GetNextImage();
}

void USBCamera::end_acquisition() { _ptr->EndAcquisition(); }

void USBCamera::init() { _ptr->Init(); }

void USBCamera::deinit() { _ptr->DeInit(); }

void USBCamera::set_auto_exposure(Spinnaker::ExposureAutoEnums a) {
  _ptr->ExposureAuto.SetValue(a);
}

void USBCamera::set_exposure_mode(Spinnaker::ExposureModeEnums mode) {
  _ptr->ExposureMode.SetValue(mode);
}

void USBCamera::set_exposure(double microseconds) {
  _ptr->ExposureTime.SetValue(microseconds);
}

void USBCamera::set_auto_gain(Spinnaker::GainAutoEnums a) {
  _ptr->GainAuto.SetValue(a);
}

void USBCamera::set_gain(double gain) { _ptr->Gain.SetValue(gain); }

void USBCamera::set_pixel_format(Spinnaker::PixelFormatEnums fmt) {
  _ptr->PixelFormat.SetValue(fmt);
}
