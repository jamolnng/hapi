#include "routines/pmt_calibrate.h"

#include "board.h"

#include <chrono>

using namespace hapi;

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
    if (board.is_done()) {
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

  for (gain = 0xFFu; gain > 0; gain--)
    for (threshold = 0xFFu; threshold > 0; threshold--)
      if (pass(gain, threshold, ms, board))
        return std::make_pair(gain, threshold);

  throw PMTCalibrationError();
}
