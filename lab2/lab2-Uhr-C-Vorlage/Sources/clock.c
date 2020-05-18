#include "clock.h"

#include "wrapper.h"

#define SECONDS_PER_MINUTE (60)
#define MINUTES_PER_HOUR (60)
#define HOURS_PER_DAY (24)

static unsigned int tick_count = 0;
static unsigned char enabled = 0;

static unsigned char hours = 0;
static unsigned char minutes = 0;
static unsigned char seconds = 0;

void clock_init(void)
{
    hours = 11;
    minutes = 59;
    seconds = 45;

    enabled = 1;
}

void clock_enable(void)
{
    enabled = 1;
}

void clock_disable(void)
{
    enabled = 0;
}

unsigned char clock_enabled(void)
{
    return enabled;
}

unsigned int clock_get_current_tick(void)
{
    return tick_count;
}

void clock_set_hours(unsigned char h)
{
    unsigned int tmp = h;
    hours = (unsigned char) (tmp % HOURS_PER_DAY);
}

void clock_set_minutes(unsigned char m)
{
    unsigned int tmp = m;
    minutes = (unsigned char) (tmp % MINUTES_PER_HOUR);
}

void clock_set_seconds(unsigned char s)
{
    unsigned int tmp = s;
    seconds = (unsigned char) (tmp % SECONDS_PER_MINUTE);
}

unsigned char clock_get_hours(void)
{
    return hours;
}

unsigned char clock_get_minutes(void)
{
    return minutes;
}

unsigned char clock_get_seconds(void)
{
    return seconds;
}

void clock_tick(void)
{
    tick_count++;

    if (enabled)
    {
        seconds = (seconds + 1) % SECONDS_PER_MINUTE;
        if (!seconds)
        {
            minutes = (minutes + 1) % MINUTES_PER_HOUR;
            if (!minutes)
            {
                hours = (hours + 1) % HOURS_PER_DAY;
            }
        }
    }
}
