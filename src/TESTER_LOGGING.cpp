#include <TESTER_LOGGING.h>
#include "AsyncElegantOTA.h"  // define after <ESPAsyncWebServer.h>
#include <Tester_common.h>

// create AsyncWebServer object on port 80
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
char filename[16];

void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
  Serial.printf("Listing directory: %s\r\n", dirname);

  File root = fs.open(dirname);
  if(!root){
    Serial.println("- failed to open directory");
    return;
  }
  if(!root.isDirectory()){
    Serial.println(" - not a directory");
    return;
  }

  File file = root.openNextFile();
  while(file){
    if(file.isDirectory()){
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if(levels){
        listDir(fs, file.path(), levels -1);
      }
    } else {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("\tSIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}

void readFile(fs::FS &fs, const char * path){
  Serial.printf("Reading file: %s\r\n", path);

  File file = fs.open(path);
  if(!file || file.isDirectory()){
    Serial.println("- failed to open file for reading");
    return;
  }

  Serial.println("- read from file:");
  while(file.available()){
    Serial.write(file.read());
  }
  file.close();
}

void writeFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Writing file: %s\r\n", path);

  File file = fs.open(path, "w");
  if(!file){
    Serial.println("- failed to open file for writing");
    return;
  }
  if(file.print(message)){
    Serial.println("- file written");
  } else {
    Serial.println("- write failed");
  }
  file.close();
}

void writeFile2(fs::FS &fs, const char * path, const char * message){
  if (!fs.exists(path)) {
		if (strchr(path, '/')) {  // check the correctness of the path
      Serial.printf("Create missing folders of: %s\r\n", path);
			char *pathStr = strdup(path);
			if (pathStr) {
				char *ptr = strchr(pathStr, '/');
				while (ptr) {
					*ptr = 0;
					fs.mkdir(pathStr);
					*ptr = '/';
					ptr = strchr(ptr+1, '/');
				}
			}
			free(pathStr);
		}
  }

  Serial.printf("Writing file to: %s\r\n", path);
  File file = fs.open(path, "w");
  if(!file){
    Serial.println("- failed to open file for writing");
    return;
  }
  if(file.print(message)){
    Serial.println("- file written");
  } else {
    Serial.println("- write failed");
  }
  file.close();
}

void appendFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Appending to file: %s\r\n", path);

  File file = fs.open(path, "a");
  if (!file) {
    Serial.println("- failed to open file for appending");
    return;
  }
  if (file.print(message)) {
    Serial.println("- message appended");
  } else {
    Serial.println("- append failed");
  }
  file.close();
}

// create new file and write the info+title
void newFileInit() {
  char firstLine[120]; // it is seperate into 3 to reduce the size, in fact, the final size is 90
  sprintf(filename, "/%08d.csv", sampleId);
  sprintf(firstLine, "sample id:, %08d, start time:, ", sampleId);
  strcat(firstLine, dateTime);
  strcat(firstLine, "\nTime:, State, Cycle Time, Current:\n");

  writeFile2(LittleFS, filename, firstLine);
  ESP_LOGI("Logging", "new file init");
}

void lastFileInit() {
  char firstLine[100];
  sprintf(firstLine, "%s: cut off power and resume\n", dateTime);
  appendFile(LittleFS, filename, firstLine);
}

// do whenever the limited SW is touched
// cycleTime = time for one cycle, can enter 0 when do not need to log
void logData(uint64_t cycleTtime) {
  char data[120];  // the data that should log to file
  // cal first to reduce the length
  uint16_t hour = status.motorRunTime/3600;
  uint8_t min = status.motorRunTime%3600/60;
  uint8_t sec = status.motorRunTime%60;
  char MS[5];
  if (status.motorState) {
    strcpy(MS, "down");  // reversed as we log data after state changed
  } else {
    strcpy(MS, "up");    // reversed as we log data after state changed
  }
  if (cycleTtime == 0) {
    sprintf(data, "%03d:%02d:%02d, P%d %s, N/A, %5.2f\n", hour, min, sec, status.cycleState, MS, avgCurrent_mA);
  } else {
    uint8_t CT1 = cycleTtime/1000;
    uint16_t CT2 = cycleTtime%1000;
    Serial.println(status.numCycle);
    sprintf(data, "%03d:%02d:%02d, P%d %s, %02d.%03d", hour, min, sec, status.cycleState, MS, CT1, CT2);
    char data2[127];  // too long, seperate it
    sprintf(data2, "(%d), %5.2f\n", status.numCycle, avgCurrent_mA);
    strcat(data, data2);
  }

  storeLogData(data);
}

// log the pause time 
void logPauseData(uint64_t time) {
  char data[80];  // the data that should log to file
  if (status.pauseState) {
    // cal first to reduce the length
    uint16_t hour = status.motorRunTime/3600;
    uint8_t min = status.motorRunTime%3600/60;
    uint8_t sec = status.motorRunTime%60;
    sprintf(data, "%03d:%02d:%02d, paused\n", hour, min, sec);
  } else {
    // cal first to reduce the length
    uint16_t hour = time/3600;
    uint8_t min = time%3600/60;
    uint8_t sec = time%60;
    sprintf(data, "N/A, resume, pause time:%03d:%02d:%02d\n", hour, min, sec);
  }
  storeLogData(data);
}

// TODO: quick log the status when power failure occur
void quickLog() {
  File file = LittleFS.open("/data2.txt", "w");
  if (!file) {
    Serial.println("fail to open file");
    return;
  } else {
    file.write((const uint8_t*)&status, sizeof(status));
  }
  file.close();
}

// log the last line, which tells the time and finish
void endLogging() {
  char data[80] = "test and homing finish\n";
  strcat(data, "failure reason: ");
  if (failReason == failReason_t::CURRENT_EXCEED) {
    strcat(data, "Touch stall current");
  }
  if (failReason == failReason_t::TIME_OUT) {
    strcat(data, "Time out");
  }
  if (failReason == failReason_t::PRESS_KEY) {
    strcat(data, "Key `*` pressed");
  }
  storeLogData(data, true);
  ESP_LOGI("Logging", "data logging finished");
}

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

// store the data that should log, until it exceed a large number
// then log the data and reset it
// directly do logging if this is the last data
void storeLogData(char * str, bool lastData) {
  static char data[4000];  // string the store the data that should be logged (less than 4096)
  // size_t length = sizeof(str)/sizeof(char);  // for unknown reason, sizeof is hard to measure char *, or there is some misunderstanding for me
  static uint16_t length = 0;
  length +=strlen(str);

  strcat(data, str);

  if (length >= 3900) {
    // log the stored data
    appendFile(LittleFS, filename, data);
    // Serial.println(data);
    Serial.printf("length of data is %d\n", length);

    // reset the array
    for (int j=3900; j<length; ++j) {
      data[j] = 0;
    }
    length = 0;
    strcpy(data, "");
  } else if (lastData) {
    data[length] = 0;
    appendFile(LittleFS, filename, data);
    Serial.printf("length of data is %d\n", length);
  }
}

// to download log file in webpage
void downLogFile() {
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(LittleFS, filename, "text/plain", true);  // force download the file
  });

  server.onNotFound(notFound); // if 404 not found, go to 404 not found
  AsyncElegantOTA.begin(&server); // for OTA update
  server.begin();
}

