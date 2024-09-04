#ifndef __SONAR__
#define __SONAR__
#include <Arduino.h>

class Sonar
{
    const int trigPin;
    const int echoPin;

public:
    Sonar(int trigPin, int echoPin);
    bool measure();
};

#endif