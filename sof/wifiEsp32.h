#ifndef _WifiEsp32_
#define _WifiEsp32_
/*
Gestión del uso del modulo wifi, conexión, escaneo, modo estación, modo punto de acceso

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
#include <ESPAsyncWebServer.h>                                           		          // Librería para implementar un servidor web https://github.com/me-no-dev/ESPAsyncWebServer

#define NAME "AVR155"                                                   		          // Nombre del punto de acceso de la red WiFi
#define PASS "xxxxxxx"                                                            // Contraseña del punto de acceso si falla la conexión a la red WiFi

const char*       ssid = "xxxxxxxx";                                     		        // Nombre del punto de accesos a conectar
const char*       password =  "xxxxxxxxx";                               		        // Contraseña para el punto de accesos a conectar        
bool              NetworkFound = false;                                   		        // True si la red WiFi está conectada
bool              APmode = false;                                         		        // Indicación modo Ap
bool              http_response_flag = false;                             		        // Solicitud de respuesta
struct tm         timeinfo;

// Configuración ip estatica
IPAddress local_IP(192, 168, 0, 93);
IPAddress gateway(192, 168, 0, 50);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);   

AsyncWebServer server(80);
const char* PARAM_INPUT_1 = "output";
const char* PARAM_INPUT_2 = "state";

void callback(char* topic, byte* payload, unsigned int length);

//************************************************************************************
// Listar las redes disponibles.
// Las redes aceptables son aquellas que tienen una entrada en las preferencias.
// Los SSID de las redes disponibles se guardarán para su uso en la interfaz web.
//************************************************************************************
bool listNetworks() {
  bool OK = false;
  int8_t  numSsid;
  numSsid = WiFi.scanNetworks();

  if (numSsid <= 0) {
#if DebugSerial
    Serial.println("Ningun AP encontrado");
#endif
    return OK;
  }
  for (uint8_t i = 0; i < numSsid; i++) {
    if (strcmp (WiFi.SSID(i).c_str(), ssid) == 0) {
      OK = true;
      if (APmode) {
        WiFi.softAPdisconnect(true);
        APmode = false;
      }
#if DebugSerial
      Serial.println("AP valido encontrado");
#endif
    }
  }
  return OK;
}
//************************************************************************************
// Activamos y configuramos el modo AP
//************************************************************************************
void modeAP() {
  // Canal RF 6, ISSD ON, 1 conexión
  WiFi.softAP(NAME, PASS, 6, 0, 1);                                         	        // Este ESP32 será un AP
  NetworkFound = false;
  APmode = true;
#if DebugSerial
  Serial.println("Configurado modo AP");
#endif
}
//************************************************************************************
// Conectar a WiFi usando el SSID
// Si la conexión falla, se crea un AP y la función devuelve falso.
//************************************************************************************
bool connectwifi(bool connec) {
  if (connec) {
    if (!APmode) {                                                          	        // si NO estamos en modo AP
      WiFi.softAPdisconnect(true);                                          	        // Si todavía se conserva la conexión antigua
    }
    if (!WiFi.config(local_IP, gateway, subnet, primaryDNS)) {
#if DebugSerial
      Serial.println("Fallo la configuración WIFI...");
#endif
    }
    WiFi.begin (ssid, password);                                            	        // Conectar a SSID único
    uint8_t connetCount = 0;
    while (WiFi.waitForConnectResult() != WL_CONNECTED) {                      	      // Tratar de conectar
      delay(1000);
#if DebugSerial
      Serial.println("Fallo en la conexión. Reconectando...");
#endif
      if (++connetCount == 20) break;
    }
    if (connetCount == 20) {                      	                                  // Si no conectamos
      if (!APmode){
        modeAP();
#if DebugSerial
        Serial.println(F("No conectado, modo AP"));
#endif
        return false;                                                                 // Error al conectar, configuración AP local  
      }                                      
    } else {
      APmode = false;
#if DebugSerial
      Serial.println(F("conectado"));
      Serial.print (F("Conectado a SSID: "));
      Serial.println (ssid);
      Serial.print (F("IP: "));
      Serial.println (WiFi.localIP());
#endif
    }
  } else {
    if (!APmode)
      modeAP();                                                               	      // ISSD NO encontrado, configuración AP local
#if DebugSerial
    Serial.println("Forzar modo AP");
#endif
    return false;
  }
  return true;
}
//************************************************************************************
// Configuración conexión WIFI
//************************************************************************************
bool confiWifi() {
  WiFi.disconnect();                                                          	      // Después de reiniciar el router todavía podría estar conectado
  WiFi.mode (WIFI_STA);                                                       	      // Este ESP32 es una estación
  WiFi.persistent (false);                                                    	      // No se guardar SSID y contraseña
  WiFi.setAutoReconnect(true);                                                        // Reconectar si se pierde la conexión
  delay (100);
  NetworkFound = connectwifi(listNetworks());                                 	      // Buscamos redes disponible y conectar a red WiFi
  return NetworkFound;
}
//************************WEB SERVER**************************************************

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>ESP32-AVR155 Web Server</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="icon" href="data:,">
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    h2 {font-size: 3.0rem;}
    p {font-size: 3.0rem;}
    body {max-width: 600px; margin:0px auto; padding-bottom: 25px;}
    .switch {position: relative; display: inline-block; width: 120px; height: 68px} 
    .switch input {display: none}
    .slider {position: absolute; top: 0; left: 0; right: 0; bottom: 0; background-color: #ccc; border-radius: 6px}
    .slider:before {position: absolute; content: ""; height: 52px; width: 52px; left: 8px; bottom: 8px; background-color: #fff; -webkit-transition: .4s; transition: .4s; border-radius: 3px}
    input:checked+.slider {background-color: #b30000}
    input:checked+.slider:before {-webkit-transform: translateX(52px); -ms-transform: translateX(52px); transform: translateX(52px)}
  </style>
</head>
<body>
  <h2>Control AVR155</h2>
  %BUTTONPLACEHOLDER%
<script>function toggleCheckbox(element) {
  var xhr = new XMLHttpRequest();
  if(element.checked){ xhr.open("GET", "/update?output="+element.id+"&state=1", true); setTimeout(function(){location.reload()}, 1000);}
  else { xhr.open("GET", "/update?output="+element.id+"&state=0", true); }
  xhr.send();
}
</script>
</body>
</html>
)rawliteral";

// Marcadores de posición con la sección de botones en su página web
String processor(const String& var){
#if DebugSerial
  Serial.print ("RX valor actualización WEB: ");
  Serial.println(var);
#endif
  if(var == "BUTTONPLACEHOLDER"){
    String buttons = "";
    buttons += "<h4>Pulsador - ON</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"1\"><span class=\"slider\"></span></label>";
    buttons += "<h4>Pulsador - OFF</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"2\"><span class=\"slider\"></span></label>";
    buttons += "<h4>Pulsador - MUTE</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"3\"><span class=\"slider\"></span></label>";
    buttons += "<h4>Pulsador - AVR</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"4\"><span class=\"slider\"></span></label>";
    buttons += "<h4>Pulsador - DVD</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"5\"><span class=\"slider\"></span></label>";
    buttons += "<h4>Pulsador - AM/FM</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"6\"><span class=\"slider\"></span></label>";
    buttons += "<h4>Pulsador - CD</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"7\"><span class=\"slider\"></span></label>";
    buttons += "<h4>Pulsador - HDMI</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"8\"><span class=\"slider\"></span></label>";
    buttons += "<h4>Pulsador - AUX</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"9\"><span class=\"slider\"></span></label>";
    buttons += "<h4>Pulsador - TAPE</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"10\"><span class=\"slider\"></span></label>";
    buttons += "<h4> Bajar VOLUMEN</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"11\"><span class=\"slider\"></span></label>";
    buttons += "<h4> Subir VOLUMEN</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"12\"><span class=\"slider\"></span></label>";

    return buttons;
  }
  return String();
}

void iniWebServer() {
#if DebugSerial
      Serial.print("Acceso a pagina WEB: ");
  Serial.println(WiFi.localIP());
#endif
  // Ruta para la raíz / de la página web
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

  // Envíar una solicitud GET a <dirección ip>/update?output=<inputMessage1>&state=<inputMessage2>
  server.on("/update", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage1;
    String inputMessage2;
    // GET input1 valor a on <dirección ip>/update?output=<inputMessage1>&state=<inputMessage2>
    if (request->hasParam(PARAM_INPUT_1) && request->hasParam(PARAM_INPUT_2)) {
      inputMessage1 = request->getParam(PARAM_INPUT_1)->value();
      inputMessage2 = request->getParam(PARAM_INPUT_2)->value();

      // orden ->
      if (inputMessage2.toInt() == 1) {
        uint8_t orden = inputMessage1.toInt();
        switch (orden) {
          case 1:
            CodeAvrTX(0, CodeOn, 0);
#if DebugSerial
            Serial.println("Orden ON");
#endif
            break;
          case 2:
            CodeAvrTX(0, CodeOff, 0);
#if DebugSerial
            Serial.println("Orden OFF");
#endif
            break;
          case 3:
            CodeAvrTX(0, CodeMute, 0);
#if DebugSerial
            Serial.println("Orden Mute");
#endif
            break;
          case 4:
            CodeAvrTX(0, CodeAvr, 0);
            mode = 30;
#if DebugSerial
            Serial.println("Modo AVR");
#endif
            break;
          case 5:
            CodeAvrTX(0, CodeDvd, 0);
            mode = 40;
#if DebugSerial
            Serial.println("Entrada DVD");
#endif
            break;
          case 6:
            CodeAvrTX(0, CodeAmFm, 0);
            mode = 80;
#if DebugSerial
            Serial.println("Modo Am-Fm");
#endif
            break;
          case 7:
            CodeAvrTX(0, CodeCd, 0);
            mode = 50;
#if DebugSerial
            Serial.println("Entrada CD");
#endif
            break;
          case 8:
            CodeAvrTX(0, CodeHdmi1, 0);
            mode = 70;
#if DebugSerial
            Serial.println("Entrada HDMI1");
#endif
            break;
          case 9:
            CodeAvrTX(0, CodeAux, 0);
            mode = 20;
#if DebugSerial
            Serial.println("Entrada AUX");
#endif
            break;
          case 10:
            CodeAvrTX(0, CodeTape, 0);
            mode = 10;
#if DebugSerial
            Serial.println("Entrada TAPE");
#endif
            break;
          case 11:
            CodeAvrTX(0, CodeDecvol, 0);
#if DebugSerial
            Serial.println("Decremento volumen");
#endif
            break;
          case 12:
            CodeAvrTX(0, CodeIncvol, 0);
#if DebugSerial
            Serial.println("Incremento volumen");
#endif
            break;

        }
        
      }
    }
    else {
      inputMessage1 = "SIN mensaje";
      inputMessage2 = "SIN mensaje";
    }
#if DebugSerial
    Serial.print("Botón: ");
    Serial.print(inputMessage1);
    Serial.print(" - pasando a: ");
    Serial.println(inputMessage2);
#endif
    request->send(200, "text/plain", "OK");
  });

  // Start servidor
  server.begin();

}


#endif

