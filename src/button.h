// Debounced push button (the Scout "play" button).

#ifndef SRC_BUTTON_H_
#define SRC_BUTTON_H_

#include <cstdint>

namespace cw {

// An active-low button with the internal pull-up enabled.
class Button {
 public:
  explicit Button(int pin);

  // Configures the pin as an input with pull-up.
  void Begin();

  // Returns true exactly once per press, after the input has been stable
  // for the debounce time. Call from every loop().
  bool WasPressed(uint32_t now_ms);

 private:
  const int pin_;
  bool raw_pressed_ = false;
  bool stable_pressed_ = false;
  uint32_t last_change_ms_ = 0;
};

}  // namespace cw

#endif  // SRC_BUTTON_H_
