#include "routines/get_config.h"
#include "logger.h"
#include "routines/str_utils.h"

#include <filesystem>

using namespace hapi;

// default configuration parameters
std::map<std::string, std::string> config_defaults = {
    {"output", "."},       {"camera_trigger", "1"},     {"trigger_source", "0"},
    {"delay", "0b1000"},   {"exp", "0b0010"},           {"pulse", "0b11111"},
    {"image_type", "png"}, {"pmt_threshold", "0x10"},   {"pmt_gain", "0xFF"},
    {"interval", "3000"},  {"camera_gain", "47.994267"}};

Config get_config() {
  Logger &log = Logger::instance();
  // load config
  Config config(config_defaults);
  std::filesystem::path config_path = "/opt/hapi/hapi.conf";
  log.info() << "Loading config from " << config_path << "." << std::endl;
  try {
    if (std::filesystem::exists(config_path))
      config.load(config_path);
    else
      log.warning() << "Config file not found. Using defaults." << std::endl;
  } catch (const std::exception &ex) {
    log.exception(ex) << "Failed to load config. Using defaults." << std::endl;
    config = Config(config_defaults);
  }
  log.info() << "Config:" << std::endl;
  for (auto const &item : config.items())
    log.info() << "    " << item.first << ": " << item.second << std::endl;
  return config;
}

std::string get_image_type(Config &config) {
  Logger &log = Logger::instance();
  // get the image type from the config. default to png
  log.info() << "Loading image type from the config." << std::endl;
  std::string image_type;
  try {
    image_type = config.get<std::string>("image_type");
    lower(image_type);
    // png, ppm, pgm, tiff, jpeg, jpg, bmp
    if (image_type != "png" && image_type != "ppm" && image_type != "pgm" &&
        image_type != "tiff" && image_type != "jpeg" && image_type != "jpg" &&
        image_type != "bmp") {
      log.warning() << "Unsupported image type given: " << image_type
                    << ". Defaulting to png." << std::endl;
      image_type = "png";
    }
  } catch (const std::exception &ex) {
    log.exception(ex)
        << "Failed to load image type from config. Defaulting to png."
        << std::endl;
    image_type = "png";
  }
  log.info() << "Image type set to " << image_type << std::endl;
  return image_type;
}

std::filesystem::path get_out_dir(std::string &start_time, Config &config) {
  Logger &log = Logger::instance();
  std::filesystem::path out_dir;
  try {
    out_dir = std::filesystem::path(config.get<std::string>("output"));
  } catch (const std::exception &ex) {
    log.exception(ex) << "Failed to get output directory from config. "
                         "Defaulting to current working directory."
                      << std::endl;
    out_dir = std::filesystem::current_path();
  }
  out_dir /= start_time;
  log.info() << "Output directory set to " << out_dir << std::endl;
  return out_dir;
}