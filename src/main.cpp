#include <Arduino.h>
#include <FirebaseJson.h>
#include <utils.h>

#include <led_matrix.h>
#include <led.h>
#include <sonar.h>
#include <ir.h>
#include <servo_motor.h>
#include <esp_websocket_client.h>
#include <WString.h>
#include <MB_String.h>

#define LED_MATRIX_PIN 4
#define GREEN_LED_PIN 13
#define RED_LED_PIN 14
#define SONAR_ECHO_PIN 21
#define SONAR_TRIG_PIN 19
#define IR_PIN 18
#define BUTTON_PIN 23
#define SERVO_MOTOR_PIN 22

void parseMessage(FirebaseJson message);

#define DEVICE_NAME "dispositivo-13"
#define INPUT_TOPIC "dispositivo-13-in"
#define OUTPUT_TOPIC "dispositivo-13-out"

LedMatrix *ledMatrix;
Led *greenLed;
Led *redLed;
Sonar *sonar;
IR *ir;
ServoMotor *servoMotor;

esp_websocket_client_handle_t webSocket;

volatile bool pressDetected = false;
volatile bool presenceDetected = false;

bool isSonarDetecting = false;
int minSonarDetectDistance = 0;
int maxSonarDetectDistance = 10000;

void IRAM_ATTR buttonInterrupt()
{
  pressDetected = true;
}

void IRAM_ATTR irInterrupt()
{
  presenceDetected = true;
}

void webSocketEvent(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
  esp_websocket_event_data_t *data = (esp_websocket_event_data_t *)event_data;

  if (event_id == WEBSOCKET_EVENT_CONNECTED)
  {
    Serial.printf("Connected!\n");
    subscribeToTopic(webSocket, INPUT_TOPIC, DEVICE_NAME);
  }
  else if (event_id == WEBSOCKET_EVENT_DATA)
  {
    if (data->op_code == 0x08 && data->data_len == 2)
    {
      Serial.printf("WS Received Closed Message");
    }
    else if (data->data_len > 8)
    {
      char *buffer;
      buffer = (char *)malloc(data->data_len * sizeof(char));
      if (buffer == NULL)
      {
        Serial.printf("Unable to Allocate Buffer");
        return;
      }
      sprintf(buffer, "%.*s", data->data_len, (char *)data->data_ptr);

      FirebaseJson lastMessage;
      lastMessage.setJsonData(buffer);

      FirebaseJsonData topicJson;
      lastMessage.get(topicJson, "topic");
      if (topicJson.success && topicJson.type == "string")
      {
        String topic = topicJson.to<String>().c_str();
        Serial.printf("Received message from topic \"%s\": \n%s\n", topic.c_str(), buffer);
        parseMessage(lastMessage);
      }
    }
  }
}

void setup()
{
  Serial.begin(115200);
  Serial.setDebugOutput(true);

  connectToWiFi("Redmi Note 11", "tuamamma");

  esp_websocket_client_config_t websocket_cfg = {};
  websocket_cfg.uri = "ws://192.168.210.242:20000";
  webSocket = esp_websocket_client_init(&websocket_cfg);
  esp_websocket_register_events(webSocket, WEBSOCKET_EVENT_ANY, webSocketEvent, (void *)webSocket);
  esp_websocket_client_start(webSocket);

  ledMatrix = new LedMatrix(LED_MATRIX_PIN);
  greenLed = new Led(GREEN_LED_PIN);
  redLed = new Led(RED_LED_PIN);
  sonar = new Sonar(SONAR_TRIG_PIN, SONAR_ECHO_PIN);
  ir = new IR(IR_PIN);
  servoMotor = new ServoMotor(SERVO_MOTOR_PIN);
}

