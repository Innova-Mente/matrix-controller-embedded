#ifndef __IR__
#define __IR__
#include <Arduino.h>

class IR
{
  const int outPin;

public:
  IR(int outPin);
  bool detectPresence();
};

#endif