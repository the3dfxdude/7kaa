;Filename    : I_CTRL.ASM
;Description : VGA image buffer control routines


INCLUDE ALL.INC

.DATA

;--------------------------------------------------

public image_buf, image_width, image_height

image_buf       DD      ?
image_width     DD      ?
image_height    DD      ?

;--------------------------------------------------


.CODE

;------- BEGIN OF FUNCTION IMGinit ------------
;
; Initialize the settings of the image buffer
;
; Syntax : IMGinit(char*, int, int)
;
; int  imageWidth  - width and height of the image buffer
; int  imageHeight
;
		PUBLIC  IMGinit
IMGinit         PROC    imageWidth, imageHeight
		STARTPROC

		MOV	EAX, imageWidth
		MOV	image_width, EAX

		MOV	EAX, imageHeight
		MOV	image_height, EAX

		ENDPROC
IMGinit         ENDP

;---------- END OF FUNCTION IMGinit ---------


;------- BEGIN OF FUNCTION IMGgetState ------------
;
; Initialize the settings of the image buffer
;
; Syntax : IMGgetState(VgaFunState *)
;
; VgaFunState     - pointer to hold the image_width and image_height
;
		PUBLIC  IMGgetState
IMGgetState	PROC    vgaState
		STARTPROC

		MOV	EDI, vgaState
		MOV	EAX, image_width
		STOSD
		MOV	EAX, image_height
		STOSD

		ENDPROC
IMGgetState         ENDP

;---------- END OF FUNCTION IMGgetState ---------

END
