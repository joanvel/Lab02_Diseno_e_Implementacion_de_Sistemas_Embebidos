#include "Motor.h"
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <WebServer.h>
#include "time.h"   // ⬅️ CAMBIO

#define SAMPLE_PERIOD 50
#define REPORT_PERIOD 500
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3c

enum state_t {
    IDLE,
    SAMPLE,
    REPORT,
    SETPOINT,
    STOP
};

state_t state = IDLE;

Motor motor(1.0f, 0.1f); // Ajustar Kp y Ki según sea necesario
unsigned long last_report = 0;
unsigned long last_sample = 0;
float currentRPM;   // Valor de RPM actual


Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ==============================
// CONFIG WiFi
// ==============================
const char* ssid = "VL25";
const char* password = "12345678";

// ==============================
// CONFIG NTP (para timestamp)
// ==============================
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -18000;
const int daylightOffset_sec = 0;

// ==============================
// SERVIDOR WEB
// ==============================
WebServer server(80);

// Variables globales
int motorSetpointRPM = 0;
String lastUpdateTimestamp = "Never";

// ==============================
// PÁGINA WEB (igual que antes)
// ==============================
const char MAIN_page[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <title>Control de Motor - ESP32</title>
  <style>
    body { font-family: Arial; text-align: center; margin-top: 40px; }
    .card { background: #f0f0f0; padding: 20px; width: 300px; margin: auto; border-radius: 10px; }
  </style>
</head>
<body>
  <h2>Control de Velocidad del Motor</h2>

  <div class="card">
    <p><strong>Setpoint (RPM):</strong></p>
    <input id="rpmInput" type="number" min="0" max="5000" value="1000">
    <br><br>
    <button onclick="sendRPM()">Enviar</button>

    <hr>
    <p><strong>RPM actual:</strong> <span id="rpmValue">0</span></p>
    <p><strong>Última actualización:</strong> <span id="timestamp">Never</span></p>
  </div>

  <script>
    function sendRPM() {
      let rpm = document.getElementById("rpmInput").value;

      fetch('/set_rpm', {
        method: 'POST',
        headers: { "Content-Type": "text/plain" },
        body: rpm
      });
    }

    // Actualizar UI cada 1 segundo
    setInterval(() => {
      fetch('/status')
        .then(response => response.json())
        .then(data => {
          document.getElementById("rpmValue").innerText = data.rpm;
          document.getElementById("timestamp").innerText = data.timestamp;
        });
    }, 1000);
  </script>
</body>
</html>
)rawliteral";

// ==============================
// HANDLER: Página principal
// ==============================
void handleRoot() {
  server.send_P(200, "text/html", MAIN_page);
}

// ==============================
// HANDLER: Recibir RPM + TIMESTAMP
// ==============================
void handleSetRPM() {
  if (server.hasArg("plain")) {
    motorSetpointRPM = server.arg("plain").toInt();

    Serial.print("Nuevo setpoint: ");
    Serial.print(motorSetpointRPM);
    Serial.print(" | Timestamp: ");
    Serial.println(lastUpdateTimestamp);

    server.send(200, "text/plain", "Setpoint recibido");
  } 
  else {
    server.send(400, "text/plain", "No se recibió ningún valor");
  }
}

void handleStatus() {

  // ========== OBTENER TIMESTAMP NTP ==========
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    char buffer[30];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);
    lastUpdateTimestamp = String(buffer);
  } else {
    lastUpdateTimestamp = "NTP error";
  }
  // ===============================================

  String json = "{";
  json += "\"rpm\":" + String(currentRPM) + ",";
  json += "\"timestamp\":\"" + lastUpdateTimestamp + "\"";
  json += "}";

  server.send(200, "application/json", json);
}


