#ifndef __PHOTO_RESISTOR__
#define __PHOTO_RESISTOR__
#include <Arduino.h>

class PhotoResistor
{
    const int pin;

public:
    PhotoResistor(int pin);
    int measure();
};

#endif