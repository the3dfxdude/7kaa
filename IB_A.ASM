;Filename    : IB_A.ASM
;Description : Blt a bitmap to the display surface buffer without color key transparency handling


INCLUDE IMGFUN.INC

.CODE


;----------- BEGIN OF FUNCTION IMGbltArea ------------
;
; Put an non-compressed bitmap on image buffer.
; It does not handle color key transparency.
; It can blt a specific area of the source image to the
; destination buffer instead of the whole source image.
;
; Syntax : IMGbltArea( imageBuf, pitch, desX, desY, bitmapBuf, srcX1, srcY1, srcX2, srcY2 )
;
; char *imageBuf    - the pointer to the display surface buffer
; int  pitch        - pitch of the display surface buffer
; int  desX, desY   - where to put the area on the surface buffer
; char *bitmapPtr   - the pointer to the bitmap buffer
; int  srcX1, srcY1 - where to get the area on the source buffer
;      srcX2, srcY2
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

		PUBLIC    IMGbltArea
IMGbltArea   	PROC   	  imageBuf, pitch, desX, desY, bitmapPtr, srcX1, srcY1, srcX2, srcY2
		LOCAL	  srcLineDiff
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
		ADD	ESI, 2			 ; by pass the bitmap height, we don't need it. srcY2 and srcY1 will give us the data we need

		MUL     srcY1			 ; calculate the source starting address
		ADD	EAX, srcX1               ; bitmap width * srcY1 + srcX1
		ADD	ESI, EAX

		MOV	EAX, srcX2		 ; srcLineDiff = bitmap width - (srcX2-srcX1+1)
		SUB	EAX, srcX1
		INC	EAX

		MOV	srcLineDiff, EBX
		SUB	srcLineDiff, EAX

		MOV	EDX, pitch	 ; EDX = lineDiff
		SUB	EDX, EAX		 ; lineDiff = image_width - (srcX2-srcX1+1)

		MOV	EBX, EAX		 ; EBX - line pixel copy count

		MOV	ECX, srcY2		 ; blt lines = srcY2-srcY1+1
		SUB	ECX, srcY1
		INC	ECX

		CLD                              ; clear direction flag for MOVSB

		;---------- pixels copying loop ----------;

		CALC_ADDR_2 EDI, desX, desY, srcX1, srcY1, pitch     ; Get the address to the destination buffer
@@putLine:
		PUSH	ECX
		MOV     ECX, EBX
	    REP MOVSB

		ADD	EDI, EDX		 ; EDX = lineDiff
		ADD	ESI, srcLineDiff

		POP	ECX
		LOOP    @@putLine                ; decrease the remain height and loop

@@end:          ENDPROC
IMGbltArea   	ENDP

;----------- END OF FUNCTION IMGbltArea ----------


END
