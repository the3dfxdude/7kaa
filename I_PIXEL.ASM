;Filename    : I_PIXEL.ASM
;Description : PIXELIZE (1:4 tone) of size 32x32 to the display surface buffer

INCLUDE IMGFUN.INC

.CODE


;----------- BEGIN OF FUNCTION IMGpixel32x32 ------------
;
; Syntax : IMGpixel( imageBuf, pitch, x, y, colour )
;
; char *imageBuf - the pointer to the display surface buffer
; int  pitch       - the pitch of the display surface buffer
; int  x,y       - where to put the image on the surface buffer
; int  color     - color to pixelize with.
;
;-------------------------------------------------

PUTFOUR		MACRO
		ADD	EDI,EBX
		MOV	[ESI], AL
		MOV	[EDI], AL
		ADD	ESI,EBX
		ENDM

		PUBLIC    IMGpixel32x32
IMGpixel32x32  	PROC   	  imageBuf, pitch, x, y, color
		STARTPROC

		MOV	EAX, imageBuf		; store the address of the image buffer to a variable
		MOV	image_buf, EAX

		MOV	EDX, pitch		; EDX = lineDiff
		ADD	EDX,EDX
		SUB	EDX,30
		CLD                              ; clear direction flag for MOVSB

		;------- pixels loop --------;

		CALC_ADDR EDI, x, y, pitch	; Get the offset to the image buffer address
		MOV	ESI,EDI
		MOV	EAX,color
		MOV	EBX,4
		MOV	ECX,16
@@loopstart:
		ADD	EDI,2
		MOV	[ESI], AL
		MOV	[EDI], AL
		ADD	ESI,EBX
		PUTFOUR
		PUTFOUR
		PUTFOUR
		PUTFOUR
		PUTFOUR
		PUTFOUR
		ADD	EDI,EBX
		MOV	[ESI], AL
		MOV	[EDI], AL
		ADD	ESI,2

		;------ skip two line ------;
		ADD	EDI,EDX
		ADD	ESI,EDX
		LOOP	@@loopstart

@@end:          ENDPROC
IMGpixel32x32  	ENDP

;----------- END OF FUNCTION IMGpixel32x32 ----------


END
