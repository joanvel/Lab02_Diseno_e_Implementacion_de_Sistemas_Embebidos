# Control de motor y monitoreo ambiental vía MQTT (ESP32 + HiveMQ + Node-RED)
Este módulo implementa un sistema de comunicación bidireccional mediante MQTT entre un ESP32 y servicios externos, permitiendo:
* Publicar datos de sensores (DHT22) hacia un broker MQTT seguro (HiveMQ Cloud)
* Recibir comandos de control (RPM) para un motor DC desde el mismo broker
* Visualización y control remoto mediante Node-RED, usando un dashboard local
## Estructura del proyecto
```text
/ex2
│
├── Demonstration/
|   └── video.mp4    // Video demostración del sistema
├── node-red/        // Carpeta con el proyecto en node-red
|   └── flows.json    // Proyecto en node-red
├── ex2.ino     // Archivo principal con FSM + WiFi + servidor web
└── README.md
```
## Conectividad y seguridad
* El ESP32 se conecta a una red WiFi local.
* Se establece una conexión MQTT segura (TLS) con HiveMQ Cloud usando:
  * Puerto 8883
  * Usuario y contraseña
* Para simplificar el manejo de certificados en el ESP32, se utiliza:
  ```cpp
  espClient.setInsecure();
  ```
## Comunicación MQTT
### Tópicos utilizados
| Tipo | Tópico |
|------|--------|
| Publicación | esp32/sensors/dht22 |
| Suscripción | esp32/control/rpm |

## Publicación de sensores
Cada 5 segundos, el ESP32:
1. Lee temperatura y humedad desde el DHT22
2. Construye un mensaje JSON:
   ```json
   {
      "temperature": 24.3,
      "humidity": 58.1,
      "unit": {
        "temperature": "C",
        "humidity": "%"
      }
    }
    ```
3. Publica el mensaje en:
   * esp32/sensors/dht22
Esto permite que Node-RED, u otro cliente MQTT, visualice los datos en tiempo real.
## Control remoto del motor (RPM)
El ESP32 se suscribe al topic:
* esp32/control/rpm
Cuando recibe un mensaje:
1. El payload se interpreta como JSON:
   ```json
   {
      "rpm": 120
   }
   ```
2. El valor de rpm se convierte a una señal PWM proporcional:
   ```cpp
   value = rpm * 255 / 400;
   ```
3. Dependiendo del signo del RPM:
   * RPM positivo → giro horario
   * RPM negativo → giro antihorario
4. Se actualizan los pines:
   * PWM_PIN
   * WISE_PIN
   * ANTIWISE_PIN
Esto permite controlar velocidad y sentido de giro del motor de forma remota.
## Integración con Node-RED
Para facilitar el uso del sistema:
* Se desarrolló una interfaz gráfica en Node-RED que permite:
  * Visualizar temperatura y humedad
  * Controlar el RPM del motor mediante un slider
* En el repositorio se incluye el archivo:
  * flow.json
* Cualquier usuario puede:
  1. Importar el flow en Node-RED
  2. Conectar su propio broker o usar HiveMQ
  3. Controlar el ESP32 desde su PC sin modificar el firmware

### La carpeta Demonstration contiene un video del funcionamiento del sistema

Para mayor información revisar el reporte técnico en la raíz del repositorio.
