#include "board.h"

#include <thread>

#include <wiringPi.h>
#include <wiringPiI2C.h>

using namespace hapi;

#define HAPI_PIN_DELAY std::chrono::milliseconds(100)

Board::Board() {
  // initialize wiringPi and create the board instance
  wiringPiSetup();
  _i2c = wiringPiI2CSetup(0x51);
  piHiPri(99);
  pinMode(_arm_pin, OUTPUT);
  pinMode(_done_pin, INPUT);
  pinMode(_trigger_pin, OUTPUT);
  pinMode(_trigger_source_pin, OUTPUT);
  for (unsigned int i = 0; i < 4; i++) {
    pinMode(_delay_pins[i], OUTPUT);
  }
  for (unsigned int i = 0; i < 4; i++) {
    pinMode(_exp_pins[i], OUTPUT);
  }
  for (unsigned int i = 0; i < 5; i++) {
    pinMode(_pulse_pins[i], OUTPUT);
  }
  reset();
  set_trigger_source(_trigger_source);
}

void Board::trigger() {
  if (_trigger_source == Board::TriggerSource::PI) {
    digitalWrite(_trigger_pin, HIGH);
    std::this_thread::sleep_for(HAPI_PIN_DELAY);
    digitalWrite(_trigger_pin, LOW);
    std::this_thread::sleep_for(HAPI_PIN_DELAY);
  }
}

void Board::arm() {
  digitalWrite(_arm_pin, HIGH);
  std::this_thread::sleep_for(HAPI_PIN_DELAY);
}

void Board::disarm() {
  digitalWrite(_arm_pin, LOW);
  std::this_thread::sleep_for(HAPI_PIN_DELAY);
}

bool Board::is_done() { return digitalRead(_done_pin); }

void Board::reset() {
  arm();
  disarm();
}

void Board::set_trigger_source(Board::TriggerSource source) {
  _trigger_source = source;
  digitalWrite(_trigger_source_pin, (int)source);
  std::this_thread::sleep_for(HAPI_PIN_DELAY);
}

void Board::set_delay(unsigned int delay) {
  for (unsigned int i = 0; i < 4; i++) {
    digitalWrite(_delay_pins[i], (delay >> i) & 1);
  }
  std::this_thread::sleep_for(HAPI_PIN_DELAY);
}

void Board::set_exp(unsigned int exp) {
  for (unsigned int i = 0; i < 4; i++) {
    digitalWrite(_exp_pins[i], (exp >> i) & 1);
  }
  std::this_thread::sleep_for(HAPI_PIN_DELAY);
}

void Board::set_pulse(unsigned int pulse) {
  for (unsigned int i = 0; i < 5; i++) {
    digitalWrite(_pulse_pins[i], (pulse >> i) & 1);
  }
  std::this_thread::sleep_for(HAPI_PIN_DELAY);
}
void Board::set_pmt_gain(int gain_byte) {
  wiringPiI2CWriteReg8(_i2c, 0x00, gain_byte);
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

void Board::set_pmt_threshold(int threshold_byte) {
  wiringPiI2CWriteReg8(_i2c, 0x01, threshold_byte);
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
}
