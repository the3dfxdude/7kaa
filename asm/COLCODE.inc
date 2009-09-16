; Filename    : COLCODE.INC
; Description : ASM include file for defining special colour
;               see/update also COLCODE.H for C

; for transparent code repeated for 1 to UNIQUE_REPEAT_CODE_NUM times,
; write FEW_TRANSPARENT_CODE(repeated_times)
; for transparent code repeated for UNIQUE_REPEAT_CODE_NUM+1 to 255 times,
; write two bytes, MANY_TRANSPARENT_CODE and repeated_times

TRANSPARENT_CODE        = 255
UNIQUE_REPEAT_CODE_NUM	=   7		; total no. of bytes used by transparent pixels and compressed transparent pixels is 7+1 (the last 1 is the first byte of the 2 bytes compression code)

ENCODE_FEW_TRANS_CODE MACRO reg         ; reg must be byte
	neg	reg
	ENDM

DECODE_FEW_TRANS_CODE MACRO reg	        ; must be a byte
	neg	reg
	ENDM

MANY_TRANSPARENT_CODE   = 0f8h
MIN_TRANSPARENT_CODE    = 0f8h
MAX_TRASNPARENT_CODE    = 0ffh

; never appear color code 0
SHADOW_CODE             =  000h
OUTLINE_CODE            =  0f2h
OUTLINE_SHADOW_CODE     =  0f3h

; do not remap colour
PRE_REMAP MACRO
	xlatb	[EBX]
	ENDM

POST_REMAP MACRO
	ENDM

JUMP_IF_TRANS MACRO byteCode, goLabel
	cmp	byteCode, MIN_TRANSPARENT_CODE
	jae	goLabel
	ENDM

JUMP_IF_NOT_TRANS MACRO byteCode, goLabel
	cmp	byteCode, MIN_TRANSPARENT_CODE
	jb	goLabel
	ENDM

JUMP_IF_MANY_TRANS MACRO byteCode, goLabel
	cmp	byteCode, MANY_TRANSPARENT_CODE
	je	goLabel
	ENDM

JUMP_IF_NOT_MANY_TRANS MACRO byteCode, goLabel
	cmp	byteCode, MANY_TRANSPARENT_CODE
	jne	goLabel
	ENDM


