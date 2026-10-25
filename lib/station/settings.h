// Station settings that a Leader can change from the web page.

#ifndef LIB_STATION_SETTINGS_H_
#define LIB_STATION_SETTINGS_H_

#include <optional>
#include <string>

namespace cw {

// Playback settings. Fields are plain ints so that values parsed from a web
// form can never wrap around before ClampSettings() sees them.
struct Settings {
  int wpm = 15;
  int letter_gap_ms = 2500;
  int pitch_hz = 800;
  bool loud = true;
  int repeats = 1;
  bool coach = false;
};

// Returns `settings` with every numeric field forced into its valid range.
Settings ClampSettings(const Settings& settings);

// Parses a decimal integer (optionally negative) from a form field. Returns
// std::nullopt for empty text, any non-digit character, or a magnitude
// above one million.
std::optional<int> ParseIntField(const std::string& text);

}  // namespace cw

#endif  // LIB_STATION_SETTINGS_H_
