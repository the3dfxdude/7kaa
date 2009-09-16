;Filename    : IR_A.ASM
;Description : Remap a bitmap on vga image buffer with clipping


INCLUDE IMGFUN.INC

.CODE

;--------- BEGIN OF FUNCTION IMGremapArea -----------
;
; Remap on the VGA screen
;
; char *imageBuf   - the pointer to the display surface buffer
; int  pitch       - the pitch of the display surface buffer
; int  x1,y1         - the top left vertex of the bar
; char *bitmapPtr  - the pointer to the bitmap array
; char **colorTableArray - the pointer to the scale 0 of remap table array
; int  srcX1, srcY1 srcX2, srcY2 - where to get on the source buffer
;
		PUBLIC IMGremapArea
IMGremapArea    PROC   imageBuf, pitch, x1, y1, bitmapPtr, colorTableArray, srcX1, srcY1, srcX2, srcY2
		LOCAL  mapWidth:DWORD, srcLineDiff:DWORD, destLineDiff:DWORD
		STARTPROC

		MOV	EAX, imageBuf		; store the address of the image buffer to a variable
		MOV	image_buf, EAX

		;------ calc bar width and height -----;

		MOV     AX , DS
		MOV	ES , AX

		XOR	EAX, EAX
		MOV	ESI, bitmapPtr
		LODSW
		MOV	EBX, EAX
		ADD	ESI, 2 			 ; by pass height

		MUL     srcY1			 ; calculate the source starting address
		ADD	EAX, srcX1               ; bitmap width * srcY1 + srcX1
		ADD	ESI, EAX

		MOV	EAX, srcX2		 ; srcLineDiff = bitmap width - (srcX2-srcX1+1)
		SUB	EAX, srcX1
		INC	EAX
		MOV	mapWidth, EAX
		MOV	srcLineDiff, EBX
		SUB	srcLineDiff, EAX

		MOV	EDX, pitch		 ; EDX = lineDiff
		SUB	EDX, EAX		 ; lineDiff = image_width - (srcX2-srcX1+1)
		MOV	destLineDiff, EDX

		MOV	ECX, srcY2		 ; blt lines = srcY2-srcY1+1
		SUB	ECX, srcY1
		INC	ECX

		MOV	EDX, colorTableArray

		CLD                              ; clear direction flag for MOVSB

		;------- pixels copying loop --------;

		CALC_ADDR_2 EDI, x1, y1, srcX1, srcY1, pitch     ; Get the address to the destination buffer

@@startY:	PUSH	ECX
		MOV	ECX, mapWidth
@@startX:
		LODSB
		MOVSX	EAX, AL
		MOV	EBX, [EDX + 4*EAX]
		MOV	AL,[EDI]
		XLATB	[EBX]
		STOSB
		LOOP	@@startX

		ADD	ESI, srcLineDiff
		POP	ECX
		ADD	EDI, destLineDiff
		LOOP	@@startY

@@end:		ENDPROC

IMGremapArea        ENDP


;---------- END OF FUNCTION IMGremapArea ------------

END
