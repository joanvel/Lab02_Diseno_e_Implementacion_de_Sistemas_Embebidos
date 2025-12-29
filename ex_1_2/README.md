# Control de Velocidad de Motor DC vía Web con ESP32
## Descripción del proyecto
Este proyecto extiende el control de velocidad de un motor DC con encoder y control PI incorporando conectividad WiFi y una interfaz web para:
* Ajustar el setpoint de RPM de forma remota.
* Monitorear la velocidad actual del motor en tiempo real.
* Registrar la última actualización con timestamp NTP.
El ESP32 actúa simultáneamente como:
* Controlador en lazo cerrado
* Servidor web HTTP
* Cliente NTP

## Estructura del proyecto
```text
/ex_1_2
│
├── Motor.h        // Clase Motor (encoder + control PI)
├── Motor.ino      // Implementación del control PI y PWM
└── ex_1_2.ino     // Archivo principal con FSM + WiFi + servidor web
```
## Funcionamiento general
El sistema opera como un controlador de velocidad remoto:
1. El encoder mide la velocidad real del motor.
2. El controlador PI ajusta el PWM para alcanzar el setpoint.
3. El usuario ingresa el setpoint desde un navegador web.
4. El ESP32 recibe el setpoint vía HTTP.
5. El estado del motor se publica periódicamente vía JSON.
6. Un timestamp NTP acompaña cada actualización.
Todo el flujo del control está gobernado por una máquina de estados finitos (FSM).
## Máquina de estados (FSM)
```cpp
enum state_t {
    IDLE,
    SAMPLE,
    REPORT,
    SETPOINT,
    STOP
};
```
### Estados principales:
* IDLE
  * Decide si:
    * Se muestrea el motor
    * Se reporta información
    * Se recibe un comando local por Serial
  * SAMPLE
    * Lee el encoder (RPM).
    * Ejecuta el controlador PI.
    * Actualiza la señal PWM.
  * REPORT
    * Muestra información por:
      * Monitor serial
      * Display OLED
    * SETPOINT
      * Permite ajustar el setpoint por Serial (modo local).
    * STOP
      * Detiene el motor y reinicia el controlador.
## Interfaz Web
La interfaz web permite:
* Ingresar el setpoint de RPM.
* Visualizar:
  * RPM actual
  * Timestamp de la última actualización
### Características
* HTML, CSS y JavaScript embebidos en el ESP32.
* Actualización automática cada 1 segundo.
* Comunicación vía HTTP POST y JSON.
## Servidor HTTP y API
### Endpoints disponibles
* /
  * Página principal con la interfaz de control.
* /set_rpm (POST)
  * Recibe el setpoint del motor en RPM.
* /status
  * Devuelve un JSON con:
    * ```json
      {
        "rpm": 1240,
        "timestamp": "2025-01-01 15:42:18"
      }
      ```
## Timestamp con NTP
El ESP32 sincroniza la hora mediante:
* Servidor: pool.ntp.org
* Zona horaria configurada (GMT-5)
El timestamp se incluye en las respuestas del servidor y en los logs.
## Visualización local (OLED)
El OLED muestra:
* RPM actual
* Setpoint
* Error del controlador
Configuración:
* Resolución: 128×64
* Dirección I2C: 0x3C

## Control del motor
* Encoder incremental con PCNT.
* Controlador PI.
* PWM con control de dirección mediante puente H.
* Setpoint actualizado tanto por Web como por Serial.

## Requisitos
### Hardware
* ESP32
* Motor DC con encoder incremental
* Puente H
* Display OLED SSD1306
* Fuente adecuada para el motor
* Red WiFi
### Software
* Arduino IDE
* Soporte ESP32
* Librerías:
  * Adafruit_GFX
  * Adafruit_SSD1306
  * WiFi
  * WebServer
  * Wire

## Conexiones principales
### Encoder → ESP32
| Señal | GPIO |
|-------|------|
| ENC A | GPIO 32 |
| ENC B | GPIO 33 |

### Puente H → ESP32
| Puente H | ESP32 |
|----------|-------|
| EN A | GPIO 5 |
| IN 1 | GPIO 19 |
| IN 2 | GPIO 18 |

### OLED → ESP32
| Señal | GPIO |
|-------|------|
| SDA | GPIO 21 |
| SCL | GPIO 22 |

## Uso del sistema
### Acceso web
1. Conectar el ESP32 a WiFi.
2. Abrir el navegador.
3. Ingresar a la IP mostrada en el monitor serial.
4. Ajustar el setpoint desde la interfaz web.

### Control por Serial
* s → Ingresar setpoint manual
  * número → nuevo setpoint
* x → detener motor
* Baudios: 115200

Para más información, leer el informe en la raíz del repositorio.
