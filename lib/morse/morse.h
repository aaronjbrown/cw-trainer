// International Morse code table and a Farnsworth-style timing sequencer.
//
// The sequencer turns a message into a series of tone/silence steps. Letters
// are sent at a normal character speed, while the gaps between letters are
// stretched so that a beginner can look each letter up on a chart.

#ifndef LIB_MORSE_MORSE_H_
#define LIB_MORSE_MORSE_H_

#include <optional>
#include <string>

namespace cw {

// Returns the dot/dash pattern for `c` (e.g. ".-" for 'A'), case-insensitive.
// Returns nullptr for space and for characters with no Morse code.
const char* MorsePattern(char c);

// Durations, in milliseconds, used to send a message.
struct Timing {
  int dit_ms;
  int letter_gap_ms;
  int word_gap_ms;
  int repeat_gap_ms;
};

// Builds timing from a character speed in words per minute and the silence
// between letters. Inputs are expected to be clamped to valid ranges already.
Timing MakeTiming(int wpm, int letter_gap_ms);

// One tone or silence. `letter_index` is the index into the message of the
// letter sounding now, or of the letter just sent during a gap.
struct Step {
  bool tone_on;
  int duration_ms;
  int letter_index;
};

// Produces the steps for a message lazily, one at a time.
class MorseSequencer {
 public:
  MorseSequencer() = default;

  // Begins sending `text` `repeats` times. Restarts any message in progress.
  void Start(const std::string& text, const Timing& timing, int repeats);

  // Abandons the message; Next() returns std::nullopt until the next Start().
  void Stop();

  // Returns the next step, or std::nullopt once finished or stopped.
  std::optional<Step> Next();

  bool is_playing() const { return playing_; }

  // Index of the letter most recently reached, or -1 before the first step.
  int letter_index() const { return letter_index_; }

 private:
  // Returns the index of the first character at or after `from` that has a
  // Morse pattern, or -1. Sets `crossed_space` if a space was skipped.
  int FindPlayable(int from, bool& crossed_space) const;

  // Decides what follows the tone just sent and stores it as the pending gap.
  void AdvanceAfterTone();

  std::string text_;
  Timing timing_{};
  int repeats_ = 0;
  int repeat_ = 0;
  int char_index_ = 0;
  int element_index_ = 0;
  int pending_gap_ms_ = 0;
  bool finished_ = false;
  bool playing_ = false;
  int letter_index_ = -1;
};

}  // namespace cw

#endif  // LIB_MORSE_MORSE_H_
