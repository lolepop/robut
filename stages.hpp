#pragma once

#include <Servo.h>
#include "motor.hpp"

const unsigned long minTimeOverRamp = 5 * 1000; // time until ramp is definitely cleared
const float sensorStopDist = 20.0f; // cutoff distance to stop, adjust for 5cm termination distance

const int restAngle = 115; // angle at rest
const int throwAngle = 0; // final throw angle

enum RobotStage {
  INIT = 0,
  RUNNING = 1,
  WALL_REACHED = 2,
  BACKWARD = 3,
  TERMINATED = 4
};

struct Stage {
  virtual ~Stage() = default;

  virtual bool loop(float distCm) = 0;
  virtual RobotStage stageRepr() = 0;
};

// before running starts
struct InitStage : public Stage {
  unsigned long startTime;
  const unsigned long minStartTime = 500; // wait before starting gesture will be recognised
  const float minStartGestureDist = 10.0f;

  InitStage(unsigned long timeNow) : startTime(timeNow) {}
  ~InitStage() override {}
  
  bool loop(float distCm) override {
    Serial.println("Initializing!");
    auto timeNow = millis();
    auto timeElapsed = timeNow - startTime;
    return timeElapsed >= minStartTime && distCm <= minStartGestureDist;
  }

  RobotStage stageRepr() override {
    return RobotStage::INIT;
  }
};

struct RunningStage : public Stage {
  unsigned long startTime;
  unsigned long& completionTimeOut;

  RunningStage(unsigned long& completionTimeOut, unsigned long timeNow) : startTime(timeNow), completionTimeOut(completionTimeOut) {}
  ~RunningStage() override {}
  
  bool loop(float distCm) override {
    Serial.print("Running! ");
    Serial.println(distCm);

    auto timeNow = millis();
    auto timeElapsed = timeNow - startTime;
    completionTimeOut = timeElapsed;
    bool shouldStop = timeElapsed >= minTimeOverRamp && distCm <= sensorStopDist;
    setMotorMode(MovementMode::FORWARD);
    return shouldStop;
  }

  RobotStage stageRepr() override {
    return RobotStage::RUNNING;
  }
};

struct ThrowingStage : public Stage {
  unsigned long startTime;
  Servo& servo;

  ThrowingStage(Servo& servo, unsigned long timeNow) : startTime(timeNow), servo(servo) {}
  ~ThrowingStage() override {}
  
  bool loop(float distCm) override {
    Serial.println("Throwing!");
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
    return RobotStage::WALL_REACHED;
  }
};

// run backwards until forward 1 way time is elapsed (back at start)
struct BackwardStage : public Stage {
  unsigned long startTime;
  unsigned long targetDuration;
  const unsigned long durationOffset = 0; // if it needs longer or shorter amount of time relative to initial path

  BackwardStage(unsigned long targetDuration, unsigned long timeNow) : startTime(timeNow), targetDuration(targetDuration) {}
  ~BackwardStage() override {}
  
  bool loop(float distCm) override {
    Serial.println("Back!");
    setMotorMode(MovementMode::REVERSE);
    auto timeNow = millis();
    auto timeElapsed = timeNow - startTime;
    return timeElapsed >= targetDuration + durationOffset;
  }

  RobotStage stageRepr() override {
    return RobotStage::BACKWARD;
  }
};

struct TerminatedStage : public Stage {
  TerminatedStage() {}
  ~TerminatedStage() override {}
  
  bool loop(float distCm) override {
    Serial.println("Done!");
    setMotorMode(MovementMode::STOP);
    return false;
  }

  RobotStage stageRepr() override {
    return RobotStage::TERMINATED;
  }
};