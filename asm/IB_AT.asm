;Filename    : IB_AT.ASM
;Description : Blt a bitmap to the display surface buffer without color key transparency handling


INCLUDE IMGFUN.INC
INCLUDE COLCODE.INC

.CODE



;----------- BEGIN OF FUNCTION IMGbltAreaTrans ------------
;
; Put an non-compressed bitmap on image buffer.
; It does not handle color key transparency.
; It can blt a specific area of the source image to the
; destination buffer instead of the whole source image.
;
; Syntax : IMGbltAreaTrans( imageBuf, pitch, desX, desY, bitmapBuf, srcX1, srcY1, srcX2, srcY2 )
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

		PUBLIC    IMGbltAreaTrans
IMGbltAreaTrans PROC   	  imageBuf, pitch, desX, desY, bitmapPtr, srcX1, srcY1, srcX2, srcY2
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

		MOV	EBX, EAX		 ; EBX - line pixel copy count

		MOV	EDX, pitch	 ; EDX = lineDiff
		SUB	EDX, EAX		 ; lineDiff = image_width - (srcX2-srcX1+1)

		MOV	ECX, srcY2		 ; blt lines = srcY2-srcY1+1
		SUB	ECX, srcY1
		INC	ECX

		CLD                              ; clear direction flag for MOVSB

		;---------- pixels copying loop ----------;

		CALC_ADDR_2 EDI, desX, desY, srcX1, srcY1, pitch     ; Get the address to the destination buffer
@@moreLines:
		PUSH	ECX
		MOV     ECX, EBX

		SHR 	ECX, 2
		JZ	SHORT @@nextScan

;-----------------------------------------------------------------------//
; The idea here is to not branch very often so we unroll the loop by four
; and try to not branch when a whole run of pixels is either transparent
; or not transparent.
;
; There are two loops. One loop is for a run of pixels equal to the
; transparent color, the other is for runs of pixels we need to store.
;
; When we detect a "bad" pixel we jump to the same position in the
; other loop.
;
; Here is the loop we will stay in as long as we encounter a "transparent"
; pixel in the source.
;-----------------------------------------------------------------------//

		align 	4
@@same:
		mov 	al, ds:[esi]
		cmp 	al, TRANSPARENT_CODE
		jne 	short @@diff0

@@same0:
		mov 	al, ds:[esi+1]
		cmp 	al, TRANSPARENT_CODE
		jne 	short @@diff1

@@same1:
		mov 	al, ds:[esi+2]
		cmp 	al, TRANSPARENT_CODE
		jne 	short @@diff2

@@same2:
		mov 	al, ds:[esi+3]
		cmp 	al, TRANSPARENT_CODE
		jne 	short @@diff3

@@same3:
		add 	edi,4
		add 	esi,4
		dec 	ecx
		jnz 	short @@same
		jz  	short @@nextScan

;-----------------------------------------------------------------------//
; Here is the loop we will stay in as long as
; we encounter a "non transparent" pixel in the source.
;-----------------------------------------------------------------------//

		align 	4
@@diff:
		mov 	al, ds:[esi]
		cmp 	al, TRANSPARENT_CODE
		je 	short @@same0

@@diff0:
		mov 	es:[edi],al
		mov 	al, ds:[esi+1]
		cmp 	al, TRANSPARENT_CODE
		je 	short @@same1

@@diff1:
		mov 	es:[edi+1],al
		mov 	al, ds:[esi+2]
		cmp 	al, TRANSPARENT_CODE
		je 	short @@same2

@@diff2:
		mov 	es:[edi+2],al
		mov 	al, ds:[esi+3]
		cmp 	al, TRANSPARENT_CODE
		je 	short @@same3

@@diff3:
		mov 	es:[edi+3],al

		add 	edi,4
		add 	esi,4
		dec 	ecx
		jnz 	short @@diff
		jz  	short @@nextScan

;-----------------------------------------------------------------------//
; We are at the end of a scan, check for odd leftover pixels to do
; and go to the next scan.
;-----------------------------------------------------------------------//

		align 	4
@@nextScan:
		mov 	ecx,ebx           	; ebx = bitmap width
		and 	ecx,11b			; if its pixel count is an odd number
		jnz 	short @@oddStuff

;-----------------------------------------------------------------------//
; move on to the start of the next line
;-----------------------------------------------------------------------//

@@nextScan1:
		add 	edi, edx 		; edx = lineDiff
		add	esi, srcLineDiff

		pop	ecx
		loop	@@moreLines
		jmp 	short @@end

;-----------------------------------------------------------------------//
; If the width is not a multiple of 4 we will come here to clean up
; the last few pixels
;-----------------------------------------------------------------------//

@@oddStuff:
		inc 	ecx
@@oddLoop:
		dec 	ecx
		jz  	short @@nextScan1
		mov 	al, ds:[esi]
		inc 	esi
		inc 	edi
		cmp 	al, TRANSPARENT_CODE
		je  	short @@oddLoop
		mov 	es:[edi-1],al
		jmp 	short @@oddLoop

@@end:          ENDPROC
IMGbltAreaTrans ENDP

;----------- END OF FUNCTION IMGbltAreaTrans ----------


END
