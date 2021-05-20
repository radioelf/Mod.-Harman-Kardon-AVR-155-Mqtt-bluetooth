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
#define PASS "esp32AvR155"                                                            // Contraseña del punto de acceso si falla la conexión a la red WiFi

const char*       ssid = "nombre_wifi";                                     		        // Nombre del punto de accesos a conectar
const char*       password =  "contrasenya";                               		        // Contraseña para el punto de accesos a conectar        
bool              NetworkFound = false;                                   		        // True si la red WiFi está conectada
bool              APmode = false;                                         		        // Indicación modo Ap
bool              http_response_flag = false;                             		        // Solicitud de respuesta
struct tm         timeinfo;

// Configuración ip estática
IPAddress local_IP(192, 168, x, xxx);
IPAddress gateway(192, 168, x, xxx);
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
    Serial.println("Ningún AP encontrado");
    return OK;
  }
  for (uint8_t i = 0; i < numSsid; i++) {
    if (strcmp (WiFi.SSID(i).c_str(), ssid) == 0) {
      OK = true;
      if (APmode) {
        WiFi.softAPdisconnect(true);
        APmode = false;
      }
      Serial.println("AP valido encontrado");
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
  error = 95;
  Serial.println("Configurado modo AP");
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
      Serial.println("Fallo la configuración WIFI...");
    }
    WiFi.begin (ssid, password);                                            	        // Conectar a SSID único
    uint8_t connetCount = 0;
    while (WiFi.waitForConnectResult() != WL_CONNECTED) {                      	      // Tratar de conectar
      delay(1000);
      Serial.println("Fallo en la conexión. Reconectando...");
      if (++connetCount == 20) break;
    }
    if (connetCount == 20) {                      	                                  // Si no conectamos
      if (!APmode){
        modeAP();
        Serial.println(F("No conectado, modo AP"));
        return false;                                                                 // Error al conectar, configuración AP local  
      }                                      
    } else {
      APmode = false;
      Serial.println(F("conectado"));
      Serial.print (F("Conectado a SSID: "));
      Serial.println (ssid);
      Serial.print (F("IP: "));
      Serial.println (WiFi.localIP());
     
    }
  } else {
    if (!APmode)
      modeAP();                                                               	      // ISSD NO encontrado, configuración AP local
    Serial.println("Forzar modo AP");
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
String outputState(int output){
  Serial.print ("RX orden WEB: ");
  Serial.println(output);
  if(digitalRead(output)){
    //return "checked";
    return "";
  }
  else {
    return "";
  }
}

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
  Serial.print ("RX valor actualización WEB: ");
  Serial.println(var);
  if(var == "BUTTONPLACEHOLDER"){
    String buttons = "";
    buttons += "<h4>Pulsador - ON</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"1\" " + outputState(1) + "><span class=\"slider\"></span></label>";
    buttons += "<h4>Pulsador - OFF</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"2\" " + outputState(2) + "><span class=\"slider\"></span></label>";
    buttons += "<h4>Pulsador - MUTE</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"3\" " + outputState(3) + "><span class=\"slider\"></span></label>";
    buttons += "<h4>Pulsador - AVR</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"4\" " + outputState(4) + "><span class=\"slider\"></span></label>";
    buttons += "<h4>Pulsador - DVD</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"5\" " + outputState(5) + "><span class=\"slider\"></span></label>";
    buttons += "<h4>Pulsador - AM/FM</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"6\" " + outputState(6) + "><span class=\"slider\"></span></label>";
    buttons += "<h4>Pulsador - CD</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"7\" " + outputState(7) + "><span class=\"slider\"></span></label>";
    buttons += "<h4>Pulsador - HDMI</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"8\" " + outputState(8) + "><span class=\"slider\"></span></label>";
    buttons += "<h4>Pulsador - AUX</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"9\" " + outputState(9) + "><span class=\"slider\"></span></label>";
    buttons += "<h4>Pulsador - TAPE</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"10\" " + outputState(10) + "><span class=\"slider\"></span></label>";
    buttons += "<h4> Bajar VOLUMENT</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"11\" " + outputState(11) + "><span class=\"slider\"></span></label>";
    buttons += "<h4> Subir VOLUMENT</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"12\" " + outputState(12) + "><span class=\"slider\"></span></label>";

    return buttons;
  }
  return String();
}

void iniWebServer() {
  Serial.print("Acceso a pagina WEB: ");
  Serial.println(WiFi.localIP());

  // Ruta para la raíz / de la página web
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

  // Envíe una solicitud GET a <dirección ip>/update?output=<inputMessage1>&state=<inputMessage2>
  server.on("/update", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage1;
    String inputMessage2;
    // GET input1 valor a on <dirección ip>/update?output=<inputMessage1>&state=<inputMessage2>
    if (request->hasParam(PARAM_INPUT_1) && request->hasParam(PARAM_INPUT_2)) {
      inputMessage1 = request->getParam(PARAM_INPUT_1)->value();
      inputMessage2 = request->getParam(PARAM_INPUT_2)->value();

      // orden ->
      digitalWrite(inputMessage1.toInt(), inputMessage2.toInt());
    }
    else {
      inputMessage1 = "SIN mensaje";
      inputMessage2 = "SIN mensaje";
    }
    Serial.print("Botón: ");
    Serial.print(inputMessage1);
    Serial.print(" - pasando a: ");
    Serial.println(inputMessage2);
    request->send(200, "text/plain", "OK");
  });

  // Start server
  server.begin();

}


#endif

