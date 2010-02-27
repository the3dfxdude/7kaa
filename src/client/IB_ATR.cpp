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
 *Filename    : I_ATR.ASM
 *Description : Blt a bitmap to the display surface buffer with color key transparency handling
 *              and colour remapping
 *
 * converted to C++
 */


#include <IMGFUN.h>
#include <COLCODE.h>


//----------- BEGIN OF FUNCTION IMGbltAreaTransRemap ------------
//
// Put an non-compressed bitmap on image buffer.
// It does not handle color key transparency.
// It can blt a specific area of the source image to the
// destination buffer instead of the whole source image.
//
// Syntax : IMGbltAreaTransRemap( imageBuf, pitch, desX, desY, bitmapBuf, srcX1, srcY1, srcX2, srcY2 )
//
// char *imageBuf    - the pointer to the display surface buffer
// int  pitch        - pitch of the display surface buffer
// int  desX, desY   - where to put the area on the surface buffer
// char *bitmapPtr   - the pointer to the bitmap buffer
// int  srcX1, srcY1 - where to get the area on the source buffer
//      srcX2, srcY2
// char *colorTable  - a 256-entry color remapping table
//
//-------------------------------------------------
//
// Format of the bitmap data :
//
// <short>  width
// <short>  height
// <char..> bitmap image
//
//-------------------------------------------------
// Note: this function is used by the mouse cursor
void _stdcall IMGbltAreaTransRemap(char* imageBuf, int pitch, int desX, int desY, char* bitmapBuf, int srcX1, int srcY1, int srcX2, int srcY2, char*colorTable)
{
	int destline = (desY+srcY1)*pitch + (desX+srcX1);
	int bitmapWidth = ((unsigned char*)bitmapBuf)[0] + (((unsigned char*)bitmapBuf)[1]<<8);
	//int bitmapHeight = ((unsigned char*)bitmapBuf)[2] + (((unsigned char*)bitmapBuf)[3]<<8);
	int width = srcX2 - srcX1 + 1;
	int height = srcY2 - srcY1 + 1;
	int esi = 4 + srcY1 * bitmapWidth + srcX1;	// 4 bytes are header fields (width, height)

	for ( int j=0; j<height; ++j,destline+=pitch,esi+=bitmapWidth )
	{
		for ( int i=0; i<width; ++i )
		{
			if ( ((unsigned char*)bitmapBuf)[esi + i] != TRANSPARENT_CODE)
			{
				imageBuf[destline + i] = colorTable[ bitmapBuf[esi+i] ];
			}
		}
	}
}
//----------- END OF FUNCTION IMGbltAreaTransRemap ----------
