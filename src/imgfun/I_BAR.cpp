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
 *Filename    : I_BAR.ASM
 *Description : Draw a rectangle bar on vga image buffer
 *
 * converted to c++
 */


#include <ALL.h>
#include <IMGFUN.h>


//--------- BEGIN OF FUNCTION IMGbar -----------//
//
// Draw a bar on the VGA screen
//
// Note : No border checking is made in this function.
//	 Placing an icon outside image buffer will cause serious BUG.
//
// char *imageBuf   - the pointer to the display surface buffer
// int  pitch       - the pitch of the display surface buffer
// int  x1,y1       - the top left vertex of the bar
// int  x2,y2       - the bottom right vertex of the bar
// int  color       - the color of the line
//
void _stdcall IMGbar(char* imageBuf,int pitch,int x1,int y1,int x2,int y2,int color)
{
	int dest = y1 * pitch + x1;
	int width = x2 - x1 + 1;
	// note: only the byte value of color is used (see memset reference)
	for (int y=y1; y<=y2; ++y, dest+=pitch)
	{
		memset(&imageBuf[dest], color, width);
	}
}


//---------- END OF FUNCTION IMGbar ------------//


