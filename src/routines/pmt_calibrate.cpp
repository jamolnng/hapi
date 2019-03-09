#include "routines/pmt_calibrate.h"

#include <atomic>
#include <chrono>
#include <cmath>
#include <iomanip>

#include "board.h"
#include "logger.h"

namespace hapi {

// bool that states whether the program should remain running. Defined in
// os_utils.cpp
extern volatile std::atomic<bool> running;

/**
 * pass
 *
 * Checks if with the given gain and threshold the board has not been triggered
 * in a time interval
 *
 * gain: the gain value to check
 * threshold: the trigger threshold
 * time_limit: the time limit to check within
 */
inline bool pass(const unsigned int gain, const unsigned int threshold,
                 const std::chrono::milliseconds& time_limit, Board& board) {
  Logger& log = Logger::instance();
  board.set_pmt_gain(gain);
  board.set_pmt_threshold(threshold);
  board.reset();
  board.arm();
  auto start_time = std::chrono::high_resolution_clock::now();
  auto current_time = start_time;
  while (current_time - start_time < time_limit) {
    if (board.is_done() || !running) {
      auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                    current_time - start_time)
                    .count();
      if (ms > 0) {
        log.debug() << "Gain: 0x" << std::hex << gain << " Threshold: 0x"
                    << threshold << " Triggered in: " << std::dec << ms
                    << " milliseconds." << std::endl;
      }
      board.disarm();
      return false;
    }
    current_time = std::chrono::high_resolution_clock::now();
  }
  board.disarm();
  return true;
}

std::pair<unsigned int, unsigned int> pmt_calibrate(long long time_limit) {
  auto ms = std::chrono::milliseconds(time_limit);
  Board& board = Board::instance();

  board.set_trigger_source(Board::TriggerSource::PMT);
  Logger& log = Logger::instance();

  int gain = 0xFF;
  int threshold = 0x00;
  for (gain = 0xFF; gain >= 0; gain--) {
    if (!running) {
      goto exit;
    }
    if ((gain + 1) % 16 == 0) {
      log.info() << std::setfill('0') << std::right << std::hex
                 << "Testing gain range: 0x" << std::setw(2) << gain << "-0x"
                 << std::setw(2) << (gain - 0x0Fu) << std::endl;
    }
    if (pass(gain, threshold, ms, board)) {
      log.info() << "Found gain: 0x" << std::hex << std::setw(2)
                 << std::setfill('0') << gain << std::endl;
      for (threshold = 0x00; threshold <= 0xFF; threshold++) {
        if (!running) {
          goto exit;
        }
        if (!pass(gain, threshold, ms, board)) {
          return std::make_pair(gain, std::max(threshold - 1, 0));
        }
      }
      break;
    }
  }

exit:
  throw PMTCalibrationError();
}
}  // namespace hapi
