#include "usb_camera.h"

#include <iostream>

#include "SpinGenApi/SpinnakerGenApi.h"
#include "Spinnaker.h"

using namespace hapi;

using namespace Spinnaker;
using namespace Spinnaker::GenApi;
using namespace Spinnaker::GenICam;

USBCamera::USBCamera(CameraPtr ptr) : _ptr(ptr) {}

USBCamera::~USBCamera() {}

bool USBCamera::is_initialized() { return _ptr->IsInitialized(); }

void USBCamera::configure_trigger(USBCamera::TriggerType type) {
  _ptr->TriggerMode.SetValue(Spinnaker::TriggerModeEnums::TriggerMode_Off);
  if (type == TriggerType::SOFTWARE)
    _ptr->TriggerSource.SetValue(
        Spinnaker::TriggerSourceEnums::TriggerSource_Software);
  else
    _ptr->TriggerSource.SetValue(
        Spinnaker::TriggerSourceEnums::TriggerSource_Line0);
  _ptr->TriggerMode.SetValue(Spinnaker::TriggerModeEnums::TriggerMode_On);
  _type = type;
}

void USBCamera::grab_next_image_by_trigger() {
  if (_type == USBCamera::TriggerType::SOFTWARE)
    _ptr->TriggerSoftware.Execute();
}

void USBCamera::reset_trigger() {
  _ptr->TriggerMode.SetValue(Spinnaker::TriggerModeEnums::TriggerMode_Off);
}

void USBCamera::print_device_info() {
  INodeMap& nmap = _ptr->GetTLDeviceNodeMap();

  std::cout << std::endl
            << "*** DEVICE INFORMATION ***" << std::endl
            << std::endl;

  FeatureList_t features;
  CCategoryPtr category = nmap.GetNode("DeviceInformation");
  if (IsAvailable(category) && IsReadable(category)) {
    category->GetFeatures(features);

    FeatureList_t::const_iterator it;
    for (it = features.begin(); it != features.end(); ++it) {
      CNodePtr feature_node = *it;
      std::cout << feature_node->GetName() << " : ";
      CValuePtr value = (CValuePtr)feature_node;
      std::cout << (IsReadable(value) ? value->ToString()
                                      : "Node not readable");
      std::cout << std::endl;
    }
  } else {
    std::cout << "Device control information not available." << std::endl;
  }
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
