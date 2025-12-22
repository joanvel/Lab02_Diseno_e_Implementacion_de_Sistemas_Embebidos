#include "DHTesp.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <WebServer.h>
#include <time.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3c
#define report_period 2000

// ====== CONFIGURACIÓN WiFi ======
const char* ssid = "VL25";
const char* password = "12345678";

// ==============================
// CONFIG NTP (para timestamp)
// ==============================
const char* ntpServer = "pool.ntp.org";   // ⬅️ CAMBIO
const long gmtOffset_sec = -18000;        // Colombia UTC-5  ⬅️ CAMBIO
const int daylightOffset_sec = 0;         // ⬅️ CAMBIO
String lastUpdateTimestamp = "Never";    // ⬅️ CAMBIO

float temp;

const int DHT_PIN = 15;

DHTesp dhtSensor;

int last_report = millis();

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ====== SERVIDOR ======
WebServer server(80);

// ====== HTML PRINCIPAL (se envía al cliente) ======
const char MAIN_page[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <title>ESP32 Monitor Ambiental</title>
  <style>
    body { font-family: Arial; text-align: center; margin-top: 40px; }
    .card { background: #eee; padding: 20px; width: 300px; margin: auto; border-radius: 10px; }
    h2 { margin-bottom: 10px; }
    .value { font-size: 2em; }
  </style>
</head>
<body>
  <div class="card">
    <h2>Monitor Ambiental</h2>
    <p><strong>Temperatura:</strong> <span id="temp" class="value">--</span> °C</p>
    <p><strong>Humedad:</strong> <span id="hum" class="value">--</span> %</p>
    <p><strong>Última actualización:</strong> <span id="ts">--</span></p>

  </div>

  <script>
    function fetchData() {
      fetch('/data')
        .then(response => response.json())
        .then(data => {
          document.getElementById('temp').textContent = data.temperature;
          document.getElementById('hum').textContent = data.humidity;
          document.getElementById('ts').textContent = data.timestamp;
        });
    }


    setInterval(fetchData, 2000);  // refresco cada 2 segundos
    fetchData();
  </script>
</body>
</html>
)rawliteral";

// ===========================================================
// HANDLERS
// ===========================================================

void handleRoot() {
  server.send_P(200, "text/html", MAIN_page);
}

void handleData() {
  TempAndHumidity data = dhtSensor.getTempAndHumidity();

  // Just for testing without sensor
  // data.temperature = 1.0;
  // data.humidity = 0.0;

  // ========== ⬅️ OBTENER TIMESTAMP NTP ==========
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    char buffer[30];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);
    lastUpdateTimestamp = String(buffer);
  } else {
    lastUpdateTimestamp = "NTP error";
  }
  // ===============================================

  // Crear JSON
  String json = "{";
  json += "\"temperature\":" + String(data.temperature, 1) + ",";
  json += "\"humidity\":" + String(data.humidity, 1) + ",";
  //json += "\"humidity\":\"" + String(data.humidity, 1) + "\"";
  json += "\"timestamp\":\"" + lastUpdateTimestamp + "\"";
  json += "}";

  server.send(200, "application/json", json);
}



void setup() {
  Wire.begin(21, 22);
  Serial.begin(115200);
  dhtSensor.setup(DHT_PIN, DHTesp::DHT11);

  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  WiFi.begin(ssid, password);
  Serial.print("Conectando a WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConectado!");
  Serial.println(WiFi.localIP()); // Dirección del servidor

  // ============ ⬅️ INICIAR NTP PARA TIMESTAMP ============
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  // ========================================================

  server.on("/", handleRoot);
  server.on("/data", handleData);

  server.begin();
  Serial.println("Servidor iniciado en el puerto 80");


}

void loop() {
  if(millis() - last_report >= report_period)
  {
    display.clearDisplay();
    display.setCursor(0, 0);
    last_report = millis();
    TempAndHumidity  data = dhtSensor.getTempAndHumidity();
    float val = analogRead(34)*80.0/4095.0;
    temp = analogRead(34)*80.0/4095.0;

    Serial.println("Temp: " + String(data.temperature, 1) + "°C");
    display.println("Temp: " + String(data.temperature, 1) + "°C");
    Serial.println("Humidity: " + String(data.humidity, 1) + "%");
    display.println("Humidity: " + String(data.humidity, 1) + "%");
    Serial.println("---");
    display.println("---");
    Serial.println("Temp LM75: " + String(val, 2) + "°C");
    display.println("Temp LM75: " + String(val, 2) + "°C");
    display.display();
  }
  server.handleClient();
}