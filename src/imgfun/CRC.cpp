/*
 Seven Kingdoms: Ancient Adversaries

 Copyright 1997,1998 Enlight Software Ltd.

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.


Filename    : CRC.ASM
Description : calculate 8-bit CRC

Converted to C++
*/

#include <CRC.h>

#define DIVISOR	0x14B //101001011b

//----------- BEGIN OF FUNCTION crc8 ------------//
//
// calculate the remainder of cyclic division
//
// Syntax : CRC8( dataBuf, dataLen )
//
// unsigned char *dataBuf - data buffer
// int dataLen            - length of data
//

CRC_TYPE IMGcall crc8(unsigned char * dataBuf, int dataLen)
{
	int ax = 0;
	if (dataLen > 0)
	{
		// non last byte
		for (ax = *dataBuf++; dataLen > 1; --dataLen)
		{
			ax <<= 8;
			ax |= *dataBuf++;
			if ( ax & 0x8000 )
				ax ^= DIVISOR << 7;
			if ( ax & 0x4000 )
				ax ^= DIVISOR << 6;
			if ( ax & 0x2000 )
				ax ^= DIVISOR << 5;
			if ( ax & 0x1000 )
				ax ^= DIVISOR << 4;
			if ( ax & 0x0800 )
				ax ^= DIVISOR << 3;
			if ( ax & 0x0400 )
				ax ^= DIVISOR << 2;
			if ( ax & 0x0200 )
				ax ^= DIVISOR << 1;
			if ( ax & 0x0100 )
				ax ^= DIVISOR;
		}
		// last byte
		ax <<= 8;
		if ( ax & 0x8000 )
			ax ^= DIVISOR << 7;
		if ( ax & 0x4000 )
			ax ^= DIVISOR << 6;
		if ( ax & 0x2000 )
			ax ^= DIVISOR << 5;
		if ( ax & 0x1000 )
			ax ^= DIVISOR << 4;
		if ( ax & 0x0800 )
			ax ^= DIVISOR << 3;
		if ( ax & 0x0400 )
			ax ^= DIVISOR << 2;
		if ( ax & 0x0200 )
			ax ^= DIVISOR << 1;
		if ( ax & 0x0100 )
			ax ^= DIVISOR;
	}
	return ax;
}

//----------- END OF FUNCTION crc8 ----------//

