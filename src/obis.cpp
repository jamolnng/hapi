#include "obis.h"
#include <cstring>
#include <iostream>

std::exception_ptr safe_stoi(int &i, const std::string &str,
                             std::size_t *pos = 0, int base = 10) {
  try {
    i = std::stoi(str, pos, base);
  } catch (...) {
    return std::current_exception();  // capture
  }
  return std::exception_ptr();
}

OBISLaser::OBISLaser(std::string device) {
  _serial.open(device, (speed_t)B115200, true);
  clear_error();
  state(OBISLaser::State::Off);
  sys_info(true);
}

OBISLaser::~OBISLaser(void) {
  state(OBISLaser::State::Off);
  _serial.close();
}

void OBISLaser::send(std::string cmd) {
  _serial.write(cmd + "\r\n");
  complete_handshake();
}

void OBISLaser::complete_handshake(void) {
  return;
  if (_handshake) {
    std::string str = _serial.getline();
    if (str.compare("OK\r\n") != 0) {
      if (str.substr(0, 3).compare("ERR") == 0) {
        std::string error =
            error_str(error_no(str.substr(0, str.length() - 2)));
        throw std::runtime_error("Query error: Handshake not ok: " + error);
      }
      throw std::runtime_error("Handshake not ok: " + str);
    }
  }
}

void OBISLaser::reset(void) { send("*RST"); }

FaultCode OBISLaser::test(void) { return query<FaultCode>("*TST?"); }

void OBISLaser::handshake(const OBISLaser::State s) {
  _handshake = (bool)s;
  send("syst:comm:hand " + state_str(s));
}

const OBISLaser::State OBISLaser::handshake(void) {
  return query<OBISLaser::State>("syst:comm:hand?");
}

void OBISLaser::prompt(const OBISLaser::State s) {
  send("syst:comm:prom " + state_str(s));
}

const OBISLaser::State OBISLaser::prompt(void) {
  return query<OBISLaser::State>("syst:comm:prom?");
}

void OBISLaser::auto_start(const OBISLaser::State s) {
  send("syst:aut " + state_str(s));
}

const OBISLaser::State OBISLaser::auto_start(void) {
  return query<OBISLaser::State>("syst:aut?");
}

void OBISLaser::amod_type(const unsigned short t) {
  send("syst:inf:amod:type " + std::to_string(t));
}

const unsigned short OBISLaser::amod_type(void) {
  return query<unsigned short>("syst:inf:amod:type?");
}

const StatusCode OBISLaser::status(void) {
  return query<StatusCode>("syst:stat?");
}

const FaultCode OBISLaser::fault(void) {
  return query<FaultCode>("syst:faul?");
}

void OBISLaser::indicator(const OBISLaser::State s) {
  send("syst:ind:las " + state_str(s));
}

const OBISLaser::State OBISLaser::indicator(void) {
  return query<OBISLaser::State>("syst:ind:las?");
}

unsigned int OBISLaser::error_count(void) {
  return query<unsigned int>("syst:err:coun?");
}

void OBISLaser::clear_error(void) { send("syst:err:cle"); }

const OBISLaser::sys_info_t OBISLaser::sys_info(bool q) {
  if (q) {
    _info._idn = query<decltype(_info._idn)>("*IDN?");
    _info._model = query<decltype(_info._model)>("syst:inf:mod?");
    _info._mdate = query<decltype(_info._mdate)>("syst:inf:mdat?");
    _info._cdate = query<decltype(_info._cdate)>("syst:inf:cdat?");
    _info._firmware = query<decltype(_info._firmware)>("syst:inf:fver?");
    _info._snumber = query<decltype(_info._snumber)>("syst:inf:snum?");
    _info._pnumber = query<decltype(_info._pnumber)>("syst:inf:pnum?");
    _info._protocol = query<decltype(_info._protocol)>("syst:inf:pver?");
    _info._wavelength = query<decltype(_info._wavelength)>("syst:inf:wav?");
    _info._power = query<decltype(_info._power)>("syst:inf:pow?");
    _info._dtype = query<decltype(_info._dtype)>("syst:inf:type?");
    _info._powerNominal = query<decltype(_info._powerNominal)>("sour:pow:nom?");
    _info._powerMin = query<decltype(_info._powerMin)>("sour:pow:lim:low?");
    _info._powerMax = query<decltype(_info._powerMax)>("sour:pow:lim:high?");
    _info._baseplateMaxTemp =
        query<decltype(_info._baseplateMaxTemp)>("sour:temp:prot:bas:high?");
    _info._baseplateMinTemp =
        query<decltype(_info._baseplateMinTemp)>("sour:temp:prot:bas:low?");
    _info._diodeMaxTemp =
        query<decltype(_info._diodeMaxTemp)>("sour:temp:prot:diod:high?");
    _info._diodeMinTemp =
        query<decltype(_info._diodeMinTemp)>("sour:temp:prot:diod:low?");
    _info._internalTempMax =
        query<decltype(_info._internalTempMax)>("sour:temp:prot:int:high?");
    _info._internalTempMin =
        query<decltype(_info._internalTempMin)>("sour:temp:prot:int:low?");
    _info._currentMin =
        query<decltype(_info._currentMin)>("sour:curr:lim:low?");
    _info._currentMax =
        query<decltype(_info._currentMax)>("sour:curr:lim:high?");
  }
  return _info;
}

