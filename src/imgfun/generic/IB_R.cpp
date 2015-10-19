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
 *Filename    : I_R.ASM
 *Description : Blt a bitmap to the display surface buffer with colour remapping
 *              but without color key transparency handling
 *
 * converted to C++
 */


#include <IMGFUN.h>
#include <COLCODE.h>


//-------- BEGIN OF FUNCTION IMGbltRemap ----------
//
// Put an non-compressed bitmap on image buffer.
// It does not handle color key transparency but colorRemapping
//
// Syntax : IMGblt( imageBuf, pitch, x, y, bitmapBuf )
//
// char *imageBuf - the pointer to the display surface buffer
// int  pitch        - pitch of the display surface buffer
// int  x,y       - where to put the image on the surface buffer
// char *bitmapPtr  - the pointer to the bitmap buffer
// char *colorTable - a 256-entry color remapping table
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


void IMGcall IMGbltRemap(char* imageBuf, int pitch, int x, int y, char* bitmapPtr, char* colorTable)
{
	int destline = y * pitch + x;
	int width = ((unsigned char*)bitmapPtr)[0] + (((unsigned char*)bitmapPtr)[1]<<8);
	int height = ((unsigned char*)bitmapPtr)[2] + (((unsigned char*)bitmapPtr)[3]<<8);
	int srcline = 4;		// skip 4 byte header
	int al;

	for ( int j=0; j<height; ++j, destline+=pitch, srcline+=width )
	{
		for ( int i=0; i<width; ++i )
		{
			al = ((unsigned char*)bitmapPtr)[ srcline + i ];
			imageBuf[ destline + i ] = colorTable[ al ];
		}
	}
}

//----------- END OF FUNCTION IMGbltRemap ----------
