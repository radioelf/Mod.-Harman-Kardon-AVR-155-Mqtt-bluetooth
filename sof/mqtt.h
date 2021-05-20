#ifndef _MQTT_
#define _MQTT_
/*
  Envio al sevidor mosquitto de los datos a través de los topic

    Creative Commons License Disclaimer

  UNLESS OTHERWISE MUTUALLY AGREED TO BY THE PARTIES IN WRITING, LICENSOR OFFERS THE WORK AS-IS
  AND MAKES NO REPRESENTATIONS OR WARRANTIES OF ANY KIND CONCERNING THE WORK, EXPRESS, IMPLIED,
  STATUTORY OR OTHERWISE, INCLUDING, WITHOUT LIMITATION, WARRANTIES OF TITLE, MERCHANTIBILITY,
  FITNESS FOR A PARTICULAR PURPOSE, NONINFRINGEMENT, OR THE ABSENCE OF LATENT OR OTHER DEFECTS,
  ACCURACY, OR THE PRESENCE OF ABSENCE OF ERRORS, WHETHER OR NOT DISCOVERABLE. SOME JURISDICTIONS
  DO NOT ALLOW THE EXCLUSION OF IMPLIED WARRANTIES, SO SUCH EXCLUSION MAY NOT APPLY TO YOU.
  EXCEPT TO THE EXTENT REQUIRED BY APPLICABLE LAW, IN NO EVENT WILL LICENSOR BE LIABLE TO YOU
  ON ANY LEGAL THEORY FOR ANY SPECIAL, INCIDENTAL, CONSEQUENTIAL, PUNITIVE OR EXEMPLARY DAMAGES
  ARISING OUT OF THIS LICENSE OR THE USE OF THE WORK, EVEN IF LICENSOR HAS BEEN ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGES.

  http://creativecommons.org/licenses/by-sa/3.0/

  Author: Radioelf  http://radioelf.blogspot.com.es/
*/

#include <PubSubClient.h>                                                     	      // Librería para MQTT https://github.com/knolleary/pubsubclient

const char*       mqttServer = "192.168.x.xxx";
const int         mqttPort = 1883 ;
const char*       mqttUser = "nombre";
const char*       mqttPassword = "contrasenya";

String topicSt = "/status";
String topicOn = "/switch/on";
String topicOff = "/switch/off";
String topicOK = "/switch/ok";
String topicMute = "/switch/mute";
String topicUpvol = "/switch/upvolumen";
String topicLowvol = "/switch/lowvolumen";
String topicAvr = "/switch/avr";
String topicCd = "/switch/cd";
String topicDvd = "/switch/dvd";
String topicAmFm = "/switch/am_fm";
String topicAux = "/switch/aux";
String topicTape = "/switch/tape";
String topicVideo1 = "/switch/video1";
String topicNoche = "/switch/noche";
String topicDerecha = "/switch/derecha";
String topicIzquieda = "/switch/izquierda";
String topicArriba = "/switch/arriba";
String topicAbajo = "/switch/abajo";
String topicDts = "/switch/dts";
String topicDtsNeo = "/switch/dtsneo";

bool MQTT_RETAIN = false;                                                     	      // MQTT bandera retain
uint8_t MQTT_QOS = 0;                                                                 // MQTT QoS mensajes
const char *willTopic = 0;                                                            // MQTT willTopic
const char *willMessage = 0;                                                          // MQTT willMessage

WiFiClient        wmqttclient;                                                        // Instancia para mqtt
PubSubClient      mqttclient (wmqttclient);

//************************************************************************************
// Publicamos  el topic y el valor double pasado
//************************************************************************************
void PublishMqtt(const char* topic, double value) {
  char topicState [strlen(topic) + 7];
  String StateTopic = String (topic) + "/state";
  StateTopic.toCharArray(topicState, StateTopic.length() + 1);
  char string_mqtt [9];
  if (value == 1 || value == 0)
    dtostrf (value, 1, 0, string_mqtt);
  else
    dtostrf (value, 6, 2, string_mqtt);                                         	    // double, número de caracteres, número de decimales, char resultante
  mqttclient.publish(topicState, string_mqtt);
}

//************************************************************************************
// Publicamos  el topic y la trama pasada
//************************************************************************************
void PublishMqttT(const char* topic, char* string_mqtt) {
  mqttclient.publish(topic, string_mqtt);
}

