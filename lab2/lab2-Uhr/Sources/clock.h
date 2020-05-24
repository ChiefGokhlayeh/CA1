#ifndef CLOCK_H_
#define CLOCK_H_

/**
 * @brief Initialize the clock module.
 *
 * Sets the initial time and enables auto-increment on tick.
 */
void clock_init(void);

/**
 * @brief Increments time by one tick (1 sec).
 *
 * Internally takes care of second and minute overflows.
 *
 * Tick is ignored if auto-increment on tick is disabled.
 */
void clock_tick(void);

/**
 * @brief Enable auto-increment on tick.
 */
void clock_enable(void);

/**
 * @brief Disable auto-increment on tick.
 */
void clock_disable(void);

/**
 * @brief Query state of auto-increment on tick.
 *
 * @return True, if auto-increment on tick is enabled, otherwise false.
 */
unsigned char clock_enabled(void);

/**
 * @brief Overwrite current hours count by given value.
 *
 * If @p h is outside the valid range of 0-23, the value will overflow at 24 and
 * wrap around to somewhere within the valid range.
 *
 * @param h
 * New value for hours count. Valid range: 0-23.
 */
void clock_set_hours(unsigned char h);

/**
 * @brief Overwrite current minutes count by given value.
 *
 * If @p m is outside the valid range of 0-59, the value will overflow at 60 and
 * wrap around to somewhere within the valid range.
 *
 * @param m
 * New value for minutes count. Valid range: 0-59.
 */
void clock_set_minutes(unsigned char m);

/**
 * @brief Overwrite current seconds count by given value.
 *
 * If @p s is outside the valid range of 0-59, the value will overflow at 60 and
 * wrap around to somewhere within the valid range.
 *
 * @param s
 * New value for seconds count. Valid range: 0-59.
 */
void clock_set_seconds(unsigned char s);

/**
 * @brief Get current hours count.
 *
 * @return Current hours count. Range: 0-23.
 */
unsigned char clock_get_hours(void);

/**
 * @brief Get current minutes count.
 *
 * @return Current minutes count. Range: 0-59.
 */
unsigned char clock_get_minutes(void);

/**
 * @brief Get current seconds count.
 *
 * @return Current seconds count. Range: 0-59.
 */
unsigned char clock_get_seconds(void);

#endif
