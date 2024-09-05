#include <led.h>

Led::Led(int pin) : pin{pin}
{
  pinMode(pin, OUTPUT);
  turnOff();
}

void Led::turnOff()
{
  digitalWrite(pin, LOW);
}

void Led::turnOn()
{
  digitalWrite(pin, HIGH);
}

void Led::setState(bool state)
{
  if (state)
  {
    turnOn();
  }
  else
  {
    turnOff();
  }
}
