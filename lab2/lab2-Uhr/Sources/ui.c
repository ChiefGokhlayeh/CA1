#include "ui.h"

#include "clock.h"
#include "wrapper.h"
#include "thermometer.h"

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

#define INCREMENT_HOURS_BUTTON (BUTTON3)
#define INCREMENT_MINUTES_BUTTON (BUTTON2)
#define INCREMENT_SECONDS_BUTTON (BUTTON1)
#define SWITCH_US_MODE_BUTTON (BUTTON1)
#define SWITCH_MODE_BUTTON (BUTTON4)

#define TOGGLE_LED (0x01)
#define MODE_LED (0x80)

#define BUTTON_PORT (PTH)

#define LONG_PRESS_SHORT_TICK_COUNT (150)
#define SHORT_COOLDOWN_SHORT_TICK_COUNT (40)
#define SHORT_PRESS_SHORT_TICK_COUNT (1)
#define INFO_TEXT_SWITCH_LONG_TICK_COUNT (10)

#define CLOCK_DIGITS_LENGTH (2)
#define TEMPERATURE_DIGITS_LENGTH (2)
#define TEMPERATURE_SIGN_LENGTH (1)
#define DECIMAL_LENGTH (6)
#define LCD_LINE_LENGTH (16)

#define IS_BUTTON_ON_COOLDOWN(MASK, FIELD) ((FIELD) & (MASK))

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

enum ui_info_text
{
    GROUP_MEMBERS,
    COPYRIGHT
};

static const char *COPYRIGHT_TEXT = "(C) IT 2020";
static const char *GROUP_MEMBERS_TEXT = "Baulig, Janusch";

static enum ui_mode cur_mode;

static unsigned int short_tick_count = 0;
static unsigned int press_start_tick = 0;
static unsigned char last_button_state = 0;
static unsigned char button_cooldown_flags = 0;

static char line_buffer[LCD_LINE_LENGTH + 1];
static char dec_buffer[DECIMAL_LENGTH];

static enum ui_info_text cur_info_text;
static unsigned char ticks_info_text_displayed = 0;

static unsigned char us_mode = 0;

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
    PORTB = 0x00;
}

static void init_buttons(void)
{
    DDRH = 0x00;
}

static void update_time_display(void)
{
    int temperature = 0;
    char *cur_pos = &(line_buffer[0]);

    unsigned char hours = clock_get_hours();
    unsigned char pm = 0;
    if (us_mode)
    {
        if (hours >= 12)
        {
            if (hours > 12)
            {
                int tmp = hours;
                hours = (unsigned char)(tmp % 12);
            }
            pm = 1;
        }
        else
        {
            pm = 0;

            if (hours == 0)
            {
                hours = 12;
            }
        }
    }

    dec_to_ascii_wrapper(dec_buffer, hours);
    cur_pos = memcpy(cur_pos, dec_buffer + DECIMAL_LENGTH - CLOCK_DIGITS_LENGTH, CLOCK_DIGITS_LENGTH);
    *cur_pos = ':';
    cur_pos++;

    dec_to_ascii_wrapper(dec_buffer, clock_get_minutes());
    cur_pos = memcpy(cur_pos, dec_buffer + DECIMAL_LENGTH - CLOCK_DIGITS_LENGTH, CLOCK_DIGITS_LENGTH);
    *cur_pos = ':';
    cur_pos++;

    dec_to_ascii_wrapper(dec_buffer, clock_get_seconds());
    cur_pos = memcpy(cur_pos, dec_buffer + DECIMAL_LENGTH - CLOCK_DIGITS_LENGTH, CLOCK_DIGITS_LENGTH);
    *cur_pos = ' ';
    cur_pos++;

    if (us_mode)
    {
        if (pm)
        {
            cur_pos = memcpy(cur_pos, "pm ", 3);
        }
        else
        {
            cur_pos = memcpy(cur_pos, "am ", 3);
        }
    }

    temperature = thermometer_get_measurement();
    dec_to_ascii_wrapper(dec_buffer, temperature);
    cur_pos = memcpy(cur_pos, dec_buffer, TEMPERATURE_SIGN_LENGTH);
    cur_pos = memcpy(cur_pos, dec_buffer + DECIMAL_LENGTH - TEMPERATURE_DIGITS_LENGTH, TEMPERATURE_DIGITS_LENGTH);
    *cur_pos = 'C';
    cur_pos++;
    *cur_pos = '\0';

    write_line_wrapper(line_buffer, 0);
}

static void update_info_text_display(void)
{
    switch (cur_info_text)
    {
    case GROUP_MEMBERS:
        write_line_wrapper(GROUP_MEMBERS_TEXT, 1);
        break;
    case COPYRIGHT:
        write_line_wrapper(COPYRIGHT_TEXT, 1);
        break;
    default:
        write_line_wrapper("", 1);
        break;
    }
}

