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
 *Filename    : IBJ_T.ASM
 *Description : Add a bitmap and another display surface to the display
 *              surface on the same place with transparency handling
 *
 * converted to C++
 */


#include <IMGFUN.h>
#include <COLCODE.h>


//-------- BEGIN OF FUNCTION IMGjoinTrans ----------
//
// Put an non-compressed bitmap on image buffer.
// It handles color key transparency. The color key code is 255.
//
// Syntax : IMGjoinTrans( imageBuf, backBuf x, y, bitmapBuf )
//
// char *imageBuf - the pointer to the display surface buffer
// int imgPitch   - the pitch of the display surface buffer
// char *backBuf  - the pointer to the back buffer
// int backPitch  - the pitch of the back buffer
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

void _stdcall IMGjoinTrans(char* imageBuf, int imgPitch, char* backBuf, int backPitch, int x, int y, char* bitmapPtr)
{
	int destline = y * imgPitch + x;
	int backline = y * backPitch + x;
	int width = ((unsigned char*)bitmapPtr)[0] + (((unsigned char*)bitmapPtr)[1]<<8);
	int height = ((unsigned char*)bitmapPtr)[2] + (((unsigned char*)bitmapPtr)[3]<<8);
	int srcline = 4;		// skip 4 byte header
	int al;

	for ( int j=0; j<height; ++j, destline+=imgPitch, backline+=backPitch, srcline+=width )
	{
		for ( int i=0; i<width; ++i )
		{
			al = ((unsigned char*)bitmapPtr)[ srcline + i ];
			if (al == TRANSPARENT_CODE)
			{
				imageBuf[ destline + i ] = backBuf[ backline + i ];
			}
			else
			{
				imageBuf[ destline + i ] = al;
			}
		}
	}
}

//----------- END OF FUNCTION IMGjoinTrans ----------
