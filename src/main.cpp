#include <Arduino.h>
#include <ezButton.h>
#include <WiFi.h>
#include <SPI.h>
#include <TESTER_INA219.h>
#include <TESTER_LOGGING.h>
#include <Tester_Display.h>
#include <Tester_common.h>

bool print = true;
long TT;

/*-------------------define for pins-------------------*/

#define MOTOR_CTRL_PIN_1 15 // Motorl Control Board PWM 1
#define MOTOR_CTRL_PIN_2 16 // Motorl Control Board PWM 2

/*-------------------ezButton object-------------------*/

ezButton limitSwitch_Up(37);   // create ezButton object that attach to pin 37
ezButton limitSwitch_Down(38); // create ezButton object that attach to pin 38

/*----------------var for control motor----------------*/

volatile bool motorHoming = false;  // will directly go to homing when true
uint64_t recordTime;            // for record the time of motor, mainly for testing
// uint64_t startTime = millis();  // for record the starting time of the test 
bool resumeAfterCutOff = false; // for finding if last time stop by cut off power, also change to false after get all resume data
// uint64_t resumeStartTime = 0;   // for storing the time of motor run time for last test (cut off -ed)
uint64_t powerFailRecTime = 0;  // for record how long did power fail occur

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

uint8_t T_P2running = 1;    // running time of motor on up in pattern 2

uint64_t sampleId = 12345678;  // the sample ID, should be 8 numbers
char dateTime[64];          // string to store the start date and time
bool loadProfile = true;    // the option of load profile, true=default, false=predefine
bool downloadFile = false;  // when user click btn to down file, it will become true and start download

/*-------------------var for current-------------------*/

float current_mA;       // the current at a specific time, unit=mA
float avgCurrent_mA;    // the average current in pass second, unit=mA
bool powerFail = false; // the condition of power supply, true when disconnected

/*-------------------var for display-------------------*/

MotorStatus status = {true, 0, false, false, 0, 0};
failReason_t failReason = failReason_t::NOT_YET;

/*------------------function protypes------------------*/

void stopTest();
void pauseAll(uint8_t i=0);
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
void tftDisplay(void * arg);

/*------------------------Timers------------------------*/

// create pointer for timer
hw_timer_t *Timer0_cfg = NULL; // create a pointer for timer0

