#ifndef CLOCK_H_
#define CLOCK_H_

void clock_init(void);

void clock_tick(void);

void clock_enable(void);

void clock_disable(void);

unsigned char clock_enabled(void);

unsigned int clock_get_current_tick(void);

void clock_set_hours(unsigned char h);

void clock_set_minutes(unsigned char m);

void clock_set_seconds(unsigned char s);

unsigned char clock_get_hours(void);

unsigned char clock_get_minutes(void);

unsigned char clock_get_seconds(void);

#endif
