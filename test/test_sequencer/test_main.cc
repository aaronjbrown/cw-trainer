// Host tests for the Morse table and timing sequencer.

#include <unity.h>

#include <optional>
#include <string>
#include <vector>

#include "morse.h"

namespace {

// Drains the sequencer into a vector of steps.
std::vector<cw::Step> Drain(cw::MorseSequencer& sequencer) {
  std::vector<cw::Step> steps;
  while (std::optional<cw::Step> step = sequencer.Next()) {
    steps.push_back(*step);
    if (steps.size() > 1000) break;  // Guard against runaway sequences.
  }
  return steps;
}

void AssertStep(const cw::Step& step, bool tone_on, int duration_ms,
                int letter_index) {
  TEST_ASSERT_EQUAL(tone_on, step.tone_on);
  TEST_ASSERT_EQUAL_INT(duration_ms, step.duration_ms);
  TEST_ASSERT_EQUAL_INT(letter_index, step.letter_index);
}

const cw::Timing kDefaultTiming = cw::MakeTiming(15, 2500);

}  // namespace

void setUp() {}
void tearDown() {}

void TestPatternTable() {
  TEST_ASSERT_EQUAL_STRING(".-", cw::MorsePattern('A'));
  TEST_ASSERT_EQUAL_STRING("...", cw::MorsePattern('s'));
  TEST_ASSERT_EQUAL_STRING("-----", cw::MorsePattern('0'));
  TEST_ASSERT_EQUAL_STRING("----.", cw::MorsePattern('9'));
  TEST_ASSERT_EQUAL_STRING("--.-", cw::MorsePattern('Q'));
  TEST_ASSERT_NULL(cw::MorsePattern('!'));
  TEST_ASSERT_NULL(cw::MorsePattern(' '));
}

void TestTimingDefaults() {
  const cw::Timing timing = cw::MakeTiming(15, 2500);
  TEST_ASSERT_EQUAL_INT(80, timing.dit_ms);
  TEST_ASSERT_EQUAL_INT(2500, timing.letter_gap_ms);
  TEST_ASSERT_EQUAL_INT(5000, timing.word_gap_ms);
  TEST_ASSERT_EQUAL_INT(5000, timing.repeat_gap_ms);
  TEST_ASSERT_EQUAL_INT(3000, cw::MakeTiming(25, 500).repeat_gap_ms);
}

void TestSequenceE() {
  cw::MorseSequencer sequencer;
  sequencer.Start("E", kDefaultTiming, 1);
  TEST_ASSERT_TRUE(sequencer.is_playing());
  const std::vector<cw::Step> steps = Drain(sequencer);
  TEST_ASSERT_EQUAL_size_t(1, steps.size());
  AssertStep(steps[0], true, 80, 0);
  TEST_ASSERT_FALSE(sequencer.is_playing());
}

void TestSequenceIT() {
  cw::MorseSequencer sequencer;
  sequencer.Start("IT", kDefaultTiming, 1);
  const std::vector<cw::Step> steps = Drain(sequencer);
  TEST_ASSERT_EQUAL_size_t(5, steps.size());
  AssertStep(steps[0], true, 80, 0);
  AssertStep(steps[1], false, 80, 0);
  AssertStep(steps[2], true, 80, 0);
  AssertStep(steps[3], false, 2500, 0);
  AssertStep(steps[4], true, 240, 1);
}

void TestWordGap() {
  cw::MorseSequencer sequencer;
  sequencer.Start("E E", kDefaultTiming, 1);
  const std::vector<cw::Step> steps = Drain(sequencer);
  TEST_ASSERT_EQUAL_size_t(3, steps.size());
  AssertStep(steps[0], true, 80, 0);
  AssertStep(steps[1], false, 5000, 0);
  AssertStep(steps[2], true, 80, 2);
}

void TestSkipsUnsupported() {
  cw::MorseSequencer sequencer;
  sequencer.Start("E!E", kDefaultTiming, 1);
  const std::vector<cw::Step> steps = Drain(sequencer);
  TEST_ASSERT_EQUAL_size_t(3, steps.size());
  AssertStep(steps[0], true, 80, 0);
  AssertStep(steps[1], false, 2500, 0);
  AssertStep(steps[2], true, 80, 2);
}

void TestRepeats() {
  cw::MorseSequencer sequencer;
  sequencer.Start("E", kDefaultTiming, 2);
  const std::vector<cw::Step> steps = Drain(sequencer);
  TEST_ASSERT_EQUAL_size_t(3, steps.size());
  AssertStep(steps[0], true, 80, 0);
  AssertStep(steps[1], false, 5000, 0);
  AssertStep(steps[2], true, 80, 0);
}

void TestStopMidway() {
  cw::MorseSequencer sequencer;
  sequencer.Start("SOS", kDefaultTiming, 1);
  TEST_ASSERT_TRUE(sequencer.Next().has_value());
  TEST_ASSERT_TRUE(sequencer.Next().has_value());
  sequencer.Stop();
  TEST_ASSERT_FALSE(sequencer.is_playing());
  TEST_ASSERT_FALSE(sequencer.Next().has_value());
  sequencer.Start("E", kDefaultTiming, 1);
  const std::vector<cw::Step> steps = Drain(sequencer);
  TEST_ASSERT_EQUAL_size_t(1, steps.size());
  AssertStep(steps[0], true, 80, 0);
}

void TestEmptyText() {
  cw::MorseSequencer sequencer;
  sequencer.Start("!!", kDefaultTiming, 1);
  TEST_ASSERT_FALSE(sequencer.Next().has_value());
  TEST_ASSERT_FALSE(sequencer.is_playing());
}

void TestLetterIndexBeforeStart() {
  cw::MorseSequencer sequencer;
  TEST_ASSERT_EQUAL_INT(-1, sequencer.letter_index());
  sequencer.Start("HI", kDefaultTiming, 1);
  TEST_ASSERT_EQUAL_INT(-1, sequencer.letter_index());
  sequencer.Next();
  TEST_ASSERT_EQUAL_INT(0, sequencer.letter_index());
}

void TestLetterIndexHoldsThroughGap() {
  cw::MorseSequencer sequencer;
  sequencer.Start("IT", kDefaultTiming, 1);
  for (int i = 0; i < 4; ++i) sequencer.Next();  // I, then the letter gap.
  TEST_ASSERT_EQUAL_INT(0, sequencer.letter_index());
  sequencer.Next();  // T sounds.
  TEST_ASSERT_EQUAL_INT(1, sequencer.letter_index());
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(TestPatternTable);
  RUN_TEST(TestTimingDefaults);
  RUN_TEST(TestSequenceE);
  RUN_TEST(TestSequenceIT);
  RUN_TEST(TestWordGap);
  RUN_TEST(TestSkipsUnsupported);
  RUN_TEST(TestRepeats);
  RUN_TEST(TestStopMidway);
  RUN_TEST(TestEmptyText);
  RUN_TEST(TestLetterIndexBeforeStart);
  RUN_TEST(TestLetterIndexHoldsThroughGap);
  return UNITY_END();
}
