#include "station_json.h"

#include <cstdio>
#include <map>
#include <optional>
#include <string>

namespace cw {
namespace {

const char* Bool(bool value) { return value ? "true" : "false"; }

std::string Quoted(const std::string& text) {
  return "\"" + JsonEscape(text) + "\"";
}

// Returns the parsed value of `name` in `fields`, if present and valid.
std::optional<int> Field(const std::map<std::string, std::string>& fields,
                         const char* name) {
  const auto it = fields.find(name);
  if (it == fields.end()) return std::nullopt;
  return ParseIntField(it->second);
}

}  // namespace

std::string JsonEscape(const std::string& text) {
  std::string escaped;
  for (char c : text) {
    switch (c) {
      case '"':
        escaped += "\\\"";
        break;
      case '\\':
        escaped += "\\\\";
        break;
      case '\n':
        escaped += "\\n";
        break;
      default:
        if (static_cast<unsigned char>(c) < 0x20) {
          char buffer[8];
          std::snprintf(buffer, sizeof(buffer), "\\u%04x", c);
          escaped += buffer;
        } else {
          escaped += c;
        }
    }
  }
  return escaped;
}

std::string StatusJson(const Settings& settings, const MessageList& messages,
                       bool playing, bool revealed, int letter) {
  std::string json = "{\"playing\":";
  json += Bool(playing);
  json += ",\"index\":" + std::to_string(messages.current());
  json += ",\"count\":" + std::to_string(messages.size());
  json += ",\"text\":" + Quoted(messages.CurrentText());
  json += ",\"revealed\":";
  json += Bool(revealed);
  json += ",\"letter\":" + std::to_string(letter);
  json += ",\"settings\":{\"wpm\":" + std::to_string(settings.wpm);
  json += ",\"gapMs\":" + std::to_string(settings.letter_gap_ms);
  json += ",\"pitch\":" + std::to_string(settings.pitch_hz);
  json += ",\"loud\":";
  json += Bool(settings.loud);
  json += ",\"repeats\":" + std::to_string(settings.repeats);
  json += ",\"coach\":";
  json += Bool(settings.coach);
  json += "}}";
  return json;
}

std::string MessagesJson(const MessageList& messages) {
  std::string json = "{\"messages\":[";
  for (int i = 0; i < messages.size(); ++i) {
    if (i > 0) json += ',';
    json += Quoted(messages.at(i));
  }
  json += "],\"current\":" + std::to_string(messages.current()) + "}";
  return json;
}

Settings UpdateSettings(const Settings& current,
                        const std::map<std::string, std::string>& fields) {
  Settings updated = current;
  if (const auto value = Field(fields, "wpm")) updated.wpm = *value;
  if (const auto value = Field(fields, "gapMs")) {
    updated.letter_gap_ms = *value;
  }
  if (const auto value = Field(fields, "pitch")) updated.pitch_hz = *value;
  if (const auto value = Field(fields, "loud")) updated.loud = *value != 0;
  if (const auto value = Field(fields, "repeats")) updated.repeats = *value;
  if (const auto value = Field(fields, "coach")) updated.coach = *value != 0;
  return ClampSettings(updated);
}

}  // namespace cw