void OBISLaser::user(const unsigned short index, const std::string data) {
  std::string str = (data.length() > 31 ? data.substr(0, 31) : data);
  send("syst:inf:user " + std::to_string(index) + ", " + str);
}

const std::string OBISLaser::user(const unsigned short index) {
  return query<std::string>("syst:inf:user? " + std::to_string(index));
}

const unsigned int OBISLaser::cycles(void) {
  return query<unsigned int>("syst:cycl?");
}

const unsigned int OBISLaser::hours(void) {
  return query<unsigned int>("syst:hour?");
}

const unsigned int OBISLaser::diode_hours(void) {
  return query<unsigned int>("syst:diod:hour?");
}

const double OBISLaser::power(void) { return query<double>("sour:pow:lev?"); }

const double OBISLaser::current(void) {
  return query<double>("sour:pow:curr?");
}

const double OBISLaser::baseplate_temp(void) {
  return query<double>("sour:temp:bas?");
}

const bool OBISLaser::lock(void) {
  return (bool)query<OBISLaser::State>("syst:lock?");
}

void OBISLaser::mode(const OBISLaser::SourceType t) {
  if (t == OBISLaser::SourceType::ConstantPower ||
      t == OBISLaser::SourceType::ConstantCurrent) {
    send("sour:am:int " + source_str(t));
  } else {
    send("sour:am:ext " + source_str(t));
  }
}

const OBISLaser::SourceType OBISLaser::mode(void) {
  return query<OBISLaser::SourceType>("sour:am:sour?");
}

void OBISLaser::power_level_immediate_amplitude(const double value) {
  send("sour:pow:lev:imm:ampl " + std::to_string(value));
}

const double OBISLaser::power_level_immediate_amplitude(void) {
  return query<double>("sour:pow:lev:ampl?");
}

void OBISLaser::state(const OBISLaser::State s) {
  send("sour:am:stat " + state_str(s));
}

const OBISLaser::State OBISLaser::state(void) {
  return query<OBISLaser::State>("sour:am:stat?");
}

void OBISLaser::cdrh(const OBISLaser::State s) {
  send("syst:cdrh " + state_str(s));
}

const OBISLaser::State OBISLaser::cdrh(void) {
  return query<OBISLaser::State>("syst:cdrh?");
}

void OBISLaser::temp_probe(const OBISLaser::State s) {
  send("sour:temp:apr " + state_str(s));
}

const OBISLaser::State OBISLaser::temp_probe(void) {
  return query<OBISLaser::State>("sour:temp:apr?");
}

void OBISLaser::calibrate(void) { send("sour:pow:cal"); }

void OBISLaser::uncalibrate(void) { send("sour:pow:unc"); }

void OBISLaser::blanking(const OBISLaser::State s) {
  send("sour:am:blank " + state_str(s));
}

const OBISLaser::State OBISLaser::blanking(void) {
  return query<OBISLaser::State>("sour:am:blank?");
}

const double OBISLaser::diode_temp(void) {
  return query<double>("sour:temp:diod?");
}

const double OBISLaser::dset_temp(void) {
  return query<double>("sour:temp:dset?");
}

const double OBISLaser::internal_temp(void) {
  return query<double>("sour:temp:int?");
}

const std::vector<OBISLaser::StatusBits> OBISLaser::status_bits(
    const unsigned long status) {
  std::vector<OBISLaser::StatusBits> r;
  for (auto i = 0u; i < sizeof status; i++) {
    unsigned long mask = 1 << i;
    if (status & mask) {
      r.push_back(static_cast<OBISLaser::StatusBits>(mask));
    }
  }
  return r;
}

