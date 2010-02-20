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
 *Filename    : IB_A.ASM
 *Description : Blt a bitmap to the display surface buffer without color key transparency handling
 *
 * converted to c++
 */


#include <ALL.h>		// for memcpy
#include <IMGFUN.h>



//----------- BEGIN OF FUNCTION IMGbltArea ------------
//
// Put an non-compressed bitmap on image buffer.
// It does not handle color key transparency.
// It can blt a specific area of the source image to the
// destination buffer instead of the whole source image.
//
// Syntax : IMGbltArea( imageBuf, pitch, desX, desY, bitmapBuf, srcX1, srcY1, srcX2, srcY2 )
//
// char *imageBuf    - the pointer to the display surface buffer
// int  pitch        - pitch of the display surface buffer
// int  desX, desY   - where to put the area on the surface buffer
// char *bitmapPtr   - the pointer to the bitmap buffer
// int  srcX1, srcY1 - where to get the area on the source buffer
//      srcX2, srcY2
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

// [alex] TODO: use a struct for bitmaps
void _stdcall IMGbltArea(char* imageBuf,int pitch, int desX, int desY, char* bitmapBuf, int srcX1, int srcY1, int srcX2, int srcY2)
{
	int dest = (desY+srcY1) * pitch + (desX+srcX1);
	int bitmapWidth = ((unsigned char*)bitmapBuf)[0] + (((unsigned char*)bitmapBuf)[1]<<8);
	int src = 4 + srcY1 * bitmapWidth + srcX1;	// 4 bytes are header fields (width, height)
	int width = srcX2-srcX1+1;
	int height = srcY2-srcY1+1;

	for (int j=0; j<height; ++j, dest+=pitch, src+=bitmapWidth)
	{
		memcpy( &imageBuf[ dest ], &bitmapBuf[ src ], width );
	}
}

//----------- END OF FUNCTION IMGbltArea ----------//