void IRAM_ATTR timeCount() {
  if (!status.pauseState && status.testState) {
    static uint8_t count = 0;
    if (count == 1) {
      status.motorRunTime++;
      count = 0;
    } else {
      count++;
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(TFT_CS, OUTPUT);

  // set up LittleFS
  // place here because FS must be mount first
  if (!LittleFS.begin(true)) {
    ESP_LOGE("FS", "LittleFS Mount Failed");
    return;
  } else {
    listDir(LittleFS, "/", 2);
  }

  // setup for timer0
  Timer0_cfg = timerBegin(0, 8000, true);   // prescaler = 8000
  timerAttachInterrupt(Timer0_cfg, &timeCount,
                       false);             // call the function motorcontrol()
  timerAlarmWrite(Timer0_cfg, 5000, true); // time = 8000*5000/80,000,000 = 500ms
  timerAlarmEnable(Timer0_cfg);            // start the interrupt

  /*Create a task for running motor up and down continuously */
  xTaskCreate(motorCycle,
              "Running the Motor Cycle",
              4096,
              NULL,
              12,  // highest priority task
              NULL);
  
  // *Create a task for homing
  xTaskCreate(homingRollerClamp,      // function that should be called
              "Homing roller clamp",  // name of the task (debug use)
              4096,           // stack size
              NULL,           // parameter to pass
              13,             // task priority, 0-24, 24 highest priority
              NULL);          // task handle

  // I2C is too slow that cannot use interrupt
  xTaskCreate(getI2CData,     // function that should be called
              "Get I2C Data", // name of the task (debug use)
              4096,           // stack size
              NULL,           // parameter to pass
              1,              // task priority, 0-24, 24 highest priority
              NULL);          // task handle

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
  // Create a task for TFT display
  xTaskCreate(tftDisplay,       // function that should be called
              "TFT display",    // name of the task (debug use)
              4096,             // stack size
              NULL,             // parameter to pass
              3,                // task priority, 0-24, 24 highest priority
              NULL);            // task handle
}

void loop() {
  // if (print) {
  //   // Serial.printf("average current: %f\n", avgCurrent_mA);
  //   Serial.printf("resumeAfterCutOff: %d, ", resumeAfterCutOff);
  //   Serial.printf("testState: %d, ", status.testState);
  //   Serial.printf("date time : %s\n", dateTime);
  //   print = false;
  // }
}

/*------------------function protypes------------------*/

void stopTest() {
  // write to file to tell that next time should not resume
  writeFile(LittleFS, "/data1.txt", "0");
  writeFile(LittleFS, "/data2.txt", "0");

  // do homing
  motorHoming = true;
  while (motorHoming) {
    vTaskDelay(200);
  }
  ESP_LOGI("Homing", "homing completed, all stopped");

  // all finish after homing
  recordTime = millis();
  status.testState = false;
  status.cycleState = 0;
  downloadFile = true;  // TODO: remove it after we can use keypad input
  Serial.println(recordTime);
  vTaskDelay(UINT_MAX); 
}

// pause the test
// i is for P2move up, get the time
// do not want to use sleep as it is harmful to the program
// can try to use delay() in main loop but not recommened
void pauseAll(uint8_t i) {
  if (status.pauseState) {
    uint64_t recTime = millis();
    motorOn(0);
    logPauseData();
    while (status.pauseState) {
      vTaskDelay(200);
    }

    if ((millis()-recTime) >= 1000) { // not double press, resume the test
      // when resume, record the time again
      // startTime += (millis() - recTime);
      
      // should reach timeout. i.e., not to finish this loop before touch LS -> to bypass timeout check
      if (status.motorState) {
        if (status.cycleState == 1) {
          motorOn(PWM_P1UP);
          while (limitSwitch_Up.getStateRaw() == 1) {
            vTaskDelay(20);
          }
        } else if (status.cycleState == 2) {
          motorOn(PWM_P2UP);
          uint16_t timeUsed = i*20;
          vTaskDelay(T_P2running*1000 + 50 - timeUsed); // +50 for preventing error
        }
        
      } else {
        if (status.cycleState == 1) {
          motorOn(PWM_P1DOWN);
        } else if (status.cycleState == 2) {
          motorOn(PWM_P2DOWN);
        }
        while (limitSwitch_Down.getStateRaw() == 1) {
          vTaskDelay(20);
        }
      }
      logPauseData((millis()-recTime)/1000);
    } else {
      // change the fail reason if needed and stop it
      if (failReason == failReason_t::NOT_YET) {
        failReason = failReason_t::PRESS_KEY;
      }
      stopTest(); // to prevent any unexpected non-stop, i.e. place here
    }
  }
}

// when timeout, stop the test
// timeout = timeout set bt users
// time = the last record time
void timeoutCheck(uint8_t timeout, uint32_t time) {
  if ((millis()-time) >= (timeout*1000)) {
    if (failReason == failReason_t::NOT_YET) {
      failReason = failReason_t::TIME_OUT;
    }
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
// time is the number of time that motor On/Down should run, default is 3
void motorP1(uint8_t time) {
  static uint32_t recTime;
  if (status.motorState) {
    motorOn(PWM_P1UP);
    recTime = millis();
    do {
      vTaskDelay(20);
      timeoutCheck(T_OUT_P1UP, recTime-powerFailRecTime);
      pauseAll();
    } while (limitSwitch_Up.getStateRaw() == 1);
    status.motorState = false;  // motor move down
  } else {
    motorOn(PWM_P1DOWN);
    recTime = millis();
    do {
      vTaskDelay(20);
      timeoutCheck(T_OUT_P1DOWN, recTime-powerFailRecTime);
      pauseAll();
    } while (limitSwitch_Down.getStateRaw() == 1);
    status.motorState = true;
    motorOn(0); // for safety, write the motor to stop, will overwrite soon
  }
  logData(0);
  powerFailRecTime = 0;

  if (time >= 1) { // run the next time
    Serial.printf("pattern 1 finish half, %d times remains\n", time);
    status.passedNum++;
    motorP1(time-1);
  }
}

// run the motor for pattern 2
// time is the number of time that motor On/Down should run, default is 7
void motorP2(uint8_t time) {
  static uint32_t recTime;
  if (status.motorState) {
    motorOn(PWM_P2UP);
    recTime = millis();
    vTaskDelay(20);
    uint32_t count = T_P2running*50;
      for (uint8_t i=0; i<count; ++i) { // total delay for 20*50=1000ms
        if ((limitSwitch_Up.getStateRaw() == 1) && (millis()-recTime<=(T_P2running*1000))) {
          vTaskDelay(20);
          timeoutCheck(T_OUT_P2UP, recTime-powerFailRecTime);
          pauseAll(i);
        } else {
          i=count; // if touch limit SW, directory go to next state
        }
      }
    status.motorState = false;  // motor move down
  } else {
    motorOn(PWM_P2DOWN);
    recTime = millis();
    do {
      vTaskDelay(20);
      timeoutCheck(T_OUT_P2DOWN, recTime-powerFailRecTime);
      pauseAll();
    } while (limitSwitch_Down.getStateRaw() == 1);
    status.motorState = true;
    motorOn(0); // for safety, write the motor to stop, will overwrite soon
  }
  logData(0);
  powerFailRecTime = 0;

  if (time >= 1) { // run the next time
    Serial.printf("pattern 2 finish half, %d times remains\n", time);
    status.passedNum++;
    motorP2(time-1);
  }
}

/*--------------------task functions--------------------*/

void motorCycle(void * arg) {
  // check if test resume
  resumeAfterCutOff = readResumeData();
  if (!resumeAfterCutOff) {
    motorHoming = true;
    while (motorHoming) {
      vTaskDelay(20);
    }
    ESP_LOGI("Homing", "homing completed");

    while (!status.testState) {  // wait until user start the test (and homing completed)
      vTaskDelay(500);
    }
    Serial.println("Start test");
    vTaskDelay(1000); // delay for 1 sec for logging init
    // startTime = millis();
  } else {  // resume the test
    while (!strchr(dateTime, ':')) {  // wait until get time
      vTaskDelay(100);
    }
    lastFileInit();
    // status.testState = true;
    uint64_t recTime = millis();
    if (status.cycleState == 1) {
      motorP1(numTime_P1 - status.passedNum);
      status.cycleState++;
      motorP2(numTime_P2);
    } else {
      motorP2(numTime_P2 - status.passedNum);
    }
    logData(millis()-recTime);
  }

  for (;;) {
    status.numCycle++;
    status.passedNum = 0;
    static uint64_t recTime = millis();
    status.cycleState = 1;
    motorP1(numTime_P1);
    status.cycleState++;
    status.passedNum = 0;
    motorP2(numTime_P2);
    logData(millis()-recTime);
    recTime = millis();
  }
}

void homingRollerClamp(void * arg) {
  for(;;) {
    while (!motorHoming) {
      vTaskDelay(200);
      print = true;
    }

    if (limitSwitch_Down.getStateRaw() == 0) {  // touched
      motorHoming = false;
      motorOn(0);
    } else {
      // motor move down
      motorOn(-255);
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
      if (failReason == failReason_t::NOT_YET) {
        failReason = failReason_t::CURRENT_EXCEED;
      }
      stopTest();
      ESP_LOGI("Stop test", "fail reason: current:%5.2f", avgCurrent_mA);
    }
  }
}

void loggingData(void * parameter) {
  static bool finishLogging = false;

  for (;;) {
    if (status.testState) {
      if(!resumeAfterCutOff) {
        newFileInit();  // create new file and header
        ESP_LOGI("Logging", "Logging initialized");

        // save the user input and test info
        saveResumeData();
      }

      // to prevent from checking power before connecting power
      // i.e. for testing currently: connect to microUSB -> cut off power -> reboot ESP -> turn on power
      // it should be useless after we can do from hardware
      // but better keep this 5s delay as it have no negative impact but may prevent unexpected condition
      vTaskDelay(5000);
      
      // after create file, wait for finish
      // data logging will be done when in needed
      while (status.testState)  {
        // logData();
        if (powerFail) {
          Serial.println("power fail occur");

          // pause it to stop the motor run time
          status.pauseState = true;

          // log all data before memory lost
          quickLog();
          storeLogData("", true); // write the buffer to file. Warning is not important, or add a NULL char * to remove it
          
          // prevent from calling it second time
          // pauseAll();  // should not call pauseAll, but should work similar
          if (status.pauseState) {
            powerFailRecTime = 0;
            // motorOn(0);  // s.t. we can not to output to motor to move it if resume soon
            logPauseData();
            while (status.pauseState) {
              vTaskDelay(50);
              powerFailRecTime += 50;
              Serial.println(powerFailRecTime);
              if (!powerFail) {
                // when power connect back, resume everything
                status.pauseState = false;
                logPauseData((millis()-powerFailRecTime)/1000);
              }
            }
          }
          // vTaskDelay(20000);  // assume the back up power will used up within 20s
        }

        vTaskDelay(500);  // put this after checking power to prevent triggered by stopping
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
      status.testState = false;
      finishLogging = false;
    }

    vTaskDelay(500);

    // force download file
    if (downloadFile) {
      // connect to wifi
      WiFi.begin("TP-Link_TNK", "wluxe1907");
      while (WiFi.status() != WL_CONNECTED) {
        vTaskDelay(1000);
      }

      downLogFile();
      Serial.print("you can download the file now, IP Address: ");
      Serial.println(WiFi.localIP().toString());

      while (!status.testState) {  // the user should download before next test start
        vTaskDelay(500);
      }
      //disconnect WiFi
      ESP_LOGI("WiFi", "WiFi disconnected");
      WiFi.disconnect(true);
      WiFi.mode(WIFI_OFF);

      downloadFile = false;
    }
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

  while (!status.testState || resumeAfterCutOff) {  // when user inputting, or resume
    // get time
    struct tm timeinfo;
    while (!getLocalTime(&timeinfo)) {
      ESP_LOGE("NTP time", "Fail to obtain time");
      vTaskDelay(UINT_MAX); // stop here if fail to get the time
    }
    strftime(dateTime, 64, "%d %b, %y %H:%M:%S", &timeinfo);
    vTaskDelay(500);  // to match the tft update interval
    resumeAfterCutOff = false;  // delay first, to prevent unexpected problem
  }

  //disconnect WiFi
  ESP_LOGI("WiFi", "WiFi disconnected");
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);

  vTaskDelete(NULL);  // delete itself
}

void tftDisplay(void * arg) {
  display_init();
  input_screen();
  monitor_screen();

  for(;;) {
    lv_timer_handler(); // Should be call periodically
    vTaskDelay(5);      // The timing is not critical but it should be about 5 milliseconds to keep the system responsive
  }
}