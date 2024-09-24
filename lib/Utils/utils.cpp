#include <utils.h>

void connectToWiFi(const char *SSID, const char *password)
{
  WiFiMulti wifiMulti;
  wifiMulti.addAP(SSID, password);

  while (wifiMulti.run() != WL_CONNECTED)
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

void publishMessage(WebSocketsClient &webSocket, String topic, String target, String action, String params)
{
  String message = "{ \"type\": \"publish\", \"topic\": \"" + topic + "\", \"payload\": { \"target\": \"" + target + "\", \"action\": \"" + action + "\", \"params\": " + params + "}}";
  webSocket.sendTXT(message);

  Serial.printf("Published message on topic \"%s\": \n%s\n", topic.c_str(), message.c_str());
}

void subscribeToTopic(WebSocketsClient &webSocket, String topic, String clientName)
{
  String message = "{ \"type\": \"subscribe\", \"topic\": \"" + topic + "\", \"payload\": { \"clientName\": \"" + clientName + "\"}}";
  webSocket.sendTXT(message);

  Serial.printf("Subscribed to topic \"%s\"\n", topic.c_str());
}