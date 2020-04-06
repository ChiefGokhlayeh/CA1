;
;   Lab 1 - Test program for LCD driver
;
;   Computerarchitektur / Computer Architecture
;   (C) 2020 J. Friedrich, W. Zimmermann
;   Hochschule Esslingen
;
;   Author:   J.Friedrich
;   Last Modified: W.Zimmermann, Mar 24, 2020
;                  Andreas Baulig, Apr 06, 2020

        IFNDEF _HCS12_SERIALMON
            IFNDEF SIMULATOR 
                SIMULATOR: EQU 1
            ENDIF
        ENDIF

; Export symbols
        XDEF Entry, main

; Import symbols
        XREF __SEG_END_SSTACK                   ; End of stack
        XREF initLCD, writeLine, delay_10ms     ; LCD functions
        XREF delay_0_5sec                       ; Delay function
        XREF initLED, setLED, getLED, toggleLED ; LED functions
        XREF hextoascii                         ; hex2ascii function
        XREF dectoascii                         ; dec2ascii function

; Include derivative specific macros
        INCLUDE 'mc9s12dp256.inc'

; RAM: Variable data section
.data:  SECTION
i:      DC.W 1
dec:    DC.B 7
hex:    DC.B 5

; ROM: Constant data
.const: SECTION

; ROM: Code section
.init:  SECTION

main:
Entry:
                    LDS #__SEG_END_SSTACK          ; Initialize stack pointer
                    CLI                             ; Enable interrupts, needed for debugger

                    JSR delay_10ms                 ; Delay 20ms during power up
                    JSR delay_10ms

                    JSR initLCD                    ; Initialize the LCD
                    JSR initLED

                    MOVW #0, i
L0:                 LDD i
                    JSR setLED

                    LDX #dec
                    JSR dectoascii

                    LDAB #0
                    JSR  writeLine

                    LDD i
                    LDX #hex
                    JSR hextoascii

                    LDAB #1
                    JSR  writeLine

L1:                 LDX i
    IFDEF  SIMULATOR
                    BRSET PTH, #$01, button0_pressed
                    BRSET PTH, #$02, button1_pressed
                    BRSET PTH, #$04, button2_pressed
                    BRSET PTH, #$08, button3_pressed
    ELSE
                    BRCLR PTH, #$01, button0_pressed
                    BRCLR PTH, #$02, button1_pressed
                    BRCLR PTH, #$04, button2_pressed
                    BRCLR PTH, #$08, button3_pressed
    ENDIF
                    INX
                    BRA continue
button0_pressed:    TFR X, D
                    ADDD #16
                    TFR D, X
                    BRA continue
button1_pressed:    TFR X, D
                    ADDD #10
                    TFR D, X
                    BRA continue
button2_pressed:    TFR X, D
                    SUBD #10
                    TFR D, X
                    BRA continue
button3_pressed:    TFR X, D
                    SUBD #1
                    TFR D, X
                    BRA continue
continue:           STX i
                    JSR delay_0_5sec
                    BRA L0

back:               BRA back






