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

;Filename    : IR_BAR.ASM
;Description : Remap a rectangle bar on vga image buffer


INCLUDE IMGFUN.inc

.CODE

;--------- BEGIN OF FUNCTION IMGremapBar -----------
;
; Draw a bar on the VGA screen
;
; Note : No border checking is made in this function.
;	 Placing an icon outside image buffer will cause serious BUG.
;
; char *imageBuf   - the pointer to the display surface buffer
; int  x1,y1       - the top left vertex of the bar
; int  x2,y2       - the bottom right vertex of the bar
; char *colorTable - the pointer to the remap table
;
		PUBLIC IMGremapBar
IMGremapBar     PROC   imageBuf, pitch, x1, y1, x2, y2, colorTable
		LOCAL  barWidth:DWORD
		STARTPROC

		MOV	EAX, imageBuf		; store the address of the image buffer to a variable
		MOV	image_buf, EAX

		;------ calc bar width and height -----;

		MOV     AX , DS
		MOV	ES , AX

		MOV     EBX, x2
		SUB	EBX, x1
		INC	EBX
		MOV	barWidth, EBX

		MOV	ECX, y2
		SUB	ECX, y1
		INC	ECX

		MOV	EDX, pitch		 ; EDX = lineDiff
		SUB	EDX, EBX		 ; lineDiff = image_width - icon_width

		MOV	EBX, colorTable

		CLD                              ; clear direction flag for MOVSB

		;------- pixels copying loop --------;

		CALC_ADDR EDI, x1, y1, pitch            ; Get the offset to the image buffer address

@@startY:	PUSH	ECX
		MOV	ECX, barWidth
@@startX:
		MOV	AL,[EDI]
		XLATB	[EBX]
		STOSB
		LOOP	@@startX

		POP	ECX
		ADD	EDI, EDX
		LOOP	@@startY

@@end:		ENDPROC

IMGremapBar          ENDP


;---------- END OF FUNCTION IMGremapBar ------------

END
