#include <Stepper.h>
#include <Wire.h>
#include <ArduinoJson.h>

int stepsPerRevolution = 2048; // Steps to make a revolution
Stepper myStepper(stepsPerRevolution, 8,10,9,11); // 8,10,9,11 are the pins where the stepper is allocated

int steps = 192;
// Stepper Motor speed
int motSpeed = 3; 

const byte I2C_SLAVE_ADDR = 0x07;

const char ASK_FOR_LENGTH = 'L';
const char ASK_FOR_DATA = 'D';
int yValue;
int interruptorValue;
char request = ' ';
char requestIndex = 0;
String message; 

void setup() {
    Serial.begin(115200);
    myStepper.setSpeed(motSpeed);
    Wire.begin(I2C_SLAVE_ADDR);
    Wire.onRequest(requestEvent);
    Wire.onReceive(receiveEvent);
}

void receiveEvent(int bytes)
{
  while (Wire.available())
  {
   request = (char)Wire.read();
  }
}

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

void loop() {
    // deberia conectar el joystick a A0 y A1
    //Reset the message to Master
    message="";
    runStepperByProximity();
    delay(500);
}

void runStepperByProximity()
{   
    // Define the Json Structure
    StaticJsonDocument<136> doc; 
    StaticJsonDocument<52> actuator1;
    StaticJsonDocument<52> sensor1;
    StaticJsonDocument<52> sensor2;
    JsonArray arrActuators = doc.createNestedArray("actuators");
    JsonArray arrSensors = doc.createNestedArray("sensors");
    

    yValue = analogRead(A1);
    interruptorValue = digitalRead(7);
    int outputValue = 0;
    if (interruptorValue == 0){
      if (yValue > 600) {
        stepper_fwd();
        actuator1["current_value"]=1;
        Serial.println("Dirección: Arriba");
      } 
    } else {
      //parpadeo(0.5);
      actuator1["current_value"]=0;
      Serial.println("Detención por fin de rango de movimiento");
    }
    if (yValue < 500) {
        stepper_back();
        actuator1["current_value"]=-1;
        
        Serial.print(yValue);
        Serial.println("Dirección: Abajo");
    } 
    
    Serial.print("Proximity: ");
    Serial.println(interruptorValue);

    // Fill the json according to the group interoperability standard
    actuator1["type"]="stepper";
    sensor1["type"]="fotosensor";
    sensor1["current_value"]=interruptorValue;
    sensor2["type"]="joystick";
    sensor2["current_value"] = yValue;
    arrActuators.add(actuator1);
    arrSensors.add(sensor1);
    arrSensors.add(sensor2);
    
    doc["controller_name"]="Arduino-nano-7";

    // Convert Json to String
    serializeJson(doc, message);
    
    Serial.println(message.length());
}

void stepper_fwd() {
  myStepper.step(steps); 
}

void stepper_back() {
  myStepper.step(-steps);
}