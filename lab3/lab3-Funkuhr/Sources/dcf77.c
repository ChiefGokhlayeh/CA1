/*  Radio signal clock - DCF77 Module

    Computerarchitektur / Computer Architecture
    (C) 2020 J. Friedrich, W. Zimmermann
    Hochschule Esslingen

    Author:   W.Zimmermann, May 06, 2020
    Modified: A.Baulig, J.Janusch, June 11, 2020
*/

/*
; N O T E:  T H I S  M O D U L E  I S  N O T  F U L L Y  F U N C T I O N A L
;
; The file contains function prototypes for functions you have to implement.
; See the sections marked as "--- Add your code here --- --- ??? ---".
;
*/

#include <hidef.h>       // Common defines
#include <mc9s12dp256.h> // CPU specific defines
#include <stdio.h>
#include <stdlib.h>

#include "dcf77.h"
#include "led.h"
#include "clock.h"
#include "lcd.h"

#define MAX_LONG_SYMBOL_TIME (230)
#define MIN_LONG_SYMBOL_TIME (170)
#define MAX_SHORT_SYMBOL_TIME (130)
#define MIN_SHORT_SYMBOL_TIME (70)
#define MAX_START_OF_MINUTE_TIME (2100)
#define MIN_START_OF_MINUTE_TIME (1900)
#define MAX_SECOND_TIME (1100)
#define MIN_SECOND_TIME (900)
#define BITS_PER_FRAME (59)

#define BIT_AT(BUF, OFFSET) (!!(((unsigned char *)(BUF))[(OFFSET) / 8] & (1 << ((OFFSET) % 8))))

// Global variable holding the last DCF77 event
DCF77EVENT dcf77Event = NODCF77EVENT;

// Modul internal global variables
static int dcf77Year = 2025, dcf77Month = 1, dcf77Day = 1, dcf77Hour = 0, dcf77Minute = 0; //dcf77 Date and time as integer values
static unsigned char frame[BITS_PER_FRAME / 8 + 1];
static unsigned char frameLength = 0;
static unsigned char frameStarted = 0;
static unsigned char previousSignal = 0;
static int lastTime = 0;

// Prototypes of functions simulation DCF77 signals, when testing without
// a DCF77 radio signal receiver
void initializePortSim(void); // Use instead of initializePort() for simulator testing
char readPortSim(void);       // Use instead of readPort() for simulator testing

// ****************************************************************************
// Initialize the hardware port on which the DCF77 signal is connected as input
// Parameter:   -
// Returns:     -
//
// NOT USED IN THE CORONA EDITION
//
//void initializePort(void)
//{
// --- Add your code here ----------------------------------------------------
// --- ??? --- ??? --- ??? --- ??? --- ??? --- ??? --- ??? --- ??? --- ??? ---
//}

// ****************************************************************************
// Read the hardware port on which the DCF77 signal is connected as input
// Parameter:   -
// Returns:     0 if signal is Low, >0 if signal is High
//
// NOT USED IN THE CORONA EDITION
//
//char readPort(void)
//{
// --- Add your code here ----------------------------------------------------
// --- ??? --- ??? --- ??? --- ??? --- ??? --- ??? --- ??? --- ??? --- ??? ---
//    return -1;
//}

// ****************************************************************************
//  Initialize DCF77 module
//  Called once before using the module
void initDCF77(void)
{
    setClock((char)dcf77Hour, (char)dcf77Minute, 0);
    displayDateDcf77();

#ifdef SIMULATOR
    initializePortSim();
#else
    initializePort();
#endif
}

// ****************************************************************************
// Display the date derived from the DCF77 signal on the LCD display, line 1
// Parameter:   -
// Returns:     -
void displayDateDcf77(void)
{
    char datum[32];

    (void)sprintf(datum, "%02d.%02d.%04d", dcf77Day, dcf77Month, dcf77Year);
    writeLine(datum, 1);
}

void displaySuccessfulSync(void)
{
    setLED(0x08);
}

void displayLossOfSync(void)
{
    clrLED(0x08);
}

void handleLossOfSync(void)
{
    frameLength = 0;
    frameStarted = 0;
    displayLossOfSync();
}

void displaySuccessfulDecoding(void)
{
    clrLED(0x04);
}

void displayDecodingError(void)
{
    setLED(0x04);
}

// ****************************************************************************
//  Read and evaluate DCF77 signal and detect events
//  Must be called by user every 10ms
//  Parameter:  Current CPU time base in milliseconds
//  Returns:    DCF77 event, i.e. second pulse, 0 or 1 data bit or minute marker
DCF77EVENT sampleSignalDCF77(int currentTime)
{
    DCF77EVENT event = NODCF77EVENT;
    char currentSignal;
    int timeDelta = currentTime - lastTime;

#ifdef SIMULATOR
    currentSignal = readPortSim(); // Sample simulated DCF77 signal
#else
    currentSignal = readPort(); // Sample DCF77 signal
#endif

    if (currentSignal != previousSignal)
    { // some kind of edge
        if (currentSignal)
        { // rising edge

            clrLED(0x02); // Req. 1.4 turn off LED B.1 when signal is high
            if (timeDelta > MAX_LONG_SYMBOL_TIME)
            {
                event = INVALID;
            }
            else if (timeDelta >= MIN_LONG_SYMBOL_TIME)
            {
                event = VALIDONE;
            }
            else if (timeDelta <= MAX_SHORT_SYMBOL_TIME && timeDelta >= MIN_SHORT_SYMBOL_TIME)
            {
                event = VALIDZERO;
            }
            else
            {
                event = INVALID;
            }
        }
        else
        { // falling edge

            setLED(0x02); // Req. 1.4 turn on LED B.1 when signal is low
            if (timeDelta <= MAX_START_OF_MINUTE_TIME)
            {
                if (timeDelta >= MIN_START_OF_MINUTE_TIME)
                {
                    event = VALIDMINUTE;
                }
                else if (timeDelta <= MAX_SECOND_TIME && timeDelta >= MIN_SECOND_TIME)
                {
                    event = VALIDSECOND;
                }
                else
                {
                    event = INVALID;
                }
                lastTime = currentTime;
            }
        }
    }
    else
    { // no edge
        if (timeDelta < MAX_START_OF_MINUTE_TIME)
        {
            event = NODCF77EVENT;
        }
        else
        { // time delta exceeded maximum no-edge threshold
            event = INVALID;
        }
    }

    previousSignal = currentSignal;

    return event;
}

