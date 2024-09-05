#ifndef __UTILS__
#define __UTILS__
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WebSocketsClient.h>

void connectToWiFi(const char *SSID, const char *password);

void setupWebSocket(WebSocketsClient &webSocket, String serverAddress);

void publishMessage(WebSocketsClient &webSocket, String topic, String target, String action, String params);

void subscribeToTopic(WebSocketsClient &webSocket, String topic, String clientName);

#endif