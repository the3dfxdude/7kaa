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
 *Filename    : I_READ.ASM
 *Description : Read a bitmap from the display surface buffer
 *
 * converted to c++
 */

#include <ALL.h>	// for memcpy
#include <IMGFUN.h>


//----------- BEGIN OF FUNCTION IMGread ------------//
//
// Put an non-compressed bitmap on image buffer.
// It does not handle color key transparency.
//
// Syntax : IMGread( imageBuf, pitch, x1, y1, x2, y2, bitmapBuf )
//
// char *imageBuf      - the pointer to the display surface buffer
// int  pitch          - pitch of the surface buffer
// int  x1, y1, x2, y2 - the read of the surface buffer to read
// char *bitmapPtr     - the pointer to the bitmap buffer
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

void _stdcall IMGread(char* imageBuf,int pitch,int x1,int y1,int x2,int y2,char* bitmapPtr)
{
	int width = x2-x1 + 1;
	int height = y2-y1 + 1;
	int offset;
	int src_offset;

	// Alex: note: this is a binary compatible replacement for the asm
	// TODO: convert bitmapPtr from char to a struct
	bitmapPtr[0] = width & 0xff;
	bitmapPtr[1] = width>>8;
	bitmapPtr[2] = height & 0xff;
	bitmapPtr[3] = height>>8;
	offset = 4;
	src_offset = y1 * pitch + x1;
	for (int y=0; y<height; ++y)
	{
		memcpy( &bitmapPtr[offset], &imageBuf[src_offset], width );
		offset += width;
		src_offset += pitch;
	}
}

//----------- END OF FUNCTION IMGread ----------//

