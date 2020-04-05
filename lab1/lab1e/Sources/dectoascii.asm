;   Lab 1 - dectoascii
;   Convert 16-bit signed value to decimal zero-terminated ASCIIZ string
;   Subroutine dectoascii
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
        XDEF dectoascii

; ROM: Constant data
.const: SECTION

; ROM: Code section
.init: SECTION

dectoascii:
        PSHX                ; Save registers
        PSHY                ;
        PSHD                ;
        
        TFR X, Y            ; Transfer address of output string buffer over to Y. We need X for division and Y is otherwise unused, so we use Y to access the buffer from now on.
        
        TSTA                ; Check if D (val) is negative or positive
        BPL d2a_pl          ;
        
        MOVB #'-', Y        ; Else branch: insert '-'
        COMA                ; Make D (val) positive by applying two's complement
        COMB                ;
        ADDD #1             ;
        BRA d2a_cn          ; Exit else branch (continue)

d2a_pl: MOVB #' ', Y        ; Then branch: insert ' ' 

d2a_cn: INY                 ; Increment Y (write end of output string)
        LDX #10000          ; Extract decimal digit of n'th decimal power by performing integer division and contineing with the remainder (aka division/remainder method)
        IDIV                ; Do division
        PSHD                ; Save D (remainder)
        TFR X, D            ; Transfer X (integer result of division) into D so we can add to it.
        ADDD #'0'           ; Add the offset of ASCII '0' to division result
        STAB 1, Y+          ; Store ASCII representation at current write pos.
        PULD                ; Restore D (remainder)
        
        LDX #1000           ; Same as above, but now we extract the 1,000th digit by dividing the remainder of the last division with 1,000.
        IDIV                ;
        PSHD                ;
        TFR X, D            ;
        ADDD #'0'           ;
        STAB 1, Y+          ;
        PULD                ;
        
        LDX #100            ;
        IDIV                ;
        PSHD                ;
        TFR X, D            ;
        ADDD #'0'           ;
        STAB 1, Y+          ;
        PULD                ;
        
        LDX #10             ;
        IDIV                ;
        PSHD                ;
        TFR X, D            ;
        ADDD #'0'           ;
        STAB 1, Y+          ;
        PULD                ;
        
        ADDD #'0'           ; Final step: take remainder of previous division and add the '0' offset onto it
        STAB 1, Y+          ;
        
        CLR Y               ; Insert zero-terminator
        
        PULD                ; Restore registers
        PULY                ;
        PULX                ;
        RTS                 ; Return
