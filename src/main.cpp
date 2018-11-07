#include <atomic>
#include <csignal>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>

#include <sys/types.h>
#include <unistd.h>

#include "Spinnaker.h"

#include "board.h"
#include "config.h"
#include "logger.h"
#include "usb_camera.h"

using namespace hapi;

// default configuration parameters
std::map<std::string, std::string> config_defaults = {
    {"output", "."},      {"trigger_type", "1"}, {"arm_pin", "26"},
    {"delay_pin0", "7"},  {"delay_pin1", "0"},   {"delay_pin2", "1"},
    {"delay_pin3", "2"},  {"exp_pin0", "13"},    {"exp_pin1", "6"},
    {"exp_pin2", "14"},   {"exp_pin3", "10"},    {"pulse_pin0", "24"},
    {"pulse_pin1", "27"}, {"pulse_pin2", "25"},  {"pulse_pin3", "28"},
    {"pulse_pin4", "29"}, {"done_pin", "23"},    {"delay", "0b1000"},
    {"exp", "0b0010"},    {"pulse", "0b11111"},  {"image_type", "png"}};

// Returns true if this program is running with root permissions
bool is_root() { return getuid() == 0 && geteuid() == 0; }

// Returns a std::string of the current time in the format YYYY_MM_DD-HH_MM_SS
std::string str_time() {
  std::time_t t = std::time(nullptr);
  char mbstr[100];
  std::string str;
  static unsigned int unknown_time = 0;
  if (std::strftime(mbstr, sizeof(mbstr), "%Y_%m_%d-%H_%M_%S", std::gmtime(&t)))
    str = std::string(mbstr);
  else
    str = "unknown" + std::to_string(unknown_time++);
  return str;
}

