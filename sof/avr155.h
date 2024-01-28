#ifndef _AVR155_
#define _AVR155_
/*
 El equipo AVR155 utiliza el protocolo es el NEC extendido para recibir ordenes a 
 traves del mando distancia de infarrrojos.
Los dos primeros bytes son el codigo fabricante, byte 3 codigo y byte 4 codigo 
Trama 	1 start bit 9ms + off 4.5ms + 4bytes datos
Bit a 0 	560µs pulso, 560µs pausa ->1.125ms
Bit a 1 	560µs pulso, 1690µs pausa ->2,25ms
Repetición tecla: espera de 50ms + 9ms pulso, 2.250ms pausa, 560µs pulso, ~100ms pausa
Periodo reposo entre transmisiones: 50ms

Codigo envio al  microcontrolador orden ON (IC90 T5CC1 pin 35, R476)
                                        <-bit0   <-bit8  <-bit16  <-bit34
                                            v        v        v        v
       on      80    70   C0  3F     10000000 01110000 11000000 00111111

*/

#include <Arduino.h>

const uint16_t direccion1 = 0x7080;                                                   // 0111000010000000
const uint16_t direccion2 = 0x7282;                                                   // 0111001010000010

const uint8_t CodeOn = 0xC0;                                                          // direccion1
const uint8_t CodeOff = 0x9F;                                                         // direccion1
const uint8_t CodeMute = 0xC1;                                                        // direccion1
const uint8_t CodeAvr = 0xC0;                                                         // direccion1
const uint8_t CodeDvd = 0xD0;                                                         // direccion1
const uint8_t CodeCd = 0xC4;                                                          // direccion1
const uint8_t CodeVid1 = 0xCA;                                                        // direccion1
const uint8_t CodeVid2 = 0xCB;                                                        // direccion1
const uint8_t CodeVid3 = 0xCE;                                                        // direccion1
const uint8_t CodeHdmi1 = 0xF2;                                                       // direccion1
const uint8_t codeHdmi2 = 0xF3;                                                       // direccion1
const uint8_t CodeHdmi3 = 0xFC;                                                       // direccion1
const uint8_t CodeAux = 0xCD;                                                         // direccion1
const uint8_t CodeTape = 0xCC;                                                        // direccion1
const uint8_t CodeSleep = 0xDB;                                                       // direccion1
const uint8_t CodeAmFm = 0x81;                                                        // direccion1
const uint8_t CodeTest = 0x8C;                                                        // direccion1
const uint8_t CodeIncvol = 0xC7;                                                      // direccion1
const uint8_t CodeDecvol = 0xC8;                                                      // direccion1
const uint8_t CodeSpkr = 0x53;                                                        // direccion1
const uint8_t Code1 = 0x87;                                                           // direccion1
const uint8_t Code2 = 0x88;                                                           // direccion1
const uint8_t Code3 = 0x89;                                                           // direccion1
const uint8_t Code4 = 0x8A;                                                           // direccion1
const uint8_t Code5 = 0x8B;                                                           // direccion1
const uint8_t Code6 = 0x8C;                                                           // direccion1
const uint8_t Code7 = 0x8D;                                                           // direccion1
const uint8_t Code8 = 0x8E;                                                           // direccion1
const uint8_t Code9 = 0x9D;                                                           // direccion1
const uint8_t Code0 = 0x9E;                                                           // direccion1
const uint8_t CodeTunm = 0x93;                                                        // direccion1
const uint8_t CodeMen = 0x86;                                                         // direccion1
const uint8_t CodeIncTuning = 0x84;                                                   // direccion1
const uint8_t CodeDecTuning = 0x85;                                                   // direccion1
const uint8_t CodeDirect = 0x9B;                                                      // direccion1
const uint8_t CodeDim = 0xDC;                                                         // direccion1

