#include "player.h"

#include <Arduino.h>

#include <mutex>
#include <optional>
#include <string>

namespace cw {
namespace {

// Above the Arduino loop task (priority 1) on the same core, so playback
// pre-empts web request handling whenever a step is due.
constexpr UBaseType_t kTaskPriority = 2;
constexpr uint32_t kTaskStackBytes = 4096;
constexpr BaseType_t kTaskCore = 1;
constexpr TickType_t kPollTicks = 1;

}  // namespace

Player::Player(Piezo* piezo) : piezo_(piezo) {}

void Player::Begin() {
  xTaskCreatePinnedToCore(&Player::TaskMain, "player", kTaskStackBytes, this,
                          kTaskPriority, nullptr, kTaskCore);
}

void Player::TaskMain(void* player) {
  Player* self = static_cast<Player*>(player);
  for (;;) {
    self->Update(millis());
    vTaskDelay(kPollTicks);
  }
}

void Player::Start(const std::string& text, const Settings& settings) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (active_) return;
  const Settings clamped = ClampSettings(settings);
  sequencer_.Start(text, MakeTiming(clamped.wpm, clamped.letter_gap_ms),
                   clamped.repeats);
  pitch_hz_ = clamped.pitch_hz;
  loud_ = clamped.loud;
  active_ = sequencer_.is_playing();
  step_started_ = false;
}

void Player::Stop() {
  std::lock_guard<std::mutex> lock(mutex_);
  StopLocked();
}

void Player::StopLocked() {
  sequencer_.Stop();
  piezo_->Off();
  active_ = false;
}

bool Player::is_playing() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return active_;
}

int Player::letter_index() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return sequencer_.letter_index();
}

void Player::Update(uint32_t now_ms) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (!active_) return;
  // Signed difference keeps the comparison correct when millis() wraps.
  if (step_started_ && static_cast<int32_t>(now_ms - step_end_ms_) < 0) {
    return;
  }
  const std::optional<Step> step = sequencer_.Next();
  if (!step) {
    StopLocked();
    return;
  }
  if (step->tone_on) {
    piezo_->On(pitch_hz_, loud_);
  } else {
    piezo_->Off();
  }
  // Schedule from the previous step's end, not from `now_ms`, so a late
  // wake-up never accumulates into drift.
  const uint32_t step_start_ms = step_started_ ? step_end_ms_ : now_ms;
  step_end_ms_ = step_start_ms + static_cast<uint32_t>(step->duration_ms);
  step_started_ = true;
}

}  // namespace cw
