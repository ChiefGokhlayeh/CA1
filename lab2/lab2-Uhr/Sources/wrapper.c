#include "wrapper.h"

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
extern void dec_to_ascii(void);

/**
 * @brief Assembly domain function for writing a line buffer onto the LCD.
 *
 * Refer to the Assembly domain implementation for details.
 *
 * @warning Do not call directly! Use C wrapper instead.
 */
extern void write_line(void);

void wait_for_interrupt(void)
{
    asm WAI;
}

void write_line_wrapper(const char *text, char line)
{
    asm
    {
        LDAB line       ; Load line number into B
        LDX text        ; Load address of line buffer into X

        JSR write_line  ; Call Assembly subroutine write_line, located in lcd.asm
    }
}

void dec_to_ascii_wrapper(char *txt, int val)
{
    asm
    {
        LDX txt             ; Load address of char buffer into X
        LDD val             ; Load integer value into D

        JSR dec_to_ascii    ; Call Assembly subroutine dec_to_ascii, located in decToASCII.asm
    }
}

int long_divide_int_signed(const long *dividend, int divisor)
{
    int *tmp = &divisor;
    asm
    {
        LDX dividend    ; Load dividend address into X to obtain lower and upper WORD
        LDY 0, X        ; Load lower 16-bit WORD (most siginificant WORD) into Y
        LDD 2, X        ; Load upper 16-but WORD (least significant WORD) into D
        LDX divisor     ; Prepare division by loading division into X
        EDIVS           ; Do signed integer division of (Y:D)/(X)->Y remainder: D
        LDX tmp         ; Load address of divisor variable to receive result (resuse of divisor as it is no longer needed at this point)
        STY 0, X        ; Store result in divisor (now called result)
    }

    return divisor;
}
