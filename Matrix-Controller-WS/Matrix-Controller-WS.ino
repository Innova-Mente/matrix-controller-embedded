#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include "led_matrix.h"
#include <ArduinoJson.h>

#include <WebSocketsClient.h>

#include <Hash.h>

LedMatrix* pMat;

ESP8266WiFiMulti WiFiMulti;
WebSocketsClient webSocket;

#define USE_SERIAL Serial

bool connected = false;
bool subscribed = false;
JsonDocument lastMsg;
bool msgArrived = false;

String topic = "matrix-08";

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {

	switch(type) {
		case WStype_DISCONNECTED:{
			USE_SERIAL.printf("[WSc] Disconnected!\n");
			break;
    }
    case WStype_CONNECTED: {
			USE_SERIAL.printf("[WSc] Connected to url: %s\n", payload);
      connected = true;
			// send message to server when Connected
			// webSocket.sendTXT("Connected");
      String sub = String("{ \"cmd\": \"subscribe\", \"topic\": \"") + topic + "\"}";
      webSocket.sendTXT(sub);
 
			break;
    }
		case WStype_TEXT: {
			USE_SERIAL.printf("[WSc] get text: %s\n", payload);
      deserializeJson(lastMsg, payload);
      msgArrived = true;

			// send message to server
			// webSocket.sendTXT("message here");
			break;
    }
    case WStype_BIN: {
			USE_SERIAL.printf("[WSc] get binary length: %u\n", length);
			hexdump(payload, length);

			// send data to server
			// webSocket.sendBIN(payload, length);
			break;
    }
    
    case WStype_PING:{
            // pong will be send automatically
            USE_SERIAL.printf("[WSc] get ping\n");
            break;
    }
    case WStype_PONG:{
            // answer to a ping we send
            USE_SERIAL.printf("[WSc] get pong\n");
            break;
    }
  }

}

void setup() {
	// USE_SERIAL.begin(921600);
	USE_SERIAL.begin(115200);

  pMat = new LedMatrix();
  pMat->clear();

	//Serial.setDebugOutput(true);
	USE_SERIAL.setDebugOutput(true);

	USE_SERIAL.println();
	USE_SERIAL.println();
	USE_SERIAL.println();

	for(uint8_t t = 4; t > 0; t--) {
		USE_SERIAL.printf("[SETUP] BOOT WAIT %d...\n", t);
		USE_SERIAL.flush();
		delay(1000);
	}

	WiFiMulti.addAP("LittleBarfly", "303HotelLittleBarfly");

	//WiFi.disconnect();
	while(WiFiMulti.run() != WL_CONNECTED) {
		delay(100);
	}

	// server address, port and URL
	webSocket.begin("192.168.182.178", 20000, "/");

	// event handler
	webSocket.onEvent(webSocketEvent);

	// use HTTP Basic Authorization this is optional remove if not needed
	// webSocket.setAuthorization("user", "Password");

	// try ever 5000 again if connection has failed
	webSocket.setReconnectInterval(5000);
  
  // start heartbeat (optional)
  // ping server every 15000 ms
  // expect pong from server within 3000 ms
  // consider connection disconnected if pong is not received 2 times
  webSocket.enableHeartbeat(15000, 3000, 2);

}

void loop() {
	webSocket.loop();

  if (msgArrived){
    msgArrived = false;
    Serial.println("New message arrived.");
    String cmd = lastMsg["cmd"];
    if (cmd=="clear"){
        pMat->clear();
    } else if (cmd=="fillColor"){
        JsonObject color = lastMsg["color"];
        int r = color["r"];
        int g = color["g"];
        int b = color["b"];
        pMat->fillColor(RGB(r,g,b));
    }
  }
}