// ==============================
// SETUP
// ==============================
void setup() {
    Wire.begin(21, 22);
    Serial.begin(115200);

    pinMode(ENC_A, INPUT_PULLUP);
    pinMode(ENC_B, INPUT_PULLUP);
    pinMode(POS_PIN, OUTPUT);
    pinMode(NEG_PIN, OUTPUT);
    pinMode(PWM_PIN, OUTPUT);

    pcnt_config_t pcnt_config{};
    pcnt_config.pulse_gpio_num = ENC_A;
    pcnt_config.ctrl_gpio_num  = ENC_B;
    pcnt_config.channel        = PCNT_CHANNEL_0;
    pcnt_config.unit           = PCNT_UNIT;
    pcnt_config.pos_mode       = PCNT_COUNT_INC;
    pcnt_config.neg_mode       = PCNT_COUNT_DEC;
    pcnt_config.lctrl_mode     = PCNT_MODE_REVERSE;
    pcnt_config.hctrl_mode     = PCNT_MODE_KEEP;
    pcnt_config.counter_h_lim  = 32767;
    pcnt_config.counter_l_lim  = -32768;

    pcnt_unit_config(&pcnt_config);

    pcnt_set_filter_value(PCNT_UNIT, 200);  // filtra rebotes
    pcnt_filter_enable(PCNT_UNIT);

    pcnt_counter_clear(PCNT_UNIT);
    pcnt_counter_resume(PCNT_UNIT);

    display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    Serial.println("Conectando a WiFi...");
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(300);
        Serial.print(".");
    }

    Serial.println("\nConectado!");
    Serial.print("IP del servidor: ");
    Serial.println(WiFi.localIP());

    // ============ INICIAR NTP PARA TIMESTAMP ============
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    // ========================================================

    // Rutas HTTP
    server.on("/", handleRoot);
    server.on("/set_rpm", HTTP_POST, handleSetRPM);
    server.on("/status", handleStatus);


    server.begin();
    Serial.println("Servidor iniciado en puerto 80.");
}

// ==============================
// LOOP
// ==============================
void loop() {
    float controlSignal;
    float sp;

    // Finite state machine for motor control
    switch (state) {
        case IDLE:
            // Esperar comando para iniciar muestreo
            if (Serial.available() > 0) {
                char c = Serial.read();
                if (c == 's') { // Comando para iniciar muestreo
                    state = SETPOINT;
                } else if (c == 'x'){
                    state = STOP;
                }
            } else if(millis() - last_sample > SAMPLE_PERIOD) {
                state = SAMPLE;
            } else if(millis() - last_report > REPORT_PERIOD) {
                state = REPORT;
            }
            break;
        case SAMPLE:
            // Muestrear RPM y calcular señal de control
            currentRPM = motor.getRPM(last_sample);
            motor.getError();

            motor.computeControlSignal();
            state = IDLE;
            last_sample = millis();
            break;
        case REPORT:
            // Reportar datos por Serial y pantalla OLED
            controlSignal = motor.getControlSignal();
            display.clearDisplay();
            display.setCursor(0, 0);
            Serial.print("RPM: ");
            Serial.print(currentRPM);
            Serial.print(" | Set point: ");
            Serial.print(motor.getSetpoint());
            Serial.print(" | Error: ");
            Serial.println(motor.getError());
            Serial.print("Control Signal: ");
            Serial.println(controlSignal);
            display.print("RPM: ");
            display.println(currentRPM);
            display.print("Set point: ");
            display.println(motor.getSetpoint());
            display.print("Err: ");
            display.println(motor.getError());
            display.display();
            state = IDLE;
            last_report = millis();
            break;
        case SETPOINT:
            // Esperar comando para cambiar setpoint o detener
            while (Serial.available() == 0); // Esperar hasta que haya datos
            sp = Serial.parseFloat();
            motorSetpointRPM = static_cast<int>(sp);
            motor.setSetpoint(sp);
            state = IDLE;
            break;

        case STOP:
            // Detener motor y limpiar estados
            motor.setRPMDirect(0);
            motor.setSetpoint(0);
            motor.clearIntegral();
            motorSetpointRPM = 0;
            state = IDLE;
            break;
    }

    motor.setSetpoint(static_cast<float>(motorSetpointRPM));
    server.handleClient();
}
