/*  Radio signal clock - Free running clock

    Computerarchitektur / Computer Architecture
    (C) 2020 J. Friedrich, W. Zimmermann
    Hochschule Esslingen

    Author:   W.Zimmermann, May 06, 2020
*/

#include <stdio.h>
#include <stdlib.h>

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

enum DayOfWeek
{
    DAYOFWEEK_INVALID = 0,
    DAYOFWEEK_MONDAY = 1,
    DAYOFWEEK_TUESDAY,
    DAYOFWEEK_WEDNESDAY,
    DAYOFWEEK_THURSDAY,
    DAYOFWEEK_FRIDAY,
    DAYOFWEEK_SATURDAY,
    DAYOFWEEK_SUNDAY,

    DAYOFWEEK_MAX
};

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

// Module internal global variables
static struct DateTime
{
    unsigned char hour;
    unsigned char minute;
    unsigned char second;
    unsigned char dayOfMonth;
    unsigned char month;
    unsigned int year;
    enum DayOfWeek dayOfWeek;
    enum Timezone timezone;
} dateTime;

static int uptime = 0;
static int ticks = 0;

static char previousButtonState = 0;

// ****************************************************************************
//  Initialize clock module
//  Called once before using the module
void initClock(void)
{
    setClock(2020, 1, 1, 0, 0, 0);
    dateTime.timezone = TIMEZONE_DE;

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
        dateTime.timezone = (dateTime.timezone + 1) % TIMEZONE_MAX;
    }
    previousButtonState = buttonState;
}

// ****************************************************************************
// Check if given year is leap-year.
// Parameters:  year
// Returns:     number of days in given month
static unsigned char isLeapYear(unsigned int year)
{
    return year % 4 == 0 && (year % 100 != 0 || year % 400 == 0);
}

// ****************************************************************************
// Look-up number of days for given month. Does take leap-years into account.
// Parameters:  year and month
// Returns:     number of days in given month
static unsigned char daysInMonth(unsigned char month, unsigned int year)
{
    if (month == 1 || month == 3 || month == 5 || month == 7 ||
        month == 8 || month == 10 || month == 12)
    {
        return 31;
    }
    else if (month == 2)
    { /* Special treatment for February. */

        /* Determining a leapyear is based on three rules:
             *  1) divisible by 4 -> leapyear
             *  2) and divisible by 100 -> not leapyear
             *  3) and divisible by 400 -> leapyear
             */
        return isLeapYear(year)
                   ? 29 /* leapyear */
                   : 28 /* non-leapyear */;
    }
    else
    {
        return 30;
    }
}

// ****************************************************************************
// Calculate day-of-week given a date as year, month and day-of-month. Makes use
// of the algorithm proposed by Arpit Mishra (https://www.hackerearth.com/blog/developers/how-to-find-the-day-of-a-week/).
// Parameters:  Calendar date as year, month, day-of-month.
// Returns:     Value of day-of-week as enum with valid range from 1 (Monday) to
//              7 (Sunday).
static enum DayOfWeek calculateDayOfWeek(unsigned int y, unsigned char m, unsigned char d)
{
    /* This algorithm calculated the day-of-week for a given gregorian calendar
     * date. It does so using the fact that some months share the same
     * day-of-week at the beginning of the month. Notably, in a normal year (365
     * days), 01. Jan has the same dow as Apr and Jul. Feb and Aug also share
     * the same dow, and so on. We can define a look-up table for each month's
     * start dow relative to 01. Jan. Note that to compensate for leap-days
     * every other offset starting with March is decremented by 1. */
    static unsigned char monthOffsetLookup[12] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
    enum DayOfWeek result;

    y -= m < 3; /* If month is Jan or Feb, do not subtract leap day. Otherwise
                 * we would potentially add this years leap day onto Jan. */

    /* The part of the formula:
     *  $ y / 4 - y / 100 + y / 400 $
     * calculates the number leap days leading up to year $y$. Note that we
     * assume integer division, which automatically does a floor(...) of the
     * result.
     * We assume that the Gregorian calendar starts on a Monday 0001-01-01,
     * year 0000 does not exist.
     * The part:
     *  $ y + monthOffsetLookup[m - 1] $
     * calculates the weekday based on the year we're in and the value from the
     * look-up table. We then simply add the day-of-month and the number of
     * leap-days from earlier, divide the whole thing by 7 and return the
     * remainder. */
    result = (y / 4 - y / 100 + y / 400 +
              y + monthOffsetLookup[m - 1] + d) %
             (DAYOFWEEK_MAX - 1);

    /* The algorithm has one shortcoming in that Sunday maps to 0 and 7. Instead
     * complicating the math even further though, it's simpler to just add a
     * check at the end. Maybe not as elegant, but faster and easier to
     * understand. */
    return result <= DAYOFWEEK_INVALID ? DAYOFWEEK_SUNDAY : result;
}

