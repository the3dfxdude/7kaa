;Filename    : IB_TD.ASM
;Description : Blt a bitmap to the display surface buffer
;	       with decompression, transparency handling


INCLUDE IMGFUN.INC
INCLUDE COLCODE.INC

.CODE


;----------- BEGIN OF FUNCTION IMGbltTransDecompress ------
;
; Put a compressed bitmap on image buffer.
; It does handle color key transparency.
;
; Syntax : IMGbltTransDecompress( imageBuf, pitch, x, y, bitmapBuf)
;
; char *imageBuf - the pointer to the display surface buffer
; int  x,y       - where to put the image on the surface buffer
; char *bitmapPtr  - the pointer to the bitmap buffer
;
; two counters are maintained, EDX and ECX for counting no. of rows
; and no. of columns remaining to draw
; if the counter reach zero, exit the loop
;
; ESI initally points to the start of bitmap data
; EDI initally points to the top left hand cornder of the destination
;     in video memory
;
; compressed data is loaded from ESI, into AL
; If AL is non-transparent, blit the point to video memory.
; If AL is transparent, seek EDI forward. If the right side of the
; destination is passed,
;   1. seek EDI to the left side of the next line
;   2. if run-length is still very long, seek one more line
;   3. residue (of run-length) is added to EDI, ECX will count from a number
;      lower than the width of bitmap
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

		PUBLIC    IMGbltTransDecompress
IMGbltTransDecompress PROC imageBuf,pitch,x,y,bitmapPtr
		LOCAL	bitmapWidth:DWORD, bitmapHeight:DWORD
		STARTPROC

		MOV	EAX, imageBuf		; store the address of the image buffer to a variable
		MOV	image_buf, EAX

		;------ get the bitmap width and height -----;

		MOV     AX , DS
		MOV	ES , AX
		MOV     ESI, bitmapPtr
		CLD

		;------ calculate destination on the video memory ----;
		CALC_ADDR EDI, x, y, pitch	; Get the address to the destination buffer

		;------ calculate bitmapWidth - no. of points ploted on each row --;
		XOR	EAX, EAX
		LODSW                            ; get bitmap width
		MOV     bitmapWidth, EAX

		;----- calculate number of rows to be ploted on the screen
		LODSW                            ; get bitmap height
		MOV	bitmapHeight, EAX
		MOV     EDX, EAX		; EDX contains no. of rows remain

@@loopY:
		MOV	ECX, bitmapWidth
@@loopX:
		LODSB
		JUMP_IF_TRANS al, @@compressed1
@@nonTrans:
		; -----  00-F7, simply blit the point on video memory ----
		STOSB

		; ----------  test end of line ------------
		; (pass the right of clipping window)
		LOOP	@@loopX
		ADD	EDI, pitch
		SUB	EDI, bitmapWidth
		; JMP	@@endloopY	; reduce jump
		DEC	EDX
		JNE	@@loopY
		JMP	@@end

		ALIGN	4
@@compressed1:
		JUMP_IF_NOT_MANY_TRANS al, @@compress1a		; equal to F8
		; F8
		LODSB
		ENCODE_FEW_TRANS_CODE al
@@compress1a:
		; F7-FF
		DECODE_FEW_TRANS_CODE al
		MOVZX	AX,AL

		CMP	ECX, EAX
		JBE	@@compress2
		; ECX > EAX
		; meaning the run-length is still within output bitmap
		ADD	EDI, EAX	; skip the number of points
		SUB	ECX, EAX
		JMP	@@loopX

		ALIGN	4
@@compress2:
		; run-length is outside clip window
		; adjust EDI to point to left of next line
		SUB	EAX, ECX
		ADD	EDI, pitch
		ADD	EDI, ECX
		SUB	EDI, bitmapWidth

@@compress3a:
		; if EAX is longer than width of bitmap,
		; position to EDI one line below
		CMP	EAX, bitmapWidth
		JB	@@compress4
		ADD	EDI, pitch
		SUB	EAX, bitmapWidth
		DEC	EDX			; minus y remain by 1
		JNE	@@compress3a
		JMP	@@end

		ALIGN	4
@@compress4:
		; add remainder to EDI
		; ECX has another initial value other than bitmapWidth
		ADD	EDI, EAX
		MOV     ECX, bitmapWidth
		SUB	ECX, EAX
		DEC	EDX
		JNE	@@loopX
		JMP	@@end

		ALIGN	4
@@endloopY:
		DEC	EDX
		JNE	@@loopY

@@end:          ENDPROC
IMGbltTransDecompress   	ENDP

;----------- END OF FUNCTION IMGbltTransDecompress ----------

END
