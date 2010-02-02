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

;Filename    : I_BAR.ASM
;Description : Draw a rectangle bar on vga image buffer


INCLUDE IMGFUN.inc

.CODE

;--------- BEGIN OF FUNCTION IMGbar -----------
;
; Draw a bar on the VGA screen
;
; Note : No border checking is made in this function.
;	 Placing an icon outside image buffer will cause serious BUG.
;
; char *imageBuf   - the pointer to the display surface buffer
; int  pitch       - the pitch of the display surface buffer
; int  x1,y1       - the top left vertex of the bar
; int  x2,y2       - the bottom right vertex of the bar
; int  color       - the color of the line
;
		PUBLIC IMGbar
IMGbar          PROC   imageBuf, pitch, x1, y1, x2, y2, color
		STARTPROC

		MOV	EAX, imageBuf		; store the address of the image buffer to a variable
		MOV	image_buf, EAX

		;------ calc bar width and height -----;

		MOV     AX , DS
		MOV	ES , AX

		MOV     EBX, x2
		SUB	EBX, x1
		INC	EBX

		MOV	ECX, y2
		SUB	ECX, y1
		INC	ECX

		MOV	EDX, pitch		 ; EDX = lineDiff
		SUB	EDX, EBX		 ; lineDiff = image_width - icon_width

		MOV	EAX, color
		; replicate colour to 4 bytes in EAX
		PUSH	ECX
		MOV	AH,AL
		MOV	CX,AX
		SHL	EAX,16
		MOV	AX,CX
		POP	ECX

		CLD                              ; clear direction flag for MOVSB

		;------- pixels copying loop --------;

		CALC_ADDR EDI, x1, y1, pitch            ; Get the offset to the image buffer address
		; ########## begin Gilbert #############;
		TEST	BL,3			 ; test if width a multiple of 4
		JZ	@@putLineDWord

		; width is not a multiple of four
		ROR	BX, 2			 ; BL = width / 4
		SHR	BH, 6			 ; BH = width mod 4, so width must be < 1024
@@putLine:
		PUSH	ECX
		MOVZX   ECX, BL
		REP	STOSD
		MOVZX	ECX, BH
		REP	STOSB
		ADD	EDI, EDX		 ; EDX = lineDiff

		POP	ECX
		LOOP    @@putLine                ; decrease the remain height and loop

@@end:          ENDPROC

@@putLineDWord:
		SHR	EBX,2

@@putLineDWord1:
		PUSH	ECX
		MOV	ECX,EBX
		REP	STOSD
		ADD	EDI,EDX
		POP	ECX
		LOOP	@@putLineDWord1

@@end2:		ENDPROC
		;############# end Gilbert ############## ;

IMGbar          ENDP


;---------- END OF FUNCTION IMGbar ------------

END
