#pragma once

#include <Servo.h>
#include "motor.hpp"

const unsigned long minTimeOverRamp = 10 * 1000; // time until ramp is definitely cleared
const float sensorStopDist = 20.0f; // cutoff distance to stop, adjust for 5cm termination distance

const int restAngle = 0; // angle at rest
const int throwAngle = 90; // final throw angle

enum RobotStage {
  INIT,
  RUNNING,
  WALL_REACHED,
  BACKWARD,
  TERMINATED
};

struct Stage {
  virtual bool loop(float distCm) = 0;
  virtual RobotStage stageRepr() = 0;
};

// before running starts
struct InitStage : public Stage {
  unsigned long startTime;
  const unsigned long minStartTime = 500; // wait before starting gesture will be recognised
  const float minStartGestureDist = 10.0f;

  InitStage(unsigned long timeNow) : startTime(timeNow) {}
  
  bool loop(float distCm) override {
    auto timeNow = millis();
    auto timeElapsed = timeNow - startTime;
    return timeElapsed >= minStartTime && distCm <= minStartGestureDist;
  }

  RobotStage stageRepr() override {
    RobotStage::INIT;
  }
};

struct RunningStage : public Stage {
  unsigned long startTime;
  unsigned long& completionTimeOut;

  RunningStage(unsigned long& completionTimeOut, unsigned long timeNow) : startTime(timeNow), completionTimeOut(completionTimeOut) {}
  
  bool loop(float distCm) override {
    auto timeNow = millis();
    auto timeElapsed = timeNow - startTime;
    completionTimeOut = timeElapsed;
    bool shouldStop = timeElapsed >= minTimeOverRamp && distCm <= sensorStopDist;
    setMotorMode(MovementMode::FORWARD);
    return shouldStop;
  }

  RobotStage stageRepr() override {
    RobotStage::RUNNING;
  }
};

struct ThrowingStage : public Stage {
  unsigned long startTime;
  Servo& servo;

  ThrowingStage(Servo& servo, unsigned long timeNow) : startTime(timeNow), servo(servo) {}
  
  bool loop(float distCm) override {
    setMotorMode(MovementMode::STOP);
    // TODO: may want to make this async
    delay(500);
    servo.write(throwAngle);
    delay(500);
    servo.write(restAngle);
    delay(300); // dont need to wait for full retraction
    return true;
  }

  RobotStage stageRepr() override {
    RobotStage::WALL_REACHED;
  }
};

// run backwards until forward 1 way time is elapsed (back at start)
struct BackwardStage : public Stage {
  unsigned long startTime;
  unsigned long targetDuration;
  const unsigned long durationOffset = 0; // if it needs longer or shorter amount of time relative to initial path

  BackwardStage(unsigned long targetDuration, unsigned long timeNow) : startTime(timeNow), targetDuration(targetDuration) {}
  
  bool loop(float distCm) override {
    setMotorMode(MovementMode::REVERSE);
    auto timeNow = millis();
    auto timeElapsed = timeNow - startTime;
    return timeElapsed >= targetDuration + durationOffset;
  }

  RobotStage stageRepr() override {
    RobotStage::BACKWARD;
  }
};

struct TerminatedStage : public Stage {
  TerminatedStage() {}
  
  bool loop(float distCm) override {
    setMotorMode(MovementMode::STOP);
    return false;
  }

  RobotStage stageRepr() override {
    RobotStage::TERMINATED;
  }
};