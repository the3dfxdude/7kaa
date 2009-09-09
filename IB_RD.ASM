;Filename    : IB_RD.ASM
;Description : decompress and color remapping handling with color key transparency


INCLUDE IMGFUN.INC
INCLUDE COLCODE.INC

.CODE

;-------- BEGIN OF FUNCTION IMGremapDecompress ----------
;
; Decompress bitmap on image buffer with Transparency.
; It handles color key transparency. The color key code is 255.
;
; R - color remapping
; D - decompression
;
; Syntax : IMGremapDecompress(desPtr, srcPtr, colorTable )
;
; char *desPtr   - the pointer to the compressed surface buffer
; char *srcPtr   - the pointer to the decompressed surface buffer
; char *colorTable - a 256-entry color remapping table
;
;-------------------------------------------------
;
; Format of the compressed data
; compressed	decompressed
; FF		FF
; FE (-2)	FF FF
; FD (-3)	FF FF FF
; FC (-4)	FF FF FF FF
; FB (-5)	FF FF FF FF FF
; FA (-6)	FF FF FF FF FF FF
; F9 (-7)	FF FF FF FF FF FF FF
; F8 B		FF ... <B times>
;-------------------------------------------------
;
; Format of the bitmap data :
;
; <short>  width
; <short>  height
; <char..> bitmap image
;
;-------------------------------------------------

		PUBLIC    IMGremapDecompress
IMGremapDecompress PROC    desPtr, srcPtr, colorTable
		STARTPROC
		MOV	AX,DS
		MOV	ES,AX
		MOV	ESI,srcPtr
		MOV	EDI,desPtr
		LODSW			; width
		MOV	CX,AX
		STOSW
		LODSW			; height
		STOSW
		MUL	CX
		; DX:AX stores the decompressed size
		; move it to EDX
		SHL	EDX, 16
		MOV	DX, AX

		; load colour map
		MOV	EBX, colorTable
		XOR	ECX, ECX
@@next:
		LODSB			; get compressed data
		PRE_REMAP
		; test data between F8-FF
		JUMP_IF_TRANS al, @@compressed
		POST_REMAP
		STOSB
		DEC	EDX		; test if completed
		JNZ	@@next
		JMP	@@endloop


@@compressed:
		JUMP_IF_NOT_MANY_TRANS al, @@compressed2
		; F8 B
		LODSB			; load the count
		MOV	CL,AL
		MOV	AL,0FFH
		SUB	EDX,ECX		; test if completed
		;PUSHF
		REP	STOSB
		;POPF
		JA	@@next
		JMP	@@endloop

@@compressed2:
		; F9 - FF
		MOV	CL,AL		; repetition = -al
		MOV	AL,0FFH
		DECODE_FEW_TRANS_CODE cl
		SUB	EDX,ECX		; test if completed
		;PUSHF
		REP	STOSB
		;POPF
		JA	@@next
@@endloop:

@@end:          ENDPROC
IMGremapDecompress ENDP

;----------- END OF FUNCTION IMGbltTransRemap ----------

END
