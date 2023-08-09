;-----------------------------------------------------------------------------
; variables.inc
;
; Stefan Wessels, 2023
; This is free and unencumbered software released into the public domain.

;-----------------------------------------------------------------------------
zp_size = ZP_LAST_INDEX         ; all this for command line
zp_sisz = zp_size - __ZP_START__

.segment "DIRBUF"
.align $100
prodosIOBuffer: .res $400

;-----------------------------------------------------------------------------
.segment "DATA"
zp_save:    .res zp_size
old_interrupt_handler:  .res 4
.align $100
filenames:  .res 256*16
filedata:
