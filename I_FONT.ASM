;Filename    : I_FONT.ASM
;Description : Image buffer font displaying function


INCLUDE IMGFUN.INC

.CODE


;-------- BEGIN OF FUNCTION IMGputBitFont ------
;
; Put font on IMG screen, bit to byte copying
;
; Syntax : IMGputBitFont( x,y,width,fontOffset,fontWidth,fontHeight,bitmapBuf,color)
;
; char *imageBuf    - the pointer to the display surface buffer
; int pitch         - the pitch of the display surface buffer
; int  x,y          - the start location of the font
; int  bitmapWidth  - bitmap buffer line width (in byte)
; int  fontOffset   - font offset from the bitmap buffer
; int  fontWidth    - font width
; int  fontHeight   - font height (in bit)
; char *bitmapBuf   - bitmap buffer
;
; int   foreColor   - foreground color of the font
; [int] backColor   - background color of the font
;		      (default : -1 (transparent background color)
;
; The bitmapBuf is origanized in a bitmap of all fonts packed together.
; To get the font of one character, you get the several from one line,
; and then next line, the distance between two line is represented by width
;
;		     AX - bitmap byte register
;		     BX - current bit position in the byte
;		     CX - counter
; 		     DX - temporary register for color
;
;		     DS:SI - Source address
;		     ES:DI - Destination address
;
;		     widthDiff  - width difference (640 - width)
;
		PUBLIC    IMGputBitFont
IMGputBitFont   PROC      imageBuf, pitch, x, y, bitmapWidth, fontOffset, fontWidth, fontHeight, bitmapPtr, foreColor, backColor
		LOCAL     widthDiff, lastSI, bitMask
		STARTPROC

		MOV	EAX, imageBuf		; store the address of the image buffer to a variable
		MOV	image_buf, EAX

		;---- Initialize destination variables ----

		MOV    AX, DS
		MOV    ES, AX

		MOV    EAX, pitch
		SUB    EAX, fontWidth
		MOV    widthDiff, EAX

		CLD                      ; clear direction flag for MOVSB

		;------- initialize source variables --------

		MOV     ESI, bitmapPtr

		MOV	EAX, fontOffset
		SHR	EAX, 3			 ; Divide by 16, bit --> byte
		ADD	ESI, EAX
		MOV	lastSI, ESI

		MOV	ECX, fontOffset
		AND	ECX, 07h                  ; the bit of the current byte
		MOV	EBX, 8000h
		SHR	EBX, CL
		MOV	bitMask, EBX

		LODSW				 ; Load DS:SI --> AX
		XCHG	AL, AH			 ; Restore the lo-hi to hi-lo

		;------- Draw one line for this loop ------
@@putLine:
		CALC_ADDR  EDI, x, y, pitch           ; Get the offset to the video address

		MOV	ECX, fontWidth            ; Get the offset to the video address

@@putPixel:	TEST 	AX, BX			 ; test one bit only
		JZ 	@@putBack  	         ; if 1->paint, 0->ingore

@@putFore:	MOV	EDX, foreColor		 ; foreColor->dest. byte
		MOV     ES:[EDI], DL
		JMP	@@nextPixel

@@putBack:	MOV     EDX, backColor
		CMP	EDX, 0FFFFFFFFH
		JE	@@nextPixel
		MOV	ES:[EDI], DL

		;-------- Next Pixel ---------------------

@@nextPixel:    SHR	EBX, 1
		JNZ	@@nextDest		; Next destination byte

		LODSW
		XCHG	AL, AH			 ; Restore the lo-hi to hi-lo

		MOV	EBX, 8000h		; New byte

		;-------- Next destination pixel -----------

@@nextDest:	INC     EDI
		LOOP    @@putPixel

@@nextLine:	INC	y               	; decrease height of the bar
		DEC	fontHeight
		JZ      @@end

		;------ check if y is in another bank -----

		MOV	ESI, lastSI
		ADD	ESI, bitmapWidth	; next line
		MOV	lastSI, ESI
		MOV	EBX, bitMask
		LODSW
		XCHG	AL, AH			 ; Restore the lo-hi to hi-lo

		ADD     EDI, widthDiff
		JMP     @@putLine 	; if no. exceed next bank, then loop

@@end:          ENDPROC
IMGputBitFont   ENDP

;----------- END OF FUNCTION IMGputBitFont ----------


END

