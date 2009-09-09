;Filename    : IB_ATRD.ASM
;Description : Blt a bitmap to the display surface buffer
;	       with decompression, transparency handling and colormaping


INCLUDE IMGFUN.INC
INCLUDE COLCODE.INC

.CODE


;----------- BEGIN OF FUNCTION IMGbltAreaTransRemapDecompress ------
;
; Put a compressed bitmap on image buffer.
; It does handle color key transparency and remaping.
;
; Syntax : IMGbltAreaTransRemapDecompress( imageBuf, pitch, x, y, bitmapBuf,
;                 x1, y1, x2, y2, colorTable)
;
; char *imageBuf - the pointer to the display surface buffer
; int  pitch     - pitch of the display surface buffer
; int  x,y       - where to put the image on the surface buffer
; char *bitmapPtr  - the pointer to the bitmap buffer
; int  x1,y1,x2,y2 - clipping window from left, top of the bitmap
; char *colorTable - color remaping table
;
; the area to be blit to the screen is called clip window
; the width of clip window is stored in clipWindow
;
; ESI and AH points to a point in the bitmap data
; if ESI points to a compression key (i.e. F8-FF),
; AH refer to how many of points are passed.
; i.e. [esi] = F8 21, AH may to 0 to 20
;
; SeekForward function move the (ESI,AH) pointer
; to forward ECX points (decompressed)
;
; ESI and AH are positioned to the top left hand corner of clip window
; and move right, and then downward
;
; After the cursor moved right, it checks for three cases:
; 1. non-transparent data
; 2. transparent data, but do not pass the right of the clip window
; 3. transparent data, and will pass the right of the clip window
;
; for case 1, blit the point and move one point right. If the right
; side of the clip window is passed, position EDI and (ESI,AH) to the
; left side of the clip window on next line.
;
; for case 2, simply move the EDI and (ESI,AH) forward by the run-length
; count
;
; for case 3, skip EDI and (ESI,AH) to the left side of the clip window
; on the next line
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

		PUBLIC    IMGbltAreaTransRemapDecompress
		PUBLIC	  STDCALL SeekForward
IMGbltAreaTransRemapDecompress PROC imageBuf,pitch,x,y,bitmapPtr,x1,y1,x2,y2,colorTable
		LOCAL	clipWidth:DWORD, bitmapWidth:DWORD, bitmapHeight:DWORD
		LOCAL	tempESI:DWORD
		STARTPROC

		MOV	EAX, imageBuf		; store the address of the image buffer to a variable
		MOV	image_buf, EAX

		;------ get the bitmap width and height -----;

		MOV     AX , DS
		MOV	ES , AX
		MOV     ESI, bitmapPtr

		;------ calculate destination on the video memory ----;
		CALC_ADDR_2 EDI, x, y, x1, y1, pitch     ; Get the address to the destination buffer

		;------ calculate clipWidth - no. of points ploted on each row --;
		XOR	EAX, EAX
		LODSW                            ; get bitmap width
		MOV     bitmapWidth, EAX
		MOV	EAX, x2
		SUB	EAX, x1
		INC	EAX
		MOV     clipWidth, EAX

		;----- calculate number of rows to be ploted on the screen
		LODSW                            ; get bitmap height
		MOV	bitmapHeight, EAX
		MOV     ECX, EAX
		MOV	ECX, y2
		SUB	ECX, y1
		; INC	ECX			; leave last line less to @@last_line
		; last line is handled separately to avoid seekForward

		JC	@@end			; if negative, skip the procedure

		; ----- position ESI to the first point to draw -------
		; AH contain the bytes remained in decompression
		; position to row y1 and column x1
		PUSH	ECX
		; ECX = y1 * bitmapWidth + x1
		MOV	EAX, y1
		MUL	bitmapWidth
		ADD	EAX, x1		; DROP EDX
		MOV	ECX, EAX
		MOV	AH,0
		CALL	SeekForward

		POP	ECX		; ECX is now the height of clipping window

		MOV	EBX, colorTable
		JECXZ	@@lastLine

@@loopY:        ; --------- start of each line ------------
		PUSH	ECX

		MOV	ECX, clipWidth
@@loopX:
		; -------- start of each point -----------
		LODSB
		PRE_REMAP
		JUMP_IF_TRANS al, @@clipCompress1
@@nonTrans:
		; ------- non-transparent data -----------
		; 00-F7, simply blit the point on video memory
		POST_REMAP
		STOSB

		; ------- test end of line ----------------
		;  (pass the right of clipping window)
		LOOP	@@loopX
		ADD	EDI, pitch
		SUB	EDI, clipWidth

		; ------- seek source(ESI) to next line -----
		MOV	ECX, bitmapWidth
		SUB	ECX, clipWidth
		MOV	AH,0
		CALL	SeekForward
		JMP	@@endloopY

		ALIGN	4
