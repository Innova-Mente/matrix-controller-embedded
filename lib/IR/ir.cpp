#include <ir.h>

IR::IR(int outPin) : outPin{outPin}
{
  pinMode(outPin, INPUT_PULLUP);
}

bool IR::detectPresence()
{
  return digitalRead(outPin) == HIGH;
}