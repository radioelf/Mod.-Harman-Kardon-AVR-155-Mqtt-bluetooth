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

const char*       mqttServer = "192.168.1.xxx";
const int         mqttPort = 1883 ;
const char*       mqttUser = "xxxxx";
const char*       mqttPassword = "xxxxx";

String topicSt = "/status";
String topicOn = "/switch/avr155_on";
String topicOff = "/switch/avr155_off";
String topicOK = "/switch/avr155_ok";
String topicMute = "/switch/avr155_mute";
String topicUpvol = "/switch/avr155_upvolumen";
String topicLowvol = "/switch/avr155_lowvolumen";
String topicAvr = "/switch/avr155_avr";
String topicCd = "/switch/avr155_cd";
String topicCanal = "/switch/avr155_ch";
String topicDvd = "/switch/avr155_dvd";
String topicHdmi1 = "/switch/avr155_hdmi1";
String topicAmFm = "/switch/avr155_am_fm";
String topicAux = "/switch/avr155_aux";
String topicTape = "/switch/avr155_tape";
String topicVideo1 = "/switch/avr155_vid1";
String topicNoche = "/switch/avr155_noche";
String topicDerecha = "/switch/avr155_derecha";
String topicIzquierda = "/switch/avr155_izquierda";
String topicArriba = "/switch/avr155_arriba";
String topicAbajo = "/switch/avr155_abajo";
String topicDts = "/switch/avr155_dts";
String topicDtsNeo = "/switch/avr155_dtsneo";
String topicSleep = "/switch/avr155_sleep";
String topicDim = "/switch/avr155_dim";
String topicStereo = "/switch/avr155_stereo";
String topicDigital = "/switch/avr155_digital";
String topicTest = "/switch/avr155_test";
String topicSurr = "/switch/avr155_surr";
String topicOsd = "/switch/avr155_osd";

