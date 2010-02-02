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

;Filename    : I_READ.ASM
;Description : Read a bitmap from the display surface buffer


INCLUDE IMGFUN.inc

.CODE


;----------- BEGIN OF FUNCTION IMGread ------------
;
; Put an non-compressed bitmap on image buffer.
; It does not handle color key transparency.
;
; Syntax : IMGread( imageBuf, pitch, x1, y1, x2, y2, bitmapBuf )
;
; char *imageBuf      - the pointer to the display surface buffer
; int  pitch          - pitch of the surface buffer
; int  x1, y1, x2, y2 - the read of the surface buffer to read
; char *bitmapPtr     - the pointer to the bitmap buffer
;
;-------------------------------------------------
;
; Format of the bitmap data :
;
; <short>  width
; <short>  height
; <char..> bitmap image
;
;-------------------------------------------------

		PUBLIC    IMGread
IMGread   	PROC   	  imageBuf, pitch, x1, y1, x2, y2, bitmapPtr
		STARTPROC

		MOV	EAX, imageBuf		; store the address of the image buffer to a variable
		MOV	image_buf, EAX

		;------ get the bitmap width and height -----;

		MOV     AX , DS
		MOV	ES , AX
		MOV     EDI, bitmapPtr

		MOV	EAX, x2
		SUB	EAX, x1
		INC 	EAX             	 ; calculate the bitmap width
		STOSW      		         ; store it into the bitmap buffer
		MOV	EBX, EAX		 ; store it to EBX for later internal processing

		MOV	EAX, y2
		SUB	EAX, y1
		INC	EAX			 ; calculate the bitmap height
		STOSW				 ; store it into the bitmap buffer
		MOV     ECX, EAX		 ; store it to ECX for later internal processing

		MOV	EDX, pitch		 ; EDX = lineDiff
		SUB	EDX, EBX		 ; lineDiff = image_width - bitmap_width

		CLD                              ; clear direction flag for MOVSB

		;------- pixels copying loop --------;

		CALC_ADDR ESI, x1, y1, pitch            ; Get the offset to the image buffer address
@@putLine:
		PUSH	ECX
		MOV     ECX, EBX
		REP MOVSB

		ADD	ESI, EDX		 ; EDX = lineDiff

		POP	ECX
		LOOP    @@putLine                ; decrease the remain height and loop

@@end:          ENDPROC
IMGread   	ENDP

;----------- END OF FUNCTION IMGread ----------


END
