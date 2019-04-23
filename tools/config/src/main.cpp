#include <experimental/filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>

#include <sys/types.h>
#include <unistd.h>

namespace fs = std::experimental::filesystem;

inline bool has_sudo() { return getuid() == 0 && geteuid() == 0; }

std::string default_formatter(std::string line) { return line; }

std::string delay_formatter(std::string line) {
  int t = std::stoi(line);
  if (t < 0) t = 0;
  if (t > 15) t = 15;
  return std::to_string(t);
}

std::string exp_formatter(std::string line) { return delay_formatter(line); }

std::string pulse_formatter(std::string line) {
  int t = std::stoi(line);
  if (t < 0) t = 0;
  if (t > 310) t = 310;
  t /= 10;
  return std::to_string(t);
}

void prompt(
    std::string k, std::string s, std::ostringstream& sout, std::string d = "",
    std::function<std::string(std::string)> formatter = default_formatter) {
  std::string line;
  std::cout << "Enter " << s << ": ";
  if (d.size() == 0) {
    while (line.size() == 0) std::getline(std::cin, line);
  } else {
    std::getline(std::cin, line);
    if (line.size() == 0) line = d;
  }
  try {
    line = formatter(line);
    sout << k << "=" << line << std::endl;
  } catch (...) {
    std::cout << "Invalid format" << std::endl;
    prompt(k, s, sout, d, formatter);
  }
}

int main(int argc, char* argv[]) {
  if (!has_sudo()) {
    std::cout << "root permissions required to run!" << std::endl;
    return -1;
  }
  std::ostringstream sout;
  prompt("output", "output directory (default: hapi/)", sout, "hapi/");
  prompt("delay", "delay in microseconds: 0-15 (default 8)", sout, "8",
         delay_formatter);
  prompt("exp", "exposure in microseconds: 0-15 (default 2)", sout, "2",
         exp_formatter);
  prompt("pulse", "pulse width in nanoseconds: 0-310 (default 310)", sout,
         "310", pulse_formatter);
  prompt("image_type",
         "Image type: png, ppm, pgm, tiff, jpeg, jpg, bmp (default tiff)", sout,
         "tiff");

  std::string ostr = sout.str();

  if (!fs::exists("/etc/hapi")) fs::create_directory("/etc/hapi");

  std::ofstream out("/etc/hapi/hapi.conf", std::ios::binary);

  if (out) {
    out << ostr;
    out.close();
  } else {
    std::cout << "Unable to write config file to: /etc/hapi/hapi.conf"
              << std::endl;
  }

  return 0;
}
