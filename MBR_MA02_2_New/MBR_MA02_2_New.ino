#include <WiFi.h>
#include "PubSubClient.h"
#include <ArduinoOTA.h>
#include <FS.h>
#include <EEPROM.h>
#include <ArduinoJson.h>
#include "ModbusRtu.h"
#include <esp_task_wdt.h>
#include <iostream>
#include <string>
#define WDT_TIMEOUT 300

#ifdef ESP32
#include <SPIFFS.h>
#endif

#define led_connection 42
#define led_published 41

  const char* ssid = "NHT_DX";
  const char* password = "nhtmic@admin";
  //const char* ssid = "TP-Link_2B32";
  //const char* password = "58252017";
  const char* mqtt_server = "192.168.1.107";

  //////////////////////SETUP/////////////////////////
  IPAddress local_IP(192, 168, 68, 43); // Static IP address192.168.100.164
  IPAddress gateway(192, 168, 68, 1);    // Gateway IP address
  IPAddress subnet(255, 255, 255, 0);     // subnet

  char Machine_no[] = "MBR_MA03";
  //////////////////////SETUP/////////////////////////

WiFiClient espClient;
PubSubClient client(espClient);

Modbus slave(1, Serial1, 0);

int8_t state = 0;

int count_connection;

void setup_wifi()
{
  esp_task_wdt_reset();
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.config(local_IP, gateway, subnet);
  //WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    esp_task_wdt_reset();
    count_connection++;
    digitalWrite(led_connection, HIGH); delay(100); digitalWrite(led_connection, LOW); delay(100);
    Serial.print(".");
    if (count_connection > 20)
    {
      ESP.restart();
    }
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
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

int timeout ;

void reconnect()
{
  esp_task_wdt_reset();
  // Loop until we're reconnected
  while (!client.connected())
  {
    digitalWrite(led_connection, HIGH);
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      //client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      //delay(5000);
      digitalWrite(led_connection, HIGH); delay(300); digitalWrite(led_connection, LOW); delay(300);
      timeout++;
      if (timeout >= 10)
      {
        ESP.restart();
      }
    }
  }
}

void setup()
{
  Serial.begin(500000);
  Serial1.begin(9600);
  esp_task_wdt_init(WDT_TIMEOUT, true);
  esp_task_wdt_add(NULL);
  esp_task_wdt_reset();
  pinMode ( led_connection , OUTPUT);
  pinMode ( led_published, OUTPUT);
  Serial.println("Booting");
  setup_wifi();

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
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  slave.start();
}

String rssi, ZR12000, ZR12002, ZR12004, ZR12010, ZR12012, ZR12014, ZR12016, ZR12018, ZR12020, ZR12022, ZR12024, ZR12026, ZR12028, ZR12030, ZR12032, ZR12034, ZR12036, ZR12038, ZR12040, ZR12042, ZR12044, ZR12046, ZR12048, ZR12050, ZR12060, ZR12120, ZR12200, ZR12210, ZR12220, ZR12230, ZR12240, ZR12250, ZR12260, ZR12270, ZR12280, ZR12410, ZR12411, ZR12412, ZR12413, ZR12414, ZR12415, ZR12416, ZR12417, ZR12418, ZR12419, ZR12420, ZR12421;

//char d_rssi[16], d1[16], d2[16], d3[16], d4[16], d5[16], d6[16], d7[16], d8[16], d9[16], d10[16], d11[16], d12[16], d13[16], d14[16], d15[16], d16[16], d17[16], d18[16], d19[16], d20[16], d21[16], d22[16], d23[16], d24[16], d25[16], d26[16], d27[16], d28[16], d29[16], d30[16], d31[16];

//////////////เพิ่มเติม////////////////////////////
const int num = 47;   //จำนวน Register
uint16_t Data[num];
unsigned long previousMillis = 0;
unsigned long tempus = 0;
const unsigned long interval = 10000; //เวลาในการส่ง Data
//////////////เพิ่มเติม////////////////////////////

