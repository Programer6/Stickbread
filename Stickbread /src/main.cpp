#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Stepper.h>

// Stepper Motor Configuration
const int stepsPerRevolution = 2048;
#define IN1 26  
#define IN2 25  
#define IN3 33  
#define IN4 32  

Stepper myStepper(stepsPerRevolution, IN1, IN3, IN2, IN4);

// Temperature Sensor Configuration
#define ONE_WIRE_BUS 4
OneWire oneWire(ONE_WIRE_BUS);  
DallasTemperature sensors(&oneWire);

// WiFi Credentials
const char* ssid = "ESP_B7BAF5";
const char* password = "";

// Web Server Setup
WebServer server(80);

// Global Variables
String temperatureC = "--";
String temperatureF = "--";
bool motorOn = false;      



void handleRoot() {
  String html = R"rawliteral(
    <!DOCTYPE html>
    <html lang="en">
    <head>
      <meta charset="UTF-8">
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
      <title>Motor Control</title>
      <style>
        body {
          font-family: Arial, sans-serif;
          text-align: center;
          background-color: #f4f4f4;
          color: #333;
          margin: 0;
          padding: 0;
        }
        h1 {
          background-color: #007BFF;
          color: white;
          padding: 15px;
          margin: 0;
        }
        .container {
          padding: 20px;
        }
        .temp-box {
          display: inline-block;
          background: white;
          padding: 20px;
          border-radius: 10px;
          box-shadow: 0px 4px 6px rgba(0, 0, 0, 0.1);
          margin: 10px;
          font-size: 24px;
        }
        button {
          background-color: #28a745;
          color: white;
          padding: 10px 20px;
          font-size: 18px;
          border: none;
          border-radius: 5px;
          cursor: pointer;
          margin: 10px;
        }
        button.off {
          background-color: #dc3545;
        }
        button:hover {
          opacity: 0.8;
        }
      </style>
    </head>
    <body>
      <h1>Motor Control & Temperature Monitoring</h1>
      <div class="container">
        <div class="temp-box">
          <p>Temperature (°C): <span id="tempC">--</span></p>
        </div>
        <div class="temp-box">
          <p>Temperature (°F): <span id="tempF">--</span></p>
        </div>
        <br>
        <button onclick="sendCommand('ON')">Turn Motor ON</button>
        <button class="off" onclick="sendCommand('OFF')">Turn Motor OFF</button>
      </div>
      <script>
        function sendCommand(command) {
          fetch('/motor', { method: 'POST', body: 'state=' + command });
        }
        setInterval(() => {
          fetch('/tempC').then(res => res.text()).then(data => document.getElementById('tempC').innerText = data);
          fetch('/tempF').then(res => res.text()).then(data => document.getElementById('tempF').innerText = data);
        }, 2000);
      </script>
    </body>
    </html>
  )rawliteral";
  server.send(200, "text/html", html);
}

void setup() {
  Serial.begin(115200);

  // Initialize Stepper
  myStepper.setSpeed(5);

  // Initialize Temperature Sensor
  sensors.begin();

  // Connect to WiFi
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  

  // Define Routes
  server.on("/", handleRoot);        
  server.on("/tempC", HTTP_GET, []() {
    server.send(200, "text/plain", temperatureC);
  });
  server.on("/tempF", HTTP_GET, []() {
    server.send(200, "text/plain", temperatureF);
  });
  server.on("/motor", HTTP_POST, handleMotor);

  server.begin();
  Serial.println("HTTP Server Started");
}

void loop() {
  server.handleClient();

  // Read temperature instantly
  String newTempC = readDSTemperatureC();
  String newTempF = readDSTemperatureF();

  // Update only if the value changes
  if (newTempC != temperatureC || newTempF != temperatureF) {
    temperatureC = newTempC;
    temperatureF = newTempF;
    Serial.println("Temperature updated: " + temperatureC + "°C / " + temperatureF + "°F");
  }

  // Handle motor operation
  if (motorOn) {  
    myStepper.step(stepsPerRevolution);
    delay(1000);                            
    myStepper.step(-stepsPerRevolution);
  }
}

void handleMotor() {
  if (server.hasArg("plain")) {
    String state = server.arg("plain");
    if (state == "state=ON") {
      motorOn = true;    
    } else if (state == "state=OFF") {
      motorOn = false;  
    }
  }
  server.send(200, "text/plain", "OK");
}

String readDSTemperatureC() {
  sensors.requestTemperatures();      
  float tempC = sensors.getTempCByIndex(0);
  return (tempC == DEVICE_DISCONNECTED_C) ? "--" : String(tempC);
}

String readDSTemperatureF() {
  sensors.requestTemperatures();      
  float tempF = sensors.getTempFByIndex(0);
  return (tempF == DEVICE_DISCONNECTED_F) ? "--" : String(tempF);
}

