#include <Arduino.h>
#include <ezButton.h>

bool print = true;

/*-------------------define for pins-------------------*/

#define MOTOR_CTRL_PIN_1 15 // Motorl Control Board PWM 1
#define MOTOR_CTRL_PIN_2 16 // Motorl Control Board PWM 2

/*-------------------ezButton object-------------------*/

ezButton limitSwitch_Up(37);   // create ezButton object that attach to pin 37
ezButton limitSwitch_Down(38); // create ezButton object that attach to pin 38

/*----------------var for control motor----------------*/

volatile bool motorHoming = true;   // will directly go to homing when true
bool motorState = true;    // for checking the motor is moving Up or Down, ture=Up
uint32_t recordTime;       // for record the time of motor 
uint32_t startTime = millis();      // for record the starting time of the test 

/*-----------------var for user inputs-----------------*/

int16_t PWM_P1UP = 255;     // PWM of motor, postitve=move up, negetive=move down
int16_t PWM_P1DOWN = -153;  // PWM of motor, postitve=move up, negetive=move down
int16_t PWM_P2UP = 102;     // PWM of motor, postitve=move up, negetive=move down
int16_t PWM_P2DOWN = -102;  // PWM of motor, postitve=move up, negetive=move down
bool pauseState = false;    // will pause the test will it goes to true

// Function prototypes
// bool limitSwitchTouched(ezButton limitSwitch);
void pauseAll(bool state);
void motorOn(int PWM);
void motorP1(uint8_t time=3);
void motorP2(uint8_t time=7);
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

void loop() {
  if (print) {
    Serial.printf("Up:%d, Down:%d\n", limitSwitch_Up.getStateRaw(), limitSwitch_Down.getStateRaw());
    print = false;
  }
}

// bool limitSwitchTouched(ezButton limitSwitch) {
//   limitSwitch.loop();
//   if (limitSwitch.getState()) return false; // untouched
//   else return true;                         // touched
// }

// pause the test
// do not want to use sleep as it is harmful to the program
// can try to use delay() in main loop but not recommened
void pauseAll(bool state) {
  if (state) {
    recordTime = millis();
    while (state) {
      vTaskDelay(200);
    }

    // when resume, record the time again
    startTime += millis() - recordTime;
  }
  
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
// time is the number of time that motor On/Down should run, default is 3
void motorP1(uint8_t time) {
  static bool state = true; // motor move up first
  if (state) {
    motorOn(PWM_P1UP);
    do {
      vTaskDelay(20);
      pauseAll(pauseState);
    } while (limitSwitch_Up.getStateRaw() == 1);
    state = false;  // motor move down
  } else {
    motorOn(PWM_P1DOWN);
    do {
      vTaskDelay(20);
      pauseAll(pauseState);
    } while (limitSwitch_Down.getStateRaw() == 1);
    state = true;
    motorOn(0); // for safety, write the motor to stop, will overwrite soon
  }

  if (time >= 1) { // run the next time
    Serial.printf("pattern 1 finish half, %d times remains\n", time);
    motorP1(time-1);
  }
}

// run the motor for pattern 2
// state should be the motorState, can help to directly change (for checking in other place)
// time is the number of time that motor On/Down should run, default is 7
void motorP2(uint8_t time) {
  static bool state = true; // motor move up first
  if (state) {
    motorOn(PWM_P2UP);
    vTaskDelay(20);
      for (uint8_t i=0; i<50; ++i) { // total delay for 20*50=1000ms
        if (limitSwitch_Up.getStateRaw() == 1) {
          vTaskDelay(20);
          pauseAll(pauseState);
        } else {
          i=50; // if touch limit SW, directory go to next state
        }
      }
    state = false;  // motor move down
  } else {
    motorOn(PWM_P2DOWN);
    do {
      vTaskDelay(20);
      pauseAll(pauseState);
    } while (limitSwitch_Down.getStateRaw() == 1);
    state = true;
    motorOn(0); // for safety, write the motor to stop, will overwrite soon
  }

  if (time >= 1) { // run the next time
    Serial.printf("pattern 2 finish half, %d times remains\n", time);
    motorP2(time-1);
  }
}

void motorCycle(void * arg) {
  while (motorHoming) {
    vTaskDelay(20);
  }
  Serial.println("homing completed");
  for (;;) {
    motorP1();
    motorP2();
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
      print = true;
    }

    vTaskDelay(50);
  }
}