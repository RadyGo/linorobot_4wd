#include "Arduino.h"
#include "Motor.h"

Motor::Motor(int motor_pinA, int motor_pinB)
{
  pinMode(motor_pinA, OUTPUT);
  pinMode(motor_pinB, OUTPUT);

  _motor_pinA = motor_pinA;
  _motor_pinB = motor_pinB;
}

void Motor::calculate_rpm(long current_encoder_ticks)
{
    //this function calculates the motor's RPM based on encoder ticks and delta time
    unsigned long current_time = millis();
    unsigned long dt = current_time - _previous_rpm_time;
    //convert the time from milliseconds to minutes
    double dtm = (double)dt / 60000;
    //calculate change in number of ticks from the encoder
    double delta_ticks = current_encoder_ticks - _previous_encoder_ticks;
    //calculate wheel's speed (RPM)
    current_rpm = (delta_ticks / double(counts_per_rev))/ dtm;
    _previous_encoder_ticks = current_encoder_ticks;
    _previous_rpm_time = current_time;
}

int Motor::calculate_pwm()
{
  // this function implements a PID controller and calculates the PWM required
  // to drive the motor
  double error;
  double pwm;
  // calculate Proportional
  error = required_rpm - current_rpm;

  pwm = (Kp * error) + (Ki * _total_pid_error) + (Kd * (error - _previous_pid_error));

  _prev_pwm = pwm;
  _previous_pid_error = error;

  //make sure calculated pwm value is within PWM range
  return constrain(pwm, -255, 255);
}

void Motor::spin(int pwm)
{
  // this function spins the motor based on calculated PWM
  if (pwm > 0)
  {
    analogWrite(_motor_pinA, 0);
    analogWrite(_motor_pinB, abs(pwm));
  } else if (pwm < 0)
  {
    analogWrite(_motor_pinB, 0);
    analogWrite(_motor_pinA, abs(pwm));
  } else
  {
    analogWrite(_motor_pinB, 0);
    analogWrite(_motor_pinA, 0);
  }
}

void Motor::stop()
{
  required_rpm = 0;
  //speed up PID in stopping the motor once PWM hits 30
  if (_prev_pwm <= 30 || _prev_pwm >= 30)
  {
    //reset PID variables
    _total_pid_error = 0;
    _prev_pwm = 0;
    _previous_pid_error = 0;
    spin(0);
  }
}
