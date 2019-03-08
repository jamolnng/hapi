#ifndef HAPI_PMT_CALIBRATE_H
#define HAPI_PMT_CALIBRATE_H
#include <stdexcept>
#include <utility>
namespace hapi {
/**
 * pmt_calibrate
 *
 * Finds calibration values for the pmt gain and trigger threshold
 *
 * millis: the limit to check within
 *
 * Returns: a pair of ints, the first the gain value, the second the threshold
 * value
 */
std::pair<unsigned int, unsigned int> pmt_calibrate(long long millis);
class PMTCalibrationError : public std::runtime_error {
 public:
  PMTCalibrationError() noexcept
      : std::runtime_error(
            "Could not find values for the gain and threshold that worked!") {}
};
};  // namespace hapi
#endif
