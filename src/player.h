// Plays a message on the piezo without blocking the main loop.

#ifndef SRC_PLAYER_H_
#define SRC_PLAYER_H_

#include <cstdint>
#include <mutex>
#include <string>

#include "morse.h"
#include "piezo.h"
#include "settings.h"

namespace cw {

// Steps a MorseSequencer in real time on its own FreeRTOS task, so a busy
// main loop (such as a slow web request) cannot stretch the timing. All
// public methods are safe to call from any task.
class Player {
 public:
  explicit Player(Piezo* piezo);

  Player(const Player&) = delete;
  Player& operator=(const Player&) = delete;

  // Starts the playback task. Call once from setup().
  void Begin();

  // Starts playing `text` with a snapshot of `settings`. Does nothing if a
  // message is already playing.
  void Start(const std::string& text, const Settings& settings);

  // Stops immediately and silences the piezo.
  void Stop();

  // Advances to the next tone or gap once the current one has elapsed.
  // Called by the playback task; public so it can be driven directly.
  void Update(uint32_t now_ms);

  bool is_playing() const;

  // Index of the letter currently sounding (or about to), -1 before start.
  int letter_index() const;

 private:
  static void TaskMain(void* player);

  // Stops playback. The caller must hold `mutex_`.
  void StopLocked();

  mutable std::mutex mutex_;
  Piezo* const piezo_;
  MorseSequencer sequencer_;
  int pitch_hz_ = 0;
  bool loud_ = true;
  bool active_ = false;
  bool step_started_ = false;
  uint32_t step_end_ms_ = 0;
};

}  // namespace cw

#endif  // SRC_PLAYER_H_
