#include "thermometer.h"
#include "wrapper.h"

#include <hidef.h>
#include <mc9s12dp256.h>

#define MIN_TEMPERATURE (-3000000L)
#define VOLT_PER_STEP (50)        // round(u_max / (M - 1))
#define DEGREE_PER_VOLT (196)     // round((t_max - t_min) / u_max)
#define CONST_CORRECTION (-12875) // -((error(1023) - error(0)) / 2)
#define RAW_TO_DECIDEGREE_FACTOR (10000)

static long raw_measurement = 0;
static long raw_temperature = 0;
static int temperature = 0;

static int fixed_float_round(int value, int modulus)
{
    if (value % modulus >= 5)
    {
        value += modulus;
    }
    return value /= modulus;
}

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

void thermometer_take_measurement(void)
{
    ATD0CTL5 = 0b10000111; // Start next measurement on single channel 7
    while (!ATD0STAT0_SCF) /* Wait for conversion to complete by polling the
                              status flag */
        ;
    raw_measurement = ATD0DR0; // Read measurement from ADC data register

    raw_temperature = raw_measurement * VOLT_PER_STEP * DEGREE_PER_VOLT + MIN_TEMPERATURE + CONST_CORRECTION;

    temperature = long_divide_int_signed(&raw_temperature, RAW_TO_DECIDEGREE_FACTOR);

    temperature = fixed_float_round(temperature, 10);
}

int thermometer_get_measurement(void)
{
    return temperature;
}
