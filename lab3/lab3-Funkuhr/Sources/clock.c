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

enum Timezone
{
    TIMEZONE_DCF77,
    TIMEZONE_US,

    TIMEZONE_MAX
};

static struct TimezoneInfo
{
    const char *Name;
    signed char Offset;
} timezoneLookupTable[TIMEZONE_MAX] = {{"DE", 0}, {"US", -6}};

// Global variable holding the last clock event
CLOCKEVENT clockEvent = NOCLOCKEVENT;

// Modul internal global variables
static unsigned char hrs = 0, mins = 0, secs = 0;
static enum Timezone timezone = TIMEZONE_DCF77;
static int uptime = 0;
static int ticks = 0;
static char previousButtonState = 0;

// ****************************************************************************
//  Initialize clock module
//  Called once before using the module
void initClock(void)
{
    displayTimeClock();

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
// Process the clock events
// This function is called every second and will update the internal time values.
// Parameter:   clock event, normally SECONDTICK
// Returns:     -
void processEventsClock(CLOCKEVENT event)
{
    if (event == NOCLOCKEVENT)
        return;

    if (++secs >= 60)
    {
        secs = 0;
        if (++mins >= 60)
        {
            mins = 0;
            if (++hrs >= 24)
            {
                hrs = 0;
            }
        }
    }
    displayTimeClock();
}

// ****************************************************************************
// Allow other modules, e.g. DCF77, so set the time
// Parameters:  hours, minutes, seconds as integers
// Returns:     -
void setClock(char hours, char minutes, char seconds)
{
    hrs = hours;
    mins = minutes;
    secs = seconds;
    ticks = 0;
}

// ****************************************************************************
// Display the time derived from the clock module on the LCD display, line 0
// Parameter:   -
// Returns:     -
void displayTimeClock(void)
{
    char uhrzeit[32] = "00:00:00";
    /* This does not account for day-border over-/underflow, BUT Req 2.2
     * only states "[...]  the TIME display shall toggle [...]".
     *
     * In fact, implementing a day over-/underflow would require heavy
     * architectural changes to be considered "done properly". The main problem
     * is the current ownership principle of date/time, as dictated by the
     * project template. clock.c holds ownership over time, while dcf77.c
     * maintains the date. Thus, date and time are separated accross two
     * modules. This makes NO sense, as date and time are intermingled, which
     * becomes apparent when having to deal with timezones. A much better
     * approach would be to make clock.c have total ownership over the principle
     * of system date and time. Then let dcf77 only intersperse updated
     * date/time information whenever it is available. The job of DISPLAYING
     * time is that of the OWNER and ONLY the owner! The fundamental principle
     * of ONLY ONE CONCURRENT OWNER PER RESOURCE is violated by clock.c and
     * dcf77.c, both accessing the LCD and parts of the date/time information.
     * Apart from that, handing month and year over- and underflows is the next
     * rats' tail. Implementing a full calendar model did not seem to be part of
     * the requirement, but would technically be needed to have correct output.
     *
     * Rationale for not changing the entire architecture should be apparent:
     * Time effort in implementing and documenting the changes made to the
     * system architecture.
     * As implemented right now, Req 2.2 is techincally fulfilled.
     */
    (void)sprintf(uhrzeit, "%02u:%02u:%02u %s", (hrs + 24 + timezoneLookupTable[timezone].Offset) % 24, mins, secs, timezoneLookupTable[timezone].Name);
    writeLine(uhrzeit, 0);
}

// ***************************************************************************
// This function is called to get the CPU time base
// Parameters:  -
// Returns:     CPU time base in milliseconds
int time(void)
{
    return uptime;
}
