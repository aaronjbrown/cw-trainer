#include "button.h"

#include <Arduino.h>

namespace cw {
namespace {

constexpr uint32_t kDebounceMs = 30;

}  // namespace

Button::Button(int pin) : pin_(pin) {}

void Button::Begin() { pinMode(pin_, INPUT_PULLUP); }

bool Button::WasPressed(uint32_t now_ms) {
  const bool pressed = digitalRead(pin_) == LOW;
  if (pressed != raw_pressed_) {
    raw_pressed_ = pressed;
    last_change_ms_ = now_ms;
  }
  if (now_ms - last_change_ms_ < kDebounceMs ||
      raw_pressed_ == stable_pressed_) {
    return false;
  }
  stable_pressed_ = raw_pressed_;
  return stable_pressed_;
}

}  // namespace cw
