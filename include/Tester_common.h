#ifndef tester_common_h
#define tester_common_h

#include <Arduino.h>

extern long TT;

extern int16_t PWM_P1UP;
extern int16_t PWM_P2UP;
extern int16_t PWM_P2DOWN;

extern uint8_t numTime_P1;
extern uint8_t numTime_P2;

extern uint8_t T_OUT_P1UP;
extern uint8_t T_OUT_P1DOWN;
extern uint8_t T_OUT_P2UP;
extern uint8_t T_OUT_P2DOWN;

extern uint64_t sampleId;
extern char dateTime[64];
extern bool loadProfile;
extern bool downloadFile;

extern uint64_t motorRunTime;

extern float current_mA;
extern float avgCurrent_mA;

#endif  // tester_common_h