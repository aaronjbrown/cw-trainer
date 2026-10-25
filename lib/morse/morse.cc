#include "morse.h"

#include <algorithm>
#include <string>

namespace cw {
namespace {

constexpr const char* kLetterPatterns[26] = {
    ".-",   "-...", "-.-.", "-..",  ".",   "..-.", "--.",  "....", "..",
    ".---", "-.-",  ".-..", "--",   "-.",  "---",  ".--.", "--.-", ".-.",
    "...",  "-",    "..-",  "...-", ".--", "-..-", "-.--", "--.."};

constexpr const char* kDigitPatterns[10] = {"-----", ".----", "..---", "...--",
                                            "....-", ".....", "-....", "--...",
                                            "---..", "----."};

// Standard "PARIS" timing: one dit lasts 1200 / wpm milliseconds.
constexpr int kMsPerDitAtOneWpm = 1200;
constexpr int kDahDits = 3;
constexpr int kMinRepeatGapMs = 3000;

}  // namespace

const char* MorsePattern(char c) {
  if (c >= 'a' && c <= 'z') c = static_cast<char>(c - 'a' + 'A');
  if (c >= 'A' && c <= 'Z') return kLetterPatterns[c - 'A'];
  if (c >= '0' && c <= '9') return kDigitPatterns[c - '0'];
  return nullptr;
}

Timing MakeTiming(int wpm, int letter_gap_ms) {
  Timing timing;
  timing.dit_ms = kMsPerDitAtOneWpm / wpm;
  timing.letter_gap_ms = letter_gap_ms;
  timing.word_gap_ms = 2 * letter_gap_ms;
  timing.repeat_gap_ms = std::max(kMinRepeatGapMs, timing.word_gap_ms);
  return timing;
}

void MorseSequencer::Start(const std::string& text, const Timing& timing,
                           int repeats) {
  text_ = text;
  timing_ = timing;
  repeats_ = repeats;
  repeat_ = 0;
  element_index_ = 0;
  pending_gap_ms_ = 0;
  finished_ = false;
  letter_index_ = -1;
  bool crossed_space = false;
  char_index_ = FindPlayable(0, crossed_space);
  playing_ = char_index_ >= 0 && repeats_ > 0;
}

void MorseSequencer::Stop() { playing_ = false; }

std::optional<Step> MorseSequencer::Next() {
  if (!playing_) return std::nullopt;
  if (finished_) {
    playing_ = false;
    return std::nullopt;
  }
  if (pending_gap_ms_ > 0) {
    // A gap belongs to the letter just sent, so coach mode shows the next
    // letter only once it starts to sound.
    const Step gap{false, pending_gap_ms_, letter_index_};
    pending_gap_ms_ = 0;
    return gap;
  }
  const char element = MorsePattern(text_[char_index_])[element_index_];
  const int dits = element == '-' ? kDahDits : 1;
  const Step tone{true, dits * timing_.dit_ms, char_index_};
  letter_index_ = char_index_;
  AdvanceAfterTone();
  return tone;
}

int MorseSequencer::FindPlayable(int from, bool& crossed_space) const {
  for (int i = from; i < static_cast<int>(text_.size()); ++i) {
    if (MorsePattern(text_[i]) != nullptr) return i;
    if (text_[i] == ' ') crossed_space = true;
  }
  return -1;
}

void MorseSequencer::AdvanceAfterTone() {
  const char* pattern = MorsePattern(text_[char_index_]);
  ++element_index_;
  if (pattern[element_index_] != '\0') {
    pending_gap_ms_ = timing_.dit_ms;
    return;
  }
  element_index_ = 0;
  bool crossed_space = false;
  const int next = FindPlayable(char_index_ + 1, crossed_space);
  if (next >= 0) {
    char_index_ = next;
    pending_gap_ms_ =
        crossed_space ? timing_.word_gap_ms : timing_.letter_gap_ms;
    return;
  }
  if (++repeat_ < repeats_) {
    char_index_ = FindPlayable(0, crossed_space);
    pending_gap_ms_ = timing_.repeat_gap_ms;
    return;
  }
  finished_ = true;
}

}  // namespace cw
