/*  Lab 3 - Radio signal clock - C Main Program

    Computerarchitektur / Computer Architecture
    (C) 2020 J. Friedrich, W. Zimmermann 
    Hochschule Esslingen

    Author:   W.Zimmermann, May 06, 2020
    Modified: 

*/

#include <hidef.h>                              // Common defines
#include <mc9s12dp256.h>                        // CPU specific defines

#pragma LINK_INFO DERIVATIVE "mc9s12dp256b"

#include "led.h"
#include "lcd.h"
#include "clock.h"
#include "dcf77.h"
#include "ticker.h"
#include "os.h"


osTCB osTaskList[OSNUMTASKS] = 			// List of all tasks and associated trigger events
{
    processEventsClock, (int*) &clockEvent,     // --- Clock task
	
    processEventsDCF77, (int*) &dcf77Event,     // --- DCF77 task

    NULL, NULL 	      				// --- Must always be the last entry
};


// ****************************************************************************
void main(void)
{   EnableInterrupts;                           // Allow interrupts

//  Initialize all modules
    initLED();                                  // Initialize LEDs on port B
    initLCD();                                  // Initialize LCD display
    initClock();                                // Initialize Clock module
    initDCF77();                                // Initialize DCF77 module
    initTicker();                               // Initialize the time ticker
    
    writeLine("* Radio Clock *", 0);
    writeLine("(C) HE Prof Z.",  1);

//  Start the operating system which will case the tasks in osTaskList, if the associated events are triggered
    initOS(osTaskList);				// Note: This function never returns!
}



