#include <Wire.h>
#include <time.h>
#include <ArduinoJson.h>

const int sensor = 2;
const int coil = 4;
const int buzzer = 7;

const char ASK_FOR_LENGTH = 'L';
const char ASK_FOR_DATA = 'D';
String message; 

char request = ' ';
char requestIndex = 0;

void setup() {
  Wire.begin(4);                /* join i2c bus with address 4 */
  // Wire.onReceive(receiveEvent); /* register receive event */
  // Wire.onRequest(requestEvent); /* register request event */
  Serial.begin(9600);           /* start serial for debug */
  pinMode(sensor, INPUT);  // Sensor
  pinMode(coil, OUTPUT); // Coil
  pinMode(buzzer, OUTPUT); // Buzzer
  digitalWrite(sensor, HIGH);

  Wire.onRequest(requestEvent);
  Wire.onReceive(receiveEvent);

}

void loop() {
  message = ""; 
  StaticJsonDocument<136> doc; 
  StaticJsonDocument<52> actuator1;
  StaticJsonDocument<52> actuator2;
  StaticJsonDocument<52> sensor1;
  actuator1["type"]="speaker";
  actuator2["type"]="rele";
  sensor1["type"]= "position_sensor";
  JsonArray arrActuators = doc.createNestedArray("actuators");
  JsonArray arrSensors = doc.createNestedArray("sensors");
  
  arrActuators.add(actuator1);
  arrActuators.add(actuator2);
  arrSensors.add(sensor1);
  

  int sensor_state = digitalRead(sensor);
  if(sensor_state == 0){
    digitalWrite(coil, HIGH);
    
    digitalWrite(buzzer, HIGH);
    delay(500);
    digitalWrite(buzzer, LOW);
    delay(500);

    actuator1["current_value"] = 1;
    actuator2["current_value"] = 1;
    sensor1["current_value"] = 0;
  }
  if(sensor_state == 1){
    digitalWrite(coil, LOW);

    actuator1["current_value"] = 0;
    actuator2["current_value"] = 0;
    sensor1["current_value"] = 1;
  }
  doc["controller_name"]="Arduino-Uno";
  // Convert Json to String
  serializeJson(doc, message);
  Serial.println(message); 
  Serial.println();
  
}


// function that executes whenever data is received from master
void receiveEvent(int howMany) {
 while (0 < Wire.available()) {
    request = Wire.read();      /* receive byte as a character */
    Serial.print(request);           /* print the character */
  }
  Serial.println();             /* to newline */
}

// function that executes whenever data is requested from master
void requestEvent()
{
  if(request == ASK_FOR_LENGTH)
  {
    Wire.write(message.length());
    Serial.println(request);
    Serial.println(message.length());    
    char requestIndex = 0;
  }
  if(request == ASK_FOR_DATA)
  {
    if(requestIndex < (message.length() / 32)) 
    {
      Wire.write(message.c_str() + requestIndex * 32, 32);
      requestIndex ++;
      Serial.println(requestIndex); 
    }
    else
    {
      Wire.write(message.c_str() + requestIndex * 32, (message.length() % 32));
      requestIndex = 0;
    }
  }
}