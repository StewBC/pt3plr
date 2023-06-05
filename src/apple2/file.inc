;-----------------------------------------------------------------------------
; file.inc
;
; Stefan Wessels, 2023
; This is free and unencumbered software released into the public domain.

;-----------------------------------------------------------------------------
.import __MAIN_SIZE__
.segment "DATA"

getPrefixParam:
    .byte $01
    .addr PATHNAME

createParam:
    .byte $07                                   ; param_count
createName:
    .addr PATHNAME                              ; pathname
    .byte $C3                                   ; access
    .byte $06                                   ; file_type (6 is binay)
    .word $0000                                 ; aux_type
    .byte $01                                   ; storage_type
    .word $0000                                 ; create_date
    .word $0000                                 ; create_time

openParam:
    .byte $03                                   ; param_count
openName:
    .addr PATHNAME                              ; pathname
    .addr prodosIOBuffer                        ; io_buffer
openRef:
    .byte $00                                   ; ref_num

readParam:
    .byte $04                                   ; param_count
readRef:
    .byte $00                                   ; ref_num
readAddress:
    .addr filedata                              ; data_buffer
readLength:
    .word $0200                                 ; request_count
    .word $0000                                 ; trans_count

writeParam:
    .byte $04                                   ; param_count
writeRef:
    .byte $00                                   ; ref_num
writeAddress:
    .word $0000                                 ; data_buffer
writeLength:
    .word $0000                                 ; request_count
    .word $0000                                 ; trans_count

closeParam:
    .byte   $01                                 ; param_count
closeRef:
    .byte   $00                                 ; ref_num
    
;-----------------------------------------------------------------------------
.segment "CODE"

;-----------------------------------------------------------------------------
.proc fileGetPrefix
    jsr MLI
    .byte GET_PREFIX
    .addr getPrefixParam
    ldx PATHNAME
    inx
    stx PATHPOS
    rts
.endproc

;-----------------------------------------------------------------------------
; IN: a - <name, x - >name
; OUT: name is in PATHNAME
.proc fileSetName

    s16 PTR3_L
    ldx PATHPOS                                 ; append to the end of the path
    ldy #0                                      ; from the 1st char pf the name
    lda (PTR3_L),y
    sta TEMP
    inc TEMP
    iny
readName:
    lda (PTR3_L),y                              ; copy name to path
    sta PATHNAME,x
    inx 
    iny
    cpy TEMP
    beq :+ 
    bne readName
:
    dex                                         ; don't count the trailing null
    stx PATHNAME

    rts 

.endproc 

;-----------------------------------------------------------------------------
; PATHNAME contains the name to load (for example fileSetName was used)
.proc fileLoad

    jsr MLI
    .byte OPEN_CALL
    .word openParam
    bcc :+
    jmp error

: 
    lda openRef
    sta readRef
    sta closeRef

    jsr MLI
    .byte READ_CALL
    .word readParam
    bcs error

    jsr MLI
    .byte CLOSE_CALL
    .word closeParam

error:
    rts 

.endproc
