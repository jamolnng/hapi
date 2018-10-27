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
    {"done_pin", "23"},          {"pulse_pin4", "29"},  {"delay", "0b1000"},
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
  if (std::filesystem::exists("/opt/hapi/hapi.conf"))
    config.load("/opt/hapi/hapi.conf");

  // initialize the camera
  std::shared_ptr<USBCamera> camera;
  try {
    // initialize spinnaker system
    USBCamera::init_sys();
    // if no cameras are detected attempt to refresh a max of 5 times
    for (unsigned int i = 0; i < 5 && USBCamera::num_cams() == 0; i++) {
      std::this_thread::sleep_for(std::chrono::seconds(1));
      USBCamera::update_cameras();
    }
    std::cout << "Number of cameras detected: " << USBCamera::num_cams()
              << std::endl;
    // if no cameras detected raise an error and exit
    if (USBCamera::num_cams() == 0)
      throw std::runtime_error("No cameras detected!");

    // get the camera
    camera = USBCamera::get(0);
    camera->init();
    // ensure camera is initialized
    while (!camera->is_initialized()) std::this_thread::yield();
  } catch (const std::exception &ex) {
    print_ex(ex);
    USBCamera::cleanup();
    std::cout << "Exiting..." << std::endl;
    return -1;
  }

  camera->print_device_info();

  // initialize the board
  shared_ptr<Board> board;
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
    print_ex(ex);
    USBCamera::cleanup();
    Board::cleanup();
    std::cout << "Exiting..." << std::endl;
    return -1;
  }

  // create output directory where images are stored
  std::filesystem::path out_dir;
  try {
    out_dir = std::filesystem::path(config["output"]);
    out_dir /= start_time;
    if (!std::filesystem::exists(out_dir))
      std::filesystem::create_directory(out_dir);
  } catch (const std::exception &ex) {
    print_ex(ex);
    USBCamera::cleanup();
    Board::cleanup();
    std::cout << "Exiting..." << std::endl;
  }

  try {
    // configure trigger
    USBCamera::TriggerType trigger_type = USBCamera::TriggerType::SOFTWARE;
    if (config["trigger_type"] == "1")
      trigger_type = USBCamera::TriggerType::HARDWARE;
    camera->configure_trigger(trigger_type);

    // begin acquisition
    camera->set_aquisition_mode("Continuous");
    camera->begin_acquisition();

    board->arm();
    while (running) {
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
      ImagePtr result = camera->acquire_image();
      // get the time the image was taken
      std::string image_time = str_time();
      if (!result->IsIncomplete()) {
        ImagePtr converted = result->Convert(PixelFormat_Mono8, HQ_LINEAR);
        // save the image
        std::filesystem::path fname = out_dir;
        fname /= image_time + "." + config["image_type"];
        converted->Save(fname.c_str());
      }
      result->Release();
      // wait for image to be freed before we arm
      while (result->IsInUse()) std::this_thread::yield();
      board->arm();
      // sleep for 2 board clock cycles so the board can reset
      std::this_thread::sleep_for(std::chrono::nanoseconds(20));
    }

    camera->end_acquisition();
    camera->reset_trigger();
    camera->deinit();
  } catch (const std::exception &ex) {
    print_ex(ex);
    USBCamera::cleanup();
    Board::cleanup();
    return -1;
  }

  USBCamera::cleanup();
  Board::cleanup();

  return 0;
}