//************************************************************************************
// Envió periódico y autodeteción para Home Assistant
//Auto detección para Home Assistant <discovery_prefix>/<component>/[<node_id>/]<object_id>/config
//************************************************************************************
void discovery (bool force) {
  static bool autoDetec = 0;
  static uint8_t RunDiscovery = 0;
  String topic = clientId + "/app";
  mqttclient.publish(topic.c_str(), "Radioelf", MQTT_RETAIN);
  topic = clientId + "/version";
  mqttclient.publish(topic.c_str(), "VERSION", MQTT_RETAIN);
  topic = clientId + "/board";
  mqttclient.publish(topic.c_str(), (char*) "ARVR155_RADIOELF");
  topic = clientId + "/host";
  mqttclient.publish(topic.c_str(), (char*) "Control ARVR155");
  topic = clientId + "/desc/";
  mqttclient.publish(topic.c_str(), (char *)"CONTROL_ARVR155");
  topic = clientId + "/ssid";
  mqttclient.publish(topic.c_str(), (char*) ssid);
  topic = clientId + "/ip";
  mqttclient.publish(topic.c_str(), (char*) WiFi.localIP().toString().c_str());
  topic = clientId + "/mac";
  mqttclient.publish(topic.c_str(), (char*) WiFi.macAddress().c_str());
  char payload[5];
  snprintf(payload, 4, "%d", WiFi.RSSI());
  topic = clientId + "/rssi";
  mqttclient.publish(topic.c_str(), payload);

  if (RunDiscovery++ != 12 && autoDetec == 1 && force == 0) return;
  
  RunDiscovery = 0;
  // Incio, force o aproximadamente 1 hora
  uint8_t nTopic = 0;
  String ValueName = "Estado On";
  String Name = "avr155_on";
  String topicName = clientId + topicOn;
  uint8_t mac[6];
  WiFi.macAddress(mac);
  String Mac = String(mac[0], 16) + String(mac[1], 16) + String(mac[2], 16) + String(mac[3], 16) + String(mac[4], 16) + String(mac[5], 16);

  while (nTopic < 13) {
    String message = "{";
    message += String(F("\"name\":\"")) + ValueName + String(F("\",\""));
    message += String(F("icon\":\"mdi:distribute-vertical-bottom")) + String(F("\",\""));
    message += String(F("state_topic\":\"")) + topicName + String(F("\",\""));
    message += String(F("command_topic\":\"")) + clientId + String(F("/switch/")) + Name + String(F("/set\",\""));
    message += String(F("payload_on\":\"1\",\"payload_off\":\"0\",\""));
    message += String(F("availability_topic\":\"")) + clientId + String(F("/status\",\""));
    message += String(F("unique_id\":\"")) + String(F("AVR155-ESP32_")) + Name + String(F("\",\""));
    message += String(F("device\":{\"identifiers\":\"")) + Mac + String(F("\",\""));
    message += String(F("name\":\"Control AVR155")) + String(F("\",\"sw_version\":\"Control-AVR155 ")) + String(VERSION) + String(F("\",\"model\":\"avr155\",\"manufacturer\":\"Radioelf\"}"));
    message += "}";

    String topic = ("homeassistant/switch/" + clientId + "_" + Name + "/config");
    autoDetec = mqttclient.publish((char*) topic.c_str(), (char*) message.c_str(), MQTT_RETAIN);
    nTopic ++;
    switch (nTopic) {
      case 1:
        ValueName = "Estado OFF";
        Name = "avr155_off";
        topicName = clientId + topicOff;
        break;
      case 2:
        ValueName = "botón OK";
        Name = "ok";
        topicName = clientId + topicOK;
        break;
      case 3:
        ValueName = "Orden Mute";
        Name = "mute";
        topicName = clientId + topicMute;
        break;
      case 4:
        ValueName = "Orden subir volumen";
        Name = "subir_volumen";
        topicName = clientId + topicUpvol;
        break;
      case 5:
        ValueName = "Orden bajar volumen";
        Name = "bajar_volumen";
        topicName = clientId + topicLowvol;
        break;
      case 6:
        ValueName = "Modo AVR";
        Name = "avr";
        topicName = clientId + topicAvr;
        break;
      case 7:
        ValueName = "Modo DVD";
        Name = "dvd";
        topicName = clientId + topicDvd;
        break;
      case 8:
        ValueName = "Modo CD";
        Name = "cd";
        topicName = clientId + topicCd;
        break;
      case 9:
        ValueName = "Modo AM/FM";
        Name = "am_fm";
        topicName = clientId + topicAmFm;
        break;
      case 10:
        ValueName = "Modo AUX";
        Name = "aux";
        topicName = clientId + topicAux;
        break;
      case 11:
        ValueName = "Modo Tape";
        Name = "tape";
        topicName = clientId + topicTape;
        break;
      case 12:
        ValueName = "Modo Video1";
        Name = "vid1";
        topicName = clientId + topicVideo1;
        break;
	  case 13:
        ValueName = "Modo Noche";
        Name = "noche";
        topicName = clientId + topicNoche;
        break;
	  case 14:
        ValueName = "botón Derecha";
        Name = "derecha";
        topicName = clientId + topicDerecha;
        break;
	  case 15:
        ValueName = "Botón Izquierda";
        Name = "izquierda";
        topicName = clientId + topicIzquierda;
        break;
	  case 16:
        ValueName = "Botón Arriba";
        Name = "arriba";
        topicName = clientId + topicArriba;
        break;
	  case 17:
        ValueName = "Botón Abajo";
        Name = "abajo";
        topicName = clientId + topicAbajo;
        break;
	  case 18:
        ValueName = "Modo DTS";
        Name = "dts";
        topicName = clientId + topicDts;
        break;
	  case 19:
        ValueName = "Modo DTSneo";
        Name = "dtsneo";
        topicName = clientId + topicDtsNeo;
        break;
    }
    delay (1);
  }
  mqttclient.publish(topicSt.c_str(), autoDetec ? "online" : "offline", MQTT_RETAIN);

}

