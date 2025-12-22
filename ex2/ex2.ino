#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <DHT.h>

// ======= CONFIGURACIÓN WIFI =======
const char* ssid = "VL25";
const char* password = "12345678";

// ======= CONFIGURACIÓN MQTT =======
const char* mqtt_server = "89e7d56a6e334cb5846a7e9442890433.s1.eu.hivemq.cloud";
const int mqtt_port = 8883;
const char* mqtt_user = "Test_user";
const char* mqtt_pass = "Co123456";

// ======= TOPICS =======
const char* topic_pub = "esp32/sensors/dht22";
const char* topic_sub = "esp32/control/rpm";

// ======= DHT =======
#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// ======= MOTOR =======
#define PWM_PIN 19
#define WISE_PIN 18
#define ANTIWISE_PIN 5
float rpm;

// ======= OBJETOS =======
WiFiClientSecure espClient;
PubSubClient client(espClient);

unsigned long lastMsg = 0;

// ======= CALLBACK MQTT =======
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensaje recibido [");
  Serial.print(topic);
  Serial.print("]: ");

  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println(message);

  // Parse JSON
  StaticJsonDocument<200> doc;
  deserializeJson(doc, message);
  rpm = doc["rpm"];
  int value = rpm*255/400;
  if(rpm < 0){
    analogWrite(PWM_PIN, -value);
    digitalWrite(WISE_PIN, LOW);
    digitalWrite(ANTIWISE_PIN, HIGH);
  }else{
    analogWrite(PWM_PIN, value);
    digitalWrite(WISE_PIN, HIGH);
    digitalWrite(ANTIWISE_PIN, LOW);
  }

  Serial.print("Setpoint RPM recibido: ");
  Serial.println(rpm);
}

// ======= CONEXIÓN MQTT =======
void reconnect() {
  while (!client.connected()) {
    Serial.print("Conectando MQTT...");
    if (client.connect("ESP32Client", mqtt_user, mqtt_pass)) {
      Serial.println(" conectado");
      client.subscribe(topic_sub, 1);  // QoS 1
    } else {
      Serial.print(" fallo, rc=");
      Serial.println(client.state());
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  dht.begin();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  espClient.setInsecure(); // Simplifica TLS
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  pinMode(PWM_PIN, OUTPUT);
  pinMode(WISE_PIN, OUTPUT);
  pinMode(ANTIWISE_PIN, OUTPUT);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;

    float t = dht.readTemperature();
    float h = dht.readHumidity();

    StaticJsonDocument<200> doc;
    doc["temperature"] = t;
    doc["humidity"] = h;
    doc["unit"]["temperature"] = "C";
    doc["unit"]["humidity"] = "%";

    char buffer[256];
    serializeJson(doc, buffer);

    client.publish(topic_pub, buffer, true);
    Serial.println("Datos publicados");
  }
}
