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

;Filename    : I_XOR.ASM
;Description : Performance XOR operation on a vga image area


INCLUDE IMGFUN.inc

.CODE

;--------- BEGIN OF FUNCTION IMGxor -----------
;
; Performance XOR operation on a vga image area
;
; char *imageBuf   - the pointer to the display surface buffer
; int  pitch       - the pitch of the display surface buffer
; int  x1,y1       - the top left vertex of the bar
; int  x2,y2       - the bottom right vertex of the bar
;
		PUBLIC IMGxor
IMGxor          PROC   imageBuf, pitch, x1, y1, x2, y2
		STARTPROC

		MOV	EAX, imageBuf		; store the address of the image buffer to a variable
		MOV	image_buf, EAX

		MOV    	AX, DS
		MOV    	ES, AX

		CLD                      ; clear direction flag for MOVSB

		MOV    	EBX, x2           	 ; The width of the line
		SUB    	EBX, x1
		INC    	EBX

		MOV    	EDX, y1
@@doLine:
		CALC_ADDR EDI, x1, EDX, pitch             ; Get the offset to the video address

		MOV    	ECX, EBX

@@doPixel:	; XOR   	BYTE PTR ES:[EDI], 0FFH
		NOT   	BYTE PTR ES:[EDI]
		INC    	EDI
		LOOP   	@@doPixel

		INC    	EDX
		CMP	EDX, y2
		JLE	@@doLine

@@end:          ENDPROC
IMGxor          ENDP


;---------- END OF FUNCTION IMGxor ------------

END
