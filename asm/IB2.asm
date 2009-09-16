;Filename    : IB2.ASM
;Description : Blt a bitmap to the display surface buffer without color key transparency handling


INCLUDE IMGFUN.INC

.CODE


;----------- BEGIN OF FUNCTION IMGblt2 ------------
;
; Put an non-compressed bitmap on image buffer.
; It does not handle color key transparency.
;
; Syntax : IMGblt2( imageBuf, pitch, x, y, bitmapWidth, bitmapHeight, bitmapBuf )
;
; char *imageBuf    - the pointer to the display surface buffer
; int  pitch        - picth of display surface buffer
; int  x,y          - where to put the image on the surface buffer
; int  bitmapWidth  - width of the bitmap
; int  bitmapHeight - height of the bitmap
; char *bitmapPtr   - the pointer to the bitmap buffer
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

		PUBLIC    IMGblt2
IMGblt2   	PROC   	  imageBuf, pitch, x, y, bitmapWidth, bitmapHeight, bitmapPtr
		STARTPROC

		MOV	EAX, imageBuf		; store the address of the image buffer to a variable
		MOV	image_buf, EAX

		;------ get the bitmap width and height -----;

		MOV     AX , DS
		MOV	ES , AX
		MOV     ESI, bitmapPtr

		MOV     EBX, bitmapWidth
		MOV     ECX, bitmapHeight

		MOV	EDX, pitch	 ; EDX = lineDiff
		SUB	EDX, EBX		 ; lineDiff = image_width - bitmap_width

		CLD                              ; clear direction flag for MOVSB

		;------- pixels copying loop --------;

		CALC_ADDR EDI, x, y, pitch       ; Get the offset to the image buffer address
@@putLine:
		PUSH	ECX
		MOV     ECX, EBX
	    REP MOVSB

		ADD	EDI, EDX		 ; EDX = lineDiff

		POP	ECX
		LOOP    @@putLine                ; decrease the remain height and loop

@@end:          ENDPROC
IMGblt2   	ENDP

;----------- END OF FUNCTION IMGblt2 ----------


END
