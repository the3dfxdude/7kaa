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
 *Filename    : IR_AM.ASM
 *Description : Remap a bitmap on vga image buffer with clipping, with mirroring
 *
 * converted to C++
 */


#include <IMGFUN.h>
#include <COLCODE.h>


//--------- BEGIN OF FUNCTION IMGremapAreaHMirror -----------
//
// Remap on the VGA screen
//
// char *imageBuf   - the pointer to the display surface buffer
// int  pitch       - the pitch of the display surface buffer
// int  x1,y1         - the top left vertex of the bar
// char *bitmapPtr  - the pointer to the bitmap array
// char **colorTableArray - the pointer to the scale 0 of remap table array
// int  srcX1, srcY1 srcX2, srcY2 - where to get on the source buffer
//

void IMGcall IMGremapAreaHMirror(char*imageBuf,int pitch,int desX,int desY,char*bitmapPtr,unsigned char**colorTableArray,int srcX1,int srcY1,int srcX2,int srcY2)
{
	int dest = (desY+srcY1) * pitch + (desX+srcX1);
	int bitmapWidth = ((unsigned char*)bitmapPtr)[0] + (((unsigned char*)bitmapPtr)[1]<<8);
	//int bitmapHeight = ((unsigned char*)bitmapPtr)[2] + (((unsigned char*)bitmapPtr)[3]<<8);
	int src = 4 + srcY1 * bitmapWidth + srcX1	// 4 bytes are header fields (width, height)
		+ bitmapWidth - 1;						// src points to the end pixel of the line
	int width = srcX2-srcX1+1;
	int height = srcY2-srcY1+1;
	int al;

	for (int j=0; j<height; ++j, dest+=pitch, src+=bitmapWidth)
	{
		for ( int i=0; i<width; ++i )
		{
			al = bitmapPtr[ src - i ];		// NOTE: must be signed
			((unsigned char*)imageBuf)[ dest + i ] =
				colorTableArray[ al ][ imageBuf[dest+i] ];
		}
	}
}

//----------- END OF FUNCTION IMGremapAreaHMirror ----------
