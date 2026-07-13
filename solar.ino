#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "Ganesan_2.4G";
const char* password = "vkarthick";

WebServer server(80);

//----------------------
// CORS Helper
//----------------------

void enableCORS() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "*");
}

//----------------------
// Simulation Variables
//----------------------

unsigned long lastUpdate = 0;

String weather = "sunny";
bool motorOn = false;

float solarVoltage = 20.4;
float solarCurrent = 8.5;
float solarPower = 173.4;

float batteryVoltage = 52.5;
int batterySOC = 76;
float batteryTemp = 31.5;

bool charging = true;
String mode = "MPPT";

//----------------------
// Simulation Engine
//----------------------

void updateSimulation() {

  if (millis() - lastUpdate >= 30000 || lastUpdate == 0) {

    lastUpdate = millis();

    if (weather == "sunny") {

      solarVoltage = random(195,220)/10.0;
      solarCurrent = random(80,100)/10.0;

    }
    else if(weather == "cloudy"){

      solarVoltage = random(150,180)/10.0;
      solarCurrent = random(35,60)/10.0;

    }
    else{

      solarVoltage = random(100,130)/10.0;
      solarCurrent = random(5,20)/10.0;

    }

    solarPower = solarVoltage * solarCurrent;

    if(motorOn){

      batterySOC -= random(1,3);

    }
    else{

      if(charging && batterySOC < 100)
          batterySOC += random(1,2);

    }

    batterySOC = constrain(batterySOC,0,100);

    batteryVoltage = 48.0 + batterySOC * 0.06;

    batteryTemp = random(280,360)/10.0;

    if(solarPower < 30){

      charging = false;
      mode = "Standby";

    }
    else if(batterySOC > 95){

      charging = true;
      mode = "Float";

    }
    else{

      charging = true;
      mode = "MPPT";

    }

    Serial.println("\n========== UPDATED ==========");

    Serial.print("Weather          : ");
    Serial.println(weather);

    Serial.print("Solar Voltage    : ");
    Serial.println(solarVoltage);

    Serial.print("Solar Current    : ");
    Serial.println(solarCurrent);

    Serial.print("Solar Power      : ");
    Serial.println(solarPower);

    Serial.print("Battery Voltage  : ");
    Serial.println(batteryVoltage);

    Serial.print("Battery SOC      : ");
    Serial.println(batterySOC);

    Serial.print("Battery Temp     : ");
    Serial.println(batteryTemp);

    Serial.print("Charging         : ");
    Serial.println(charging);

    Serial.print("Motor            : ");
    Serial.println(motorOn);

    Serial.print("Mode             : ");
    Serial.println(mode);

    Serial.println("=============================\n");

  }

}

//----------------------
// API
//----------------------

void sendData(){

  updateSimulation();

  String json = "{";

  json += "\"solarVoltage\":"+String(solarVoltage,1)+",";
  json += "\"solarCurrent\":"+String(solarCurrent,1)+",";
  json += "\"solarPower\":"+String(solarPower,1)+",";

  json += "\"batteryVoltage\":"+String(batteryVoltage,1)+",";
  json += "\"batterySOC\":"+String(batterySOC)+",";
  json += "\"batteryTemp\":"+String(batteryTemp,1)+",";

  json += "\"charging\":";
  json += charging ? "true," : "false,";

  json += "\"motor\":";
  json += motorOn ? "true," : "false,";

  json += "\"mode\":\""+mode+"\"";

  json += "}";

  enableCORS();
  server.send(200,"application/json",json);

}

//----------------------
// Weather APIs
//----------------------

void sunny(){

  weather="sunny";
  enableCORS();
  server.send(200,"text/plain","Sunny Mode");

}

void cloudy(){

  weather="cloudy";
  enableCORS();
  server.send(200,"text/plain","Cloudy Mode");

}

void rainy(){

  weather="rainy";
  enableCORS();
  server.send(200,"text/plain","Rainy Mode");

}

//----------------------
// Motor APIs
//----------------------

void motorON(){

  motorOn=true;
  enableCORS();
  server.send(200,"text/plain","Motor ON");

}

void motorOFF(){

  motorOn=false;
  enableCORS();
  server.send(200,"text/plain","Motor OFF");

}

//----------------------
// Setup
//----------------------

void setup(){

  Serial.begin(115200);

  delay(1000);

  WiFi.begin(ssid,password);

  Serial.print("Connecting");

  while(WiFi.status()!=WL_CONNECTED){

    delay(500);
    Serial.print(".");

  }

  Serial.println();
  Serial.println("WiFi Connected!");

  Serial.print("IP Address : ");
  Serial.println(WiFi.localIP());

  server.on("/data", HTTP_GET, sendData);

  server.on("/sunny", HTTP_GET, sunny);

  server.on("/cloudy", HTTP_GET, cloudy);

  server.on("/rainy", HTTP_GET, rainy);

  server.on("/motor/on", HTTP_GET, motorON);

  server.on("/motor/off", HTTP_GET, motorOFF);

  // Handle browser preflight requests
  server.onNotFound([]() {
    if (server.method() == HTTP_OPTIONS) {
      enableCORS();
      server.send(200);
    } else {
      server.send(404, "text/plain", "Not Found");
    }
  });

  server.begin();

  Serial.println("HTTP Server Started");

}

//----------------------
// Loop
//----------------------

void loop(){

  server.handleClient();

  updateSimulation();

}