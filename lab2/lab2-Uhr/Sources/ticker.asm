;
;   Ticker Interrupt,
;   The interrupt service routine shall be called every 10ms
;   Uses Enhanced Capture Timer ECT channel 4
;
;   Computerarchitektur / Computer Architecture
;   (C) 2020 J. Friedrich, W. Zimmermann
;   Hochschule Esslingen
;
;   Author:   J.Friedrich
;   Last Modified: W.Zimmermann, Apr 15, 2020
;   Usage:
;               JSR initTicker --> Initialize ticker
;                                 (must be called once)
;
;   Within the ISR a counter ticks will be incremented. tick counts up to 1sec,
;   and then will be reset. The program checks tick and executes some user defined
;   code (here: toggles the LEDs) once per second
;

; !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
; N O T E: THIS CODE  CONTAINS (at least) 2 BUGS
; You have to fix these bugs, before the code actually works as specified.
; !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

; Export symbols
        XDEF init_ticker

; Import symbols
        XREF handle_long_tick
        XREF handle_short_tick

; Include derivative specific macros
        INCLUDE 'mc9s12dp256.inc'

; Defines
ONESEC      equ 100
TENMS       equ 1875            ; 10 ms
TIMER_ON    equ $80             ; tscr1 value to turn ECT on
TIMER_CH4   equ $10             ; Bit position for channel 4
TCTL1_CH4   equ $03             ; Mask corresponds to TCTL1 OM4, OL4

; RAM: Variable data section
.data:  SECTION
ticks:    ds.b 1                ; Ticker counter

; ROM: Constant data
.const: SECTION

.intVect: SECTION
        ORG $FFE6
int12:  DC.W isrECT4

; ROM: Code section
.init:  SECTION

;********************************************************************
; Public interface function: initLCD ... Initialize Ticker (called once)
; Parameter: -
; Return:    -
init_ticker:
        ldab #TIMER_ON          ; Timer master ON switch
        stab TSCR1
        bset TIOS,#TIMER_CH4    ; Set channel 4 in "output compare" mode

        bset TIE,#TIMER_CH4     ; Enable channel 4 interrupt; bit 4 corresponds to channel 4

        movb #0, ticks          ; Set tick counter to 0

        ; Set timer prescaler (bus clock : prescale factor)
        ; In our case: divide by 2^7 = 128. This gives a timer
        ; driver frequency of 187500 Hz or 5.3333 us time interval
        ldab TSCR2
        andb #$f8
        orab #7
        stab TSCR2
        bclr TCTL1,#TCTL1_CH4   ; Switch timer on
        rts

;********************************************************************
; Internal function: isrECT4 ... Interrupt service routine, called by the timer ticker every 10ms
; Parameter: -
; Return:    -
isrECT4:
        ldd  TC4                ; Schedule the next ISR period
        addd #TENMS
        std  TC4
        ldab #TIMER_CH4         ; Clear the interrupt flag, write a 1 to bit 4
        stab TFLG1

        JSR handle_short_tick   ; Call into C domain and let interrupt be further handled there

        inc  ticks              ; Check, if 1 sec has passed
        ldaa ticks
        cmpa #ONESEC
        bne  notYet             ; If not, skip rest of the ISR

        clr  ticks              ; If yes, execute user's code

        JSR handle_long_tick    ; Call into C domain and let interrupt be further handled there

notYet: rti
