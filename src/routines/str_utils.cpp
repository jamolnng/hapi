#include "routines/str_utils.h"

#include <algorithm>
#include <ctime>

namespace hapi {
// Returns a std::string of the current time in the format YYYY_MM_DD-HH_MM_SS
std::string str_time() {
  std::time_t t = std::time(nullptr);
  char mbstr[100];
  std::string str;
  static unsigned int unknown_time = 0;
  if (std::strftime(mbstr, sizeof(mbstr), "%Y_%m_%d-%H_%M_%S", std::gmtime(&t)))
    str = std::string(mbstr);
  else
    str = "unknown" + std::to_string(unknown_time++);
  return str;
}

void lower(std::string &in) {
  std::transform(in.begin(), in.end(), in.begin(), ::tolower);
}
}  // namespace hapi
