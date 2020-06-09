/*  Header for LED module

    Computerarchitektur / Computer Architecture
    (C) 2020 J. Friedrich, W. Zimmermann 
    Hochschule Esslingen

    Author:   W.Zimmermann, May 06, 2020
    Modified: 
*/

// Public functions, for details see led.asm
void initLED(void);
void toggleLED(unsigned char mask);
void setLED(unsigned char mask);
void clrLED(unsigned char mask);
