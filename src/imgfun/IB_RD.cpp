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
 *Filename    : IB_RD.ASM
 *Description : decompress and color remapping handling with color key transparency
 *
 * converted to C++
 */


#include <ALL.h>		// for memset()
#include <IMGFUN.h>
#include <COLCODE.h>



//-------- BEGIN OF FUNCTION IMGremapDecompress ----------
//
// Decompress bitmap on image buffer with Transparency.
// It handles color key transparency. The color key code is 255.
//
// R - color remapping
// D - decompression
//
// Syntax : IMGremapDecompress(desPtr, srcPtr, colorTable )
//
// char *desPtr   - the pointer to the compressed surface buffer
// char *srcPtr   - the pointer to the decompressed surface buffer
// char *colorTable - a 256-entry color remapping table
//
//-------------------------------------------------
//
// Format of the compressed data
// compressed	decompressed
// FF		FF
// FE (-2)	FF FF
// FD (-3)	FF FF FF
// FC (-4)	FF FF FF FF
// FB (-5)	FF FF FF FF FF
// FA (-6)	FF FF FF FF FF FF
// F9 (-7)	FF FF FF FF FF FF FF
// F8 B		FF ... <B times>
//-------------------------------------------------
//
// Format of the bitmap data :
//
// <short>  width
// <short>  height
// <char..> bitmap image
//
//-------------------------------------------------

void IMGcall IMGremapDecompress(char* desPtr, char* srcPtr, char* colorTable)
{
	int width = ((unsigned char*)srcPtr)[0] + (((unsigned char*)srcPtr)[1]<<8);
	int height = ((unsigned char*)srcPtr)[2] + (((unsigned char*)srcPtr)[3]<<8);
	// write the destination bitmap header (width, height)
	((unsigned char*)desPtr)[0] = width;
	((unsigned char*)desPtr)[1] = width>>8;
	((unsigned char*)desPtr)[2] = height;
	((unsigned char*)desPtr)[3] = height>>8;
	int esi = 4;		// 4 bytes of bitmap header (16bit width, 16bit height)
	int destline = 4;	// 4 bytes of bitmap header
	int pixelsToSkip = 0;
	int al;

	for ( int j=0; j<height; ++j,destline+=width )
	{
		for ( int i=0; i<width; ++i )
		{
			if (pixelsToSkip != 0)
			{
				if (pixelsToSkip >= width-i)
				{
					// skip to next line
					memset( &desPtr[ destline+i ], TRANSPARENT_CODE, width-i);
					pixelsToSkip -= width-i;
					break;
				}
				memset( &desPtr[ destline+i ], TRANSPARENT_CODE, pixelsToSkip);
				i += pixelsToSkip;
				pixelsToSkip = 0;
			}
			al = ((unsigned char*)srcPtr)[ esi++ ];		// load source byte
			if (al < MIN_TRANSPARENT_CODE)
			{
				((unsigned char*)desPtr)[ destline + i ] = al;	// normal pixel
			}
			else
			{
				((unsigned char*)desPtr)[ destline + i ] = TRANSPARENT_CODE;				// transparent pixel
				if (al == MANY_TRANSPARENT_CODE)
				{
					pixelsToSkip = ((unsigned char*)srcPtr)[ esi++ ] -1;		// skip many pixels
				}
				else
				{
					pixelsToSkip = 256 - al -1;					// skip (neg al) pixels
				}
			}
		}
	}
}
//----------- END OF FUNCTION IMGremapDecompress ----------
