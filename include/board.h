#ifndef HAPI_BOARD_H
#define HAPI_BOARD_H

#include <memory>

namespace hapi {
// Controls the HAPI-E board
class Board {
 public:
  // the board instance
  static Board& instance() {
    static Board _instance;
    return _instance;
  }

  // arms the board so it can capture images
  void arm();
  // disarms the board
  void disarm();

  // returns true if the board has captured an image
  bool is_done();

  // clears the state of the board
  void reset();

  // sets the pin used to arm the board
  void set_arm_pin(int pin);
  // sets the pin used to determine if the board has captured an image
  void set_done_pin(int pin);
  // sets the pins used to determine the delay
  // binary coded decimal
  void set_delay_pins(int pin0, int pin1, int pin2, int pin3);
  // sets the pins used to determine the exposure time
  // binary coded decimal
  void set_exp_pins(int pin0, int pin1, int pin2, int pin3);
  // sets the pins used to determine the pulse width
  // binary coded decimal
  void set_pulse_pins(int pin0, int pin1, int pin2, int pin3, int pin4);

  // sets the delay in microseconds
  void set_delay(unsigned int delay);
  // sets the exposure in microseconds
  void set_exp(unsigned int exp);
  // sets the pulse width in tens of nanoseconds
  void set_pulse(unsigned int pulse);

 private:
  Board();

  int _arm_pin{0};
  int _done_pin{0};
  int _delay_pins[4];
  int _exp_pins[4];
  int _pulse_pins[5];
};
}  // namespace hapi
#endif
