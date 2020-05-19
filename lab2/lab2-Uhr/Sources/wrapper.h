/*  Wrappers for assembler functions in LCD.asm and decToASCII.asm

    Computerarchitektur / Computer Architecture
    (C) 2020 J. Friedrich, W. Zimmermann
    Hochschule Esslingen

    Author:  W.Zimmermann, Jan 30, 2020
*/

/**
 * @brief Wrapper module containing C to Assembly wrapper functions for various
 * operations.
 */

#ifndef WRAPPER_H_
#define WRAPPER_H_

// Prototypes and wrapper functions for dectoascii (from lab 1)
/**
 * @brief Assembly domain function for converting a WORD to ASCII in decimal
 * format.
 *
 * Refer to the Assembly domain implementation for details.
 *
 * This function must be called through its wrapper #dec_to_ascii_wrapper() to
 * make parameters visible.
 *
 * @warning Do not call directly! Use C wrapper instead.
 */
void dec_to_ascii(void);
/**
 * @brief Wrapper function for converting a WORD to ASCII in decimal format.
 *
 * Refer to the Assembly domain implementation for details.
 *
 * @param[in,out] text
 * On input, must point to a allocated char buffer of at least 6 bytes in
 * length. On function exit, this buffer will contain the converted decimal
 * integer character sequence.
 *
 * @param value
 * Integer value to be converted.
 */
void dec_to_ascii_wrapper(char *text, int value);

// Prototypes and wrapper functions for LCD driver (from lab 1)
/**
 * @brief Assembly domain function for initializing the LCD driver.
 */
void init_lcd(void);
/**
 * @brief Assembly domain function for delaying 10 milliseconds.
 */
void delay_10ms(void);
/**
 * @brief Assembly domain function for writing a line buffer onto the LCD.
 *
 * Refer to the Assembly domain implementation for details.
 *
 * @warning Do not call directly! Use C wrapper instead.
 */
void write_line(void);
/**
 * @brief Wrapper function for writing a line buffer onto the LCD.
 *
 * @param text
 * Zero-terminated buffer of chars to be written onto the LCD. Note that the max
 * char count supported by the LCD is 16. Characters beyond this limit are
 * ignored.
 *
 * @param line
 * Line number to write to (0..1).
 */
void write_line_wrapper(const char *text, char line);

// Prototypes and wrapper functions for ticker (from lab 2)
/**
 * @brief Assembly domain function for initializing the tick interrupt
 * generator.
 *
 * Refer to the Assembly domain implementation for details.
 */
void init_ticker(void);

// Prototypes and wrapper functions for miscellanous instructions
/**
 * @brief Wrapper function for halting the CPU until any interrupt occurs.
 *
 * Interrupts must be enabled *before* calling this function.
 */
void wait_for_interrupt(void);
/**
 * @brief Divide a signed `DWORD` (double `WORD`, aka 32-bit) value by a signed
 * `WORD` (16-bit) divisor.
 *
 * The result is expected to be a signed WORD (16-bit) value and is returned.
 *
 * Makes use of the `EDIVS` instruction on HCS12 maschines.
 *
 * @param dividend
 * Pointer to a `DWORD` value, which shall be divided by the given divisor. The
 * reason for this to be passed as a pointer, is the lack of call-convention
 * support for `DWORD` values, without the use of libc.
 *
 * @param divisor
 * 16-but value of the divisor.
 *
 * @return The result of the division as 16-bit value. The result will be
 * cropped, should it be bigger than 16-bits.
 */
int long_divide_int_signed(const long *dividend, int divisor);

// Note: init_lcd, init_ticker and delay_10ms can be called directly without
//       wrappers, as they don't have parameters and return values.

#endif /* WRAPPER_H_ */
