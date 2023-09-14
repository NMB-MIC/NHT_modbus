#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <ArduinoOTA.h>
#include <ArduinoJson.h>
#include "PubSubClient.h"
#include <ESP32_FTPClient.h>
#include "ModbusRtu.h"
#include <SPIFFS.h>
#include <FS.h>
#include "Spiffs_rw.h"
#include <esp_task_wdt.h>
#include <iostream>
#include <string>

#define WDT_TIMEOUT 30
#define led_connection 41
#define led_published 42

#define FORMAT_SPIFFS_IF_FAILED true
Modbus slave(1, Serial1, 0);

/////////////////////////Set Up Network////////////////////////
const char* ssid = "NHT_DX";
const char* password = "nhtmic@admin";

const char* mqtt_server = "192.168.1.107";

IPAddress local_IP(192, 168, 68, 91); // Static IP
IPAddress gateway(192, 168, 68, 1);    // Gateway IP address
IPAddress subnet(255, 255, 255, 0);     // subnet

char Machine_no[] = "IC11B";

char ftp_server[] = "192.168.1.107";
char ftp_user[]   = "admin";
char ftp_pass[]   = "1234";

ESP32_FTPClient ftp (ftp_server, ftp_user, ftp_pass, 5000, 2);

/////////////////////////Set Up Network////////////////////////

WiFiClient espClient;
PubSubClient client(espClient);

uint16_t prevDataState = 0;
int alarmCount = 0;
File file;
String output, output_end;
int lineCount, Year;

//////////////Setup////////////////////////////
const int maxAlarms = 500;  // จำนวนสูงสุดของรายการ alarm list
bool alarmStates[maxAlarms];
String alarmNames[maxAlarms] = {
  "WORN WHEEL",
  "DRESSER ERROR",
  "GRINDER GAUGE ERROR",
  "LOADING ERROR",
  "I.D SMALL",
  "I.D LARGE",
  "GRINDER FULL WORK",
  "GRINDER CHUTE EMPTY",
  "A/F ADJ. YIELD STOP",
  "SORTING FULL WORK COUNTER",
  "SORTING NO WORK",
  "REPEAT COUNTER",
  "TRANSFER LOADER ERROR",
  "NEXT M/C CHUTE FULL",
  "ID SMALL(GE)",
  "GE CRUSH",
  "DPM. ERROR",
  "GE NOT ON",
  "TOTAL TAPER ADJ.LIMIT ERROR",
  "GAUGE ERROR (NO SIGNAL)",
  "OK1 TRAP SHUTTER ERROR",
  "OK2 TRAP SHUTTER ERROR",
  "-NG TRAP SHUTTER ERROR",
  "+NG TRAP SHUTTER ERROR",
  "SORTING NO WORK STOP",
  "GRINDING CYCLE TIME OVER",
  "RESET BY LOADING",
  "GE CHECK MODE (NO USE)",
  "GE NOISE CHECK (NO USE)",
  "GE CHECK MODE (NO USE)",
};

String status_MC[5] = {
  "1",
  "2",
  "3",
  "4",
  "5",
};

int bitIndex;
const int num = 50;   // จำนวน Data ทั้งหมด
const int alarm_coil = 30;
const int mc_coil = 5;
#define mc_run 30
#define mc_stop 31
#define mc_alarm 32
#define mc_noWork 33
#define mc_fullWork 34
#define mc_clear 35

#define yy Data[36]
#define mm Data[37]
#define dd Data[38]
#define hh Data[39]
#define mn Data[40]
#define sec Data[41]
uint16_t Data[num];

int8_t state1 = 0;
unsigned long previousMillis = 0;
unsigned long previousMillis2 = 0;
unsigned long previousMillis3 = 0;
unsigned long previousMillis4 = 0;
const unsigned long interval = 100; //เวลาอ่าน Modbus
const unsigned long interval2 = 5000; //เวลาในการอ่านข้อมูลใน SPIFFS
const unsigned long interval3 = 2000; // Reconnect MQTT Time
const unsigned long interval4 = 60000; // FTP Time
//////////////เพิ่มเติม////////////////////////////
int8_t state = 0;
String  rssi, mc_no, mc_status, actionyear, actionmonth, actiondate, actionhour, actionmin, actionsec , avgct, utl_s1, utl_s2, utl_s3, utl_total, prod_s1, prod_s2, prod_s3, prod_total;
//char r[16], d0[16], d1[16], d2[16], d3[16], d4[16], d5[16], d6[16], d7[16], d8[16], d9[16], d10[16], d11[16], d12[16], d13[16], d14[16], d15[16], d16[16], d17[16], d18[16], d19[16]; // Add register ,d6[16],…….n;

