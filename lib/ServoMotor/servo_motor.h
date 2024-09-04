#ifndef __SERVO_MOTOR__
#define __SERVO_MOTOR__
#include <Servo.h>
#include <Arduino.h>

class ServoMotor
{
    const int pin;
    int currentAngle;
    Servo servo;

public:
    ServoMotor(int pin);
    void update(int angle);
};

#endif