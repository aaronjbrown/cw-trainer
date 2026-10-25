// On-device test: Morse timing must not stretch while the main loop is busy
// (for example while the web server is handling a request).

#include <Arduino.h>
#include <unity.h>

#include "piezo.h"
#include "player.h"
#include "settings.h"

namespace {

cw::Piezo* piezo = nullptr;
cw::Player* player = nullptr;

}  // namespace

void setUp() {}
void tearDown() {}

// "E E" at 15 WPM with a 500 ms letter gap is 80 + 1000 + 80 = 1160 ms. The
// caller busy-waits without yielding, as a slow web request would.
void TestTimingUnaffectedByBusyLoop() {
  cw::Settings settings;
  settings.letter_gap_ms = 500;
  const uint32_t start_ms = millis();
  player->Start("E E", settings);
  while (player->is_playing() && millis() - start_ms < 3000) {
  }
  const uint32_t elapsed_ms = millis() - start_ms;
  TEST_ASSERT_UINT32_WITHIN(15, 1160, elapsed_ms);
}

void setup() {
  delay(2000);  // Give the serial monitor time to connect.
  piezo = new cw::Piezo(25, 26, 2);
  piezo->Begin();
  player = new cw::Player(piezo);
  player->Begin();
  UNITY_BEGIN();
  RUN_TEST(TestTimingUnaffectedByBusyLoop);
  UNITY_END();
}

void loop() {}
