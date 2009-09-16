;Filename    : IR_AM.ASM
;Description : Remap a bitmap on vga image buffer with clipping, with mirroring


INCLUDE IMGFUN.INC

.CODE

;--------- BEGIN OF FUNCTION IMGremapAreaHMirror -----------
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
		PUBLIC IMGremapAreaHMirror
IMGremapAreaHMirror	PROC   imageBuf, pitch, x1, y1, bitmapPtr, colorTableArray, srcX1, srcY1, srcX2, srcY2
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
		ADD	EAX, EBX
		SUB	EAX, srcX2               ; bitmap width * srcY1 + (bitmapWidth - srcX2 -1)
		DEC	EAX
		ADD	ESI, EAX

		MOV	EAX, srcX2		 ; srcLineDiff = bitmap width - (srcX2-srcX1+1)
		SUB	EAX, srcX1
		INC	EAX
		MOV	mapWidth, EAX
		MOV	srcLineDiff, EBX
		SUB	srcLineDiff, EAX

		MOV	EDX, pitch		 ; EDX = lineDiff
		ADD	EDX, EAX		 ; lineDiff = image_width + (srcX2-srcX1+1)
		MOV	destLineDiff, EDX

		MOV	ECX, srcY2		 ; blt lines = srcY2-srcY1+1
		SUB	ECX, srcY1
		INC	ECX

		MOV	EDX, colorTableArray

		CLD                              ; clear direction flag for MOVSB

		;------- pixels copying loop --------;

		CALC_ADDR_2 EDI, x1, y1, srcX2, srcY1, pitch     ; Get the address to the destination buffer

@@startY:	PUSH	ECX
		MOV	ECX, mapWidth
@@startX:
		LODSB
		MOVSX	EAX, AL
		MOV	EBX, [EDX + 4*EAX]
		MOV	AL,[EDI]
		XLATB	[EBX]
		MOV	[EDI],AL
		DEC	EDI
		LOOP	@@startX

		ADD	ESI, srcLineDiff
		POP	ECX
		ADD	EDI, destLineDiff
		LOOP	@@startY

@@end:		ENDPROC

IMGremapAreaHMirror ENDP


;---------- END OF FUNCTION IMGremapAreaHMirror ------------

END
