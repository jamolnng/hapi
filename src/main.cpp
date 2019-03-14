#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>

#include <unistd.h>

#include "Spinnaker.h"

#include "argparse.h"
#include "board.h"
#include "config.h"
#include "logger.h"
#include "obis.h"
#include "routines/acquisition.h"
#include "routines/get_config.h"
#include "routines/os_utils.h"
#include "routines/pmt_calibrate.h"
#include "routines/str_utils.h"
#include "usb_camera.h"

using namespace hapi;

void initialize_board(Config &config, HAPIMode mode);
void initialize_camera(std::shared_ptr<USBCamera> &camera, Config &config);
void initialize_laser(OBISLaser &laser);
// resets board and frees spinnaker system
void cleanup(Spinnaker::CameraList &clist, Spinnaker::SystemPtr &system,
             std::shared_ptr<USBCamera> &camera, HAPIMode mode,
             OBISLaser &laser);

int main(int argc, char *argv[]) {
  std::string start_time = str_time();

  Logger &log = Logger::instance();
  log.set_stream(std::cout);

  ArgumentParser parser("HAPI");
  parser.add_argument("-m", "--mode",
                      "Sets the mode (trigger, interval, test). trigger=use "
                      "pmt trigger, interval=take image at set interval, "
                      "test=test pmt trigger",
                      false);
  parser.add_argument("-c", "--calibrate",
                      "--calibrate [interval ms] Runs the PMT calibration code "
                      "with the given test interval.",
                      false);
  try {
    parser.parse(argc, argv);
  } catch (const ArgumentParser::ArgumentNotFound &ex) {
    log.exception(ex) << "Failed to parse command line arguments." << std::endl;
    return -1;
  }
  if (parser.is_help()) {
    return 0;
  }

  std::string mode_str = "trigger";
  if (parser.exists("m")) {
    mode_str = parser.get<std::string>("m");
  }
  lower(mode_str);

  HAPIMode mode = HAPIMode::TRIGGER;
  if (mode_str == "trigger") {
    mode = HAPIMode::TRIGGER;
  } else if (mode_str == "interval") {
    mode = HAPIMode::INTERVAL;
  } else if (mode_str == "test") {
    mode = HAPIMode::TRIGGER_TEST;
  } else {
    log.critical() << "Unknown mode: " << mode_str
                   << ". Options are trigger, interval, test." << std::endl;
    log.critical() << "Exiting (-1)..." << std::endl;
    return -1;
  }

  // require sudo permissions to access hardware
  if (!is_root()) {
    log.critical() << "Root permissions required to run." << std::endl;
    log.critical() << "Exiting (-1)..." << std::endl;
    return -1;
  }

  if (!set_usbfs_mb()) {
    log.critical() << "Failed to set usbfs memory." << std::endl;
    log.critical() << "Exiting (-1)..." << std::endl;
    return -1;
  }

  if (!initialize_signal_handlers()) {
    log.critical() << "Exiting (-1)..." << std::endl;
    return -1;
  }

  Config config = get_config();
  std::string image_type = get_image_type(config);
  std::filesystem::path out_dir = get_out_dir(start_time, config);

  try {
    initialize_board(config, mode);
  } catch (const std::exception &ex) {
    log.exception(ex) << "Failed to initialize the HAPI-E board." << std::endl;
    log.critical() << "Exiting (-1)..." << std::endl;
    return -1;
  }

  if (parser.exists("c")) {
    try {
      long long ms = 5000;
      try {
        ms = parser.get<long long>("c");
      } catch (const std::exception &ex) {
        if (parser.get<std::string>("c").size() > 0) {
          log.exception(ex)
              << "Failed to get interval time. Defaulting to 5000 ms."
              << std::endl;
        }
      }
      log.info() << "Calibrating with interval: " << ms << " milliseconds."
                 << std::endl;
      auto vals = pmt_calibrate(ms);
      auto gain = vals.first;
      auto threshold = vals.second;
      log.info() << "Calibration success!" << std::endl;
      Board &board = Board::instance();
      board.set_pmt_gain(gain);
      board.set_pmt_threshold(threshold);
      log.info() << std::hex << "Gain:      " << gain << std::endl;
      log.info() << std::hex << "Threshold: " << threshold << std::endl;
    } catch (const PMTCalibrationError &ex) {
      log.exception(ex) << "Failed to calibrate the PMT." << std::endl;
      log.critical() << "Exiting (-1)..." << std::endl;
      return -1;
    }
  }

  log.info() << "Initializing laser." << std::endl;
  std::string device = "/dev/" + hapi::exec("ls /dev | grep ttyACM");
  OBISLaser laser(device);
  try {
    initialize_laser(laser);
  } catch (const std::exception &ex) {
    log.exception(ex) << "Failed to initialize the laser." << std::endl;
    log.critical() << "Exiting (-1)..." << std::endl;
    return -1;
  }

  Spinnaker::SystemPtr system;
  Spinnaker::CameraList clist;
  std::shared_ptr<USBCamera> camera;
  if (mode != HAPIMode::TRIGGER_TEST) {
    log.info() << "Initializing Spinnaker system." << std::endl;
    system = Spinnaker::System::GetInstance();
    log.info() << "Getting list of cameras." << std::endl;
    clist = system->GetCameras();

    if (clist.GetSize() == 0) {
      // attempt to refresh cameras 5 times if none detected initially
      log.error() << "No cameras detected." << std::endl;
      log.info() << "Attempting to refresh camera list." << std::endl;
      for (unsigned int i = 0; i < 5 && clist.GetSize() == 0; i++) {
        log.info() << "Refreshing..." << std::endl;
        system->UpdateCameras();
        clist = system->GetCameras();
        std::this_thread::sleep_for(std::chrono::seconds(1));
      }
    }

    // if no cameras detected exit
    if (clist.GetSize() == 0) {
      log.critical() << "No cameras detected." << std::endl;
      cleanup(clist, system, camera, mode, laser);
      log.critical() << "Exiting (-1)..." << std::endl;
      return -1;
    }
    log.info() << "Number of cameras detected " << clist.GetSize() << "."
               << std::endl;

    // initialize the camera
    log.info() << "Getting camera object." << std::endl;
    camera = std::make_shared<USBCamera>(clist.GetByIndex(0));

    try {
      initialize_camera(camera, config);
    } catch (const std::exception &ex) {
      log.exception(ex) << std::endl;
      cleanup(clist, system, camera, mode, laser);
      log.critical() << "Exiting (-1)..." << std::endl;
      return -1;
    }
  }

  std::chrono::milliseconds interval_time(config.get<unsigned int>("interval"));

  try {
    acquisition_loop(camera, laser, out_dir, image_type, interval_time, mode);
  } catch (const std::exception &ex) {
    log.exception(ex) << std::endl;
    cleanup(clist, system, camera, mode, laser);
    log.critical() << "Exiting (-1)..." << std::endl;
    return -1;
  }

  cleanup(clist, system, camera, mode, laser);
  log.info() << "Exiting (0)..." << std::endl;
  return 0;
}

