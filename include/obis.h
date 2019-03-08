#ifndef OBISLASER_H
#define OBISLASER_H

#include "serial.h"

#include <exception>
#include <sstream>
#include <stdexcept>
#include <vector>

using FaultCode = unsigned long;
using StatusCode = unsigned long;

class OBISLaser {
 public:
  enum StatusBits {
    LaserFault = 0x000001,
    LaserEmission = 0x000002,
    LaserReady = 0x000004,
    LaserStandby = 0x000008,
    CDRHDelay = 0x000010,
    LaserHardwareFault = 0x000020,
    LaserError = 0x000040,
    LaserPowerCalibration = 0x000080,
    LaserWarmUp = 0x000100,
    LaserNoise = 0x000200,
    ExternalOperatingMode = 0x000400,
    FieldCalibrartion = 0x000800,
    LaserPowerVoltage = 0x001000
  };

  enum FaultBits {
    BasePlateTempFault = 0x000001,
    DiodeTempFault = 0x000002,
    InternalTempFault = 0x000004,
    LaserPowerSupplyFault = 0x000008,
    I2CError = 0x000010,
    OverCurrent = 0x000020,
    LastChecksumError = 0x000040,
    ChecksumRecovery = 0x000080,
    BufferOverflow = 0x000100,
    WarmUpLimitFault = 0x000200,
    TECDriverError = 0x000400,
    CCBError = 0x000800,
    DiodeTempLimitError = 0x001000,
    LaserReadyFault = 0x002000,
    PhotoDiodeFault = 0x004000,
    FatalFault = 0x008000,
    StartupFault = 0x010000,
    WatchdogTimerReset = 0x020000,
    FieldCalibrartionError = 0x040000,
    OverPower = 0x100000
  };

  enum State { Off = 0, On = 1 };

  enum SourceType {
    ConstantPower,
    ConstantCurrent,
    Digital,
    Analog,
    Mixed,
    DIGSO,
    MIXSO
  };

  enum DeviceType { DDL, OPSL, Mini, Master, Other };

  // struct date_t
  //{
  //	unsigned short mm, dd, yyyy;
  //};

  struct sys_info_t {
    // Laser identification string
    std::string _idn;
    // model name
    std::string _model;
    // manufacturing date
    std::string _mdate;
    // calibration date
    std::string _cdate;
    // serial number
    std::string _snumber;
    // part number
    std::string _pnumber;
    // firmware version
    std::string _firmware;
    // protocol version
    std::string _protocol;
    // wavelength of laser in nanometers
    double _wavelength;
    // power rating
    double _power;
    // device type
    DeviceType _dtype;
    // nominal CW output power
    double _powerNominal;
    // minimum CW output power
    double _powerMin;
    // maximum CW output power
    double _powerMax;
    // maximum operational base plate temperature
    double _baseplateMaxTemp;
    // minimum operational base plate temperature
    double _baseplateMinTemp;
    // maximum operational diode temperature
    double _diodeMaxTemp;
    // minimum operational diode temperature
    double _diodeMinTemp;
    // maximum operational interal temperature
    double _internalTempMax;
    // minimum operational interal temperature
    double _internalTempMin;
    // minimum operating current
    double _currentMin;
    // maximum operating current
    double _currentMax;
  };

  static const std::vector<StatusBits> status_bits(const StatusCode status);
  static const std::vector<FaultBits> fault_bits(const FaultCode fault);

  static const std::vector<std::string> status(const StatusCode status);
  static const std::vector<std::string> fault(const FaultCode fault);

  static const std::string status_str(const OBISLaser::StatusBits bit);
  static const std::string fault_str(const OBISLaser::FaultBits bit);
  static const std::string state_str(const OBISLaser::State state);
  static const std::string source_str(const OBISLaser::SourceType type);

  static const unsigned int error_no(const std::string err);
  static const std::string error_str(const unsigned int err);

  OBISLaser(std::string device);
  ~OBISLaser(void);

  void send(std::string cmd);

  template <typename T>
  const T query(const std::string str);

  void complete_handshake(void);

  // causes device to warm boot if implemented
  void reset(void);
  // runs a self-test procedure if implemented
  FaultCode test(void);

  // toggle system handshaking
  void handshake(const State s);
  // queries the system handshaking
  const State handshake(void);

  // toggles system command prompt
  void prompt(const State s);
  // queries system command prompt
  const State prompt(void);

  // toggle laser auto start feature
  void auto_start(const State s);
  // queries the laser auto start feature
  const State auto_start(void);

  // sets the analog modulation type (1 for )
  void amod_type(const unsigned short t);
  // queries the analog modulation type
  const unsigned short amod_type(void);

  // queries the system status
  const StatusCode status(void);
  // queries current system faults
  const FaultCode fault(void);

  // toggle laser status indicator(s) (LEDs)
  void indicator(const State s);
  // queries the laser status indicator(s) (LEDs)
  const State indicator(void);

  // queries the number of error records in the error queue
  unsigned int error_count(void);

  // clears all of the error records in the queue
  void clear_error(void);

  // returns the system info, if query it queries the device
  const sys_info_t sys_info(bool query = false);

  // enters and stores user-defined information, data can have a maximum length
  // of 31 characters
  void user(const unsigned short index, const std::string data);
  // queries the user-defined information
  const std::string user(const unsigned short index);

  // void fcdate(const date_t date);
  // const date_t fcdate(void);

  // returns the number of on/off cycles
  const unsigned int cycles(void);

  // returns the hours the laser has been on
  const unsigned int hours(void);

  // returns the hours the diode has been operated
  const unsigned int diode_hours(void);

  // returns the present output power of the laser
  const double power(void);

  // returns the preset output current of the laser
  const double current(void);

  // returns the present laser baseplate temperature
  const double baseplate_temp(void);

  // returns the status of the system interlock
  const bool lock(void);

  // sets the laser operating mode
  void mode(const SourceType t);
  // returns the laser operating mode
  const SourceType mode(void);

  // sets the present laser power level
  void power_level_immediate_amplitude(const double value);
  // returns the present laser power level
  const double power_level_immediate_amplitude(void);

  // turn the laser on or off
  void state(const State s);
  // queries the current laser emission status
  const State state(void);

  // enables or disables the CDRH laser emission delay
  void cdrh(const State s);
  // queries the status of the CDRH laser emission delay
  const State cdrh(void);

  // enables or disables temperature control of the laser diode
  void temp_probe(const State s);
  // queries temperature control of the laser diode
  const State temp_probe(void);

  // starts a self-laser power calibration
  void calibrate(void);
  // undoes the filed calibration
  void uncalibrate(void);

  // enables or disables blanking in analog modulation mode
  void blanking(const State s);
  // queries present state of analog modulation blanking
  const State blanking(void);

  // queries the present laser diode temperature
  const double diode_temp(void);
  // queries the diode set point temperature
  const double dset_temp(void);
  // queries the present internal laser temperature
  const double internal_temp(void);

 private:
  bool _handshake;
  sys_info_t _info;
  SerialInterface _serial;

  const std::string result(const std::string str);
};

template <>
const std::string OBISLaser::query(const std::string str);

template <>
const OBISLaser::State OBISLaser::query(const std::string str);

template <>
const OBISLaser::DeviceType OBISLaser::query(const std::string str);

template <>
const OBISLaser::SourceType OBISLaser::query(const std::string str);

template <typename T>
const T OBISLaser::query(const std::string str) {
  T val;
  std::istringstream ss(result(str));
  ss >> val;
  return val;
}

#endif
