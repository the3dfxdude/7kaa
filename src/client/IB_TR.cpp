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
 *Filename    : IB_TR.ASM
 *Description : Blt a bitmap to the display surface buffer with color key transparency and color remapping handling
 *
 * converted to C++
 */


#include <IMGFUN.h>
#include <COLCODE.h>


//----------- BEGIN OF FUNCTION IMBbltTransRemap ------------
//
// Put an non-compressed bitmap on image buffer.
// It does not handle color key transparency.
// It can blt a specific area of the source image to the
// destination buffer instead of the whole source image.
//
// Syntax : IMBbltTransRemap( imageBuf, pitch, x, y, bitmapBuf, colorTable )
//
// char *imageBuf    - the pointer to the display surface buffer
// int  pitch        - pitch of the display surface buffer
// int  x,y         - where to put the image on the surface buffer
// char *bitmapPtr   - the pointer to the bitmap buffer
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
void _stdcall IMGbltTransRemap(char* imageBuf, int pitch, int x, int y, char* bitmapBuf, char*colorTable)
{
	int destline = y*pitch + x;
	int bitmapWidth = ((unsigned char*)bitmapBuf)[0] + (((unsigned char*)bitmapBuf)[1]<<8);
	int bitmapHeight = ((unsigned char*)bitmapBuf)[2] + (((unsigned char*)bitmapBuf)[3]<<8);
	int esi = 4;	// 4 bytes are header fields (width, height)

	for ( int j=0; j<bitmapHeight; ++j,destline+=pitch,esi+=bitmapWidth )
	{
		for ( int i=0; i<bitmapWidth; ++i )
		{
			if ( ((unsigned char*)bitmapBuf)[esi + i] != TRANSPARENT_CODE)
			{
				imageBuf[destline + i] = colorTable[ bitmapBuf[esi+i] ];
			}
		}
	}
}
//----------- END OF FUNCTION IMGbltTransRemap ----------
