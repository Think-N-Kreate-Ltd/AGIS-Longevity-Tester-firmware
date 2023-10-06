#ifndef tester_common_h
#define tester_common_h

#include <Arduino.h>
#include <esp_log.h>

extern long TT;

extern bool resumeAfterCutOff;
extern uint64_t resumeStartTime;

extern int16_t PWM_P1UP;
extern int16_t PWM_P1DOWN;
extern int16_t PWM_P2UP;
extern int16_t PWM_P2DOWN;

extern uint8_t numTime_P1;
extern uint8_t numTime_P2;

extern uint8_t T_OUT_P1UP;
extern uint8_t T_OUT_P1DOWN;
extern uint8_t T_OUT_P2UP;
extern uint8_t T_OUT_P2DOWN;

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