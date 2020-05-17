;
;   Lab 2 - Testprogram for Ticker Interrupt
;
;   Computerarchitektur / Computer Architecture
;   (C) 2020 J. Friedrich, W. Zimmermann
;   Hochschule Esslingen
;
;   Author:   J.Friedrich
;   Last Modified: W.Zimmermann, Apr 15, 2020

; Export symbols
        XDEF Entry, main

; Import symbols
        XREF __SEG_END_SSTACK           ; End of stack
        XREF initTicker                 ; Ticker function

; Include derivative specific macros
        INCLUDE 'mc9s12dp256.inc'

; Defines

; RAM: Variable data section
.data:  SECTION

; ROM: Constant data
.const: SECTION

; ROM: Code section
.init:  SECTION

main:
Entry:
        LDS  #__SEG_END_SSTACK          ; Initialize stack pointer
        CLI                             ; Enable interrupts, needed for debugger

        ldaa #$FF
        staa DDRB                       ; Initialize port B as outputs
        clr  PORTB
        
        BSET DDRJ,#2                    ; Initialize port J.1 as output
        BCLR PTJ,#2

        jsr  initTicker                 ; Initialize the ticker

back:   bra back




