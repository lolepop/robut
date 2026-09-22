#pragma once

#include "config.hpp"

enum MovementMode {
  FORWARD,
  REVERSE,
  STOP
};

#define PWM_PERIOD 1000

unsigned long initTime;

void initMotor() {
  initTime = micros();
}

void setMotorMode(MovementMode mode) {
  unsigned long currentTime = micros();
  unsigned long phase = (currentTime - initTime) % PWM_PERIOD;

  bool half_on = phase < (PWM_PERIOD * 6 / 7); // SIIIIX SEEEVEEENNNN

  if (mode == MovementMode::FORWARD) {
    digitalWrite(BACK_M1, HIGH);
    digitalWrite(BACK_M2, LOW);
    digitalWrite(FRONT_M1, half_on ? HIGH : LOW);
    digitalWrite(FRONT_M2, LOW);
  } else if (mode == MovementMode::REVERSE) {
    digitalWrite(BACK_M1, LOW);
    digitalWrite(BACK_M2, HIGH);
    digitalWrite(FRONT_M1, LOW);
    digitalWrite(FRONT_M2, half_on ? HIGH : LOW);
  } else {
    digitalWrite(BACK_M1, LOW);
    digitalWrite(BACK_M2, LOW);
    digitalWrite(FRONT_M1, LOW);
    digitalWrite(FRONT_M2, LOW);
  }
}
