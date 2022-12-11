#include <ESP8266WiFi.h>
#include "ESPAsyncWebServer.h"
#include <Wire.h>
#include <Time.h>
#include <TimeLib.h>
#include <ArduinoJson.h>
#include <WiFiClient.h>

int led1 = D2;
int led2 = D3;
int led3 = D4;
int led4 = D5;
int led5 = D6;
int rele = D7;
int e_relay;
int e_led1;
int e_led2;
int e_led3;
int e_led4;
int e_led5;
double e_mic;
int sonido_analogico = A0;
int sonido_digital = D1;
const int sensitivity = -58; //Sensibilidad del microfono  
const int gain = 20;             
const int sampleWindow = 1000;
unsigned int sample;


const char* ssid = "Alejandro";
const char* password = "12345678";
WiFiClient wifiClient;

AsyncWebServer server(80);
IPAddress IP;

void setup() {
  Serial.begin(115200);
  Serial.println();
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");  
  }

  Serial.println("Wifi Connected");
  Serial.println("IP: ");
  Serial.println(WiFi.localIP());
 // Serial.print("Setting AP (Access Point)…");
  //WiFi.softAP(ssid, password);

 IP = WiFi.softAPIP();
  //Serial.print("AP IP address: ");
  //Serial.println(IP);

  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  pinMode(led4, OUTPUT);
  pinMode(led5, OUTPUT);
  pinMode(rele, OUTPUT);

  server.on("/recolect_data", HTTP_GET, [](AsyncWebServerRequest *request){
   String x = GenerateAndSerializeJson (e_relay, e_led1, e_led2, e_led3, e_led4, e_led5, e_mic);
   //String x = GenerateAndSerializeJson (0, 1, 1, 1, 1, 0, 45);
    request->send_P(200, "text/plain", x.c_str());
  });

  bool status;
  server.begin();
}

String GenerateAndSerializeJson(int relay, int led1, int led2, int led3, int led4, int led5, double mic){ 
  DynamicJsonDocument doc(1024);
  char json[2000];

  doc["controller_name"] = "NodeMCU esp8266 - G2";
  time_t fecha = now();
  String ahora=String(year(fecha)) + "-" + String(month(fecha)) + "-" + String(day(fecha))+ "T" + String(hour(fecha)) + ":" + String(minute(fecha)) + ":" + String(second(fecha)) + "Z";
  doc["date"] = ahora;

  // Armando el array actuators
  JsonArray arrActuator = doc.createNestedArray("actuators"); 
  DynamicJsonDocument act(1024);
  
  JsonObject actrelay = act.to<JsonObject>();
  actrelay["type"]="relay";
  actrelay["current_value"] = relay; 
  arrActuator.add(actrelay);

  JsonObject actled1 = act.to<JsonObject>();
  actled1["type"]="led1";
  actled1["current_value"] = led1; 
  arrActuator.add(actled1);

  JsonObject actled2 = act.to<JsonObject>();
  actled2["type"]="led2";
  actled2["current_value"] = led2; 
  arrActuator.add(actled2);

  JsonObject actled3 = act.to<JsonObject>();
  actled3["type"]="led3";
  actled3["current_value"] = led3; 
  arrActuator.add(actled3);

  JsonObject actled4 = act.to<JsonObject>();
  actled4["type"]="led4";
  actled4["current_value"] = led4; 
  arrActuator.add(actled4);

  JsonObject actled5 = act.to<JsonObject>();
  actled5["type"]="led5";
  actled5["current_value"] = led5; 
  arrActuator.add(actled5);

  // Armando array de sensores
  JsonArray arrSensor = doc.createNestedArray("sensors"); 
  StaticJsonDocument<300> sen; 
  JsonObject sensors = sen.to<JsonObject>(); 
  sensors["type"]="mic";
  sensors["current_value"] = mic; 
  arrSensor.add(sensors);

  serializeJsonPretty(doc, json);
  Serial.println(convertToString(json));
  int json_size = sizeof(json) / sizeof(char);
  return convertToString(json);
}
String convertToString(char* a){
    String s(a);
    return s;
}

void loop() {
    
    unsigned long startMillis= millis();
    unsigned int peakToPeak = 0; 
    unsigned int signalMax = 0;
    unsigned int signalMin = 1024;

    int val_analogico = analogRead(sonido_analogico);
    Serial.print("Valor analogico: ");
    Serial.print(val_analogico);
    Serial.print("\t");
    Serial.print("->");
    Serial.print("\t");
  
   while (millis() - startMillis < sampleWindow){
     sample = analogRead(sonido_analogico);
     if (sample < 1024){
      if (sample > signalMax){
       signalMax = sample;
      }
      else if (sample < signalMin){
         signalMin = sample;
      }
     }
    }
    peakToPeak = signalMax - signalMin;
    double volts = ((peakToPeak * 3.3) / 1024)*10;
    double volts_db = 20*log10(volts/0.001259); 
    Serial.print("dB: " + String(volts_db) );
    Serial.print("\n");
    e_mic = volts_db;

    //Comienzo con el rele apagado
    digitalWrite (rele, LOW);
    e_relay = 0;

    if (val_analogico < 500){
      digitalWrite (led1, HIGH);
      digitalWrite (led2, HIGH);
      digitalWrite (led3, HIGH);
      digitalWrite (led4, HIGH);
      digitalWrite (led5, HIGH);
      e_led1 = 1;
      e_led2 = 1;
      e_led3 = 1;
      e_led4 = 1;
      e_led5 = 1;
      digitalWrite (rele, HIGH);
      e_relay = 1;
    }
    else if (val_analogico < 650){
      digitalWrite (led1, HIGH);
      digitalWrite (led2, HIGH);
      digitalWrite (led3, HIGH);
      digitalWrite (led4, HIGH);
      digitalWrite (led5, LOW);
      e_led1 = 1;
      e_led2 = 1;
      e_led3 = 1;
      e_led4 = 1;
      e_led5 = 0;
    }
    else if (val_analogico < 750){
      digitalWrite (led1, HIGH);
      digitalWrite (led2, HIGH);
      digitalWrite (led3, HIGH);
      digitalWrite (led4, LOW);
      digitalWrite (led5, LOW);
      e_led1 = 1;
      e_led2 = 1;
      e_led3 = 1;
      e_led4 = 0;
      e_led5 = 0;
    }
    else if (val_analogico < 850){
      digitalWrite (led1, HIGH);
      digitalWrite (led2, HIGH);
      digitalWrite (led3, LOW);
      digitalWrite (led4, LOW);
      digitalWrite (led5, LOW);
      e_led1 = 1;
      e_led2 = 1;
      e_led3 = 0;
      e_led4 = 0;
      e_led5 = 0;
    }
    else if (val_analogico < 950){
      digitalWrite (led1, HIGH);
      digitalWrite (led2, LOW);
      digitalWrite (led3, LOW);
      digitalWrite (led4, LOW);
      digitalWrite (led5, LOW);
      e_led1 = 1;
      e_led2 = 0;
      e_led3 = 0;
      e_led4 = 0;
      e_led5 = 0;
    }
    else {
      digitalWrite (led1, LOW);
      digitalWrite (led2, LOW);
      digitalWrite (led3, LOW);
      digitalWrite (led4, LOW);
      digitalWrite (led5, LOW);
      e_led1 = 0;
      e_led2 = 0;
      e_led3 = 0;
      e_led4 = 0;
      e_led5 = 0;
    }
    if (e_relay == 1) {
      delay(5000);
    }
    delay(500);
}
