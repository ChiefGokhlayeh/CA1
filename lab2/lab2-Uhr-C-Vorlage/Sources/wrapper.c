#include "wrapper.h"

void wait_for_interrupt(void)
{
    asm WAI;
}

void write_line_wrapper(const char *text, char line)
{
    asm
    {
        LDAB line
        LDX text

        JSR write_line
    }
}

void dec_to_ascii_wrapper(char *txt, int val)
{
    asm
    {
        LDX txt
        LDD val

        JSR dec_to_ascii

        STD val
        STX txt
    }
}
