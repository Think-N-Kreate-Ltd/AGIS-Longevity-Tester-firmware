#ifndef tester_common_h
#define tester_common_h

#include <Arduino.h>
#include <esp_log.h>

extern long TT;

extern bool resumeAfterCutOff;
// extern uint64_t resumeStartTime;

struct MotorSetting {
  int16_t PWM_UP;      // PWM of motor of move up, postitve=move up, negetive=move down
  int16_t PWM_DOWN;    // PWM of motor of move down, postitve=move up, negetive=move down
  uint8_t numTime;     // no. of time that run as a cycle, should multiply by 2 and subtract by 1
  uint8_t T_OUT_UP;    // timeout of motor of move up
  uint8_t T_OUT_DOWN;  // timeout of motor of move down
};
extern MotorSetting setPattern[];
extern uint8_t sizeOfPattern;

extern uint8_t T_P2running;

extern uint64_t sampleId;
extern char dateTime[64];
extern bool loadProfile;
extern bool downloadFile;

extern float current_mA;
extern float avgCurrent_mA;
extern bool powerFail;

// extern bool motorState;
// extern uint8_t cycleState;
// extern bool testState;
// extern bool pauseState;
// extern uint64_t motorRunTime;
// extern uint64_t numCycle;

struct MotorStatus {
  bool motorState;      // for checking the motor is moving Up or Down, ture=Up
  uint8_t cycleState;   // for checking the motor is moving which cycle, 0 for stop
  bool testState;       // true after user finish input and start, until homing finish
  bool pauseState;      // will pause the test will it goes to true
  uint64_t motorRunTime;// the total time that the motor run, not including the pause time
  uint64_t numCycle;    // for recording the number of cycle
  uint8_t passedNum;    // new added, to record how many time the LS is touched in a cycle
};
extern MotorStatus status;

enum class failReason_t {
  NOT_YET,
  PRESS_KEY,
  CURRENT_EXCEED,
  TIME_OUT
};
extern failReason_t failReason;

#endif  // tester_common_h