//************************************************************************************
// Recepción mqtt
//************************************************************************************
void mqttCallback (char* topic, byte* message, unsigned int leng) {
String messageRx;
#if DebugSerial
  Serial.print("RX mqtt: ");
  Serial.print(topic);
  Serial.print(" mensaje: ");
  for (int i = 0; i < leng; i++) {
    if (leng-1 != i)
      Serial.print((char)message[i]);
    else
      Serial.println((char)message[i]);
    messageRx += (char)message[i];
  }
#endif
  if (messageRx == "restartCmd") {
#if DebugSerial
    Serial.println ("RESET");
#endif
    ESP.restart();                                                                    // Comando reset?
  }
  if (messageRx == "updateCmd") {
    discovery(true);                                                                  // Comando actualizar
#if DebugSerial
    Serial.println ("Actualizar");
#endif
    return;
  }
  String  ComandSwitch = clientId + "/switch/avr155_off/set";
  if ((String(topic) == ComandSwitch) && (messageRx == "1")) {
    CodeAvrTX(0, CodeOn);
#if DebugSerial
    Serial.println ("Orden OFF");
#endif
    return;
  }
  ComandSwitch = clientId + "/switch/avr155_on/set";
  if ((String(topic) == ComandSwitch) && (messageRx == "1")) {
    CodeAvrTX(0, CodeOff);
#if DebugSerial
    Serial.println ("Orden ON");
#endif
    return;
  }
  ComandSwitch = clientId + "/switch/avr155_on/set";
  if ((String(topic) == ComandSwitch) && (messageRx == "1")) {
    CodeAvrTX(0, CodeMute);
#if DebugSerial
    Serial.println ("Mute");
#endif
    return;
  }
  ComandSwitch = clientId + "/switch/avr155_on/set";
  if ((String(topic) == ComandSwitch) && (messageRx == "1")) {
    CodeAvrTX(0, CodeAvr);
#if DebugSerial
    Serial.println ("Modo AVR");
#endif
    return;
  }
  ComandSwitch = clientId + "/switch/avr155_on/set";
  if ((String(topic) == ComandSwitch) && (messageRx == "1")) {
    CodeAvrTX(0, CodeDvd);
#if DebugSerial
    Serial.println ("Entrada DVD");
#endif
    return;
  }
  ComandSwitch = clientId + "/switch/avr155_on/set";
  if ((String(topic) == ComandSwitch) && (messageRx == "1")) {
    CodeAvrTX(0, CodeCd);
#if DebugSerial
    Serial.println ("Entrada CD");
#endif
    return;
  }
  ComandSwitch = clientId + "/switch/avr155_on/set";
  if ((String(topic) == ComandSwitch) && (messageRx == "1")) {
    CodeAvrTX(0, CodeVid1);
#if DebugSerial
    Serial.println ("Entrada Video1");
#endif
    return;
  }
  ComandSwitch = clientId + "/switch/avr155_on/set";
  if ((String(topic) == ComandSwitch) && (messageRx == "1")) {
    CodeAvrTX(0, CodeHdmi1);
#if DebugSerial
    Serial.println ("Entrada HDMI1");
#endif
    return;
  }
  ComandSwitch = clientId + "/switch/avr155_on/set";
  if ((String(topic) == ComandSwitch) && (messageRx == "1")) {
    CodeAvrTX(0, CodeAux);
#if DebugSerial
    Serial.println ("Entrada AUX");
#endif
    return;
  }
  ComandSwitch = clientId + "/switch/avr155_on/set";
  if ((String(topic) == ComandSwitch) && (messageRx == "1")) {
    CodeAvrTX(0, CodeTape);
#if DebugSerial
    Serial.println ("Entrada TAPE");
#endif
    return;
  }
  ComandSwitch = clientId + "/switch/avr155_on/set";
  if ((String(topic) == ComandSwitch) && (messageRx == "1")) {
    CodeAvrTX(0, CodeSleep);
#if DebugSerial
    Serial.println ("Orden Reposo");
#endif
    return;
  }
  ComandSwitch = clientId + "/switch/avr155_on/set";
  if ((String(topic) == ComandSwitch) && (messageRx == "1")) {
    CodeAvrTX(0, CodeTest);
#if DebugSerial
    Serial.println ("Orden Test");
#endif
    return;
  }
  ComandSwitch = clientId + "/switch/avr155_on/set";
  if ((String(topic) == ComandSwitch) && (messageRx == "1")) {
    CodeAvrTX(0, Code6ch);
#if DebugSerial
    Serial.println ("Orden 6CH");
#endif
    return;
  }
  ComandSwitch = clientId + "/switch/avr155_on/set";
  if ((String(topic) == ComandSwitch) && (messageRx == "1")) {
    CodeAvrTX(0, CodeSurr);
#if DebugSerial
    Serial.println ("Orden Dolby Surround");
#endif
    return;
  }
  ComandSwitch = clientId + "/switch/avr155_on/set";
  if ((String(topic) == ComandSwitch) && (messageRx == "1")) {
    CodeAvrTX(0, CodeOsd);
#if DebugSerial
    Serial.println ("Orden Mensajes TFT");
#endif
    return;
  }
  ComandSwitch = clientId + "/switch/avr155_on/set";
  if ((String(topic) == ComandSwitch) && (messageRx == "1")) {
    CodeAvrTX(0, CodeOff);
#if DebugSerial
    Serial.println ("Orden ON");
#endif
    return;
  }
  ComandSwitch = clientId + "/switch/avr155_on/set";
  if ((String(topic) == ComandSwitch) && (messageRx == "1")) {
    CodeAvrTX(0, CodeIncvol);
#if DebugSerial
    Serial.println ("Incremento volumen");
#endif
    return;
ComandSwitch = clientId + "/switch/avr155_on/set";
  if ((String(topic) == ComandSwitch) && (messageRx == "1")) {
    CodeAvrTX(0, CodeDecvol);
#if DebugSerial
    Serial.println ("Decremento volumen");
#endif
    return;
ComandSwitch = clientId + "/switch/avr155_on/set";
  if ((String(topic) == ComandSwitch) && (messageRx == "1")) {
    CodeAvrTX(0, CodeIncvol);
#if DebugSerial
    Serial.println ("Incremento volumen");
#endif
    return;
ComandSwitch = clientId + "/switch/avr155_on/set";
  if ((String(topic) == ComandSwitch) && (messageRx == "1")) {
    CodeAvrTX(0, CodeArriba);
#if DebugSerial
    Serial.println ("Botón Subir");
#endif
    return;

ComandSwitch = clientId + "/switch/avr155_on/set";
  if ((String(topic) == ComandSwitch) && (messageRx == "1")) {
    CodeAvrTX(0, CodeAbajo);
#if DebugSerial
    Serial.println ("Botón Bajar");
#endif
    return;

ComandSwitch = clientId + "/switch/avr155_on/set";
  if ((String(topic) == ComandSwitch) && (messageRx == "1")) {
    CodeAvrTX(0, CodeDerecha);
#if DebugSerial
    Serial.println ("Botón Derecha");
#endif
    return;

ComandSwitch = clientId + "/switch/avr155_on/set";
  if ((String(topic) == ComandSwitch) && (messageRx == "1")) {
    CodeAvrTX(0, CodeIzquierda);
#if DebugSerial
    Serial.println ("Botón Izquierda");
#endif
    return;

ComandSwitch = clientId + "/switch/avr155_on/set";
  if ((String(topic) == ComandSwitch) && (messageRx == "1")) {
    CodeAvrTX(0, CodeOk);
#if DebugSerial
    Serial.println ("Botón OK");
#endif
    return;
	
ComandSwitch = clientId + "/switch/avr155_on/set";
  if ((String(topic) == ComandSwitch) && (messageRx == "1")) {
    CodeAvrTX(0, CodeDts);
#if DebugSerial
    Serial.println ("Botón DTS");
#endif
    return;
	
	ComandSwitch = clientId + "/switch/avr155_on/set";
  if ((String(topic) == ComandSwitch) && (messageRx == "1")) {
    CodeAvrTX(0, CodeDtsNeo);
#if DebugSerial
    Serial.println ("Botón DtsNeo");
#endif
    return;
	
	ComandSwitch = clientId + "/switch/avr155_on/set";
  if ((String(topic) == ComandSwitch) && (messageRx == "1")) {
    CodeAvrTX(0, CodeNoche);
#if DebugSerial
    Serial.println ("Modo Noche");
#endif
    return;

  }
#if DebugSerial
  Serial.println("MQTT SIN acción");
#endif
}

//************************************************************************************
// Conexión o reconexión al servidor mqtt
//************************************************************************************
bool mqttConnect() {
  if (!mqttclient.connect((char *)clientId.c_str(), mqttUser, mqttPassword, willTopic, MQTT_QOS, MQTT_RETAIN, willMessage, 0)) {
    Serial.print ("Fallo al Reconectar al broker, rc=");                           // -2 fallo de conexión
#if DebugSerial
    Serial.println (mqttclient.state());
#endif
    return false;
  }
  mqttclient.publish(topicSt.c_str(), "online", MQTT_RETAIN);
  const String SubTopic1 = clientId + "/switch/#";
  if (!mqttclient.subscribe(SubTopic1.c_str(), 0)) 
#if DebugSerial
Serial.println("MQTT ERROR suscripción SWITCH");
#endif
  const String SubTopic2 = clientId + "/cmd/#";
  if (!mqttclient.subscribe(SubTopic2.c_str(), 0)) 
#if DebugSerial
Serial.println("MQTT ERROR suscripción CMD");
  Serial.println("MQTT reconexión");
#endif
  return true;
  }

//************************************************************************************
// Inicializamos mqtt
//************************************************************************************
void mqttIni() {
  mqttclient.disconnect();
  //mqttclient.setBufferSize(1024);                                                   // Tamaño máximo de paquete MQTT
  mqttclient.setKeepAlive(300);                                                       // Tiempo de espera entre intercambio de mensajes
  mqttclient.setClient(wmqttclient);
  mqttclient.setServer(mqttServer, mqttPort);
  mqttclient.setCallback(mqttCallback); 

  //                              clientID         username    password    willTopic  willQoS,  willRetain,  willMessage,  cleanSession =1 (default)
  if (mqttclient.connect((char *)clientId.c_str(), mqttUser, mqttPassword, willTopic, MQTT_QOS, MQTT_RETAIN, willMessage, 0)){
    discovery(true);
#if DebugSerial
    Serial.println("MQTT OK -Discovery-");
#endif
    const String SubTopic1 = clientId  + "/switch/#";
    if (!mqttclient.subscribe(SubTopic1.c_str(), 0)) 
#if DebugSerial
Serial.println("MQTT ERROR suscripción SWITCH");
    Serial.print(F("MQTT Subcrito a: "));
    Serial.println (SubTopic1);
#endif
    const String SubTopic2 = clientId  + "/cmd/#";
    if (!mqttclient.subscribe(SubTopic2.c_str(), 0)) 
#if DebugSerial
Serial.println("MQTT ERROR suscripción CMD");
    Serial.print("MQTT Subcrito a: ");
    Serial.println (SubTopic2);
  }
  else {
    Serial.print ("Fallo al conectar al broker, rc=");
    Serial.println (mqttclient.state());
#endif
  }
}

#endif
