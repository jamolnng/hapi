#include <atomic>
#include <csignal>
#include <exception>
#include <filesystem>
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
#include "usb_camera.h"

using namespace hapi;

// bool that states whether the program should remain running
volatile std::atomic<bool> running = true;

// default configuration parameters
std::map<std::string, std::string> config_defaults = {
    {"output", "/home/pi/hapi"}, {"trigger_type", "1"}, {"arm_pin", "26"},
    {"delay_pin0", "7"},         {"delay_pin1", "0"},   {"delay_pin2", "1"},
    {"delay_pin3", "2"},         {"exp_pin0", "13"},    {"exp_pin1", "6"},
    {"exp_pin2", "14"},          {"exp_pin3", "10"},    {"pulse_pin0", "24"},
    {"pulse_pin1", "27"},        {"pulse_pin2", "25"},  {"pulse_pin3", "28"},
    {"pulse_pin4", "29"},        {"done_pin", "23"},    {"delay", "0b1000"},
    {"exp", "0b0010"},           {"pulse", "0b11111"},  {"image_type", "png"}};

// print an exception to the console
void print_ex(const std::exception &ex) {
  std::cout << "*** EXCEPTION ***" << std::endl
            << ex.what() << std::endl
            << std::endl;
}

// signal handler tells the program to exit
void signal_handler(int s) { running = false; }

// Returns true if this program is running with root permissions
bool has_sudo() { return getuid() == 0 && geteuid() == 0; }

// Returns a std::string of the current time in the format YYYY_MM_DD-HH_MM_SS
std::string str_time() {
  std::string str = "unknown";
  std::time_t t = std::time(nullptr);
  char mbstr[100];
  if (std::strftime(mbstr, sizeof(mbstr), "%Y_%m_%d-%H_%M_%S", std::gmtime(&t)))
    str = std::string(mbstr);
  return str;
}

