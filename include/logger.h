#ifndef HAPI_LOGGER_H
#define HAPI_LOGGER_H

#include <ostream>
#include <string>
#include <vector>

namespace hapi {
class Logger {
 public:
  enum LogLevel { DEBUG, INFO, WARNING, ERROR, CRITICAL };

  // the logger instance
  static Logger &instance() {
    static Logger _instance;
    return _instance;
  }

  void set_stream(std::ostream &out);
  void set_streams(std::ostream &debug, std::ostream &info, std::ostream &warn,
                   std::ostream &error, std::ostream &critical);

  std::ostream &log(LogLevel l);
  std::ostream &debug();
  std::ostream &info();
  std::ostream &warning();
  std::ostream &error();
  std::ostream &critical();
  std::ostream &exception(const std::exception &ex);

  std::ostream &append();

 private:
  Logger();
  ~Logger();
  std::ostream _debug;
  std::ostream _info;
  std::ostream _warning;
  std::ostream _error;
  std::ostream _critical;
  std::ostream &_last;
};
}  // namespace hapi

#endif
