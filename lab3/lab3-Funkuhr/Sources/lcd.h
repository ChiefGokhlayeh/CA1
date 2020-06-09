/*  Header for LCD module

    Computerarchitektur / Computer Architecture
    (C) 2020 J. Friedrich, W. Zimmermann 
    Hochschule Esslingen

    Author:   W.Zimmermann, May 06, 2020
    Modified: 
*/

// Public functions, for details see lcd.asm
void initLCD(void);
void writeLine(char* text, unsigned char zeilennummer);
void delay_10ms(void);
