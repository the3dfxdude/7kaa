;Filename    : I_R.ASM
;Description : Blt a bitmap to the display surface buffer with colour remapping
;              but without color key transparency handling


INCLUDE IMGFUN.INC

.CODE


;----------- BEGIN OF FUNCTION IMGbltRemap ------------
;
; Put an non-compressed bitmap on image buffer.
; It does not handle color key transparency but colorRemapping
;
; Syntax : IMGblt( imageBuf, pitch, x, y, bitmapBuf )
;
; char *imageBuf - the pointer to the display surface buffer
; int  pitch        - pitch of the display surface buffer
; int  x,y       - where to put the image on the surface buffer
; char *bitmapPtr  - the pointer to the bitmap buffer
; char *colorTable - a 256-entry color remapping table
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

		PUBLIC    IMGbltRemap
IMGbltRemap   	PROC   	  imageBuf, pitch, x, y, bitmapPtr, colorTable
		LOCAL	drawWidth
		STARTPROC

		MOV	EAX, imageBuf		; store the address of the image buffer to a variable
		MOV	image_buf, EAX

		;------ get the bitmap width and height -----;

		MOV     AX , DS
		MOV	ES , AX
		MOV     ESI, bitmapPtr

		XOR	EAX, EAX
		LODSW                            ; get bitmap width
		MOV     EBX, EAX
		MOV	drawWidth, EAX

		LODSW                            ; get bitmap height
		MOV     ECX, EAX

		MOV	EDX, pitch	 ; EDX = lineDiff
		SUB	EDX, EBX		 ; lineDiff = pitch - bitmap_width

		CLD                              ; clear direction flag for MOVSB

		;------- pixels copying loop --------;

		CALC_ADDR EDI, x, y, pitch              ; Get the offset to the image buffer address
		MOV	EBX, colorTable
@@putLine:
		PUSH	ECX
		MOV     ECX, drawWidth
@@putPoint:
		LODSB
		XLATB	[EBX]
		STOSB
		LOOP	@@putPoint

		ADD	EDI, EDX		 ; EDX = lineDiff

		POP	ECX
		LOOP    @@putLine                ; decrease the remain height and loop

@@end:          ENDPROC
IMGbltRemap   	ENDP

;----------- END OF FUNCTION IMGbltRemap ----------


END
