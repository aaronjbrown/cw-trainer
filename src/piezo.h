// Drives the piezo transducer with a square-wave tone, plus a status LED.

#ifndef SRC_PIEZO_H_
#define SRC_PIEZO_H_

namespace cw {

// Push-pull piezo driver. In loud mode pin B carries the same LEDC signal as
// pin A, inverted, doubling the voltage swing across the piezo. In quiet
// mode pin B is held low. Both pins are low whenever no tone is playing.
class Piezo {
 public:
  Piezo(int pin_a, int pin_b, int led_pin);

  Piezo(const Piezo&) = delete;
  Piezo& operator=(const Piezo&) = delete;

  // Configures the pins and LEDC channel. Leaves everything silent and low.
  void Begin();

  // Starts a tone at `hz`, push-pull if `loud`, and lights the LED.
  void On(int hz, bool loud);

  // Silences the tone, drives both pins low and turns the LED off.
  void Off();

 private:
  const int pin_a_;
  const int pin_b_;
  const int led_pin_;
};

}  // namespace cw

#endif  // SRC_PIEZO_H_
