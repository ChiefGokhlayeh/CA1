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

#define INCREMENT_HOURS_BUTTON (BUTTON1)
#define INCREMENT_MINUTES_BUTTON (BUTTON2)
#define INCREMENT_SECONDS_BUTTON (BUTTON3)
#define SWITCH_MODE_BUTTON (BUTTON4)

#define BUTTON_PORT (PTH)

#define LONG_PRESS_TICK_COUNT (2)
#define SHORT_PRESS_TICK_COUNT (0)

#define CHAR_PER_CLOCK_DIGIT (2)
#define CHAR_PER_DEC (6)
#define LCD_CHAR_PER_LINE (16)

#define IS_BUTTON_IGNORED(MASK, FIELD) ((FIELD) & (MASK))

#ifdef SIMULATOR
#define IS_BUTTON_PRESSED(MASK, FIELD) ((FIELD) & (MASK))
#else
#define IS_BUTTON_PRESSED(MASK, FIELD) (!((FIELD) & (MASK)))
#endif

enum ui_mode
{
    NORMAL_MODE,
    SET_MODE
};

static enum ui_mode cur_mode;

static unsigned int press_start_tick = 0;
static unsigned char last_button_state = 0;
static unsigned char button_ignore_flags = 0;

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

static void write_line_time(unsigned int offset,
                            unsigned char hours,
                            unsigned char minutes,
                            unsigned char seconds)
{
    char *cur_pos = &(line_buffer[offset]);

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
    *cur_pos = '\0';

    write_line_wrapper(line_buffer, 0);
}

static void update_time_display(void)
{
    write_line_time(0, clock_get_hours(), clock_get_minutes(), clock_get_seconds());
}

static void update_edit_display(void)
{
    char *cur_pos = &(line_buffer[0]);
    *cur_pos = 'E';
    cur_pos++;
    *cur_pos = ' ';
    cur_pos++;

    write_line_time(cur_pos - line_buffer, clock_get_hours(), clock_get_minutes(), clock_get_seconds());
}

static void change_mode(enum ui_mode next_mode)
{
    switch (next_mode)
    {
    case NORMAL_MODE:
        clock_enable();
        break;

    case SET_MODE:
        clock_disable();
    default:
        break;
    }

    cur_mode = next_mode;
}

void ui_init(void)
{
    init_led();

    init_lcd();
    write_line_wrapper("Clock Template", 0);
    write_line_wrapper("(C) HE Prof. Z", 1);

    init_buttons();

    cur_mode = NORMAL_MODE;
}

unsigned int check_button_press(unsigned char cur_button_state, unsigned char button, unsigned int ticks_to_activate)
{
    unsigned int time_pressed;

    if (IS_BUTTON_PRESSED(button, cur_button_state) && !IS_BUTTON_IGNORED(button, button_ignore_flags))
    {
        if (!IS_BUTTON_PRESSED(button, last_button_state))
        {
            press_start_tick = clock_get_current_tick();
        }
        time_pressed = clock_get_current_tick() - press_start_tick;

        if (time_pressed > ticks_to_activate)
        {
            button_ignore_flags |= button;
            return time_pressed;
        }
    }

    if (!IS_BUTTON_PRESSED(button, cur_button_state))
    {
        button_ignore_flags &= ~button;
    }

    return 0;
}

unsigned int check_button_held(unsigned char cur_button_state, unsigned char button, unsigned int ticks_to_activate)
{
    unsigned int time_pressed;

    if (IS_BUTTON_PRESSED(button, cur_button_state))
    {
        if (!IS_BUTTON_PRESSED(button, last_button_state))
        {
            press_start_tick = clock_get_current_tick();
        }
        time_pressed = clock_get_current_tick() - press_start_tick;

        if (time_pressed > ticks_to_activate)
        {
            return time_pressed;
        }
    }

    return 0;
}

void ui_tick(void)
{
    unsigned char cur_button_state = BUTTON_PORT;
    unsigned int ticks_held = 0;

    switch (cur_mode)
    {
    case NORMAL_MODE:
        update_time_display();

        if (check_button_press(cur_button_state, SWITCH_MODE_BUTTON, LONG_PRESS_TICK_COUNT))
        {
            change_mode(SET_MODE);
        }
        break;
    case SET_MODE:
        update_edit_display();

        if (check_button_press(cur_button_state, SWITCH_MODE_BUTTON, SHORT_PRESS_TICK_COUNT))
        {
            change_mode(NORMAL_MODE);
        }

        ticks_held = check_button_held(cur_button_state, INCREMENT_HOURS_BUTTON, SHORT_PRESS_TICK_COUNT);
        if (ticks_held)
        {
            clock_set_hours(clock_get_hours() + 1);
        }
        ticks_held = check_button_held(cur_button_state, INCREMENT_MINUTES_BUTTON, SHORT_PRESS_TICK_COUNT);
        if (ticks_held)
        {
            clock_set_minutes(clock_get_minutes() + 1);
        }
        ticks_held = check_button_held(cur_button_state, INCREMENT_SECONDS_BUTTON, SHORT_PRESS_TICK_COUNT);
        if (ticks_held)
        {
            clock_set_seconds(clock_get_seconds() + 1);
        }
        break;
    default:
        break;
    }

    last_button_state = cur_button_state;
}