@@clipCompress1:
		; ------ transparent data -------------
		MOV	tempESI, ESI		; save ESI
		; note: run-length is now in [ESI]-AH
		JUMP_IF_NOT_MANY_TRANS al, @@clipCompress1a
		LODSB				; load the run-length count
		ENCODE_FEW_TRANS_CODE al
@@clipCompress1a:
		MOV	DL,AL
		DECODE_FEW_TRANS_CODE dl
		SUB	DL,AH
		MOVZX	DX,DL

		CMP	ECX, EDX
		JBE	@@clipCompress2

		; ECX > EDX
		; meaning the run-length is still within clip window
		ADD	EDI, EDX	; skip the number of points
		SUB	ECX, EDX
		MOV	AH,0
		JMP	@@loopX

		ALIGN	4
@@clipCompress2:
		; -------  run-length is outside clip window --------
		; EDI seek the the start of the next line
		; EDI += ECX - clipWidth + pitch
		ADD	EDI, pitch
		SUB	ECX, clipWidth
		ADD	EDI, ECX
		; find out the no. of byte to call SeekForward
		; to seek the left side of clip window of next line
		; ECX = (ECX - clipWidth) + bitmapWidth
		ADD	ECX, bitmapWidth
		MOV	ESI, tempESI	; restore ESI to compresion key (f8-ff)
		DEC	ESI
		CALL    SeekForward

@@endloopY:	POP	ECX
		LOOP    @@loopY

		;------- similarly blit the last line -------;
@@lastLine:
		MOV	ECX, clipWidth
@@lastLoopX:
		LODSB
		PRE_REMAP
		JUMP_IF_TRANS al, @@lastClipCompress1
@@lastNonTrans:
		POST_REMAP
		STOSB

		; --------- test end of line ----------
		LOOP	@@lastLoopX
		JMP	@@endLast

		ALIGN	4
@@lastClipCompress1:
		; note: run-length is now in AH not [ESI]
		JUMP_IF_NOT_MANY_TRANS al, @@lastClipCompress1a
		; F8
		LODSB
		ENCODE_FEW_TRANS_CODE al
@@lastClipCompress1a:
		MOV	DL,AL
		DECODE_FEW_TRANS_CODE dl
		SUB	DL,AH
		MOVZX	DX,DL

		CMP	ECX, EDX
		JBE	@@endLast
		; ECX > EDX
		; meaning the run-length is still within clip window
		ADD	EDI, EDX	; skip the number of points
		SUB	ECX, EDX
		MOV	AH,0

		JMP	@@lastLoopX
		ALIGN	4
@@endLast:

@@end:          ENDPROC
IMGbltAreaTransRemapDecompress   	ENDP

;----------- END OF FUNCTION IMGbltAreaTransRemapDecompress ----------



; BUGHERE : do not PRE_REMAP here, as some function do not need remap
;----------- BEGIN OF FUNCTION SeekForward -----
; change ESI, bytes started in compression key in AH
; to forward ECX points

SeekForward	PROC
		PUSH	EDX
		XOR	EDX,EDX

@@loopstart:	JECXZ	@@end2
@@loopstart1:	LODSB
		JUMP_IF_TRANS al, @@compressed1

		;----- non-transparent key, skip it --------;
		LOOP	@@loopstart1
		JMP	@@end1
		ALIGN	4
@@compressed1:
		JUMP_IF_MANY_TRANS al, @@compressed2
		; [ESI] is between F9 and FF
		; the decompressed length is NEG AL
		; but it has passed AH byte of them
		MOV	DL,AL
		DECODE_FEW_TRANS_CODE dl
		MOV	AL,CL
		SUB     DL,AH		; no. of steps to proceed is DL
					; = (runlength) - AH
		ADD	AL,AH		; AL will be moved to AH if
					; EDX > ECX

		SUB	ECX,EDX
		MOV	AH,0		; don't optimize it to XOR AH,AH
		JAE	@@loopstart

		;----- skip to much, go back, and return AH -------;
		DEC	ESI		; ESI point back to byte F?
		MOV	AH,AL		; AH += ECX (when at @@compressed1)
		JMP	@@end2
		ALIGN	4
@@compressed2:
		; [ESI] is F8
		LODSB			; load the run-length count
		MOV	DL,AL
		MOV	AL,CL
		SUB	DL,AH
		ADD	AL,AH		; AL will be moved to AH if
					; EDX > ECX
		SUB	ECX,EDX
		MOV	AH,0		; don't optimize it to XOR AH,AH
		JAE	@@loopstart

		;----- skip too much, go back and return AH ------;
		DEC	ESI		; ESI point back to F8
		DEC	ESI
		MOV	AH,AL		; AH += ECX (when at @@compressed2)
		JMP	@@end2
		ALIGN	4
@@end1:
		XOR	AH,AH
@@end2:
		POP	EDX
		RET
SeekForward	ENDP


;----------- END OF FUNCTION SeekForward  ------


END
