// Host tests for settings clamping and form-field parsing.

#include <unity.h>

#include <optional>

#include "settings.h"

void setUp() {}
void tearDown() {}

void TestDefaultsUnchangedByClamp() {
  const cw::Settings clamped = cw::ClampSettings(cw::Settings());
  TEST_ASSERT_EQUAL_INT(15, clamped.wpm);
  TEST_ASSERT_EQUAL_INT(2500, clamped.letter_gap_ms);
  TEST_ASSERT_EQUAL_INT(800, clamped.pitch_hz);
  TEST_ASSERT_TRUE(clamped.loud);
  TEST_ASSERT_EQUAL_INT(1, clamped.repeats);
  TEST_ASSERT_FALSE(clamped.coach);
}

void TestClampsLow() {
  cw::Settings settings;
  settings.wpm = 0;
  settings.letter_gap_ms = 0;
  settings.pitch_hz = 10;
  settings.repeats = 0;
  cw::Settings clamped = cw::ClampSettings(settings);
  TEST_ASSERT_EQUAL_INT(5, clamped.wpm);
  TEST_ASSERT_EQUAL_INT(500, clamped.letter_gap_ms);
  TEST_ASSERT_EQUAL_INT(400, clamped.pitch_hz);
  TEST_ASSERT_EQUAL_INT(1, clamped.repeats);
  settings.wpm = -5;
  TEST_ASSERT_EQUAL_INT(5, cw::ClampSettings(settings).wpm);
}

void TestClampsHigh() {
  cw::Settings settings;
  settings.wpm = 99;
  settings.letter_gap_ms = 60000;
  settings.pitch_hz = 99999;
  settings.repeats = 9;
  const cw::Settings clamped = cw::ClampSettings(settings);
  TEST_ASSERT_EQUAL_INT(25, clamped.wpm);
  TEST_ASSERT_EQUAL_INT(6000, clamped.letter_gap_ms);
  TEST_ASSERT_EQUAL_INT(4000, clamped.pitch_hz);
  TEST_ASSERT_EQUAL_INT(3, clamped.repeats);
}

void TestParseIntField() {
  TEST_ASSERT_EQUAL_INT(15, cw::ParseIntField("15").value_or(-999));
  TEST_ASSERT_EQUAL_INT(-3, cw::ParseIntField("-3").value_or(-999));
  TEST_ASSERT_FALSE(cw::ParseIntField("abc").has_value());
  TEST_ASSERT_FALSE(cw::ParseIntField("").has_value());
  TEST_ASSERT_FALSE(cw::ParseIntField("-").has_value());
  TEST_ASSERT_FALSE(cw::ParseIntField("12x").has_value());
  TEST_ASSERT_FALSE(cw::ParseIntField("99999999999").has_value());
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(TestDefaultsUnchangedByClamp);
  RUN_TEST(TestClampsLow);
  RUN_TEST(TestClampsHigh);
  RUN_TEST(TestParseIntField);
  return UNITY_END();
}
