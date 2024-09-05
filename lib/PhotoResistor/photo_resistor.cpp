#include <photo_resistor.h>

PhotoResistor::PhotoResistor(int pin) : pin{pin}
{
  pinMode(pin, INPUT);
}

float PhotoResistor::measure()
{
  int absoluteValue = analogRead(this->pin);
  float percentage = absoluteValue / 1023 * 100;

  return percentage;
}
