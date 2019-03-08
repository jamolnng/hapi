#ifndef SERIAL_H
#define SERIAL_H

#include <cstdio>
#include <cstdlib>
#include <string>

//#if defined(__linux__)
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

//#endif

//#if defined(_WIN32) || defined(_WIN64)
// using speed_t = unsigned int;
//#endif

class SerialInterface {
 public:
  SerialInterface(void);
  SerialInterface(std::string device, speed_t baud, bool blocking = true);
  ~SerialInterface(void);

  void open(std::string device, speed_t baud, bool blocking = true);
  void close(void);
  bool is_open(void);

  std::string getline(char delim = '\n');
  std::string read(void);

  int write(const std::string str);
  int write(const char c);

  int printf(const char *fmt, ...);

  bool is_blocking(void);

 protected:
  int _device_fd;
  std::FILE *_device_f{nullptr};
  struct termios _config;

  bool _blocking;
};

#endif
