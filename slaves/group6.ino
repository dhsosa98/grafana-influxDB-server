// C++ code -- Grupo 6 -- Lab 3
// Import required libraries
#include <ESP8266WiFi.h>
#include "ESPAsyncWebServer.h"
#include <ArduinoJson.h>
#include <Time.h>
#include <TimeLib.h>
#include <Wire.h>
#include <WiFiClient.h>

// Set your access point network credentials
const char* ssid = "Alejandro";
const char* password = "12345678";

WiFiClient wifiClient;
// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

int sensor = 0;      // variable del Sensor IR

int pinBuzzer = 1;   // PIN Buzzer

int pinSensor = 14;   // PIN Sensor

int pinLedR = 15;  // pin Rojo del led RGB
int pinLedV = 13;  // pin Verde del led RGB
int pinLedA = 12;   // pin Azul del led RGB

void verde()
{
 //Hacer color verde
  digitalWrite(pinLedR,0);
  digitalWrite(pinLedV,255);
  digitalWrite(pinLedA,0);
}

void azul()
{
 //Hacer color verde
  digitalWrite(pinLedR,0);
  digitalWrite(pinLedV,0);
  digitalWrite(pinLedA,255);
}

void rojo()
{
 //Hacer color verde
  digitalWrite(pinLedR,255);
  digitalWrite(pinLedV,0);
  digitalWrite(pinLedA,0);
}

void combi()
{
 //Hacer color verde
  digitalWrite(pinLedR,0);
  digitalWrite(pinLedV,255);
  digitalWrite(pinLedA,255);
}

void apagarleds()
{
  digitalWrite(pinLedR,0);
  digitalWrite(pinLedV,0);
  digitalWrite(pinLedA,0);
}



///// DECLARACION DE VARIABLES /////



int pausa = 1000;
int  cont ;
int  contador_de_objetos   ;
int  contador_lapso_de_tiempo ;
bool seguir_ejecutando ;
String estado_de_finalizacion ;
bool No_se_detecto_ninguno_en_5_segundos ;
int estado_sensor;
int estado_buzzer;
int estado_rgb;
int estado_display;

int cant = 0;

IPAddress IP;


