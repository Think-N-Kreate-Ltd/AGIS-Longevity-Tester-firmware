#include <Arduino.h>
#include <ezButton.h>
#include <WiFi.h>
#include <TESTER_INA219.h>
#include <TESTER_LOGGING.h>

bool print = true;
long TT;

/*-------------------define for pins-------------------*/

#define MOTOR_CTRL_PIN_1 15 // Motorl Control Board PWM 1
#define MOTOR_CTRL_PIN_2 16 // Motorl Control Board PWM 2

/*-------------------ezButton object-------------------*/

ezButton limitSwitch_Up(37);   // create ezButton object that attach to pin 37
ezButton limitSwitch_Down(38); // create ezButton object that attach to pin 38

/*----------------var for control motor----------------*/

volatile bool motorHoming = true;   // will directly go to homing when true
bool motorState = true;    // for checking the motor is moving Up or Down, ture=Up
uint64_t recordTime;       // for record the time of motor 
uint64_t startTime = millis();      // for record the starting time of the test 

bool pauseState = false;    // will pause the test will it goes to true

/*-----------------var for user inputs-----------------*/

int16_t PWM_P1UP = 255;     // PWM of motor of pattern 1 move up, postitve=move up, negetive=move down
int16_t PWM_P1DOWN = -153;  // PWM of motor of pattern 1 move down, postitve=move up, negetive=move down
int16_t PWM_P2UP = 102;     // PWM of motor of pattern 2 move up, postitve=move up, negetive=move down
int16_t PWM_P2DOWN = -102;  // PWM of motor of pattern 2 move down, postitve=move up, negetive=move down

uint8_t numTime_P1 = 3;     // no. of time that pattern 1 run as a cycle, should multiply by 2 and subtract by 1
uint8_t numTime_P2 = 7;     // no. of time that pattern 2 run as a cycle, should multiply by 2 and subtract by 1

uint8_t T_OUT_P1UP = 10;    // timeout of motor of pattern 1 move up
uint8_t T_OUT_P1DOWN = 10;  // timeout of motor of pattern 1 move down
uint8_t T_OUT_P2UP = 2;     // timeout of motor of pattern 2 move up
uint8_t T_OUT_P2DOWN = 2;   // timeout of motor of pattern 2 move down

uint64_t sampleId = 12345678;  // the sample ID, not define yet
char dateTime[64];          // string to store the start date and time
bool loadProfile = true;    // the option of load profile, true=default, false=predefine

/*-------------------var for display-------------------*/

float current_mA;       // the current at a specific time, unit=mA
float avgCurrent_mA;    // the average current in pass second, unit=mA
bool testState = false; // true after user finish input and start, until homing finish
uint64_t motorRunTime;  // the total time that the motor run, not including the pause time

/*------------------function protypes------------------*/

void stopTest();
void pauseAll(bool state);
void timeoutCheck(uint8_t timeout, uint32_t time);
void motorOn(int PWM);
void motorP1(uint8_t time=3);
void motorP2(uint8_t time=7);

/*--------------------task functions--------------------*/

void motorCycle(void * arg);
void homingRollerClamp(void * arg);
void getI2CData(void * arg);
void loggingData(void * parameter);
void enableWifi(void * arg);

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

  // I2C is too slow that cannot use interrupt
  // xTaskCreate(getI2CData,     // function that should be called
  //             "Get I2C Data", // name of the task (debug use)
  //             4096,           // stack size
  //             NULL,           // parameter to pass
  //             1,              // task priority, 0-24, 24 highest priority
  //             NULL);          // task handle

  /*Create a task for data logging*/
  xTaskCreate(loggingData,       /* Task function. */
              "Data Logging",    /* String with name of task. */
              4096,              /* Stack size in bytes. */
              NULL,              /* Parameter passed as input of the task */
              4,                 /* Priority of the task. */
              NULL);             /* Task handle. */
  
  xTaskCreate(enableWifi,     // function that should be called
              "Enable WiFi",  // name of the task (debug use)
              4096,           // stack size
              NULL,           // parameter to pass
              20,             // task priority, 0-24, 24 highest priority
              NULL);          // task handle
}

void loop() {
  if (print) {
    Serial.printf("record time: %d\n", TT);
    Serial.println(dateTime);
    print = false;
  }
}

/*------------------function protypes------------------*/

