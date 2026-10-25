// The list of messages the station can play, and the current selection.

#ifndef LIB_STATION_MESSAGES_H_
#define LIB_STATION_MESSAGES_H_

#include <cstdint>
#include <string>
#include <vector>

namespace cw {

constexpr int kMaxMessageLength = 20;
constexpr int kMaxMessages = 30;

// Cleans up typed text into a playable message: uppercase, trimmed, runs of
// spaces collapsed, only A-Z, 0-9 and space kept, at most kMaxMessageLength
// characters. Returns "" if nothing playable is left. `note` receives a
// sentence describing anything removed or shortened, or "" if nothing was.
std::string NormalizeMessage(const std::string& raw, std::string& note);

// What MessageList::Remove() did.
enum class RemoveResult {
  kNotFound,        // Nothing removed: bad index, or the text didn't match.
  kRemoved,         // Removed a message other than the current one.
  kRemovedCurrent,  // Removed the current message; another is now current.
};

// An ordered list of messages with one selected as current. Never empty:
// it falls back to the default messages whenever it would become empty.
class MessageList {
 public:
  // Starts with the default messages, with the first one selected.
  MessageList();

  int size() const { return static_cast<int>(messages_.size()); }
  const std::string& at(int index) const { return messages_[index]; }
  int current() const { return current_; }
  const std::string& CurrentText() const { return messages_[current_]; }

  // Normalises and appends `raw`. Returns false, with a reason in `note`, if
  // nothing playable is left or the list is full. On success `note`
  // describes any clean-up that was applied.
  bool Add(const std::string& raw, std::string& note);

  // Removes the message at `index`, but only if it is still
  // `expected_text`, so a delete from an out-of-date page can't remove the
  // wrong message. Keeps the same message selected where possible.
  RemoveResult Remove(int index, const std::string& expected_text);

  // Selects the next message, wrapping to the first.
  void Next();

  // Selects a message other than the current one using `random_value` as
  // the random source. Does nothing when there is only one message.
  void PickRandom(uint32_t random_value);

  // Selects the message at `index`, but only if it is still
  // `expected_text`. Returns false, changing nothing, otherwise.
  bool Select(int index, const std::string& expected_text);

  // Selects `index`, clamped to the valid range.
  void SetCurrent(int index);

  // Returns all messages joined by newlines, for saving.
  std::string Serialize() const;

  // Replaces the list from Serialize() output, normalising each line and
  // falling back to the defaults if nothing usable remains. Selects the
  // first message.
  void Deserialize(const std::string& data);

 private:
  void LoadDefaults();

  std::vector<std::string> messages_;
  int current_ = 0;
};

}  // namespace cw

#endif  // LIB_STATION_MESSAGES_H_
