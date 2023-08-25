#include <Arduino.h>
#include <ezButton.h>

/*-------------------define for pins-------------------*/

#define MOTOR_CTRL_PIN_1 15 // Motorl Control Board PWM 1
#define MOTOR_CTRL_PIN_2 16 // Motorl Control Board PWM 2

/*-------------------ezButton object-------------------*/

ezButton limitSwitch_Up(37);   // create ezButton object that attach to pin 37
ezButton limitSwitch_Down(38); // create ezButton object that attach to pin 38

/*----------------var for control motor----------------*/
volatile bool motorHoming = true;   // will directly go to homing when true
bool motorState = true;    // for checking the motor is moving Up or Down, ture=Up

// Function prototypes
bool limitSwitchTouched(ezButton limitSwitch);
void motorOn(int PWM);
void motorP1(bool& state, uint8_t time=2);
void motorP2(bool& state, uint8_t time=4);
void motorCycle(void * arg);
void homingRollerClamp(void * arg);

void setup() {
  Serial.begin(115200);

  /*Create a task for running motor up and down continuously */
  xTaskCreate(motorCycle,
              "Running the Motor Cycle",
              4096,
              NULL,
              12,  // highest priority task
              NULL);
  
  // *Create a task for different kinds of little things
  xTaskCreate(homingRollerClamp,      // function that should be called
              "Homing roller clamp",  // name of the task (debug use)
              4096,           // stack size
              NULL,           // parameter to pass
              13,             // task priority, 0-24, 24 highest priority
              NULL);          // task handle

}

void loop() {/*should be nothing*/}

bool limitSwitchTouched(ezButton limitSwitch) {
  limitSwitch.loop();
  if (limitSwitch.getState()) return false; // untouched
  else return true;                         // touched
}

// move the motor
// set PWM to be positive to move up, negetive to move down, 0 to stop
// max PWM is 255 (100%), half is 122 (50%)
void motorOn(int PWM) {
  if (PWM >= 0) {
    analogWrite(MOTOR_CTRL_PIN_1, PWM);
    analogWrite(MOTOR_CTRL_PIN_2, 0);
  } else {
    PWM = abs(PWM);
    analogWrite(MOTOR_CTRL_PIN_1, 0);
    analogWrite(MOTOR_CTRL_PIN_2, PWM);
  }
}

// run the motor for pattern 1
// state should be the motorState, can help to directly change (for checking in other place)
// time is the number of time that pattern 1 should run, default is 2
void motorP1(bool& state, uint8_t time) {
  state = true; // motor move up first
  if (state) {
    motorOn(255);
    while (!limitSwitchTouched(limitSwitch_Up)) {
      vTaskDelay(20);
    }
    state = false;  // motor move down
  } else {
    motorOn(-76);
    while (!limitSwitchTouched(limitSwitch_Down)) {
      vTaskDelay(20);
    }
    state = true;
    motorOn(0); // for safety, write the motor to stop, will overwrite soon
  }

  if (time > 0) { // run the next time
    motorP1(state, time-1);
  }
}

// run the motor for pattern 2
// state should be the motorState, can help to directly change (for checking in other place)
// time is the number of time that pattern 1 should run, default is 4
void motorP2(bool& state, uint8_t time) {
  state = true; // motor move up first
  if (state) {
    motorOn(51);
    while (!limitSwitchTouched(limitSwitch_Up)) {
      vTaskDelay(20);
    }
    state = false;  // motor move down
  } else {
    motorOn(-51);
    while (!limitSwitchTouched(limitSwitch_Down)) {
      vTaskDelay(20);
    }
    state = true;
    motorOn(0); // for safety, write the motor to stop, will overwrite soon
  }

  if (time > 0) { // run the next time
    motorP1(state, time-1);
  }
}

void motorCycle(void * arg) {
  for (;;) {
    motorP1(motorState);
    motorP2(motorState);
  }
}

void homingRollerClamp(void * arg) {
  for(;;) {
    if (limitSwitch_Down.getStateRaw() == 0) {  // touched
      motorHoming = false;
      motorOn(0);
    } else {
      // motor move down
      motorOn(-255);
    }

    while (!motorHoming) {
      vTaskDelay(200);
    }

    vTaskDelay(50);
  }
}