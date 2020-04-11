;   Lab 1 - hextoascii
;   Convert 16-bit value to zero-terminated ASCIIZ string
;   Subroutine hextoascii
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
        XDEF hextoascii

; ROM: Constant data
.const: SECTION
H2A:    DC.B "0123456789ABCDEF"

; ROM: Code section
.init: SECTION

hextoascii:
        PSHX                ; Save registers
        PSHY                ;
        PSHD                ;

        LDY #$3078          ; Hard-code the "0x" at the start of string
        STY 6, X+           ;

        CLR X               ; Insert final zero-terminator at end of string
        DEX                 ; Set write pointer to last char in output (loop will go in reverse, saving some shifts)

        LDY #0              ; Initialize Y as counter
        BRA h2a_L2          ; Jump to start of loop

h2a_L0: PSHD                ; Save current D (shifted val)
        ANDB #$0F           ; Mask out all bits except last nibble
        CLRA                ; Clear A register so we can safely do a TFR of entire D (we could do "TFR B, Y" but that would do implicit sign extension, which we actually don't want)
        PSHY                ; Save Y (counter) register
        TFR D, Y            ; Transfer D (last nibble of shifted val) into Y so we can do a array lookup with it
        LDAA H2A, Y         ; Load the corrosponding hex-char from lookup-array
        PULY                ; Restore Y (counter) register
        STAA 1, X-          ; Write back hex-char to output string and decrement write pointer
        PULD                ; Restore D (non-masked val)

h2a_L1: JSR rshiftnibble    ; Shift D by one nibble to the right
        INY                 ; Increment Y counter

h2a_L2: CPY #4              ; Check if counter has reached limit
        BLT h2a_L0          ; Continue loop if counter less than 4

        PULD                ; Restore registers
        PULY                ;
        PULX                ;
        RTS                 ; Return

rshiftnibble:
        PSHX                ; Save registers

        LDX #0              ; Initialize X as counter
        BRA rsh_L2          ; Jump to loop
rsh_L0: LSRD                ; Shift D (=output) by 1 digit to the right
rsh_L1: INX                 ; Increment X counter
rsh_L2: CPX #4              ; Check if counter has reached limit
        BLT rsh_L0          ;

        PULX                ; Restore registers
        RTS                 ; Return