void initialize_board(Config &config, HAPIMode mode) {
  Logger &log = Logger::instance();
  // initialize the board
  log.info() << "Initializing the HAPI-E board." << std::endl;
  Board &board = Board::instance();
  log.info() << "Setting delay, exposure, and pulse width." << std::endl;
  board.set_delay(config.get<unsigned int>("delay"));
  board.set_exp(config.get<unsigned int>("exp"));
  board.set_pulse(config.get<unsigned int>("pulse"));

  log.info() << "Setting PMT gain and threshold." << std::endl;
  board.set_pmt_gain(config.get<unsigned int>("pmt_gain"));
  board.set_pmt_threshold(config.get<unsigned int>("pmt_threshold"));

  if (mode == HAPIMode::INTERVAL) {
    board.set_trigger_source(Board::TriggerSource::PI);
    log.info() << "Using PI as trigger source." << std::endl;
  } else {
    board.set_trigger_source(Board::TriggerSource::PMT);
    log.info() << "Using PMT as trigger source." << std::endl;
  }

  log.info() << "Resetting board." << std::endl;
  board.reset();
}

void cleanup(Spinnaker::CameraList &clist, Spinnaker::SystemPtr &system,
             std::shared_ptr<USBCamera> &camera, HAPIMode mode,
             OBISLaser &laser) {
  laser.state(OBISLaser::State::Off);
  Board &board = Board::instance();
  Logger &log = Logger::instance();
  log.info() << "Cleaning up..." << std::endl;
  if (camera != nullptr) {
    if (camera->is_initialized()) {
      log.info() << "Resetting camera trigger." << std::endl;
      try {
        camera->reset_trigger();
      } catch (const std::exception &ex) {
        log.exception(ex) << "Failed to reset camera trigger." << std::endl;
      }
      log.info() << "De-initializing camera." << std::endl;
      try {
        camera->deinit();
      } catch (const std::exception &ex) {
        log.exception(ex) << "Failed to de-initialize camera." << std::endl;
      }
    }
    log.info() << "Releasing camera." << std::endl;
    camera.reset();
  }
  log.info() << "Disarming HAPI-E board." << std::endl;
  board.disarm();
  if (mode != HAPIMode::TRIGGER_TEST) {
    log.info() << "Clearing camera list." << std::endl;
    try {
      clist.Clear();
    } catch (const std::exception &ex) {
      log.exception(ex) << "Failed to clear camera list." << std::endl;
    }
    log.info() << "Releasing Spinnaker system." << std::endl;
    try {
      system->ReleaseInstance();
    } catch (const std::exception &ex) {
      log.exception(ex) << "Failed to release Spinnaker system." << std::endl;
    }
  }
}

