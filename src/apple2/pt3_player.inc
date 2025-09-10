;===============
; VMW PT3 Player
;===============

; zero page definitions
; SQW
; .include	"zp.inc"
.include	"hardware.inc"

; Location the files load at.
; If you change this, you need to update the Makefile
PT3_LOC = filedata ; SQW $4000

PT3_USE_ZERO_PAGE = 1

; Number of files.  Should probably detect this automatically
NUM_FILES	=	20


	;=============================
	; Setup
	;=============================
pt3_setup:
	jsr     HOME
	jsr     TEXT

	bit	LORES		; Lo-res graphics
	bit	SET_GR
	bit	TEXTGR		; split text/graphics

	jsr	clear_screens

	;===============================
	; Check for Apple II/II+/IIe/IIc
	;===============================
	; this is used to see if we have lowecase support

	lda	$FBB3           ; IIe and newer is $06
	cmp	#6
	beq	done_lowecase_smc

	lda	#$d0		; set if older than a IIe
	sta	apple_ii_smc

done_lowecase_smc:


	;===============
	; Init disk code
	;===============

	; SQW
	; jsr	rts_init

	;===============
	; init variables
	;===============

	lda	#0
	sta	DRAW_PAGE
	sta	DONE_PLAYING
	sta	WHICH_FILE
	sta	LOOP

	;=======================
	; Detect mockingboard
	;========================

	jsr	mockingboard_detect		; call detection routine

	bcs	mockingboard_found

	lda	#<not_message			; if not found, print that
	sta	OUTL
	lda	#>not_message
	sta	OUTH
	inc	CV
	jsr	move_and_print

;	jmp	forever_loop			; and wait forever

	jmp	init_mockingboard		; try anyway in slot#4

mockingboard_found:
;	lda     #<found_message			; print found message
;	sta     OUTL
;	lda     #>found_message
;	sta     OUTH
;	inc     CV
;	jsr     move_and_print

	;==================================================
	; patch the playing code with the proper slot value
	;==================================================

	jsr	mockingboard_patch


	;============================
	; Init the Mockingboard
	;============================
init_mockingboard:

	; set up 50Hz interrupt

	jsr	mockingboard_init
	jsr	mockingboard_setup_interrupt

	; init the mockingboard

	jsr	reset_ay_both
	jsr	clear_ay_both

	;==================
	; load first song
	;==================

	jsr	new_song

	;============================
	; Init Background
	;============================

	jsr	set_gr_page0
	jsr	fire_init


	;============================
	; Enable 6502 interrupts
	;============================
start_interrupts:
	cli		; clear interrupt mask


	;============================
	; Loop forever
	;============================
main_loop:

	; Do the visualization

	jsr	draw_fire_frame
	jsr	put_letters
	jsr	page_flip

check_done:
	lda	DONE_PLAYING
	asl			; bit 7 to carry, bit 6 to bit 7
	beq	main_loop	; if was all zeros, loop
	bcs	main_loop	; if high bit set, paused
	sei			; disable interrupts
	bmi	minus_song	; if bit 6 set, then left pressed

				; else, either song finished or
				; right pressed
	; SQW - new commands below
	tax
	and #$40
	bne plus_song
	txa
	and #$20
	bne down_ten_songs
	txa
	and #$10
	bne up_ten_songs
	txa
	and #$08
	bne shuffle_songs
quit_player:
	jmp do_quit_player

shuffle_songs:
	jsr do_shuffle
	; Reseto so 1st file will play
	ldx #0
	stx WHICH_FILE
	beq done_play

plus_song:
	jsr	increment_file
	bcc	done_play	; branch always

minus_song:
	jsr	decrement_file
	bcc done_play ; branch always

down_ten_songs:
	ldx #10
:
	jsr	decrement_file
	dex
	bne :-
	beq done_play


up_ten_songs:
	ldx #10
:
	jsr	increment_file
	dex
	bne :-

done_play:

	lda	#0
	sta	DONE_PLAYING

	; clear the flame
	; FIXME: doesn't matter as we aren't displaying right now

	jsr	fire_setline

	jsr	new_song

	; re-enable the flame
	lda	#7
	jsr	fire_setline

	bmi	start_interrupts






;========================================
;========================================

; Helper routines below

;========================================
;========================================









	;=================
	; load a new song
	;=================

new_song:

	;=========================
	; Init Variables
	;=========================

	; ?

	;===========================
	; Print loading message
	;===========================

	jsr	clear_bottoms		; clear bottom of page 0/1

	lda	#0			; print LOADING message
	sta	CH
	lda	#21
	sta	CV

	lda	#<loading_message
	sta	OUTL
	lda	#>loading_message
	sta	OUTH
        jsr     print_both_pages


	;===========================
	; Load in PT3 file
	;===========================

	lda main::str_path	; SQW check if it's command line or jukebox
	beq jukebox
	cmp #$AF
	bne cmd_line_play
	jmp do_quit_player	; If path is / ($AF) then the song has played, so quit
