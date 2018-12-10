#include "routines/pmt_calibrate.h"

#include <atomic>
#include <chrono>
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
  auto start_time = std::chrono::high_resolution_clock::now();
  auto current_time = start_time;
  board.set_pmt_gain(gain);
  board.set_pmt_threshold(threshold);
  board.arm();
  while (current_time - start_time < time_limit) {
    if (board.is_done() || !running) {
      board.disarm();
      return false;
    }
    current_time = std::chrono::high_resolution_clock::now();
  }
  board.disarm();
  return true;
}

std::pair<unsigned int, unsigned int> pmt_calibrate(long long time_limit) {
  unsigned int gain;
  unsigned int threshold;
  auto ms = std::chrono::milliseconds(time_limit);
  Board& board = Board::instance();

  board.set_trigger_source(Board::TriggerSource::PMT);
  Logger& log = Logger::instance();

  for (gain = 0xFFu; gain > 0; gain--) {
    if ((gain + 1) % 16 == 0)
      log.info() << std::setfill('0') << std::setw(2) << std::hex
                 << "Testing with gain: 0x" << gain << std::endl;
    for (threshold = 0xFFu; threshold > 0; threshold--) {
      if (pass(gain, threshold, ms, board))
        return std::make_pair(gain, threshold);
      if (!running) goto exit;
    }
  }

exit:
  throw PMTCalibrationError();
}
}  // namespace hapi
