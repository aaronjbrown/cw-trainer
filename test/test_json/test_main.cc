// Host tests for the web API's JSON output and settings-form handling.

#include <unity.h>

#include <map>
#include <string>

#include "messages.h"
#include "settings.h"
#include "station_json.h"

void setUp() {}
void tearDown() {}

void TestJsonEscape() {
  TEST_ASSERT_EQUAL_STRING("HI", cw::JsonEscape("HI").c_str());
  TEST_ASSERT_EQUAL_STRING("a\\\"b\\\\c\\n",
                           cw::JsonEscape("a\"b\\c\n").c_str());
  TEST_ASSERT_EQUAL_STRING("\\u0001", cw::JsonEscape("\x01").c_str());
}

void TestStatusJson() {
  cw::Settings settings;
  cw::MessageList messages;
  messages.SetCurrent(2);
  const std::string json =
      cw::StatusJson(settings, messages, /*playing=*/true, /*revealed=*/false,
                     /*letter=*/1);
  TEST_ASSERT_EQUAL_STRING(
      "{\"playing\":true,\"index\":2,\"count\":10,\"text\":\"RADIO\","
      "\"revealed\":false,\"letter\":1,\"settings\":{\"wpm\":15,"
      "\"gapMs\":2500,\"pitch\":800,\"loud\":true,\"repeats\":1,"
      "\"coach\":false}}",
      json.c_str());
}

void TestMessagesJson() {
  cw::MessageList messages;
  messages.Deserialize("DOG\nCAT");
  messages.SetCurrent(1);
  TEST_ASSERT_EQUAL_STRING("{\"messages\":[\"DOG\",\"CAT\"],\"current\":1}",
                           cw::MessagesJson(messages).c_str());
}

void TestUpdateSettingsKeepsMissingAndInvalid() {
  cw::Settings current;
  current.pitch_hz = 1200;
  const std::map<std::string, std::string> fields = {
      {"wpm", "0"}, {"pitch", "abc"}, {"loud", "0"}, {"coach", "1"}};
  const cw::Settings updated = cw::UpdateSettings(current, fields);
  TEST_ASSERT_EQUAL_INT(5, updated.wpm);               // Clamped.
  TEST_ASSERT_EQUAL_INT(1200, updated.pitch_hz);       // Invalid: kept.
  TEST_ASSERT_EQUAL_INT(2500, updated.letter_gap_ms);  // Missing: kept.
  TEST_ASSERT_FALSE(updated.loud);
  TEST_ASSERT_TRUE(updated.coach);
}

void TestUpdateSettingsAllFields() {
  const std::map<std::string, std::string> fields = {
      {"wpm", "20"}, {"gapMs", "4000"}, {"pitch", "99999"},
      {"loud", "1"}, {"repeats", "2"},  {"coach", "0"}};
  const cw::Settings updated = cw::UpdateSettings(cw::Settings(), fields);
  TEST_ASSERT_EQUAL_INT(20, updated.wpm);
  TEST_ASSERT_EQUAL_INT(4000, updated.letter_gap_ms);
  TEST_ASSERT_EQUAL_INT(4000, updated.pitch_hz);
  TEST_ASSERT_TRUE(updated.loud);
  TEST_ASSERT_EQUAL_INT(2, updated.repeats);
  TEST_ASSERT_FALSE(updated.coach);
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(TestJsonEscape);
  RUN_TEST(TestStatusJson);
  RUN_TEST(TestMessagesJson);
  RUN_TEST(TestUpdateSettingsKeepsMissingAndInvalid);
  RUN_TEST(TestUpdateSettingsAllFields);
  return UNITY_END();
}
