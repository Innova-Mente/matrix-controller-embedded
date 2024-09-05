#include <servo_motor.h>

#define START_ANGLE 90
#define WAIT_IN_BETWEEN_CHANGES 10

ServoMotor::ServoMotor(int pin) : pin{pin}, servo{}
{
  servo.attach(pin);
  currentAngle = servo.read();
  setAngle(START_ANGLE);
}

void ServoMotor::setAngle(int newAngle)
{
  newAngle = constrain(newAngle, 0, 180);

  if (newAngle >= currentAngle)
  {
    for (int pos = currentAngle; pos <= newAngle; pos++)
    {
      servo.write(pos);
      delay(WAIT_IN_BETWEEN_CHANGES);
    }
  }
  else
  {
    for (int pos = currentAngle; pos >= newAngle; pos--)
    {
      servo.write(pos);
      delay(WAIT_IN_BETWEEN_CHANGES);
    }
  }

  currentAngle = newAngle;
}

void ServoMotor::incrementAngle(int increment)
{
  setAngle(currentAngle + increment);
}