void initialize_camera(std::shared_ptr<USBCamera> &camera, Config &config) {
  Logger &log = Logger::instance();
  log.info() << "Initializing camera." << std::endl;
  camera->init();
  // wait until camera is initialized
  while (!camera->is_initialized()) {
    std::this_thread::yield();
  }

  log.info() << "Disabling auto exposure." << std::endl;
  camera->set_auto_exposure(Spinnaker::ExposureAutoEnums::ExposureAuto_Off);

  log.info() << "Setting exposure mode to timed." << std::endl;
  camera->set_exposure_mode(Spinnaker::ExposureModeEnums::ExposureMode_Timed);

  log.info() << "Setting camera exposure time to 20,000 microseconds."
             << std::endl;
  camera->set_exposure(20000);

  log.info() << "Disabling auto gain." << std::endl;
  camera->set_auto_gain(Spinnaker::GainAutoEnums::GainAuto_Off);

  float gain = config.get<float>("camera_gain");  // 1.0 <= gain <= 47.994267
  if (gain < 1.0f || gain > 47.994267f) {
    throw std::out_of_range("Gain must be between 1.0 and 47.994267");
  }

  log.info() << "Setting gain to " << gain << " dB." << std::endl;
  camera->set_gain(gain);

  log.info() << "Camera info:" << std::endl;
  try {
    for (auto i : camera->get_device_info()) {
      log.info() << "    " << i.first << ": " << i.second << std::endl;
    }
  } catch (const std::exception &ex) {
    log.exception(ex) << "Failed to print device info." << std::endl;
  }

  // configure trigger
  log.info() << "Configuring trigger." << std::endl;
  USBCamera::TriggerType trigger_type = USBCamera::TriggerType::SOFTWARE;
  if (config["camera_trigger"] == "1") {
    trigger_type = USBCamera::TriggerType::HARDWARE;
    log.info() << "Camera using hardware trigger." << std::endl;
  } else {
    log.info() << "Camera using software trigger." << std::endl;
  }
  camera->configure_trigger(trigger_type);

  log.info() << "Setting acquisition mode to continuous." << std::endl;
  camera->set_acquisition_mode(
      Spinnaker::AcquisitionModeEnums::AcquisitionMode_Continuous);
}

void initialize_laser(OBISLaser &laser) {
  Logger &log = Logger::instance();
  laser.handshake(OBISLaser::State::Off);
  laser.cdrh(OBISLaser::State::Off);
  laser.mode(OBISLaser::SourceType::Digital);
  laser.auto_start(OBISLaser::State::On);
  laser.state(OBISLaser::State::On);

  log.info() << "Laser info:" << std::endl;
  log.info() << "    IDN: " << laser.sys_info()._idn;
  log.info() << "    Model: " << laser.sys_info()._model;
  log.info() << "    Serial Number: " << laser.sys_info()._snumber;
  log.info() << "    Firmware: " << laser.sys_info()._firmware;
  log.info() << "    Wavelength: " << laser.sys_info()._wavelength << std::endl;
  log.info() << "    Laser cycles: " << laser.cycles() << std::endl;
  log.info() << "    Laser hours: " << laser.hours() << std::endl;
  log.info() << "    Laser diode hours: " << laser.diode_hours() << std::endl;

  FaultCode fault = laser.fault();
  if (fault != 0) {
    for (auto f : laser.fault_bits(fault)) {
      log.error() << "Laser fault: " << laser.fault_str(f) << std::endl;
    }
    throw std::runtime_error("Laser fault");
  }
}
