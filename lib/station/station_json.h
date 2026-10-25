// JSON responses and form handling for the station's web API. Kept free of
// Arduino code so it can be tested on the host.

#ifndef LIB_STATION_STATION_JSON_H_
#define LIB_STATION_STATION_JSON_H_

#include <map>
#include <string>

#include "messages.h"
#include "settings.h"

namespace cw {

// Escapes `text` for use inside a JSON string literal.
std::string JsonEscape(const std::string& text);

// Builds the /api/status response.
std::string StatusJson(const Settings& settings, const MessageList& messages,
                       bool playing, bool revealed, int letter);

// Builds the /api/messages response: the list and the selected index.
std::string MessagesJson(const MessageList& messages);

// Applies submitted form fields (wpm, gapMs, pitch, loud, repeats, coach) to
// `current`. Missing or unparseable fields keep their current value, and the
// result is clamped.
Settings UpdateSettings(const Settings& current,
                        const std::map<std::string, std::string>& fields);

}  // namespace cw

#endif  // LIB_STATION_STATION_JSON_H_
