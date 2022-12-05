#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Wire.h>

#define SERVER_IP "192.168.43.46:8000"

#ifndef STASSID
#define STASSID "Alejandro" //Wifi SSID
#define STAPSK  "12345678" //Wifi PASSWORD

#define SDA_PIN D1         //Declare SCL Pin on NodeMCU 
#define SCL_PIN D2         //Declare SDA Pin on NodeMCU

#endif
String response;
String CURRENT_SLAVE_IP;

const char ASK_FOR_LENGTH = 'L';
const char ASK_FOR_DATA = 'D';

byte I2C_SLAVE_ADDR;

DynamicJsonDocument jsonDoc(1024);

#define SLAVESI2CSIZE 4
byte slaves[SLAVESI2CSIZE] = {0x01, 0x04, 0x05, 0x07};

#define SLAVESIPSIZE 3
String slavesIps[SLAVESIPSIZE] = { "192.168.43.26", "192.168.43.27", "192.168.43.9" };

void setup() {

  Serial.begin(115200);
  Wire.begin(D1, D2);

  Serial.println();
  Serial.println();
  Serial.println();
  WiFi.begin(STASSID, STAPSK);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());

}

void loop() {
  // Recolect Data by I2C
  for (int i = 0; i < SLAVESI2CSIZE; i = i + 1) {
      response="";
      Serial.println(I2C_SLAVE_ADDR);
      I2C_SLAVE_ADDR=slaves[i];
      askSlaveByI2C();
      if (!(response.equals(""))) { DeserializeResponse(); }
      delay(3000);
  }
  delay(4000);
  // Recolect Data by Http
  for (int i = 0; i < SLAVESIPSIZE; i = i + 1) {
      response="";
      CURRENT_SLAVE_IP=slavesIps[i];
      recolectDataByHttp();
      delay(3000);
  }
  delay(4000);
  sendToGrafana();
  delay(2000);
}

void recolectDataByHttp(){
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    HTTPClient http;
    Serial.print("[HTTP] begin...\n");
    // configure traged server and url
    http.begin(client, "http://" + CURRENT_SLAVE_IP + "/recolect_data"); //HTTP
    http.addHeader("Content-Type", "application/json");

    Serial.print("[HTTP] GET...\n");
    // start connection and send HTTP header and body
    int httpCode = http.GET();

    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] POST... code: %d\n", httpCode);

      // file found at server
      if (httpCode == HTTP_CODE_OK) {
        const String& payload = http.getString();
        response=http.getString();
        DeserializeResponse();
        Serial.println("received payload:\n<<");
        Serial.println(payload);
        Serial.println(">>");
        
      }
    } else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
  }
}

void sendToGrafana(){
  if (WiFi.status() == WL_CONNECTED) {
    String json;
    WiFiClient client;
    HTTPClient http;
    Serial.print("[HTTP] begin...\n");
    // configure traged server and url
    http.begin(client, "http://" SERVER_IP "/insert_data"); //HTTP
    http.addHeader("Content-Type", "application/json");
    serializeJson(jsonDoc, json);

    Serial.print("[HTTP] POST...\n");
    Serial.println(json);
    // start connection and send HTTP header and body
    int httpCode = http.POST(json);

    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] POST... code: %d\n", httpCode);

      // file found at server
      if (httpCode == HTTP_CODE_OK) {
        const String& payload = http.getString();
        Serial.println("received payload:\n<<");
        Serial.println(payload);
        Serial.println(">>");
      }
    } else {
      Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    jsonDoc.clear();
    http.end();
  }
}

void askSlaveByI2C()
{  
  unsigned int responseLength = askForLengthByI2C();
  if (responseLength == 0) return;
  Serial.println(responseLength);
  askForDataByIC2(responseLength);
  delay(500);
}

unsigned int askForLengthByI2C()
{
  Wire.beginTransmission(I2C_SLAVE_ADDR);
  Wire.write(ASK_FOR_LENGTH);
  Wire.endTransmission();

  Wire.requestFrom(I2C_SLAVE_ADDR, 1);
  unsigned int responseLength = Wire.read();
  return responseLength;
}

void askForDataByIC2(unsigned int responseLength)
{
  Wire.beginTransmission(I2C_SLAVE_ADDR);
  Wire.write(ASK_FOR_DATA);

  Wire.endTransmission();

  for (int requestIndex = 0; requestIndex <= (responseLength / 32); requestIndex++){
    Wire.requestFrom(I2C_SLAVE_ADDR, requestIndex < (responseLength / 32) ? 32 : responseLength % 32);
    while (Wire.available())
    {
        response += (char)Wire.read();
    }
  }
  Serial.println(response.length());
}

void DeserializeResponse()
{
  Serial.println(response);
  DynamicJsonDocument groups(1024);
  DeserializationError error = deserializeJson(groups, response);
  if (error) { return; }
  if (I2C_SLAVE_ADDR==0x01){
      jsonDoc["group1"]=groups;
  }
  if (I2C_SLAVE_ADDR==0x04){
      jsonDoc["group4"]=groups;
  }
  if (I2C_SLAVE_ADDR==0x05){
      jsonDoc["group5"]=groups;
  }
  if (I2C_SLAVE_ADDR==0x07){
      jsonDoc["group7"]=groups;
  }
  if (CURRENT_SLAVE_IP.equals(slavesIps[1])){
    jsonDoc["group2"]=groups;
  }
   if (CURRENT_SLAVE_IP.equals(slavesIps[0])){
    jsonDoc["group3"]=groups;
  }
   if (CURRENT_SLAVE_IP.equals(slavesIps[2])){
    jsonDoc["group6"]=groups;
  }
  groups.clear();
}
