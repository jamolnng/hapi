#include <algorithm>
#include <atomic>
#include <chrono>
#include <csignal>
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

#include "Spinnaker.h"

#include "argparse.h"
#include "board.h"
#include "config.h"
#include "logger.h"
#include "routines/acquisition.h"
#include "routines/get_config.h"
#include "routines/is_root.h"
#include "routines/pmt_calibrate.h"
#include "routines/str_utils.h"
#include "usb_camera.h"

using namespace hapi;

// bool that states whether the program should remain running
extern volatile std::atomic<bool> running{true};

void signal_handler(int sig);
// sets usb filesystem memory to 1000 megabytes
bool set_usbfs_mb();
bool initialize_signal_handlers();
void initialize_board(Config &config);
// resets board and frees spinnaker system
void cleanup(Spinnaker::CameraList &clist, Spinnaker::SystemPtr &system,
             std::shared_ptr<USBCamera> &camera);
void initialize_camera(std::shared_ptr<USBCamera> &camera, Config &config);

int main(int argc, char *argv[]) {
  std::string start_time = str_time();

  Logger &log = Logger::instance();
  log.set_stream(std::cout);

  ArgumentParser parser("HAPI");
  parser.add_argument("--mode",
                      "Sets the mode (trigger, interval, test). Trigger=use "
                      "pmt trigger, interval=take image at set interval, "
                      "test=test pmt trigger");
  parser.add_argument("-m",
                      "Sets the mode (trigger, interval, test). Trigger=use "
                      "pmt trigger, interval=take image at set interval, "
                      "test=test pmt trigger");
  parser.add_argument("-c",
                      "-c [interval ms] Runs the PMT calibration code with the "
                      "given test interval.");
  parser.add_argument("--calibrate",
                      "--calibrate [interval ms] Runs the PMT calibration code "
                      "with the given test interval.");
  try {
    parser.parse(argc, argv);
  } catch (const ArgumentParser::ArgumentNotFound &ex) {
    log.exception(ex) << "Failed to parse command line arguments." << std::endl;
    return -1;
  }

  std::string mode_str = "trigger";
  if (parser.exists("mode"))
    mode_str = parser.get<std::string>("mode");
  else if (parser.exists("m"))
    mode_str = parser.get<std::string>("m");
  lower(mode_str);

  HAPIMode mode = HAPIMode::TRIGGER;
  if (mode_str == "trigger")
    mode = HAPIMode::TRIGGER;
  else if (mode_str == "interval")
    mode = HAPIMode::INTERVAL;
  else if (mode_str == "test")
    mode = HAPIMode::TRIGGER_TEST;

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

  if (parser.exists("c") || parser.exists("calibrate")) {
    try {
      long long ms;
      if (parser.exists("c"))
        ms = parser.get<long long>("c");
      else
        ms = parser.get<long long>("calibrate");
      auto vals = pmt_calibrate(ms);
      auto gain = vals.first;
      auto threshold = vals.second;
      log.info() << "Calibration success!" << std::endl;
      config["pmt_gain"] = std::to_string(gain);
      config["pmt_threshold"] = std::to_string(threshold);
      log.info() << std::hex << "Gain: " << gain << std::endl;
      log.info() << std::hex << "Threshold: " << threshold << std::endl;
    } catch (const PMTCalibrationError &ex) {
      log.exception(ex) << "Failed to calibrate the PMT." << std::endl;
      log.error() << "Exiting (-1)..." << std::endl;
      return -1;
    }
  }

  try {
    initialize_board(config);
  } catch (const std::exception &ex) {
    log.exception(ex) << "Failed to initialize the HAPI-E board." << std::endl;
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
      cleanup(clist, system, {nullptr});
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
      cleanup(clist, system, camera);
      return -1;
    }
  }

  std::chrono::milliseconds interval_time =
      std::chrono::milliseconds(config.get<int>("interval"));

  try {
    acquisition_loop(camera, out_dir, image_type, interval_time, mode);
  } catch (const std::exception &ex) {
    log.exception(ex) << std::endl;
    cleanup(clist, system, camera);
    return -1;
  }

  cleanup(clist, system, camera);
  log.info() << "Exiting (0)..." << std::endl;
  return 0;
}

void signal_handler(int sig) { running = false; }

bool set_usbfs_mb() {
  Logger &log = Logger::instance();
  // set usbfs memory
  log.info() << "Setting usbfs memory to 1000mb." << std::endl;
  std::system(
      "sudo sh -c 'echo 1000 > "
      "/sys/module/usbcore/parameters/usbfs_memory_mb'");
  std::ifstream in("/sys/module/usbcore/parameters/usbfs_memory_mb",
                   std::ios::binary);
  unsigned int mb = 0;
  if (in) {
    in >> mb;
    in.close();
  }
  return mb == 1000;
}

bool initialize_signal_handlers() {
  // set signal handler
  Logger &log = Logger::instance();
  auto set_sh = [&](int sig) -> bool {
    log.info() << "Registering signal handler for signal " << sig << "."
               << std::endl;
    if (std::signal(sig, signal_handler) == SIG_ERR) {
      log.critical() << "Failed to set signal handler for signal " << sig
                     << std::endl;
      return false;
    }
    return true;
  };
  // register signal handlers
  return set_sh(SIGINT) &&
#ifdef SIGQUIT
#define set_sh(SIGQUIT) &&
#endif
         set_sh(SIGABRT);
}

void initialize_board(Config &config) {
  Logger &log = Logger::instance();
  // initialize the board
  log.info() << "Initializing the HAPI-E board." << std::endl;
  Board &board = Board::instance();
  log.info() << "Setting delay, exposure, and pulse width." << std::endl;
  board.set_delay(config.get<unsigned int>("delay"));
  board.set_exp(config.get<unsigned int>("exp"));
  board.set_pulse(config.get<unsigned int>("pulse"));

  log.info() << "Setting PMT gain and threshold." << std::endl;
  board.set_pmt_gain(config.get<int>("pmt_gain"));
  board.set_pmt_threshold(config.get<int>("pmt_threshold"));

  Board::TriggerSource source =
      static_cast<Board::TriggerSource>(config.get<int>("trigger_source"));

  board.set_trigger_source(source);

  if (source == Board::TriggerSource::PMT)
    log.info() << "Using PMT as trigger source." << std::endl;
  else
    log.info() << "Using PI as trigger source." << std::endl;

  log.info() << "Resetting board." << std::endl;
  board.reset();
}

void cleanup(Spinnaker::CameraList &clist, Spinnaker::SystemPtr &system,
             std::shared_ptr<USBCamera> &camera) {
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

void initialize_camera(std::shared_ptr<USBCamera> &camera, Config &config) {
  Logger &log = Logger::instance();
  log.info() << "Initializing camera." << std::endl;
  camera->init();
  // wait until camera is initialized
  while (!camera->is_initialized()) std::this_thread::yield();

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
  if (gain < 1.0f || gain > 47.994267f)
    throw std::out_of_range("Gain must be between 1.0 and 47.994267");

  log.info() << "Setting gain to " << gain << " dB." << std::endl;
  camera->set_gain(gain);

  log.info() << "Device info:" << std::endl;
  try {
    for (auto i : camera->get_device_info())
      log.info() << i.first << ": " << i.second << std::endl;
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
