#include <chrono>
#include <fstream>
#include <iostream>

#include "argparse.h"
#include "board.h"
#include "config.h"
#include "logger.h"
#include "routines/get_config.h"
#include "routines/os_utils.h"
#include "routines/pmt_calibrate.h"

using namespace hapi;

int main(int argc, char* argv[]) {
  Logger& log = Logger::instance();
  log.set_stream(std::cout);

  ArgumentParser parser("HAPI PMT Calibration");
  parser.add_argument("-w", "--write",
                      "Writes the calibrated values to the hapi config.",
                      false);
  parser.add_argument(
      "-i", "--interval",
      "The time interval in milliseconds that it should check in.", false);
  try {
    parser.parse(argc, argv);
  } catch (const ArgumentParser::ArgumentNotFound& ex) {
    log.exception(ex) << "Failed to parse command line arguments." << std::endl;
    log.critical() << "Exiting (-1)..." << std::endl;
    return -1;
  }
  if (parser.is_help()) return 0;

  auto write = parser.get<bool>("w");
  long long ms = 5000;
  if (parser.exists("i")) ms = parser.get<int>("i");

  std::pair<unsigned int, unsigned int> vals;
  try {
    log.info() << "Calibrating..." << std::endl;
    vals = pmt_calibrate(ms);
  } catch (const PMTCalibrationError& ex) {
    log.critical()
        << "Could not find values for the gain and threshold that worked."
        << std::endl;
    log.critical() << "Exiting (-1)..." << std::endl;
    return -1;
  }
  auto gain = vals.first;
  auto threshold = vals.second;

  log.info() << "Calibration success!" << std::endl;
  log.info() << std::hex << "Gain:      0x" << std::setw(2) << std::setfill('0')
             << gain << std::endl;
  log.info() << std::hex << "Threshold: 0x" << std::setw(2) << std::setfill('0')
             << threshold << std::endl;

  if (write) {
    if (!is_root()) {
      log.critical() << "Root permissions required to save config."
                     << std::endl;
      log.critical() << "Exiting (-1)..." << std::endl;
      return -1;
    }
    log.info() << "Reading current configuration." << std::endl;
    Config config = get_config();
    log.info() << "Updating gain and threshold." << std::endl;
    config["pmt_gain"] = std::to_string(gain);
    config["pmt_threshold"] = std::to_string(threshold);
    log.info() << "Saving new configuration to /etc/hapi/hapi.conf"
               << std::endl;
    config.save("/etc/hapi/hapi.conf");
  }
  return 0;
}
