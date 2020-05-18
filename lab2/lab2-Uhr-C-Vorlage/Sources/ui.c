#include "ui.h"

#include "clock.h"
#include "wrapper.h"

#include <hidef.h>
#include <mc9s12dp256.h>

/* As this exercise will only be executed on simulator, we hardcode the define
 * here */
#ifndef SIMULATOR
#define SIMULATOR
#endif

#define BUTTON1 (0x01)
#define BUTTON2 (0x02)
#define BUTTON3 (0x04)
#define BUTTON4 (0x08)

#define BUTTON_PORT (PTH)

#define CHAR_PER_CLOCK_DIGIT (2)
#define CHAR_PER_DEC (6)
#define LCD_CHAR_PER_LINE (16)

#ifdef SIMULATOR
#define IS_BUTTON_PRESSED(MASK) (BUTTON_PORT & (MASK))
#else
#define IS_BUTTON_PRESSED(MASK) (!(BUTTON_PORT & (MASK)))
#endif

static int press_start_tick = 0;

static char line_buffer[LCD_CHAR_PER_LINE + 1];
static char dec_buffer[CHAR_PER_DEC];

static void *memcpy(void *dst, const void *src, unsigned char len)
{
    for (; len > 0; len--)
    {
        *((char *)dst) = *((const char *)src);
        ((char *)dst)++;
        ((const char *)src)++;
    }
    return dst;
}

static void init_led(void)
{
    DDRJ_DDRJ1 = 1; // Port J.1 as output
    PTIJ_PTIJ1 = 0;
    DDRB = 0xFF; // Port B as output
    PORTB = 0x55;
}

static void init_buttons(void)
{
    DDRH = 0x00;
}

static void update_time_display(unsigned char hours,
                                unsigned char minutes,
                                unsigned char seconds)
{
    char *cur_pos = &(line_buffer[0]);

    dec_to_ascii_wrapper(dec_buffer, hours);
    cur_pos = memcpy(cur_pos, dec_buffer + CHAR_PER_DEC - CHAR_PER_CLOCK_DIGIT, CHAR_PER_CLOCK_DIGIT);
    *cur_pos = ':';
    cur_pos++;

    dec_to_ascii_wrapper(dec_buffer, minutes);
    cur_pos = memcpy(cur_pos, dec_buffer + CHAR_PER_DEC - CHAR_PER_CLOCK_DIGIT, CHAR_PER_CLOCK_DIGIT);
    *cur_pos = ':';
    cur_pos++;

    dec_to_ascii_wrapper(dec_buffer, seconds);
    cur_pos = memcpy(cur_pos, dec_buffer + CHAR_PER_DEC - CHAR_PER_CLOCK_DIGIT, CHAR_PER_CLOCK_DIGIT);
    *cur_pos = '\n';

    write_line_wrapper(line_buffer, 0);
}

void ui_init(void)
{
    init_led();

    init_lcd();
    write_line_wrapper("Clock Template", 0);
    write_line_wrapper("(C) HE Prof. Z", 1);

    init_buttons();
}

void ui_tick(void)
{
    update_time_display(clock_get_hours(), clock_get_minutes(), clock_get_seconds());
}

void ui_poll_buttons(void)
{
    if (IS_BUTTON_PRESSED(BUTTON1))
    {
        press_start_tick = clock_get_current_tick();
    }
    if (IS_BUTTON_PRESSED(BUTTON2))
    {
        press_start_tick = clock_get_current_tick();
    }
    if (IS_BUTTON_PRESSED(BUTTON3))
    {
        press_start_tick = clock_get_current_tick();
    }
}