// read the file for getting info of the last test
// `,` is used to trigger the reading and storing
// return true if need to resume, false if restart <- for homing
bool readResumeData() {
  bool resume = true; // var for return
  Serial.printf("Reading file: %s\r\n", "/data1.txt");

  File file = LittleFS.open("/data1.txt");
  if(!file || file.isDirectory()){
    Serial.println("- failed to open file for reading");
    resume = false;
  }

  if (!file.available()) {
    resume = false;
    ESP_LOGW("Resume reading", "fail to open file, or empty file, restart test");
  } else {
    char c = file.read(); // read the last state
    if (c == '0') { 
      resume = false;
      ESP_LOGI("Resume reading", "stopped last time, restart test");
    } else {  
      readFile(LittleFS, "/data1.txt");
      ESP_LOGI("Resume reading", "paused last time, resume test, wait a while");
      c = file.read();  // there should be a comma then, pass it

      // get the file name
      for (uint8_t j=0; j<13; ++j) {
        filename[j] = file.read();
      }
      Serial.println(filename);
      c = file.read();  // there should be a comma then, pass it

      // get user input data done in last time
      int16_t motorData[11]; // array to store all motor data
      char MD[5]; // string to store the read-ed motor data
      uint8_t count = 0;
      uint8_t i = 0;
      while (file.available() && (count<11)) {
        c = file.read();
        if (c == 44) {  // read the comma
          *(motorData + count) = atoi(MD);
          count++;
          i = 0;
          Serial.printf("%s, ", MD);
          // reset DF to NULL to store the next reading
          for (uint8_t j=0; j<5; ++j) {
            MD[j] = 0;
          }
        } else {  
          MD[i] = c;
          ++i;
        }
      }
      // save the motor data
      PWM_P1UP = motorData[0];
      PWM_P1DOWN = motorData[1];
      PWM_P2UP = motorData[2];
      PWM_P2DOWN = motorData[3];
      numTime_P1 = motorData[4];
      numTime_P2 = motorData[5];
      T_OUT_P1UP = motorData[6];
      T_OUT_P1DOWN = motorData[7];
      T_OUT_P2UP = motorData[8];
      T_OUT_P2DOWN = motorData[9];
      T_P2running = motorData[10];

      // check the data
      for (uint8_t j=0; j<11; ++j) {
        Serial.printf("\nmotorData[%d] = %d", j, motorData[j]);
      }

      // also get the data of motor status
      readResumeData2();
    }
  }

  file.close();
  return resume;
}

