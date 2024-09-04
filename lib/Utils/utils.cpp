#include <utils.h>

void connectToWiFi(const char *SSID, const char *password)
{
  ESP8266WiFiMulti WiFiMulti;
  WiFiMulti.addAP(SSID, password);

  while (WiFiMulti.run() != WL_CONNECTED)
  {
    delay(100);
  }
}

void setupWebSocket(WebSocketsClient &webSocket, String serverAddress)
{
  // setup websocket client and event handler
  webSocket.begin(serverAddress, 20000, "/");
  webSocket.setReconnectInterval(5000);

  // ping server every 15000 ms and expect pong from server within 3000 ms
  // consider connection disconnected if pong is not received 2 times in a row
  webSocket.enableHeartbeat(15000, 3000, 2);
}
