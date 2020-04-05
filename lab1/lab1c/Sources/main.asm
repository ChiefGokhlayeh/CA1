;   Lab 1 - Test program for toLower
;   Convert a zero-terminated ASCIIZ string to lower characters
;   Main program
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
        XDEF Entry, main

; import symbols
        XREF __SEG_END_SSTACK           ; End of stack
        XREF tolower

; include derivative specific macros
        INCLUDE 'mc9s12dp256.inc'

; Defines

; RAM: Variable data section
.data:  SECTION
Vtext:  DS.B    80                      ; String array (in RAM)

; ROM: Constant data
.const: SECTION
Ctext:  DC.B  "Test 12345 *!? ABCDE abcde zZ", 0

; ROM: Code section
.init:  SECTION

main:                                   ; Begin of the program
Entry:
        LDS  #__SEG_END_SSTACK          ; Initialize stack pointer
        CLI                             ; Enable interrupts, needed for debugger

        LDX #Ctext
        LDY #Vtext
        JSR strcpy                      ; Copy string at X to memory at Y
        
        LDD #Vtext                      ; Load address of Vtext into D so we can pass it to tolower()
        JSR tolower
        

halt:   BRA halt

strcpy:
        PSHX                            ; Save registers
        PSHY                            ;
        PSHB                            ;
        
        LDAB X                          ; Load first char of X before entering loop
        BRA cpy_L2                      ; Jump to loop check
        
cpy_L0: LDAB X                          ; Copy Ctext[X] to Vtext[X]
        STAB Y                          ;

cpy_L1: INX                             ; Increment index X
        INY                             ; Increment index Y
        
cpy_L2: CMPB #0                         ; Check if end-of-string is reached by comparing B (i.e. Ctext[X] against '\0')
        BGT cpy_L0                      ; If end-of-string: exit loop
        
        PULB                            ; Restore registers
        PULY                            ;
        PULX                            ;
        
        RTS                             ; Return