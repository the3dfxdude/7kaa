;Filename    : I_EREMAP.ASM
;Description : Blt exploration remap of 32x32 to the display surface buffer
; a modified version, 16 mask
; second modification, 8-bit remap table selector instead of 1-bit tone mask

INCLUDE IMGFUN.INC

; ------------ Define constant ------------

		PUBLIC IMGexploreRemap32x32

MASKCOLOUR	= 0

EAST_BIT_MASK	= 1
CENTRE_BIT_MASK = 2
WEST_BIT_MASK   = 4

NORTH1_MASK_OFFS	= 0
SOUTH1_MASK_OFFS	= 0100h
WEST1_MASK_OFFS	= 0200h
EAST1_MASK_OFFS	= 0300h
NW_MASK_OFFS	= 0400h
NE_MASK_OFFS	= 0500h
SW_MASK_OFFS	= 0600h
SE_MASK_OFFS	= 0700h
XNW_MASK_OFFS	= 0800h
XNE_MASK_OFFS	= 0900h
XSW_MASK_OFFS	= 0a00h
XSE_MASK_OFFS	= 0b00h
NORTH2_MASK_OFFS	= 0c00h
SOUTH2_MASK_OFFS	= 0d00h
WEST2_MASK_OFFS	= 0e00h
EAST2_MASK_OFFS	= 0f00h

		.DATA
; bit 0 = north sqaure, bit 1 = north west square, bit 2 = west square
NW_SQR_DECISION DD XSE_MASK_OFFS, WEST1_MASK_OFFS, XSE_MASK_OFFS, WEST1_MASK_OFFS
		DD NORTH1_MASK_OFFS, NW_MASK_OFFS, NORTH1_MASK_OFFS, -1

; bit 0 = east square, bit 1 = north east square, bit 2 = north square
NE_SQR_DECISION DD XSW_MASK_OFFS, NORTH2_MASK_OFFS, XSW_MASK_OFFS, NORTH2_MASK_OFFS
		DD EAST1_MASK_OFFS, NE_MASK_OFFS, EAST1_MASK_OFFS, -1

; bit 0 = south square, bit 1 = south west square, bit 2 = west square
SW_SQR_DECISION DD XNE_MASK_OFFS, WEST2_MASK_OFFS, XNE_MASK_OFFS, WEST2_MASK_OFFS
		DD SOUTH1_MASK_OFFS, SW_MASK_OFFS, SOUTH1_MASK_OFFS, -1

; bit 0 = east square, bit 1 = south east square, bit 2 = south square
SE_SQR_DECISION DD XNW_MASK_OFFS, SOUTH2_MASK_OFFS, XNW_MASK_OFFS, SOUTH2_MASK_OFFS
		DD EAST2_MASK_OFFS, SE_MASK_OFFS, EAST2_MASK_OFFS, -1

buf_pitch	DD ?

		.CODE


;------------ BEGIN OF MACRO REMAPDOT ----------
REMAPDOT		MACRO
		LOCAL	@@remapdot1
		LODSB
		MOVSX	EAX, AL
		MOV	EBX, [EDX + 4*EAX]
		MOV	AL, [EDI]
		XLATB	[EBX]
		STOSB
		ENDM
;------------ END OF MACRO REMAPDOT ----------


;------------ BEGIN OF FUNCTION IMGremap16x16 -----------
IMGremap16x16 PROC
; input :
; EDI = destination
; ESI = bitmapPtr
; EDX = colorRemapArray
;
		PUSH	EAX
		PUSH	EBX
		PUSH	ECX
		PUSH	EDX
		PUSH	ESI
		PUSH	EDI

		CLD
		MOV	ECX, 16
@@line0:
		REMAPDOT
		REMAPDOT
		REMAPDOT
		REMAPDOT

		REMAPDOT
		REMAPDOT
		REMAPDOT
		REMAPDOT

		REMAPDOT
		REMAPDOT
		REMAPDOT
		REMAPDOT

		REMAPDOT
		REMAPDOT
		REMAPDOT
		REMAPDOT

		ADD	EDI, buf_pitch
		SUB	EDI, 16

		DEC	ECX
		JNZ	@@line0

@@line2:	POP	EDI
		POP	ESI
		POP	EDX
		POP	ECX
		POP	EBX
		POP	EAX
		RET