void parseMessage(FirebaseJson lastMessage)
{

  FirebaseJsonData targetJson, actionJson;
  lastMessage.get(targetJson, "payload/target");
  lastMessage.get(actionJson, "payload/action");
  String target = targetJson.to<String>().c_str();
  String action = actionJson.to<String>().c_str();

  if (target == "ledMatrix")
  {
    if (action == "clear")
    {
      ledMatrix->clear();
    }
    else if (action == "fillColor")
    {
      FirebaseJsonData rJson, gJson, bJson;
      lastMessage.get(rJson, "payload/params/r");
      lastMessage.get(gJson, "payload/params/g");
      lastMessage.get(bJson, "payload/params/b");
      int r = rJson.to<int>();
      int g = gJson.to<int>();
      int b = bJson.to<int>();

      ledMatrix->fillColor(RGB(r, g, b));
    }
    else if (action == "fade")
    {
      FirebaseJsonData rJson, gJson, bJson, directionJson;
      lastMessage.get(rJson, "payload/params/r");
      lastMessage.get(gJson, "payload/params/g");
      lastMessage.get(bJson, "payload/params/b");
      lastMessage.get(directionJson, "payload/params/direction");
      int r = rJson.to<int>();
      int g = gJson.to<int>();
      int b = bJson.to<int>();
      String direction = directionJson.to<String>().c_str();

      if (direction == "left")
      {
        ledMatrix->fadeLeft(RGB(r, g, b));
      }
      else if (direction == "right")
      {
        ledMatrix->fadeRight(RGB(r, g, b));
      }
    }
  }
  else if (target == "greenLed")
  {
    if (action == "update")
    {
      FirebaseJsonData stateJson;
      lastMessage.get(stateJson, "payload/params/state");
      bool state = stateJson.to<bool>();
      greenLed->setState(state);
    }
  }
  else if (target == "redLed")
  {
    if (action == "update")
    {
      FirebaseJsonData stateJson;
      lastMessage.get(stateJson, "payload/params/state");
      bool state = stateJson.to<bool>();
      redLed->setState(state);
    }
  }
  else if (target == "button")
  {
    if (action == "detectPress")
    {
      pinMode(BUTTON_PIN, INPUT_PULLUP);
      attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), buttonInterrupt, RISING);
    }
  }
  else if (target == "sonar")
  {
    if (action == "detectDistance")
    {
      isSonarDetecting = true;

      FirebaseJsonData minJson, maxJson;
      lastMessage.get(minJson, "payload/params/min");
      lastMessage.get(maxJson, "payload/params/max");
      minSonarDetectDistance = minJson.to<int>();
      maxSonarDetectDistance = maxJson.to<int>();
    }
    else if (action == "measureDistance")
    {
      float distance = sonar->measure();
      FirebaseJson json;
      json.add("distance", distance);
      String message;
      json.toString(message, true);
      publishMessage(webSocket, OUTPUT_TOPIC, "sonar", "distanceMeasured", message);
    }
  }
  else if (target == "servo")
  {
    if (action == "rotate")
    {
      FirebaseJsonData angleJson;
      lastMessage.get(angleJson, "payload/params/angle");
      int angle = angleJson.to<int>();
      servoMotor->incrementAngle(angle);
    }
  }
  else if (target == "ir")
  {
    if (action == "detectPresence")
    {
      attachInterrupt(digitalPinToInterrupt(IR_PIN), irInterrupt, RISING);
    }
    else if (action == "measurePresence")
    {
      bool presence = ir->detectPresence();
      FirebaseJson json;
      json.add("presence", presence);
      String message;
      json.toString(message, true);
      publishMessage(webSocket, OUTPUT_TOPIC, "ir", "presenceMeasured", message);
    }
  }

  if (pressDetected)
  {
    detachInterrupt(digitalPinToInterrupt(BUTTON_PIN));
    pressDetected = false;
    publishMessage(webSocket, OUTPUT_TOPIC, "button", "pressDetected", "{}");
  }
  if (presenceDetected)
  {
    detachInterrupt(digitalPinToInterrupt(IR_PIN));
    presenceDetected = false;
    publishMessage(webSocket, OUTPUT_TOPIC, "ir", "presenceDetected", "{}");
  }
  if (isSonarDetecting)
  {
    float distance = sonar->measure();
    if (distance >= minSonarDetectDistance && distance <= maxSonarDetectDistance)
    {
      isSonarDetecting = false;
      publishMessage(webSocket, OUTPUT_TOPIC, "sonar", "distanceDetected", "{}");
    }
  }
}

void loop()
{
}