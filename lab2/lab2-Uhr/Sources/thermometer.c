#include "thermometer.h"
#include "wrapper.h"

#include <hidef.h>
#include <mc9s12dp256.h>

#define MIN_RAW_TEMPERATURE (-3000000L)
#define VOLT_PER_STEP (50)        // round(u_max / (M - 1))
#define DEGREE_PER_VOLT (196)     // ceil((t_max - t_min) / u_max)
#define CONST_CORRECTION (-12875) // -((error(1023) - error(0)) / 2)
#define RAW_TO_DECIDEGREE_FACTOR (10000)

static long raw_measurement = 0;
static long raw_temperature = 0;
static int temperature = 0;

/**
 * @brief Performs a fixed-decimal-point rounding and division, given a specific
 * radix. The result is a rounded integer with the fixed-decimal-point removed.
 *
 * Example 1:
 * ```c
 * int value = 1234567; // think of it as 123.4567
 * int radix =   10000;
 * int result = fixed_float_round(value, radix);
 * assert(result == 123);
 * ```
 *
 * Example 2:
 * ```c
 * int value = 987; // think of it as 98.7
 * int radix =  10;
 * int result = fixed_float_round(value, radix);
 * assert(result == 99);
 * ```
 *
 * @param value Fixed-decimal-point value to be rounded.
 * @param radix Radix in powers of 10.
 *
 * @return Rounded integer with the fixed-decimal-point divided out and removed.
 */
static int fixed_float_round(int value, int radix)
{
    if (value % radix >= 5)
    {
        value += radix;
    }
    return value /= radix;
}

/* See header. */
void thermometer_init(void)
{
    ATD0CTL2_ADPU = 0x01;  // Enable ADC unit
    ATD0CTL2_AFFC = 0x01;  // Enable auto clear on read
    ATD0CTL2_ASCIE = 0x00; // Disable interrupt (we're using polling)

    ATD0CTL3_S1C = 0x01; // Do a single measurement
    ATD0CTL3_S2C = 0x00; //
    ATD0CTL3_S4C = 0x00; //
    ATD0CTL3_S8C = 0x00; //

    ATD0CTL4_SRES8 = 0x00; // Select 10 bit (highest) resolution
    ATD1CTL4_PRS = 0x05;   // Select highest ADC clockrate

    ATD1CTL5_DJM = 0x01;  // Right-adjust 10 bit value in data register
    ATD1CTL5_DSGN = 0x00; // Select unsigned value
    ATD1CTL5_SCAN = 0x00; // Disable continouos mode (single shot measurements)
    ATD1CTL5_MULT = 0x00; // No multi-channel conversion please
    ATD1CTL5_Cx = 0x07;   // Select ADC channel 7 (thermometer)
}

/* See header. */
void thermometer_take_measurement(void)
{
    ATD0CTL5 = 0b10000111; // Start next measurement on single channel 7
    while (!ATD0STAT0_SCF) /* Wait for conversion to complete by polling the
                              status flag */
        ;
    raw_measurement = ATD0DR0; // Read measurement from ADC data register

    /* Convert the raw measurement (0-1024) to the intermediate temperature
     * value range of -3012700 (equivalent to -30.127°C) to +7012700 (equivalent
     * to +70.127°C). This intermediate value range was chosen to achieve
     * maximum precision, given the integer nature of the pre-calculated
     * conversion coefficients VOLT_PER_STEP and DEGREE_PER_VOLT.
     *
     * To obtain the final temperature value in °C, rounding and division by
     * the 10^5 must be performed. */
    raw_temperature = raw_measurement * VOLT_PER_STEP * DEGREE_PER_VOLT + MIN_RAW_TEMPERATURE + CONST_CORRECTION;

    /* We convert down to deci-°C (10^-1 * 1°C), to be able to store the value
     * in a simple 16-but value. Doing so requires 32-bit division, which would
     * normally be handled by libc code. Here we do it ourselves, using the
     * available hardware instructions */
    temperature = long_divide_int_signed(&raw_temperature, RAW_TO_DECIDEGREE_FACTOR);

    /* Finally we round the value to obtain an acceptable error. */
    temperature = fixed_float_round(temperature, 10);
}

/* See header. */
int thermometer_get_measurement(void)
{
    return temperature;
}
