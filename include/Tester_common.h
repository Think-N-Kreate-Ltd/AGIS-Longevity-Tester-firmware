#ifndef tester_common_h
#define tester_common_h

#include <Arduino.h>

extern long TT;

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

extern bool motorState;
extern uint8_t cycleState;
extern bool testState;
extern bool pauseState;
extern uint64_t motorRunTime;
extern uint64_t numCycle;

enum class failReason_t {
  NOT_YET,
  PRESS_KEY,
  CURRENT_EXCEED,
  TIME_OUT
};
extern failReason_t failReason;

#endif  // tester_common_h