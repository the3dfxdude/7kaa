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

;Filename    : IB_32.ASM
;Description : Blt a bitmap of 32x32 to the display surface buffer without color key transparency handling

INCLUDE IMGFUN.inc

.CODE


;----------- BEGIN OF FUNCTION IMGblt32x32 ------------
;
; Put an non-compressed 32x32 bitmap on image buffer.
; It does not handle color key transparency.
;
; Syntax : IMGblt( imageBuf, pitch, x, y, bitmapBuf )
;
; char *imageBuf - the pointer to the display surface buffer
; int  pitch     - pitch of the display surface buffer
; int  x,y       - where to put the image on the surface buffer
; char *bitmapPtr  - the pointer to the bitmap buffer
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

PUTLINE		MACRO
		MOV	CL,AL
		REP MOVSD
		ADD	EDI,EDX
		ENDM

		PUBLIC    IMGblt32x32
IMGblt32x32   	PROC   	  imageBuf, pitch, x, y, bitmapPtr
		STARTPROC

		MOV	EAX, imageBuf		; store the address of the image buffer to a variable
		MOV	image_buf, EAX

		;------ get the bitmap width and height -----;
		; compare it with 32x32
		MOV     AX , DS
		MOV	ES , AX
		MOV     ESI, bitmapPtr

		LODSD
		CMP	EAX, 00200020h
		JNE	@@end

		MOV	EDX, pitch	 ; EDX = lineDiff
		SUB	DX, AX 		 ; lineDiff = image_width - bitmap_width

		XOR	EAX, EAX
		XOR	ECX, ECX
		CLD                              ; clear direction flag for MOVSB

		;------- pixels copying loop --------;

		CALC_ADDR EDI, x, y, pitch      ; Get the offset to the image buffer address
		MOV	AL,20h /4		; initial value of CX

@@line0:	PUTLINE
		PUTLINE
		PUTLINE
		PUTLINE

@@line4:	PUTLINE
		PUTLINE
		PUTLINE
		PUTLINE

@@line8:	PUTLINE
		PUTLINE
		PUTLINE
		PUTLINE

@@line12:	PUTLINE
		PUTLINE
		PUTLINE
		PUTLINE

@@line16:	PUTLINE
		PUTLINE
		PUTLINE
		PUTLINE

@@line20:	PUTLINE
		PUTLINE
		PUTLINE
		PUTLINE

@@line24:	PUTLINE
		PUTLINE
		PUTLINE
		PUTLINE

@@line28:	PUTLINE
		PUTLINE
		PUTLINE
		PUTLINE

@@end:          ENDPROC
IMGblt32x32   	ENDP

;----------- END OF FUNCTION IMGblt32x32 ----------


END
