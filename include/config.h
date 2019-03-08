#ifndef HAPI_CONFIG_H
#define HAPI_CONFIG_H

#if _HAS_CXX17
#include <filesystem>
#else
#include <experimental/filesystem>
namespace std {
namespace filesystem = std::experimental::filesystem;
};
#endif
#include <map>
#include <sstream>
#include <string>
#include <type_traits>

#include <iostream>

namespace std {
namespace filesystem = std::experimental::filesystem;
}

namespace hapi {
class Config {
 public:
  Config() = default;
  Config(std::map<std::string, std::string> &defaults);
  ~Config();

  void load(std::filesystem::path p);

  void save(std::filesystem::path p);

  bool has(std::string &key);
  std::string &operator[](std::string &&key);
  const std::string &operator[](const std::string &&key) const;

  template <typename T>
  T get(const std::string &&key) {
    std::string i = _items.at(key);
    std::istringstream in(i);
    T t;
    if (std::is_integral<T>::value) {
      if (i.find('x') != std::string::npos) {
        in >> std::hex >> t >> std::ws;
      } else if (i.find('b') != std::string::npos) {
        t = std::stoi(i.substr(i.find('b') + 1), nullptr, 2);
      } else {
        in >> t >> std::ws;
      }
    } else {
      in >> t >> std::ws;
    }
    return t;
  }

  const std::map<std::string, std::string> &items();

 private:
  std::map<std::string, std::string> _items;
};
template <>
inline std::string Config::get<std::string>(const std::string &&key) {
  return _items.at(key);
}
}  // namespace hapi

#endif