static void change_mode(enum ui_mode next_mode)
{
    switch (next_mode)
    {
    case NORMAL_MODE:
        clock_enable();

        PORTB &= ~MODE_LED;
        break;

    case SET_MODE:
        clock_disable();

        PORTB |= MODE_LED;
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

    init_buttons();

    cur_mode = NORMAL_MODE;
    cur_info_text = GROUP_MEMBERS;

    update_info_text_display();
}

unsigned int check_button_press(unsigned char cur_button_state, unsigned char button, unsigned int ticks_to_activate)
{
    unsigned int time_pressed;

    if (IS_BUTTON_PRESSED(button, cur_button_state) && !IS_BUTTON_ON_COOLDOWN(button, button_cooldown_flags))
    {
        if (!IS_BUTTON_PRESSED(button, last_button_state))
        {
            press_start_tick = short_tick_count;
        }
        time_pressed = short_tick_count - press_start_tick;

        if (time_pressed > ticks_to_activate)
        {
            button_cooldown_flags |= button;
            return time_pressed;
        }
    }

    if (!IS_BUTTON_PRESSED(button, cur_button_state))
    {
        button_cooldown_flags &= ~button;
    }

    return 0;
}

unsigned int check_button_held(unsigned char cur_button_state, unsigned char button, unsigned int ticks_to_activate, unsigned int ticks_to_reactivate)
{
    unsigned int time_pressed;

    if (IS_BUTTON_PRESSED(button, cur_button_state))
    {
        if (IS_BUTTON_ON_COOLDOWN(button, button_cooldown_flags))
        {
            time_pressed = short_tick_count - press_start_tick;

            if (time_pressed > ticks_to_reactivate)
            {
                press_start_tick = short_tick_count;
                button_cooldown_flags &= ~button;
            }
        }
        else
        {
            if (!IS_BUTTON_PRESSED(button, last_button_state))
            {
                press_start_tick = short_tick_count;
            }
            time_pressed = short_tick_count - press_start_tick;

            if (time_pressed > ticks_to_activate)
            {
                button_cooldown_flags |= button;
                return time_pressed;
            }
        }
    }

    return 0;
}

void ui_short_tick(void)
{
    unsigned char update_required = 0;
    unsigned int ticks_held = 0;
    unsigned char cur_button_state = BUTTON_PORT;

    short_tick_count++;

    switch (cur_mode)
    {
    case NORMAL_MODE:
        if (check_button_press(cur_button_state, SWITCH_MODE_BUTTON, LONG_PRESS_SHORT_TICK_COUNT))
        {
            change_mode(SET_MODE);
        }
        if (check_button_press(cur_button_state, SWITCH_US_MODE_BUTTON, SHORT_PRESS_SHORT_TICK_COUNT))
        {
            us_mode = !us_mode;
        }
        break;
    case SET_MODE:
        if (check_button_press(cur_button_state, SWITCH_MODE_BUTTON, SHORT_PRESS_SHORT_TICK_COUNT))
        {
            change_mode(NORMAL_MODE);
        }

        ticks_held = check_button_held(cur_button_state, INCREMENT_HOURS_BUTTON, SHORT_PRESS_SHORT_TICK_COUNT, SHORT_COOLDOWN_SHORT_TICK_COUNT);
        if (ticks_held)
        {
            update_required = 1;
            clock_set_hours(clock_get_hours() + 1);
        }
        ticks_held = check_button_held(cur_button_state, INCREMENT_MINUTES_BUTTON, SHORT_PRESS_SHORT_TICK_COUNT, SHORT_COOLDOWN_SHORT_TICK_COUNT);
        if (ticks_held)
        {
            update_required = 1;
            clock_set_minutes(clock_get_minutes() + 1);
        }
        ticks_held = check_button_held(cur_button_state, INCREMENT_SECONDS_BUTTON, SHORT_PRESS_SHORT_TICK_COUNT, SHORT_COOLDOWN_SHORT_TICK_COUNT);
        if (ticks_held)
        {
            update_required = 1;
            clock_set_seconds(clock_get_seconds() + 1);
        }

        if (update_required)
        {
            update_time_display();
        }
        break;
    default:
        break;
    }

    last_button_state = cur_button_state;
}

void ui_long_tick(void)
{
    switch (cur_mode)
    {
    case NORMAL_MODE:
        update_time_display();
        break;
    case SET_MODE:
        update_time_display();
        break;
    default:
        break;
    }

    ticks_info_text_displayed++;
    if (ticks_info_text_displayed >= INFO_TEXT_SWITCH_LONG_TICK_COUNT)
    {
        switch (cur_info_text)
        {
        case GROUP_MEMBERS:
            cur_info_text = COPYRIGHT;
            break;
        case COPYRIGHT:
            cur_info_text = GROUP_MEMBERS;
            break;
        default:
            break;
        }

        update_info_text_display();
        ticks_info_text_displayed = 0;
    }

    PORTB = PORTB & TOGGLE_LED ? PORTB & ~TOGGLE_LED : PORTB | TOGGLE_LED;
}
