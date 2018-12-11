#include "logger.h"

#include "routines/str_utils.h"

using namespace hapi;

Logger::Logger()
    : _debug(nullptr),
      _info(nullptr),
      _warning(nullptr),
      _error(nullptr),
      _critical(nullptr),
      _last(nullptr){};

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

std::ostream &Logger::log(Logger::LogLevel l) {
  switch (l) {
    case Logger::LogLevel::DEBUG:
      _debug << str_time() << " | DEBUG    | ";
      _last.rdbuf(_debug.rdbuf());
      return _debug;
    case Logger::LogLevel::INFO:
      _info << str_time() << " | INFO     | ";
      _last.rdbuf(_info.rdbuf());
      return _info;
    case Logger::LogLevel::WARNING:
      _warning << str_time() << " | WARNING  | ";
      _last.rdbuf(_warning.rdbuf());
      return _warning;
    case Logger::LogLevel::ERROR:
      _error << str_time() << " | ERROR    | ";
      _last.rdbuf(_error.rdbuf());
      return _error;
    case Logger::LogLevel::CRITICAL:
      _critical << str_time() << " | CRITICAL | ";
      _last.rdbuf(_critical.rdbuf());
      return _critical;
  }
  return _last;
}

std::ostream &Logger::debug() { return log(LogLevel::DEBUG); }
std::ostream &Logger::info() { return log(LogLevel::INFO); }
std::ostream &Logger::warning() { return log(LogLevel::WARNING); }
std::ostream &Logger::error() { return log(LogLevel::ERROR); }
std::ostream &Logger::critical() { return log(LogLevel::CRITICAL); }
std::ostream &Logger::exception(const std::exception &ex) {
  error() << "***** EXCEPTION *****" << std::endl;
  error() << ex.what() << std::endl;
  return log(LogLevel::ERROR);
}

std::ostream &Logger::append() { return _last; }
