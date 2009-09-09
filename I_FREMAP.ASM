;Filename    : I_FREMAP.ASM
;Description : Blt fog remap of 32x32 to the display surface buffer

INCLUDE IMGFUN.INC

; ------------ Define constant ------------

		PUBLIC IMGfogRemap32x32

; B value is the visibility of a location
; 0 = fully invisible, 89 = fully visible

; there is 10 scale of darkening remapping, 1 identity remapping (changeless remapping)
; and 10 scale of brightening remapping (but not used here)

; suppose the B value of four corners of a 16x16 block is a, b, d and c
; using linear model, to interpolate the interior pixel :
; B(x,y) = [(16-x)(16-y)a + (x)(16-y)b + (16-x)(y)d + xyc ] / 256
;        = [ (16-x)(16a + y(d-a)) + (x)(16b + y(c-b)) ] / 256
; let C1(y) = 16a + y(d-a)
; let C2(y) = 16b + y(c-b)
; B(x,y) = [ (16-x) C1(y) + x C2(y) ] / 256
;
; B(x+1,y) - B(x,y) = [ C2(y) - C1(y) ] / 256
;
; so B(0,0) = a
;    B(x+1,y) = B(x,y) + [ C2(y) - C1(y) ] / 256
;    B(0,y) = [ 16 C1(y) ] / 256
;
;    C1(0) = 16a
;    C1(y+1) = C1(y) + (d-a)
;    C2(0) = 16b
;    C2(y+1) = C2(y) + (c-b)

		.data

cornera		dw	?
cornerb		dw	?
cornerc		dw	?
cornerd		dw	?

; bxy		dw	?	; B(x,y) * 256, now stored in DX
c1		dw	?	; C1(y)
c2		dw	?	; C2(y)
c2subC1		dw	?	; [C2(y) - C1(y)]

buf_pitch	dd	?

		.code

;------------ BEGIN OF FUNCTION IMGfogRemap16x16 -----------
;
; draw a fogged 16x16 square by remapping
;
; input :
; EDI = destination
; ESI = colorRemapArray
; cornera = north west corner of 16x16
; cornerb = north east corner of 16x16
; cornerc = south east corner of 16x16
; cornerd = south west corner of 16x16
;
;----------------------------------------------------------

IMGfogRemap16x16 PROC
		PUSH	EAX
		PUSH	EBX
		PUSH	ECX
		PUSH	EDX
		PUSH	ESI
		PUSH	EDI

		CLD
		MOV	ECX, 16

		; C1(0) = 16a
		; c1 = 16a
		MOV	AX, cornera
		SHL	AX,4
		MOV	c1, AX

		; C2(0) = 16b
		; c2 = 16b
		MOV	AX, cornerb
		SHL	AX, 4
		MOV	c2, AX

@@line0:
		PUSH	ECX

		; c2subc1 = c2 - c1
		MOV	AX, c2
		SUB	AX, c1
		MOV	c2subc1, AX

		; B(0,y) = 16 C1(y) / 256
		; bxy = 16 * c1
		MOV	AX, c1
		SHL	AX, 4
		MOV	DX, AX

		MOV	ECX, 4
@@line1:
		MOV	EAX,[EDI]

		MOVZX	EBX, DX
		SHR	EBX, 11		; if cornerx between 0-87, EBX now between 0-10
		; get the remap table [ESI + 4*(EBX-10)]
		MOV	EBX, [ESI + 4*EBX - 40]
		XLATB	[EBX]
		; B(x+1,y) = B(x,y) + [ C2(y) - C1(y) ] /256
		; bxy = bxy + c2subc1
		ADD	DX, c2subc1
		ROR	EAX, 8

		MOVZX	EBX, DX
		SHR	EBX, 11
		MOV	EBX, [ESI + 4*EBX - 40]
		XLATB	[EBX]
		ADD	DX, c2subc1
		ROR	EAX, 8

		MOVZX	EBX, DX
		SHR	EBX, 11
		MOV	EBX, [ESI + 4*EBX - 40]
		XLATB	[EBX]
		ADD	DX, c2subc1
		ROR	EAX, 8

		MOVZX	EBX, DX
		SHR	EBX, 11
		MOV	EBX, [ESI + 4*EBX - 40]
		XLATB	[EBX]
		ADD	DX, c2subc1
		ROR	EAX, 8

		STOSD

		LOOP	@@line1

		; end of line
		; C1(y+1) = C1(y) + (d-a)
		MOV	AX, cornerd
		SUB	AX, cornera
		ADD	c1, AX

		; C2(y+1) = C2(y) + (c-b)
		MOV	AX, cornerc
		SUB	AX, cornerb
		ADD	c2, AX

		POP	ECX

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
IMGfogRemap16x16	ENDP
;------------ END OF FUNCTION IMGfogRemap16x16 -----------