cmd_line_play:
	tax			; play the command line file
	lda #$AF		; set path to / ($AF) so quits after playing
	sta main::str_path
	lda #1
	sta NUM_PT3_FILES

	stx PATHNAME
:
	lda main::str_path,x	; copy path from command line to our path buffer
	sta PATHNAME,x
	dex
	bne :-

	lda #<(PATHNAME+1)
	sta INL
	lda #>(PATHNAME+1)
	sta INH
	bne :+		; BRA over jukebox load
jukebox:
	jsr	get_filename
:
	lda	#8		; print filename to screen
	sta	CH
	;lda	#21
	;sta	CV

	lda	INL
	sta	OUTL
	lda	INH
	sta	OUTH
	jsr	print_both_pages


	; needs to be space-padded $A0 30-byte filename

	lda	#<readfile_filename
	sta	namlo
	lda	#>readfile_filename
	sta	namhi

	ldy	#0
	ldx	#30		; 30 chars
name_loop:
	lda	(INL),Y
	beq	space_loop
	ora	#$80
	sta	(namlo),Y
	iny
	dex
	bne	name_loop
	beq	done_name_loop
space_loop:
	lda	#$a0		; pad with ' '
	sta	(namlo),Y
	iny
	dex
	bne	space_loop

done_name_loop:

	; open and read a file
	; loads to whatever it was BSAVED at (default is $4000)
	lda main::str_path	; SQW if command line, name at PATHNAME already
	cmp #$AF
	bne juke_read
    jsr fileLoad		; just load that file
	bcc :+
	jmp do_quit_player	

juke_read:
	jsr	read_file		; read PT3 file from disk
:

	;=========================
	; Print Info
	;=========================

	jsr	clear_bottoms		; clear bottom of page 0/1

	; NUL terminate the strings we want to print
	lda	#0
	sta	PT3_LOC+$3E
	sta	PT3_LOC+$62

upcase:
	; if Apple II/II+, uppercase the lowercase letters
apple_ii_smc:
	beq	no_uppercase

	ldy	#$1e
upcase_loop:
	lda	PT3_LOC,Y

	cmp	#$60
	bcc	not_lowercase	; blt
	sbc	#$20
	sta	PT3_LOC,Y
not_lowercase:
	iny
	cpy	#$63
	bne	upcase_loop

no_uppercase:
	; print title

	lda	#>(PT3_LOC+$1E)		; point to header title
	sta	OUTH
	lda	#<(PT3_LOC+$1E)
	sta	OUTL

	lda	#20			; VTAB 20: HTAB 4
	sta	CV
	lda	#4
	sta	CH
	jsr     print_both_pages	; print, tail call


	; Print Author

	lda	#<(PT3_LOC+$42)
	sta	OUTL

	inc	CV			; VTAB 21: HTAB 4
	jsr     print_both_pages	; print, tail call

	; Print clock

	lda	#>(clock_string)	; point to clock string
	sta	OUTH
	lda	#<(clock_string)
	sta	OUTL

	lda	#23			; VTAB 23: HTAB 13
	sta	CV
	lda	#13
	sta	CH
	jsr     print_both_pages	; print, tail call


	; Print which file

	; first update with current values

	lda	WHICH_FILE
	adc	#$1
        ldy     #0
	jsr	convert_decimal

	lda	NUM_PT3_FILES
        ldy     #4
	jsr	convert_decimal

	; now print modified string

	lda	#>(which_song_string)	; point to which song string
	sta	OUTH
	lda	#<(which_song_string)
	sta	OUTL

	lda	#0			; HTAB 1
	sta	CH
	jsr     print_both_pages	; print, tail call



	; Print MHz indicator

	lda	#>(mhz_string)	; point to MHz string
	sta	OUTH
	lda	#<(mhz_string)
	sta	OUTL

	lda	#34			; HTAB 34
	sta	CH
	jsr     print_both_pages	; print, tail call

	; update the MHz indicator with current state

	ldx     #'0'+$80
	lda	convert_177_smc1
	cmp	#$18
	beq	done_MHz

	ldx	#'7'+$80

done_MHz:
        stx     $7F4
        stx     $BF4

	; Print Left Arrow (INVERSE)
	lda	#'<'
	sta	$6D0
	sta	$AD0

	lda	#'-'
	sta	$6D1
	sta	$AD1

	; Print Rright Arrow (INVERSE)
	sta	$6F6
	sta	$AF6

	lda	#'>'
	sta	$6F7
	sta	$AF7

	jsr	pt3_init_song

