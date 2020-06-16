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

// ****************************************************************************
// Maximum duration of a LONG symbol (representing binary 1) of the DCF77
// protocol.
#define MAX_LONG_SYMBOL_TIME (230)
// ****************************************************************************
// Minimum duration of a LONG symbol (representing binary 1) of the DCF77
// protocol.
#define MIN_LONG_SYMBOL_TIME (170)
// ****************************************************************************
// Maximum duration of a SHORT symbol (representing binary 0) of the DCF77
// protocol.
#define MAX_SHORT_SYMBOL_TIME (130)
// ****************************************************************************
// Minimum duration of a SHORT symbol (representing binary 0) of the DCF77
// protocol.
#define MIN_SHORT_SYMBOL_TIME (70)
// ****************************************************************************
// Maximum duration of a START-OF-MINUTE symbol (representing the end-of-frame)
// of the DCF77 protocol.
#define MAX_START_OF_MINUTE_TIME (2100)
// ****************************************************************************
// Minimum duration of a START-OF-MINUTE symbol (representing the end-of-frame)
// of the DCF77 protocol.
#define MIN_START_OF_MINUTE_TIME (1900)
// ****************************************************************************
// Maximum time between two falling edges (representing one second elapsed) of
// the DCF77 protocol.
#define MAX_SECOND_TIME (1100)
// ****************************************************************************
// Minimum time between two falling edges (representing one second elapsed) of
// the DCF77 protocol.
#define MIN_SECOND_TIME (900)
// ****************************************************************************
// Bits held in a full DCF77 frame.
#define BITS_PER_FRAME (59)

// ****************************************************************************
// Macro to ease access to individual bits inside a byte buffer.
// Parameters:  pointer to  buffer, offset of bit from inside buffer
// Returns:     value of bit at given offset
#define BIT_AT(BUF, OFFSET) (!!(((unsigned char *)(BUF))[(OFFSET) / 8] & (1 << ((OFFSET) % 8))))

// Global variable holding the last DCF77 event
DCF77EVENT dcf77Event = NODCF77EVENT;

// Modul internal global variables
static unsigned char dcf77Month = 1, dcf77Day = 1, dcf77Hour = 0, dcf77Minute = 0; //dcf77 Date and time as integer values
static int dcf77Year = 2025;
/*static enum DayOfWeek dcf77DayOfWeek = DAYOFWEEK_INVALID; */
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
    setClock(dcf77Year, dcf77Month, dcf77Day, /* dcf77DayOfWeek, */ dcf77Hour, dcf77Minute, 0);

#ifdef SIMULATOR
    initializePortSim();
#else
    initializePort();
#endif
}

// ****************************************************************************
//  Indicate successful synchronization of DCF77 signal.
void displaySuccessfulSync(void)
{
    setLED(0x08);
}

// ****************************************************************************
//  Indicate loss-of-signal/synchronization with DCF77 signal.
void displayLossOfSync(void)
{
    clrLED(0x08);
}

// ****************************************************************************
// Action to take when loss-of-signal occurs. Will reset the current DCF77 frame
// and indicate the event on the LEDs.
void handleLossOfSync(void)
{
    frameLength = 0;
    frameStarted = 0;
    displayLossOfSync();
}

// ****************************************************************************
//  Indicate successful decoding of DCF77 frame.
void displaySuccessfulDecoding(void)
{
    clrLED(0x04);
}

