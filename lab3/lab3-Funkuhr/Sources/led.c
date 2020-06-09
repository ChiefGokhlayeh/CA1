/*  LED module

    Computerarchitektur / Computer Architecture
    (C) 2020 J. Friedrich, W. Zimmermann 
    Hochschule Esslingen

    Author:   W.Zimmermann, May 06, 2020
    Modified: 
*/

#include <mc9s12dp256.h>                        // CPU specific defines
#include "led.h"

// Initialize LEDs (called once)
void initLED(void)
{   DDRJ = DDRJ | 0x02;		// Port J.1 as output
    PTJ  = PTJ & ~0x02;		// ... set to 0, i.e. activate LEDs
    
    DDRB = 0xFF;		// Port B as outputs
    PORTB= 0;			// ... and turn all LEDs off
    
    DDRP = 0xFF;	// Turn seven segment display off
    PTP  = 0xFF;
}

// Toggle LEDs specified by binary mask, e.g. 0x81 toggles LED7 and LED0
#pragma INLINE
void toggleLED(unsigned char mask)
{   PORTB = PORTB ^ mask;
}

// Set LEDs specified by binary mask, e.g. 0x81 set LED7 and LED0
#pragma INLINE
void setLED(unsigned char mask)
{   PORTB = PORTB | mask;
}

// Clear LEDs specified by binary mask, e.g. 0x81 clears LED7 and LED0
#pragma INLINE
void clrLED(unsigned char mask)
{   PORTB = PORTB & (~mask);
}


