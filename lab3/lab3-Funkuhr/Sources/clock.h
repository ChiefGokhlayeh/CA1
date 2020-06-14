/*  Header for Clock module

    Computerarchitektur / Computer Architecture
    (C) 2020 J. Friedrich, W. Zimmermann
    Hochschule Esslingen

    Author:   W.Zimmermann, May 06, 2020
*/

// Data type for clock events
typedef enum
{
    NOCLOCKEVENT = 0,
    SECONDTICK
} CLOCKEVENT;

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

enum Timezone
{
    TIMEZONE_DE,
    TIMEZONE_US,

    TIMEZONE_MAX
};

// Global variable holding the last clock event
extern CLOCKEVENT clockEvent;

// Public functions, for details see clock.c
void initClock(void);
void processEventsClock(CLOCKEVENT event);
void setClock(unsigned int y, unsigned char m, unsigned char dom, enum DayOfWeek dow, unsigned char hours, unsigned char minutes, unsigned char seconds);
void displayDateTime(void);
