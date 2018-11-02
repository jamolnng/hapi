#include "config.h"

#include <fstream>
#include <sstream>

using namespace hapi;

Config::Config(std::map<std::string, std::string> &defaults) {
  _items.insert(defaults.begin(), defaults.end());
}

Config::~Config() {}

void Config::load(std::filesystem::path p) {
  std::ifstream in = std::ifstream(p);
  if (in) {
    std::string line;
    while (std::getline(in, line)) {
      if (line[0] == '#') continue;
      std::istringstream is_line(line);
      std::string key;
      if (std::getline(is_line, key, '=')) {
        std::string value;
        if (std::getline(is_line, value)) _items[key] = value;
      }
    }
    in.close();
  }
}

bool Config::has(std::string &key) { return _items.find(key) != _items.end(); }

const std::string &Config::operator[](std::string &&key) const {
  return _items.at(key);
}

const std::string &Config::operator[](const std::string &&key) const {
  return _items.at(key);
}

int Config::get_int(const std::string &&key, int base) {
  std::string i = _items.at(key);
  std::string::size_type n = i.find_first_of('b');
  if (n != std::string::npos) return std::stoi(i.substr(n + 1), nullptr, 2);
  n = i.find_first_of('x');
  if (n != std::string::npos) return std::stoi(i.substr(n + 1), nullptr, 16);
  return std::stoi(i, nullptr, base);
}