// read the file and decode to get the last status of motor
void readResumeData2() {
  Serial.println("Reading file: /data2.txt");

  File file = LittleFS.open("/data2.txt", "r");
  if (!file) {
    Serial.println("fail to open file");
    return;
  } else {
    file.read((uint8_t*)&status, sizeof(status));
    resumeStartTime = status.motorRunTime;  // udpate the motor run time
    Serial.printf("readings are %d, %d, %d, %d, %d, %d\n", status.motorState, status.cycleState, 
                  status.testState, status.pauseState, status.motorRunTime, status.numCycle);
  }
  file.close();
}

void saveResumeData() {
  char data[80];  // the data that should log to file
  char data2[80];
  char data3[80];
  sprintf(data, "1,%s,%d,%d,", filename, PWM_P1UP, PWM_P1DOWN);
  sprintf(data2, "%d,%d,%d,%d,", PWM_P2UP, PWM_P2DOWN, numTime_P1, numTime_P2);
  // strcat(data, data2);
  sprintf(data3, "%d,%d,%d,%d,%d,", T_OUT_P1UP, T_OUT_P1DOWN, T_OUT_P2UP, T_OUT_P2DOWN, T_P2running);
  // strcat(data, data3);

  writeFile2(LittleFS, "/data1.txt", data);
  appendFile(LittleFS, "/data1.txt", data2);
  appendFile(LittleFS, "/data1.txt", data3);
  Serial.println(data);
  Serial.println(data2);
  Serial.println(data3);
}

// to delete all files in a dir (not include dir)
// dirname="/" to delete files which is not in a dir
// level=1 to delete all files expect those in sub-dir
void deleteRfStar(fs::FS &fs, const char * dirname, uint8_t levels) {
  Serial.printf("deleting directory: %s\r\n", dirname);

  // check for dir name correctness
  File root = fs.open(dirname);
  if(!root){
    Serial.println("failed to open directory");
    return;
  }
  if(!root.isDirectory()){
    Serial.println("not a directory");
    return;
  }

  // create a char array to store the path
  // the path is for file that should be deleted
  char rm_path[99];

  while (true) {  // keep doing until the directory is empty
    File file = root.openNextFile();
    if (file) {
      if (file.isDirectory()){
        Serial.printf("DIR: %s\n", file.name());
        if (levels) {
          // only go to sub-dir if allowed
          // i.e. default will open 2 sub-dir and delete all file in that
          deleteRfStar(fs, file.path(), levels -1);
        }
      } else {
        // it is a file, print the name and delete it
        Serial.printf("FILE: %s\t", file.name());
        strcpy(rm_path, dirname);
        if (rm_path[strlen(rm_path)-1] != 47) { // 47="/"
          strcat(rm_path, "/");
        }
        strcat(rm_path, file.name());
        // in LittleFS, the file must be closed before we can delete it
        file.close();
        if (fs.remove(rm_path)) {
          Serial.println("Deleted");
        } else {
          Serial.printf("Fail to delete, path=%s\n", rm_path);
        }
      }
    } else {
      // enter here when file directory is empty
      break;
    }
  }
}