const std::vector<OBISLaser::FaultBits> OBISLaser::fault_bits(
    const unsigned long fault) {
  std::vector<OBISLaser::FaultBits> r;
  for (auto i = 0u; i < sizeof fault; i++) {
    unsigned long mask = 1 << i;
    if (fault & mask) {
      r.push_back(static_cast<OBISLaser::FaultBits>(mask));
    }
  }
  return r;
}

const std::vector<std::string> OBISLaser::status(const unsigned long status) {
  std::vector<OBISLaser::StatusBits> bits = OBISLaser::status_bits(status);
  std::vector<std::string> r;

  for (auto bit : bits) {
    r.push_back(status_str(bit));
  }

  return r;
}

const std::vector<std::string> OBISLaser::fault(const unsigned long fault) {
  std::vector<OBISLaser::FaultBits> bits = OBISLaser::fault_bits(fault);
  std::vector<std::string> r;

  for (auto bit : bits) {
    r.push_back(OBISLaser::fault_str(bit));
  }

  return r;
}

const std::string OBISLaser::status_str(const OBISLaser::StatusBits bit) {
  switch (bit) {
    case OBISLaser::StatusBits::LaserFault:
      return "Laser Fault";
    case OBISLaser::StatusBits::LaserEmission:
      return "Laser Emission";
    case OBISLaser::StatusBits::LaserReady:
      return "Laser Ready";
    case OBISLaser::StatusBits::LaserStandby:
      return "Laser Standby";
    case OBISLaser::StatusBits::CDRHDelay:
      return "CDRH Delay";
    case OBISLaser::StatusBits::LaserHardwareFault:
      return "Laser Hardware Fault";
    case OBISLaser::StatusBits::LaserError:
      return "Laser Error is queued";
    case OBISLaser::StatusBits::LaserPowerCalibration:
      return "Laser is within factory calibration specification";
    case OBISLaser::StatusBits::LaserWarmUp:
      return "Laser is warmed up";
    case OBISLaser::StatusBits::LaserNoise:
      return "Noise level is over 30";
    case OBISLaser::StatusBits::ExternalOperatingMode:
      return "External operating mode is selected";
    case OBISLaser::StatusBits::FieldCalibrartion:
      return "Field calibration is in progress";
    case OBISLaser::StatusBits::LaserPowerVoltage:
      return "12V laser power voltage is set";
    default:
      return "";
  }
}

const std::string OBISLaser::fault_str(const OBISLaser::FaultBits bit) {
  switch (bit) {
    case OBISLaser::FaultBits::BasePlateTempFault:
      return "Base plate temperature out of range";
    case OBISLaser::FaultBits::DiodeTempFault:
      return "Diode temperature out of range";
    case OBISLaser::FaultBits::InternalTempFault:
      return "Internal temperature out of range";
    case OBISLaser::FaultBits::LaserPowerSupplyFault:
      return "No electrical power to laser diode";
    case OBISLaser::FaultBits::I2CError:
      return "I2C bus error";
    case OBISLaser::FaultBits::OverCurrent:
      return "Diode over current";
    case OBISLaser::FaultBits::LastChecksumError:
      return "EEPROM checksum error in at least one section";
    case OBISLaser::FaultBits::ChecksumRecovery:
      return "EEPROM was restored to default settings";
    case OBISLaser::FaultBits::BufferOverflow:
      return "Bus message buffer overflow";
    case OBISLaser::FaultBits::WarmUpLimitFault:
      return "Warm-up time limit exceeded";
    case OBISLaser::FaultBits::TECDriverError:
      return "TE controller driver failure";
    case OBISLaser::FaultBits::CCBError:
      return "RS-485 bus error";
    case OBISLaser::FaultBits::DiodeTempLimitError:
      return "Diode temperature off by > 5C from set point";
    case OBISLaser::FaultBits::LaserReadyFault:
      return "Fail to emit at set power level";
    case OBISLaser::FaultBits::PhotoDiodeFault:
      return "Negative photodiode readout";
    case OBISLaser::FaultBits::FatalFault:
      return "Irrecoverable system failure";
    case OBISLaser::FaultBits::StartupFault:
      return "Errors encountered during firmware startup";
    case OBISLaser::FaultBits::WatchdogTimerReset:
      return "Firmware resumed from watchdog reset";
    case OBISLaser::FaultBits::FieldCalibrartionError:
      return "Errors encountered during field calibration";
    case OBISLaser::FaultBits::OverPower:
      return "Ouput power above limit";
    default:
      return "";
  }
}

const std::string OBISLaser::state_str(const OBISLaser::State state) {
  switch (state) {
    default:
    case OBISLaser::State::Off:
      return "off";
    case OBISLaser::State::On:
      return "on";
  }
}

