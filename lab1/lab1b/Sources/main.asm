
;   Lab 1 - Problem 2.2
;   Incrementing a value once per second and binary output to LEDs
;
;   Computerarchitektur / Computer Architecture
;   (C) 2020 J. Friedrich, W. Zimmermann
;   Hochschule Esslingen
;
;   Author:   W.Zimmermann, Mar 24, 2020
;            (based on code provided by J. Friedrich)
;   Modified: 2020-03-30 Andreas Bauligm Jakob Janusch
;

; export symbols
        XDEF Entry, main

; import symbols
        XREF __SEG_END_SSTACK           ; End of stack
        XREF delay_0_5sec               ; Custom delay function
        XREF initLED                    ; LED API
        XREF setLED                     ; (cont.)
        XREF getLED                     ; (cont.)
        XREF toggleLED                  ; (cont.)

; ROM: Code section
.init: SECTION

main:                                   ; Begin of the program
Entry:
        LDS  #__SEG_END_SSTACK          ; Initialize stack pointer
        CLI                             ; Enable interrupts, needed for debugger

        JSR initLED                     ; Setup LED Pin I/O

; These instructions only serve to test the fuctionallity of led.asm
; A self-test if you will :P
        LDAB #$55                       ; Test out setLED routine...
        JSR setLED                      ; (cont.)

        TFR B, A                        ; Save state of B in A for assert(...) further down
        LDAB #$FF                       ; Test out toggleLED routine by toggeling all LEDs...
        JSR toggleLED                   ; (cont.)

        LDAB #0                         ; Test out getLED routine...
        JSR getLED                      ; (cont.)
        COMA                            ; Do the same operation as toggleLED, just on register A instead
        CBA                             ; Sanity-wise, each bit we wrote via setLED should now be toggled, right?
        BNE panic                       ; Poor man's assert(false)

; Now, get back to actual business
        LDD #0                          ; Reset D counter

start:  JSR setLED                      ; Display current value of B on LEDs
        JSR delay_0_5sec                ; Wait some time (to make LED state visible)
        ADDD #2                         ; Incremet D by 2
        CMPB #63                        ; Check if 63 <= D and ...
        BLS start                       ; ... jump back to start if true
        LDD #0                          ; Reset D and ...
        BRA start                       ; ... jump back to start

panic:  BRA panic                       ; DON'T PANIC! ... but yeah, actually, PANIC!!!
