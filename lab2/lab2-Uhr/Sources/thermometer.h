/**
 * @brief Thermometer Module, manages the thermometer connected via ADC.
 */

#ifndef THERMOMETER_H_
#define THERMOMETER_H_

/**
 * @brief Initialize the module and ADC peripheral.
 *
 * Afterwards measurements can be taken.
 */
void thermometer_init(void);

/**
 * @brief Take a single temperature measurement and convert it to °C.
 *
 * The value is stored internally and can be obtained via
 * #thermometer_get_measurement().
 *
 * Measuring and converting is done synchronously, meaning the caller is blocked
 * until the full procedure is completed.
 */
void thermometer_take_measurement(void);

/**
 * @brief Obtain the last measurement taken via #thermometer_take_measurement().
 *
 * @return Last temperature measurement in °C. Values range from -30 to +70.
 */
int thermometer_get_measurement(void);

#endif /* THERMOMETER_H_ */
