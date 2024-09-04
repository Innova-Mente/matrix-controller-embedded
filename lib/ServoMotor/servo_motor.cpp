#include <servo_motor.h>

#define SERVO_CHANGE_STEP 1
#define SERVO_WAIT_BETWEEN_CHANGES 5

ServoMotor::ServoMotor(int pin) : pin{pin}, currentAngle{0}, servo{}
{
  this->servo.attach(pin);
  this->servo.write(this->currentAngle);
}

void ServoMotor::update(int angle)
{
  int newAngle = min(max(currentAngle + angle, 0), 180);
  if (currentAngle >= newAngle)
  {
    for (int pos = currentAngle; pos >= newAngle; pos -= SERVO_CHANGE_STEP)
    {
      this->servo.write(pos);
      delay(SERVO_WAIT_BETWEEN_CHANGES);
    }
  }
  else
  {
    for (int pos = currentAngle; pos <= newAngle; pos += SERVO_CHANGE_STEP)
    {
      this->servo.write(pos);
      delay(SERVO_WAIT_BETWEEN_CHANGES);
    }
  }
  this->currentAngle = newAngle;
}
