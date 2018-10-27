#include "board.h"
#include <wiringPi.h>

using namespace hapi;

std::shared_ptr<Board> Board::_board = nullptr;

Board::Board() {}

Board::~Board() {}

std::shared_ptr<Board> Board::instance() {
  if (_board == nullptr) init();
  return _board;
}

void Board::init() {
  wiringPiSetup();
  _board = std::make_shared<Board>();
}

void Board::cleanup() { _board->disarm(); }

void Board::arm() { digitalWrite(_arm_pin, HIGH); }

void Board::disarm() { digitalWrite(_arm_pin, LOW); }

bool Board::is_done() { return digitalRead(_done_pin); }

void Board::set_arm_pin(int pin) {
  pinMode(pin, OUTPUT);
  _arm_pin = pin;
}

void Board::set_done_pin(int pin) {
  pinMode(pin, OUTPUT);
  _done_pin = pin;
}

void Board::set_delay_pins(int pin0, int pin1, int pin2, int pin3) {
  _delay_pins[0] = pin0;
  _delay_pins[1] = pin1;
  _delay_pins[2] = pin2;
  _delay_pins[3] = pin3;
  for (unsigned int i = 0; i < 4; i++) pinMode(_delay_pins[i], OUTPUT);
}

void Board::set_exp_pins(int pin0, int pin1, int pin2, int pin3) {
  _exp_pins[0] = pin0;
  _exp_pins[1] = pin1;
  _exp_pins[2] = pin2;
  _exp_pins[3] = pin3;
  for (unsigned int i = 0; i < 4; i++) pinMode(_exp_pins[i], OUTPUT);
}

void Board::set_pulse_pins(int pin0, int pin1, int pin2, int pin3, int pin4) {
  _pulse_pins[0] = pin0;
  _pulse_pins[1] = pin1;
  _pulse_pins[2] = pin2;
  _pulse_pins[3] = pin3;
  _pulse_pins[4] = pin4;
  for (unsigned int i = 0; i < 5; i++) pinMode(_pulse_pins[i], OUTPUT);
}

void Board::set_delay(int delay) {
  for (unsigned int i = 0; i < 4; i++)
    digitalWrite(_delay_pins[i], (delay > i) & 1);
}

void Board::set_exp(int exp) {
  for (unsigned int i = 0; i < 4; i++)
    digitalWrite(_exp_pins[i], (delay > i) & 1);
}

void Board::set_pulse(int pulse) {
  for (unsigned int i = 0; i < 5; i++)
    digitalWrite(_pulse_pins[i], (delay > i) & 1);
}