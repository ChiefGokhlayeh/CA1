;   Lab 1 - Test program for hextoascii
;   Convert 16-bit signed value to decimal zero-terminated ASCIIZ string
;   Main program
;
;   Computerarchitektur / Computer Architecture
;   (C) 2020 J. Friedrich, W. Zimmermann
;   Hochschule Esslingen
;
;   Author:   W.Zimmermann, Jan 30, 2020
;            (based on code provided by J. Friedrich)
;   Modified: 2020-04-05 Andreas Baulig, Jakob Janusch
;

; export symbols
        XDEF Entry, main

; import symbols
        XREF __SEG_END_SSTACK           ; End of stack
        XREF dectoascii

; include derivative specific macros
        INCLUDE 'mc9s12dp256.inc'

; RAM: Variable data section
.data:  SECTION
dec:    DC.B    7

; ROM: Code section
.init:  SECTION

main:                                   ; Begin of the program
Entry:
        LDS  #__SEG_END_SSTACK          ; Initialize stack pointer
        CLI                             ; Enable interrupts, needed for debugger

        LDY #$7FFF                      ; Initialize counter
        LDX #dec                        ; Load address of output dec-string

loop:   TFR Y, D
        JSR dectoascii
        INY
        BRA loop