unsigned char calculateParity(const void *buf, unsigned char from, unsigned char to)
{
    unsigned char parity = BIT_AT(buf, from);
    unsigned char i;
    for (i = from + 1; i < (to + 1); i++)
    {
        parity ^= BIT_AT(buf, i);
    }
    return parity;
}

int decodeFrame(const unsigned char *frame)
{
    unsigned char minutesParity = calculateParity(frame, 21, 27);
    unsigned char hoursParity = calculateParity(frame, 29, 34);
    unsigned char dateParity = calculateParity(frame, 36, 57);

    if (minutesParity == BIT_AT(frame, 28) &&
        hoursParity == BIT_AT(frame, 35) &&
        dateParity == BIT_AT(frame, 58) &&
        BIT_AT(frame, 20))
    {
        dcf77Minute = BIT_AT(frame, 21) +
                      BIT_AT(frame, 22) * 2 +
                      BIT_AT(frame, 23) * 4 +
                      BIT_AT(frame, 24) * 8 +
                      BIT_AT(frame, 25) * 10 +
                      BIT_AT(frame, 26) * 20 +
                      BIT_AT(frame, 27) * 40;
        dcf77Hour = BIT_AT(frame, 29) +
                    BIT_AT(frame, 30) * 2 +
                    BIT_AT(frame, 31) * 4 +
                    BIT_AT(frame, 32) * 8 +
                    BIT_AT(frame, 33) * 10 +
                    BIT_AT(frame, 34) * 20;
        dcf77Day = BIT_AT(frame, 36) +
                   BIT_AT(frame, 37) * 2 +
                   BIT_AT(frame, 38) * 4 +
                   BIT_AT(frame, 39) * 8 +
                   BIT_AT(frame, 40) * 10 +
                   BIT_AT(frame, 41) * 20;
        dcf77Month = BIT_AT(frame, 45) +
                     BIT_AT(frame, 46) * 2 +
                     BIT_AT(frame, 47) * 4 +
                     BIT_AT(frame, 48) * 8 +
                     BIT_AT(frame, 49) * 10;
        dcf77Year = BIT_AT(frame, 50) +
                    BIT_AT(frame, 51) * 2 +
                    BIT_AT(frame, 52) * 4 +
                    BIT_AT(frame, 53) * 8 +
                    BIT_AT(frame, 54) * 10 +
                    BIT_AT(frame, 55) * 20 +
                    BIT_AT(frame, 56) * 40 +
                    BIT_AT(frame, 57) * 80 + 2000;
        return EXIT_SUCCESS;
    }
    else
    {
        return EXIT_FAILURE;
    }
}

// ****************************************************************************
// Process the DCF77 events
// Contains the DCF77 state machine
// Parameter:   Result of sampleSignalDCF77 as parameter
// Returns:     -
void processEventsDCF77(DCF77EVENT event)
{
    switch (event)
    {
    default:
    case INVALID:
        handleLossOfSync();
        break;
    case VALIDZERO:
        if (frameStarted)
        {
            if (frameLength < BITS_PER_FRAME)
            {
                // unset m'th bit in n'th octet
                frame[frameLength / 8] &= ~(1 << (frameLength % 8));
                frameLength++;
            }
            else
            {
                handleLossOfSync();
            }
        }
        break;
    case VALIDONE:
        if (frameStarted)
        {
            if (frameLength < BITS_PER_FRAME)
            {
                // set m'th bit in n'th octet
                frame[frameLength / 8] |= 1 << (frameLength % 8);
                frameLength++;
            }
            else
            {
                handleLossOfSync();
            }
        }
        break;
    case VALIDSECOND:
    case NODCF77EVENT:
        /* do nothing */
        break;
    case VALIDMINUTE:
        frameStarted = 1;
        if (frameLength >= BITS_PER_FRAME)
        {
            if (EXIT_SUCCESS == decodeFrame(frame))
            {
                setClock((char)dcf77Hour, (char)dcf77Minute, 0);
                displaySuccessfulSync();
                displaySuccessfulDecoding();
            }
            else
            {
                displayDecodingError(); // indicate parity or sanity check failure (R1.5b)
                displayLossOfSync(); // indicate wrong data (R1.5a)
            }
        }
        else
        {
            displayLossOfSync(); // wrong amount of data (R1.5a)
        }
        frameLength = 0;
        break;
    }

    displayDateDcf77();
}
