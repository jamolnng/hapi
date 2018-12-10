#ifndef HAPI_GET_CONFIG_H
#define HAPI_GET_CONFIG_H
#include "config.h"

#if _HAS_CXX17
#include <filesystem>
#else
#include <experimental/filesystem>
namespace std {
namespace filesystem = std::experimental::filesystem;
};
#endif
#include <string>

namespace hapi {
Config get_config();
std::string get_image_type(Config &config);
std::filesystem::path get_out_dir(std::string &start_time, Config &config);
};  // namespace hapi
#endif