bool MQTT_RETAIN = false;                                                             // MQTT bandera retain
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
  String ValueName = "AVR155";
  String Name = "avr155";
  String Unit = "";
  String topicName = clientId + topicSt;
  topicSt = topicName;
  uint8_t mac[6];
  WiFi.macAddress(mac);
  String Mac = String(mac[0], 16) + String(mac[1], 16) + String(mac[2], 16) + String(mac[3], 16) + String(mac[4], 16) + String(mac[5], 16);

  String messageStatus = "{";
  messageStatus += String(F("\"unit_of_measurement\":\"")) + Unit + String(F("\",\"icon\":\"mdi:cast-connected")) + String(F("\",\""));
  messageStatus += String(F("name\":\"")) + ValueName + String(F("\",\""));
  messageStatus += String(F("state_topic\":\"")) + topicName + String(F("\",\""));
  messageStatus += String(F("availability_topic\":\"")) + topicName + String(F("\",\""));
  messageStatus += String(F("unique_id\":\"")) + String(F("AVR155-ESP32_")) + Name + String(F("\",\""));
  messageStatus += String(F("device\":{\"identifiers\":\"")) + Mac + String(F("\",\""));
  messageStatus += String(F("name\":\"Control AVR155")) + String(F("\",\"sw_version\":\"Control-AVR155 ")) + String(VERSION) + "-" + String(Compiler) + String(F("\",\"model\":\"avr155\",\"manufacturer\":\"Radioelf\"}"));
  messageStatus += "}";
  String topicS = ("homeassistant/sensor/" + clientId + "_" + Name + "/config");
  autoDetec = mqttclient.publish((char *)topicS.c_str(), (char *)messageStatus.c_str(), MQTT_RETAIN);

  ValueName = "Estado On";
  Name = "avr155_on";
  topicName = clientId + topicOn;
  while (nTopic < 24) {
    String message = "{";
    message += String(F("\"name\":\"")) + ValueName + String(F("\",\""));
    message += String(F("icon\":\"mdi:distribute-vertical-bottom")) + String(F("\",\""));
    message += String(F("state_topic\":\"")) + topicName + String(F("\",\""));
    message += String(F("command_topic\":\"")) + clientId + String(F("/switch/")) + Name + String(F("/set\",\""));
    message += String(F("payload_on\":\"1\",\"payload_off\":\"0\",\""));
    message += String(F("unique_id\":\"")) + String(F("AVR155-ESP32_")) + Name + String(F("\",\""));
    message += String(F("device\":{\"identifiers\":\"")) + Mac + String(F("\",\""));
    message += String(F("name\":\"Control AVR155")) + String(F("\",\"sw_version\":\"Control-AVR155 ")) + String(VERSION) + "-" + String(Compiler) + String(F("\",\"model\":\"avr155\",\"manufacturer\":\"Radioelf\"}"));
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
        Name = "avr155_ok";
        topicName = clientId + topicOK;
        break;
      case 3:
        ValueName = "Orden Mute";
        Name = "avr155_mute";
        topicName = clientId + topicMute;
        break;
      case 4:
        ValueName = "Orden subir volumen";
        Name = "avr155_upvolumen";
        topicName = clientId + topicUpvol;
        break;
      case 5:
        ValueName = "Orden bajar volumen";
        Name = "avr155_lowvolumen";
        topicName = clientId + topicLowvol;
        break;
      case 6:
        ValueName = "Modo AVR";
        Name = "avr155_avr";
        topicName = clientId + topicAvr;
        break;
      case 7:
        ValueName = "Modo DVD";
        Name = "avr155_dvd";
        topicName = clientId + topicDvd;
        break;
      case 8:
        ValueName = "Modo CD";
        Name = "avr155_cd";
        topicName = clientId + topicCd;
        break;
      case 9:
        ValueName = "Modo AM/FM";
        Name = "avr155_am_fm";
        topicName = clientId + topicAmFm;
        break;
      case 10:
        ValueName = "Modo AUX";
        Name = "avr155_aux";
        topicName = clientId + topicAux;
        break;
      case 11:
        ValueName = "Modo Tape";
        Name = "avr155_tape";
        topicName = clientId + topicTape;
        break;
      case 12:
        ValueName = "Modo Video1";
        Name = "avr155_vid1";
        topicName = clientId + topicVideo1;
        break;
	  case 13:
        ValueName = "Modo Noche";
        Name = "avr155_noche";
        topicName = clientId + topicNoche;
        break;
	  case 14:
        ValueName = "botón Derecha";
        Name = "avr155_derecha";
        topicName = clientId + topicDerecha;
        break;
	  case 15:
        ValueName = "Botón Izquierda";
        Name = "avr155_izquierda";
        topicName = clientId + topicIzquierda;
        break;
	  case 16:
        ValueName = "Botón Arriba";
        Name = "avr155_arriba";
        topicName = clientId + topicArriba;
        break;
	  case 17:
        ValueName = "Botón Abajo";
        Name = "avr155_abajo";
        topicName = clientId + topicAbajo;
        break;
	  case 18:
        ValueName = "Modo DTS";
        Name = "avr155_dts";
        topicName = clientId + topicDts;
        break;
	  case 19:
        ValueName = "Modo DTSneo";
        Name = "avr155_dtsneo";
        topicName = clientId + topicDtsNeo;
        break;
    case 20:
      ValueName = "Modo Reposo";
      Name = "avr155_sleep";
      topicName = clientId + topicSleep;
      break;
    case 21:
      ValueName = "Modo Hdmi1";
      Name = "avr155_hdmi1";
      topicName = clientId + topicHdmi1;
      break;
    case 22:
      ValueName = "Modo Canal";
      Name = "avr155_ch";
      topicName = clientId + topicCanal;
      break;
    case 23:
      ValueName = "Botón SURR";
      Name = "avr155_surr";
      topicName = clientId + topicSurr;
      break;
    }
    delay (1);
  }
  mqttclient.publish(topicSt.c_str(), autoDetec ? "online" : "offline", MQTT_RETAIN);

}

