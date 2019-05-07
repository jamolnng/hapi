#include "routines/acquisition.h"

#include "board.h"
#include "logger.h"
#include "routines/str_utils.h"

#include <atomic>
#include <thread>

#include <iostream>

namespace hapi {
extern volatile std::atomic<bool> running;

// This loops while the program is running to acquire images from the camera
// when a particle is detected.
void acquisition_loop(std::shared_ptr<USBCamera> &camera, OBISLaser &laser,
                      std::filesystem::path &out_dir, std::string &image_type,
                      std::chrono::milliseconds interval_time, HAPIMode mode) {
  Board &board = Board::instance();
  Logger &log = Logger::instance();

  if (use_camera(mode)) {
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

  log.info() << "Entering main loop." << std::endl;
  while (running) {
    if (mode == HAPIMode::CW) {
      std::this_thread::yield();
      continue;
    }
    log.info() << "Arming HAPI-E board." << std::endl;
    board.arm();
    if (mode == HAPIMode::INTERVAL || mode == HAPIMode::ALIGN) {
      log.info() << "Waiting for interval." << std::endl;
      while (std::chrono::duration_cast<std::chrono::milliseconds>(
                 current_time - last_time)
                 .count() < interval_time.count()) {
        current_time = std::chrono::high_resolution_clock::now();
        FaultCode fault = laser.fault();
        if (fault != 0) {
          std::vector<OBISLaser::FaultBits> faults = laser.fault_bits(fault);
          for (auto f : faults) {
            log.error() << "Laser fault: " << laser.fault_str(f) << std::endl;
          }
          // TODO: be able to handle some types of laser faults (overheating)
          running = false;
        }
        if (running) {
          std::this_thread::yield();
        } else {
          break;
        }
      }
      if (running) {
        log.info() << "Sending trigger." << std::endl;
        board.trigger();
      }
    } else {
      log.info() << "Waiting for trigger." << std::endl;
    }
    // wait for the board to signal it has taken an image
    while (!board.is_done()) {
      if (running) {
        std::this_thread::yield();
      } else {
        // exit the program if signaled
        break;
      }
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

    if (use_camera(mode)) {
      try {
        acquire_image(camera, out_dir, image_type, image_count, image_time,
                      mode);
        image_count++;
      } catch (const std::exception &ex) {
        log.exception(ex) << "Failed to acquire image." << std::endl;
        break;
      }
    }
  }

  if (use_camera(mode)) {
    log.info() << "Ending acquisition." << std::endl;
    camera->end_acquisition();
  }
}

// Acquires an image from the camera and saves it.
void acquire_image(std::shared_ptr<USBCamera> &camera,
                   std::filesystem::path &out_dir, std::string &image_type,
                   unsigned int image_count, const std::string &image_time,
                   HAPIMode mode) {
  Logger &log = Logger::instance();
  // get the image from the camera
  log.info() << "Acquiring image from camera." << std::endl;
  Spinnaker::ImagePtr result = camera->acquire_image();
  if (result->IsIncomplete()) {
    log.info() << "Image incomplete with status " << result->GetImageStatus()
               << "." << std::endl;
  } else {
    if (mode != HAPIMode::ALIGN && image_count == 0) {
      if (!std::filesystem::exists(out_dir /
                                   (out_dir.stem().string() + "_thumbs"))) {
        log.info() << "First image. Creating output directory." << std::endl;
        // creates out dir and thumbnail dir in one command
        std::filesystem::create_directories(
            out_dir / (out_dir.stem().string() + "_thumbs"));
      }
    }
    if (!std::filesystem::exists("/var/www/hapi/")) {
      log.info() << "Creating /var/www/hapi/ directory." << std::endl;
      std::filesystem::create_directories("/var/www/hapi/");
    }
    // save the image
    log.info() << "Converting image to mono 8 bit with no color processing."
               << std::endl;
    // Spinnaker::ImagePtr converted = result->Convert(
    //    Spinnaker::PixelFormat_Mono8, Spinnaker::NO_COLOR_PROCESSING);
    std::filesystem::path fname;
    std::filesystem::path last = "/var/www/hapi/last.png";
    if (mode == HAPIMode::ALIGN) {
      fname = "/var/www/hapi/biglast.tiff";
      log.info() << "Saving image (" << image_count << ") " << fname << "."
                 << std::endl;
      result->Save(fname.string().c_str());
      log.info() << "Creating thumbnail image." << std::endl;
      std::string convert = "sudo convert " + fname.string() +
                            " -thumbnail 600 " + last.string() + " &";
      std::system(convert.c_str());
    } else {
      fname = out_dir / (image_time + "." + image_type);
      log.info() << "Saving image (" << image_count << ") " << fname << "."
                 << std::endl;
      result->Save(fname.string().c_str());
      std::filesystem::path thumb =
          out_dir / (out_dir.stem().string() + "_thumbs");
      thumb /= image_time + "_thumb" + "." + image_type;
      log.info() << "Creating thumbnail image." << std::endl;
      std::string convert = "sudo convert " + fname.string() +
                            " -thumbnail 600 " + thumb.string();
      std::string convert_last =
          "sudo convert " + thumb.string() + " " + last.string();
      std::string cmd = "(" + convert + " && " + convert_last + ") &";
      std::system(cmd.c_str());
    }
  }
  log.info() << "Releasing image." << std::endl;
  result->Release();
  // wait for image to be freed before we arm
  while (result->IsInUse()) {
    std::this_thread::yield();
  }
}

// Returns true if the specified HAPI mode requires the camera.
bool use_camera(HAPIMode mode) {
  return mode == HAPIMode::INTERVAL || mode == HAPIMode::TRIGGER ||
         mode == HAPIMode::ALIGN;
}
};  // namespace hapi
