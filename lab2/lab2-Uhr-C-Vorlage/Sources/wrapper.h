/*  Wrappers for assembler functions in LCD.asm and decToASCII.asm

    Computerarchitektur / Computer Architecture
    (C) 2020 J. Friedrich, W. Zimmermann
    Hochschule Esslingen

    Author:  W.Zimmermann, Jan 30, 2020
*/

#ifndef __wrapper__
#define __wrapper__

// Prototypes and wrapper functions for dectoascii (from lab 1)
void dectoascii(void);

void decToASCII_Wrapper(char *txt, int val)
{
    asm
    {
        LDX txt
        LDD val

        JSR dectoascii

        STD val
        STX txt
    }
}

// Prototypes and wrapper functions for LCD driver (from lab 1)
void initLCD(void);
void delay_10ms(void);
void writeLine(void);

void WriteLine_Wrapper(char *text, char line)
{
    asm
    {
        LDAB line
        LDX text

        JSR writeLine
    }
}

// Prototypes and wrapper functions for ticker (from lab 2)
void initTicker(void);

// Note: initLCD, initTicker and delay_10ms can be called directly without
//       wrappers, as they don't have parameters and return values.

#endif
