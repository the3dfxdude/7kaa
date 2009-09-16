;Filename    : ALL.INC
;Description : General purpose include file

.386
.MODEL FLAT, STDCALL


;--------- BEGIN OF MARCO STARTPROC ----------
;
; Codes placed at the beginning of a function
;
; Syntax : STARTPROC
;
STARTPROC       MACRO
		PUSH    EBX
		PUSH    ESI
		PUSH    EDI
		ENDM

;----------- END OF MARCO STARTPROC ---------


;-------- BEGIN OF MARCO ENDPROC -----------
;
; Codes placed at the ending of a function
;
ENDPROC         MACRO
		POP  EDI
		POP  ESI
		POP  EBX
		RET
		ENDM

;----------- END OF MARCO ENDPROC ------------

