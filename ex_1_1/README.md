# Monitor Ambiental IoT con ESP32, DHT y Servidor Web
## Descripción del proyecto
Este proyecto implementa un monitor ambiental IoT basado en ESP32, capaz de:
* Medir temperatura y humedad mediante un sensor DHT.
* Mostrar las mediciones localmente en un display OLED SSD1306.
* Publicar los datos en tiempo real a través de un servidor web HTTP.
* Incluir timestamp obtenido vía NTP para cada actualización.
El sistema expone una interfaz web dinámica que se actualiza automáticamente mediante peticiones AJAX a un endpoint JSON.
## Funcionamiento general
El ESP32 actúa como:
* Nodo de adquisición (sensores)
* Servidor web embebido
* Cliente NTP para sincronización horaria
El flujo del sistema es:
1. El ESP32 se conecta a una red WiFi.
2. Sincroniza la hora mediante NTP.
3. Lee periódicamente los sensores.
4. Muestra los datos en el OLED.
5. Sirve una página web con los datos actuales.
6. Responde a peticiones /data con un objeto JSON.
## Interfaz web
La página web:
* Se sirve desde la raíz (/)
* Consulta periódicamente el endpoint /data
* Actualiza los valores sin recargar la página
### Datos mostrados:
* Temperatura (°C)
* Humedad (%)
* Última actualización (timestamp)
El refresco se realiza cada 2 segundos usando fetch().
## Servidor HTTP y API REST
### Endpoints disponibles
* /
  * Devuelve la página HTML principal.
  * Contiene el código CSS y JavaScript embebido.
* /data
  * Devuelve un objeto JSON con:
    ```json
    {
    "temperature": 25.3,
    "humidity": 61.2,
    "timestamp": "2025-01-01 14:32:10"
    }
    ```
## Temporización
El sistema utiliza millis() para:
* Actualizar el OLED cada report_period ms.
* Mantener el servidor web activo sin bloqueos.
El servidor HTTP se atiende continuamente mediante:
```cpp
server.handleClient();
```

## Visualización local (OLED)
El display OLED muestra:
* Temperatura del DHT
* Humedad del DHT
* Temperatura de un sensor analógico adicional
Configuración:
* Resolución: 128×64
* Dirección I2C: 0x3C
## Timestamp con NTP
El ESP32 sincroniza la hora usando:
* Servidor: pool.ntp.org
* Zona horaria configurada (GMT-5)
El timestamp se agrega a cada respuesta JSON y se muestra en la web.
## Requisitos
### Hardware
* ESP32
* Sensor DHT (DHT11 o DHT22)
* Display OLED SSD1306 (I2C)
* Sensor analógico de temperatura (opcional)
* Conexión WiFi
### Software
* Arduino IDE
* Soporte para ESP32
* Librería DHTesp
* Librerías Adafruit_GFX y Adafruit_SSD1306
* Librería WiFi
* Librería WebServer
* Librería Wire

## Conexiones
### DHT → ESP32
| Señal | GPIO |
|-------|------|
| DATA | GPIO 15 |

### OLED → ESP32
| Señal | GPIO |
|-------|------|
| SDA | GPIO 21 |
| SCL | GPIO 22 |

### Sensor analógico → ESP32 (opcional)
| Señal | GPIO |
|-------|------|
| OUT | GPIO 34 |

## Instalación y uso
1. Instalar Arduino IDE.
2. Agregar soporte para ESP32.
3. Instalar las librerías requeridas.
4. Configurar SSID y contraseña WiFi en el código.
5. Conectar el hardware.
6. Cargar el programa al ESP32.
7. Abrir el monitor serial a 115200 baudios.
8. Ingresar en un navegador a la IP mostrada por Serial.

### En la carpeta Demostration se encuentra un video con el funcionamiento del sistema.

Para mayor información, revisar el reporte técnico en la raíz del repositorio.
