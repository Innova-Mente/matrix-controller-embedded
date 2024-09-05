#include <Arduino.h>
#include <ArduinoJson.h>
#include <utils.h>

#include <led_matrix.h>
#include <led.h>
#include <sonar.h>
#include <ir.h>
#include <servo_motor.h>
#include <photo_resistor.h>

#define USE_SERIAL Serial
#define DEVICE_NUMBER String("2")

#define LED_MATRIX_PIN D1
#define GREEN_LED_PIN D0
#define RED_LED_PIN D4
#define SONAR_ECHO_PIN D5
#define SONAR_TRIG_PIN D6
#define IR_PIN D7
#define BUTTON_PIN D2
#define SERVO_MOTOR_PIN D8
#define PHOTO_RESISTOR_PIN A0

LedMatrix *ledMatrix;
Led *greenLed;
Led *redLed;
Sonar *sonar;
IR *ir;
ServoMotor *servoMotor;
PhotoResistor *photoResistor;

volatile bool pressDetected = false;
volatile bool presenceDetected = false;

bool isSonarDetecting = false;
int minSonarDetectDistance = 0;
int maxSonarDetectDistance = 10000;

bool isPhotoResistorDetecting = false;
int minPhotoResistorDetectBrightness = 0;
int maxPhotoResistorDetectBrightness = 100;

WebSocketsClient webSocket;
JsonDocument lastMessage;
bool newMessageArrived = false;

String createPublishMessage(String target, String action, String params)
{
  return "{ \"type\": \"publish\", \"topic\": \"dispositivo-" + DEVICE_NUMBER + "-out\", \"payload\": { \"target\": \"" + target + "\", \"action\": \"" + action + "\", \"params\": " + params + "}}";
}

String createSubscribeMessage(String topic)
{
  return "{ \"type\": \"subscribe\", \"topic\": \"" + topic + "\"}";
}

void webSocketEvent(WStype_t eventType, uint8_t *message, size_t messageLength)
{
  if (eventType == WStype_CONNECTED)
  {
    USE_SERIAL.printf("Connected!\n");

    String message = createSubscribeMessage("dispositivo-" + DEVICE_NUMBER + "-in");
    webSocket.sendTXT(message);
  }
  else if (eventType == WStype_TEXT)
  {
    USE_SERIAL.printf("Received message: \n%s\n", message);

    deserializeJson(lastMessage, message);
    newMessageArrived = true;
  }
}

void IRAM_ATTR buttonInterrupt()
{
  pressDetected = true;
}

void IRAM_ATTR irInterrupt()
{
  presenceDetected = true;
}

void setup()
{
  USE_SERIAL.begin(115200);
  USE_SERIAL.setDebugOutput(true);

  connectToWiFi("TIM-83625402", "hK2XG5uf5RbxCAHy2zXZuGxD");

  setupWebSocket(webSocket, "192.168.1.10");
  webSocket.onEvent(webSocketEvent);

  ledMatrix = new LedMatrix(LED_MATRIX_PIN);
  greenLed = new Led(GREEN_LED_PIN);
  redLed = new Led(RED_LED_PIN);
  sonar = new Sonar(SONAR_TRIG_PIN, SONAR_ECHO_PIN);
  ir = new IR(IR_PIN);
  servoMotor = new ServoMotor(SERVO_MOTOR_PIN);
  photoResistor = new PhotoResistor(PHOTO_RESISTOR_PIN);
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
    }
    else if (target == "greenLed")
    {
      if (action == "update")
      {
        bool state = lastMessage["payload"]["params"]["state"];
        if (state)
        {
          greenLed->turnOn();
        }
        else
        {
          greenLed->turnOff();
        }
      }
    }
    else if (target == "redLed")
    {
      if (action == "update")
      {
        bool state = lastMessage["payload"]["params"]["state"];
        if (state)
        {
          redLed->turnOn();
        }
        else
        {
          redLed->turnOff();
        }
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
        String message = createPublishMessage("sonar", "distanceMeasured", String("{ \"distance\": ") + distance + "}");
        webSocket.sendTXT(message);
      }
    }
    else if (target == "servo")
    {
      if (action == "rotate")
      {
        int angle = lastMessage["payload"]["params"]["angle"];
        servoMotor->update(angle);
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
        String message = createPublishMessage("ir", "presenceMeasured", String("{ \"presence\": ") + presence + "}");
        webSocket.sendTXT(message);
      }
    }
    else if (target == "photoResistor")
    {
      if (action == "detectBrightness")
      {
        isPhotoResistorDetecting = true;
        minPhotoResistorDetectBrightness = lastMessage["payload"]["params"]["min"];
        maxPhotoResistorDetectBrightness = lastMessage["payload"]["params"]["max"];
      }
      else if (action == "measureBrightness")
      {
        float brightness = photoResistor->measure();
        String message = createPublishMessage("photoResistor", "brightnessMeasured", String("{ \"brightness\": ") + brightness + "}");
        webSocket.sendTXT(message);
      }
    }
  }

  if (pressDetected)
  {
    detachInterrupt(digitalPinToInterrupt(BUTTON_PIN));
    pressDetected = false;

    String message = createPublishMessage("button", "pressDetected", "{}");
    webSocket.sendTXT(message);
  }
  if (presenceDetected)
  {
    detachInterrupt(digitalPinToInterrupt(IR_PIN));
    presenceDetected = false;

    String message = createPublishMessage("ir", "presenceDetected", "{}");
    webSocket.sendTXT(message);
  }
  if (isSonarDetecting)
  {
    float distance = sonar->measure();
    if (distance >= minSonarDetectDistance && distance <= maxSonarDetectDistance)
    {
      isSonarDetecting = false;
      String message = createPublishMessage("sonar", "distanceDetected", "{}");
      webSocket.sendTXT(message);
    }
  }
  if (isPhotoResistorDetecting)
  {
    float brightness = photoResistor->measure();
    if (brightness >= minPhotoResistorDetectBrightness && brightness <= maxPhotoResistorDetectBrightness)
    {
      isPhotoResistorDetecting = false;
      String message = createPublishMessage("photoResistor", "brightnessDetected", "{}");
      webSocket.sendTXT(message);
    }
  }
}