int main(int argc, char *argv[]) {
  std::string start_time = str_time();

  // require sudo permissions to access hardware
  if (!has_sudo()) {
    std::cout << "root permissions required to run!" << std::endl;
    return -1;
  }

  // register signal handler
  for (int i = 0; i < 35) std::signal(i, signal_handler);

  // load config
  Config config = Config(config_defaults);
  try {
    if (std::filesystem::exists("/opt/hapi/hapi.conf"))
      config.load("/opt/hapi/hapi.conf");
  } catch (const std::exception &ex) {
    std::cout << "Failed to load config. Using defaults." << std::endl;
    print_ex(ex);
  }

  // get the image type from the config. default to png
  std::string image_type;
  try {
    image_type = config["image_type"];
  } catch (const std::exception &ex) {
    std::cout << "Failed to load image type from config. Defaulting to png"
              << std::endl;
    print_ex(ex);
    image_type = "png";
  }

  // initialize the board
  std::shared_ptr<Board> board;
  try {
    board = Board::instance();
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

    board.set_delay(config.get_int("delay"));
    board.set_exp(config.get_int("exp"));
    board.set_pulse(config.get_int("pulse"));

    board.disarm();
  } catch (const std::exception &ex) {
    std::cout << "Failed to initialize HAPI-E board." << std::endl;
    print_ex(ex);
    std::cout << "Exiting..." << std::endl;
    return -1;
  }

  Spinnaker::SystemPtr system = Spinnaker::System::GetInstance();
  Spinnaker::CameraList clist = system->GetCameras();

  auto cleanup = [&board, &clist, &system]() {
    std::cout << "Cleaning up..." << std::endl;
    board->disarm();
    clist.Clear();
    system->ReleaseInstance();
  };

  // attempt to refresh cameras 5 times if none detected initially
  std::cout << "No cameras detected. Attempting to refresh camera list."
            << std::endl;
  while (unsigned int i = 0; i < 5 && clist.GetSize() == 0; i++) {
    std::cout << "Refreshing..." << std::endl;
    system->UpdateCameras();
    clist = system->GetCameras();
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
  std::cout << "Number of cameras detected: " << clist.GetSize() << std::endl;

  // if no cameras detected exit
  if (clist.GetSize() == 0) {
    cleanup();
    std::cout << "No cameras detected." << std::endl
              << "Exiting..." << std::end;
    return -1;
  }

  // initialize the camera
  USBCamera camera = USBCamera(clist->GetByIndex(0));
  try {
    camera.init();
    // wait until camera is initialized
    while (!camera.is_initialized()) std::this_thread::yield();
  } catch (const std::exception &ex) {
    std::cout << "Failed to initialize camera." << std::endl;
    print_ex(ex);
    cleanup();
    std::cout << "Exiting..." << std::endl;
    return -1;
  }

  try {
    camera.print_device_info();
  } catch (const std::exception &ex) {
    std::cout << "Failed to get device info." << std::endl;
    print_ex(ex);
  }

  // create output directory where images are stored
  std::filesystem::path out_dir;
  try {
    out_dir = std::filesystem::path(config["output"]);
    out_dir /= start_time;
    if (!std::filesystem::exists(out_dir))
      std::filesystem::create_directory(out_dir);
  } catch (const std::exception &ex) {
    std::cout << "Failed to create output directory." << std::endl;
    print_ex(ex);
    cleanup();
    std::cout << "Exiting..." << std::endl;
    return -1;
  }

  // configure trigger
  try {
    USBCamera::TriggerType trigger_type = USBCamera::TriggerType::SOFTWARE;
    if (config["trigger_type"] == "1")
      trigger_type = USBCamera::TriggerType::HARDWARE;
    camera.configure_trigger(trigger_type);
  } catch (const std::exception &ex) {
    std::cout << "Failed to configure trigger." << std::endl;
    print_ex(ex);
    cleanup();
    std::cout << "Exiting..." << std::endl;
    return -1;
  }

  try {
    // begin acquisition
    camera.set_aquisition_mode("Continuous");
    camera.begin_acquisition();

    // arm the board so it is ready to acquire images
    board->arm();

    // main acquisition loop
    while (running) {
      try {
        // wait for the board to signal it has taken an image
        while (!board->is_done()) {
          if (running)
            std::this_thread::yield();
          else
            // exit the program if signaled
            break;
        }
        // exit if no image was captured and the program was signaled to exit
        if (!board->is_done() && !running) break;
        // disarm the board so no other images can be captured while we process
        // the current one
        board->disarm();

        // get the image from the camera
        ImagePtr result = camera.acquire_image();
        // get the time the image was taken
        std::string image_time = str_time();
        if (result->IsIncomplete()) {
          std::cout << "Image incomplete with status: "
                    << result->GetImageStatus() << std::endl;
        } else {
          ImagePtr converted = result->Convert(PixelFormat_Mono8, HQ_LINEAR);
          // save the image
          std::filesystem::path fname = out_dir;
          fname /= image_time + "." + image_type;
          converted->Save(fname.c_str());
        }
        result->Release();
        // wait for image to be freed before we arm
        while (result->IsInUse()) std::this_thread::yield();
        board->arm();
        // sleep for 2 board clock cycles so the board can reset
        std::this_thread::sleep_for(std::chrono::nanoseconds(20));
      } catch (const std::exception &ex) {
        std::cout << "Failed to acquire image." << std::endl;
        print_ex(ex);
      }
    }
    camera.end_acquisition();
  } catch (const std::exception &ex) {
    std::cout << "Failed to run acquisition loop." << std::endl;
    print_ex(ex);
    cleanup();
    return -1;
  }

  try {
    camera.reset_trigger();
  } catch (const std::exception &ex) {
    std::cout << "Failed to reset trigger." << std::endl;
    print_ex(ex);
  }

  try {
    camera.deinit();
  } catch (const std::exception &ex) {
    std::cout << "Failed to deinitialize the camera." << std::endl;
    print_ex(ex);
  }

  cleanup();

  return 0;
}
