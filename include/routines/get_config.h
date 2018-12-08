#ifndef HAPI_GET_CONFIG_H
#define HAPI_GET_CONFIG_H
#include "config.h"

#include <filesystem>
#include <string>

namespace hapi {
Config get_config();
std::string get_image_type(Config &config);
std::filesystem::path get_out_dir(std::string &start_time, Config &config);
};  // namespace hapi
#endif
