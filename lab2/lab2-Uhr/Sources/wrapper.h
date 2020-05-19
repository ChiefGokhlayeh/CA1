/*  Wrappers for assembler functions in LCD.asm and decToASCII.asm

    Computerarchitektur / Computer Architecture
    (C) 2020 J. Friedrich, W. Zimmermann
    Hochschule Esslingen

    Author:  W.Zimmermann, Jan 30, 2020
*/

#ifndef WRAPPER_H_
#define WRAPPER_H_

// Prototypes and wrapper functions for dectoascii (from lab 1)
void dec_to_ascii(void);
void dec_to_ascii_wrapper(char *text, int value);

// Prototypes and wrapper functions for LCD driver (from lab 1)
void init_lcd(void);
void delay_10ms(void);
void write_line(void);
void write_line_wrapper(const char *text, char line);

// Prototypes and wrapper functions for ticker (from lab 2)
void init_ticker(void);

// Prototypes and wrapper functions for miscellanous instructions
void wait_for_interrupt(void);
int long_divide_int_signed(long *dividend, int divisor);

// Note: init_lcd, init_ticker and delay_10ms can be called directly without
//       wrappers, as they don't have parameters and return values.

#endif /* WRAPPER_H_ */