;------------ BEGIN OF FUNCTION IMGbar16x16 -----------
;
; draw a black 16x16 square
;
; input :
; EDI = destination
;
;----------------------------------------------------------

IMGbar16x16 PROC
		PUSH	EAX
		PUSH	ECX
		PUSH	EDI

		CLD
		MOV	ECX, 16
		XOR	EAX, EAX	; color 0
@@line0:
		STOSD
		STOSD
		STOSD
		STOSD

		ADD	EDI, buf_pitch
		SUB	EDI, 16

		LOOP	@@line0

@@line2:	POP	EDI
		POP	ECX
		POP	EAX
		RET
IMGbar16x16	ENDP
;------------ END OF FUNCTION IMGbar16x16 -----------


;------------ BEGIN OF FUNCTION IMGbarRemap16x16 -----------
;
; draw a black 16x16 square
;
; input :
; EDI = destination
; EBX = colorTable
;
;----------------------------------------------------------

IMGbarRemap16x16 PROC
		PUSH	EAX
		PUSH	ECX
		PUSH	EDI

		CLD
		MOV	ECX, 16
@@line0:
		PUSH	ECX
		MOV	ECX,4
@@line1:
		MOV	EAX,[EDI]
		XLATB	[EBX]
		ROR	EAX,8
		XLATB	[EBX]
		ROR	EAX,8
		XLATB	[EBX]
		ROR	EAX,8
		XLATB	[EBX]
		ROR	EAX,8
		STOSD
		LOOP	@@line1

		POP	ECX
		ADD	EDI, buf_pitch
		SUB	EDI, 16
		LOOP	@@line0

@@line2:	POP	EDI
		POP	ECX
		POP	EAX
		RET
IMGbarRemap16x16	ENDP
;------------ END OF FUNCTION IMGbarRemap16x16 -----------


;---------- BEGIN OF FUNCTION decideBarRemap -----------
;
; decide whether to call IMGfogRemap16x16, IMGbar16x16 or skip
;
; input cornera, cornerb, cornerc and cornerd
; ESI = colorRemapArray
; EDI = destination
;
;-------------------------------------------------------

decideBarRemap	PROC
		PUSH	EAX
		PUSH	EBX
		PUSH	ECX
		PUSH	EDX

		XOR	ECX,ECX

		MOV	AX, cornera
		SHR	AX, 3		; EAX=10 if visible, EAX=0 if invisible
		MOV	DL, AL

		MOV	AX, cornerb
		SHR	AX, 3
		CMP	DL, AL		; to test grade of all corners are equal
		SETNZ	BL
		ADD	CL, BL
		SHL	EDX, 8
		MOV	DL, AL

		MOV	AX, cornerc
		SHR	AX, 3
		CMP	DL, AL		; to test grade of all corners are equal
		SETNZ	BL
		ADD	CL, BL
		SHL	EDX, 8
		MOV	DL, AL

		MOV	AX, cornerd
		SHR	AX, 3
		CMP	DL, AL		; to test grade of all corners are equal
		SETNZ	BL
		ADD	CL, BL
		SHL	EDX, 8
		MOV	DL, AL

		JECXZ	@@equalCorners		; grade of corners are different, call IMGfogRemap16x16
@@fogRemap:
		CALL	IMGfogRemap16x16
		JMP	@@skipped

@@equalCorners:
		CMP	DL, 0ah			; four corners are fully visible
		JE	@@skipped

		CMP	DL, 0			; four corners are invisible
		JNE	@@barRemap
		CALL	IMGbar16x16
		JMP	@@skipped
