#ifndef __UTILS__
#define __UTILS__
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <esp_websocket_client.h>

void connectToWiFi(const char *SSID, const char *password);

void publishMessage(esp_websocket_client_handle_t &webSocket, String topic, String target, String action, String params);

void subscribeToTopic(esp_websocket_client_handle_t &webSocket, String topic, String clientName);

#endif