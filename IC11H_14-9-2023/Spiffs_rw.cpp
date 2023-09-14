#include "spiffs_rw.h"
#include <SPIFFS.h> 

void listDir(fs::FS &fs, const char * dirname, uint8_t levels) {
  Serial.printf("Listing directory: %s\r\n", dirname);

  File root = fs.open(dirname);
  if (!root) {
    Serial.println("- failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    Serial.println(" - not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if (levels) {
        listDir(fs, file.path(), levels - 1);
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

void readFile(fs::FS &fs, const char * path) {
  //Serial.printf("Reading file: %s\r\n", path);

  File file = fs.open(path);
  if (!file || file.isDirectory()) {
    Serial.println("- failed to open file for reading");
    return;
  }

  //Serial.println("- read from file:");
  while (file.available()) {
    Serial.write(file.read());
  }
  file.close();
}

void writeFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Writing file: %s\r\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("- failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("- file written");
  } else {
    Serial.println("- write failed");
  }
  file.close();
}

void appendFile(fs::FS &fs, const char * path, const char * message) {
  File file = fs.open(path, "r");
  if (!file) {
    Serial.println("- failed to open file for reading");
    return;
  }

  String fileContent = "";
  while (file.available()) {
    fileContent += file.readStringUntil('\n') + "\n";
  }
  file.close();

  if (fileContent.indexOf(message) == -1) {
    file = fs.open(path, FILE_APPEND);
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
}


void renameFile(fs::FS &fs, const char * path1, const char * path2) {
  Serial.printf("Renaming file %s to %s\r\n", path1, path2);
  if (fs.rename(path1, path2)) {
    Serial.println("- file renamed");
  } else {
    Serial.println("- rename failed");
  }
}

void deleteFile(fs::FS &fs, const char * path) {
  Serial.printf("Deleting file: %s\r\n", path);
  if (fs.remove(path)) {
    Serial.println("- file deleted");
  } else {
    Serial.println("- delete failed");
  }
}

void deleteLine(const char *path, int lineNumber) {
  File file = SPIFFS.open(path, "r");
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }

  String fileContent = "";
  int currentLine = 1;
  while (file.available()) {
    String line = file.readStringUntil('\n');
    if (currentLine != lineNumber) {
      fileContent += line + "\n";
    }
    currentLine++;
  }
  file.close();

  file = SPIFFS.open(path, "w");
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }

  file.print(fileContent);
  file.close();

  //Serial.println("Line deleted");
}

void deleteEmptyLines(const char *path) {
  File file = SPIFFS.open(path, "r");
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }

  String fileContent = "";
  while (file.available()) {
    String line = file.readStringUntil('\n');
    line.trim(); 
    if (!line.isEmpty()) {
      fileContent += line + "\n";
    }
  }
  file.close();

  file = SPIFFS.open(path, "w");
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }

  file.print(fileContent);
  file.close();
}

void appendToLine(fs::FS &fs, const char *path, size_t lineNumber, const char *data, const char *alarmNameToCheck) {
  File file = fs.open(path, "r+");
  if (!file) {
    Serial.println("Failed to open file for appending");
    return;
  }

  for (size_t i = 1; i < lineNumber; i++) {
    file.readStringUntil('\n');
  }

  long currentPosition = file.position();
  file.readStringUntil('\n');
  file.seek(currentPosition);

  String text = String(data);
  int x = countLines("/data.txt");
  if (x == 1) {
    text = text + "\n";
  } else {
    text = "\n" + text;
  }
  file.print(text);

  file.close();
  //Serial.println("Data appended to line");
}


void appendToLine_MC(fs::FS &fs, const char *path, size_t lineNumber, const char *data, const char *mc_status) {
  File file = fs.open(path, "r+");
  if (!file) {
    Serial.println("Failed to open file for appending");
    return;
  }

  for (size_t i = 1; i < (lineNumber - 1); i++) {
    file.readStringUntil('\n');
  }

  String line2 = file.readStringUntil('\n');
  String lastMessage;
  int commaIndex = line2.indexOf(',');
  if (commaIndex != -1 && commaIndex + 1 < line2.length()) {
    lastMessage = line2.substring(commaIndex + 1);
  }
  //Serial.println(lastMessage);
  //delay(5000);
  if (String(mc_status) != lastMessage)
  {
    long currentPosition = file.position();
    file.readStringUntil('\n');
    file.seek(currentPosition);
    String text = String(data);
    int x = countLines("/mc_status.txt");
    if (x == 1) {
      text = text + "\n";
    } else {
      text = "\n" + text;
    }
    file.print(text);
  }
  file.close();
}

