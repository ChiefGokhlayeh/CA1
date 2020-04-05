
;   Lab 1 - Problem 2.1
;   Incrementing a value once per second and binary output to LEDs
;
;   Computerarchitektur / Computer Architecture
;   (C) 2020 J. Friedrich, W. Zimmermann
;   Hochschule Esslingen
;
;   Author:   W.Zimmermann, Mar 24, 2020
;            (based on code provided by J. Friedrich)
;   Modified: 2020-03-30 Andreas Baulig
;

; export symbols
        XDEF Entry, main

; import symbols
        XREF __SEG_END_SSTACK           ; End of stack

; include derivative specific macros
        INCLUDE 'mc9s12dp256.inc'

; Defines
SPEED:  EQU     2048                   ; Assuming 24MHz clock-cycle and about 3 cycles per loop iteration this should equate to 500ms delay (delay_clks = SPEED * SPEED * 3)

; RAM: Variable data section
.data: SECTION

; ROM: Constant data
.const:SECTION

; ROM: Code section
.init: SECTION

main:                                   ; Begin of the program
Entry:
        LDS  #__SEG_END_SSTACK          ; Initialize stack pointer
        CLI                             ; Enable interrupts, needed for debugger
        
        BSET DDRJ, #2                   ; Bit Set:   Port J.1 as output
        BCLR PTJ,  #2                   ; Bit Clear: J.1=0 --> Activate LEDs

        MOVB #$FF, DDRB                 ; $FF -> DDRB:  Port B.7...0 as outputs (LEDs)
        MOVB #$55, PORTB                ; $55 -> PORTB: Turn on every other LED ... will of course only be overwritten immediately
        
        LDD #0							; Reset D counter
        
start:  STAB PORTB                      ; Display current value of B on LEDs
        JSR delay_0_5sec
        ADDD #2                         ; Incremet D by 2
        CMPB #63                        ; Check if 63 <= D and ...
        BLS start                       ; ... jump back to start if true
        LDD #0                          ; Reset D and ...
        BRA start                       ; ... jump back to start
        
        
delay_0_5sec:
        PSHX
        PSHY
        LDX  #SPEED                     ; 
waitO:  LDY  #SPEED                     ; (Uses two nested counter loops with registers X and Y)
waitI:  DBNE Y, waitI                   ; --- Decrement Y and branch to waitI if not equal to 0
        DBNE X, waitO                   ; --- Decrement X and branch to waitO if not equal to 0
        PULY
        PULX
        RTS