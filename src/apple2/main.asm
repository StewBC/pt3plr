;-----------------------------------------------------------------------------
; pt3_player, ProDOS pre-loader
;
; Stefan Wessels, 2023
; This is free and unencumbered software released into the public domain.
;
; Use the ca65 assembler and make to build

;-----------------------------------------------------------------------------
; assembler directives
; .debuginfo on
; .listbytes unlimited

;-----------------------------------------------------------------------------
.segment "CODE"

.include "macros.inc"
.include "defs.inc"                             ; constants

.import __ZP_START__

;-----------------------------------------------------------------------------
.proc main
    jmp     start
    .byte   $EE, $EE        ; signature
    .byte   65              ; pathname buffer length ($2005)
str_path:
    .res    65              ; pathname buffer ($2006)

start:
    jsr init          ; Set ZP values to 0
    lda str_path
    bne cmd_line_file
    jsr loadFileNames ; Get a list of PT3 files in the PT3 folder
    bcs quit
    lda ERROR_CODE
    bne quit

cmd_line_file:
    lda #$FF          ; Setup to load any length file (No error checking for too large files)
    sta readLength
    sta readLength+1

    jmp pt3_setup     ; Go to deater's code
quit:
    jsr MLI
    .byte QUIT_CALL
    .addr quitParam

quitParam: 
    .byte 4           ; number of parameters is 4
    .byte 0           ; quit type
    .word 0000        ; reserved
    .byte 0           ; reserved
    .word 0000        ; reserved

.endproc

                      ;-----------------------------------------------------------------------------
; Call before doing anything else
.proc init
    ; clear ZP storage
    lda #0
    ldx #<(ZP_LAST_INDEX - __ZP_START__)
:
    sta <(__ZP_START__-1),x
    dex
    bne :-
    rts
.endproc

;-----------------------------------------------------------------------------
; Load a directory and extract up to 255 file names with .pt3 extension into
; a buffer.  Returns with carry clear if no error, or error code in a.
; Assumes FILE_COUNT_L, FILES_PROCESSED_L, ERROR_CODE and num_files all eq 0
.proc loadFileNames

    ; set up a pointer to the (to be) loaded file data
    lda #<filedata
    sta PTR1_L
    lda #>filedata
    sta PTR1_L+1
    ; set a pointer to where file names start
    lda #<filenames
    sta PTR2_L
    lda #>filenames
    sta PTR2_L+1

    ; get current directory
    jsr fileGetPrefix
    ; add PT3/
    l16 folder
    jsr fileSetName

    ; make adjusted path "permanent"
    lda folder
    clc
    adc PATHPOS
    sta PATHPOS

    ; open the "directory" file PT3/
    jsr MLI
    .byte OPEN_CALL
    .word openParam
    sta ERROR_CODE
    bcc :+
    rts
:
    lda openRef
    sta readRef
    sta closeRef

    ; read the first block
    jsr MLI
    .byte READ_CALL
    .word readParam
    sta ERROR_CODE
    bcs done

    ; extract number of files in this directory
    ldy #$25
    lda (PTR1_L),y
    sta FILE_COUNT_L
    iny
    lda (PTR1_L),y
    sta FILE_COUNT_L+1

    ; prep to extract names - past directory name
    lda #<(filedata+4+.sizeof(s_subDirs))
    sta PTR1_L
    lda #>(filedata+4+.sizeof(s_subDirs))

block_loop:
    lda #2
    sta BLOCK_BANK
    ; extract the filenames this block
    jsr populateFileNames
    bcs done

    ; read the next block
    jsr MLI
    .byte READ_CALL
    .word readParam
    sta ERROR_CODE
    bcs done

    ; reset the pointer to data in the block
    lda #<(filedata+4)
    sta PTR1_L
    lda #>(filedata+4)
    sta PTR1_L+1
    ; and repeat
    bne block_loop

done:
    ; close the file
    jsr MLI
    .byte CLOSE_CALL
    .word closeParam
    bcs :+
    lda ERROR_CODE
    beq :+
    sec
:
    rts

.endproc

;-----------------------------------------------------------------------------
; This will extract the filenames from 1 512 byte block read from disk
; when done, if carry set, the whole directory was processed
; First 255 (up to) files matching storage type 2 and with extension
; .pt3 will have been recoded
.proc populateFileNames

    ; 1 st byte is storage type and file name length
    ldy #0
    lda (PTR1_L),y
    ; if it's zero - deleted - next file
    beq next_file
    sta TEMP
    and #$f0
    ; check for storage type $20 - if not ignore this file
    cmp #$20
    bne next_FILE_COUNT_Led
    lda TEMP
    and #$0f
    ; if the file name is not at least 5 chars, ignore file
    cmp #5
    bcc next_FILE_COUNT_Led
    sta (PTR2_L),y
    tax
:
    ; copy the name
    iny
    lda (PTR1_L),y
    sta (PTR2_L),y
    dex
    ; for the last 3 characters - check that they are .pt3
    cpx #4
    bcs :-
    cmp extension,x
    ; if not .pt3 (3tp.) then next file
    bne next_FILE_COUNT_Led
    cpx #0
    bne :-
    ; the file looks like a tp3, added
    inc NUM_PT3_FILES
    ; Only record up to 255 files
    lda NUM_PT3_FILES
    cmp #$ff
    beq all_done
    ; move the file name pointer to next entry
    lda #$10
    adc PTR2_L
    sta PTR2_L
    bcc next_FILE_COUNT_Led
    inc PTR2_L+1

next_FILE_COUNT_Led:
    ; inc the number of files processed
    inc FILES_PROCESSED_L
    bne :+
    inc FILES_PROCESSED_L+1
:
    ; see if all files processed
    lda FILES_PROCESSED_L
    cmp FILE_COUNT_L
    bne next_file
    lda FILES_PROCESSED_L+1
    cmp FILE_COUNT_L+1
    bne next_file

    ; set carry when all files processed or 255 files remembered
all_done:
    sec
    rts

next_file:
    ; advance the pointer in the file data
    clc
    lda #.sizeof(s_subDirs)
    adc PTR1_L
    sta PTR1_L
    bcc populateFileNames
    inc PTR1_L+1
    ; see if a full block of data was processed
    dec BLOCK_BANK
    bne populateFileNames
    ; done with this block from the file, return with clear carry
    clc
    rts
.endproc

;-----------------------------------------------------------------------------
; Read a file where the name is (1 past the length) in INL/INH
.proc read_file
    ; go back to the length (aligned so don't worry about underflow fixup)
    ldx INL
    dex
    txa
    ldx INH
    jsr fileSetName
    jsr fileLoad
    bcc done
    ; If there was an error, try the next file
    jsr increment_file
    jmp read_file
done: 
    rts
.endproc

;-----------------------------------------------------------------------------
extension: .byte "3TP."
folder: .byte $4,"PT3/"

;-----------------------------------------------------------------------------
.include "zp.inc"                           ; Zero Page usage (variables)
.include "file.inc"                             ; LOAD / SAVE routine
.include "pt3_player.inc"
.include "variables.inc"                        ; variables (DATA segment)