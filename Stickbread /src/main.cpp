#include <Arduino.h>
#include <Stepper.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// Data wire is connected to GPIO 4
#define ONE_WIRE_BUS 4

// Adjust to match stepper motor
const int stepsPerRevolution = 2048;

// Motor Driver Pins
#define IN1 26
#define IN2 25
#define IN3 33
#define IN4 32

// Initialize Stepper and Temperature Sensor
Stepper myStepper(stepsPerRevolution, IN1, IN3, IN2, IN4);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// Global variables
String temperatureC = "--";
String temperatureF = "--";
bool motorOn = false;
unsigned long lastTime = 0;
unsigned long timerDelay = 30000;

// Wi-Fi Credentials (REPLACE with your credentials)
const char* ssid = "ESP32 Test";
const char* password = "";

// Create AsyncWebServer object
AsyncWebServer server(80);

// HTML (Simplified and improved)
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<title>Motor Control</title>
<style>
body { font-family: sans-serif; text-align: center; margin-top: 50px; }
h1 { color: #333; }
button { margin: 10px; padding: 10px 20px; font-size: 18px; }
</style>
</head>
<body>
<h1>Motor Control</h1>
<p>Temperature (°C): <span id="tempC">--</span></p>
<p>Temperature (°F): <span id="tempF">--</span></p>
<button onclick="sendCommand('ON')">ON</button>
<button onclick="sendCommand('OFF')">OFF</button>

<script>
function sendCommand(cmd) {
  fetch('/', { method: 'POST', body: 'OnOff=' + cmd });
}
setInterval(updateTemp, 10000); // Update every 10 seconds

function updateTemp() {
  ['tempC', 'tempF'].forEach(tempType => {
    fetch(`/${tempType}`)
      .then(response => response.text())
      .then(data => document.getElementById(tempType).textContent = data);
  });
}
</script>
</body>
</html>
)rawliteral";

// Temperature reading functions
String readDSTemperatureC() {
  sensors.requestTemperatures();
  float tempC = sensors.getTempCByIndex(0);
  if (tempC == DEVICE_DISCONNECTED_C) {
    Serial.println("Error: Could not read temperature data (C)");
    return "--";
  }
  return String(tempC);
}

String readDSTemperatureF() {
  sensors.requestTemperatures();
  float tempF = sensors.getTempFByIndex(0);
  if (tempF == DEVICE_DISCONNECTED_F) {
    Serial.println("Error: Could not read temperature data (F)");
    return "--";
  }
  return String(tempF);
}

void connectWiFi() {
  WiFi.begin(ssid, password);
  unsigned long startAttemptTime = millis();

  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected to WiFi!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nFailed to connect to WiFi. Proceeding offline...");
  }
}

void setup() {
  Serial.begin(115200);
  sensors.begin();

  // Setup motor pins
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  // Connect to WiFi
  connectWiFi();

  // Define server routes
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html);
  });

  server.on("/", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("OnOff", true)) {
      String command = request->getParam("OnOff", true)->value();
      motorOn = (command == "ON");
    }
    request->send(200); // Send an OK response
  });

  server.on("/tempC", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/plain", temperatureC.c_str());
  });

  server.on("/tempF", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/plain", temperatureF.c_str());
  });

  // Start server
  server.begin();
}

void loop() {
  unsigned long currentMillis = millis();

  // Update temperature readings periodically
  if (currentMillis - lastTime >= timerDelay) {
    lastTime = currentMillis;
    temperatureC = readDSTemperatureC();
    temperatureF = readDSTemperatureF();
    Serial.println("Temperature updated.");
  }

  // Control motor behavior
  if (motorOn) {
    static bool direction = true; // Toggle direction
    if (direction) {
      myStepper.step(stepsPerRevolution / 4); // Quarter revolution forward
    } else {
      myStepper.step(-stepsPerRevolution / 4); // Quarter revolution backward
    }
    direction = !direction; // Change direction
    delay(1000); // Adjust motor speed
  }
}
