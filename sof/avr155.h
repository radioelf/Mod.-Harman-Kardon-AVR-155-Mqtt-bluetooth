#ifndef _AVR155_
#define _AVR155_
/*
Protocolo infrarrojos usado por el AVR155 es el NEC extendido
Trama 	1 start bit 9ms + off 4.5ms + 4bytes datos (los dos primeros bytes código fabricante, byte 3 código y byte 4 código invertido)
Bit a 0 	560µs pulso, 560µs pausa ->1.125ms
Bit a 1 	560µs pulso, 1690µs pausa ->2,25ms
repetición tecla espera de 50ms + 9ms pulso, 2.250ms pausa, 560µs pulso, ~100ms pausa
periodo reposo entre transmisiones 50ms

Código envió al  micro controlador orden ON (IC90 T5CC1 pin 35, R476)
                                        <-bit0   <-bit8  <-bit16  <-bit34
                                            v        v        v        v
       on      80    70   C0  3F     10000000 01110000 11000000 00111111

*/

#include <Arduino.h>

const uint16_t direccion1 = 0x7080;                                                   // 0111000010000000
const uint16_t direccion2 = 0x7282;                                                   // 0111001010000010

const uint8_t CodeOn = 0xC0;
const uint8_t CodeOff = 0x9F;
const uint8_t codeMute = 0xC1;
const uint8_t codeAvr = 0x35;
const uint8_t codeDvd = 0xD0;
const uint8_t codeCd = 0xC4;
const uint8_t codeVid1 = 0x53;
const uint8_t codeVid2 = 0xD3;
const uint8_t codeVid3 = 0x73;
const uint8_t codeHdmi1 = 0x4F;
const uint8_t codeHdmi2 = 0xCF;
const uint8_t codeHdm13 = 0x3F;
const uint8_t codeAux = 0xB3;
const uint8_t codeTape = 0x33;
const uint8_t codeSleep = 0xDB;
const uint8_t codeTest = 0x31;
const uint8_t code6ch = 0xDB;
const uint8_t codeSurr = 0x1A;
const uint8_t codeOsd = 0x3A;
const uint8_t codeIncvol = 0xE3;
const uint8_t codeDecvol = 0x13;
const uint8_t codeCh = 0xBA;
const uint8_t codeSpkr = 0xCA;
const uint8_t codeArriba = 0x71;
const uint8_t codeAbajo = 0xF1;
const uint8_t codeDerecha = 0x43;
const uint8_t codeIzquierda = 0x83;
const uint8_t codeOk = 0x21;
const uint8_t codeDigital = 0xA2;
const uint8_t codeDelay = 0x4A;
const uint8_t code1 = 0x87;
const uint8_t code2 = 0x88;
const uint8_t code3 = 0x89;
const uint8_t code4 = 0x8A;
const uint8_t code5 = 0x8B;
const uint8_t code6 = 0x8C;
const uint8_t code7 = 0x8D;
const uint8_t code8 = 0x8E;
const uint8_t code9 = 0x9D;
const uint8_t code0 = 0x9E;
const uint8_t codeTunm = 0xC9;
const uint8_t codeMen = 0x61;
const uint8_t codeIncTuning = 0x21;
const uint8_t codeDecTuning = 0xA1;
const uint8_t codeDirect = 0xD9;
const uint8_t codeBorrar = 0x9B;
const uint8_t codeIncPreset = 0x0B;
const uint8_t codeDecPreset = 0x8B;
const uint8_t codeTone = 0xA3;
const uint8_t codeRds = 0xBB;
const uint8_t codeDolby = 0x0A;
const uint8_t codeDts = 0x05;
const uint8_t codeDtsNeo = 0x85;
const uint8_t codeNoche = 0x69;
const uint8_t codeLogic = 0x45;
const uint8_t codeStereo = 0xD9;
const uint8_t codeSkipSwn = 0x0C;
const uint8_t codeSkipUp = 0x8C;
const uint8_t codeDim = 0x3B;

// Envió de 32 bytes al micro controlador T5CC1 del AVR155
void CodeAvrTX (bool adders, uint8_t command) {
uint8_t i, x = 0;
uint16_t ini;
    adders ? ini = direccion2: ini = direccion1;
    pinMode(DataPin, OUTPUT_OPEN_DRAIN);                                              // pin como salida
    digitalWrite(DataPin, LOW);
    delayMicroseconds(9000);                                                          // 9ms a 0
    digitalWrite(DataPin, HIGH);
    delayMicroseconds(4500);                                                          // 4.5ms a 1
    for (i = 0; i < 16; i++) {
        digitalWrite(DataPin, LOW);
        delayMicroseconds(560);                                                       // 560us a 0
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
    pinMode(DataPin, INPUT);
}

#endif
