// Host tests for message normalisation and the message list.

#include <unity.h>

#include <string>

#include "messages.h"

namespace {

void AssertDefaults(const cw::MessageList& list) {
  TEST_ASSERT_EQUAL_INT(10, list.size());
  TEST_ASSERT_EQUAL_STRING("HI", list.at(0).c_str());
  TEST_ASSERT_EQUAL_STRING("CAT", list.at(9).c_str());
}

}  // namespace

void setUp() {}
void tearDown() {}

void TestDefaults() {
  const cw::MessageList list;
  AssertDefaults(list);
  TEST_ASSERT_EQUAL_INT(0, list.current());
  TEST_ASSERT_EQUAL_STRING("HI", list.CurrentText().c_str());
}

void TestNormalize() {
  std::string note;
  TEST_ASSERT_EQUAL_STRING(
      "HI THERE", cw::NormalizeMessage("  hi   there! ", note).c_str());
  TEST_ASSERT_FALSE(note.empty());

  TEST_ASSERT_EQUAL_STRING("RADIO",
                           cw::NormalizeMessage("radio", note).c_str());
  TEST_ASSERT_TRUE(note.empty());

  TEST_ASSERT_EQUAL_STRING("", cw::NormalizeMessage("!!!", note).c_str());

  const std::string long_text = "ABCDEFGHIJKLMNOPQRSTUVWXY";  // 25 letters.
  const std::string shortened = cw::NormalizeMessage(long_text, note);
  TEST_ASSERT_EQUAL_INT(20, shortened.size());
  TEST_ASSERT_NOT_EQUAL(std::string::npos, note.find("20"));

  // The cut falls just after a space, which must not be left dangling.
  const std::string cut_at_space =
      cw::NormalizeMessage("ABCDEFGHIJKLMNOPQRS TUV", note);
  TEST_ASSERT_EQUAL_STRING("ABCDEFGHIJKLMNOPQRS", cut_at_space.c_str());
}

void TestAddRejectsEmptyAndFull() {
  cw::MessageList list;
  std::string note;
  TEST_ASSERT_FALSE(list.Add("!!", note));
  TEST_ASSERT_FALSE(note.empty());
  TEST_ASSERT_EQUAL_INT(10, list.size());

  TEST_ASSERT_TRUE(list.Add("hello scouts", note));
  TEST_ASSERT_EQUAL_STRING("HELLO SCOUTS", list.at(10).c_str());
  while (list.size() < cw::kMaxMessages) {
    TEST_ASSERT_TRUE(list.Add("A", note));
  }
  TEST_ASSERT_FALSE(list.Add("B", note));
  TEST_ASSERT_FALSE(note.empty());
  TEST_ASSERT_EQUAL_INT(cw::kMaxMessages, list.size());
}

void TestRemoveCurrentLastItem() {
  cw::MessageList list;
  list.SetCurrent(9);
  list.Remove(9, "CAT");
  TEST_ASSERT_EQUAL_INT(9, list.size());
  TEST_ASSERT_EQUAL_INT(8, list.current());
}

void TestRemoveBeforeCurrentKeepsSameMessage() {
  cw::MessageList list;
  list.SetCurrent(2);  // RADIO
  list.Remove(0, "HI");
  TEST_ASSERT_EQUAL_STRING("RADIO", list.CurrentText().c_str());
}

void TestRemoveAllRestoresDefaults() {
  cw::MessageList list;
  while (list.size() > 1) list.Remove(0, list.at(0));
  TEST_ASSERT_EQUAL_STRING("CAT", list.CurrentText().c_str());
  list.Remove(0, "CAT");
  AssertDefaults(list);
  TEST_ASSERT_EQUAL_INT(0, list.current());
}

void TestRemoveOutOfRangeIgnored() {
  cw::MessageList list;
  TEST_ASSERT_TRUE(list.Remove(-1, "HI") == cw::RemoveResult::kNotFound);
  TEST_ASSERT_TRUE(list.Remove(10, "HI") == cw::RemoveResult::kNotFound);
  AssertDefaults(list);
}

void TestRemoveStaleTextIgnored() {
  cw::MessageList list;
  // The page still shows HI at index 0, but the list has changed.
  TEST_ASSERT_TRUE(list.Remove(0, "SOS") == cw::RemoveResult::kNotFound);
  AssertDefaults(list);
}