const uint8_t CodeDts = 0xA0;                                                         // direccion2
const uint8_t CodeDolby = 0x50;                                                       // direccion2
const uint8_t CodeIncPreset = 0x0B;                                                   // direccion2
const uint8_t CodeSkipSwn = 0x0C;                                                     // direccion2
const uint8_t CodeSurr = 0x58;                                                        // direccion2
const uint8_t CodeCh = 0x5D;                                                          // direccion2
const uint8_t CodeOk = 0x84;                                                          // direccion2
const uint8_t Code6ch = 0xDB;                                                         // direccion2
const uint8_t CodeOsd = 0x5C;                                                         // direccion2
const uint8_t CodeDerecha = 0xc2;                                                     // direccion2
const uint8_t CodeLogic = 0xA2;                                                       // direccion2
const uint8_t CodeDelay = 0xD9;                                                       // direccion2
const uint8_t CodeNoche = 0x96;                                                       // direccion2
const uint8_t CodeArriba = 0x99;                                                      // direccion2
const uint8_t CodeIzquierda = 0xC1;                                                   // direccion2
const uint8_t CodeDtsNeo = 0xA1;                                                      // direccion2
const uint8_t CodeDecPreset = 0x8B;                                                   // direccion2
const uint8_t CodeSkipUp = 0x8C;                                                      // direccion2
const uint8_t CodeBorrar = 0x9B;                                                      // direccion2
const uint8_t CodeDigital = 0x54;                                                     // direccion2
const uint8_t CodeTone = 0xC5;                                                        // direccion2
const uint8_t CodeRds = 0xDD;                                                         // direccion2
const uint8_t CodeStereo = 0x9B;                                                      // direccion2
const uint8_t CodeAbajo = 0x9A;                                                       // direccion2

// Envio de 32 bytes al microcontrolador T5CC1 del AVR155
void CodeAvrTX (bool adders, uint8_t command, uint8_t repit) {
uint8_t i, x = 0;
uint16_t ini;

#if DebugSerial > 1
    Serial.print("Enviar datos a cpu AVR155, direccion: ");
    Serial.print(adders);
    Serial.print(" orden: ");
    Serial.print(command);
    Serial.print(" repeticiones: ");
    Serial.println(repit);
#endif
    adders ? ini = direccion2 : ini = direccion1;
    digitalWrite(ControlPin, HIGH);                                                   // Desconectamos señal receptor infrarrojos a CPU AVR155
#if StatusLed
    digitalWrite(LedPin, HIGH);
#endif
    //pinMode(DataPin, OUTPUT_OPEN_DRAIN);                                            // pin como salida (colector abierto)
    pinMode(DataPin, OUTPUT);
    digitalWrite(DataPin, LOW);
    delayMicroseconds(9000); // 9ms a 0
    digitalWrite(DataPin, HIGH);
    delayMicroseconds(4500); // 4.5ms a 1
    for (i = 0; i < 16; i++) {
        digitalWrite(DataPin, LOW);
        delayMicroseconds(560); // 560us a 0
        digitalWrite(DataPin, HIGH);
        bitRead(ini, i) ? delayMicroseconds(1690) : delayMicroseconds(560);           // 1->1690us 0->560us a 1
    }
    do {
        for (i = 0; i < 8; i++) {
            digitalWrite(DataPin, LOW);
            delayMicroseconds(560);                                                   // 560us a 0
            digitalWrite(DataPin, HIGH);
            bitRead(command, i) ? delayMicroseconds(1690) : delayMicroseconds(560);   // 1->1690us 0->560us a 1
            if (x == 1 && i ==7) digitalWrite(DataPin, LOW);                          // último bit a enviar
        }
        command = ~command;                                                           // complemento (invertimos byte)
    } while (++x == 1);
    delayMicroseconds(560);
    digitalWrite(DataPin, HIGH);
    if (repit != 0) {
        for (i = 0; i < repit; i++) {
            delay (50);                                                               // 50ms
            digitalWrite(DataPin, LOW);
            delayMicroseconds(9000);                                                  // 9ms a 0
            digitalWrite(DataPin, HIGH);
            delayMicroseconds(2250);                                                  // 2.250us a 1
            digitalWrite(DataPin, LOW);
            delayMicroseconds(560);                                                   // 560us a 0
            digitalWrite(DataPin, HIGH);
            delay(100);      
        }
    }
#if StatusLed
    digitalWrite(LedPin, LOW);
#endif
    pinMode(DataPin, INPUT);
    digitalWrite(ControlPin, LOW);                                                    // Conectamos señal receptor infrarrojos a CPU AVR155
}
#endif
