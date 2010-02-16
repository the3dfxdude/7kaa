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
 *Filename    : I_PIXEL.ASM
 *Description : PIXELIZE (1:4 tone) of size 32x32 to the display surface buffer
 *
 *converted to c++
 */

#include <IMGFUN.h>


//----------- BEGIN OF FUNCTION IMGpixel32x32 ------------//
//
// Syntax : IMGpixel( imageBuf, pitch, x, y, colour )
//
// char *imageBuf - the pointer to the display surface buffer
// int  pitch       - the pitch of the display surface buffer
// int  x,y       - where to put the image on the surface buffer
// int  color     - color to pixelize with.
//
//-------------------------------------------------

void _stdcall IMGpixel32x32(char* imageBuf,int pitch,int x,int y,int color)
{
	int line_start;
	for (int j=0; j<32; j+=2)
	{
		line_start = (y+j) * pitch + x;
		for (int i=0; i<32; i+=2)
		{
			imageBuf[ line_start + i ] = color;
		}
	}
}

//----------- END OF FUNCTION IMGpixel32x32 ----------//
