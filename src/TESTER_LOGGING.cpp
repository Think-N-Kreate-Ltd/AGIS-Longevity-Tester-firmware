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
  char firstLine[64]; // it is seperate into 3 times to write
  sprintf(filename, "/%08d.csv", sampleId);
  sprintf(firstLine, "sample id:, %d, start time:, ", sampleId);

  vTaskDelay(50);
  writeFile2(LittleFS, filename, firstLine);
  appendFile(LittleFS, filename, dateTime);
  appendFile(LittleFS, filename, "\nTime:, State, Cycle Time, Current:\n");
}

// do whenever the limited SW is touched
// cycleTime = time for one cycle, can enter 0 when do not need to log
void logData(uint64_t cycleTtime) {
  char data[120];  // the data that should log to file
  // cal first to reduce the length
  uint16_t hour = motorRunTime/3600;
  uint8_t min = motorRunTime%3600/60;
  uint8_t sec = motorRunTime%60;
  char MS[5];
  if (motorState) {
    strcpy(MS, "down");  // reversed as we log data after state changed
  } else {
    strcpy(MS, "up");    // reversed as we log data after state changed
  }
  if (cycleTtime == 0) {
    sprintf(data, "%03d:%02d:%02d, P%d %s, N/A, %5.2f\n", hour, min, sec, cycleState, MS, avgCurrent_mA);
  } else {
    uint8_t CT1 = cycleTtime/1000;
    uint16_t CT2 = cycleTtime%1000;
    Serial.println(numCycle);
    sprintf(data, "%03d:%02d:%02d, P%d %s, %02d.%03d", hour, min, sec, cycleState, MS, CT1, CT2);
    char data2[127];  // too long, seperate it
    sprintf(data2, "(%d), %5.2f\n", numCycle, avgCurrent_mA);
    strcat(data, data2);
  }

  appendFile(LittleFS, filename, data);
}

// log the last line, which tells the time and finish
void endLogging() {
  appendFile(LittleFS, filename, "test and homing finish\n");
  char data[50];
  strcpy(data, "failure reason: ");
  if (failReason == failReason_t::CURRENT_EXCEED) {
    strcat(data, "Touch stall current");
  }
  if (failReason == failReason_t::TIME_OUT) {
    strcat(data, "Time out");
  }
  if (failReason == failReason_t::PRESS_KEY) {
    strcat(data, "Key `*` pressed");
  }
  appendFile(LittleFS, filename, data);
  Serial.println("data logging finished");
}

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
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