void stopTest() {
  motorHoming = true;
  while (motorHoming) {
    vTaskDelay(200);
  }
  Serial.println("homing completed, all stopped");

  // all finish after homing
  // TODO: think how to stop it, now cannot stop for current problem
  recordTime = millis();
  Serial.println(recordTime);
  vTaskDelay(UINT_MAX); 
}

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

// when timeout, stop the test
// timeout = timeout set bt users
// time = the last record time
void timeoutCheck(uint8_t timeout, uint32_t time) {
  if ((millis()-time) >= (timeout*1000)) {
    stopTest();
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
  static uint32_t recTime;
  if (state) {
    motorOn(PWM_P1UP);
    recTime = millis();
    TT = recTime;
    do {
      vTaskDelay(20);
      pauseAll(pauseState);
      timeoutCheck(T_OUT_P1UP, recTime);
    } while (limitSwitch_Up.getStateRaw() == 1);
    state = false;  // motor move down
  } else {
    motorOn(PWM_P1DOWN);
    recTime = millis();
    TT = recTime;
    do {
      vTaskDelay(20);
      pauseAll(pauseState);
      timeoutCheck(T_OUT_P1DOWN, recTime);
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
  static uint32_t recTime;
  if (state) {
    motorOn(PWM_P2UP);
    recTime = millis();
    TT = recTime;
    vTaskDelay(20);
      for (uint8_t i=0; i<50; ++i) { // total delay for 20*50=1000ms
        if (limitSwitch_Up.getStateRaw() == 1) {
          vTaskDelay(20);
          pauseAll(pauseState);
          timeoutCheck(T_OUT_P2UP, recTime);
        } else {
          i=50; // if touch limit SW, directory go to next state
        }
      }
    state = false;  // motor move down
  } else {
    motorOn(PWM_P2DOWN);
    recTime = millis();
    TT = recTime;
    do {
      vTaskDelay(20);
      pauseAll(pauseState);
      timeoutCheck(T_OUT_P2DOWN, recTime);
    } while (limitSwitch_Down.getStateRaw() == 1);
    state = true;
    motorOn(0); // for safety, write the motor to stop, will overwrite soon
  }

  if (time >= 1) { // run the next time
    Serial.printf("pattern 2 finish half, %d times remains\n", time);
    motorP2(time-1);
  }
}

/*--------------------task functions--------------------*/

void motorCycle(void * arg) {
  while (motorHoming) {
    vTaskDelay(20);
  }
  Serial.println("homing completed");
  for (;;) {
    motorP1(numTime_P1);
    motorP2(numTime_P2);
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

void getI2CData(void * arg) {
  ina219SetUp();
  for (;;) {
    // get current data every second
    getCurrent();
    if (avgCurrent_mA >= 150) {
      stopTest();
      Serial.println(avgCurrent_mA);
    }
  }
}

void loggingData(void * parameter) {
  // set up, only run once
  // rmOldData(); // move to `enableWifi` as this is not needed with no wifi connection
  static bool finishLogging = false;

  for (;;) {
    if (testState) {
      newFileInit();  // create new file and header
      Serial.println("Logging initialized");
      
      // after create file, wait for finish
      // data logging will be done when in needed
      while (testState)  {
        vTaskDelay(500);
        // logData();
      }

      finishLogging = true;
    }

    // only run once when finish
    if (finishLogging) {
      while (motorHoming) {
        // wait for homing complete to log the last data
        vTaskDelay(200);
      }
      endLogging();
      testState = false;
      finishLogging = false;
    }

    vTaskDelay(500);
  }
}

void enableWifi(void * arg) {
  // try to connect to wifi
  WiFi.begin("TP-Link_TNK", "wluxe1907");
  while (WiFi.status() != WL_CONNECTED) {
    vTaskDelay(1000);
  }

  // config time logging with NTP server
  configTime(28800, 0, "pool.ntp.org");  // 60x60x8=28800, +8h for Hong Kong

  while (!testState) {  // when user inputting
    // get time
    struct tm timeinfo;
    while (!getLocalTime(&timeinfo)) {
      Serial.println("Fail to obtain time");
      vTaskDelay(UINT_MAX); // stop here if fail to get the time
    }
    strftime(dateTime, 64, "%d %b, %y %H:%M:%S", &timeinfo);
    vTaskDelay(1000);
  }

  //disconnect WiFi
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);

  vTaskDelete(NULL);  // delete itself
}