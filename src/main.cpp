#include <Arduino.h>
#include <ezButton.h>

#define MOTOR_CTRL_PIN_1 15 // Motorl Control Board PWM 1
#define MOTOR_CTRL_PIN_2 16 // Motorl Control Board PWM 2
#define PWM_PIN          4  // input pin for the potentiometer

ezButton limitSwitch_Up(37);   // create ezButton object that attach to pin 37
ezButton limitSwitch_Down(38); // create ezButton object that attach to pin 38
volatile int PWMValue = 0;     // PWM value to control the speed of motor

// Function prototypes
bool limitSwitchTouched(ezButton limitSwitch);
void motorUpAndDownTask(void * arg);
void motorOnUp();
void motorOnDown();
void motorOff();

void setup() {
  Serial.begin(115200);

  /*Create a task for running motor up and down continuously */
  xTaskCreate(motorUpAndDownTask,
              "Running Motor Up and Down continuously",
              4096,
              NULL,
              24,  // highest priority task
              NULL);

}

void loop() {
//   if (limitSwitchTouched(limitSwitch_Up)) {
//     Serial.printf("Up Touched\n");
//   }
// //   else {
// //     Serial.printf("Up Untouched\n");
// //   }

//   if (limitSwitchTouched(limitSwitch_Down)) {
//     Serial.printf("Down Touched\n");
//   }
//   else {
//     Serial.printf("Down Untouched\n");
//   }
  
//   delay(200);
}

bool limitSwitchTouched(ezButton limitSwitch) {
  limitSwitch.loop();
  if (limitSwitch.getState()) return false; // untouched
  else return true;                         // touched
}

void motorOnUp() {
  Serial.println("up");
  // limitSwitch_Up.loop();   // MUST call the loop() function first

  // if (limitSwitch_Up.getState()) { // untouched
    // Read PWM value
    PWMValue = analogRead(PWM_PIN);

    analogWrite(MOTOR_CTRL_PIN_1, (PWMValue / 16)); // PWMValue: 0->4095
    analogWrite(MOTOR_CTRL_PIN_2, 0);
  // }
  // else { // touched
  //   motorOff();
  // }
}

void motorOnDown() {
  // Serial.println("down");
  // limitSwitch_Down.loop();   // MUST call the loop() function first

  // if (limitSwitch_Down.getState()) { // untouched
    // Read PWM value
    PWMValue = analogRead(PWM_PIN);

    analogWrite(MOTOR_CTRL_PIN_2, (PWMValue / 16)); // PWMValue: 0->4095
    analogWrite(MOTOR_CTRL_PIN_1, 0);
  // }
  // else { // touched
  //   motorOff();
  // }
}

void motorOff() {
  analogWrite(MOTOR_CTRL_PIN_1, 0);
  analogWrite(MOTOR_CTRL_PIN_2, 0);
}

void motorUpAndDownTask(void * arg) {
  for (;;) {
    while (!limitSwitchTouched(limitSwitch_Up)) {
      motorOnUp();
    }
    motorOff();

    while (!limitSwitchTouched(limitSwitch_Down)) {
      motorOnDown();
    }
    motorOff();
    
    vTaskDelay(10);
  }
}