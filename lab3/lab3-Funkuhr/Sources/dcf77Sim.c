/*  Simulation of the DCF77 impulse signals
    for testing the radio signal clock withoud DCF77 radio signal receiver.

    Computerarchitektur / Computer Architecture
    (C) 2020 J. Friedrich, W. Zimmermann 
    Hochschule Esslingen

    Author:   W.Zimmermann, May 06, 2020
    Modified: W.Zimmermann, Jun 07, 2029 Changed comments for dcf77Data0...3[].

    Function readPort() must be called periodically once every 10ms. The function returns
    the value of the (simulated) DCF77 impulse signal. The simulation provides a time range
    of 8 minutes, then the signals repeat.
*/

#include <mc9s12dp256.h>                        // CPU specific defines
#include <stdlib.h>

// DCF77 simulation data sets
long dcf77Data0[16] =                 
{		0xD0F20000, 0x0480AC6A,              // 06.05.2020, 16:08 ... <<< botton on PTH.7
    0xD1120000, 0x0480AC6A,
    0xC1320000, 0x0480AC6A,
    0xD2120000, 0x0480AC6A,
    0xC2320000, 0x0480AC6A,
    0xC2520000, 0x0480AC6A,
    0xD2720000, 0x0480AC6A,
    0xC2920000, 0x0480AC6A
};

long dcf77Data1[16] =
{		0x45120000, 0x0080B482,              // 08.05.2020, 12:29 ... <<< botton on PTH.6
    0x55320000, 0x0080B482,
    0x46120000, 0x0080B482,
    0x56320000, 0x0080B482,
    0x56520000, 0x0080B482,
    0x46720000, 0x0080B482,
    0x56920000, 0x0080B482,
    0x46B20000, 0x0080B482,    
};

long dcf77Data2[16] =
{		0x7B120000, 0x0480BD0C,              // 10.05.2020, 23:59 ... <<< botton on PTH.5
    0x6B320000, 0x0480BD0C,
    0x00120000, 0x0080A510,
    0x10320000, 0x0080A510,
    0x10520000, 0x0080A510,
    0x00720000, 0x0080A510,
    0x10920000, 0x0080A510,
    0x00B20000, 0x0080A510,
};

long dcf77Data3[16] =
{		0x7B120000, 0x0080A51C,              // 11.05.2020, 23:59 ... <<< no button pressed
    0x6B320000, 0x0080A51C,
    0x00120000, 0x0080A920,
    0x10320000, 0x0080A920,
    0x10520000, 0x0080A920,
    0x00720000, 0x0080A920,
    0x10920000, 0x0080A920,
    0x00B20000, 0x0080A920,
};
/*  Note: Depending on your implementation of the DCF77 synchronization and decoding, the first
    minute displayed may be one minute later than the values shown in the comments above.
*/

int dcf77DataMin = 8;                   // ... for 8 minutes

char readPortSim(void)
{   static int i10ms = 9;               // Time counter, counts  10ms increments of a 100ms period
    static int i100ms =9;               //               counts 100ms increments of a 1s    period
    static int iSec  = 45;              //               counts 1s    increments of a 1min  period
    static int iMin  = 0;
    char signal = 0x01;                 // Default output signal is a High

    i10ms  = (i10ms +1) % 10;           // Update the time counters
    if (i10ms == 0)
    {   i100ms = (i100ms+1) % 10;
        if (i100ms == 0)
        {   iSec = (iSec + 1) % 60;
            if (iSec == 0)
                iMin = (iMin + 1) % 60;
        }

    }

    if (iSec < 59)                      // If it is not the last second of a minute
    {   if (i100ms < 1)                 // ... and if we are at the first 100ms of a second
        {   signal = 0;                 // ...... output Low
        } else if (i100ms < 2)          // ... if we are at the second 200ms of a second
        {   char n = (char) (iMin % dcf77DataMin);
            char i = (char) (iSec / 32);
            char j = (char) (iSec % 32);
            long temp;
            if (PTH & 0x80) 
            {    temp = dcf77Data0[n*2+i];  // <<< botton on PTH.7
            } else if (PTH & 0x40) 
            {    temp = dcf77Data1[n*2+i];  // <<< botton on PTH.6
            } else if (PTH & 0x20) 
            {    temp = dcf77Data2[n*2+i];  // <<< botton on PTH.5
            } else
            {    temp = dcf77Data3[n*2+i];  // <<< no button pressed
            }
            temp = (temp >> j) & 0x01;
            if (temp)                   // ...... and if the data bit is 1 output another Low
                signal = 0;
        }
    }
    
    if (PTH & 0x01)		 	// Simulate DCF77 signal black out by pressing button on PTH.0
    {   return 0x01;
    }
    if (PTH & 0x02)		        // or by pressing button on PTH.1
    {   return rand() > 0x4000 ? 0 : 1;
    }
    
    return signal;
}

void initializePortSim(void)
{   DDRH = DDRH & 0b11111100;
}