;-----------------------------------------------------------------------------
; variables.inc
;
; Stefan Wessels, 2023
; This is free and unencumbered software released into the public domain.

;-----------------------------------------------------------------------------
.segment "DIRBUF"
.align $100
prodosIOBuffer: .res $400

;-----------------------------------------------------------------------------
.segment "DATA"
.align $100
filenames:  .res 256*16
filedata:
