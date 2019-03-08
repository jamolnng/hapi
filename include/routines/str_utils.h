#ifndef HAPI_STR_UTILS_H
#define HAPI_STR_UTILS_H

#include <string>

namespace hapi {
// Returns a std::string of the current time in the format YYYY_MM_DD-HH_MM_SS
std::string str_time();
void lower(std::string &in);
};  // namespace hapi

#endif
