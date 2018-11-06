#ifndef HAPI_LOGGER_H
#define HAPI_LOGGER_H

#include <ostream>
#include <string>

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

  void log(LogLevel l, std::string msg);
  void debug(std::string msg);
  void info(std::string msg);
  void warning(std::string msg);
  void error(std::string msg);
  void critical(std::string msg);
  void exception(const std::exception &ex, std::string msg);

 private:
  Logger();
  ~Logger();
  std::ostream _debug;
  std::ostream _info;
  std::ostream _warning;
  std::ostream _error;
  std::ostream _critical;
};
}  // namespace hapi

#endif
