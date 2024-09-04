#ifndef __PIR__
#define __PIR__
#include <Arduino.h>

class Pir
{
    const int outPin;

public:
    Pir(int outPin);
    bool detectPresence();
};

#endif