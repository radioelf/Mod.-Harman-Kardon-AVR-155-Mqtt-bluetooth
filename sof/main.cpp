/*
GPIO's reservados

Algunos GPIO no se pueden usar libremente, ya que están asignados a los periféricos del
 módulo o tienen funciones especiales durante el arranque:

El pin GPIO0 se usa como pin de arranque y debe estar bajo para ingresar al modo 
de descarga UART. ¡Asegúrese de que ningún dispositivo periférico lo baje durante el 
arranque o el firmware no se iniciará!

El pin GPIO2 se usa como pin de arranque y debe estar bajo para ingresar al modo de 
descarga UART. ¡Asegúrese de que no sea elevado por un dispositivo periférico 
durante el arranque o no podrá actualizar un firmware en el módulo!

GPIO06 a GPIO11 están reservados para FLASH. ¡No puedes usarlos en absoluto!

GPIO12 se utiliza como un pin de arranque para seleccionar el voltaje de salida de un 
regulador interno que alimenta el chip flash (VDD_SDIO). Este pin tiene un menú 
desplegable interno, por lo que si se deja desconectado, se leerá bajo al reiniciar 
(seleccionando la operación predeterminada de 3.3V). ¡Asegúrese de que ningún 
dispositivo periférico lo tire hacia arriba durante el inicio o es posible que el módulo no pueda iniciarse!

GPIO15 se puede utilizar para detener la salida de depuración en Serial durante el 
arranque. Si se baja, no habrá salida en el puerto serie durante el proceso de arranque.
Esto puede resultar útil en aplicaciones que funcionan con baterías en las que no desea
utilizar el puerto serie para reducir el consumo de energía.

GPIO34-39 solo se puede configurar como modo de entrada y no tiene funciones de 
extracción o extracción de software

GPIO37 y 38 no están disponibles en la mayoría de los módulos

GPIO16 y 17 se utilizan para Flash interno (entrada de lectores)
ESCRITOR DE ESP32:

GPIO16 y 17 se utilizan para PSRAM
	
*/

#include <Arduino.h>
#include <WiFi.h>

#include <dac.h>

#define VERSION     "V. 0.0.1"                                                        // Versión
#define DebugSerial 1

#define DataPin 21                                                                    // pin datos al microprocesador AVR155
#define ControlPin 22                                                                 // pin control interruptor TS5A3167 (caja4 D1)
#define LedPin  2                                                                     // Led PCB

uint8_t error = 0;
String clientId = "AVR155-";

#include "wifiEsp32.h"
#include "avr155.h"
#include "mqtt.h"


const String      Compiler = String(__DATE__);                                        // Obtenemos la fecha de la compilación

void setup() {
  pinMode (DataPin, INPUT);
  pinMode (LedPin, OUTPUT);
  pinMode (ControlPin, OUTPUT);

  digitalWrite (LedPin, HIGH);

  Serial.begin(115200);
  Serial.println ("\n----->");

  // Imprime información y memoria
  Serial.print ("Iniciando ESP32 ejecutando en núcleo ");
  Serial.print (xPortGetCoreID()); Serial.print  (" a ");
  Serial.print (ESP.getCpuFreqMHz());
  Serial.print ("Mhz. Version:");
  Serial.print (VERSION);
  Serial.print (". Memoria libre de: ");
  Serial.println(ESP.getFreeHeap());
  Serial.print ("Compilado: ");
  Serial.println (Compiler);

  a2d_sink.start((char *)"MiMusica");

  uint8_t mac[6];    
  WiFi.macAddress(mac);
  String Mac = String(String(mac[3], 16) + String(mac[4], 16) + String(mac[5], 16));
  clientId = clientId + Mac;                                                          // Creamos in id único can la MAC

  topicSt = clientId + topicSt;
  
  NetworkFound = confiWifi();  
  if (NetworkFound) {
    Serial.println("");
    Serial.println("WiFi conectado");
    Serial.print("ID: ");
    Serial.println(clientId);
    Serial.print("Direccion IP: ");
    Serial.println(WiFi.localIP());
    Serial.print("MAC: ");
    Serial.println(WiFi.macAddress());
    mqttIni();
  } 

#if DebugSerial
  Serial.println("Iniciado...");
#endif
  iniWebServer();
  digitalWrite (LedPin, LOW);
}

uint8_t Pulse = 0;
uint8_t MqttPublic = 0;
//************************************************************************************
// Principal (núcleo 1)
//************************************************************************************
void loop() {
  mqttclient.loop();                                                                  // Mantenemos activa la conexión MQTT
  
  if (WiFi.status() != 3) {
    Serial.println("Desconectado de WIFI");
    if (connectwifi(true) == false) {
      Serial.println("Modo AP");
    }
  }
  if (!mqttclient.connected()){
    Serial.println("MQTT desconectado!!");
    do {
      for (uint8_t i = 0; i < 10; i++) {
        digitalWrite(LedPin, HIGH);
        delay(50);
        digitalWrite(LedPin, LOW);
        delay(50);
      }
    } while (!mqttConnect());
  }

  if (++Pulse == 100) {                                                               // 100x50 = 50000
    Pulse  = 0;
    digitalWrite (LedPin, HIGH);
    delay(50);
    if (++MqttPublic % 12 == 0) {                                                     // ≈1 minuto 
      mqttclient.publish(topicSt.c_str(), "online", MQTT_RETAIN);
      Serial.println("MQTT online");
      if (MqttPublic == 720) {                                                        // ≈1 hora
        MqttPublic = 0;
        discovery (true);
        Serial.println("MQTT Discovery");
      }
    }
    digitalWrite(LedPin, LOW);
  } 
  else {
    delay (50);
  }
}