void WiFiEvent(WiFiEvent_t event, WiFiEventInfo_t info)
{
  switch (event) {
    case SYSTEM_EVENT_STA_CONNECTED:
      Serial.println("Connected to WiFi");
      Serial.println(WiFi.localIP());
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      Serial.println("Disconnected from WiFi");
      break;
    default:
      break;
  }
}

void callback(char* topic, byte* payload, unsigned int length)
{
  esp_task_wdt_reset();
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    //digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
  } else {
    //digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }

}

void reconnect()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    esp_task_wdt_reset();
    //Serial.println("Wifi connected");
    if (!client.connected())
    {
      if (client.connect("ESP32S2_Client")) {

      } else {
        Serial.print("Failed, rc=");
        Serial.print(client.state());
        Serial.println(" try again in 5 seconds");
        digitalWrite(led_published, LOW);
        //delay(5000);
      }
    } else {
      //Serial.println("MQTT connected");
    }
    client.loop();

  } else {
    Serial.println("Not connected");
    digitalWrite(led_published, LOW);
  }
}

void setup()
{
  Serial.begin(500000);
  Serial1.begin(115200);
  esp_task_wdt_init(WDT_TIMEOUT, true);
  esp_task_wdt_add(NULL);
  esp_task_wdt_reset();
  pinMode ( led_connection , OUTPUT);
  pinMode ( led_published, OUTPUT);
  delay(1000);
  WiFi.onEvent(WiFiEvent);
  WiFi.config(local_IP, gateway, subnet);
  WiFi.begin(ssid, password);
  ArduinoOTA.setHostname(Machine_no);

  // No authentication by default
  ArduinoOTA.setPassword("1234");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA
  .onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  })
  .onEnd([]() {
    Serial.println("\nEnd");
  })
  .onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  })
  .onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });

  ArduinoOTA.begin();

  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  if (!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)) {  //FORMAT_SPIFFS_IF_FAILED
    Serial.println("SPIFFS Mount Failed");
    return;
  }

  //SPIFFS.format();                   // format file
  //deleteFile(SPIFFS, "/data.txt");   // Delete data.txt      //////////////////////////////////////
  listDir(SPIFFS, "/", 0);
  appendFile(SPIFFS, "/data.txt", "topic,occurred,restored,mark");
  appendFile(SPIFFS, "/mc_status.txt", "occurred,mc_status");
  loadAlarmStatesFromFile("/state.txt", alarmStates, alarm_coil);
  for (int i = 0; i < alarm_coil; i++)
  {
    Serial.print("data["); Serial.print(i); Serial.print("] : "); Serial.println(alarmStates[i]);
  }

  slave.start();

  client.setServer(mqtt_server, 1883); // MQTT broker address and port
  client.setCallback(callback);
}

