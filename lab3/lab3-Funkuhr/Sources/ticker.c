/*  Ticker module

    Computerarchitektur / Computer Architecture
    (C) 2020 J. Friedrich, W. Zimmermann 
    Hochschule Esslingen

    Author:   W.Zimmermann, May 06, 2020

    Ticker Interrupt, calling an Interrupt Service Routine every 10ms
    Uses Enhanced Capture Timer ECT channel 4

    The timer is initialized in such a way, that the ISR isrECT4 will be called every 10ms.
    In the ISR a user-provided callback function
                                void tick10ms(void)
    will be called. This function must end before the next timer interrupt event, i.e.
    the callbacks run time must be less than 10ms!
 
*/

#include "ticker.h"
#include <mc9s12dp256.h>


// Defines
#define TENMS       1875   	// 10 ms
#define TIMER_ON    0x80       	// tscr1 value to turn ECT on
#define TIMER_CH4   0x10        // Bit position for channel 4
#define TCTL1_CH4   0x03        // Mask corresponds to TCTL1 OM4, OL4


// External function
void tick10ms(void);


// Public interface function: initLCD ... Initialize Ticker channel 4 (called once)
void initTicker(void)
{   TSCR1 = TIMER_ON; 		// Timer master ON switch
    TIOS  = TIOS | TIMER_CH4;	// Set channel 4 in "output compare" mode
    TIE   = TIE  | TIMER_CH4;   // Enable channel 4 interrupt; bit 4 corresponds to channel 4

    // Set timer prescaler (bus clock : prescale factor)
    // In our case: divide by 2^7 = 128. This gives a timer
    // driver frequency of 187500 Hz or 5.3333 us time interval
#ifdef SIMULATOR
    TSCR2 = (TSCR2 & 0xF8) | 0x05;// Speedup clock for debugging in simulator
#else
    TSCR2 = (TSCR2 & 0xF8) | 0x07;
#endif

    TC4 = TCNT + TENMS;		// First timer event
    
    TCTL1 = TCTL1 & ~TCTL1_CH4; // Turn timer channel 4 on
}


// Internal function: isrECT4 ... Interrupt service routine, called by the timer ticker every 10ms
void interrupt 12 isrECT4(void)
{   TC4 = TC4 + TENMS;      	// Schedule the next ISR period
	
    TFLG1 = TFLG1 | TIMER_CH4;	// Clear the interrupt flag, write a 1 to bit 4
	
    tick10ms();           	// External function called every 10ms
}

