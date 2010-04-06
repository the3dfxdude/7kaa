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
 *Filename    : IB_32.ASM
 *Description : Blt a bitmap of 32x32 to the display surface buffer without color key transparency handling
 *
 * converted to C++
 */

#include <ALL.h>		// for memcpy
#include <IMGFUN.h>


//----------- BEGIN OF FUNCTION IMGblt32x32 ------------
//
// Put an non-compressed 32x32 bitmap on image buffer.
// It does not handle color key transparency.
//
// Syntax : IMGblt( imageBuf, pitch, x, y, bitmapBuf )
//
// char *imageBuf - the pointer to the display surface buffer
// int  pitch     - pitch of the display surface buffer
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
void _stdcall IMGblt32x32(char*imageBuf,int pitch,int x,int y,char*bitmapBuf)
{
	int destline = y*pitch + x;
	int bitmapWidth = ((unsigned char*)bitmapBuf)[0] + (((unsigned char*)bitmapBuf)[1]<<8);
	int esi = 4;		// 4 bytes of bitmap header (16bit width, 16bit height)

	for ( int j=0; j<32; ++j,destline+=pitch,esi+=bitmapWidth )
	{
		memcpy( &imageBuf[destline], &bitmapBuf[esi], 32 );
	}
}
//----------- END OF FUNCTION IMGblt32x32 ----------
