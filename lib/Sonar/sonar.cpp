#include <sonar.h>

Sonar::Sonar(int trigPin, int echoPin) : trigPin{trigPin}, echoPin{echoPin}
{
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
};

float Sonar::measure()
{
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  float distance = pulseIn(echoPin, HIGH) * 0.034 / 2;

  delay(50); // in case we call measure multiple times in a row

  return distance;
}
