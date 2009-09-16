;Filename    : I_BLTT.ASM
;Description : Blt a bitmap to the display surface buffer without color key transparency handling


INCLUDE IMGFUN.INC

.CODE


;----------- BEGIN OF FUNCTION IMGblt ------------
;
; Put an non-compressed bitmap on image buffer.
; It does not handle color key transparency.
;
; Syntax : IMGblt( imageBuf, pitch, x, y, bitmapBuf )
;
; char *imageBuf - the pointer to the display surface buffer
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

		PUBLIC    IMGblt
IMGblt   	PROC   	  imageBuf, pitch, x, y, bitmapPtr
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

		LODSW                            ; get bitmap height
		MOV     ECX, EAX

		MOV	EDX, pitch	 ; EDX = lineDiff
		SUB	EDX, EBX		 ; lineDiff = image_width - bitmap_width

		CLD                              ; clear direction flag for MOVSB

		;------- pixels copying loop --------;

		CALC_ADDR EDI, x, y, pitch              ; Get the offset to the image buffer address
@@putLine:
		PUSH	ECX
		MOV     ECX, EBX
	    REP MOVSB

		ADD	EDI, EDX		 ; EDX = lineDiff

		POP	ECX
		LOOP    @@putLine                ; decrease the remain height and loop

@@end:          ENDPROC
IMGblt   	ENDP

;----------- END OF FUNCTION IMGblt ----------


END
