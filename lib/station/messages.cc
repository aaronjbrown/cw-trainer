#include "messages.h"

#include <algorithm>
#include <string>

#include "morse.h"

namespace cw {
namespace {

constexpr const char* kDefaultMessages[] = {
    "HI", "SOS", "RADIO", "SCOUT", "JOTA", "CQ", "HELLO", "FUN", "DOG", "CAT"};

bool IsSpace(char c) { return c == ' ' || c == '\t' || c == '\r'; }

void TrimTrailingSpace(std::string& text) {
  while (!text.empty() && text.back() == ' ') text.pop_back();
}

}  // namespace

std::string NormalizeMessage(const std::string& raw, std::string& note) {
  std::string result;
  bool removed = false;
  bool pending_space = false;
  for (char c : raw) {
    if (IsSpace(c)) {
      pending_space = !result.empty();
      continue;
    }
    const char* pattern = MorsePattern(c);
    if (pattern == nullptr) {
      removed = true;
      continue;
    }
    if (pending_space) result += ' ';
    pending_space = false;
    result += (c >= 'a' && c <= 'z') ? static_cast<char>(c - 'a' + 'A') : c;
  }
  const bool shortened = static_cast<int>(result.size()) > kMaxMessageLength;
  if (shortened) {
    result.resize(kMaxMessageLength);
    TrimTrailingSpace(result);
  }
  note.clear();
  if (removed) note += "Removed unsupported characters.";
  if (shortened) {
    if (!note.empty()) note += ' ';
    note +=
        "Shortened to " + std::to_string(kMaxMessageLength) + " characters.";
  }
  return result;
}

MessageList::MessageList() { LoadDefaults(); }

bool MessageList::Add(const std::string& raw, std::string& note) {
  if (size() >= kMaxMessages) {
    note = "The list is full (" + std::to_string(kMaxMessages) +
           " messages). Delete one first.";
    return false;
  }
  const std::string message = NormalizeMessage(raw, note);
  if (message.empty()) {
    note = "Nothing playable in that message. Use letters and numbers.";
    return false;
  }
  messages_.push_back(message);
  return true;
}

RemoveResult MessageList::Remove(int index, const std::string& expected_text) {
  if (index < 0 || index >= size() || messages_[index] != expected_text) {
    return RemoveResult::kNotFound;
  }
  const RemoveResult result = index == current_ ? RemoveResult::kRemovedCurrent
                                                : RemoveResult::kRemoved;
  messages_.erase(messages_.begin() + index);
  if (messages_.empty()) {
    LoadDefaults();
    return result;
  }
  if (index < current_) --current_;
  current_ = std::min(current_, size() - 1);
  return result;
}

void MessageList::Next() { current_ = (current_ + 1) % size(); }

void MessageList::PickRandom(uint32_t random_value) {
  if (size() < 2) return;
  const uint32_t others = static_cast<uint32_t>(size() - 1);
  current_ = static_cast<int>((current_ + 1 + random_value % others) % size());
}

bool MessageList::Select(int index, const std::string& expected_text) {
  if (index < 0 || index >= size() || messages_[index] != expected_text) {
    return false;
  }
  current_ = index;
  return true;
}

void MessageList::SetCurrent(int index) {
  current_ = std::clamp(index, 0, size() - 1);
}

std::string MessageList::Serialize() const {
  std::string data;
  for (const std::string& message : messages_) {
    if (!data.empty()) data += '\n';
    data += message;
  }
  return data;
}

void MessageList::Deserialize(const std::string& data) {
  messages_.clear();
  current_ = 0;
  size_t start = 0;
  while (start <= data.size() && size() < kMaxMessages) {
    size_t end = data.find('\n', start);
    if (end == std::string::npos) end = data.size();
    std::string note;
    const std::string message =
        NormalizeMessage(data.substr(start, end - start), note);
    if (!message.empty()) messages_.push_back(message);
    start = end + 1;
  }
  if (messages_.empty()) LoadDefaults();
}

void MessageList::LoadDefaults() {
  messages_.assign(std::begin(kDefaultMessages), std::end(kDefaultMessages));
  current_ = 0;
}

}  // namespace cw
