// Saves and restores settings and messages in the ESP32's flash (NVS), so
// they survive the station being unplugged.

#ifndef SRC_STORAGE_H_
#define SRC_STORAGE_H_

#include "messages.h"
#include "settings.h"

namespace cw {

// Loads saved state. Missing values fall back to defaults and every value
// is clamped, so corrupt or partial data still gives a usable station.
void LoadState(Settings& settings, MessageList& messages);

// Saves all settings.
void SaveSettings(const Settings& settings);

// Saves the message list and which message is selected.
void SaveMessages(const MessageList& messages);

// Erases everything this firmware has saved.
void ClearState();

}  // namespace cw

#endif  // SRC_STORAGE_H_
