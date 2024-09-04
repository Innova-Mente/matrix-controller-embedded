#ifndef __BUTTON__
#define __BUTTON__
#include <Arduino.h>

class Button
{
  int pin;

public:
  Button(int pin);
  bool isPressed();
};

#endif