String readLine(fs::FS &fs, const char *path, size_t lineNumber) {
  String result;

  File file = fs.open(path, "r");
  if (!file) {
    Serial.println("Failed to open file for reading");
    return "";
  }

  for (size_t i = 1; i < lineNumber; i++) {
    file.readStringUntil('\n');
  }

  String line = file.readStringUntil('\n');
  file.close();

  size_t dotIndex = line.indexOf('.');
  if (dotIndex != -1) {
    result = line.substring(dotIndex + 1); // ตัดข้อมูลที่อยู่หลังจุด (.)
  }

  return result;
}

String formatTime(uint16_t data0, uint16_t data1, uint16_t data2, uint16_t data3, uint16_t data4, uint16_t data5) {
  String formatted_time = String(data0) + "-" + (data1 < 10 ? "0" : "") + String(data1) + "-" + (data2 < 10 ? "0" : "") + String(data2) + " ";

  String hour = (data3 < 10) ? "0" + String(data3) : String(data3);
  String minute = (data4 < 10) ? "0" + String(data4) : String(data4);
  String second = (data5 < 10) ? "0" + String(data5) : String(data5);

  formatted_time += hour + ":" + minute + ":" + second;

  return formatted_time;
}

int countLines(const char *path) {
  File file = SPIFFS.open(path, "r");
  if (!file) {
    Serial.println("Failed to open file for reading");
    return 0;
  }

  int lineCount = 0;
  while (file.available()) {
    char c = file.read();
    if (c == '\n') {
      lineCount++;
    }
  }
  file.close();

  return lineCount + 1; //
}

void insertDataAfterPrefix(const char *path, const char *prefix, const char *dataToInsert) {
  File file = SPIFFS.open(path, "r");
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }

  String fileContent = "";
  while (file.available()) {
    String line = file.readStringUntil('\n');
    if (line.startsWith(prefix)) {
      int commaIndex = line.indexOf(',');
      if (commaIndex != -1) {
        // Check if there is a space after the comma
        if ( line.endsWith("*"))
        {
          Serial.print("");
        }
        else if (commaIndex + 1 < line.length() && line.charAt(commaIndex + 1) == ' ' ) {
          line = line.substring(0, commaIndex + 1) + String(dataToInsert);
        } else {
          line += String(dataToInsert);
        }
      }
    }
    fileContent += line + "\n";
  }
  file.close();

  file = SPIFFS.open(path, "w");
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }

  file.print(fileContent);
  file.close();
  //Serial.println("Data inserted after prefix '" + String(prefix) + "'");
}

void loadAlarmStatesFromFile(const char* filename, bool* alarmStates, int alarm_coil) 
{
  File file = SPIFFS.open(filename, "r"); 

  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }

  String fileData = file.readStringUntil('\n');
  file.close(); 

  int index = 0;
  char* token = strtok((char*)fileData.c_str(), ",");
  while (token != NULL && index < alarm_coil) {
    alarmStates[index] = atoi(token);
    token = strtok(NULL, ",");
    index++;
  }
}

void saveAlarmStatesToFile(const char* filename, bool* alarmStates, int alarm_coil) 
{
  File file = SPIFFS.open(filename, "w"); 

  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }

  for (int i = 0; i < alarm_coil; i++) {
    file.print(alarmStates[i]); 

    if (i < alarm_coil - 1) {
      file.print(","); 
    }
  }

  file.println(); 
  file.close(); 
}

void removeLinesNotOnDate(const char *filename, const char *dateToKeep) {
  // Initialize SPIFFS
  if (!SPIFFS.begin()) {
    Serial.println("SPIFFS initialization failed!");
    return;
  }

  // Open the file for reading
  File file = SPIFFS.open(filename, "r");
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }

  // Create a temporary buffer for the file content
  String fileContent = "";

  // Read the first line and store it
  String firstLine = file.readStringUntil('\n');
  fileContent += firstLine + '\n';

  // Read the original file line by line, starting from the second line
  while (file.available()) {
    String line = file.readStringUntil('\n');
    if (line.indexOf(dateToKeep) != -1) {
      // Append lines that contain the specified date to the fileContent
      fileContent += line + '\n';
    }
  }

  // Close the file
  file.close();

  // Open the file for writing
  file = SPIFFS.open(filename, "w");
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }

  // Write the filtered content back to the file
  file.print(fileContent);

  // Close the file
  file.close();
}

void moveDataFromFile(const char* sourceFilename, const char* destinationFilename) 
{
  File sourceFile = SPIFFS.open(sourceFilename, "r"); 
  File destinationFile = SPIFFS.open(destinationFilename, "w"); 

  if (!sourceFile || !destinationFile) {
    Serial.println("Failed to open files");
    return;
  }

  String line1 = sourceFile.readStringUntil('\n');
  destinationFile.println(line1);

  while (sourceFile.available()) {
    String line = sourceFile.readStringUntil('\n');
    if (line.endsWith("*")) {

      destinationFile.println(line);
    }
  }

  sourceFile.close();
  destinationFile.close();
}
