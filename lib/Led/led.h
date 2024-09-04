#ifndef __LED__
#define __LED__
#include <Arduino.h>

class Led
{
	int pin;

public:
	Led(int pin);
	void turnOff();
	void turnOn();
};

#endif