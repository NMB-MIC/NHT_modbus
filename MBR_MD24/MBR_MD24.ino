#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "PubSubClient.h"
#include <ModbusMaster.h>
#include <ArduinoJson.h>
#include <esp_task_wdt.h>
#include <iostream>
#include <string>
#define WDT_TIMEOUT 300

ModbusMaster node;

#define led_connection 42
#define led_published 41

const char* ssid = "TP-Link_2B32";
const char* password = "58252017";
//const char* ssid = "NHT_DX";
//const char* password = "nhtmic@admin";
//const char* ssid = "TP-Link_EF46";
//const char* password = "98148813";
const char* mqtt_server = "192.168.1.107";


//////////////////////SETUP/////////////////////////
IPAddress local_IP(192, 168, 1, 24); // Static IP address192.168.100.164
IPAddress gateway(192, 168, 1, 1);    // Gateway IP address
IPAddress subnet(255, 255, 255, 0);     // subnet

char Machine_no[] = "MBR_MD24";
//////////////////////SETUP/////////////////////////

WiFiClient espClient;
PubSubClient client(espClient);

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
    digitalWrite(led_connection, HIGH);delay(100);digitalWrite(led_connection, LOW);delay(100);  
    Serial.print(".");
    if(count_connection>20)
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
  while(!client.connected()) 
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
      digitalWrite(led_connection, HIGH);delay(300);digitalWrite(led_connection, LOW);delay(300);    
      timeout++;
      if(timeout >= 10)
      {
        ESP.restart();
      }
    }
  }
}

void setup() 
{
  Serial.begin(9600);
  Serial1.begin(9600);
  esp_task_wdt_init(WDT_TIMEOUT, true); 
  esp_task_wdt_add(NULL);
  esp_task_wdt_reset();
  node.begin(1,Serial1);
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
}

String rssi,D6000,D6002,D6004,D6100,D6102,D6104,D6010,D6020,D6030,D6040,D6050,D6012,D6022,D6032,D6042,D6052,D6080,D6082,D6084,D6086,D6088,D6090,D6310,D6300,D6312,D6302,D32,D6060,D6062,D6070,D6072;

char d_rssi[16],d1[16],d2[16],d3[16],d4[16],d5[16],d6[16],d7[16],d8[16],d9[16],d10[16],d11[16],d12[16],d13[16],d14[16],d15[16],d16[16],d17[16],d18[16],d19[16],d20[16],d21[16],d22[16],d23[16],d24[16],d25[16],d26[16],d27[16],d28[16],d29[16],d30[16],d31[16];

int num = 31;

