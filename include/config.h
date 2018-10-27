#ifndef CONFIG_H
#define CONFIG_H

#include <filesystem>
#include <map>
#include <string>

namespace hapi
{
class Config
{
  public:
    Config() = default;
    Config(std::map<std::string, std::string> &defaults);
    ~Config();

    void load(std::filesystem::path p);

    bool has(std::string &key);
    std::string &operator[](std::string &&key);
    std::string &operator[](const std::string &&key);
    const std::string &operator[](std::string &&key) const;
    const std::string &operator[](const std::string &&key) const;

    int get_int(const std::string &&key, int base = 10);

  private:
    std::map<std::string, std::string> _items;
};
}

#endif