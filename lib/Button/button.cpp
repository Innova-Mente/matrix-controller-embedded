#include <button.h>

Button::Button(int pin) : pin{pin}
{
  pinMode(pin, INPUT_PULLUP);
}

bool Button::isPressed()
{
  return digitalRead(this->pin) == HIGH;
}
