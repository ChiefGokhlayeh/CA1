;   Lab 1 - Test program for hextoascii
;   Convert 16-bit value to zero-terminated ASCIIZ string
;   Main program
;
;   Computerarchitektur / Computer Architecture
;   (C) 2020 J. Friedrich, W. Zimmermann
;   Hochschule Esslingen
;
;   Author:   W.Zimmermann, Jan 30, 2020
;            (based on code provided by J. Friedrich)
;   Modified: 2020-04-05 Andreas Baulig
;

; export symbols
        XDEF Entry, main

; import symbols
        XREF __SEG_END_SSTACK           ; End of stack
        XREF hextoascii

; include derivative specific macros
        INCLUDE 'mc9s12dp256.inc'

; RAM: Variable data section
.data:  SECTION
hex:    DC.B    6

; ROM: Code section
.init:  SECTION

main:                                   ; Begin of the program
Entry:
        LDS  #__SEG_END_SSTACK          ; Initialize stack pointer
        CLI                             ; Enable interrupts, needed for debugger
        
        LDY #0                          ; Initialize counter
        LDX #hex                        ; Load address of output hex-string
        
loop:   TFR Y, D
        JSR hextoascii 
        INY
        BRA loop
