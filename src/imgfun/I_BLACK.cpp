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
 *Filename    : I_BLACK.ASM
 *Description : Draw a black 32x32 sqaure on vga image buffer
 *
 * converted to c++
 */


#include <ALL.h>
#include <IMGFUN.h>

#define BLACK 0
#define FIXED_WIDTH 32
#define FIXED_HEIGHT 32

//--------- BEGIN OF FUNCTION IMGblack32x32 -----------//
//
// Draw a black 32x32 square on the VGA screen
//
// Note : No border checking is made in this function.
//	 Placing an icon outside image buffer will cause serious BUG.
//
// char *imageBuf   - the pointer to the display surface buffer
// int  pitch       - the pitch of the display surface buffer
// int  x1,y1       - the top left vertex of the bar

void IMGcall IMGblack32x32(char* imageBuf,int pitch,int x1,int y1)
{
	int dest = y1*pitch + x1;
	for (int y=0; y<FIXED_WIDTH; ++y, dest+=pitch)
	{
		memset(&imageBuf[dest], BLACK, FIXED_HEIGHT);
	}
}


//---------- END OF FUNCTION IMGblack32x32 ------------//