int main(int argc, char *argv[]) {
  std::string start_time = str_time();

  Logger &log = Logger::instance();
  log.set_stream(std::cout);

  // require sudo permissions to access hardware
  if (!is_root()) {
    log.critical() << "Root permissions required to run." << std::endl
                   << "Exiting (-1)..." << std::endl;
    return -1;
  }

  // set usbfs memory
  log.info() << "Setting usbfs memory to 1000mb... "
             << std::system(
                    "sudo sh -c 'echo 1000 > "
                    "/sys/module/usbcore/parameters/usbfs_memory_mb'")
             << std::endl;

  // bool that states whether the program should remain running
  volatile std::atomic<bool> running{true};

  auto set_sh = [&](int sig) {
    log.info() << "Registering signal handler for signal " << sig << std::endl;
    if (std::signal(sig, [&running](int) { running = false; }) == SIG_ERR) {
      log.critical() << "Failed to set signal handler for signal " << sig
                     << std::endl
                     << "Exiting (-1)..." << std::endl;
      exit(-1);
    }
  };

  // register signal handlers
  set_sh(SIGINT);
  set_sh(SIGQUIT);
  set_sh(SIGABRT);
  set_sh(SIGKILL);

  // load config
  Config config = Config(config_defaults);
  std::filesystem::path config_path = "/opt/hapi/hapi.conf";
  log.info() << "Loading config from " << config_path << std::endl;
  try {
    if (std::filesystem::exists(config_path))
      config.load(config_path);
    else
      log.warning() << "Config file not found. Using defaults." << std::endl;
  } catch (const std::exception &ex) {
    log.exception(ex) << "Failed to load config. Using defaults." << std::endl;
  }
  log.info() << "Config:" << std::endl;
  for (auto const &item : config.items())
    log.info() << item.first << ": " << item.second << std::endl;

  // get the image type from the config. default to png
  log.info() << "Setting the image type from the config." << std::endl;
  std::string image_type;
  try {
    image_type = config["image_type"];
    // png, ppm, pgm, tiff, jpeg, jpg, bmp
    if (image_type != "png" && image_type != "ppm" && image_type != "pgm" &&
        image_type != "tiff" && image_type != "jpeg" && image_type != "jpg" &&
        image_type != "bmp") {
      log.warning() << "Unsupported image type given: " << image_type
                    << ". Defaulting to png." << std::endl;
      image_type = "png";
    }
  } catch (const std::exception &ex) {
    log.exception(ex)
        << "Failed to load image type from config. Defaulting to png."
        << std::endl;
    image_type = "png";
  }
  log.info() << "Image type set to " << image_type << std::endl;

  // initialize the board
  log.info() << "Initializing the HAPI-E board." << std::endl;
  Board &board = Board::instance();
  try {
    log.info() << "Configuring I/O." << std::endl;
    board.set_arm_pin(config.get_int("arm_pin"));
    board.set_done_pin(config.get_int("done_pin"));

    board.set_delay_pins(
        config.get_int("delay_pin0"), config.get_int("delay_pin1"),
        config.get_int("delay_pin2"), config.get_int("delay_pin3"));

    board.set_exp_pins(config.get_int("exp_pin0"), config.get_int("exp_pin1"),
                       config.get_int("exp_pin2"), config.get_int("exp_pin3"));

    board.set_pulse_pins(
        config.get_int("pulse_pin0"), config.get_int("pulse_pin1"),
        config.get_int("pulse_pin2"), config.get_int("pulse_pin3"),
        config.get_int("pulse_pin4"));

    log.info() << "Setting delay, exposure, and pulse width." << std::endl;
    board.set_delay(config.get_int("delay"));
    board.set_exp(config.get_int("exp"));
    board.set_pulse(config.get_int("pulse"));

    log.info() << "Resetting board." << std::endl;
    board.reset();
  } catch (const std::exception &ex) {
    log.exception(ex) << "Failed to initialize the HAPI-E board." << std::endl;
    log.critical() << "Exiting (-1)..." << std::endl;
    return -1;
  }
  log.info() << "Done initializing HAPI-E board." << std::endl;

  log.info() << "Initializing Spinnaker system." << std::endl;
  Spinnaker::SystemPtr system = Spinnaker::System::GetInstance();
  log.info() << "Getting list of cameras." << std::endl;
  Spinnaker::CameraList clist = system->GetCameras();

  auto cleanup = [&board, &clist, &system]() {
    log.info() << "Cleaning up..." << std::endl;
    board.disarm();
    try {
      clist.Clear();
    } catch (const std::exception &ex) {
      log.exception(ex) << "Failed to clear camera list." << std::endl;
    }
    try {
      system->ReleaseInstance();
    } catch (const std::exception &ex) {
      log.exception(ex) << "Failed to release Spinnaker system." << std::endl;
    }
  };

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
    cleanup();
    log.critical() << "Exiting (-1)..." << std::endl;
    return -1;
  } else {
    log.info() << "Number of cameras detected " << clist.GetSize() << "."
               << std::endl;
  }

  // initialize the camera
  log.info() << "Getting camera object." << std::endl;
  USBCamera camera = USBCamera(clist.GetByIndex(0));
  try {
    log.info() << "Initializing camera." << std::endl;
    camera.init();
    // wait until camera is initialized
    while (!camera.is_initialized()) std::this_thread::yield();
  } catch (const std::exception &ex) {
    log.exception(ex) << "Failed to initialize camera." << std::endl;
    cleanup();
    log.critical() << "Exiting (-1)..." << std::endl;
    return -1;
  }
  log.info() << "Camera initialized." << std::endl;

  log.info() << "Disabling auto exposure." << std::endl;
  camera.set_auto_exposure(Spinnaker::ExposureAutoEnums::ExposureAuto_Off);
  log.info() << "Setting exposure mode to timed." << std::endl;
  camera.set_exposure_mode(Spinnaker::ExposureModeEnums::ExposureMode_Timed);
  log.info() << "Setting camera exposure time to 20,000 microseconds."
             << std::endl;
  camera.set_exposure(20000);

  double gain = 47.994267;  // 1.0 <= gain <= 47.994267
  log.info() << "Disabling auto gain." << std::endl;
  camera.set_auto_gain(Spinnaker::GainAutoEnums::GainAuto_Off);
  log.info() << "Setting gain to " << gain << " dB." << std::endl;
  camera.set_gain(gain);

  log.info() << "Device info:" << std::endl;
  try {
    camera.print_device_info();
  } catch (const std::exception &ex) {
    log.exception(ex) << "Failed to print device info." << std::endl;
  }

  std::filesystem::path out_dir;
  try {
    out_dir = std::filesystem::path(config["output"]);
  } catch (const std::exception &ex) {
    log.exception(ex) << "Failed to get output directory from config. "
                         "Defaulting to current working directory."
                      << std::endl;
    out_dir = std::filesystem::current_path();
  }
  out_dir /= start_time;
  log.info() << "Output directory set to " << out_dir << std::endl;

  // configure trigger
  log.info() << "Configuring trigger." << std::endl;
  try {
    USBCamera::TriggerType trigger_type = USBCamera::TriggerType::SOFTWARE;
    if (config["trigger_type"] == "1") {
      trigger_type = USBCamera::TriggerType::HARDWARE;
      log.info() << "Using hardware trigger." << std::endl;
    } else {
      log.info() << "Using software trigger." << std::endl;
    }
    camera.configure_trigger(trigger_type);
  } catch (const std::exception &ex) {
    log.exception(ex) << "Failed to configure trigger." << std::endl;
    cleanup();
    log.critical() << "Exiting (-1)..." << std::endl;
    return -1;
  }

  try {
    // begin acquisition
    log.info() << "Setting acquisition mode to continuous." << std::endl;
    camera.set_acquisition_mode(
        Spinnaker::AcquisitionModeEnums::AcquisitionMode_Continuous);
    log.info() << "Beginning acquisition." << std::endl;
    camera.begin_acquisition();

    // arm the board so it is ready to acquire images
    log.info() << "Arming the HAPI-E board." << std::endl;
    board.arm();

    // image count
    unsigned int image_count = 0;

    // main acquisition loop
    log.info() << "Entering acquisition loop." << std::endl;
    while (running) {
      try {
        log.info() << "Waiting for trigger." << std::endl;
        // wait for the board to signal it has taken an image
        while (!board.is_done()) {
          if (running)
            std::this_thread::yield();
          else
            // exit the program if signaled
            break;
        }
        // exit if no image was captured and the program was signaled to exit
        if (!board.is_done() && !running) {
          log.info() << "Exit requested." << std::endl;
          break;
        };
        log.info() << "Trigger recieved." << std::endl;
        // get the time the image was taken
        std::string image_time = str_time();
        // disarm the board so no other images can be captured while we process
        // the current one
        log.info() << "Disarming the HAPI-E board." << std::endl;
        board.disarm();

        // get the image from the camera
        log.info() << "Acquiring image from camera." << std::endl;
        Spinnaker::ImagePtr result = camera.acquire_image();
        if (result->IsIncomplete()) {
          log.info() << "Image incomplete with status "
                     << result->GetImageStatus() << " "
                     << Spinnaker::ImagePtrGetImageStatusDescription(
                            result->GetImageStatus())
                     << "." << std::endl;
        } else {
          if (image_count == 0) {
            if (!std::filesystem::exists(out_dir)) {
              log.info() << "First image. Creating output directory."
                         << std::endl;
              std::filesystem::create_directory(out_dir);
            }
          }
          // save the image
          std::filesystem::path fname = out_dir;
          fname /= image_time + "." + image_type;
          log.info()
              << "Converting image to mono 8 bit with no color processing."
              << std::endl;
          Spinnaker::ImagePtr converted = result->Convert(
              Spinnaker::PixelFormat_Mono8, Spinnaker::NO_COLOR_PROCESSING);
          log.info() << "Saving image (" << image_count << ") " << fname
                     << "...";
          converted->Save(fname.c_str());
          log.info() << "Done." << std::endl;
        }
        log.info() << "Releasing image." << std::endl;
        result->Release();
        // wait for image to be freed before we arm
        while (result->IsInUse()) std::this_thread::yield();
        log.info() << "Arming HAPI-E board." << std::endl;
        board.arm();
      } catch (const std::exception &ex) {
        log.exception(ex) << "Failed to acquire image." << std::endl;
      }
    }
    log.info() << "Ending acquisition." << std::endl;
    camera.end_acquisition();
  } catch (const std::exception &ex) {
    log.exception(ex) << "Failed to run acquisition loop." << std::endl;
    cleanup();
    log.critical() << "Exiting (-1)..." << std::endl;
    return -1;
  }

  log.info() << "Resetting camera trigger." << std::endl;
  try {
    camera.reset_trigger();
  } catch (const std::exception &ex) {
    log.exception(ex) << "Failed to reset camera trigger." << std::endl;
  }

  log.info() << "De-initializing camera." << std::endl;
  try {
    camera.deinit();
  } catch (const std::exception &ex) {
    log.exception(ex) << "Failed to de-initialize camera." << std::endl;
  }
  cleanup();
  log.info() << "Exiting (0)..." << std::endl;
  return 0;
}
