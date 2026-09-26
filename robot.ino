#include <Servo.h>
#include "config.hpp"
#include "stages.hpp"
#include "motor.hpp"
#include "sensor.hpp"

// Speed of sound in cm/μs.
#define SPEED_OF_SOUND 0.0345

Servo servo;
Stage* currentStage = nullptr;
unsigned long oneWayCompletionTime = 0;

void setup() {
  pinMode(TRIG, OUTPUT);
  digitalWrite(TRIG, LOW);

  pinMode(ECHO, INPUT);
  Serial.begin(9600);

  pinMode(FRONT_M1, OUTPUT);
  pinMode(FRONT_M2, OUTPUT);
  pinMode(BACK_M1, OUTPUT);
  pinMode(BACK_M2, OUTPUT);

  servo.attach(SERVO_PIN, 660, 2400); // TODO: calibrate this
  servo.write(restAngle);

  currentStage = new InitStage(millis());
  initMotor();
}

void runStage(float sensorDistCm) {
  if (currentStage == nullptr)
    return;

  RobotStage stageType = currentStage->stageRepr();
  if (!currentStage->loop(sensorDistCm))
    return;
  delete currentStage;

  switch (stageType) {
    case RobotStage::INIT:
      currentStage = new RunningStage(oneWayCompletionTime, millis());
    break;
    case RobotStage::RUNNING:
      currentStage = new ThrowingStage(servo, millis());
    break;
    case RobotStage::WALL_REACHED:
      currentStage = new BackwardStage(oneWayCompletionTime, millis());
    break;
    case RobotStage::BACKWARD:
      currentStage = new TerminatedStage();
    break;
    case RobotStage::TERMINATED:
      Serial.println("should not happen");
      currentStage = nullptr;
    break;
  }

}

void loop() {
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);

  int microsecs = pulseIn(ECHO, HIGH);
  int averagedMicrosecs = appendSensorData(microsecs);
  float cms = averagedMicrosecs * SPEED_OF_SOUND * 0.5f;
  // Serial.print(cms);
  // Serial.print(":\t");
  runStage(cms);

  delay(10);
}