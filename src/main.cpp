#include <Arduino.h>
#include <ArduinoJson.h>
#include <utils.h>

#include <led_matrix.h>
#include <led.h>
#include <sonar.h>
#include <pir.h>
#include <servo_motor.h>
#include <photo_resistor.h>

#define USE_SERIAL Serial
#define DEVICE_NUMBER String("1")

#define LED_MATRIX_PIN D1
#define LED_PIN D8
#define SONAR_ECHO_PIN D5
#define SONAR_TRIG_PIN D6
#define PIR_PIN D0
#define BUTTON_PIN D2
#define SERVO_MOTOR_PIN D7
#define PHOTO_RESISTOR_PIN A0

LedMatrix *ledMatrix;
Led *led;
Sonar *sonar;
Pir *pir;
ServoMotor *servoMotor;
PhotoResistor *photoResistor;

bool pressDetected = false;
bool presenceDetected = false;

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

void IRAM_ATTR pirInterrupt()
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
  led = new Led(LED_PIN);
  sonar = new Sonar(SONAR_TRIG_PIN, SONAR_ECHO_PIN);
  pir = new Pir(PIR_PIN);
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
    else if (target == "led")
    {
      if (action == "update")
      {
        bool state = lastMessage["payload"]["params"]["state"];
        if (state)
        {
          led->turnOn();
        }
        else
        {
          led->turnOff();
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
    else if (target == "pir")
    {
      if (action == "detectPresence")
      {
        attachInterrupt(digitalPinToInterrupt(PIR_PIN), pirInterrupt, RISING);
      }
      else if (action == "measurePresence")
      {
        String presence = pir->detectPresence() ? "true" : "false";
        String message = createPublishMessage("pir", "presenceMeasured", String("{ \"presence\": ") + presence + "}");
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
    detachInterrupt(digitalPinToInterrupt(PIR_PIN));
    presenceDetected = false;

    String message = createPublishMessage("pir", "presenceDetected", "{}");
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