void loop() 
{
  ArduinoOTA.handle();
  if (!client.connected()) 
  {
    esp_task_wdt_reset();
    reconnect();
  }
  client.loop();
  esp_task_wdt_reset();
  uint8_t j, result;
  uint16_t data[num];
  digitalWrite(led_connection, HIGH);  
  digitalWrite(led_published, HIGH);
  Serial.println("\n---------------starting loop----------------");

  result = node.readHoldingRegisters(0, num);
  if (result == node.ku8MBSuccess)
  {
    for (j = 0; j < num; j++)
    {
      data[j] = node.getResponseBuffer(j);
      delay(10);
    }
    D6000 = String(data[0]);Serial.print("Daily Total : ");Serial.println(D6000);
    D6002 = String(data[1]);Serial.print("Daily OK : ");Serial.println(D6002);
    D6004 = String(data[2]);Serial.print("Daily NG : ");Serial.println(D6004);
    D6100 = String(data[3]);Serial.print("Shift Total : ");Serial.println(D6100);
    D6102 = String(data[4]);Serial.print("Shift OK : ");Serial.println(D6102);
    D6104 = String(data[5]);Serial.print("Shift NG : ");Serial.println(D6104);
    D6010 = String(data[6]);Serial.print("C1 OK : ");Serial.println(D6010);
    D6020 = String(data[7]);Serial.print("C2 OK : ");Serial.println(D6020);
    D6030 = String(data[8]);Serial.print("C3 OK : ");Serial.println(D6030);
    D6040 = String(data[9]);Serial.print("C4 OK : ");Serial.println(D6040);
    D6050 = String(data[10]);Serial.print("C5 OK : ");Serial.println(D6050);
    D6012 = String(data[11]);Serial.print("C1 NG : ");Serial.println(D6012);
    D6022 = String(data[12]);Serial.print("C2 NG : ");Serial.println(D6022);
    D6032 = String(data[13]);Serial.print("C3 NG : ");Serial.println(D6032);
    D6042 = String(data[14]);Serial.print("C4 NG : ");Serial.println(D6042);
    D6052 = String(data[15]);Serial.print("C5 NG : ");Serial.println(D6052);
    D6080 = String(data[16]);Serial.print("D2 RTNR NG : ");Serial.println(D6080);
    D6082 = String(data[17]);Serial.print("D1 RTNR NG : ");Serial.println(D6082);
    D6084 = String(data[18]);Serial.print("PRE PRESS NG : ");Serial.println(D6084);
    D6086 = String(data[19]);Serial.print("MAIN PRESS NG : ");Serial.println(D6086);
    D6088 = String(data[20]);Serial.print("PRESS CHECK NG : ");Serial.println(D6088);
    D6090 = String(data[21]);Serial.print("RTNR CAMERA NG : ");Serial.println(D6090);
    D6310 = String(data[22]);Serial.print("BALL NG TOTAL DAILY : ");Serial.println(D6310);
    D6300 = String(data[23]);Serial.print("BALL NG TOTAL SHIFT : ");Serial.println(D6300);
    D6312 = String(data[24]);Serial.print("NG DATA TOTAL DAILY : ");Serial.println(D6312);
    D6302 = String(data[25]);Serial.print("NG DATA TOTAL SHIFT : ");Serial.println(D6302);
    D32 = String(data[26]);Serial.print("CYCLE TIME : ");Serial.println(D32);
    D6060 = String(data[27]);Serial.print("BALL CHECK CAMERA Q'TY : ");Serial.println(D6060);
    D6062 = String(data[28]);Serial.print("BALL CHECK CAMERA ANGLE : ");Serial.println(D6062);
    D6070 = String(data[29]);Serial.print("SEPARATE NG 1ST : ");Serial.println(D6070);
    D6072 = String(data[30]);Serial.print("SEPARATE NG 2ND : ");Serial.println(D6072);    
    esp_task_wdt_reset();
    digitalWrite(led_published, LOW);delay(500);digitalWrite(led_published, HIGH);delay(500);
  }
  rssi = WiFi.RSSI(); // WiFi strength
  Serial.print("rssi : ");Serial.println(WiFi.RSSI());

  delay(500);
  esp_task_wdt_reset();
  D6000.toCharArray(d1, 16);
  D6002.toCharArray(d2, 16);
  D6004.toCharArray(d3, 16);
  D6100.toCharArray(d4, 16);
  D6102.toCharArray(d5, 16);
  D6104.toCharArray(d6, 16);
  D6010.toCharArray(d7, 16);
  D6020.toCharArray(d8, 16);
  D6030.toCharArray(d9, 16);
  D6040.toCharArray(d10, 16);
  D6050.toCharArray(d11, 16);
  D6012.toCharArray(d12, 16);
  D6022.toCharArray(d13, 16);
  D6032.toCharArray(d14, 16);
  D6042.toCharArray(d15, 16);
  D6052.toCharArray(d16, 16);
  D6080.toCharArray(d17, 16);
  D6082.toCharArray(d18, 16);
  D6084.toCharArray(d19, 16);
  D6086.toCharArray(d20, 16);
  D6088.toCharArray(d21, 16);
  D6090.toCharArray(d22, 16);
  D6310.toCharArray(d23, 16);
  D6300.toCharArray(d24, 16);
  D6312.toCharArray(d25, 16);
  D6302.toCharArray(d26, 16);
  D32.toCharArray(d27, 16);
  D6060.toCharArray(d28, 16);
  D6062.toCharArray(d29, 16);
  D6070.toCharArray(d30, 16);
  D6072.toCharArray(d31, 16);
  rssi.toCharArray(d_rssi, 16);
  esp_task_wdt_reset();

  // สร้าง JSON object
  StaticJsonDocument<5000> doc;
  doc["M/C"] = Machine_no;
  doc["rssi"] = d_rssi;
  doc["D6000"] = d1;
  doc["D6002"] = d2;
  doc["D6004"] = d3;
  doc["D6100"] = d4;
  doc["D6102"] = d5;
  doc["D6104"] = d6;
  doc["D6010"] = d7;
  doc["D6020"] = d8;
  doc["D6030"] = d9;
  doc["D6040"] = d10;
  doc["D6050"] = d11;
  doc["D6012"] = d12;
  doc["D6022"] = d13;
  doc["D6032"] = d14;
  doc["D6042"] = d15;
  doc["D6052"] = d16;
  doc["D6080"] = d17;
  doc["D6082"] = d18;
  doc["D6084"] = d19;
  doc["D6086"] = d20;
  doc["D6088"] = d21;
  doc["D6090"] = d22;
  doc["D6310"] = d23;
  doc["D6300"] = d24;
  doc["D6312"] = d25;
  doc["D6302"] = d26;
  doc["D32"] = d27;
  doc["D6060"] = d28;
  doc["D6062"] = d29;
  doc["D6070"] = d30;
  doc["D6072"] = d31;
  
  // แปลง JSON object เป็น string
  String jsonStr;
  serializeJson(doc, jsonStr);

  // ส่งข้อมูลผ่าน MQTT
  client.publish("iot/mbr_md24/mcdata", jsonStr.c_str());
  Serial.println(jsonStr);

  
  Serial.println("\n---------------finish loop------------------\n\n");
  delay(5000);
}
