#include <cmath>
#include <experimental/filesystem>
#include <functional>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>

#include <sys/types.h>
#include <unistd.h>

#include "config.h"
#include "routines/get_config.h"

using namespace hapi;

namespace fs = std::experimental::filesystem;

using oformatter = std::function<bool(std::string, std::string&)>;
using iformatter = std::function<std::string(std::string)>;

bool odefault_formatter(std::string i, std::string& o) {
  o = i;
  return true;
}

std::string idefault_formatter(std::string i) { return i; }

template <typename T>
oformatter ot_formatter(T min, T max) {
  return [=](std::string i, std::string& o) {
    try {
      std::istringstream iss(i);
      T v;
      iss >> v;
      if (v >= min && v <= max) {
        o = i;
        return true;
      }
    } catch (...) {
    }
    return false;
  };
}

oformatter obinary_formatter(double min, double max) {
  return [=](std::string i, std::string& o) {
    try {
      double v = std::stod(i);
      if (v >= min && v <= max) {
        v = std::round((v - min) / (max - min) * 255.0);
        int i = v;
        o = std::to_string(i);
        return true;
      }
    } catch (...) {
    }
    return false;
  };
}

iformatter ibinary_formatter(double min, double max) {
  return [=](std::string i) {
    std::istringstream in(i);
    int t;
    if (i.find('x') != std::string::npos) {
      in >> std::hex >> t >> std::ws;
    } else if (i.find('b') != std::string::npos) {
      t = std::stoi(i.substr(i.find('b') + 1), nullptr, 2);
    } else {
      in >> t >> std::ws;
    }
    double v = ((double)t) / 255.0 * (max - min) + min;
    return std::to_string(v);
  };
}

void prompt(Config& config, std::string key, std::string desc,
            iformatter ifmt = idefault_formatter,
            oformatter ofmt = odefault_formatter) {
  std::cout << desc
            << " (current: " << ifmt(config[std::forward<std::string>(key)])
            << "): ";
  std::string i, o;
  std::getline(std::cin, i);
  if (i.size() != 0) {
    while (!ofmt(i, o)) {
      std::cout << "Invalid argument" << std::endl;
      std::cout << desc
                << " (current: " << ifmt(config[std::forward<std::string>(key)])
                << "): ";
      std::getline(std::cin, i);
    }
    config[std::forward<std::string>(key)] = o;
  }
}

inline bool has_sudo() { return getuid() == 0 && geteuid() == 0; }

int main(int argc, char* argv[]) {
  if (!has_sudo()) {
    std::cout << "root permissions required to run!" << std::endl;
    return -1;
  }
  std::cout << "HAPI Config" << std::endl
            << "Leave field blank to keep current value" << std::endl
            << std::endl;
  Config config = get_config();
  prompt(config, "output", "Data output location");
  prompt(config, "delay", "Laser pulse delay in milliseconds from 1.0 to 10.0",
         ibinary_formatter(1, 10), obinary_formatter(1, 10));
  prompt(config, "exp", "Camera exposure time in milliseconds from 1.0 to 10.0",
         ibinary_formatter(1, 10), obinary_formatter(1, 10));
  prompt(config, "pulse", "Laser pulse in nanoseconds from 10 to 200",
         ibinary_formatter(10, 200), obinary_formatter(10, 200));
  prompt(config, "image_type", "Image type; tiff, png, bmp, jpg");
  prompt(config, "pmt_threshold", "PMT threshold in volts from 0.0 to 1.0",
         ibinary_formatter(0, 1), obinary_formatter(0, 1));
  prompt(config, "pmt_gain", "PMT gain in volts from 0.5 to 1.1",
         ibinary_formatter(0.5, 1.1), obinary_formatter(0.5, 1.1));
  prompt(config, "interval", "Interval in milliseconds for interval mode",
         idefault_formatter,
         ot_formatter<int>(0, std::numeric_limits<int>::max()));
  prompt(config, "camera_gain", "Camera gain in dB from 1.0 to 47.994267",
         idefault_formatter, ot_formatter<double>(1, 47.994267));

  if (!fs::exists("/etc/hapi")) fs::create_directory("/etc/hapi");

  config.save("/etc/hapi/hapi.conf");

  return 0;
}
