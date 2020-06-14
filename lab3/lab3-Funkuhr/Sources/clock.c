/*  Radio signal clock - Free running clock

    Computerarchitektur / Computer Architecture
    (C) 2020 J. Friedrich, W. Zimmermann
    Hochschule Esslingen

    Author:   W.Zimmermann, May 06, 2020
*/

#include <stdio.h>

#include "clock.h"
#include "lcd.h"
#include "led.h"
#include "dcf77.h"

#include <mc9s12dp256.h>

// Defines
#define ONESEC (1000 / 10) // 10ms ticks per second
#define MSEC200 (200 / 10)

static struct TimezoneInfo
{
    const char *Name;
    signed char Offset;
} timezoneLookupTable[TIMEZONE_MAX] = {{"DE", 0}, {"US", -6}};

static const char *const dayOfWeekLookupTable[DAYOFWEEK_MAX] = {"--------",
                                                                "Monday",
                                                                "Tuesday",
                                                                "Wednesday",
                                                                "Thursday",
                                                                "Friday",
                                                                "Saturday",
                                                                "Sunday"};
// Global variable holding the last clock event
CLOCKEVENT clockEvent = NOCLOCKEVENT;

// Modul internal global variables
static unsigned char hrs = 0, mins = 0, secs = 0, dayOfMonth = 0, months = 0;
static unsigned int years = 0;
static enum DayOfWeek dayOfWeek = DAYOFWEEK_INVALID;
static enum Timezone timezone = TIMEZONE_DE;
static int uptime = 0;
static int ticks = 0;
static char previousButtonState = 0;

// ****************************************************************************
//  Initialize clock module
//  Called once before using the module
void initClock(void)
{
    displayDateTime();

#ifdef SIMULATOR
    /* dcf77Sim should have already initialized DDRH as we need it. Really NOT
     * and optimal architecture -- relying on a TEST-CODE(!!!) to do
     * initialization for main logic, but such is this entire project template.
     */
#else
    // only initialize port H.3 and H.0 for timezone and DCF77 signal
    DDRH = DDRH & 0b00001001;
#endif
}

// ****************************************************************************
// This function is called periodically every 10ms by the ticker interrupt.
// Keep processing short in this function, run time must not exceed 10ms!
// Callback function, never called by user directly.
void tick10ms(void)
{
    char buttonState;
    if (++ticks >= ONESEC) // Check if one second has elapsed
    {
        clockEvent = SECONDTICK; // ... if yes, set clock event
        ticks = 0;
        setLED(0x01); // ... and turn on LED on port B.0 for 200msec
    }
    else if (ticks == MSEC200)
    {
        clrLED(0x01);
    }
    uptime = uptime + 10; // Update CPU time base

    dcf77Event = sampleSignalDCF77(uptime); // Sample the DCF77 signal

    buttonState = !!(PTH & 0x08);                           // get button press state
    if (buttonState != previousButtonState && !buttonState) // only toggle timezone on rising edge (button release)
    {
        timezone = (timezone + 1) % TIMEZONE_MAX;
    }
    previousButtonState = buttonState;
}

// ****************************************************************************
// Handle overflow at above 24 hours
// Will handle subsequent overflows in day-of-month, month and year.
// If no overflow is detected, this function will do nothing.
// Parameter:   hours       pointer to hour field (in/out-parameter)
//              dayOfMonth  pointer to day-of-month field (in/out-parameter)
//              dayOfWeek   pointer to day-of-week field (in/out-parameter)
//              month       pointer to month field (in/out-parameter)
//              year        pointer to year field (in/out-parameter)
// Returns:     -
void handleHourOverflow(unsigned char *hours, unsigned char *dayOfMonth, enum DayOfWeek *dayOfWeek, unsigned char *month, unsigned int *year)
{
    char daysOverflow = 0;
    if (*hours >= 24)
    {
        *hours %= 24;
        if (*month == 1 || *month == 3 || *month == 5 || *month == 7 ||
            *month == 8 || *month == 10 || *month == 12)
        {
            if (++(*dayOfMonth) > 31)
            {
                *dayOfMonth = 1;
                daysOverflow = 1;
            }
        }
        else if (*month == 2)
        { /* Special treatment for February. */

            /* Determining a leapyear is based on three rules:
             *  1) divisible by 4 -> leapyear
             *  2) and divisible by 100 -> not leapyear
             *  3) and divisible by 400 -> leapyear
             */
            if (++(*dayOfMonth) > (*year % 4 == 0 &&
                                           (*year % 100 != 0 || *year % 400 == 0)
                                       ? 29 /* leapyear */
                                       : 28 /* non-leapyear */))
            {
                *dayOfMonth = 1;
                daysOverflow = 1;
            }
        }
        else
        {
            if (++(*dayOfMonth) > 30)
            {
                *dayOfMonth = 1;
                daysOverflow = 1;
            }
        }

        if (*dayOfWeek != DAYOFWEEK_INVALID)
        {
            /* Increment day-of-week but keep in mind that SUNDAY wraps
             * around to MONDAY, not to INVALID! That's what the modulo
             * by max-1 is for. */
            *dayOfWeek = (*dayOfWeek + 1) % (DAYOFWEEK_MAX - 1);
        }

        if (daysOverflow)
        { /* Days overflowed into a new month, increment months and
           * check year border */
            if (++(*month) > 12)
            {
                *month = 1;
                (*year)++;
            }
        }
    }
}