const std::string OBISLaser::source_str(const OBISLaser::SourceType type) {
  switch (type) {
    default:
    case OBISLaser::SourceType::ConstantPower:
      return "cwp";
    case OBISLaser::SourceType::ConstantCurrent:
      return "cwc";
    case OBISLaser::SourceType::Digital:
      return "dig";
    case OBISLaser::SourceType::Analog:
      return "analog";
    case OBISLaser::SourceType::DIGSO:
      return "digso";
    case OBISLaser::SourceType::Mixed:
      return "mixed";
    case OBISLaser::SourceType::MIXSO:
      return "mixso";
  }
}

const unsigned int OBISLaser::error_no(const std::string err) {
  int r = -1;
  if (err.substr(0, 4).compare("ERR-") == 0) {
    safe_stoi(r, err.substr(4));
    return (unsigned int)abs(r);
  }
  if (err.substr(0, 3).compare("ERR") == 0) {
    safe_stoi(r, err.substr(3));
    return (unsigned int)abs(r);
  }
  return -1;
}

const std::string OBISLaser::error_str(const unsigned int err) {
  switch (err) {
    case 400:
      return "Broadcast of query is prohibited";
    case 350:
      return "Error queue is full";
    case 321:
      return "Out of memory";
    case 310:
      return "Unexpected/unrecoverable hardware or software fault";
    case 257:
      return "File to open not named";
    case 256:
      return "File does not exist";
    case 241:
      return "Device unavailable";
    case 221:
      return "Settings conflict";
    case 220:
      return "Invalid parameter";
    case 203:
      return "Command protected";
    case 200:
      return "Execution error";
    case 109:
      return "Missing parameter";
    case 102:
      return "Syntax error";
    case 100:
      return "Unrecognized command/query";
    case 0:
      return "No error";
    case 500:
      return "Coherent Connection bus error";
    case 510:
      return "I2C bus fault";
    case 520:
      return "Controller time out";
    case 900:
      return "Coherent Connection message time out";
    default:
      return "Unknown error";
  }
}

const std::string OBISLaser::result(const std::string str) {
  _serial.write(str + "\r\n");
  std::string line = _serial.getline();
  if (line.length() >= 3) {
    if (line.substr(0, 3).compare("ERR") == 0) {
      std::string error =
          error_str(error_no(line.substr(0, line.length() - 2)));
      throw std::runtime_error("Query error: " + str + " " + error);
    }
  }
  complete_handshake();
  return line;
}

template <>
const std::string OBISLaser::query(const std::string str) {
  return result(str);
}

template <>
const OBISLaser::State OBISLaser::query(const std::string str) {
  if (result(str).compare("ON\r\n") == 0) return OBISLaser::State::On;
  return OBISLaser::State::Off;
}

template <>
const OBISLaser::DeviceType OBISLaser::query(const std::string str) {
  std::string line = result(str);
  if (line.substr(0, 3).compare("DDL") == 0) {
    return OBISLaser::DeviceType::DDL;
  }
  if (line.substr(0, 4).compare("OPSL") == 0) {
    return OBISLaser::DeviceType::OPSL;
  }
  if (line.substr(0, 4).compare("MINI") == 0) {
    return OBISLaser::DeviceType::Mini;
  }
  if (line.substr(0, 6).compare("MASTER") == 0) {
    return OBISLaser::DeviceType::Master;
  }
  return OBISLaser::DeviceType::Other;
}

template <>
const OBISLaser::SourceType OBISLaser::query(const std::string str) {
  std::string line = result(str);
  if (line.substr(0, 3).compare("CWP") == 0) {
    return OBISLaser::SourceType::ConstantPower;
  }
  if (line.substr(0, 4).compare("CWC") == 0) {
    return OBISLaser::SourceType::ConstantCurrent;
  }
  if (line.substr(0, 4).compare("DIGITAL") == 0) {
    return OBISLaser::SourceType::Digital;
  }
  if (line.substr(0, 6).compare("ANALOG") == 0) {
    return OBISLaser::SourceType::Analog;
  }
  if (line.substr(0, 6).compare("MIXED") == 0) {
    return OBISLaser::SourceType::Mixed;
  }
  if (line.substr(0, 6).compare("DIGSO") == 0) {
    return OBISLaser::SourceType::DIGSO;
  }
  if (line.substr(0, 6).compare("MIXSO") == 0) {
    return OBISLaser::SourceType::MIXSO;
  }
  return OBISLaser::SourceType::ConstantPower;
}