void loop()
{
  ArduinoOTA.handle();
  if (!client.connected())
  {
    reconnect();
  }
  client.loop();
  esp_task_wdt_reset();
  state = slave.poll( Data, num );
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval && state == 8)
  {
    Serial.println("\n---------------starting loop----------------");
    previousMillis = currentMillis;
    esp_task_wdt_reset();
    String rssi = String(WiFi.RSSI());

    ///////////////////////////////////////////////////
    ZR12000 = String(Data[0]);  Serial.print("Daily_OK : ");                 Serial.println(ZR12000);
    ZR12002 = String(Data[1]);  Serial.print("Daily_NG : ");                 Serial.println(ZR12002);
    ZR12004 = String(Data[2]);  Serial.print("Daily_Total : ");              Serial.println(ZR12004);
    ZR12010 = String(Data[3]);  Serial.print("Ball_C1_OK : ");               Serial.println(ZR12010);
    ZR12012 = String(Data[4]);  Serial.print("Ball_C2_OK : ");               Serial.println(ZR12012);
    ZR12014 = String(Data[5]);  Serial.print("Ball_C3_OK : ");               Serial.println(ZR12014);
    ZR12016 = String(Data[6]);  Serial.print("Ball_C4_OK : ");               Serial.println(ZR12016);
    ZR12018 = String(Data[7]);  Serial.print("Ball_C5_OK : ");               Serial.println(ZR12018);
    ZR12020 = String(Data[8]);  Serial.print("Ball_C1_NG : ");               Serial.println(ZR12020);
    ZR12022 = String(Data[9]);  Serial.print("Ball_C2_NG : ");               Serial.println(ZR12022);
    ZR12024 = String(Data[10]); Serial.print("Ball_C3_NG : ");               Serial.println(ZR12024);
    ZR12026 = String(Data[11]); Serial.print("Ball_C4_NG : ");               Serial.println(ZR12026);
    ZR12028 = String(Data[12]); Serial.print("Ball_C5_NG : ");               Serial.println(ZR12028);
    ZR12030 = String(Data[13]); Serial.print("Ball_Check_Camera_Qty : ");    Serial.println(ZR12030);
    ZR12032 = String(Data[14]); Serial.print("Ball_Check_Camera_Angle : ");  Serial.println(ZR12032);
    ZR12034 = String(Data[15]); Serial.print("Separate_NG_1st : ");          Serial.println(ZR12034);
    ZR12036 = String(Data[16]); Serial.print("Saparate_NG_2nd : ");          Serial.println(ZR12036);
    ZR12038 = String(Data[17]); Serial.print("MN_MI_RTNR_NG : ");            Serial.println(ZR12038);
    ZR12040 = String(Data[18]); Serial.print("D2_RTNR_NG : ");               Serial.println(ZR12040);
    ZR12042 = String(Data[19]); Serial.print("D1_RTNR_NG : ");               Serial.println(ZR12042);
    ZR12044 = String(Data[20]); Serial.print("Pre_Press_NG : ");             Serial.println(ZR12044);
    ZR12046 = String(Data[21]); Serial.print("Main_Press_NG : ");            Serial.println(ZR12046);
    ZR12048 = String(Data[22]); Serial.print("Press_Check_NG : ");           Serial.println(ZR12048);
    ZR12050 = String(Data[23]); Serial.print("RTNR_Camera_NG : ");           Serial.println(ZR12050);
    ZR12060 = String(Data[24]); Serial.print("Cycle_Time : ");               Serial.println(ZR12060);
    ZR12120 = String(Data[25]); Serial.print("Target_Utilize : ");           Serial.println(ZR12120);
    ZR12200 = String(Data[26]); Serial.print("Error_Time : ");               Serial.println(ZR12200);
    ZR12210 = String(Data[27]); Serial.print("Alarm_Time : ");               Serial.println(ZR12210);
    ZR12220 = String(Data[28]); Serial.print("Run_Time : ");                 Serial.println(ZR12220);
    ZR12230 = String(Data[29]); Serial.print("Stop_Time : ");                Serial.println(ZR12230);
    ZR12240 = String(Data[30]); Serial.print("Wait_Part_Time : ");           Serial.println(ZR12240);
    ZR12250 = String(Data[31]); Serial.print("Full_Part_Time : ");           Serial.println(ZR12250);
    ZR12260 = String(Data[32]); Serial.print("Adjust_Time : ");              Serial.println(ZR12260);
    ZR12270 = String(Data[33]); Serial.print("Set_Up_Time : ");              Serial.println(ZR12270);
    ZR12280 = String(Data[34]); Serial.print("Plan_Stop_Time : ");           Serial.println(ZR12280);
    ZR12410 = String(Data[35]); Serial.print("MODEL 1 : ");                  Serial.println(ZR12410);
    ZR12411 = String(Data[36]); Serial.print("MODEL 2 : ");                  Serial.println(ZR12411);
    ZR12412 = String(Data[37]); Serial.print("MODEL 3 : ");                  Serial.println(ZR12412);
    ZR12413 = String(Data[38]); Serial.print("MODEL 4 : ");                  Serial.println(ZR12413);
    ZR12414 = String(Data[39]); Serial.print("MODEL 5 : ");                  Serial.println(ZR12414);
    ZR12415 = String(Data[40]); Serial.print("MODEL 6 : ");                  Serial.println(ZR12415);
    ZR12416 = String(Data[41]); Serial.print("MODEL 7 : ");                  Serial.println(ZR12416);
    ZR12417 = String(Data[42]); Serial.print("MODEL 8 : ");                  Serial.println(ZR12417);
    ZR12418 = String(Data[43]); Serial.print("MODEL 9 : ");                  Serial.println(ZR12418);
    ZR12419 = String(Data[44]); Serial.print("MODEL 10 : ");                 Serial.println(ZR12419);
    ZR12420 = String(Data[45]); Serial.print("MODEL 11 : ");                 Serial.println(ZR12420);
    ZR12421 = String(Data[46]); Serial.print("MODEL 12 : ");                 Serial.println(ZR12421);
    ///////////////////////////////////////////////////////////////////

    esp_task_wdt_reset();
    digitalWrite(led_published, LOW); delay(100);digitalWrite(led_published, HIGH); 

    rssi = WiFi.RSSI(); // WiFi strength
    Serial.print("rssi : "); Serial.println(WiFi.RSSI());
    // สร้าง JSON object
    StaticJsonDocument<5000> doc;
    doc["mc_no"]                    = Machine_no;
    doc["rssi"]                     = rssi;
    doc["Daily_OK"]                 = ZR12000;
    doc["Daily_NG"]                 = ZR12002;
    doc["Daily_Total"]              = ZR12004;
    doc["Ball_C1_OK"]               = ZR12010;
    doc["Ball_C2_OK"]               = ZR12012;
    doc["Ball_C3_OK"]               = ZR12014;
    doc["Ball_C4_OK"]               = ZR12016;
    doc["Ball_C5_OK"]               = ZR12018;
    doc["Ball_C1_NG"]               = ZR12020;
    doc["Ball_C2_NG"]               = ZR12022;
    doc["Ball_C3_NG"]               = ZR12024;
    doc["Ball_C4_NG"]               = ZR12026;
    doc["Ball_C5_NG"]               = ZR12028;
    doc["Ball_Check_Camera_Qty"]    = ZR12030;
    doc["Ball_Check_Camera_Angle"]  = ZR12032;
    doc["Separate_NG_1st"]          = ZR12034;
    doc["Separate_NG_2nd"]          = ZR12036;
    doc["MN_MI_RTNR_NG"]            = ZR12038;
    doc["D2_RTNR_NG"]               = ZR12040;
    doc["D1_RTNR_NG"]               = ZR12042;
    doc["Pre_Press_NG"]             = ZR12044;
    doc["Main_Press_NG"]            = ZR12046;
    doc["Press_Check_NG"]           = ZR12048;
    doc["RTNR_Camera_NG"]           = ZR12050;
    doc["Cycle_Time"]               = ZR12060;
    doc["Target_Utilize"]           = ZR12120;
    doc["Error_Time"]               = ZR12200;
    doc["Alarm_Time"]               = ZR12210;
    doc["Run_Time"]                 = ZR12220;
    doc["Stop_Time"]                = ZR12230;
    doc["Wait_Part_Time"]           = ZR12240;
    doc["Full_Part_Time"]           = ZR12250;
    doc["Adjust_Time"]              = ZR12260;
    doc["Set_Up_Time"]              = ZR12270;
    doc["Plan_Stop_Time"]           = ZR12280;
    doc["Model_1"]                  = ZR12410;
    doc["Model_2"]                  = ZR12411;
    doc["Model_3"]                  = ZR12412;
    doc["Model_4"]                  = ZR12413;
    doc["Model_5"]                  = ZR12414;
    doc["Model_6"]                  = ZR12415;
    doc["Model_7"]                  = ZR12416;
    doc["Model_8"]                  = ZR12417;
    doc["Model_9"]                  = ZR12418;
    doc["Model_10"]                 = ZR12419;
    doc["Model_11"]                 = ZR12420;
    doc["Model_12"]                 = ZR12421;

    // แปลง JSON object เป็น string
    String jsonStr;
    serializeJson(doc, jsonStr);

    // ส่งข้อมูลผ่าน MQTT
    client.publish("iot/mbr_ma03/mcdata", jsonStr.c_str());
    Serial.println(jsonStr);

    Serial.println("\n---------------finish loop------------------\n\n");
  }

}