// ****************************************************************************
// Increment years count of given date & time structure. Does take overflows
// into account. Day-of-month will be set to the minimum between day-of-month
// and maximum number of days in the month of the next year (only relevant when
// incrementing 29. Feb to a non-leap-year).
// Parameters:  Pointer to structure holding date & time information.
// Returns:     -
static void incrementYears(struct DateTime *dateTime)
{
    unsigned char daysInNewYearsMonth;
    dateTime->year++;
    daysInNewYearsMonth = daysInMonth(dateTime->month, dateTime->year);
    dateTime->dayOfMonth = dateTime->dayOfMonth > daysInNewYearsMonth ? daysInNewYearsMonth : dateTime->dayOfMonth;
    dateTime->dayOfWeek = calculateDayOfWeek(dateTime->year, dateTime->month, dateTime->dayOfMonth);
}

// ****************************************************************************
// Increment months count of given date & time structure. Does take overflows
// into account. Day-of-month will be set to the minimum between day-of-month
// and maximum number of days in the next month.
// Parameters:  Pointer to structure holding date & time information.
// Returns:     -
static void incrementMonths(struct DateTime *dateTime)
{
    unsigned char daysInNewMonth;
    if (dateTime->month >= 12)
    { // overflow
        incrementYears(dateTime);
        dateTime->month = 1;
    }
    else
    {
        dateTime->month++;
    }
    daysInNewMonth = daysInMonth(dateTime->month, dateTime->year);
    dateTime->dayOfMonth = dateTime->dayOfMonth > daysInNewMonth ? daysInNewMonth : dateTime->dayOfMonth;

    dateTime->dayOfWeek = calculateDayOfWeek(dateTime->year, dateTime->month, dateTime->dayOfMonth);
}

// ****************************************************************************
// Increment months count of given date & time structure. Does take overflows
// into account.
// Parameters:  Pointer to structure holding date & time information.
// Returns:     -
static void incrementDayOfMonths(struct DateTime *dateTime)
{
    if (dateTime->dayOfMonth >= daysInMonth(dateTime->dayOfMonth, dateTime->year))
    { // overflow
        incrementMonths(dateTime);
        dateTime->dayOfMonth = 0;
    }
    else
    {
        dateTime->dayOfMonth++;
    }

    dateTime->dayOfWeek = calculateDayOfWeek(dateTime->year, dateTime->month, dateTime->dayOfMonth);
}

// ****************************************************************************
// Increment minutes count of given date & time structure. Does take overflows
// into account.
// Parameters:  Pointer to structure holding date & time information.
// Returns:     -
static void incrementHours(struct DateTime *dateTime)
{
    if (dateTime->hour >= 23)
    { // overflow
        incrementDayOfMonths(dateTime);
        dateTime->hour = 0;
    }
    else
    {
        dateTime->hour++;
    }
}

// ****************************************************************************
// Increment minutes count of given date & time structure. Does take overflows
// into account.
// Parameters:  Pointer to structure holding date & time information.
// Returns:     -
static void incrementMinutes(struct DateTime *dateTime)
{
    if (dateTime->minute >= 59)
    { // overflow
        incrementHours(dateTime);
        dateTime->minute = 0;
    }
    else
    {
        dateTime->minute++;
    }
}

// ****************************************************************************
// Increment seconds count of given date & time structure. Does take overflows
// into account.
// Parameters:  Pointer to structure holding date & time information.
// Returns:     -
static void incrementSeconds(struct DateTime *dateTime)
{
    if (dateTime->second >= 59)
    { // overflow
        incrementMinutes(dateTime);
        dateTime->second = 0;
    }
    else
    {
        dateTime->second++;
    }
}

// ****************************************************************************
// Decrement years count of given date & time structure. Does take underflows
// into account. Day-of-month will be set to the minimum between day-of-month
// and maximum number of days in the month of the past year (only relevant when
// decrementing 29. Feb to a non-leap-year).
// Parameters:  Pointer to structure holding date & time information.
// Returns:     -
static void decrementYears(struct DateTime *dateTime)
{
    unsigned char daysInPreviousYearsMonth;
    dateTime->year--;

    /* Ensure that dateTime->dayOfMonth is a valid value, i.e. within the months
     * maximum number of days. */
    daysInPreviousYearsMonth = daysInMonth(dateTime->month, dateTime->year);
    dateTime->dayOfMonth = dateTime->dayOfMonth > daysInPreviousYearsMonth ? daysInPreviousYearsMonth : dateTime->dayOfMonth;

    dateTime->dayOfWeek = calculateDayOfWeek(dateTime->year, dateTime->month, dateTime->dayOfMonth);
}

