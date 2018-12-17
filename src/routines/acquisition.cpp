#include "routines/acquisition.h"

#include "board.h"
#include "logger.h"
#include "routines/str_utils.h"

#include <atomic>
#include <thread>

namespace hapi {
extern volatile std::atomic<bool> running;

void acquisition_loop(std::shared_ptr<USBCamera> &camera,
                      std::filesystem::path &out_dir, std::string &image_type,
                      std::chrono::milliseconds interval_time, HAPIMode mode) {
  Board &board = Board::instance();
  Logger &log = Logger::instance();

  if (mode != HAPIMode::TRIGGER_TEST) {
    // begin acquisition
    log.info() << "Beginning acquisition." << std::endl;
    camera->begin_acquisition();  
  }

  // arm the board so it is ready to acquire images
  log.info() << "Arming the HAPI-E board." << std::endl;
  board.arm();

  unsigned int image_count = 0;

  std::chrono::high_resolution_clock::time_point current_time =
      std::chrono::high_resolution_clock::now();
  std::chrono::high_resolution_clock::time_point last_time = current_time;

  // main acquisition loop
  log.info() << "Entering acquisition loop." << std::endl;
  while (running) {
    if (mode == HAPIMode::INTERVAL) {
      log.info() << "Waiting for interval." << std::endl;
      while ((current_time - last_time) < interval_time) {
        current_time = std::chrono::high_resolution_clock::now();
        if (running)
          std::this_thread::yield();
        else
          break;
      }
      log.info() << "Sending trigger." << std::endl;
      board.trigger();
    } else {
      log.info() << "Waiting for trigger." << std::endl;
    }
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

    if (mode != HAPIMode::TRIGGER_TEST) {
      try {
        acquire_image(camera, out_dir, image_type, image_count, image_time);
        image_count++;
      } catch (const std::exception &ex) {
        log.exception(ex) << "Failed to acquire image." << std::endl;
      }
    }
    log.info() << "Arming HAPI-E board." << std::endl;
    board.arm();
  }

  if (mode != HAPIMode::TRIGGER_TEST) {
    log.info() << "Ending acquisition." << std::endl;
    camera->end_acquisition();
  }
}

void acquire_image(std::shared_ptr<USBCamera> &camera,
                   std::filesystem::path &out_dir, std::string &image_type,
                   unsigned int image_count, const std::string &image_time) {
  Logger &log = Logger::instance();
  // get the image from the camera
  log.info() << "Acquiring image from camera." << std::endl;
  Spinnaker::ImagePtr result = camera->acquire_image();
  if (result->IsIncomplete()) {
    log.info() << "Image incomplete with status " << result->GetImageStatus()
               << "." << std::endl;
  } else {
    if (image_count == 0) {
      if (!std::filesystem::exists(out_dir)) {
        log.info() << "First image. Creating output directory." << std::endl;
        // creates out dir and thumbnail dir in one command
        std::filesystem::create_directories(
            out_dir / (out_dir.stem().string() + "_thumbs"));
      }
    }
    // save the image
    std::filesystem::path fname = out_dir / (image_time + "." + image_type);
    log.info() << "Converting image to mono 8 bit with no color processing."
               << std::endl;
    Spinnaker::ImagePtr converted = result->Convert(
        Spinnaker::PixelFormat_Mono8, Spinnaker::NO_COLOR_PROCESSING);
    log.info() << "Saving image (" << image_count << ") " << fname << "."
               << std::endl;
    converted->Save(fname.string().c_str());
    std::filesystem::path thumb =
        out_dir / (out_dir.stem().string() + "_thumbs");
    thumb /= image_time + "_thumb" + "." + image_type;
    std::filesystem::path last = out_dir.parent_path() / "last.png";
    log.info() << "Creating thumbnail image." << std::endl;
    std::string convert =
        "sudo convert " + fname.string() + " -resize 600 " + thumb.string();
    std::string convert_last =
        "sudo convert " + thumb.string() + " " + last.string();
    std::string cmd = "(" + convert + " && " + convert_last + ") &";
    std::system(cmd.c_str());
  }
  log.info() << "Releasing image." << std::endl;
  result->Release();
  // wait for image to be freed before we arm
  while (result->IsInUse()) std::this_thread::yield();
}
};  // namespace hapi
