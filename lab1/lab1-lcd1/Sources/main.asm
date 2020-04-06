;
;   Lab 1 - Test program for LCD driver
;
;   Computerarchitektur / Computer Architecture
;   (C) 2020 J. Friedrich, W. Zimmermann
;   Hochschule Esslingen
;
;   Author:   J.Friedrich
;   Last Modified: W.Zimmermann, Mar 24, 2020

; Export symbols
        XDEF Entry, main

; Import symbols
        XREF __SEG_END_SSTACK                   ; End of stack
        XREF initLCD, writeLine, delay_10ms     ; LCD functions

; Include derivative specific macros
        INCLUDE 'mc9s12dp256.inc'

; Defines

; RAM: Variable data section
.data:  SECTION

; ROM: Constant data
.const: SECTION
MSG1:   dc.b " Just a little",0
MSG2:   dc.b " break please", 0

msgA: DC.B "ABCDEFGHIJKLMnopqrstuvwxyz1234567890",0 	; in LCD line 0
msgB: DC.B "is this OK?", 0 				; in LCD line 1
msgC: DC.B "Keep texts short!", 0 			; in LCD line 0
msgD: DC.B "Oh yeah!", 0 				; in LCD line 1

msgE: DC.B "0123456789ABCDEFGHIJKLMNOPQRST", 0

; ROM: Code section
.init:  SECTION

main:
Entry:
        LDS  #__SEG_END_SSTACK          ; Initialize stack pointer
        CLI                             ; Enable interrupts, needed for debugger

        JSR  delay_10ms                 ; Delay 20ms during power up
        JSR  delay_10ms

        JSR  initLCD                    ; Initialize the LCD

        LDX  #MSG1                      ; MSG1 for line 0, X points to MSG1
        LDAB #0                         ; Write to line 0
        JSR  writeLine

        LDX  #MSG2                      ; MSG2 for line 1, X points to MSG2
        LDAB #1                         ; Write to line 1
        JSR  writeLine

        LDX  #msgA                      ; msgA for line 0, X points to msgA
        LDAB #0                         ; Write to line 0
        JSR  writeLine

        LDX  #msgB                      ; msgB for line 1, X points to msgB
        LDAB #1                         ; Write to line 1
        JSR  writeLine

        LDX  #msgC                      ; msgC for line 0, X points to msgC
        LDAB #0                         ; Write to line 0
        JSR  writeLine

        LDX  #msgD                      ; msgD for line 1, X points to msgD
        LDAB #1                         ; Write to line 1
        JSR  writeLine

back:   BRA back






