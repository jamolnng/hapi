#include "logger.h"

#include <ctime>

using namespace hapi;

std::string str_time() {
  std::string str = "unknown";
  std::time_t t = std::time(nullptr);
  char mbstr[100];
  if (std::strftime(mbstr, sizeof(mbstr), "%Y-%m-%d %H:%M:%S", std::gmtime(&t)))
    str = std::string(mbstr);
  return str;
}

Logger::Logger()
    : _debug(nullptr),
      _info(nullptr),
      _warning(nullptr),
      _error(nullptr),
      _critical(nullptr){};

Logger::~Logger() {
  _debug.flush();
  _info.flush();
  _warning.flush();
  _error.flush();
  _critical.flush();
}

void Logger::set_stream(std::ostream &out) {
  _debug.rdbuf(out.rdbuf());
  _info.rdbuf(out.rdbuf());
  _warning.rdbuf(out.rdbuf());
  _error.rdbuf(out.rdbuf());
  _critical.rdbuf(out.rdbuf());
}

void Logger::set_streams(std::ostream &debug, std::ostream &info,
                         std::ostream &warn, std::ostream &error,
                         std::ostream &critical) {
  _debug.rdbuf(debug.rdbuf());
  _info.rdbuf(info.rdbuf());
  _warning.rdbuf(warn.rdbuf());
  _error.rdbuf(error.rdbuf());
  _critical.rdbuf(critical.rdbuf());
}

void Logger::log(Logger::LogLevel l, std::string msg) {
  switch (l) {
    case Logger::LogLevel::DEBUG:
      _debug << str_time() << ": DEBUG    | " << msg << std::endl;
      break;
    case Logger::LogLevel::INFO:
      _info << str_time() << ": INFO     | " << msg << std::endl;
      break;
    case Logger::LogLevel::WARNING:
      _warning << str_time() << ": WARNING  | " << msg << std::endl;
      break;
    case Logger::LogLevel::ERROR:
      _error << str_time() << ": ERROR    | " << msg << std::endl;
      break;
    case Logger::LogLevel::CRITICAL:
      _critical << str_time() << ": CRITICAL | " << msg << std::endl;
      break;
  }
}

void Logger::debug(std::string msg) { Logger::log(LogLevel::DEBUG, msg); }

void Logger::info(std::string msg) { Logger::log(LogLevel::INFO, msg); }

void Logger::warning(std::string msg) { Logger::log(LogLevel::WARNING, msg); }

void Logger::error(std::string msg) { Logger::log(LogLevel::ERROR, msg); }

void Logger::critical(std::string msg) { Logger::log(LogLevel::CRITICAL, msg); }

void Logger::exception(const std::exception &ex, std::string msg) {
  Logger::error(msg);
  _error << "*** EXCEPTION ***" << std::endl << ex.what() << std::endl;
}
