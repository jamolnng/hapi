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
  INodeMap& nmap = _ptr->GetNodeMap();
  CEnumerationPtr trigger_mode = nmap.GetNode("TriggerMode");
  if (!IsAvailable(trigger_mode) || !IsReadable(trigger_mode))
    throw std::runtime_error(
        "configure_trigger: Unable to disable trigger mode (node retrieval).");

  CEnumEntryPtr trigger_mode_off = trigger_mode->GetEntryByName("Off");
  if (!IsAvailable(trigger_mode_off) || !IsReadable(trigger_mode_off))
    throw std::runtime_error(
        "configure_trigger: Unable to disable trigger mode (enum entry "
        "retrieval).");

  trigger_mode->SetIntValue(trigger_mode_off->GetValue());

  CEnumerationPtr trigger_source = nmap.GetNode("TriggerSource");
  if (!IsAvailable(trigger_source) || !IsReadable(trigger_source))
    throw std::runtime_error(
        "configure_trigger: Unable to set trigger mode (node retrieval).");

  if (type == USBCamera::TriggerType::SOFTWARE) {
    CEnumEntryPtr trigger_source_software =
        trigger_source->GetEntryByName("Software");
    if (!IsAvailable(trigger_source_software) ||
        !IsReadable(trigger_source_software))
      throw std::runtime_error(
          "configure_trigger: Unable to set trigger mode (enum entry "
          "retrieval).");

    trigger_source->SetIntValue(trigger_source_software->GetValue());
  } else if (type == USBCamera::TriggerType::HARDWARE) {
    CEnumEntryPtr trigger_source_hardware =
        trigger_source->GetEntryByName("Line0");
    if (!IsAvailable(trigger_source_hardware) ||
        !IsReadable(trigger_source_hardware))
      throw std::runtime_error(
          "configure_trigger: Unable to set trigger mode (enum entry "
          "retrieval).");

    trigger_source->SetIntValue(trigger_source_hardware->GetValue());
  }

  CEnumEntryPtr trigger_mode_on = trigger_mode->GetEntryByName("On");
  if (!IsAvailable(trigger_mode_on) || !IsReadable(trigger_mode_on))
    throw std::runtime_error(
        "configure_trigger: Unable to enable trigger mode (enum entry "
        "retrieval).");

  trigger_mode->SetIntValue(trigger_mode_on->GetValue());

  _type = type;
}

void USBCamera::grab_next_image_by_trigger() {
  INodeMap& nmap = _ptr->GetNodeMap();
  if (_type == USBCamera::TriggerType::SOFTWARE) {
    CCommandPtr trigger_cmd = nmap.GetNode("TriggerSoftware");
    if (!IsAvailable(trigger_cmd) || !IsWritable(trigger_cmd))
      throw std::runtime_error(
          "grab_next_image_by_trigger: Unable to execute software trigger.");

    trigger_cmd->Execute();
  }
}

void USBCamera::reset_trigger() {
  INodeMap& nmap = _ptr->GetNodeMap();
  CEnumerationPtr trigger_mode = nmap.GetNode("TriggerMode");
  if (!IsAvailable(trigger_mode) || !IsReadable(trigger_mode))
    throw std::runtime_error(
        "reset_trigger: Unable to disable trigger mode (node retrieval).");

  CEnumEntryPtr trigger_mode_off = trigger_mode->GetEntryByName("Off");
  if (!IsAvailable(trigger_mode_off) || !IsReadable(trigger_mode_off))
    throw std::runtime_error(
        "reset_trigger: Unable to disable trigger mode (enum entry "
        "retrieval).");

  trigger_mode->SetIntValue(trigger_mode_off->GetValue());
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

void USBCamera::set_acquisition_mode(const Spinnaker::GenICam::gcstring& mode) {
  INodeMap& nmap = _ptr->GetNodeMap();
  CEnumerationPtr acquisition_mode = nmap.GetNode("AcquisitionMode");
  if (!IsAvailable(acquisition_mode) || !IsWritable(acquisition_mode))
    throw std::runtime_error(
        "set_acquisition_mode: Unable to set acquisition mode to " + mode +
        " (node retrieval).");

  CEnumEntryPtr mode_entry = acquisition_mode->GetEntryByName(mode);
  if (!IsAvailable(mode_entry) || !IsReadable(mode_entry))
    throw std::runtime_error(
        "set_acquisition_mode: Unable to set acquisition mode to " + mode +
        " (entry retrieval).");

  acquisition_mode->SetIntValue(mode_entry->GetValue());
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

void USBCamera::set_exposure(unsigned int microseconds) {
  _ptr->ExposureTime.SetValue(microseconds);
}

void USBCamera::set_auto_gain(Spinnaker::GainAutoEnums a) {
  _ptr->GainAuto.SetValue(a);
}

void USBCamera::set_gain(float gain) { _ptr->Gain.SetValue(gain); }
