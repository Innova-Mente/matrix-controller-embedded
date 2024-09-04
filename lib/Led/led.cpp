#include <led.h>

Led::Led(int pin) : pin{pin}
{
  pinMode(pin, OUTPUT);
  turnOff();
}

void Led::turnOff()
{
  digitalWrite(this->pin, LOW);
}

void Led::turnOn()
{
  digitalWrite(this->pin, HIGH);
}
