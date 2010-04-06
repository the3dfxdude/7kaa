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
 *Filename    : I_LINE.ASM
 *Description : Draw a line in the image buffer
 *
 *converted to C++
 */

#include <IMGFUN.h>
#include <stdlib.h>		// for abs

// Alex: removed the following as they are non-reentrant. Using inline function instead.
//int buf_width;
//int buf_height;
//int buf_pitch;
inline void line_point (char* imageBuf, int pitch, int w, int h, int x, int y, int color);


//-------- BEGIN OF FUNCTION IMGline -----------//
//
// Draw a line on the IMG screen
//
// char *imageBuf - the pointer to the display surface buffer
// int  pitch     - the pitch of the display surface buffer
// int  w         - the width of the display surface buffer
// int  h         - the height of the display surface buffer
// int  x1,y1 	 - the top left vertex of the bar
// int  x2,y2 	 - the bottom right vertex of the bar
// int  color 	 - the color of the line
//
void _stdcall IMGline(char* imageBuf,int pitch,int w, int h, int x1,int y1,int x2,int y2,int color)
{
	int acc = 0;		// accumulator for the overflow (bresenham's line algorithm)
	int dirx;
	int diry;
	int deltax = x2 - x1;
	int deltay = y2 - y1;
	int x = x1;
	int y = y1;

	dirx = (deltax>0) ? (1) : (-1);		// direction
	deltax = abs(deltax);

	diry = (deltay>0) ? (1) : (-1);
	deltay = abs(deltay);

	if ( deltay >= deltax )
	{
		// proceed along y
		for ( int j = 0; j <= deltay; ++j )		// inclusive of end pixel
		{
			line_point(imageBuf, pitch, w, h, x, y, color);
			y += diry;
			acc += deltax;
			if (acc >= deltay) x += dirx;
		}
	}
	else
	{
		// proceed along x
		for ( int i = 0; i <= deltax; ++i )		// inclusive of end pixel
		{
			line_point(imageBuf, pitch, w, h, x, y, color);
			x += dirx;
			acc += deltay;
			if (acc >= deltax) y += diry;
		}
	}
}
//------- END OF FUNCTION IMGline ------------//



//------ BEGIN OF FUNCTION line_point -----------
//
// It is a private function called by VGAline().
//
// Parameter : ESI - x position
//             EDI - y position
//

inline void line_point (char* imageBuf, int pitch, int w, int h, int x, int y, int color)
{
	if ( (x >= 0) && (x < w) && (y >= 0) && (y < h) )
		imageBuf[ y * pitch + x ] = color;
}

//---------- END OF FUNCTION line_point ------------//
