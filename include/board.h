#ifndef HAPI_BOARD_H
#define HAPI_BOARD_H

#include <stdexcept>

namespace hapi {
// Controls the HAPI-E board
class Board {
 public:
  enum TriggerSource { PMT = 0, PI = 1 };
  // the board instance
  static Board& instance() {
    static Board _instance;
    return _instance;
  }
  // Triggers the HAPI-E board if the trigger source is set to the PI, if not
  // throws an error
  void trigger();
  // arms the board so it can capture images
  void arm();
  // disarms the board
  void disarm();

  // returns true if the board has captured an image
  bool is_done();

  // clears the state of the board
  void reset();

  // sets the trigger source to either come from the PMT or the PI.
  void set_trigger_source(TriggerSource source);
  // sets the delay in microseconds
  void set_delay(unsigned int delay);
  // sets the exposure in microseconds
  void set_exp(unsigned int exp);
  // sets the pulse width in tens of nanoseconds
  void set_pulse(unsigned int pulse);

  // sets the pmt gain voltage 0.5-1.1V
  // steps of 0.6/256 volts 0x00-0xFF
  void set_pmt_gain(int gain_byte);
  // sets the pmt threshold voltage 0.0-3.3V
  // steps of 3.3/256 volts 0x00-0xFF
  void set_pmt_threshold(int threshold_byte);

 private:
  Board();

  int _arm_pin{26};
  int _done_pin{23};
  int _delay_pins[4] = {7, 0, 1, 2};
  int _exp_pins[4] = {13, 6, 14, 10};
  int _pulse_pins[5] = {24, 27, 25, 28, 29};

  int _i2c;

  int _trigger_pin{3};
  int _trigger_source_pin{4};
  TriggerSource _trigger_source{TriggerSource::PMT};
};
}  // namespace hapi
#endif
