# Diseño e Implementación de Sistemas Embebidos  
## Laboratorio – ESP32 IoT Systems

**Autor:** Joan Esteban Velasco Larrea  
**Plataforma:** ESP32  
**Entorno de desarrollo:** Arduino IDE  
**Comunicación:** HTTP, MQTT, WiFi  

---

## Descripción General

Este repositorio contiene la implementación de varios retos orientados al diseño y desarrollo de sistemas embebidos basados en ESP32, enfocados en **IoT**, **comunicación en red**, **arquitecturas no bloqueantes** y **visualización de datos**.

- Servidor Web HTTP autónomo sobre ESP32.
- Sistema IoT basado en MQTT con publicación de telemetría y control remoto mediante setpoints.
- Visualización y monitoreo usando dashboards externos (Node-RED).

Cada reto se encuentra organizado en su propia carpeta, con código fuente independiente y documentación específica.

---

## Estructura del Repositorio

```text
├── ex3/
│ ├── ex3.ino
│ ├── README.md
│
├── ex4/
│ ├── Motor.h
│ ├── Motor.ino
│ ├── ex4.ino
│ ├── README.md
│
├── Informe_lab.pdf
│
└── README.md
```

---


- Cada carpeta `ex*` corresponde a un reto específico.
- Cada reto incluye su propio `README.md` con detalles de implementación.
- El archivo `Informe_lab.pdf` contiene el informe completo del laboratorio.

---

## Tecnologías y Herramientas Utilizadas

- **Microcontrolador:** ESP32  
- **Sensores:**  
  - DHT22 (temperatura y humedad)  
- **Pantallas:**  
  - OLED SSD1306 (I2C)  
- **Protocolos de Comunicación:**  
  - HTTP  
  - MQTT (TLS)  
  - WiFi  
- **Broker MQTT:** HiveMQ Cloud  
- **Dashboard IoT:** Node-RED  
- **Formato de mensajes:** JSON  

---

## Retos Incluidos

### 1. ESP32 Standalone Web Server
- Servidor HTTP embebido en ESP32.
- Monitoreo de temperatura y humedad.
- Interfaz web accesible desde red local.
- Ingreso de setpoint de RPM vía navegador.
- Arquitectura no bloqueante.
- Sincronización horaria mediante NTP.
- Visualización local en pantalla OLED.

Implementación: `ex_1_1/` y `ex_1_2/`

---

### 2. ESP32 MQTT-Based IoT System
- Publicación periódica de telemetría (temperatura y humedad).
- Comunicación segura con broker MQTT en la nube (TLS).
- Estructura de tópicos jerárquica.
- Mensajes en formato JSON.
- Suscripción a setpoint de RPM.
- Visualización y control mediante Node-RED.
- Justificación y uso de QoS en publicación y suscripción.
- El motor se maneja de forma lógica (hardware no requerido).

Implementación: `ex2/`

---

## Formato de Mensajes MQTT

Ejemplo de mensaje de telemetría:

```json
{
  "temperature": 24.6,
  "humidity": 58.2,
  "unit": {
    "temperature": "C",
    "humidity": "%"
  }
}
```
Ejemplo de mensaje de control:

```json
{
  "rpm": 120
}
```

---

## Ejecución y Pruebas
1. Abrir el proyecto correspondiente en Arduino IDE.
2. Configurar credenciales WiFi y MQTT.
3. Compilar y cargar el código al ESP32.
4. Para MQTT:
   * Verificar conexión al broker HiveMQ Cloud
   * Usar Node-RED como dashboard de visualización y control.
5. Monitorear datos através del monitor serial
