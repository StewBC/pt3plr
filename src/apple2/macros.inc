;-----------------------------------------------------------------------------
; macros.inc
;
; Stefan Wessels, 2023
; This is free and unencumbered software released into the public domain.

;-----------------------------------------------------------------------------
.macro l16 address

    .if .paramcount <> 1
    .error "ld16 needs an address (only)"
    .endif

    lda #<address
    ldx #>address

.endmacro 

;-----------------------------------------------------------------------------
.macro s16 address

    .if .paramcount <> 1
    .error "st16 needs an address (only)"
    .endif

    sta address
    stx address+1

.endmacro 
