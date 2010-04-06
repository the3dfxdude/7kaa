; Seven Kingdoms: Ancient Adversaries
;
; Copyright 1997,1998 Enlight Software Ltd.
;
; This program is free software: you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation, either version 2 of the License, or
; (at your option) any later version.
;
; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License
; along with this program.  If not, see <http://www.gnu.org/licenses/>.
;

;Filename    : I_SNOW.ASM
;Description : Draw random white dots of 32x32 sqaure on vga image buffer


INCLUDE IMGFUN.inc

.CODE

COLOR	= 70h


;--------- BEGIN OF FUNCTION IMGsnow32x32 -----------
;
; Draw random white dots of 32x32 square on the VGA screen
;
; Note : No border checking is made in this function.
;	 Placing an icon outside image buffer will cause serious BUG.
;
; char *imageBuf   - the pointer to the display surface buffer
; int  pitch       - the pitch of the display surface buffer
; int  x1,y1       - the top left vertex of the bar
; int  randSeed    - random seed
; int  seaLevel    - draw white dot if height > seaLevel

		PUBLIC IMGsnow32x32

PUTDOT		MACRO
		LOCAL 	@@putdot1
		ROL	EAX,10
		CMP	AX,DX
		JB	@@putdot1
		MOV	byte ptr [EDI], COLOR
@@putdot1:
		ADD	EDI,2
		ENDM

IMGsnow32x32    PROC   imageBuf, pitch, x1, y1, randSeed, seaLevel
		STARTPROC

		MOV	EAX, imageBuf		; store the address of the image buffer to a variable
		MOV	image_buf, EAX

		MOV	EAX, randSeed
		MOV	EBX, 15a4e35h
		MOV	ECX,16

		CLD                              ; clear direction flag for MOVSB

		;------- pixels copying loop --------;

		CALC_ADDR EDI, x1, y1, pitch            ; Get the offset to the image buffer address
		ADD	EDI, pitch
		INC	EDI

@@line:
		MUL	EBX
		MOV	EDX, seaLevel
		INC	EAX

		PUTDOT
		PUTDOT
		PUTDOT
		PUTDOT

		PUTDOT
		PUTDOT
		PUTDOT
		PUTDOT

		PUTDOT
		PUTDOT
		PUTDOT
		PUTDOT

		PUTDOT
		PUTDOT
		PUTDOT
		PUTDOT

		ADD	EDI,pitch
		ADD	EDI,pitch
		SUB	EDI,32

		DEC	ECX
		JNE	@@line

@@end:          ENDPROC

IMGsnow32x32          ENDP


;---------- END OF FUNCTION IMGsnow32x32 ------------

END
