// CW Trainer firmware entry point. Owns the station objects and runs the
// main loop.

#include <Arduino.h>

#include "button.h"
#include "messages.h"
#include "piezo.h"
#include "player.h"
#include "settings.h"
#include "storage.h"
#include "web_ui.h"

#ifndef PIO_UNIT_TESTING

namespace {

constexpr int kPiezoPinA = 25;
constexpr int kPiezoPinB = 26;
constexpr int kLedPin = 2;
constexpr int kPlayButtonPin = 0;  // The board's BOOT button.

// Created once in setup() and intentionally never destroyed.
cw::Piezo* piezo = nullptr;
cw::Player* player = nullptr;
cw::Button* play_button = nullptr;
cw::Settings* settings = nullptr;
cw::MessageList* messages = nullptr;
cw::WebUi* web_ui = nullptr;

}  // namespace

void setup() {
  Serial.begin(115200);
  piezo = new cw::Piezo(kPiezoPinA, kPiezoPinB, kLedPin);
  piezo->Begin();
  player = new cw::Player(piezo);
  player->Begin();
  play_button = new cw::Button(kPlayButtonPin);
  play_button->Begin();
  settings = new cw::Settings();
  messages = new cw::MessageList();
  cw::LoadState(*settings, *messages);
  web_ui = new cw::WebUi(settings, messages, player);
  web_ui->Begin();
  Serial.println("CW Trainer boot");
}

void loop() {
  const uint32_t now_ms = millis();
  if (play_button->WasPressed(now_ms)) {
    player->Start(messages->CurrentText(), *settings);
  }
  web_ui->Poll();
}

#endif  // PIO_UNIT_TESTING
