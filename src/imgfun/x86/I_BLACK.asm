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

;Filename    : I_BLACK.ASM
;Description : Draw a black 32x32 sqaure on vga image buffer


INCLUDE IMGFUN.inc

.CODE

COLOR	= 0
DUP_COLOR = COLOR * 01010101h


;--------- BEGIN OF FUNCTION IMGblack32x32 -----------
;
; Draw a black 32x32 square on the VGA screen
;
; Note : No border checking is made in this function.
;	 Placing an icon outside image buffer will cause serious BUG.
;
; char *imageBuf   - the pointer to the display surface buffer
; int  pitch       - the pitch of the display surface buffer
; int  x1,y1       - the top left vertex of the bar

		PUBLIC IMGblack32x32

BLACKLINE	MACRO
		MOV	CL,BL
		REP STOSD
		ADD	EDI,EDX
		ENDM

IMGblack32x32        PROC   imageBuf, pitch, x1, y1
		STARTPROC

		MOV	EAX, imageBuf		; store the address of the image buffer to a variable
		MOV	image_buf, EAX

		MOV	AX, DS
		MOV	ES, AX

		MOV	EDX, pitch	 ; EDX = lineDiff
		SUB	EDX, 32		 ; lineDiff = image_width - icon_width

		MOV	EAX, DUP_COLOR
		MOV	BL,8
		XOR	ECX, ECX

		CLD                              ; clear direction flag for MOVSB

		;------- pixels copying loop --------;

		CALC_ADDR EDI, x1, y1, pitch	; Get the offset to the image buffer address
@@line0:
		BLACKLINE
		BLACKLINE
		BLACKLINE
		BLACKLINE
@@line4:
		BLACKLINE
		BLACKLINE
		BLACKLINE
		BLACKLINE
@@line8:
		BLACKLINE
		BLACKLINE
		BLACKLINE
		BLACKLINE
@@line12:
		BLACKLINE
		BLACKLINE
		BLACKLINE
		BLACKLINE
@@line16:
		BLACKLINE
		BLACKLINE
		BLACKLINE
		BLACKLINE
@@line20:
		BLACKLINE
		BLACKLINE
		BLACKLINE
		BLACKLINE
@@line24:
		BLACKLINE
		BLACKLINE
		BLACKLINE
		BLACKLINE
@@line28:
		BLACKLINE
		BLACKLINE
		BLACKLINE
		BLACKLINE

@@end:          ENDPROC

IMGblack32x32          ENDP


;---------- END OF FUNCTION IMGblack32x32 ------------

END