void TestRemoveReportsWhetherCurrent() {
  cw::MessageList list;
  list.SetCurrent(2);  // RADIO
  TEST_ASSERT_TRUE(list.Remove(0, "HI") == cw::RemoveResult::kRemoved);
  TEST_ASSERT_TRUE(list.Remove(1, "RADIO") ==
                   cw::RemoveResult::kRemovedCurrent);
}

void TestRemoveCurrentWithSameTextNextIsCurrent() {
  cw::MessageList list;
  list.Deserialize("SOS\nSOS\nHI");
  TEST_ASSERT_TRUE(list.Remove(0, "SOS") == cw::RemoveResult::kRemovedCurrent);
  TEST_ASSERT_EQUAL_STRING("SOS", list.CurrentText().c_str());
}

void TestNextWraps() {
  cw::MessageList list;
  list.SetCurrent(9);
  list.Next();
  TEST_ASSERT_EQUAL_INT(0, list.current());
  list.Next();
  TEST_ASSERT_EQUAL_INT(1, list.current());
}

void TestSelectPicksMatchingMessage() {
  cw::MessageList list;
  TEST_ASSERT_TRUE(list.Select(9, "CAT"));
  TEST_ASSERT_EQUAL_INT(9, list.current());
}

void TestSelectStaleOrOutOfRangeIgnored() {
  cw::MessageList list;
  list.SetCurrent(2);
  TEST_ASSERT_FALSE(list.Select(9, "DOG"));
  TEST_ASSERT_FALSE(list.Select(10, "CAT"));
  TEST_ASSERT_FALSE(list.Select(-1, "HI"));
  TEST_ASSERT_EQUAL_INT(2, list.current());
}

void TestSetCurrentClamps() {
  cw::MessageList list;
  list.SetCurrent(99);
  TEST_ASSERT_EQUAL_INT(9, list.current());
  list.SetCurrent(-4);
  TEST_ASSERT_EQUAL_INT(0, list.current());
}

void TestRandomNeverSame() {
  for (uint32_t r = 0; r < 100; ++r) {
    cw::MessageList list;
    list.SetCurrent(3);
    list.PickRandom(r);
    TEST_ASSERT_NOT_EQUAL(3, list.current());
    TEST_ASSERT_TRUE(list.current() >= 0 && list.current() < 10);
  }
  cw::MessageList single;
  single.Deserialize("SOS");
  single.PickRandom(7);
  TEST_ASSERT_EQUAL_INT(0, single.current());
}

void TestSerializeRoundTrip() {
  cw::MessageList list;
  std::string note;
  list.Add("hello scouts", note);
  cw::MessageList copy;
  copy.Deserialize(list.Serialize());
  TEST_ASSERT_EQUAL_INT(11, copy.size());
  TEST_ASSERT_EQUAL_STRING("HELLO SCOUTS", copy.at(10).c_str());
  TEST_ASSERT_EQUAL_STRING("HI", copy.at(0).c_str());
}

void TestDeserializeGarbage() {
  cw::MessageList list;
  list.Deserialize("\n\n!!\n");
  AssertDefaults(list);
  list.Deserialize("dog\n\ncat");
  TEST_ASSERT_EQUAL_INT(2, list.size());
  TEST_ASSERT_EQUAL_STRING("DOG", list.at(0).c_str());
  TEST_ASSERT_EQUAL_STRING("CAT", list.at(1).c_str());
  TEST_ASSERT_EQUAL_INT(0, list.current());
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(TestDefaults);
  RUN_TEST(TestNormalize);
  RUN_TEST(TestAddRejectsEmptyAndFull);
  RUN_TEST(TestRemoveCurrentLastItem);
  RUN_TEST(TestRemoveBeforeCurrentKeepsSameMessage);
  RUN_TEST(TestRemoveAllRestoresDefaults);
  RUN_TEST(TestRemoveOutOfRangeIgnored);
  RUN_TEST(TestRemoveStaleTextIgnored);
  RUN_TEST(TestRemoveReportsWhetherCurrent);
  RUN_TEST(TestRemoveCurrentWithSameTextNextIsCurrent);
  RUN_TEST(TestNextWraps);
  RUN_TEST(TestSelectPicksMatchingMessage);
  RUN_TEST(TestSelectStaleOrOutOfRangeIgnored);
  RUN_TEST(TestSetCurrentClamps);
  RUN_TEST(TestRandomNeverSame);
  RUN_TEST(TestSerializeRoundTrip);
  RUN_TEST(TestDeserializeGarbage);
  return UNITY_END();
}