;=================================
; Calculate Length of Song
;=================================

	; There's no easy way to do this? (???)
	; We walk through the song counting frames
	; We can't even do this quickly, as the number of frames
	;   per pattern can vary, and you have to parse a channel
	;   to see this, and channel data is varying-width and so
	;   you have to parse it all.
	; Time is just number of frames/50Hz


	sta	current_pattern_smc+1

frame_count_loop_store:
	sta	current_line_smc+1
	sta	current_subframe_smc+1

frame_count_loop:

	lda	current_line_smc+1
	ora	current_subframe_smc+1
	bne	fc_pattern_good

	; load a new pattern in
	jsr	pt3_set_pattern

	lda	DONE_SONG
    bne	done_counting

fc_pattern_good:
	ldx     current_subframe_smc+1	;;ldx     #(NOTE_STRUCT_SIZE*0)
	bne	fc_line_good

	; we only calc length of chanel A, hopefully enough

	lda	#1
	sta	pt3_pattern_done_smc+1

        ; decode_note(&pt3->a,&(pt3->a_addr),pt3);
        jsr     decode_note

	lda	pt3_pattern_done_smc+1
	bne	fc_line_good

	inc     current_pattern_smc+1   ; increment pattern
        bne     frame_count_loop_store	; branch always

fc_line_good:
        inc     current_subframe_smc+1  ; subframe++
        lda     current_subframe_smc+1
        eor     pt3_speed_smc+1         ; if we hit pt3_speed, move to next
        bne     fc_do_frame

fc_next_line:
        sta     current_subframe_smc+1	; reset subframe to 0

        inc     current_line_smc+1      ; and increment line
        lda     current_line_smc+1

        eor     #64			; always end at 64.
        bne     fc_do_frame		; is this always needed?

fc_next_pattern:
        sta     current_line_smc+1	; reset line to 0

        inc     current_pattern_smc+1   ; increment pattern

fc_do_frame:
	inc	time_frame_smc+1
time_frame_smc:
	lda	#$00
	eor	#50
	bne	frame_count_loop

	sta	time_frame_smc+1

	; see if overflow low s
	ldx	$BD0+13+10
	cpx	#'9'+$80
	bne	inc_low_s

	; see if overflow high s
	ldx	$BD0+13+9
	cpx	#'5'+$80
	bne	inc_high_s

	inc	$7D0+13+7
	inc	$BD0+13+7

	ldx	#'0'+$80-1

inc_high_s:
	inx
	stx	$7D0+13+9
	stx	$BD0+13+9

clear_low_s:
	ldx	#'0'+$80-1

inc_low_s:
	inx
	stx	$7D0+13+10
	stx	$BD0+13+10

inc_done:

fc_bayern:
	bne	frame_count_loop


done_counting:

	; re-init, as we've run through it
	lda	#0
	sta	DONE_PLAYING
	sta	current_pattern_smc+1

	jmp	pt3_init_song









; SQW - touched/added all the code below

	;==================
	; Get filename
	;==================
	; WHICH_FILE holds number
	; MAX_FILES has max
	; Scroll through until find
	; point INH:INL to it
get_filename:

	lda WHICH_FILE
	sta INL
	lda #0
	sta INH
	asl INL	; * 16
	rol INH
	asl INL
	rol INH
	asl INL
	rol INH
	asl INL
	rol INH
	clc	; 1 past start - skip name length
	lda #<(filenames+1)
	adc INL
	sta INL
	lda #>(filenames+1)
	adc INH
	sta INH
	rts

	;===============================
	; Increment file we want to load
	;===============================
increment_file:
	inc	WHICH_FILE
	lda	WHICH_FILE
	eor	NUM_PT3_FILES
	bne	done_increment
	sta	WHICH_FILE
done_increment:
	rts

	;===============================
	; Decrement file we want to load
	;===============================
decrement_file:
	dec	WHICH_FILE
	lda WHICH_FILE
	cmp NUM_PT3_FILES
	bcc done_decrement
	ldy NUM_PT3_FILES
	dey
	sty	WHICH_FILE
	clc
done_decrement:
	rts


	;=========================
	; convert_decimal
	;=========================
	; convert byte (<100) to tens/ones decimal
	; this is probably not the optimal way to do this
	; value in A, output in ASCII+$80 in which_10s:which_1s
	; trashes X
