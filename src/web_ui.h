// The Leader's control panel: a Wi-Fi access point, a captive-portal DNS
// server and an HTTP server with a small JSON API.

#ifndef SRC_WEB_UI_H_
#define SRC_WEB_UI_H_

#include <DNSServer.h>
#include <WebServer.h>

#include <string>

#include "messages.h"
#include "player.h"
#include "settings.h"

namespace cw {

// Serves the web page and API. Does not own the objects it is given.
class WebUi {
 public:
  WebUi(Settings* settings, MessageList* messages, Player* player);

  WebUi(const WebUi&) = delete;
  WebUi& operator=(const WebUi&) = delete;

  // Starts the access point, DNS catch-all and HTTP routes.
  void Begin();

  // Handles pending DNS and HTTP requests. Call from every loop().
  void Poll();

 private:
  void RegisterRoutes();
  void SendJson(const std::string& json);
  void SendStatus();
  void SendMessages(const std::string& extra_fields);

  // Hides the answer and saves the selection after a message change.
  void OnMessageChanged();

  Settings* const settings_;
  MessageList* const messages_;
  Player* const player_;
  WebServer server_{80};
  DNSServer dns_;
  bool revealed_ = false;
};

}  // namespace cw

#endif  // SRC_WEB_UI_H_
