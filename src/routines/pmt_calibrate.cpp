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
  log.info() << "Trigger interval:" << time_limit << " ms" << std::endl;

  int gain = 0xFF / 2;
  int threshold = 0xFF / 2;
  int gainhalf = (0xFF + 1) / 4;  // first cut is half of the range
  int offset = 3;                 // extra adjustment to reduce random triggers
  int half = (0xFF + 1) / 4;      // first cut is half of the range
  // for (gain = 0xFF; gain >= 0; gain--) {
  while (gainhalf > 1) {
    if (!running) {
      goto exit;
    }
    log.info() << "Binary search with gain: " << gain << std::endl;
    /*if ((gain + 1) % 16 == 0) {
      log.info() << std::setfill('0') << std::right << std::hex
                 << "Testing gain range: 0x" << std::setw(2) << gain << "-0x"
                 << std::setw(2) << (gain - 0x0Fu) << std::endl;
    }
    if (pass(gain, threshold, ms, board)) {
      log.info() << "Found gain: 0x" << std::hex << std::setw(2)
                 << std::setfill('0') << gain << std::endl; */

    if (pass(gain, threshold, ms, board))  // no trigger, increase gain
    {
      gain += gainhalf;
      log.info() << "...no trigger" << std::endl;
    } else  // trigger, lower gain
    {
      gain -= gainhalf;
      log.info() << "...trigger" << std::endl;
    }
    gainhalf /= 2;
    log.info() << "Next adjustment: " << gainhalf << std::endl;

  }  // end gain binary search

  // binary search for t
  threshold = 0xFF / 2;

  while (half > 1)  // break when we
  {
    log.info() << "Binary search with threshold: " << threshold << std::endl;

    if (pass(gain, threshold, ms, board))  // no trigger, increase threshold
    {
      threshold += half;  // 0x00 is 'highest' threshold ie hardest to pass
      log.info() << "...no trigger" << std::endl;
    } else  // trigger, lower threshold
    {
      threshold -= half;
      log.info() << "...trigger" << std::endl;
    }
    half /= 2;
    log.info() << "Next adjustment: " << half << std::endl;
    if (!running) {
      goto exit;
    }
  }  // end binary search
  return std::make_pair(gain, std::max(threshold - offset, 0));

  // for (threshold = 0x00; threshold <= 0xFF; threshold++) {
  //  if ((threshold) % 16 == 0) {
  //    log.info() << std::setfill('0') << std::right << std::hex
  //             << "Testing threshold range: 0x" << std::setw(2) << threshold
  //             << std::endl;
  //  }
  //    if (!running) {
  //      goto exit;
  //    }
  //    if (!pass(gain, threshold, ms, board)) {
  //      return std::make_pair(gain, std::max(threshold - 1, 0));
  //    }
  //  }

exit:
  throw PMTCalibrationError();
}
}  // namespace hapi
