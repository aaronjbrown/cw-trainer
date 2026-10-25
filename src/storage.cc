#include "storage.h"

#include <Preferences.h>

#include <string>

namespace cw {
namespace {

constexpr char kNamespace[] = "cwtrainer";
constexpr char kWpmKey[] = "wpm";
constexpr char kLetterGapKey[] = "gap";
constexpr char kPitchKey[] = "pitch";
constexpr char kLoudKey[] = "loud";
constexpr char kRepeatsKey[] = "reps";
constexpr char kCoachKey[] = "coach";
constexpr char kMessagesKey[] = "msgs";
constexpr char kCurrentKey[] = "cur";

}  // namespace

void LoadState(Settings& settings, MessageList& messages) {
  Preferences preferences;
  preferences.begin(kNamespace, /*readOnly=*/false);
  const Settings defaults;
  Settings loaded;
  loaded.wpm = preferences.getInt(kWpmKey, defaults.wpm);
  loaded.letter_gap_ms =
      preferences.getInt(kLetterGapKey, defaults.letter_gap_ms);
  loaded.pitch_hz = preferences.getInt(kPitchKey, defaults.pitch_hz);
  loaded.loud = preferences.getBool(kLoudKey, defaults.loud);
  loaded.repeats = preferences.getInt(kRepeatsKey, defaults.repeats);
  loaded.coach = preferences.getBool(kCoachKey, defaults.coach);
  settings = ClampSettings(loaded);

  std::string data;
  if (preferences.isKey(kMessagesKey)) {
    data = preferences.getString(kMessagesKey).c_str();
  }
  messages.Deserialize(data);
  messages.SetCurrent(preferences.getInt(kCurrentKey, 0));
  preferences.end();
}

void SaveSettings(const Settings& settings) {
  Preferences preferences;
  preferences.begin(kNamespace, /*readOnly=*/false);
  preferences.putInt(kWpmKey, settings.wpm);
  preferences.putInt(kLetterGapKey, settings.letter_gap_ms);
  preferences.putInt(kPitchKey, settings.pitch_hz);
  preferences.putBool(kLoudKey, settings.loud);
  preferences.putInt(kRepeatsKey, settings.repeats);
  preferences.putBool(kCoachKey, settings.coach);
  preferences.end();
}

void SaveMessages(const MessageList& messages) {
  Preferences preferences;
  preferences.begin(kNamespace, /*readOnly=*/false);
  preferences.putString(kMessagesKey, messages.Serialize().c_str());
  preferences.putInt(kCurrentKey, messages.current());
  preferences.end();
}

void ClearState() {
  Preferences preferences;
  preferences.begin(kNamespace, /*readOnly=*/false);
  preferences.clear();
  preferences.end();
}

}  // namespace cw
