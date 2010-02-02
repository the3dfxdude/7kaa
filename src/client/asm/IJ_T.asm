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

;Filename    : IBJ_T.ASM
;Description : Add a bitmap and another display surface to the display
;              surface on the same place with transparency handling


INCLUDE IMGFUN.inc
INCLUDE COLCODE.inc

.CODE


;-------- BEGIN OF FUNCTION IMGjoinTrans ----------
;
; Put an non-compressed bitmap on image buffer.
; It handles color key transparency. The color key code is 255.
;
; Syntax : IMGjoinTrans( imageBuf, backBuf x, y, bitmapBuf )
;
; char *imageBuf - the pointer to the display surface buffer
; int imgPitch   - the pitch of the display surface buffer
; char *backBuf  - the pointer to the back buffer
; int backPitch  - the pitch of the back buffer
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

		PUBLIC    IMGjoinTrans
IMGjoinTrans   	PROC   	  imageBuf, imgPitch, backBuf, backPitch, x, y, bitmapPtr
		LOCAL     bitmapWidth:DWORD
		STARTPROC

		MOV	EAX, backBuf
		MOV     image_buf, EAX
		CALC_ADDR EBX, x, y, backPitch

		MOV	EAX, imageBuf		; store the address of the image buffer to a variable
		MOV	image_buf, EAX
		CALC_ADDR EDI, x, y, imgPitch   ; Get the offset to the image buffer address

		;------ get the bitmap width and height -----;

		MOV     AX , DS
		MOV	ES , AX
		MOV     ESI, bitmapPtr

		XOR	EAX, EAX
		LODSW                            ; get bitmap width
		MOV     bitmapWidth, EAX
		SUB	imgPitch, EAX		; lineDiff = image_width - bitmap_width
		SUB	backPitch, EAX		; lineDiff = image_width - bitmap_width

		LODSW                            ; get bitmap height
		MOV     ECX, EAX

		CLD                              ; clear direction flag for MOVSB

		;------- pixels copying loop --------;

@@moreLines:
		PUSH	ECX
		MOV     ECX, bitmapWidth	 ; ECX is the line pixel counter

@@moreColumns:
		LODSB
		CMP	AL, TRANSPARENT_CODE
		JNE	@@notrans
		MOV	AL, [EBX]
@@notrans:
		STOSB
		INC	EBX
		LOOP	@@moreColumns
		POP	ECX
		ADD	EDI, imgPitch
		ADD	EBX, backPitch
		LOOP	@@moreLines

@@end:          ENDPROC
IMGjoinTrans	ENDP

;----------- END OF FUNCTION IMGjoinTrans ----------

		END
