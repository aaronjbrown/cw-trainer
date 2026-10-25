#include "web_ui.h"

#include <WiFi.h>
#include <esp_random.h>

#include <map>
#include <optional>
#include <string>

#include "station_json.h"
#include "storage.h"
#include "web_page.h"

namespace cw {
namespace {

constexpr char kSsid[] = "CW-Trainer";
constexpr char kPassword[] = "cwtrainer";
constexpr uint16_t kDnsPort = 53;
constexpr char kPortalUrl[] = "http://192.168.4.1/";

}  // namespace

WebUi::WebUi(Settings* settings, MessageList* messages, Player* player)
    : settings_(settings), messages_(messages), player_(player) {}

void WebUi::Begin() {
  WiFi.mode(WIFI_AP);
  if (WiFi.softAP(kSsid, kPassword)) {
    Serial.printf("Wi-Fi %s started at %s\n", kSsid,
                  WiFi.softAPIP().toString().c_str());
  } else {
    Serial.println("Wi-Fi failed to start");
  }
  if (!dns_.start(kDnsPort, "*", WiFi.softAPIP())) {
    Serial.println("Captive portal DNS failed to start");
  }
  RegisterRoutes();
  server_.begin();
}

void WebUi::Poll() {
  dns_.processNextRequest();
  server_.handleClient();
}

void WebUi::RegisterRoutes() {
  server_.on("/", HTTP_GET,
             [this]() { server_.send(200, "text/html", kIndexHtml); });
  server_.on("/api/status", HTTP_GET, [this]() { SendStatus(); });
  server_.on("/api/play", HTTP_POST, [this]() {
    player_->Start(messages_->CurrentText(), *settings_);
    SendStatus();
  });
  server_.on("/api/stop", HTTP_POST, [this]() {
    player_->Stop();
    SendStatus();
  });
  server_.on("/api/reveal", HTTP_POST, [this]() {
    revealed_ = !revealed_;
    SendStatus();
  });
  server_.on("/api/next", HTTP_POST, [this]() {
    player_->Stop();
    messages_->Next();
    OnMessageChanged();
    SendStatus();
  });
  server_.on("/api/random", HTTP_POST, [this]() {
    player_->Stop();
    messages_->PickRandom(esp_random());
    OnMessageChanged();
    SendStatus();
  });
  server_.on("/api/select", HTTP_POST, [this]() {
    const std::optional<int> index =
        ParseIntField(server_.arg("index").c_str());
    if (index && *index != messages_->current() &&
        messages_->Select(*index, server_.arg("text").c_str())) {
      player_->Stop();
      OnMessageChanged();
    }
    SendStatus();
  });
  server_.on("/api/settings", HTTP_POST, [this]() {
    std::map<std::string, std::string> fields;
    for (int i = 0; i < server_.args(); ++i) {
      fields[server_.argName(i).c_str()] = server_.arg(i).c_str();
    }
    *settings_ = UpdateSettings(*settings_, fields);
    SaveSettings(*settings_);
    SendStatus();
  });
  server_.on("/api/messages", HTTP_GET, [this]() { SendMessages(""); });
  server_.on("/api/messages/add", HTTP_POST, [this]() {
    std::string note;
    const bool ok = messages_->Add(server_.arg("text").c_str(), note);
    if (ok) SaveMessages(*messages_);
    SendMessages("\"ok\":" + std::string(ok ? "true" : "false") +
                 ",\"note\":\"" + JsonEscape(note) + "\",");
  });
  server_.on("/api/messages/delete", HTTP_POST, [this]() {
    const std::optional<int> index =
        ParseIntField(server_.arg("index").c_str());
    const std::string text = server_.arg("text").c_str();
    const bool is_current = index && *index == messages_->current() &&
                            text == messages_->CurrentText();
    if (is_current) player_->Stop();
    const RemoveResult result = messages_->Remove(index.value_or(-1), text);
    if (result == RemoveResult::kNotFound) {
      SendMessages(
          "\"ok\":false,\"note\":\"The list had changed. Check it and try "
          "again.\",");
      return;
    }
    if (result == RemoveResult::kRemovedCurrent) revealed_ = false;
    SaveMessages(*messages_);
    SendMessages("\"ok\":true,");
  });
  server_.onNotFound([this]() {
    server_.sendHeader("Location", kPortalUrl, /*first=*/true);
    server_.send(302, "text/plain", "");
  });
}

void WebUi::SendJson(const std::string& json) {
  server_.sendHeader("Cache-Control", "no-store");
  server_.send(200, "application/json", json.c_str());
}

void WebUi::SendStatus() {
  SendJson(StatusJson(*settings_, *messages_, player_->is_playing(), revealed_,
                      player_->letter_index()));
}

void WebUi::SendMessages(const std::string& extra_fields) {
  // Splice optional fields such as "ok" and "note" into the list object.
  SendJson("{" + extra_fields + MessagesJson(*messages_).substr(1));
}

void WebUi::OnMessageChanged() {
  revealed_ = false;
  SaveMessages(*messages_);
}

}  // namespace cw
