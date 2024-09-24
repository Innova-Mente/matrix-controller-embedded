#include <Arduino.h>
#include <ArduinoJson.h>
#include <utils.h>

#include <led_matrix.h>
#include <led.h>
#include <sonar.h>
#include <ir.h>
#include <servo_motor.h>
#include <WebSocketsClient.h>

#define DEVICE_NUMBER String("9")
#define DEVICE_NAME String("dispositivo-" + DEVICE_NUMBER)
#define INPUT_TOPIC String(DEVICE_NAME + "-in")
#define OUTPUT_TOPIC String(DEVICE_NAME + "-out")

#define LED_MATRIX_PIN 4
#define GREEN_LED_PIN 13
#define RED_LED_PIN 14
#define SONAR_ECHO_PIN 21
#define SONAR_TRIG_PIN 19
#define IR_PIN 18
#define BUTTON_PIN 23
#define SERVO_MOTOR_PIN 22

LedMatrix *ledMatrix;
Led *greenLed;
Led *redLed;
Sonar *sonar;
IR *ir;
ServoMotor *servoMotor;

volatile bool pressDetected = false;
volatile bool presenceDetected = false;

bool isSonarDetecting = false;
int minSonarDetectDistance = 0;
int maxSonarDetectDistance = 10000;

WebSocketsClient webSocket;

bool newMessageArrived = false;
JsonDocument lastMessage;

void IRAM_ATTR buttonInterrupt()
{
  pressDetected = true;
}

void IRAM_ATTR irInterrupt()
{
  presenceDetected = true;
}

void webSocketEvent(WStype_t eventType, uint8_t *message, size_t messageLength)
{
  if (eventType == WStype_CONNECTED)
  {
    Serial.printf("Connected!\n");
    subscribeToTopic(webSocket, INPUT_TOPIC, DEVICE_NAME);
  }
  else if (eventType == WStype_TEXT)
  {
    deserializeJson(lastMessage, message);
    newMessageArrived = true;
    String topic = lastMessage["topic"].as<String>();
    Serial.printf("Received message from topic \"%s\": \n%s\n", topic.c_str(), message);
  }
}

void setup()
{
  Serial.begin(115200);
  Serial.setDebugOutput(true);

  connectToWiFi("Redmi Note 11", "tuamamma");

  setupWebSocket(webSocket, "192.168.210.242");
  webSocket.onEvent(webSocketEvent);

  ledMatrix = new LedMatrix(LED_MATRIX_PIN);
  greenLed = new Led(GREEN_LED_PIN);
  redLed = new Led(RED_LED_PIN);
  sonar = new Sonar(SONAR_TRIG_PIN, SONAR_ECHO_PIN);
  ir = new IR(IR_PIN);
  servoMotor = new ServoMotor(SERVO_MOTOR_PIN);
}

void loop()
{
  webSocket.loop();

  if (newMessageArrived)
  {
    newMessageArrived = false;

    String target = lastMessage["payload"]["target"];
    String action = lastMessage["payload"]["action"];

    if (target == "ledMatrix")
    {
      if (action == "clear")
      {
        ledMatrix->clear();
      }
      else if (action == "fillColor")
      {
        int r = lastMessage["payload"]["params"]["r"];
        int g = lastMessage["payload"]["params"]["g"];
        int b = lastMessage["payload"]["params"]["b"];

        ledMatrix->fillColor(RGB(r, g, b));
      }
      else if (action == "fade")
      {
        int r = lastMessage["payload"]["params"]["r"];
        int g = lastMessage["payload"]["params"]["g"];
        int b = lastMessage["payload"]["params"]["b"];
        String direction = lastMessage["payload"]["params"]["direction"];

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
        bool state = lastMessage["payload"]["params"]["state"];
        greenLed->setState(state);
      }
    }
    else if (target == "redLed")
    {
      if (action == "update")
      {
        bool state = lastMessage["payload"]["params"]["state"];
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
        minSonarDetectDistance = lastMessage["payload"]["params"]["min"];
        maxSonarDetectDistance = lastMessage["payload"]["params"]["max"];
      }
      else if (action == "measureDistance")
      {
        float distance = sonar->measure();
        publishMessage(webSocket, OUTPUT_TOPIC, "sonar", "distanceMeasured", String("{ \"distance\": ") + distance + "}");
      }
    }
    else if (target == "servo")
    {
      if (action == "rotate")
      {
        int angle = lastMessage["payload"]["params"]["angle"];
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
        String presence = ir->detectPresence() ? "true" : "false";
        publishMessage(webSocket, OUTPUT_TOPIC, "ir", "presenceMeasured", String("{ \"presence\": ") + presence + "}");
      }
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