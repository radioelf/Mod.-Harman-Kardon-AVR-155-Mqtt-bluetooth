#ifndef _DAC_
#define _DAC_
/*
Modulo adafruit UDA1334a
VIN - pin y GND entrada 5V 
3VO - salida regulada de 3V3
Alimentación a 3v, señales lógicas 3v3 o 5V a excepción de PLL y SF0 de solo 3v3
WSEL - (selección de  reloj izquierdo/derecho): este es el pin que le dice al DAC 
    cuándo los datos son para el canal izquierdo o cuándo son para el canal derecho
DIN - este es el pin que recibe los datos reales, tanto los datos 
    de la izquierda como de la derecha, el pin WSEL indica cuándo se está transmitiendo a 
    la izquierda o la derecha
BCLK - este es el pin le indica al amplificador cuándo leer datos en el pin de datos.
**--OPCIONALES--**
MCLK - NO USADO
SCLK - entrada opcional de reloj del sistema de modo de video de 27 MHz; de
    forma predeterminada, generamos el reloj del sistema a partir del reloj WS en modo de 
    audio, pero el UDA también puede tomar una entrada de oscilador en este pin
Mute - este pin en alto silenciará la salida
DEEM - en el modo de audio (predeterminado), se puede usar para agregar un filtro de 
    énfasis. En el modo de video, donde el reloj del sistema se genera a partir de un 
    oscilador, esta es la salida del reloj.
PLL - establece el modo PLL, de forma predeterminada a nivel bajo para Audio. 
    Puede configurarse a pulled high o ~ 1.6V para configurar la frecuencia de video PAL o NTSC
SF0 y SF1 se utilizan para establecer el formato de datos de entrada
    SF0  SF1
     0    0   I2S
     0    1   LSB 16bits
     1    0   LSB 20bits
     1    1   LSB 24bits
AGND - masa salida audio
Rout - salida audio canal derecha (3 KΩ)
Lout - salida audio canal izquierdo (3 KΩ)

info: https://github.com/pschatzmann/ESP32-A2DP   
*/

#include "esp32_bt_music_receiver.h"                                                  // https://github.com/dvxlab/esp32_bt_music_receiver 
                                   
// I2S_BCLK      26
// I2S WSEL      25
// I2S DIN       22

BlootoothA2DSink a2d_sink;

#endif
