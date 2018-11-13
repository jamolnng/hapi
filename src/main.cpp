#include <algorithm>
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
    {"output", "."},           {"trigger_type", "1"}, {"arm_pin", "26"},
    {"delay_pin0", "7"},       {"delay_pin1", "0"},   {"delay_pin2", "1"},
    {"delay_pin3", "2"},       {"exp_pin0", "13"},    {"exp_pin1", "6"},
    {"exp_pin2", "14"},        {"exp_pin3", "10"},    {"pulse_pin0", "24"},
    {"pulse_pin1", "27"},      {"pulse_pin2", "25"},  {"pulse_pin3", "28"},
    {"pulse_pin4", "29"},      {"done_pin", "23"},    {"delay", "0b1000"},
    {"exp", "0b0010"},         {"pulse", "0b11111"},  {"image_type", "png"},
    {"pmt_threshold", "0x10"}, {"pmt_gain", "0xFF"}};

// Returns true if this program is running with root permissions
bool is_root() { return getuid() == 0 && geteuid() == 0; }

// bool that states whether the program should remain running
volatile std::atomic<bool> running{true};

void signal_handler(int sig) { running = false; }

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
  return set_sh(SIGINT) && set_sh(SIGQUIT) && set_sh(SIGABRT);
}

void initialize_board(Config &config) {
  Logger &log = Logger::instance();
  // initialize the board
  log.info() << "Initializing the HAPI-E board." << std::endl;
  Board &board = Board::instance();
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

  log.info() << "Setting PMT gain and threshold." << std::endl;
  board.set_pmt_gain(config.get_int("pmt_gain"));
  board.set_pmt_threshold(config.get_int("pmt_threshold"));

  log.info() << "Resetting board." << std::endl;
  board.reset();
}

void cleanup(Spinnaker::CameraList &clist, Spinnaker::SystemPtr &system,
             USBCamera *camera) {
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
    delete camera;
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

void initialize_camera(USBCamera *camera, Config &config) {
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

  float gain = 47.994267f;  // 1.0 <= gain <= 47.994267
  log.info() << "Setting gain to " << gain << " dB." << std::endl;
  camera->set_gain(gain);

  log.info() << "Device info:" << std::endl;
  try {
    for (auto i : camera->get_device_info())
      log.info() << i.first << " : " << i.second << std::endl;
  } catch (const std::exception &ex) {
    log.exception(ex) << "Failed to print device info." << std::endl;
  }

  // configure trigger
  log.info() << "Configuring trigger." << std::endl;
  USBCamera::TriggerType trigger_type = USBCamera::TriggerType::SOFTWARE;
  if (config["trigger_type"] == "1") {
    trigger_type = USBCamera::TriggerType::HARDWARE;
    log.info() << "Using hardware trigger." << std::endl;
  } else {
    log.info() << "Using software trigger." << std::endl;
  }
  camera->configure_trigger(trigger_type);

  log.info() << "Setting acquisition mode to continuous." << std::endl;
  camera->set_acquisition_mode(
      Spinnaker::AcquisitionModeEnums::AcquisitionMode_Continuous);
}

void acquisition_loop(USBCamera *camera, std::filesystem::path &out_dir,
                      std::string &image_type) {
  Board &board = Board::instance();
  Logger &log = Logger::instance();

  // begin acquisition
  log.info() << "Beginning acquisition." << std::endl;
  camera->begin_acquisition();

  // arm the board so it is ready to acquire images
  log.info() << "Arming the HAPI-E board." << std::endl;
  board.arm();

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
      Spinnaker::ImagePtr result = camera->acquire_image();
      if (result->IsIncomplete()) {
        log.info() << "Image incomplete with status "
                   << result->GetImageStatus() << "." << std::endl;
      } else {
        if (image_count == 0) {
          if (!std::filesystem::exists(out_dir)) {
            log.info() << "First image. Creating output directory."
                       << std::endl;
            // creates out dir and thumbnail dir in one command
            std::filesystem::create_directories(out_dir / "thumbs");
          }
        }
        // save the image
        std::filesystem::path fname = out_dir;
        fname /= image_time + "." + image_type;
        log.info() << "Converting image to mono 8 bit with no color processing."
                   << std::endl;
        Spinnaker::ImagePtr converted = result->Convert(
            Spinnaker::PixelFormat_Mono8, Spinnaker::NO_COLOR_PROCESSING);
        log.info() << "Saving image (" << image_count++ << ") " << fname << "."
                   << std::endl;
        converted->Save(fname.c_str());
        std::filesystem::path thumb = out_dir / "thumbs";
        thumb /= image_time + "." + image_type;
        log.info() << "Creating thumbnail image." << std::endl;
        std::system(("sudo convert " + fname.string() + " -resize 600 " +
                     thumb.string() + " &")
                        .c_str());
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
  camera->end_acquisition();
}

Config get_config() {
  Logger &log = Logger::instance();
  // load config
  Config config(config_defaults);
  std::filesystem::path config_path = "/opt/hapi/hapi.conf";
  log.info() << "Loading config from " << config_path << "." << std::endl;
  try {
    if (std::filesystem::exists(config_path))
      config.load(config_path);
    else
      log.warning() << "Config file not found. Using defaults." << std::endl;
  } catch (const std::exception &ex) {
    log.exception(ex) << "Failed to load config. Using defaults." << std::endl;
    config = Config(config_defaults);
  }
  log.info() << "Config:" << std::endl;
  for (auto const &item : config.items())
    log.info() << "    " << item.first << ": " << item.second << std::endl;
  return config;
}

std::string get_image_type(Config &config) {
  Logger &log = Logger::instance();
  // get the image type from the config. default to png
  log.info() << "Loading image type from the config." << std::endl;
  std::string image_type;
  try {
    image_type = config["image_type"];
    std::transform(image_type.begin(), image_type.end(), image_type.begin(),
                   ::tolower);
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
  return image_type;
}

std::filesystem::path get_out_dir(std::string &start_time, Config &config) {
  Logger &log = Logger::instance();
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
  return out_dir;
}

int main(int argc, char *argv[]) {
  std::string start_time = str_time();

  Logger &log = Logger::instance();
  log.set_stream(std::cout);

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
    initialize_board(config);
  } catch (const std::exception &ex) {
    log.exception(ex) << "Failed to initialize the HAPI-E board." << std::endl;
    log.critical() << "Exiting (-1)..." << std::endl;
    return -1;
  }

  log.info() << "Initializing Spinnaker system." << std::endl;
  Spinnaker::SystemPtr system = Spinnaker::System::GetInstance();
  log.info() << "Getting list of cameras." << std::endl;
  Spinnaker::CameraList clist = system->GetCameras();

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
    cleanup(clist, system, nullptr);
    log.critical() << "Exiting (-1)..." << std::endl;
    return -1;
  }
  log.info() << "Number of cameras detected " << clist.GetSize() << "."
             << std::endl;

  // initialize the camera
  log.info() << "Getting camera object." << std::endl;
  USBCamera *camera = new USBCamera(clist.GetByIndex(0));

  try {
    initialize_camera(camera, config);
    acquisition_loop(camera, out_dir, image_type);
  } catch (const std::exception &ex) {
    log.exception(ex) << std::endl;
    cleanup(clist, system, camera);
    return -1;
  }

  cleanup(clist, system, camera);
  log.info() << "Exiting (0)..." << std::endl;
  return 0;
}
