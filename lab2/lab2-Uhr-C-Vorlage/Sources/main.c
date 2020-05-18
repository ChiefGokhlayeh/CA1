/*  Lab 2 - Main C file for Clock program

    Computerarchitektur / Computer Architecture
    (C) 2020 J. Friedrich, W. Zimmermann
    Hochschule Esslingen

    Author:  W.Zimmermann, Apr 15, 2020
*/

#include <hidef.h>                              // Common defines
#include <mc9s12dp256.h>                        // CPU specific defines

#pragma LINK_INFO DERIVATIVE "mc9s12dp256b"

#include "wrapper.h"

void initTicker(void);


// PLEASE NOTE !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!:
// Files lcd.asm, ticker.asm and decToASCII are dummy files without functionality. 
// Please overwrite them with the lcd.asm and decToASCII files, which you bug fixed 
// in lab 1, and with file ticker.asm which you bug fixed in prep task 2.1 of this lab 2.
//
// To use decToASCII you must insert file decToASCII from the first lab into
// this project and complete the associated wrapper functions in wrapper.h.
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


// ****************************************************************************

void initLED_C(void)
{   DDRJ_DDRJ1  = 1;	  	// Port J.1 as output
    PTIJ_PTIJ1  = 0;		
    DDRB        = 0xFF;		// Port B as output
    PORTB       = 0x55;
}


// ****************************************************************************
// Global variables
unsigned char clockEvent = 0;


// ****************************************************************************
void main(void) 
{   EnableInterrupts;                           // Global interrupt enable

    initLED_C();                    		// Initialize the LEDs

    initLCD();                    		// Initialize the LCD
    WriteLine_Wrapper("Clock Template", 0);
    WriteLine_Wrapper("(C) HE Prof. Z", 1);    

    initTicker();                               // Initialize the time ticker

    for(;;)                                     // Endless loop
    {   if (clockEvent)
    	{   clockEvent = 0;
    
// ??? Add your code here ???
    
    	}
    }
}
