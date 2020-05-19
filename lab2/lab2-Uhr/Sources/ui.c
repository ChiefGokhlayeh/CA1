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

/** @brief Bitmask of the button with ID 1 on pin 0 */
#define BUTTON1 (0x01)
/** @brief Bitmask of the button with ID 2 on pin 1 */
#define BUTTON2 (0x02)
/** @brief Bitmask of the button with ID 3 on pin 2 */
#define BUTTON3 (0x04)
/** @brief Bitmask of the button with ID 4 on pin 3 */
#define BUTTON4 (0x08)

/** @brief Alias of the bitmask for the *increment hours* button, used in SET-mode. */
#define INCREMENT_HOURS_BUTTON (BUTTON3)
/** @brief Alias of the bitmask for the *increment minutes* button. */
#define INCREMENT_MINUTES_BUTTON (BUTTON2)
/** @brief Alias of the bitmask for the *increment seconds* button. */
#define INCREMENT_SECONDS_BUTTON (BUTTON1)
/** @brief Alias of the bitmask for the *switch to US mode* button. */
#define SWITCH_US_MODE_BUTTON (BUTTON1)
/** @brief Alias of the bitmask for the *switch mode* button. */
#define SWITCH_MODE_BUTTON (BUTTON4)

/** @brief Bitmask of the LED on pin 0 */
#define TOGGLE_LED (0x01)
/** @brief Bitmask of the LED on pin 7 */
#define MODE_LED (0x80)

/** @brief Alias of the `PORT` on which the button pins are located */
#define BUTTON_PORT (PTH)

/** @brief Ticks needed for a long press. (short tick domain) */
#define LONG_PRESS_SHORT_TICK_COUNT (150)
/** @brief Ticks needed for a button to reactivate after being detected as pressed. (short tick domain) */
#define SHORT_COOLDOWN_SHORT_TICK_COUNT (40)
/** @brief Ticks needed for a short press. (short tick domain) */
#define SHORT_PRESS_SHORT_TICK_COUNT (1)
/** @brief Ticks needed for the info text to switch. (long tick domain) */
#define INFO_TEXT_SWITCH_LONG_TICK_COUNT (10)

/** @brief Number of chars per digit on a digital clock (i.e. HH, or MM, or SS). */
#define CLOCK_DIGITS_LENGTH (2)
/** @brief Number of chars reserved for the temperature digits. */
#define TEMPERATURE_DIGITS_LENGTH (2)
/** @brief Number of chars reserved for the sign of the temperature display. */
#define TEMPERATURE_SIGN_LENGTH (1)
/** @brief Number of chars in a decimal formatted string, returned by #dec_to_ascii(). */
#define DECIMAL_LENGTH (6)
/** @brief Max number of chars per line on the LCD */
#define LCD_LINE_LENGTH (16)

/** @brief Checks if the given button is currently marked "on-cooldown" in the given field. */
#define IS_BUTTON_ON_COOLDOWN(MASK, FIELD) ((FIELD) & (MASK))

#ifdef SIMULATOR
/** @brief Checks if the given button is currently marked "pressed" in the given field. */
#define IS_BUTTON_PRESSED(MASK, FIELD) ((FIELD) & (MASK))
#else
/** @brief Checks if the given button is currently marked "pressed" in the given field. */
#define IS_BUTTON_PRESSED(MASK, FIELD) (!((FIELD) & (MASK)))
#endif

/**
 * @brief Operating modes of the UI.
 */
enum ui_mode
{
    NORMAL_MODE, //!< NORMAL-mode of operation. Display and update time and temperature every second.
    SET_MODE     //!< SET-mode. Using buttons 1-3 will change hours, minutes or seconds respectively.
};

/**
 * @brief States of the info text display
 */
enum ui_info_text
{
    GROUP_MEMBERS, //!< Display group members.
    COPYRIGHT      //!< Display copyright.
};

static const char *COPYRIGHT_TEXT = "(C) IT 2020";
static const char *GROUP_MEMBERS_TEXT = "Baulig, Janusch";

static enum ui_mode cur_mode;

/**
 * @brief Short tick counter, used to keep track of how many short ticks passed.
 */
static unsigned int short_tick_count = 0;

/**
 * @brief Stores the state of #short_tick_count at the time of last button press.
 *
 * Used to determine length of press.
 */
static unsigned int press_start_tick = 0;

/**
 * @brief Stores the last state of the button PORT field.
 *
 * Used to detect if a button was pressed and then released.
 */
static unsigned char last_button_state = 0;

/**
 * @brief Bitfield storing cooldown information for each button.
 *
 * Used to single-fire press-events and slow down rate of held-events.
 */
static unsigned char button_cooldown_flags = 0;

static char line_buffer[LCD_LINE_LENGTH + 1];
static char dec_buffer[DECIMAL_LENGTH];

static enum ui_info_text cur_info_text;
/**
 * @brief Counter to keep track of how many (long-)ticks the current info text
 * has been displayed.
 */
static unsigned char ticks_info_text_displayed = 0;

/**
 * @brief Flag used to hold the clock display mode state. Set to `1` for US mode
 * (i.e. `HH:MM:ss am/pm`), or `0` for civilized mode.
 */
static unsigned char us_mode = 0;