//************************************************************************************
// Recepción mqtt
//************************************************************************************
void mqttCallback(char *topic, byte *message, unsigned int leng) {
  String messageRx;
#if DebugSerial
  Serial.print("RX mqtt: ");
  Serial.print(topic);
  Serial.print(" mensaje: ");
#endif
  for (int i = 0; i < leng; i++) {
#if DebugSerial
    if (leng-1 != i)
      Serial.print((char)message[i]);
    else
      Serial.println((char)message[i]);
#endif
    messageRx += (char)message[i];
  }

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
  //RX->AVR155-c7ec9c/switch/avr155_opcion/set
  String ComandSwitch = clientId + topicOn + "/set";
  if ((String(topic) == ComandSwitch) && (messageRx == "1")) {
    CodeAvrTX(0, CodeOn, 0);
#if DebugSerial
    Serial.println ("Orden ON");
#endif
    return;
  }
  ComandSwitch = clientId + topicOff + "/set";
  if ((String(topic) == ComandSwitch) && (messageRx == "1")) {
    CodeAvrTX(0, CodeOff, 0);
#if DebugSerial
    Serial.println ("Orden OFF");
#endif
    return;
  }
  ComandSwitch = clientId + topicMute + "/set";
  if ((String(topic) == ComandSwitch) && (messageRx == "1")) {
    CodeAvrTX(0, CodeMute, 0);
#if DebugSerial
    Serial.println ("Mute");
#endif
    return;
  }
  ComandSwitch = clientId + topicAvr + "/set";
  if ((String(topic) == ComandSwitch) && (messageRx == "1")) {
      if (mode == 1 || mode == 2) {
        mode == 1 ? ComandSwitch = clientId + topicTape : ComandSwitch = clientId + topicAux;
        mqttclient.publish(ComandSwitch.c_str(), "0");
      }
    CodeAvrTX(0, CodeAvr, 0);
    mode = 3;
#if DebugSerial
    Serial.println ("Modo AVR");
#endif
    return;
  }
  ComandSwitch = clientId + topicDvd + "/set";
  if ((String(topic) == ComandSwitch) && (messageRx == "1")) {
    if (mode == 1 || mode == 2) {
      mode == 1 ? ComandSwitch = clientId + topicTape : ComandSwitch = clientId + topicAux;
      mqttclient.publish(ComandSwitch.c_str(), "0");
    }
    CodeAvrTX(0, CodeDvd, 0);
    mode = 4;
#if DebugSerial
    Serial.println ("Entrada DVD");
#endif
    return;
  }
  ComandSwitch = clientId + topicCd + "/set";
  if ((String(topic) == ComandSwitch) && (messageRx == "1")) {
    if (mode == 1 || mode == 2) {
      mode == 1 ? ComandSwitch = clientId + topicTape : ComandSwitch = clientId + topicAux;
      mqttclient.publish(ComandSwitch.c_str(), "0");
    }
    CodeAvrTX(0, CodeCd, 0);
    mode = 5;
#if DebugSerial
    Serial.println("Entrada CD");
#endif
    return;
  }
  ComandSwitch = clientId + topicVideo1 + "/set";
  if ((String(topic) == ComandSwitch) && (messageRx == "1")) {
    if (mode == 1 || mode == 2) {
      mode == 1 ? ComandSwitch = clientId + topicTape : ComandSwitch = clientId + topicAux;
      mqttclient.publish(ComandSwitch.c_str(), "0");
    }
    CodeAvrTX(0, CodeVid1, 0);
    mode = 6;
#if DebugSerial
    Serial.println ("Entrada Video1");
#endif
    return;
  }
  ComandSwitch = clientId + topicHdmi1 + "/set";
  if ((String(topic) == ComandSwitch) && (messageRx == "1")) {
    if (mode == 1 || mode == 2) {
      mode == 1 ? ComandSwitch = clientId + topicTape : ComandSwitch = clientId + topicAux;
      mqttclient.publish(ComandSwitch.c_str(), "0");
    }
    CodeAvrTX(0, CodeHdmi1, 0);
    mode = 7;
#if DebugSerial
    Serial.println ("Entrada HDMI1");
#endif
    return;
  }
  ComandSwitch = clientId + topicAux + "/set";
  if (String(topic) == ComandSwitch) {
    if (mode == 1) {
      ComandSwitch = clientId + topicTape;
      mqttclient.publish(ComandSwitch.c_str(), "0");
    }
    CodeAvrTX(0, CodeAux, 0);
    ComandSwitch = clientId + topicAux;
    mqttclient.publish(ComandSwitch.c_str(), "1");
    mode = 2;
#if DebugSerial
    Serial.println ("Entrada AUX");
#endif
    return;
  }
  ComandSwitch = clientId + topicTape + "/set";
  if (String(topic) == ComandSwitch) {
    if (mode == 2) {
      ComandSwitch = clientId + topicAux;
      mqttclient.publish(ComandSwitch.c_str(), "0");
    }
    CodeAvrTX(0, CodeTape, 0);
    ComandSwitch = clientId + topicTape;
    mqttclient.publish(ComandSwitch.c_str(), "1");
    mode = 1;
#if DebugSerial
    Serial.println ("Entrada TAPE");
#endif
    return;
  }
  ComandSwitch = clientId + topicSleep + "/set";
  if ((String(topic) == ComandSwitch) && (messageRx == "1")) {
    CodeAvrTX(0, CodeSleep, 0);
#if DebugSerial
    Serial.println ("Orden Reposo");
#endif
    return;
  }
  ComandSwitch = clientId + topicTest + "/set";                                       // NO Discovery
  if ((String(topic) == ComandSwitch) && (messageRx == "1")) {
    CodeAvrTX(0, CodeTest, 0);
#if DebugSerial
    Serial.println ("Orden Test");
#endif
    return;
  }
  ComandSwitch = clientId + topicDtsNeo + "/set";
  if ((String(topic) == ComandSwitch) && (messageRx == "1")) {
    CodeAvrTX(1, Code6ch, 0);
#if DebugSerial
    Serial.println ("Orden 6CH");
#endif
    return;
  }
  ComandSwitch = clientId + topicSurr + "/set";                                       // NO Discovery
  if ((String(topic) == ComandSwitch) && (messageRx == "1")) {
    CodeAvrTX(1, CodeSurr, 0);
#if DebugSerial
    Serial.println ("Orden Dolby Surround");
#endif
    return;
  }
  ComandSwitch = clientId + topicOsd + "/set";                                        // NO Discovery
  if ((String(topic) == ComandSwitch) && (messageRx == "1")) {
    CodeAvrTX(1, CodeOsd, 0);
#if DebugSerial
    Serial.println ("Orden Mensajes TFT");
#endif
    return;
  }
  ComandSwitch = clientId + topicDim + "/set";                                        // NO Discovery
  if ((String(topic) == ComandSwitch) && (messageRx == "1")) {
    CodeAvrTX(0, CodeDim, 0);
#if DebugSerial
    Serial.println ("Orden luz display");
#endif
    return;
  }
  ComandSwitch = clientId + topicUpvol + "/set";
  if ((String(topic) == ComandSwitch) && (messageRx == "1")) {
    CodeAvrTX(0, CodeIncvol, 0);
#if DebugSerial
    Serial.println ("Incremento volumen");
#endif
    return;
  }
  ComandSwitch = clientId + topicLowvol + "/set";
  if ((String(topic) == ComandSwitch) && (messageRx == "1")) {
    CodeAvrTX(0, CodeDecvol, 0);
#if DebugSerial
    Serial.println ("Decremento volumen");
#endif
    return;
  }
  ComandSwitch = clientId + topicStereo + "/set";                                     // NO Discovery
  if ((String(topic) == ComandSwitch) && (messageRx == "1")) {
    CodeAvrTX(1, CodeStereo, 0);
#if DebugSerial
  Serial.println("Orden Stereo");
#endif
    return;
  }
  ComandSwitch = clientId + topicArriba + "/set";
  if ((String(topic) == ComandSwitch) && (messageRx == "1")) {
    CodeAvrTX(1, CodeArriba, 0);
#if DebugSerial
    Serial.println ("Botón Subir");
#endif
    return;
  }
  ComandSwitch = clientId + topicAbajo + "/set";
  if ((String(topic) == ComandSwitch) && (messageRx == "1")) {
    CodeAvrTX(1, CodeAbajo, 0);
#if DebugSerial
    Serial.println ("Botón Bajar");
#endif
    return;
  }
ComandSwitch = clientId + topicDerecha + "/set";
  if ((String(topic) == ComandSwitch) && (messageRx == "1")) {
    CodeAvrTX(1, CodeDerecha, 0);
#if DebugSerial
    Serial.println ("Botón Derecha");
#endif
    return;
  }
ComandSwitch = clientId + topicIzquierda + "/set";
  if ((String(topic) == ComandSwitch) && (messageRx == "1")) {
    CodeAvrTX(1, CodeIzquierda, 0);
#if DebugSerial
    Serial.println ("Botón Izquierda");
#endif
    return;
  }
ComandSwitch = clientId + topicOK + "/set";
  if ((String(topic) == ComandSwitch) && (messageRx == "1")) {
    CodeAvrTX(1, CodeOk, 0);
#if DebugSerial
    Serial.println ("Botón OK");
#endif
    return;
  }
ComandSwitch = clientId + topicDts + "/set";
  if ((String(topic) == ComandSwitch) && (messageRx == "1")) {
    CodeAvrTX(0, CodeDts, 0);
#if DebugSerial
    Serial.println ("Botón DTS");
#endif
    return;
  }
  ComandSwitch = clientId + topicDigital + "/set";
  if ((String(topic) == ComandSwitch) && (messageRx == "1")) {
    CodeAvrTX(1, CodeDigital, 0);
#if DebugSerial
    Serial.println ("Botón digital");
#endif
    return;
  }
	ComandSwitch = clientId + topicNoche + "/set";
  if ((String(topic) == ComandSwitch) && (messageRx == "1")) {
    CodeAvrTX(1, CodeNoche, 0);
#if DebugSerial
    Serial.println ("Modo Noche");
#endif
    return;
  }
  ComandSwitch = clientId + topicAmFm + "/set";
  if ((String(topic) == ComandSwitch) && (messageRx == "1")) {
    if (mode == 1 || mode == 2) {
      mode == 1 ? ComandSwitch = clientId + topicTape : ComandSwitch = clientId + topicAux;
      mqttclient.publish(ComandSwitch.c_str(), "0");
    }
    CodeAvrTX(0, CodeAmFm, 0);
    mode = 8;
#if DebugSerial
    Serial.println("Modo Am-Fm");
#endif
    return;
  }
  ComandSwitch = clientId + topicCanal + "/set";
  if ((String(topic) == ComandSwitch) && (messageRx == "1")) {
    CodeAvrTX(1, CodeCh, 0);
#if DebugSerial
    Serial.println("Modo Canal");
#endif
    return;
  }
  ComandSwitch = clientId + topicSurr + "/set";
  if ((String(topic) == ComandSwitch) && (messageRx == "1")) {
    CodeAvrTX(0, CodeSurr, 0);
#if DebugSerial
    Serial.println("Botón SURR");
#endif
    return;
  }
#if DebugSerial
      Serial.print("MQTT SIN acción: ");
      Serial.println(String(topic));
#endif
}

