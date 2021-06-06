/*
Control MQTT/Web para amplificador Harman Kardon AVR 155, emulando los comandos del 
mando infrarrojos y gestión de Bluetooth a2dp a traves de un modulo  UDA1334a.


    Creative Commons License Disclaimer

  UNLESS OTHERWISE MUTUALLY AGREED TO BY THE PARTIES
  AND MAKES NO REPRESENTATIONS OR WARRANTIES OF ANY 
  STATUTORY OR OTHERWISE, INCLUDING, WITHOUT LIMITAT
  FITNESS FOR A PARTICULAR PURPOSE, NONINFRINGEMENT,
  ACCURACY, OR THE PRESENCE OF ABSENCE OF ERRORS, WH
  DO NOT ALLOW THE EXCLUSION OF IMPLIED WARRANTIES, 
  EXCEPT TO THE EXTENT REQUIRED BY APPLICABLE LAW, I
  ON ANY LEGAL THEORY FOR ANY SPECIAL, INCIDENTAL, C
  ARISING OUT OF THIS LICENSE OR THE USE OF THE WORK
  POSSIBILITY OF SUCH DAMAGES.

  http://creativecommons.org/licenses/by-sa/3.0/

  Author: Radioelf  http://radioelf.blogspot.com.es/

Compiler 6/6/21
*/

#include <Arduino.h>
#include <WiFi.h>

#include <dac.h>

#define VERSION     "V. 0.1.1"                                                        // Versión
#define DebugSerial 1                                                                 // On/OFF degug serial
#define StatusLed 0                                                                   // On/OFF indicación estado por led

#define DataPin 21                                                                    // pin datos al microprocesador MCU(T5CC1) AVR155
#define ControlPin 19                                                                 // pin control interruptor TS5A3167 
#define MutePin 27                                                                    // pin modulo UDA1334a
#define RunAvr155 33                                                                  // pin AVR155 en tensión
#if StatusLed
#define LedPin  2                                                                     // pin Led PCB
#endif

const String Compiler = String(__DATE__);                                             // Obtenemos la fecha de la compilación
String clientId = "AVR155-";
uint8_t mode = 0;

#include "avr155.h"
#include "wifiEsp32.h"
#include "mqtt.h"

void setup() {
  pinMode (DataPin, INPUT);
  pinMode (ControlPin, OUTPUT);
  pinMode(MutePin, OUTPUT);
  pinMode(RunAvr155, INPUT_PULLDOWN);
#if StatusLed
  pinMode(LedPin, OUTPUT);
  digitalWrite (LedPin, HIGH);
#endif
  digitalWrite (MutePin, HIGH);
  digitalWrite(ControlPin, LOW);

#if DebugSerial
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
#endif

  a2d_sink.start((char *)"MiMusica");                                                 // bluetooth audio I2S->TS5A3167 (DAC)

  uint8_t mac[6];
  WiFi.macAddress(mac);
  String Mac = String(String(mac[3], 16) + String(mac[4], 16) + String(mac[5], 16));
  clientId = clientId + Mac;                                                          // Creamos in id unico can la MAC

  topicSt = clientId + topicSt;
  
  NetworkFound = confiWifi();  
  if (NetworkFound) {
#if DebugSerial
    Serial.println("");
    Serial.println("WiFi conectado");
    Serial.print("ID: ");
    Serial.println(clientId);
    Serial.print("Direccion IP: ");
    Serial.println(WiFi.localIP());
    Serial.print("MAC: ");
    Serial.println(WiFi.macAddress());
#endif
    mqttIni();
  } 
  iniWebServer();                                                                     // Inicializamos pagina web
#if StatusLed
  digitalWrite (LedPin, LOW);
#endif
#if DebugSerial
  if (digitalRead(RunAvr155))
    Serial.println("Equipo en marcha");
  else
    Serial.println("Equipo apagado!!");
  Serial.println("Iniciado...");
#endif
}