void loop()
{
  state1 = slave.poll(Data, num);
  digitalWrite(led_published, HIGH);
  digitalWrite(led_connection, HIGH);
  esp_task_wdt_reset();
  unsigned long currentMillis = millis();
  Year = actionyear.toInt();
  if (Year > 0 && currentMillis - previousMillis >= interval && state1 == 7 )
  {
    previousMillis = currentMillis;
    deleteEmptyLines("/data.txt");
    file = SPIFFS.open("/data.txt", "a");


    for (int i = 0; i < alarm_coil; i++)
    {
      int bitValue = (Data[i / 16] >> (i % 16)) & 1;  // อ่านค่า Bit ใน Coil
      //Serial.print("data["); Serial.print(i); Serial.print("] : "); Serial.println(alarmStates[i]);
      if (bitValue == 1 && !alarmStates[i])
      {
        bitIndex = i;
        alarmStates[i] = true;

        output = alarmNames[i] + "," + String(formatTime(yy, mm, dd, hh, mn, sec)) + ",";
        alarmCount++;
        printAlarmList();
      }
      else if (bitValue == 0 && alarmStates[i])
      {
        alarmStates[i] = false;
        if (alarmCount > 0 && i >= 0 && i < maxAlarms)
        {
          bitIndex = i;
          output_end = formatTime(yy, mm, dd, hh, mn, sec);

          insertDataAfterPrefix("/data.txt", alarmNames[bitIndex].c_str(), (output_end + ",*").c_str());
          goto loop1;

        }
      }
    }

loop1: delay(1);
    saveAlarmStatesToFile("/state.txt", alarmStates, alarm_coil);
    for (int i = alarm_coil; i <= (alarm_coil + mc_coil); i++)
    {
      int bitValue = (Data[i / 16] >> (i % 16)) & 1;  // อ่านค่า Bit ใน Coil
      //Serial.print(i);Serial.print(" : ");Serial.println(bitValue);
      if (i == mc_clear && bitValue == 1 )
      {
        SPIFFS.format();
        Serial.println("*****************************");
        ESP.restart();
      }
      if (bitValue == 1 && !alarmStates[i])
      {

        if ( i == mc_run ) {
          alarmStates[i] = true;
          printMachineStatus(status_MC[0]);
        }
        if ( i == mc_stop ) {
          alarmStates[i] = true;
          printMachineStatus(status_MC[1]);
        }
        if ( i == mc_alarm ) {
          alarmStates[i] = true;
          printMachineStatus(status_MC[2]);
        }
        if ( i == mc_noWork ) {
          alarmStates[i] = true;
          printMachineStatus(status_MC[3]);
        }
        if ( i == mc_fullWork ) {
          alarmStates[i] = true;
          printMachineStatus(status_MC[4]);
        }
      }
      if (bitValue == 0 && alarmStates[i])
      {
        alarmStates[i] = false;
      }

    }
    //////////Counter/////////////////
    esp_task_wdt_reset();

  }
  if (currentMillis - previousMillis2 >= interval2 )
  {
    previousMillis2 = currentMillis;
    esp_task_wdt_reset();
    String rssi = String(WiFi.RSSI());

    ///////////////////////////////////////////////////
    mc_no = Machine_no ;  Serial.print("MC_NO : ");                    Serial.println(Machine_no);
    //mc_status = String("Not");  Serial.print("MC_Status : ");        Serial.println(mc_status);
    actionyear = String(Data[36]);  Serial.print("YY : ");              Serial.println(actionyear);
    actionmonth = String(Data[37]);  Serial.print("MM : ");             Serial.println(actionmonth);
    actiondate = String(Data[38]);  Serial.print("DD : ");              Serial.println(actiondate);
    actionhour = String(Data[39]);  Serial.print("HH : ");              Serial.println(actionhour);
    actionmin = String(Data[40]);  Serial.print("mm : ");               Serial.println(actionmin);
    actionsec = String(Data[41]);  Serial.print("ss : ");               Serial.println(actionsec);
    avgct     = String(Data[42]);  Serial.print("Avg_Cycletime : ");                   Serial.println(avgct);
    utl_s1    = String(Data[43]);  Serial.print("Utilization_Shift1 : ");               Serial.println(utl_s1);
    utl_s2    = String(Data[44]);  Serial.print("Utilization_Shift2 : ");               Serial.println(utl_s2);
    utl_s3    = String(Data[45]);  Serial.print("Utilization_Shift3 : ");              Serial.println(utl_s3);
    utl_total    = String(Data[46]);  Serial.print("Utilization_Total : ");            Serial.println(utl_total);
    prod_s1    = String(Data[47]);  Serial.print("Production_Shift1 : ");               Serial.println(prod_s1);
    prod_s2    = String(Data[48]);  Serial.print("Production_Shift2 : ");               Serial.println(prod_s2);
    prod_s3    = String(Data[49]);  Serial.print("Production_Shift3 : ");              Serial.println(prod_s3);
    prod_total    = String(Data[50]);  Serial.print("Production_Total : ");            Serial.println(prod_total);


    ///////////////////////////////////////////////////////////////////

    esp_task_wdt_reset();
    digitalWrite(led_published, LOW);

    rssi = WiFi.RSSI(); // WiFi strength
    Serial.print("rssi : "); Serial.println(WiFi.RSSI());
    // สร้าง JSON object
    StaticJsonDocument<5000> doc;
    doc["mc_no"]                    = Machine_no;
    doc["rssi"]                     = rssi;
    doc["MC_Status"]                = mc_status;
    doc["YY"]                  = actionyear;
    doc["MM"]                  = actionmonth;
    doc["DD"]                  = actiondate;
    doc["HH"]                  = actionhour;
    doc["mm"]                  = actionmin;
    doc["ss"]                  = actionsec;
    doc["Avg_Cycletime"]       = avgct;
    doc["Utilization_Shift1"]      = utl_s1;
    doc["Utilization_Shift2"]      = utl_s2;
    doc["Utilization_Shift3"]      = utl_s3;
    doc["Utilization_Total"]       = utl_total;
    doc["Production_Shift1"]       = prod_s1;
    doc["Production_Shift2"]       = prod_s2;
    doc["Production_Shift3"]       = prod_s3;
    doc["Production_Total"]        = prod_total;

    // แปลง JSON object เป็น string
    String jsonStr;
    serializeJson(doc, jsonStr);

    // ส่งข้อมูลผ่าน MQTT
    client.publish("iot/ic11r/mcdata", jsonStr.c_str());
    Serial.println(jsonStr);

    Serial.println("\n---------------finish loop------------------\n\n");
    deleteEmptyLines("/data.txt");
    Serial.println("-----------------Read data.txt-----------------");
    readFile(SPIFFS, "/data.txt");
    Serial.println("-----------------Read data.txt-----------------");
    lineCount = countLines("/data.txt");
    Serial.print("Number of lines in file: ");
    alarmCount = lineCount;
    Serial.println(lineCount);

    deleteEmptyLines("/mc_status.txt");
    Serial.println("-----------------mc_status.txt-----------------");
    readFile(SPIFFS, "/mc_status.txt");
    Serial.println("-----------------mc_status.txt-----------------");
    lineCount = countLines("/mc_status.txt");
    Serial.print("Number of lines in file: ");
    alarmCount = lineCount;
    Serial.println(lineCount);

    /*Serial.println("-----------------state.txt-----------------");
      readFile(SPIFFS, "/state.txt");
      Serial.println("-----------------state.txt-----------------");*/

    listDir(SPIFFS, "/", 0);

  }

  if (currentMillis - previousMillis4 >= interval4 )  ////////////////Sent to FTP
  {
    previousMillis4 = currentMillis;
    if (WiFi.status() == WL_CONNECTED)
    {
      esp_task_wdt_reset();
      sent_ftp_alarm();     ////
      sent_ftp_mc();
    }
  }

  if (currentMillis - previousMillis3 >= interval3 )   ///////////////////reconnect
  {
    previousMillis3 = currentMillis;
    reconnect() ;
  }
  int h = actionhour.toInt();
  int m = actionmin.toInt();
  int s = actionsec.toInt();
  if (h == 7 && m == 0 && s <= 20 )
  {
    SPIFFS.format();
    Serial.println("*****************************");
    ESP.restart();
  }
}

