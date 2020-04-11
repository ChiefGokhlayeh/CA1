;   Lab 1 - toLower
;   Convert a zero-terminated ASCIIZ string to lower characters
;   Subroutine toLower
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
        XDEF tolower

; Defines

; Defines

; RAM: Variable data section
.data: SECTION

; ROM: Constant data
.const: SECTION

; ROM: Code section
.init: SECTION

tolower:
        PSHX                            ; Save registers
        PSHB                            ;

        TFR D, X                        ; Use X as pointer to current char
        BRA lwr_L2                      ; Jump to loop check

lwr_L0: CMPB #'A'                       ; Check if char in B is between 'A' and 'Z', otherwise continue with next char
        BLT lwr_L1                      ;
        CMPB #'Z'                       ;
        BGT lwr_L1                      ;
        ADDB #$20                       ; Make B lower-case by substracting ASCII distance of 'a' to 'A' i.e. 32 or $20 in hex
        STAB X                          ; Save B at current X address

lwr_L1: INX                             ; Increment index X

lwr_L2: LDAB X
        CMPB #0                         ;
        BGT lwr_L0                      ;

        PULB                            ; Restore registers
        PULX                            ;

        RTS                             ; Return