// ****************************************************************************
// Handle underflows at below 0 hours. hours is a unsigned value, so we
// interpret everything above 23 as underflow.
// Will handle subsequent overflows in day-of-month, month and year.
// If no underflow is detected, this function will do nothing.
// Parameter:   hours       pointer to hour field (in/out-parameter)
//              dayOfMonth  pointer to day-of-month field (in/out-parameter)
//              dayOfWeek   pointer to day-of-week field (in/out-parameter)
//              month       pointer to month field (in/out-parameter)
//              year        pointer to year field (in/out-parameter)
// Returns:     -
void handleHourUnderflow(unsigned char *hours, unsigned char *dayOfMonth, enum DayOfWeek *dayOfWeek, unsigned char *month, unsigned int *year)
{
    char daysUnderflow = 0;
    if (*hours >= 24)
    {
        *hours = 24 - 256 + *hours; /* handling underflow of unsigned value */
        if (*month == 1 || *month == 3 || *month == 5 || *month == 7 ||
            *month == 8 || *month == 10 || *month == 12)
        {
            if (--(*dayOfMonth) < 1)
            {
                *dayOfMonth = 31;
                daysUnderflow = 1;
            }
        }
        else if (*month == 2)
        { /* Special treatment for February. */

            /* Determining a leapyear is based on three rules:
             *  1) divisible by 4 -> leapyear
             *  2) and divisible by 100 -> not leapyear
             *  3) and divisible by 400 -> leapyear
             */
            if (--(*dayOfMonth) < 1)
            {
                *dayOfMonth = (*year % 4 == 0 &&
                                       (*year % 100 != 0 || *year % 400 == 0)
                                   ? 29 /* leapyear */
                                   : 28 /* non-leapyear */);
                daysUnderflow = 1;
            }
        }
        else
        {
            if (--(*dayOfMonth) < 1)
            {
                *dayOfMonth = 30;
                daysUnderflow = 1;
            }
        }

        if (*dayOfWeek != DAYOFWEEK_INVALID)
        {
            /* Decrement day-of-week but keep in mind that MONDAY wraps
             * around to SUNDAY, not to INVALID! */
            *dayOfWeek = *dayOfWeek - 1;
            *dayOfWeek = *dayOfWeek <= DAYOFWEEK_INVALID ? DAYOFWEEK_SUNDAY : *dayOfWeek;
        }

        if (daysUnderflow)
        { /* Days overflowed into a new month, increment months and
           * check year border */
            if (--(*month) < 1)
            {
                *month = 12;
                (*year)--;
            }
        }
    }
}

// ****************************************************************************
// Process the clock events
// This function is called every second and will update the internal time values.
// Parameter:   clock event, normally SECONDTICK
// Returns:     -
void processEventsClock(CLOCKEVENT event)
{
    char daysOverflow = 0;
    if (event == NOCLOCKEVENT)
        return;

    if (++secs >= 60)
    {
        secs = 0;
        if (++mins >= 60)
        {
            mins = 0;
            ++hrs;
            handleHourOverflow(&hrs, &dayOfMonth, &dayOfWeek, &months, &years);
        }
    }
    displayDateTime();
}

// ****************************************************************************
// Allow other modules, e.g. DCF77, so set the time
// Parameters:  hours, minutes, seconds as integers
// Returns:     -
void setClock(unsigned int y, unsigned char m, unsigned char dom, enum DayOfWeek dow, unsigned char hours, unsigned char minutes, unsigned char seconds)
{
    years = y;
    months = m;
    dayOfMonth = dom;
    dayOfWeek = dow;

    hrs = hours;
    mins = minutes;
    secs = seconds;

    ticks = 0;
}

// ****************************************************************************
// Display the date and time derived from the clock module on the LCD display,
// line 0 and 1
// Parameter:   -
// Returns:     -
void displayDateTime(void)
{
    unsigned char hh = hrs, mm = mins, ss = secs, dd = dayOfMonth, MM = months;
    unsigned int YYYY = years;
    enum DayOfWeek dow = dayOfWeek;
    char buf[32] = "00:00:00";

    hh += timezoneLookupTable[timezone].Offset;
    if (timezoneLookupTable[timezone].Offset > 0)
    { // positive offset values potentially overflow
        handleHourOverflow(&hh, &dd, &dow, &MM, &YYYY);
    }
    else
    { // while negative ones potentially underflow
        handleHourUnderflow(&hh, &dd, &dow, &MM, &YYYY);
    }

    (void)sprintf(buf, "%02u:%02u:%02u %s", hh, mm, ss, timezoneLookupTable[timezone].Name);
    writeLine(buf, 0);

    (void)sprintf(buf, "%.3s %02d.%02d.%04d", dayOfWeekLookupTable[dow], dd, MM, YYYY);
    writeLine(buf, 1);
}

// ***************************************************************************
// This function is called to get the CPU time base
// Parameters:  -
// Returns:     CPU time base in milliseconds
int time(void)
{
    return uptime;
}
