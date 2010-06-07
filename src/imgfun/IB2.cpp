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
 *Filename    : IB2.ASM
 *Description : Blt a bitmap to the display surface buffer without color key transparency handling
 *
 * converted to C++
 */

#include <ALL.h>		// for memcpy
#include <IMGFUN.h>


//----------- BEGIN OF FUNCTION IMGblt2 ------------
//
// Put an non-compressed bitmap on image buffer.
// It does not handle color key transparency.
//
// Syntax : IMGblt2( imageBuf, pitch, x, y, bitmapWidth, bitmapHeight, bitmapBuf )
//
// char *imageBuf - the pointer to the display surface buffer
// int  pitch     - pitch of the display surface buffer
// int  x,y       - where to put the image on the surface buffer
// int  bitmapWidth  - width of the bitmap
// int  bitmapHeight - height of the bitmap
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
void IMGcall IMGblt2(char*imageBuf,int pitch,int x,int y,int bitmapWidth,int bitmapHeight,char*bitmapBuf)
{
	int destline = y*pitch + x;
	int esi = 0;		// [Alex] NOTE: bitmapBuf is actually the RAW bitmap data, ignore the comments above!!

	for ( int j=0; j<bitmapHeight; ++j,destline+=pitch,esi+=bitmapWidth )
	{
		memcpy( &imageBuf[destline], &bitmapBuf[esi], bitmapWidth );
	}
}
//----------- END OF FUNCTION IMGblt2 ----------
