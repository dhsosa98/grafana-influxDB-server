 // La función setup corre una vez, al prenderse el Arduino
// o bien al reiniciar mediante el botón reset
  //anodo comun 
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>


#define SLAVE_ADDR 9
#define ANSWERSIZE 5




//definimos NTPClient para poder consultar el tiempo.
const long utcOffset = -10800;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP,"pool.ntp.org",utcOffset);

//definimos wifiClient para conectarnos a una red
WiFiClient wifiClient;
const char* ssid = "Alejandro";
const char* password = "12345678";

const char ASK_FOR_LENGTH = 'L';
const char ASK_FOR_DATA = 'D';
const int OFF = HIGH;


char request = ' ';
char requestIndex = 0;
int var = 0;
int cajas = 0;
int alfajores_total = 0;
int alfajores_caja = 0;
int sensorValue;
int relayStatus = 0;
char json[256];
String start_caja;
String Serializ;


ESP8266WebServer server(80);

void handleNotFound() 
{
   server.send(404, "text/plain", "Not found");
}

String readStatus() {
  StaticJsonDocument<256> doc;

  doc["controller_name"] = "nodeMCU-ESP8266";
  timeClient.update();
  doc["date"] = timeClient.getFormattedTime();
  
  JsonObject actuators_0 = doc["actuators"].createNestedObject();
  actuators_0["type"] = "rele";
  actuators_0["current_value"] = relayStatus;
  
  JsonObject sensors_0 = doc["sensors"].createNestedObject();
  sensors_0["type"] = "hall";
  sensors_0["current_value"] = sensorValue;
  
  serializeJson(doc, json);
  return json;
}
void setup()
{   
  //Wire.begin(SLAVE_ADDR);
  //Wire.onRequest(requestEvent);
  Serial.begin(9600);
  WiFi.begin(ssid,password);
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");  
  }
  Serial.println("Wifi Connected");
  Serial.println("IP: ");
  Serial.println(WiFi.localIP());
  server.on("/recolect_data", []() {
    server.send(200, "text/plain",readStatus().c_str());
  });
  pinMode(5,OUTPUT);   //Relay
  pinMode(16,OUTPUT); //Led
  timeClient.begin();
  // Start server
  server.begin();
  Serial.println("HTTP server started");
}


void loop(){
  server.handleClient();
  delay(10);
  sensorValue = analogRead(A0); 
  if(sensorValue < 30){
    if (alfajores_caja == 0){
      cajas ++;
      timeClient.update();
      httpPOST("start",timeClient.getFormattedTime());
    }
    delay(50);
    alfajores_caja ++; // lleva el conteo de alfajores en la caja actual
    alfajores_total ++; // lleva el conteo de alfajores totales
    Serial.println(alfajores_caja);
    if(alfajores_caja == 20){
      digitalWrite(5, HIGH);
      digitalWrite(16,HIGH);   //Led
      delay(5000);
      digitalWrite(5, LOW);
      digitalWrite(16,LOW);
      timeClient.update();
      httpPOST("finish",timeClient.getFormattedTime());
      relayStatus = 1;
      alfajores_caja = 0;
    }  
  }  
}

void httpPOST(String timer, String date){
  int contRepeat = 0;
  HTTPClient http;
  char *url = "http://192.168.43.7:4200/post";
  http.begin(wifiClient,url);
  http.addHeader("Content-Type","application/json");
  DynamicJsonDocument postMessage(2048);
  postMessage["message"] = timer;
  postMessage["time"] = date;
  String jsonBody;
  serializeJson(postMessage,jsonBody);
  Serial.println(jsonBody);
  int resCode = http.POST(jsonBody);
  Serial.println(resCode);
  while(resCode != 201 && contRepeat < 3){
    int resCode = http.POST(jsonBody);
    Serial.println(resCode);
    contRepeat++;
  } 
  String res = http.getString();
  parserMessage(res);
  http.end();
}
void httpGET(){ 
  HTTPClient http;
  char *url = "http://192.168.43.7:4200/";
  http.begin(wifiClient,url);
  int resCode = http.GET();
  Serial.println(resCode);
  String res = http.getString();
  Serial.println(res);
  parserMessage(res);
  http.end();
}
void parserMessage(String res){
  DynamicJsonDocument doc(2048);
  deserializeJson(doc,res);
  const char* message = doc["message"];
  Serial.println(message);
}

 
 //Serial.print("Json:");
// Serial.print(json);
 