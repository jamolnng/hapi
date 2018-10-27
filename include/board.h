#ifndef BOARD_H
#define BOARD_H

#include <memory>

namespace hapi
{
class Board
{
public:
  static std::shared_ptr<Board> instance();
  static void cleanup();

  void arm();
  void disarm();

  void set_arm_pin(int pin);
  void set_delay_pins(int pin0, int pin1, int pin2, int pin3);
  void set_exp_pins(int pin0, int pin1, int pin2, int pin3);
  void set_pulse_pins(int pin0, int pin1, int pin2, int pin3, int pin4);

  void set_delay(int delay);
  void set_exp(int exp);
  void set_pulse(int pulse);

private:
  Board();
  ~Board();

  int _arm_pin;
  int _delay_pins[4];
  int _exp_pins[4];
  int _pulse_pins[5];

  static void init();
  static std::shared_ptr<Board> _board;
};
}
#endif