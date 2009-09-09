;Filename    : I_LINE.ASM
;Description : Draw a line in the image buffer


INCLUDE IMGFUN.INC

.DATA
buf_width	dd ?
buf_height	dd ?
buf_pitch	dd ?
.CODE

;-------- BEGIN OF FUNCTION IMGline -----------
;
; Draw a line on the IMG screen
;
; char *imageBuf - the pointer to the display surface buffer
; int  pitch     - the pitch of the display surface buffer
; int  w         - the width of the display surface buffer
; int  h         - the height of the display surface buffer
; int  x1,y1 	 - the top left vertex of the bar
; int  x2,y2 	 - the bottom right vertex of the bar
; int  color 	 - the color of the line
;
		PUBLIC    IMGline
IMGline 	PROC      imageBuf, pitch, w, h, x1, y1, x2, y2, color
		LOCAL     cnt, acc, deltax, deltay, dirx, diry
		STARTPROC

		MOV	EAX, imageBuf		; store the address of the image buffer to a variable
		MOV	image_buf, EAX
		MOV	EAX, pitch
		MOV	buf_pitch, EAX
		MOV	EAX, w
		MOV	buf_width, EAX
		MOV	EAX, h
		MOV	buf_height, EAX


		MOV	EBX, color

		MOV     AX, DS
		MOV     ES, AX

		XOR     ESI, ESI        ; So ESI always = SI and EDI = DI
		XOR     EDI, EDI

		;---------------------------------;

		MOV     ESI, x1
		MOV     EDI, y1
		MOV     acc, 0
		MOV     EAX, x2
		SUB     EAX, ESI
		MOV     EDX, 1
		JNS     @@no_sx
		NEG     EAX
		MOV     EDX, -1

@@no_sx:   	MOV	dirx, EDX
		MOV	deltax, EAX
		mov	EAX, y2
		SUB     EAX, EDI
		MOV     EDX, 1
		JNS     @@no_sy
		NEG	EAX
		MOV	EDX, -1

@@no_sy:   	MOV     diry, EDX
		MOV	deltay, EAX
		CMP 	EAX, deltax
		JGE	@@y_lp
		CALL	line_point
		MOV     EAX, deltax
		MOV     cnt, EAX
@@lp1:
		DEC     cnt
		JS      @@end
		ADD     ESI, dirx
		MOV	EAX, deltay
		ADD     acc, EAX
		MOV     EAX, acc
		CMP     EAX, deltax
		JB      @@no_inc_y
		MOV     EAX, deltax
		SUB     acc, EAX
		ADD     EDI, diry
@@no_inc_y:
		CALL    line_point
		JMP     @@lp1
@@y_lp:
		CALL    line_point
		MOV     EAX, deltay
		MOV     cnt, EAX
@@lp2:
		DEC     cnt
		JS      @@end
		ADD     EDI, diry
		MOV     EAX, deltax
		ADD     acc, EAX
		MOV     EAX, acc
		CMP     EAX, deltay
		JB      @@no_inc_x
		MOV     EAX, deltay
		SUB     acc, EAX
		ADD     ESI, dirx
@@no_inc_x:
		CALL    line_point
		JMP	@@lp2

@@end:  	ENDPROC
IMGline 	ENDP

;------- END OF FUNCTION IMGline ------------



;------ BEGIN OF FUNCTION line_point -----------
;
; It is a private function called by VGAline().
;
; Parameter : ESI - x position
;             EDI - y position
;

line_point      PROC

		CMP	ESI, 0
		JL	@@end

		CMP	ESI, buf_width
		JGE	@@end

		CMP	EDI, 0
		JL	@@end

		CMP	EDI, buf_height
		JGE	@@end

		MOV     EAX, buf_pitch
		MUL     EDI
		ADD     EAX, ESI
		ADD     EAX, image_buf

		MOV     ES:[EAX], BL

@@end:          RET
line_point      ENDP

;---------- END OF FUNCTION line_point ------------

END