@@barRemap:
		MOVZX	EAX, DL
		SUB	EAX, 10
		MOV	EBX, [ESI + 4*EAX]
		CALL	IMGbarRemap16x16
		JMP	@@skipped
@@skipped:
		POP	EDX
		POP	ECX
		POP	EBX
		POP	EAX
		RET
decideBarRemap	ENDP

;------------ END OF FUNCTION decideBarRemap -----------


;---------- BEGIN OF FUNCTION IMGfogRemap32x32 -------
;
; Smooth the area in the fog square
;
; Syntax : IMGfogRemap32x32( imageBuf, pitch, x, y, colorTableArray, northRow, thisRow, southRow)
;
; char *imageBuf - the pointer to the display surface buffer
; int  pitch     - the pitch of the display surface buffer
; int  x,y       - where to put the image on the surface buffer
; char **colorTableArray - array of start of each remapping table
; unsigned char *northRow - B value of adjacent location
;                  byte 0 - north east square
;                  byte 1 - north square
;                  byte 2 - north west square
; thisRow        - byte 0 - east square
;                  byte 1 - this square
;                  byte 2 - west square
; southRow       - byte 0 - south east square
;                  byte 1 - south square
;                  byte 2 - south west square
;
;--------------------------------------------------------

IMGfogRemap32x32	PROC imageBuf, pitch, x, y, colorTableArray, northRow, thisRow, southRow
		STARTPROC

		MOV	EAX, imageBuf
		MOV	image_buf, EAX
		MOV	EAX, pitch
		MOV	buf_pitch, EAX
		MOV	ESI, colorTableArray

		; north west 16x16
@@nw0:
		MOV	EBX, northRow
		XOR	AX, AX

		MOV	AL, [EBX+1]		; north square
		MOV	cornerb, AX		; north east corner

		MOV	AL, [EBX+2]		; north west square
		MOV	cornera, AX		; north west corner

		MOV	EBX, thisRow
		MOV	AL, [EBX+1]		; this square
		MOV	cornerc, AX		; south east corner

		MOV	AL, [EBX+2]		; west square
		MOV	cornerd, AX		; south west corner

		CALC_ADDR	EDI, x ,y, pitch
		CALL	decideBarRemap

		; north east 16x16
@@ne0:
		MOV	EBX, northRow
		XOR	AX, AX

		MOV	AL, [EBX]		; north east square
		MOV	cornerb, AX		; north east corner

		MOV	AL, [EBX+1]		; north square
		MOV	cornera, AX		; north west corner

		MOV	EBX, thisRow
		MOV	AL, [EBX]		; east square
		MOV	cornerc, AX		; south east corner

		MOV	AL, [EBX+1]		; this square
		MOV	cornerd, AX		; south west corner

		CALC_ADDR_2	EDI, x, y, 16, 0, pitch
		CALL	decideBarRemap

		; south west 16x16
@@sw0:
		MOV	EBX, thisRow
		XOR	AX, AX

		MOV	AL, [EBX+1]		; this square
		MOV	cornerb, AX		; north east corner

		MOV	AL, [EBX+2]		; west square
		MOV	cornera, AX		; north west corner

		MOV	EBX, southRow
		MOV	AL, [EBX+1]		; south square
		MOV	cornerc, AX		; south east corner

		MOV	AL, [EBX+2]		; south west square
		MOV	cornerd, AX		; south west corner

		CALC_ADDR_2	EDI, x, y, 0, 16, pitch
		CALL	decideBarRemap

		; south east 16x16
@@se0:
		MOV	EBX, thisRow
		XOR	AX, AX

		MOV	AL, [EBX]		; east square
		MOV	cornerb, AX		; north east corner

		MOV	AL, [EBX+1]		; this square
		MOV	cornera, AX		; north west corner

		MOV	EBX, southRow
		MOV	AL, [EBX]		; south east square
		MOV	cornerc, AX		; south east corner

		MOV	AL, [EBX+1]		; south square
		MOV	cornerd, AX		; south west corner

		CALC_ADDR_2	EDI, x, y, 16, 16, pitch
		CALL	decideBarRemap

@@end:		ENDPROC
IMGfogRemap32x32	ENDP
;------------ END OF FUNCTION IMGfogRemap32x32 -------

		END
