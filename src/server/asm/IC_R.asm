; Seven Kingdoms: Ancient Adversaries
;
; Copyright 1997,1998 Enlight Software Ltd.
;
; This program is free software: you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation, either version 2 of the License, or
; (at your option) any later version.
;
; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License
; along with this program.  If not, see <http://www.gnu.org/licenses/>.
;

;Filename    : IC_R.ASM
;Description : Remap a display surface to another display
;              surface on the same place


INCLUDE IMGFUN.inc

.CODE

;--------- BEGIN OF FUNCTION IMGcopyRemap -----------
;
; Remap a rectangle of a display surface, but the result is
; put into another display surface
;
; Note : No border checking is made in this function.
;	 Placing an icon outside image buffer will cause serious BUG.
;
; char *imageBuf   - the pointer to the display surface buffer
; int imgPitch     - pitch of the destination display surface buffer
; char *backBuf    - the pointer to the back buffer
; int backPitch    - pitch of the back buffer
; int  x1,y1       - the top left vertex of the bar
; int  x2,y2       - the bottom right vertex of the bar
; char *colorTable - the pointer to the remap table
;
		PUBLIC IMGcopyRemap
IMGcopyRemap	PROC   imageBuf, imgPitch, backBuf, backPitch, x1, y1, x2, y2, colorTable
		LOCAL  barWidth:DWORD
		STARTPROC

		MOV	EAX, backBuf
		MOV	image_buf, EAX
		CALC_ADDR ESI, x1, y1, backPitch	; Get the offset to the back buffer address

		MOV	EAX, imageBuf		; store the address of the image buffer to a variable
		MOV	image_buf, EAX
		CALC_ADDR EDI, x1, y1, imgPitch         ; Get the offset to the image buffer address

		;------ calc bar width and height -----;

		MOV	AX , DS
		MOV	ES , AX

		MOV	EBX, x2
		SUB	EBX, x1
		INC	EBX
		MOV	barWidth, EBX

		MOV	ECX, y2
		SUB	ECX, y1
		INC	ECX

		SUB	imgPitch, EBX		 ; lineDiff = image_width - barWidth
		SUB	backPitch, EBX		 ; lineDiff = image_width - barWidth

		MOV	EBX, colorTable

		CLD                              ; clear direction flag for MOVSB

		;------- pixels copying loop --------;
		TEST	barWidth,3
		JZ	@@quadWidth		; barWidth is multiple of 4 use dword copy
		TEST    barWidth,1
		JZ	@@evenWidth		; barWidth is even, use word copy

		; -------- odd width ----------;
@@oddWidth:	SHR	barWidth,1
@@startY:	PUSH	ECX
		MOV	ECX, barWidth
		JECXZ	@@startX1
@@startX:
		LODSW
		XLATB	[EBX]
		XCHG	AL,AH
		XLATB	[EBX]
		XCHG	AL,AH
		STOSW
		LOOP	@@startX
@@startX1:	LODSB
		XLATB	[EBX]
		STOSB

		POP	ECX
		ADD	ESI, backPitch
		ADD	EDI, imgPitch
		LOOP	@@startY
		JMP	@@end

		; ------ even width -------;
@@evenWidth:	SHR	barWidth,1

@@evenStartY:	PUSH	ECX
		MOV	ECX, barWidth
@@evenStartX:
		LODSW
		XLATB	[EBX]
		XCHG	AL,AH
		XLATB	[EBX]
		XCHG	AL,AH
		STOSW
		LOOP	@@evenStartX

		POP	ECX
		ADD	ESI, backPitch
		ADD	EDI, imgPitch
		LOOP	@@evenStartY
		JMP	@@end

		; -------- quad width --------;
@@quadWidth:	SHR	barWidth,2
@@quadStartY:	PUSH	ECX
		MOV	ECX, barWidth
@@quadStartX:
		LODSD
		XLATB	[EBX]
		ROR	EAX,8
		XLATB	[EBX]
		ROR	EAX,8
		XLATB	[EBX]
		ROR	EAX,8
		XLATB	[EBX]
		ROR	EAX,8
		STOSD
		LOOP	@@quadStartX

		POP	ECX
		ADD	ESI, backPitch
		ADD	EDI, imgPitch
		LOOP	@@quadStartY

@@end:		ENDPROC

IMGcopyRemap	ENDP


;---------- END OF FUNCTION IMGcopyRemap ------------

END
