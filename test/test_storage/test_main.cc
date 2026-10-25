// On-device tests for saving settings and messages to flash (NVS).

#include <Arduino.h>
#include <Preferences.h>
#include <unity.h>

#include <string>

#include "messages.h"
#include "settings.h"
#include "storage.h"

namespace {

// Writes raw values the way a corrupted or older firmware might have.
void WriteRaw(const char* key, int value) {
  Preferences preferences;
  preferences.begin("cwtrainer", /*readOnly=*/false);
  preferences.putInt(key, value);
  preferences.end();
}

void WriteRawString(const char* key, const char* value) {
  Preferences preferences;
  preferences.begin("cwtrainer", /*readOnly=*/false);
  preferences.putString(key, value);
  preferences.end();
}

}  // namespace

void setUp() { cw::ClearState(); }
void tearDown() {}

void TestLoadEmptyGivesDefaults() {
  cw::Settings settings;
  settings.wpm = 22;
  cw::MessageList messages;
  messages.Deserialize("DOG");
  cw::LoadState(settings, messages);
  TEST_ASSERT_EQUAL_INT(15, settings.wpm);
  TEST_ASSERT_EQUAL_INT(2500, settings.letter_gap_ms);
  TEST_ASSERT_EQUAL_INT(10, messages.size());
  TEST_ASSERT_EQUAL_INT(0, messages.current());
}

void TestSettingsRoundTrip() {
  cw::Settings saved;
  saved.wpm = 20;
  saved.letter_gap_ms = 4000;
  saved.pitch_hz = 1200;
  saved.loud = false;
  saved.repeats = 2;
  saved.coach = true;
  cw::SaveSettings(saved);
  cw::Settings loaded;
  cw::MessageList messages;
  cw::LoadState(loaded, messages);
  TEST_ASSERT_EQUAL_INT(20, loaded.wpm);
  TEST_ASSERT_EQUAL_INT(4000, loaded.letter_gap_ms);
  TEST_ASSERT_EQUAL_INT(1200, loaded.pitch_hz);
  TEST_ASSERT_FALSE(loaded.loud);
  TEST_ASSERT_EQUAL_INT(2, loaded.repeats);
  TEST_ASSERT_TRUE(loaded.coach);
}

void TestMessagesRoundTrip() {
  cw::MessageList saved;
  std::string note;
  saved.Add("HELLO SCOUTS", note);
  saved.SetCurrent(10);
  cw::SaveMessages(saved);
  cw::Settings settings;
  cw::MessageList loaded;
  cw::LoadState(settings, loaded);
  TEST_ASSERT_EQUAL_INT(11, loaded.size());
  TEST_ASSERT_EQUAL_STRING("HELLO SCOUTS", loaded.CurrentText().c_str());
}

void TestCorruptValuesClamped() {
  WriteRaw("wpm", 0);
  WriteRawString("msgs", "!!");
  cw::Settings settings;
  cw::MessageList messages;
  cw::LoadState(settings, messages);
  TEST_ASSERT_EQUAL_INT(5, settings.wpm);
  TEST_ASSERT_EQUAL_INT(10, messages.size());
  TEST_ASSERT_EQUAL_STRING("HI", messages.at(0).c_str());
}

void TestSavedCurrentOutOfRange() {
  WriteRaw("cur", 99);
  cw::Settings settings;
  cw::MessageList messages;
  cw::LoadState(settings, messages);
  TEST_ASSERT_EQUAL_INT(9, messages.current());
}

void setup() {
  delay(2000);  // Give the serial monitor time to connect.
  UNITY_BEGIN();
  RUN_TEST(TestLoadEmptyGivesDefaults);
  RUN_TEST(TestSettingsRoundTrip);
  RUN_TEST(TestMessagesRoundTrip);
  RUN_TEST(TestCorruptValuesClamped);
  RUN_TEST(TestSavedCurrentOutOfRange);
  cw::ClearState();
  UNITY_END();
}

void loop() {}
