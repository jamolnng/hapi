#ifndef HAPI_GET_CONFIG_H
#define HAPI_GET_CONFIG_H
#include "config.h"
#include "filesystem.hpp"

#include <string>

namespace hapi {
// Loads and returns the config.
Config get_config();
// Returns the image type from the config.
std::string get_image_type(Config &config);
// Returns the output directory where the data will be saved to.
std::filesystem::path get_out_dir(int rn, Config &config);
};  // namespace hapi
#endif
