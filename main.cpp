

/*

    Author: Norel Farjun
    All rights reserved

*/

#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>
#include <ClosedCube_HDC1080.h>
#include <ESPAsyncWebServer.h>
ClosedCube_HDC1080 hdc1080;

#define NETWORK_NAME "ssid"
#define NETWOK_PASSWORD "password"
#define NETWORK_TIMEOUT_MS 200000

/* pins */ 
#define PUMP 18
#define LIGHT 19
#define FAN 27
#define HEAT_SOURCE 23
// SDA: 21
// SLC: 22

/* variables */
bool IS_PUMP_ON = false;
unsigned long currentStartWateringTime = millis();

AsyncWebServer server(80);
int priority = 3;
String lightMode = "off"; 
String pumpMode = "off"; 
String heaterMode = "off"; 
String fanMode = "off"; 

/* control methodes*/

void heaterOn(){
  digitalWrite(HEAT_SOURCE, HIGH); 
}
void heaterOff(){
  digitalWrite(HEAT_SOURCE,LOW);
}
void lightOn(){
  digitalWrite(LIGHT, HIGH);
} 
void lightOff(){
  digitalWrite(LIGHT, LOW);
}
void waterOn(){
  IS_PUMP_ON = true;
  digitalWrite(PUMP, HIGH);
  currentStartWateringTime = millis();
}
void waterOff(){
  digitalWrite(PUMP, LOW);
  IS_PUMP_ON = false;
}
void fanOn(){
  digitalWrite(FAN, HIGH);
}
void fanOff(){
  digitalWrite(FAN, LOW);
}

/* get data methods */

String readTemperature() {
  return String(hdc1080.readTemperature());
}

String readHumidity() {
  return String(hdc1080.readHumidity());
}



void waterControl(void * param){
  for(;;){
    if(IS_PUMP_ON){
      if(millis() - currentStartWateringTime > 300000){ // 5 minutes
        IS_PUMP_ON = false;
        vTaskDelay(7200000/portTICK_PERIOD_MS); // delay for 2 hours
        currentStartWateringTime = millis();
        IS_PUMP_ON = true;
      }   
    }
    vTaskDelay(10);
  }
}
void humidityAndTemperatureControl(void * params){
  double temp = 0.0;
  double humidity = 0.0;

  for(;;){
    temp = hdc1080.readTemperature();
    humidity = hdc1080.readHumidity();
    if(temp < 25 ){
      heaterOn();
    } 
    else if(temp > 35){
      heaterOff();
    } 
    if(humidity > 70){
      fanOn();
    } 
    else if(humidity < 50){
      fanOff();
    }
    vTaskDelay(10);
  }
}

void networkContinuousConnection(void * params){
  for(;;){
    if(WiFi.status() == WL_CONNECTED){ // network is ok 
      priority = 1;
      Serial.println("still connected");
      vTaskDelay(10000 / portTICK_PERIOD_MS);
      continue;
    }
    // else, if network is not connected
    digitalWrite(2, HIGH);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    digitalWrite(2, LOW);
    vTaskDelay(500 / portTICK_PERIOD_MS);
    digitalWrite(2, HIGH);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    digitalWrite(2, LOW);
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(NETWORK_NAME, NETWOK_PASSWORD);

    unsigned long startAttemptTime = millis();
    /*wait for the network connection or time to run out*/
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < NETWORK_TIMEOUT_MS){
      Serial.print(".");
    } 
    Serial.println();
    Serial.println("Network Connected: ");
    Serial.println(WiFi.localIP());

    server.begin();
    
    if(WiFi.status() != WL_CONNECTED){ // if failed to connect to network
    Serial.print("Failed to connect");
      vTaskDelay(20000 / portTICK_PERIOD_MS);
      continue;
    }  
  }
}

void setup() {
  
  hdc1080.begin(0x40);
  Serial.begin(9600);
  pinMode(2, OUTPUT);
  pinMode(PUMP, OUTPUT);
  pinMode(LIGHT, OUTPUT);
  pinMode(FAN, OUTPUT);
  pinMode(HEAT_SOURCE, OUTPUT);
  lightOff();
  waterOff();
  heaterOff();
  fanOff();


                          /* server listeners */

/*GET*/
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readTemperature().c_str());
  });
  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readHumidity().c_str());
  });
    server.on("/lightMode", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", lightMode.c_str());
  });
    server.on("/pumpMode", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", pumpMode.c_str());
  });
    server.on("/heaterMode", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", heaterMode.c_str());
  });
    server.on("/fanMode", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", fanMode.c_str());
  });

  /*POST*/
  server.on("/light/on", HTTP_POST, [](AsyncWebServerRequest *request){
    lightOn();
    lightMode = "on";
    request->send_P(200, "text/plain", lightMode.c_str());
  });
  server.on("/light/off", HTTP_POST, [](AsyncWebServerRequest *request){
    lightOff();
    lightMode = "off";
    request->send_P(200, "text/plain", lightMode.c_str());
  });
    server.on("/pump/on", HTTP_POST, [](AsyncWebServerRequest *request){
      waterOn();
      pumpMode = "on";
    request->send_P(200, "text/plain", pumpMode.c_str());
  });
  server.on("/pump/off", HTTP_POST, [](AsyncWebServerRequest *request){
    waterOff();
    pumpMode = "off";
    request->send_P(200, "text/plain", pumpMode.c_str());
  });
  server.on("/fan/on", HTTP_POST, [](AsyncWebServerRequest *request){
    fanOn();
    fanMode = "on";
    request->send_P(200, "text/plain", fanMode.c_str());
  });
  server.on("/fan/off", HTTP_POST, [](AsyncWebServerRequest *request){
    fanOff();
    fanMode = "off";
    request->send_P(200, "text/plain", fanMode.c_str());
  });
  server.on("/heater/on", HTTP_POST, [](AsyncWebServerRequest *request){
    heaterOn();
    heaterMode = "on";
    request->send_P(200, "text/plain", heaterMode.c_str());
  });
  server.on("/heater/off", HTTP_POST, [](AsyncWebServerRequest *request){
    heaterOff();
    heaterMode = "off";
    request->send_P(200, "text/plain", heaterMode.c_str());
  });

  /*RTOS process*/
  xTaskCreatePinnedToCore(
    networkContinuousConnection,
    "network connection task",
    5000,
    NULL,
    priority,
    NULL,
    CONFIG_ARDUINO_RUNNING_CORE
  );
  xTaskCreate(
    humidityAndTemperatureControl,
    "humidityAndTemperatureControl",
    2000,
    NULL,
    2,
    NULL
  );
  xTaskCreate(
    waterControl,
    "waterControl",
    1000,
    NULL,
    3,
    NULL
  );
  
}





void loop() {}


