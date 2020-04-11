
;   Lab 1 - Problem 2.2
;   Incrementing a value once per second and binary output to LEDs
;
;   Computerarchitektur / Computer Architecture
;   (C) 2020 J. Friedrich, W. Zimmermann
;   Hochschule Esslingen
;
;   Author:   W.Zimmermann, Mar 24, 2020
;            (based on code provided by J. Friedrich)
;   Modified: 2020-03-30 Andreas Baulig, Jakob Janusch
;

; export symbols
        XDEF initLED, setLED, getLED, toggleLED

; include derivative specific macros
        INCLUDE 'mc9s12dp256.inc'

; ROM: Code section
.init: SECTION

initLED:
        BSET DDRJ, #2                   ; Bit Set:   Port J.1 as output
        BCLR PTJ,  #2                   ; Bit Clear: J.1=0 --> Activate LEDs

        MOVB #$FF, DDRB                 ; $FF -> DDRB:  Port B.7...0 as outputs (LEDs)
        MOVB #0, PORTB                  ; Turn off each LED
        RTS                             ; Return (no cleanup needed)

setLED:
        STAB PORTB                      ; Output B on LED display
        RTS                             ; Return (no cleanup needed)

getLED:
        LDAB PORTB                      ; Load current LED state into B
        RTS                             ; Return (no cleanup needed)

toggleLED:
        EORB PORTB                      ; Toggle bits based on bitmask given by B
        STAB PORTB                      ; Output B on LED display
        RTS                             ; Return (no cleanup needed)