bool ConnectBle = true, audioStatus = false;
uint8_t Pulse = 0;
uint8_t MqttPublic = 0;
String topic = "";
//************************************************************************************
// Principal (núcleo 1)
//************************************************************************************
void loop() {
  do {
    mqttclient.loop();                                                                // Mantenemos activa la conexión MQTT
    if (!digitalRead(RunAvr155)) {
#if StatusLed
      digitalWrite(LedPin, HIGH);
      delay(50);
      digitalWrite(LedPin, LOW);
#endif
      delay(50);
      digitalWrite(MutePin, HIGH);                                                    // ON mute
#if DebugSerial
      Serial.println("Equipo apagado!!");
#endif
    } 
  } while (!digitalRead(RunAvr155));                                                  // Equipo en marcha?
  
  if (a2d_sink.statusBluetooth == 2 && ConnectBle == false) {                         // Conectado a bluetooth?
    ConnectBle = true;
    digitalWrite(MutePin, LOW);                                                       // OFF mute
    topic = clientId + topicAux;
    mqttclient.publish(topic.c_str(), "0");
    CodeAvrTX(0, CodeTape, 0);                                                        // pasamos AVR a entrada tape
    topic = clientId + topicTape;
    mqttclient.publish(topic.c_str(), "1");
    mode = 1;
#if DebugSerial
    Serial.println("Bluetooth Conectado ON TAPE");
#endif
#if StatusLed
    digitalWrite(LedPin, HIGH);
#endif
  } else if (a2d_sink.statusBluetooth == 0 && ConnectBle == true) {
    ConnectBle = false;
    digitalWrite(MutePin, HIGH);                                                      // ON mute
    topic = clientId + topicTape;
    mqttclient.publish(topic.c_str(), "0");
    CodeAvrTX(0, CodeAux, 0);                                                         // pasamos a modo aux
    topic = clientId + topicAux;
    mqttclient.publish(topic.c_str(), "1");
    mode = 2;
#if DebugSerial
    Serial.println("Bluetooth Desconectado ON AUX");
#endif
#if StatusLed
    digitalWrite(LedPin, LOW);
#endif
  }
#if DebugSerial
  if (audioStatus == false && a2d_sink.statusAudio == 2) {
    audioStatus = true;
    Serial.println("Play audio Bluetooth");
  }
  if (audioStatus == true && a2d_sink.statusAudio != 2) {
    audioStatus = false;
    Serial.println("NO audio Bluetooth");
  }
#endif
  if (mode > 9) {                                                                     // Orden desde web?
    mode = 0;
    topic = clientId + topicTape;
    mqttclient.publish(topic.c_str(), "0");
    topic = clientId + topicAux;
    mqttclient.publish(topic.c_str(), "0");
  }
  if (WiFi.status() != 3) {
#if DebugSerial
    Serial.println("Desconectado de WIFI");
#endif
    if (connectwifi(true) == false) {
#if DebugSerial
      Serial.println("Modo AP");
#endif
    }
  }
  if (!mqttclient.connected()){
#if DebugSerial
    Serial.println("MQTT desconectado!!");
#endif
    do {
      for (uint8_t i = 0; i < 10; i++) {
#if StatusLed
        digitalWrite(LedPin, HIGH);
        delay(50);
        digitalWrite(LedPin, LOW);
#endif
        delay(50);
      }
    } while (!mqttConnect());
  }

  if (++Pulse == 100) {                                                               // 100x50 = 50000
    Pulse  = 0;
#if StatusLed
    digitalWrite (LedPin, HIGH);
#endif
    delay(50);
    if (++MqttPublic % 12 == 0) {                                                     // ≈1 minuto 
      mqttclient.publish(topicSt.c_str(), "Online", MQTT_RETAIN);
      if (WiFi.RSSI() < -70) {                                                        // Señal RF baja?
        char payload[5];
        snprintf(payload, 4, "%d", WiFi.RSSI());
        String topic = clientId + "/rssi";
        mqttclient.publish(topic.c_str(), payload);
#if DebugSerial
        Serial.print("Señal RSSI baja: ");
        Serial.println(payload);
#endif
      }
#if DebugSerial
      Serial.println("MQTT online");
#endif
      if (MqttPublic == 720) {                                                        // ≈1 hora
        MqttPublic = 0;
        discovery (true);
#if DebugSerial
        Serial.println("MQTT Discovery");
#endif
      }
    }
#if StatusLed
    if (a2d_sink.statusBluetooth != 2) 
    digitalWrite(LedPin, LOW);
#endif
  } 
  else {
    delay (50);
  }
}