IMGremap16x16	ENDP
;------------ END OF FUNCTION IMGremap16x16 -----------


;---------- BEGIN OF FUNCTION IMGexploreRemap32x32 -------
;
; Smooth the area between explore and unexplored square
;
; Syntax : IMGexploreRemap32x32( imageBuf, pitch, x, y, maskPtr, colorTableArray, northRow, thisRow, southRow)
;
; char *imageBuf - the pointer to the display surface buffer
; int pitch      - the pitch of the display surface buffer
; int  x,y       - where to put the image on the surface buffer
; char *maskPtr  - pointer of masks, (address of 'EXPLMASK.BIN' is loaded)
; northRow       - explored_flag of adjacent location
;                  bit 0 - north east square (0=unexplored, 1=explored)
;                  bit 1 - north square
;                  bit 2 - north west square
; thisRow        - bit 0 - east square
;                  bit 1 - this square
;                  bit 2 - west square
; southRow       - bit 0 - south east square
;                  bit 1 - south square
;                  bit 2 - south west square
;
;--------------------------------------------------------

IMGexploreRemap32x32	PROC imageBuf, pitch, x, y, maskPtr, colorTableArray, northRow, thisRow, southRow
		STARTPROC

		MOV	EAX, imageBuf
		MOV	image_buf, EAX
		MOV	EDX, pitch
		MOV	buf_pitch, EDX
		MOV	EDX, colorTableArray

		; north west 16x16
@@nw0:		XOR	EBX, EBX
		MOV	EAX, northRow
		AND     AL, CENTRE_BIT_MASK OR WEST_BIT_MASK
		SHR	AL, 1
		MOV	BL, AL
		MOV	EAX, thisRow
		AND	AL, WEST_BIT_MASK
		OR	BL, AL
		CMP	BL,7
		JE	@@ne0

		MOV	EAX, [NW_SQR_DECISION + 4*EBX]
		MOV	ESI, maskPtr
		ADD	ESI, EAX
		CALC_ADDR	EDI, x ,y, pitch
		CALL	IMGremap16x16

		; north east 16x16
@@ne0:		XOR	EBX, EBX
		MOV	EAX, northRow
		AND	AL, CENTRE_BIT_MASK OR EAST_BIT_MASK
		SHL	AL, 1
		MOV	BL, AL
		MOV	EAX, thisRow
		AND	AL, EAST_BIT_MASK
		OR	BL, AL
		CMP	BL, 7
		JE	@@sw0

		MOV	EAX, [NE_SQR_DECISION + 4*EBX]
		MOV	ESI, maskPtr
		ADD	ESI, EAX
		CALC_ADDR_2	EDI, x, y, 16, 0, pitch
		CALL	IMGremap16x16

		; south west 16x16
@@sw0:		XOR	EBX, EBX
		MOV	EAX, southRow
		AND     AL, CENTRE_BIT_MASK OR WEST_BIT_MASK
		SHR	AL, 1
		MOV	BL, AL
		MOV	EAX, thisRow
		AND	AL, WEST_BIT_MASK
		OR	BL, AL
		CMP	BL, 7
		JE	@@se0

		MOV	EAX, [SW_SQR_DECISION + 4*EBX]
		MOV	ESI, maskPtr
		ADD	ESI, EAX
		CALC_ADDR_2	EDI, x, y, 0, 16, pitch
		CALL	IMGremap16x16

		; south east 16x16
@@se0:		XOR	EBX, EBX
		MOV	EAX, southRow
		AND	AL, CENTRE_BIT_MASK OR EAST_BIT_MASK
		SHL	AL, 1
		MOV	BL, AL
		MOV	EAX, thisRow
		AND	AL, EAST_BIT_MASK
		OR	BL, AL
		CMP	BL, 7
		JE	@@end

		MOV	EAX, [SE_SQR_DECISION + 4*EBX]
		MOV	ESI, maskPtr
		ADD	ESI, EAX
		CALC_ADDR_2	EDI, x, y, 16, 16, pitch
		CALL	IMGremap16x16

@@end:		ENDPROC
IMGexploreRemap32x32	ENDP
;------------ END OF FUNCTION IMGexploreRemap32x32 -------

		END
