#include "config.h"

#include <fstream>
#include <sstream>

using namespace hapi;

Config::Config(std::map<std::string, std::string> &defaults) {
  _items.insert(defaults.begin(), defaults.end());
}

Config::~Config() {}

void Config::load(std::filesystem::path p) {
  std::ifstream in(p, std::ios::binary);
  if (in) {
    std::string line;
    while (std::getline(in, line)) {
      if (line[0] == '#') {
        continue;
      }
      std::istringstream is_line(line);
      std::string key;
      if (std::getline(is_line, key, '=')) {
        std::string value;
        if (std::getline(is_line, value)) {
          _items[key] = value;
        }
      }
    }
    in.close();
  }
}

void Config::save(std::filesystem::path p) {
  std::ofstream out(p, std::ios::binary);
  if (out) {
    for (auto const &i : _items) {
      out << i.first << "=" << i.second << std::endl;
    }
    out.close();
  }
}

bool Config::has(std::string &key) { return _items.find(key) != _items.end(); }

std::string &Config::operator[](std::string &&key) { return _items.at(key); }

const std::string &Config::operator[](const std::string &&key) const {
  return _items.at(key);
}

const std::map<std::string, std::string> &Config::items() { return _items; }
