#ifndef B14E07C5_5834_48DE_94E2_DBA019FA913A
#define B14E07C5_5834_48DE_94E2_DBA019FA913A

#include <LittleFS.h>
#include "FS.h"
#include <ESPAsyncWebServer.h>

/*----------function that example provided (decrecated)----------*/

void listDir(fs::FS &fs, const char * dirname, uint8_t levels);
void readFile(fs::FS &fs, const char * path);
void writeFile(fs::FS &fs, const char * path, const char * message);
void writeFile2(fs::FS &fs, const char * path, const char * message);
void appendFile(fs::FS &fs, const char * path, const char * message);

/*-------------------function that self added-------------------*/

void newFileInit();
void lastFileInit();
void logData(uint64_t cycleTtime);
void logPauseData(uint64_t time = 0);
void quickLog();
void endLogging();
void downLogFile();
bool readResumeData();
void readResumeData2();
void saveResumeData();

void storeLogData(char * str, bool lastData = false);

/*------------function only use for fixing FS problem------------*/
void deleteRfStar(fs::FS &fs, const char * dirname, uint8_t levels=3);

#endif /* B14E07C5_5834_48DE_94E2_DBA019FA913A */