// ****************************************************************************
// Decrement months count of given date & time structure. Does take underflows
// into account. Day-of-month will be set to the minimum between day-of-month
// and maximum number of days in the preceeding month.
// Parameters:  Pointer to structure holding date & time information.
// Returns:     -
static void decrementMonths(struct DateTime *dateTime)
{
    unsigned char daysInPreviousMonth;
    if (dateTime->month <= 1)
    { // underflow
        decrementYears(dateTime);
        dateTime->month = 12;
    }
    else
    {
        dateTime->month--;
    }

    /* Ensure that dateTime->dayOfMonth is a valid value, i.e. within the months
     * maximum number of days. */
    daysInPreviousMonth = daysInMonth(dateTime->month, dateTime->year);
    dateTime->dayOfMonth = dateTime->dayOfMonth > daysInPreviousMonth ? daysInPreviousMonth : dateTime->dayOfMonth;

    dateTime->dayOfWeek = calculateDayOfWeek(dateTime->year, dateTime->month, dateTime->dayOfMonth);
}

// ****************************************************************************
// Decrement day-of-month count of given date & time structure. Does take
// underflows into account.
// Parameters:  Pointer to structure holding date & time information.
// Returns:     -
static void decrementDayOfMonths(struct DateTime *dateTime)
{
    if (dateTime->dayOfMonth <= 1)
    { // underflow
        decrementMonths(dateTime);
        dateTime->dayOfMonth = daysInMonth(dateTime->dayOfMonth, dateTime->year);
    }
    else
    {
        dateTime->dayOfMonth--;
    }

    dateTime->dayOfWeek = calculateDayOfWeek(dateTime->year, dateTime->month, dateTime->dayOfMonth);
}

// ****************************************************************************
// Decrement hours count of given date & time structure. Does take underflows
// into account.
// Parameters:  Pointer to structure holding date & time information.
// Returns:     -
static void decrementHours(struct DateTime *dateTime)
{
    if (dateTime->hour <= 0)
    { // underflow
        decrementDayOfMonths(dateTime);
        dateTime->hour = 23;
    }
    else
    {
        dateTime->hour--;
    }
}

// ****************************************************************************
// Decrement minutes count of given date & time structure. Does take underflows
// into account.
// Parameters:  Pointer to structure holding date & time information.
// Returns:     -
static void decrementMinutes(struct DateTime *dateTime)
{
    if (dateTime->minute <= 0)
    { // underflow
        decrementHours(dateTime);
        dateTime->minute = 59;
    }
    else
    {
        dateTime->minute--;
    }
}

// ****************************************************************************
// Decrement seconds count of given date & time structure. Does take underflows
// into account.
// Parameters:  Pointer to structure holding date & time information.
// Returns:     -
static void decrementSeconds(struct DateTime *dateTime)
{
    if (dateTime->second <= 0)
    { // underflow
        decrementMinutes(dateTime);
        dateTime->second = 59;
    }
    else
    {
        dateTime->second--;
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

    incrementSeconds(&dateTime);
    displayDateTime();
}

// ****************************************************************************
// Allow other modules, e.g. DCF77, so set the time
// Parameters:  year, month, day-of-month, hour, minute, second as integers
// Returns:     -
void setClock(unsigned int year, unsigned char month, unsigned char dayOfMonth, unsigned char hour, unsigned char minute, unsigned char second)
{
    dateTime.year = year;
    dateTime.month = month;
    dateTime.dayOfMonth = dayOfMonth;

    dateTime.hour = hour;
    dateTime.minute = minute;
    dateTime.second = second;

    dateTime.dayOfWeek = calculateDayOfWeek(dateTime.year, dateTime.month, dateTime.dayOfMonth);

    ticks = 0;
}

// ****************************************************************************
// Display the date and time derived from the clock module on the LCD display,
// line 0 and 1
// Parameter:   -
// Returns:     -
void displayDateTime(void)
{
    unsigned char i;
    struct DateTime dateTimeClone = dateTime;
    char buf[32] = "00:00:00";

    for (i = 0; i < abs(timezoneLookupTable[dateTimeClone.timezone].Offset); ++i)
    {
        if (timezoneLookupTable[dateTimeClone.timezone].Offset > 0)
        {
            incrementHours(&dateTimeClone);
        }
        else
        {
            decrementHours(&dateTimeClone);
        }
    }

    (void)sprintf(buf, "%02u:%02u:%02u %s", dateTimeClone.hour, dateTimeClone.minute, dateTimeClone.second, timezoneLookupTable[dateTimeClone.timezone].Name);
    writeLine(buf, 0);

    (void)sprintf(buf, "%.3s %02d.%02d.%04d", dayOfWeekLookupTable[dateTimeClone.dayOfWeek], dateTimeClone.dayOfMonth, dateTimeClone.month, dateTimeClone.year);
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
