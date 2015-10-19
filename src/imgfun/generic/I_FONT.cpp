/*
 * Seven Kingdoms: Ancient Adversaries
 *
 * Copyright 1997,1998 Enlight Software Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 *Filename    : I_FONT.ASM
 *Description : Image buffer font displaying function
 *
 *converted to c++
 */


#include <IMGFUN.h>

#define BITS_PER_BYTE 8


//-------- BEGIN OF FUNCTION IMGputBitFont ------//
//
// Put font on IMG screen, bit to byte copying
//
// Syntax : IMGputBitFont( x,y,width,fontOffset,fontWidth,fontHeight,bitmapBuf,color)
//
// char *imageBuf    - the pointer to the display surface buffer
// int pitch         - the pitch of the display surface buffer
// int  x,y          - the start location of the font
// int  bitmapWidth  - bitmap buffer line width (in byte)
// int  fontOffset   - font offset from the bitmap buffer
// int  fontWidth    - font width
// int  fontHeight   - font height (in bit)
// char *bitmapBuf   - bitmap buffer
//
// int   foreColor   - foreground color of the font
// [int] backColor   - background color of the font
//		      (default : -1 (transparent background color)
//
// The bitmapBuf is origanized in a bitmap of all fonts packed together.
// To get the font of one character, you get the several from one line,
// and then next line, the distance between two line is represented by width
//
//		     AX - bitmap byte register
//		     BX - current bit position in the byte
//		     CX - counter
// 		     DX - temporary register for color
//
//		     DS:SI - Source address
//		     ES:DI - Destination address
//
//		     widthDiff  - width difference (640 - width)
//
// note (alex streit): the original ASM version was optimized for LODSW, which uses 16bits.
// when converting to C++ I went with the endian independent "unsigned char"
void IMGcall IMGputBitFont(char* imageBuf,int pitch,int x,int y,int bitmapWidth,int fontOffset,int fontWidth,int fontHeight,char* bitmapPtr,int foreColor,int backColor)
{
	// the font is stored using a 1-bit pattern (8 bits to a byte)
	int srcBit = fontOffset % BITS_PER_BYTE;	// was (fontOffset&0x7)
	int destPixel = y * pitch + x;
	unsigned char bitMask;
	unsigned char ax;

	for (int j=0; j<fontHeight; ++j)
	{
		int srcByte = (j*bitmapWidth) + (fontOffset / BITS_PER_BYTE);	// was (fontOffset>>3)
		bitMask = 0x80 >> srcBit;
		ax = bitmapPtr[ srcByte++ ];

		for (int i=0; i<fontWidth; ++i)
		{
			// time to move on to the next byte?
			if (bitMask == 0)
			{
				bitMask = 0x80;
				ax = bitmapPtr[ srcByte++ ];
			}
			// is this bit set?
			if (bitMask & ax)
			{
				// yes - put foreground pixel
				imageBuf[ destPixel++ ] = foreColor;
			}
			else
			{
				// no - put background pixel
				if (backColor != -1)
					imageBuf[ destPixel++ ] = backColor;
			}
		}
	}
}

//----------- END OF FUNCTION IMGputBitFont ----------//
