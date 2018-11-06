#ifndef CONFIG_H
#define CONFIG_H

#include <experimental/filesystem>
#include <map>
#include <string>

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

  bool has(std::string &key);
  const std::string &operator[](std::string &&key) const;
  const std::string &operator[](const std::string &&key) const;

  int get_int(const std::string &&key, int base = 10);

  const std::map<std::string, std::string> &items();

 private:
  std::map<std::string, std::string> _items;
};
}  // namespace hapi

#endif
