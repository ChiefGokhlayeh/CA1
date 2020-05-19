/**
 * @brief UI module, responsible for managing the user interface and
 * interactions.
 *
 * The UI module relies on two separate time bases; *short tick* and
 * *long tick*.
 *
 * The short tick is used for low-latency user interaction like button presses
 * (including debouncing). It is expected to be called with 10 Hz frequency.
 *
 * The long tick is used for slow updates of the user interface, like clock and
 * temperature display. It is expected to be called with 1 Hz frequency.
 */

#ifndef UI_H_
#define UI_H_

/**
 * @brief Initialize the UI module.
 *
 * Internally initializes the LCD driver, button and LED subsystems.
 *
 * Afterwards #ui_short_tick() and #ui_long_tick() may be called by their
 * respective OS timers.
 */
void ui_init(void);

/**
 * @brief Handles low-latency tasks like button presses and debouncing.
 *
 * Call this function with a frequency of 10 Hz.
 */
void ui_short_tick(void);

/**
 * @brief Handles slow tasks like updating the clock and temperature display.
 */
void ui_long_tick(void);

#endif /* UI_H_ */
