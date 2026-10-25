#include "piezo.h"

#include <Arduino.h>
#include <esp32-hal-matrix.h>
#include <soc/gpio_sig_map.h>

namespace cw {
namespace {

// LEDC channel 0 is high-speed channel 0, so its output signal in the GPIO
// matrix is LEDC_HS_SIG_OUT0_IDX.
constexpr uint8_t kLedcChannel = 0;
constexpr uint8_t kLedcSignal = LEDC_HS_SIG_OUT0_IDX;
constexpr uint8_t kResolutionBits = 8;
constexpr uint32_t kHalfDuty = 128;  // 50% duty at 8-bit resolution.
constexpr int kInitialHz = 800;

}  // namespace

Piezo::Piezo(int pin_a, int pin_b, int led_pin)
    : pin_a_(pin_a), pin_b_(pin_b), led_pin_(led_pin) {}

void Piezo::Begin() {
  pinMode(pin_b_, OUTPUT);
  digitalWrite(pin_b_, LOW);
  pinMode(led_pin_, OUTPUT);
  digitalWrite(led_pin_, LOW);
  ledcSetup(kLedcChannel, kInitialHz, kResolutionBits);
  ledcAttachPin(pin_a_, kLedcChannel);
  ledcWrite(kLedcChannel, 0);
}

void Piezo::On(int hz, bool loud) {
  ledcChangeFrequency(kLedcChannel, hz, kResolutionBits);
  if (loud) {
    pinMatrixOutAttach(pin_b_, kLedcSignal, /*invertOut=*/true,
                       /*invertEnable=*/false);
  } else {
    pinMatrixOutDetach(pin_b_, false, false);
    digitalWrite(pin_b_, LOW);
  }
  ledcWrite(kLedcChannel, kHalfDuty);
  digitalWrite(led_pin_, HIGH);
}

void Piezo::Off() {
  ledcWrite(kLedcChannel, 0);
  pinMatrixOutDetach(pin_b_, false, false);
  digitalWrite(pin_b_, LOW);
  digitalWrite(led_pin_, LOW);
}

}  // namespace cw
