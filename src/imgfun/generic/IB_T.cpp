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
 *Filename    : I_BLTT.ASM
 *Description : Blt a bitmap to the display surface buffer with color key transparency handling
 *              surface on the same place with transparency handling
 *
 * converted to C++
 */


#include <IMGFUN.h>
#include <COLCODE.h>


//-------- BEGIN OF FUNCTION IMGbltTrans ----------
//
// Put an non-compressed bitmap on image buffer.
// It handles color key transparency. The color key code is 255.
//
// Syntax : IMGbltTrans( imageBuf, pitch, x, y, bitmapBuf )
//
// char *imageBuf - the pointer to the display surface buffer
// int  pitch        - pitch of the display surface buffer
// int  x,y       - where to put the image on the surface buffer
// char *bitmapPtr  - the pointer to the bitmap buffer
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


void IMGcall IMGbltTrans(char* imageBuf, int pitch, int x, int y, char* bitmapPtr)
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
			if (al != TRANSPARENT_CODE)
			{
				imageBuf[ destline + i ] = al;
			}
		}
	}
}

//----------- END OF FUNCTION IMGbltTrans ----------
