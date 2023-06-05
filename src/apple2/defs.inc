;-----------------------------------------------------------------------------
; defs.inc
;
; Stefan Wessels, 2023
; This is free and unencumbered software released into the public domain.

;-----------------------------------------------------------------------------
; System locations
PATHNAME         = $0280
MLI              = $BF00                        ; ProDOS API

;-----------------------------------------------------------------------------
; MLI call type bytes
QUIT_CALL       = $65
OPEN_CALL       = $C8
READ_CALL       = $CA
WRITE_CALL      = $CB
CLOSE_CALL      = $CC
CREATE_CALL     = $C0
GET_PREFIX      = $C7

;-----------------------------------------------------------------------------
; self-modifying address marker
PLACEHOLDER      = $FFFF

.struct s_subDirs
    storage_type_name_length   .res  1
    file_name                  .res  15
    file_type                  .res  1
    key_pointer                .res  2
    blocks_used                .res  2
    eof                        .res  3
    time                       .res  4
    version                    .res  1
    min_version                .res  1
    access                     .res  1
    aux_type                   .res  2
    last_mod                   .res  4
    header_pointer             .res  2
.endstruct