void setup(){


  Serial.begin(115200);
  WiFi.begin(ssid,password);
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");  
  }
  Serial.println("Wifi Connected");
  Serial.println("IP: ");
  Serial.println(WiFi.localIP());

  WiFi.softAP(ssid, password);

  IP = WiFi.softAPIP();
  //Serial.print("AP IP address: ");
  //Serial.println(IP);
  
  server.on("/recolect_data", HTTP_GET, [](AsyncWebServerRequest *request)
  { 
    String x = GenerateAndSerializeJson(estado_sensor, estado_buzzer, estado_rgb, estado_display);
    request->send_P(200, "text/plain", x.c_str());
  });

  
  bool status;

  server.begin();
  Serial.begin(9600);
  pinMode(pinSensor, INPUT);   // pone el pinSensor como input
  pinMode(pinBuzzer , OUTPUT); // pone el pinBuzzer como output
  pinMode(pinLedR, OUTPUT);    // pone el pinLedR como output
  pinMode(pinLedV, OUTPUT);    // pone el pinLedV como output
  pinMode(pinLedA, OUTPUT);    // pone el pinLedA como output

  cont = 0;
  contador_de_objetos = 0;  
  contador_lapso_de_tiempo = 0;
  seguir_ejecutando = true;
  estado_de_finalizacion = "x";
  No_se_detecto_ninguno_en_5_segundos = false;

  
  setTime(21,0,0,4,12,2022);

}

 
void loop(){
 while(seguir_ejecutando == true)
  {
     sensor = digitalRead(pinSensor);
     Serial.println(sensor);
     while(sensor == 1)
      { 
        digitalWrite(pinBuzzer , HIGH);
        azul();
        estado_sensor = 0;
        sensor = digitalRead(pinSensor);
        Serial.println(sensor);
        estado_rgb = 2;
        delay(5);
        cont = cont + 1;
        if(cont == 2000)
        { 
          Serial.print("llego a 5 segundos");
          Serial.print("\n");
          seguir_ejecutando = false;
          estado_de_finalizacion = "Conteo Fallido";
          break;
        }               
      }
      estado_sensor = 1;
      combi();
      estado_rgb = 3;
      digitalWrite(pinBuzzer , LOW);
      estado_buzzer = 1;
      delay(200);
      //digitalWrite(pinBuzzer , LOW);
      estado_buzzer = 0;

      if(cont < 10000)
      { 
        cont = 0;
        Serial.print("reseteo contador:");
        Serial.print(cont);
        Serial.println("");


        contador_de_objetos = contador_de_objetos + 1;
        estado_display = contador_de_objetos;
        Serial.print("Contador de objetos:");
        Serial.print(contador_de_objetos);
        Serial.println("");
        
        if(contador_de_objetos == 9)
        { 
          estado_de_finalizacion = "ConteoExitoso";
          seguir_ejecutando = false;
        }
      }

   }
  
   if (estado_de_finalizacion == "ConteoExitoso")
   {
    digitalWrite(pinBuzzer , LOW);  
    verde();
    estado_rgb = 1;
    estado_display = 10;   
    Serial.print("Conteo Exitoso");
    Serial.print("\n");
    estado_rgb = 5;
    estado_sensor = 0;
   
    delay(3000); // Exito
    digitalWrite(pinBuzzer , HIGH);
    apagarleds();

    estado_buzzer = 0;
    estado_de_finalizacion = "Termino";

   }
   if (estado_de_finalizacion == "Conteo Fallido")
   {
      rojo();
      estado_rgb = 0;
      estado_display = 11;      
      Serial.print("Conteo Fallido");
      Serial.print("\n");
 
      estado_rgb = 5;
      estado_sensor = 0;
      estado_buzzer = 0;
      delay(3000);
      apagarleds();
      digitalWrite(pinBuzzer , HIGH);

      estado_de_finalizacion = "Termino";

   }
  //  apagarleds();
  //  estado_rgb = 5;
  //  estado_sensor = 0;
  //  digitalWrite(pinBuzzer , HIGH);
  //  estado_buzzer = 0;
   //delay(3000);

}

String GenerateAndSerializeJson(int sensor, int buzzer, int RGB, int display)
{ 

  StaticJsonDocument<300> doc;

  char json[1000];

  doc["controller_name"] = "NodeMCU esp8266";
  time_t fecha = now();

  String ahora=String(year(fecha)) + "-" + String(month(fecha)) + "-" + String(day(fecha))+ "T" + String(hour(fecha)) + ":" + String(minute(fecha)) + ":" + String(second(fecha)) + "Z";
  
  //Serial.println(ahora);

  doc["date"] = ahora;

  // Armando el array actuators

  JsonArray arrActuator = doc.createNestedArray("actuators"); 

  StaticJsonDocument<200> act;

  JsonObject actdisplay = act.to<JsonObject>();

  actdisplay["type"]="display";

  actdisplay["current_value"] = display; 

  arrActuator.add(actdisplay);

  JsonObject actledRGB = act.to<JsonObject>();

  actledRGB["type"]="ledRGB";

  actledRGB["current_value"] = RGB; 

  arrActuator.add(actledRGB);

  JsonObject actspeaker = act.to<JsonObject>();

  actspeaker["type"]="speaker";

  actspeaker["current_value"] = buzzer; 

  arrActuator.add(actspeaker);

  JsonArray arrSensor = doc.createNestedArray("sensors"); 
  StaticJsonDocument<300> sen; 

  JsonObject sensors = sen.to<JsonObject>(); 

  sensors["type"]="proximity";

  sensors["current_value"] = sensor; 

  arrSensor.add(sensors);


  serializeJsonPretty(doc, json);

  //Serial.print("Json: ");

  Serial.println(convertToString(json));

  //Wire.write(json, sizeof(json));
  int json_size = sizeof(json) / sizeof(char);

  return convertToString(json);
}



String convertToString(char* a)
{
    String s(a);
 
    return s;
}