void printMachineStatus(String status_mc)
{
  int lineCount3 = countLines("/mc_status.txt");
  String output_nc = (formatTime(yy, mm, dd, hh, mn, sec)) + "," + status_mc;

  if (lineCount3 >= 2 )
  {
    appendToLine_MC(SPIFFS, "/mc_status.txt", lineCount3, output_nc.c_str(), status_mc.c_str() );
  }
}

void printAlarmList()
{
  int alarmIndex;

  lineCount = countLines("/data.txt");
  int Count2 = lineCount;
  deleteLine("/data.txt", Count2);
  delay(10);
  if (alarmIndex < alarm_coil && Count2 >= 2 )
  {
    appendToLine(SPIFFS, "/data.txt", Count2, output.c_str(), alarmNames[bitIndex].c_str());

  }

}

void sent_ftp_alarm()
{
  moveDataFromFile("/data.txt", "/destination.txt");
  lineCount = countLines("/data.txt");
  int Count3 = lineCount;
  if (Count3 > 2)
  {
    ftp.OpenConnection();
    File dataFile1 = SPIFFS.open("/destination.txt", "r");

    if (dataFile1) {
      String content1 = dataFile1.readString();
      dataFile1.close();
      if (content1.length() > 0) {
        // Upload the content to the FTP server
        ftp.InitFile("Type A");
        ftp.ChangeWorkDir("/data_alarmlist/gd/"); // Change this to your remote directory    ////data_mcstatus\gd
        ftp.NewFile("nht_gd_alarmlist_ic11b.txt");
        ftp.Write(content1.c_str());
        ftp.CloseFile();
      }
    } else {
      Serial.println("Failed to open data.txt for reading");
    }

    ftp.CloseConnection();
  } else
  {
    Serial.println("Failed to open nht_gd_alarmlist_ic11b.txt Empty");
    delay(1000);
  }
  deleteFile(SPIFFS, "/destination.txt");
}

void sent_ftp_mc()
{
  lineCount = countLines("/mc_status.txt");
  int Count4 = lineCount;
  if (Count4 > 2)
  {
    ftp.OpenConnection();
    File dataFile2 = SPIFFS.open("/mc_status.txt", "r");
    if (dataFile2) {
      String content2 = dataFile2.readString();
      dataFile2.close();
      if (content2.length() > 0) {
        // Upload the content to the FTP server
        ftp.InitFile("Type A");
        ftp.ChangeWorkDir("/data_mcstatus/gd/"); // Change this to your remote directory    ////data_mcstatus\gd
        ftp.NewFile("nht_gd_mcstatus_ic11b.txt");
        ftp.Write(content2.c_str());
        ftp.CloseFile();
      }
    } else {
      Serial.println("Failed to open mc_status.txt for reading");
    }

    ftp.CloseConnection();
  } else
  {
    Serial.println("Failed to open nht_gd_mcstatus_ic11b.txt Empty");
    delay(1000);
  }
}
