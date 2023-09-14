#ifndef SPIFFS_RW_H
#define SPIFFS_RW_H

#include <FS.h>

void listDir(fs::FS &fs, const char * dirname, uint8_t levels);
void readFile(fs::FS &fs, const char * path);
void writeFile(fs::FS &fs, const char * path, const char * message);
void appendFile(fs::FS &fs, const char * path, const char * message);
void renameFile(fs::FS &fs, const char * path1, const char * path2);
void deleteFile(fs::FS &fs, const char * path);
void deleteLine(const char *path, int lineNumber);
void deleteEmptyLines(const char *path);
void appendToLine(fs::FS &fs, const char *path, size_t lineNumber, const char *data, const char *alarmNameToCheck);
void appendToLine_MC(fs::FS &fs, const char *path, size_t lineNumber, const char *data, const char *mc_status);
String readLine(fs::FS &fs, const char *path, size_t lineNumber);
String formatTime(uint16_t data0, uint16_t data1, uint16_t data2, uint16_t data3, uint16_t data4, uint16_t data5);
int countLines(const char *path);
void insertDataAfterPrefix(const char *path, const char *prefix, const char *dataToInsert);
void loadAlarmStatesFromFile(const char* filename, bool* alarmStates, int alarm_coil);
void saveAlarmStatesToFile(const char* filename, bool* alarmStates, int alarm_coil);
void removeLinesNotOnDate(const char *filename, const char *dateToKeep);
void moveDataFromFile(const char* sourceFilename, const char* destinationFilename) ;

#endif
