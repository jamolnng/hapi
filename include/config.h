#ifndef HAPI_CONFIG_H
#define HAPI_CONFIG_H

#include <experimental/filesystem>
#include <map>
#include <sstream>
#include <string>
#include <type_traits>

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
  T get(const std::string &&key) const {
    std::string i = _items.at(key);
    if (std::is_same<T, std::string>::value) return i;
    std::istringstream in(i);
    T t;
    if (std::is_integral<T>::value) {
      if (i.find('x') != std::string::npos) {
        in >> std::hex >> t >> std::ws;
      } else if (i.find('b') != std::string::npos) {
        t = std::stoi(i.substr(i.find('b') + 1), nullptr, 2);
      }
    } else {
      i >> t >> std::ws;
    }
    return t;
  }

  const std::map<std::string, std::string> &items();

 private:
  std::map<std::string, std::string> _items;
};
}  // namespace hapi

#endif
