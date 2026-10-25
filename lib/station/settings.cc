#include "settings.h"

#include <algorithm>
#include <string>

namespace cw {
namespace {

constexpr int kMinWpm = 5;
constexpr int kMaxWpm = 25;
constexpr int kMinLetterGapMs = 500;
constexpr int kMaxLetterGapMs = 6000;
constexpr int kMinPitchHz = 400;
constexpr int kMaxPitchHz = 4000;
constexpr int kMinRepeats = 1;
constexpr int kMaxRepeats = 3;
constexpr int kMaxFieldMagnitude = 1000000;

}  // namespace

Settings ClampSettings(const Settings& settings) {
  Settings clamped = settings;
  clamped.wpm = std::clamp(settings.wpm, kMinWpm, kMaxWpm);
  clamped.letter_gap_ms =
      std::clamp(settings.letter_gap_ms, kMinLetterGapMs, kMaxLetterGapMs);
  clamped.pitch_hz = std::clamp(settings.pitch_hz, kMinPitchHz, kMaxPitchHz);
  clamped.repeats = std::clamp(settings.repeats, kMinRepeats, kMaxRepeats);
  return clamped;
}

std::optional<int> ParseIntField(const std::string& text) {
  size_t i = 0;
  const bool negative = !text.empty() && text[0] == '-';
  if (negative) i = 1;
  if (i == text.size()) return std::nullopt;
  int value = 0;
  for (; i < text.size(); ++i) {
    if (text[i] < '0' || text[i] > '9') return std::nullopt;
    value = value * 10 + (text[i] - '0');
    if (value > kMaxFieldMagnitude) return std::nullopt;
  }
  return negative ? -value : value;
}

}  // namespace cw
