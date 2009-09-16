;Filename    : IMGFUN.INC
;Description : Include file for image buffer processing functions


INCLUDE ALL.INC

;---------------------------------------------------

.DATA

; extrn image_buf:DWORD, image_width:DWORD, image_height:DWORD
extrn image_buf:DWORD

;---------------------------------------------------


;------- BEGIN OF MARCO CALC_ADDR -----------
;
; Calculate address of the image buffer of the specified line
;
; <destReg>    the destination register for restoring result offset
; <posX>       the x position
; <posY>       the y position
; <pitch>      pitch of the display surface buffer
;
CALC_ADDR       MACRO   destReg, posX, posY, pitch
		PUSH    EAX
		PUSH    EDX

		MOV	EAX, pitch
		MUL     posY
		ADD	EAX, posX
		ADD	EAX, image_buf

		MOV	destReg, EAX

		POP     EDX
		POP	EAX
		ENDM

;--------- END OF MARCO CALC_ADDR -------------


;------- BEGIN OF MARCO CALC_ADDR_2 -----------
;
; Calculate address of the image buffer of the specified line.
;
; For IMGbltArea() and IMGbltAreaTrans() only.
;
; <destReg>    the destination register for restoring result offset
; <posX>       the x position
; <posY>       the y position
;
CALC_ADDR_2     MACRO   destReg, posX, posY, offsetX, offsetY, pitch
		PUSH    EAX
		PUSH    EDX

		MOV	EAX, posY
		ADD	EAX, offsetY
		MUL pitch

		ADD	EAX, posX
		ADD	EAX, offsetX

		ADD	EAX, image_buf

		MOV	destReg, EAX

		POP     EDX
		POP	EAX
		ENDM

;--------- END OF MARCO CALC_ADDR_2 -------------

