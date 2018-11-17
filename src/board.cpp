#include "board.h"

#include <thread>

#include <wiringPi.h>
#include <wiringPiI2C.h>

using namespace hapi;

#define HAPI_PIN_DELAY std::chrono::microseconds(1)

Board::Board() {
  // initialize wiringPi and create the board instance
  wiringPiSetup();
  _i2c = wiringPiI2CSetup(0x51);
  piHiPri(99);
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

void Board::set_arm_pin(int pin) {
  pinMode(pin, OUTPUT);
  _arm_pin = pin;
}

void Board::set_done_pin(int pin) {
  pinMode(pin, INPUT);
  _done_pin = pin;
}

void Board::set_delay_pins(int pin0, int pin1, int pin2, int pin3) {
  // assign pins
  _delay_pins[0] = pin0;
  _delay_pins[1] = pin1;
  _delay_pins[2] = pin2;
  _delay_pins[3] = pin3;
  // set pin modes to output
  for (unsigned int i = 0; i < 4; i++) pinMode(_delay_pins[i], OUTPUT);
}

void Board::set_exp_pins(int pin0, int pin1, int pin2, int pin3) {
  // assign pins
  _exp_pins[0] = pin0;
  _exp_pins[1] = pin1;
  _exp_pins[2] = pin2;
  _exp_pins[3] = pin3;
  // set pin modes to output
  for (unsigned int i = 0; i < 4; i++) pinMode(_exp_pins[i], OUTPUT);
}

void Board::set_pulse_pins(int pin0, int pin1, int pin2, int pin3, int pin4) {
  // assign pins
  _pulse_pins[0] = pin0;
  _pulse_pins[1] = pin1;
  _pulse_pins[2] = pin2;
  _pulse_pins[3] = pin3;
  _pulse_pins[4] = pin4;
  // set pin mode to output
  for (unsigned int i = 0; i < 5; i++) pinMode(_pulse_pins[i], OUTPUT);
}

void Board::set_delay(unsigned int delay) {
  for (unsigned int i = 0; i < 4; i++)
    digitalWrite(_delay_pins[i], (delay >> i) & 1);
  std::this_thread::sleep_for(HAPI_PIN_DELAY);
}

void Board::set_exp(unsigned int exp) {
  for (unsigned int i = 0; i < 4; i++)
    digitalWrite(_exp_pins[i], (exp >> i) & 1);
  std::this_thread::sleep_for(HAPI_PIN_DELAY);
}

void Board::set_pulse(unsigned int pulse) {
  for (unsigned int i = 0; i < 5; i++)
    digitalWrite(_pulse_pins[i], (pulse >> i) & 1);
  std::this_thread::sleep_for(HAPI_PIN_DELAY);
}
void Board::set_pmt_gain(int gain_byte) {
  wiringPiI2CWriteReg8(_i2c, 0x00, gain_byte);
  std::this_thread::sleep_for(std::chrono::seconds(1));
}

void Board::set_pmt_threshold(int threshold_byte) {
  wiringPiI2CWriteReg8(_i2c, 0x01, threshold_byte);
  std::this_thread::sleep_for(std::chrono::seconds(1));
}
