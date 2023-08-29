#include <TESTER_LOGGING.h>

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
  char firstLine[64];
  sprintf(filename, "/data/%08d.csv", sampleId);
  sprintf(firstLine, "sample id:, %d, start time:, %s, \nTime:, Cycle Time, Current:\n", sampleId, dateTime);

  writeFile2(LittleFS, filename, firstLine);
}

// do whenever the limited SW is touched
void logData(uint8_t time) {
  char data[64];  // the data that should log to file
  sprintf(data, "%02d %02d:%02d:%02d, N/A", motorRunTime/86400, motorRunTime%86400/3600, 
          motorRunTime%3600/60, motorRunTime%60);
  appendFile(LittleFS, filename, "");
  // TODO: change state when need to finish
}

void endLogging() {
  // TODO: append the last data
  // TODO: close file(?)
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