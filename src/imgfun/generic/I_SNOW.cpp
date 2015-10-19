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
 *Filename    : I_SNOW.ASM
 *Description : Draw random white dots of 32x32 sqaure on vga image buffer
 *
 * converted to C++
 */

#include <IMGFUN.h>

#define SNOW_COLOR (0x70)
#define SNOW_MAGIC_NUMBER (0x15a4e35)	// used to generate random numbers

#define WORD_SIZE	32	// TODO: this will need to go into configure.pl
#define ROL(v,a)( ((v)<<a) | ((v)>>(WORD_SIZE-a)) )

//--------- BEGIN OF FUNCTION IMGsnow32x32 -----------
//
// Draw random white dots of 32x32 square on the VGA screen
//
// Note : No border checking is made in this function.
//	 Placing an icon outside image buffer will cause serious BUG.
//
// char *imageBuf   - the pointer to the display surface buffer
// int  pitch       - the pitch of the display surface buffer
// int  x1,y1       - the top left vertex of the bar
// int  randSeed    - random seed
// int  seaLevel    - draw white dot if height > seaLevel
void IMGcall IMGsnow32x32(char*imageBuf,int pitch,int x1,int y1,int randSeed,int seaLevel)
{
	int destline = y1*pitch + x1;
	for (int j=0; j<32; j+=2, destline+=2*pitch)
	{
		randSeed *= SNOW_MAGIC_NUMBER;
		for (int i=0; i<32; i+=2)
		{
			randSeed = ROL((unsigned int)randSeed, 10);
			if ( (randSeed&0xffff) >= (seaLevel&0xffff) )	// only compare the lower 16 bits.
				imageBuf[ destline + i ] = SNOW_COLOR;
		}
	}
}
//---------- END OF FUNCTION IMGsnow32x32 ------------