// ****************************************************************************
//  Indicate unsuccessful decoding of DCF77 frame.
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
            { // symbol too long
                event = INVALID;
            }
            else if (timeDelta >= MIN_LONG_SYMBOL_TIME)
            { // symbol within LONG boundaries
                event = VALIDONE;
            }
            else if (timeDelta <= MAX_SHORT_SYMBOL_TIME && timeDelta >= MIN_SHORT_SYMBOL_TIME)
            { // symbol within SHORT boundaries
                event = VALIDZERO;
            }
            else
            { /* symbol neither LONG nor SHORT but something in between
               * (undecidable) */
                event = INVALID;
            }
        }
        else
        { // falling edge

            setLED(0x02); // Req. 1.4 turn on LED B.1 when signal is low
            if (timeDelta <= MAX_START_OF_MINUTE_TIME)
            { // symbol not longer than longes possible symbol
                if (timeDelta >= MIN_START_OF_MINUTE_TIME)
                { // symbol within START-OF-MINUTE boundaries
                    event = VALIDMINUTE;
                }
                else if (timeDelta <= MAX_SECOND_TIME && timeDelta >= MIN_SECOND_TIME)
                { // symbol within SECOND boundaries
                    event = VALIDSECOND;
                }
                else
                { /* symbol neither START-OF-MINUTE nor SECOND but something in
                   * between (undecidable) */
                    event = INVALID;
                }
                lastTime = currentTime; /* remember time of last falling edge to
                                         * calculate time-delta of next rising
                                         * edge */
            }
        }
    }
    else
    { // no edge
        if (timeDelta < MAX_START_OF_MINUTE_TIME)
        { // symbol unchanged but time-delta still within acceptable range
            event = NODCF77EVENT;
        }
        else
        { // time-delta exceeded maximum no-edge threshold
            event = INVALID;
        }
    }

    previousSignal = currentSignal;

    return event;
}

// ****************************************************************************
// Calculate parity of bit-buffer slice.
// Parameter:  pointer to buffer, (inclusive) start and (exclusive) end offset
//             inside buffer
// Returns:    XOR sum of bits within given range
unsigned char calculateParity(const void *buf, unsigned char from, unsigned char to)
{
    unsigned char parity = BIT_AT(buf, from); // load first bit
    unsigned char i;
    for (i = from + 1; i < (to + 1); i++)
    { // calculate XOR sum of all bits in given slice
        parity ^= BIT_AT(buf, i);
    }
    return parity;
}

// ****************************************************************************
// Simple (incomplete) check if date values within valid range. Does not account
// for number of days in month or if year is a leap-year.
// Returns:    true of date is valid, false otherwise
int isDateValid(void)
{
    return dcf77Minute <= 59 &&
           dcf77Hour <= 23 &&
           dcf77Day <= 31 &&
           dcf77Month <= 12;
}

// ****************************************************************************
// Decode date and time information from given frame buffer.
// Parameter:  pointer to buffer of demodulated frame
// Returns:    EXIT_SUCCESS (0) if frame could be decoded successfully and all
//             checks passed, EXIT_FAILURE (1) otherwise.
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
        // Decoding day-of-week is not needed because we use our own day-of-week
        // algorithm. Still, here's the commented-out code.
        /* dcf77DayOfWeek = (BIT_AT(frame, 42) << 0) |
         *                  (BIT_AT(frame, 43) << 1) |
         *                  (BIT_AT(frame, 44) << 2); */
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

        if (isDateValid())
        {
            return EXIT_SUCCESS;
        }
        else
        {
            return EXIT_FAILURE;
        }
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
        /* do nothing, ignore */
        break;
    case VALIDMINUTE:
        frameStarted = 1;
        if (frameLength >= BITS_PER_FRAME)
        {
            if (EXIT_SUCCESS == decodeFrame(frame))
            {
                setClock(dcf77Year, dcf77Month, dcf77Day, /* dcf77DayOfWeek, */ dcf77Hour, dcf77Minute, 0);
                displaySuccessfulSync();
                displaySuccessfulDecoding();
            }
            else
            {
                displayDecodingError(); // indicate parity or sanity check failure (R1.5b)
                displayLossOfSync();    // indicate wrong data (R1.5a)
            }
        }
        else
        {
            displayLossOfSync(); // wrong amount of data (R1.5a)
        }
        frameLength = 0;
        break;
    }
}