/**
 * @brief Perform a libc like memcpy. Copy bytes @p len number of bytes from
 * @p src into @p dst, incremet @p dst as we go.
 *
 * @param dst
 * Destination buffer.
 *
 * @param src
 * Source buffer.
 *
 * @param len
 * Number of bytes to copy.
 *
 * @return Position of @p dst after copy.
 */
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

/**
 * @brief Initialize the LED hardware.
 *
 * All LEDs will set to OFF after init.
 */
static void init_led(void)
{
    DDRJ_DDRJ1 = 1; // Port J.1 as output
    PTIJ_PTIJ1 = 0;
    DDRB = 0xFF; // Port B as output
    PORTB = 0x00;
}

/**
 * @brief Initialize the button hardware.
 *
 * Button PORT field may afterwards be used to read the button-press state.
 */
static void init_buttons(void)
{
    DDRH = 0x00;
}

/**
 * @brief Update time and temperature by generating a new line-string and
 * writing it onto the LCD.
 */
static void update_time_and_temperature_display(void)
{
    int temperature = 0;
    char *cur_pos = &(line_buffer[0]);

    unsigned char hours = clock_get_hours();
    unsigned char pm = 0;
    if (us_mode)
    { // ugh...
        if (hours >= 12)
        { // we're in PM domain
            if (hours > 12)
            { // skip 12 PM

                int tmp = hours;                   // temporary is needed because compiler can't generate code for half WORD divide.
                hours = (unsigned char)(tmp % 12); // Get 01:00 pm, 02:00 pm, etc.
            }
            pm = 1;
        }
        else
        { // we're in AM domain
            pm = 0;

            if (hours == 0)
            { // 12 am anomaly
                hours = 12;
            }
        }
    }

    /* The following code converts hours, minutes, seconds and temperature to
     * decimal ASCII strings and appends them to the line buffer. The code is
     * complicated by the fact that dec_to_ascii() is right aligned with '0's
     * padding. It would be technically easier to just change the Assembly code,
     * but we understood "re-use" to mean "don't touch". */
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
    { // insert the am/pm after the digit display
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
    cur_pos = memcpy(cur_pos, dec_buffer, TEMPERATURE_SIGN_LENGTH);                                                // get the sign
    cur_pos = memcpy(cur_pos, dec_buffer + DECIMAL_LENGTH - TEMPERATURE_DIGITS_LENGTH, TEMPERATURE_DIGITS_LENGTH); // get first two digits (enough for -30 to +70°C).
    *cur_pos = 'C';
    cur_pos++;
    *cur_pos = '\0';

    write_line_wrapper(line_buffer, 0);
}

/**
 * @brief Update the info text depending on the current info text display state.
 */
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
        write_line_wrapper("", 1); // should never happen
        break;
    }
}

/**
 * @brief Implement transition actions between operating modes.
 *
 * @param next_mode
 * The mode to transition to.
 */
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

/**
 * @brief Check if the given buffer is being pressed.
 *
 * A button is recognized as *pressed*, if it is held down for number of
 * (short-)ticks given in @p ticks_to_activate. After a press, the button must
 * first be released again before it can trigger the next press event.
 *
 * @param cur_button_state
 * `PORT` field of the button.
 *
 * @param button
 * Bitmask of the button.
 *
 * @param ticks_to_activate
 * Number of (short-)ticks that must have passed since the first notice of the
 * pressed-state before the pressed-event is fired.
 *
 * @return If the button is recognized as pressed, the number of (short-)ticks
 * it has been held for is returned. If the button is not pressed, on cooldown
 * or below the @p ticks_to_activate threshold, 0 is returned.
 */
static unsigned int check_button_press(unsigned char cur_button_state, unsigned char button, unsigned int ticks_to_activate)
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

/**
 * @brief Check if the given buffer is being held.
 *
 * A button is recognized as *held*, if it is held down for number of
 * (short-)ticks given in @p ticks_to_activate. After that, the button is put
 * on cooldown until @p ticks_to_reactivate expires. Should the button then
 * still be held down, a new held-event is fired. This repeats indefinitely
 * until the button is released.
 *
 * The difference between a *pressed* and a *held* event, is that the after a
 * press the button must be released again before the next pressed event can
 * be fired. Held events on the other hand fire continuously with a period of
 * @p ticks_to_reactivate until the button is released.
 *
 * @param cur_button_state
 * `PORT` field of the button.
 *
 * @param button
 * Bitmask of the button.
 *
 * @param ticks_to_activate
 * Number of (short-)ticks that must have passed since the first notice of the
 * pressed-state before the pressed-event is fired.
 *
 * @return If the button is recognized as pressed, the number of (short-)ticks
 * it has been held for is returned. If the button is not pressed, on cooldown
 * or below the @p ticks_to_activate threshold, 0 is returned.
 */
static unsigned int check_button_held(unsigned char cur_button_state, unsigned char button, unsigned int ticks_to_activate, unsigned int ticks_to_reactivate)
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

/* See header. */
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

/* See header. */
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
            update_time_and_temperature_display();
        }
        break;
    default:
        break;
    }

    last_button_state = cur_button_state;
}

/* See header. */
void ui_long_tick(void)
{
    switch (cur_mode)
    {
    case NORMAL_MODE:
        update_time_and_temperature_display();
        break;
    case SET_MODE:
        update_time_and_temperature_display();
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
