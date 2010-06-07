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
 *Filename    : IR.ASM
 *Description : Remap a bitmap on vga image buffer
 *
 * converted to C++
 */


#include <IMGFUN.h>
#include <COLCODE.h>


//--------- BEGIN OF FUNCTION IMGremap -----------
//
// Remap on the VGA screen
//
// Note : No border checking is made in this function.
//	 Placing an icon outside image buffer will cause serious BUG.
//
// char *imageBuf   - the pointer to the display surface buffer
// int  pitch       - the pitch of the display surface buffer
// int  x1,y1         - the top left vertex of the bar
// char *bitmapPtr  - the pointer to the bitmap array
// char **colorTableArray - the pointer to the scale 0 of remap table array
//

void IMGcall IMGremap(char* imageBuf, int pitch, int x, int y, char* bitmapPtr, unsigned char** colorTableArray)
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
			al = bitmapPtr[ srcline + i ];		// NOTE: must be signed
			((unsigned char*)imageBuf)[ destline + i ] =
				colorTableArray[ al ][ imageBuf[destline+i] ];
		}
	}
}

//----------- END OF FUNCTION IMGremap ----------
