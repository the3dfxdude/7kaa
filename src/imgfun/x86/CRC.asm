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

;Filename    : CRC.ASM
;Description : calculate 8-bit CRC

INCLUDE ALL.inc

		.CODE

DIVISOR		= 101001011b
;----------- BEGIN OF FUNCTION crc8 ------------
;
; calculate the remainder of cyclic division
;
; Syntax : CRC8( dataBuf, dataLen )
;
; unsigned char *dataBuf - data buffer
; int dataLen            - length of data
;

		PUBLIC	crc8
crc8		PROC	dataBuf, dataLen
		PUSH	ECX
		PUSH	ESI

		XOR	EAX, EAX
		MOV	ESI, dataBuf
		MOV	ECX, dataLen
		OR	ECX, ECX
		JZ	@@end
		LODSB			; load the first byte
		DEC	ECX
		JZ	@@lastByte7
@@nonLastByte7:
		MOV	AH,AL
		LODSB

		TEST	AH, 80h
		JZ	@@nonLastByte6
		XOR	AX, DIVISOR SHL 7
@@nonLastByte6:
		TEST	AH, 40h
		JZ	@@nonLastByte5
		XOR	AX, DIVISOR SHL 6
@@nonLastByte5:
		TEST	AH, 20h
		JZ	@@nonLastByte4
		XOR	AX, DIVISOR SHL 5
@@nonLastByte4:
		TEST	AH, 10h
		JZ	@@nonLastByte3
		XOR	AX, DIVISOR SHL 4
@@nonLastByte3:
		TEST	AH, 8h
		JZ	@@nonLastByte2
		XOR	AX, DIVISOR SHL 3
@@nonLastByte2:
		TEST	AH, 4h
		JZ	@@nonLastByte1
		XOR	AX, DIVISOR SHL 2
@@nonLastByte1:
		TEST	AH, 2h
		JZ	@@nonLastByte0
		XOR	AX, DIVISOR SHL 1
@@nonLastByte0:
		TEST	AH, 1h
		JZ	@@nonLastByteX
		XOR	AX, DIVISOR
@@nonLastByteX:
		LOOP	@@nonLastByte7

@@lastByte7:
		SHL	AX, 8
		TEST	AH, 80h
		JZ	@@lastByte6
		XOR	AX, DIVISOR SHL 7
@@lastByte6:
		TEST	AH, 40h
		JZ	@@lastByte5
		XOR	AX, DIVISOR SHL 6
@@lastByte5:
		TEST	AH, 20h
		JZ	@@lastByte4
		XOR	AX, DIVISOR SHL 5
@@lastByte4:
		TEST	AH, 10h
		JZ	@@lastByte3
		XOR	AX, DIVISOR SHL 4
@@lastByte3:
		TEST	AH, 8h
		JZ	@@lastByte2
		XOR	AX, DIVISOR SHL 3
@@lastByte2:
		TEST	AH, 4h
		JZ	@@lastByte1
		XOR	AX, DIVISOR SHL 2
@@lastByte1:
		TEST	AH, 2h
		JZ	@@lastByte0
		XOR	AX, DIVISOR SHL 1
@@lastByte0:
		TEST	AH, 1h
		JZ	@@end
		XOR	AX, DIVISOR
@@end:
		AND	EAX, 0ffh	; only least 8-bit is significant
		POP	ESI
		POP	ECX
		RET
crc8	   	ENDP

;----------- END OF FUNCTION crc8 ----------


END
