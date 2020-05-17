/*  Lab 2 - Wrappers

    Computerarchitektur / Computer Architecture
    (C) 2020 J. Friedrich, W. Zimmermann
    Hochschule Esslingen

    Author:  W.Zimmermann, Apr 15, 2020
*/

#include <hidef.h>                              // Common defines
#include <mc9s12dp256.h>                        // CPU specific defines

#pragma LINK_INFO DERIVATIVE "mc9s12dp256b"

#include "wrapper.h"

char string[80]="";
int  counter = 0;

// ****************************************************************************
void main(void) 
{   int i;

    EnableInterrupts;                           // Global interrupt enable

    initLCD();                    		// Initialize the LCD
    WriteLine_Wrapper("Wrapper Demo",   0);
    WriteLine_Wrapper("(C) HE Prof. Z", 1);    

    for(;;)                                     // Endless loop 
    {   decToASCII_Wrapper(string, counter);    // Convert counter value to ASCII string ...
    	counter++;
    	
    	WriteLine_Wrapper(string, 1);		// ... and display on LCD
    	for (i=0;i<10;i++)
    	{   delay_10ms();
    	}
    }
}
