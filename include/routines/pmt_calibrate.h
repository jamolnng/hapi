#ifndef HAPI_PMT_CALIBRATE_H
#define HAPI_PMT_CALIBRATE_H
#include <stdexcept>
#include <utility>
namespace hapi {

// Returns a pair of integers of the PMT calibration parameters. The first
// integer is the gain, the second is the threshold.
//
// time_limit: milliseconds to check if the triggered in that time frame.
std::pair<unsigned int, unsigned int> pmt_calibrate(long long millis);
class PMTCalibrationError : public std::runtime_error {
 public:
  PMTCalibrationError() noexcept
      : std::runtime_error(
            "Could not find values for the gain and threshold that worked!") {}
};
};  // namespace hapi
#endif
