#include <photo_resistor.h>

PhotoResistor::PhotoResistor(int pin) : pin{pin}
{
  pinMode(pin, INPUT);
}

int PhotoResistor::measure()
{
  int absoluteValue = analogRead(this->pin);
  float percentage = map(absoluteValue, 0, 1023, 0, 100);

  return percentage;
}
