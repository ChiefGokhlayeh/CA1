
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
        XDEF delay_0_5sec

; Defines
SPEED:  EQU     2048                   ; Assuming 24MHz clock-cycle and about 3 cycles per loop iteration this should equate to 500ms delay (delay_clks = SPEED * SPEED * 3)

; ROM: Code section
.init: SECTION

delay_0_5sec:
        PSHX                            ; Save registers
        PSHY                            ; (cont.)
        LDX  #SPEED                     ; Reset outer loop counter
waitO:  LDY  #SPEED                     ; Reset inner loop counter
waitI:  DBNE Y, waitI                   ; Decrement Y until it reaches 0, then continue with outer loop
        DBNE X, waitO                   ; Decrement X and if X != 0, iterate through inner loop again
        PULY                            ; Restore registers
        PULX                            ; (cont.)
        RTS                             ; return
