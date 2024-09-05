#include <pir.h>

Pir::Pir(int outPin) : outPin{outPin}
{
  pinMode(outPin, INPUT_PULLUP);
}

bool Pir::detectPresence()
{
  return digitalRead(this->outPin) == HIGH;
}