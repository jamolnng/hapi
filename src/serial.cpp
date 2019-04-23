#include "serial.h"

#include <cstdarg>
#include <exception>
#include <iostream>
#include <sstream>
#include <stdexcept>

SerialInterface::SerialInterface(void) {}

SerialInterface::SerialInterface(std::string device, speed_t baud,
                                 bool blocking) {
  open(device, baud, blocking);
}

SerialInterface::~SerialInterface(void) { close(); }

void SerialInterface::open(std::string device, speed_t baud, bool blocking) {
  if (is_open()) {
    std::cout << "Serial interface is already open! " << __FUNCTION__
              << std::endl;
    throw std::logic_error(
        std::string("Serial interface is already open! ").append(__FUNCTION__));
  }
  _blocking = blocking;

  int flags = O_RDWR | O_NOCTTY;

  if (!blocking) {
    flags |= O_NONBLOCK;
  }
  _device_fd = ::open(device.c_str(), flags);
  _device_f = fdopen(_device_fd, "r+");

  if (!is_open()) {
    std::cout << "Could not open device: " << device
              << " Do you have permission?" << __FUNCTION__ << std::endl;
    throw std::runtime_error(std::string("Could not open device: \"")
                                 .append(device)
                                 .append("\" Do you have permission? ")
                                 .append(__FUNCTION__));
  }

  if (!isatty(_device_fd)) {
    std::cout << "Device " << device << " is not a character device! "
              << __FUNCTION__ << std::endl;
    close();
    throw std::invalid_argument(std::string("Device ")
                                    .append(device)
                                    .append(" is not a character device! ")
                                    .append(__FUNCTION__));
  }

  if (tcgetattr(_device_fd, &_config) < 0) {
    std::cout << "Error getting the attributes associated with the device! "
              << __FUNCTION__ << std::endl;
    close();
    throw std::runtime_error(
        std::string("Error getting the attributes associated with the device! ")
            .append(__FUNCTION__));
  }

  _config.c_iflag &=
      ~(IGNBRK | BRKINT | ICRNL | INLCR | PARMRK | INPCK | ISTRIP | IXON);
  _config.c_oflag = 0;
  _config.c_lflag &= ~(ECHO | ECHONL | ICANON | IEXTEN | ISIG);
  _config.c_cflag = CS8 | CREAD | CLOCAL;
  _config.c_cc[VMIN] = 1;
  _config.c_cc[VTIME] = 0;

  if (cfsetispeed(&_config, baud) < 0 || cfsetospeed(&_config, baud) < 0) {
    std::cout << "Error settings the baud rate! " << __FUNCTION__ << std::endl;
    close();
    throw std::runtime_error(
        std::string("Error setting the baud rate! ").append(__FUNCTION__));
  }
  if (tcsetattr(_device_fd, TCSAFLUSH, &_config) < 0) {
    std::cout << "Could not set the attributes associated with the device! "
              << __FUNCTION__ << std::endl;
    close();
    throw std::runtime_error(
        std::string("Could not set the attributes associated with the device! ")
            .append(__FUNCTION__));
  }
}

void SerialInterface::close(void) {
  if (is_open()) {
    ::close(_device_fd);
    fclose(_device_f);
    _device_f = nullptr;
    _device_fd = -1;
  }
}

bool SerialInterface::is_open(void) { return _device_f != nullptr; }

std::string SerialInterface::getline(char delim) {
  std::ostringstream ss;
  int ch;
  while ((ch = std::fgetc(_device_f)) != delim) {
    if (ch != EOF) {
      ss << (char)ch;
    }
  }
  ss << delim;
  return ss.str();
}

std::string SerialInterface::read(void) {
  std::ostringstream ss;
  int ch;
  while ((ch = std::fgetc(_device_f)) != EOF) {
    ss << ch;
  }
  return ss.str();
}

int SerialInterface::write(std::string str) {
  return ::write(_device_fd, str.c_str(), str.length());
}

int SerialInterface::write(char c) { return ::write(_device_fd, &c, 1); }

int SerialInterface::printf(const char *fmt, ...) {
  std::va_list args;
  va_start(args, fmt);
  int r = std::vfprintf(_device_f, fmt, args);
  va_end(args);
  return r;
}

bool SerialInterface::is_blocking(void) { return _blocking; }