//************************************************************************************
// Conexión o reconexión al servidor mqtt
//************************************************************************************
bool mqttConnect() {
#if DebugSerial
  Serial.println ("Reconexión..");
#endif
  if (!mqttclient.connect((char *)clientId.c_str(), mqttUser, mqttPassword, willTopic, MQTT_QOS, MQTT_RETAIN, willMessage, 0)){
#if DebugSerial
    Serial.print("Fallo al Reconectar al broker, rc="); // -2 fallo de conexión
    Serial.println (mqttclient.state());
#endif
    return false;
  }
  mqttclient.publish(topicSt.c_str(), "online", MQTT_RETAIN);
  const String SubTopic1 = clientId + "/switch/#";
  if (!mqttclient.subscribe(SubTopic1.c_str(), 0)) {
#if DebugSerial
  Serial.println("MQTT ERROR suscripción SWITCH");
#endif
  }
  const String SubTopic2 = clientId + "/cmd/#";
  if (!mqttclient.subscribe(SubTopic2.c_str(), 0)) {
#if DebugSerial
    Serial.println("MQTT ERROR suscripción CMD");
#endif
  }
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
    if (!mqttclient.subscribe(SubTopic1.c_str(), 0)) {
#if DebugSerial
      Serial.println("MQTT ERROR suscripción SWITCH");
    }
    else {
    Serial.print(F("MQTT Subcrito a: "));
    Serial.println (SubTopic1);
#endif
    }
    const String SubTopic2 = clientId  + "/cmd/#";
    if (!mqttclient.subscribe(SubTopic2.c_str(), 0)) {
#if DebugSerial
      Serial.println("MQTT ERROR suscripción CMD");
    }
    else {
      Serial.print("MQTT Subcrito a: ");
      Serial.println (SubTopic2);
#endif
    }
  }
#if DebugSerial
  else {
    Serial.print ("Fallo al conectar al broker, rc=");
    Serial.println (mqttclient.state());
  }
#endif
}

#endif
