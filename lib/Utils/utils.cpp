#include <utils.h>

void connectToWiFi(const char *SSID, const char *password)
{
  WiFiMulti WiFimulti;
  WiFimulti.addAP(SSID, password);

  while (WiFimulti.run() != WL_CONNECTED)
  {
    delay(100);
  }
}

void publishMessage(esp_websocket_client_handle_t &webSocket, String topic, String target, String action, String params)
{
  String message = "{ \"type\": \"publish\", \"topic\": \"" + topic + "\", \"payload\": { \"target\": \"" + target + "\", \"action\": \"" + action + "\", \"params\": " + params + "}}";
  esp_websocket_client_send_text(webSocket, message.c_str(), strlen(message.c_str()), 10000);

  Serial.printf("Published message on topic \"%s\": \n%s\n", topic.c_str(), message.c_str());
}

void subscribeToTopic(esp_websocket_client_handle_t &webSocket, String topic, String clientName)
{
  String message = "{ \"type\": \"subscribe\", \"topic\": \"" + topic + "\", \"payload\": { \"clientName\": \"" + clientName + "\"}}";
  esp_websocket_client_send_text(webSocket, message.c_str(), strlen(message.c_str()), 10000);

  Serial.printf("Subscribed to topic \"%s\"\n", topic.c_str());
}