convert_decimal:
	sed             ; output gets added up in decimal.
	sta HTD_IN
	lda #0
	sta HTD_OUT_L   ; inititalize output word as 0.
	sta HTD_OUT_H   ; (nmos 6502 will need lda#0, sta ...)
	ldx #0eh        ; $e is 14 for 2x7 bits.  (0-7 is 8 positions.)
dec_loop:
	asl HTD_IN      ; look at next high bit.  if it's 0,
	bcc htd1        ; don't add anything to the output for this bit.
	lda HTD_OUT_L   ; otherwise get the running output sum
	clc
	adc table,x     ; and add the appropriate value for this bit
	sta HTD_OUT_L    ; from the table, and store the new sum.
	lda HTD_OUT_H   ; after low byte, do high byte.
	adc table+1,x
	sta HTD_OUT_H
htd1:   
	dex             ; go down to next bit value to loop again.
	dex
	bpl dec_loop    ; if still not done, go back for another loop.
	cld
	lda HTD_OUT_H
	and #$0f
	ora #$30
	sta which_song_string,y
	iny
	lda HTD_OUT_L
	and #$f0
	lsr 
	lsr
	lsr 
	lsr
	ora #$30
	sta which_song_string,y
	iny
	lda HTD_OUT_L
	and #$0f
	ora #$30
	sta which_song_string,y
	rts

table:      .word    1, 2, 4, 8, 16H, 32H, 64H, 128H
                ; (Word directive puts low byte first.)

do_shuffle:
	; iteare over all the file names
	ldx NUM_PT3_FILES
shuffle_loop:
	dex
	cpx #$FF
	bne :+
	rts
:
	; turn index into a pointer
	stx PTR1_L
	lda #0
	sta PTR1_H
	asl PTR1_L
	rol PTR1_H
	asl PTR1_L
	rol PTR1_H
	asl PTR1_L
	rol PTR1_H
	asl PTR1_L
	rol PTR1_H
	clc
	lda #<filenames
	adc PTR1_L
	sta PTR1_L
	lda #>filenames
	adc PTR1_H
	sta PTR1_H
	; get a random index
 	lda	RANDOM
	beq :+
	asl	a
	bcc :++
:
	eor	#$1d
:
	sta	RANDOM
	cmp NUM_PT3_FILES
	bcc :+
	sec
	sbc NUM_PT3_FILES
:
	; turn random index into a pointer
	sta PTR2_L
	lda #0
	sta PTR2_H
	asl PTR2_L
	rol PTR2_H
	asl PTR2_L
	rol PTR2_H
	asl PTR2_L
	rol PTR2_H
	asl PTR2_L
	rol PTR2_H
	clc
	lda #<filenames
	adc PTR2_L
	sta PTR2_L
	lda #>filenames
	adc PTR2_H
	sta PTR2_H
	; swap the two file names
	ldy #$0F
:
	lda (PTR1_L),y;
	; use (unused) index 255 as scratch
	sta filenames+255*16,y
	lda (PTR2_L),y
	sta (PTR1_L),y
	lda filenames+255*16,y
	sta (PTR2_L),y
	dey
	bpl :-
	jmp shuffle_loop

;==========
; filenames
;==========

; SQW
; song_list:
; .include "song_list.inc"

;=========
;routines
;=========
.include	"gr_offsets.inc"
.include	"text_print.inc"
.include	"gr_fast_clear.inc"
.include	"pageflip.inc"
.include	"gr_setpage.inc"
; SQW
; .include	"qkumba_rts.inc"
.include	"keypress_minimal.inc"


; pt3_lib stuff
.include	"pt3_lib_core.inc"
.include	"pt3_lib_init.inc"
.include	"pt3_lib_mockingboard_setup.inc"
.include	"interrupt_handler.inc"
.include	"pt3_lib_mockingboard_detect.inc"

; visualization
.include	"fire.inc"
.include	"random16.inc"
.include	"gr_putsprite.inc"
.include	"put_letters.inc"

;=========
; strings
;=========
;mocking_message:	.asciiz "LOOKING FOR MOCKINGBOARD IN SLOT #4"
not_message:		.byte   "NOT "
found_message:		.asciiz "FOUND"
;done_message:		.asciiz "DONE PLAYING"
loading_message:	.asciiz "LOADING"
clock_string:		.asciiz "0:00 / 0:00"
mhz_string:		.asciiz "1.7MHZ"
which_song_string:	.asciiz	"000/000"
readfile_filename:
	; .byte "SDEMO.KRW                       "
	.byte 'S'|$80,'D'|$80,'E'|$80,'M'|$80,'O'|$80,'.'|$80,'K'|$80,'R'|$80
	.byte 'W'|$80,$A0,$A0,$A0,$A0,$A0,$A0,$A0
	.byte $A0,$A0,$A0,$A0,$A0,$A0,$A0,$A0
	.byte $A0,$A0,$A0,$A0,$A0,$A0,$A0,$A0
	.byte $A0,$A0,$A0,$A0,$A0,$A0
