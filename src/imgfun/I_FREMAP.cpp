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
 *Filename    : I_FREMAP.ASM
 *Description : Blt fog remap of 32x32 to the display surface buffer
 *
 * converted to C++
 */


#include <ALL.h>		// for memset
#include <IMGFUN.h>

// ------------ Define constant ------------
#define MAX_VISIBILITY (10)		// added by Alex: maximum level of visibility (when divided by 8)

// B value is the visibility of a location
// 0 = fully invisible, 89 = fully visible
//
// there is 10 scale of darkening remapping, 1 identity remapping (changeless remapping)
// and 10 scale of brightening remapping (but not used here)
//
// suppose the B value of four corners of a 16x16 block is a, b, d and c
// using linear model, to interpolate the interior pixel :
// B(x,y) = [(16-x)(16-y)a + (x)(16-y)b + (16-x)(y)d + xyc ] / 256
//        = [ (16-x)(16a + y(d-a)) + (x)(16b + y(c-b)) ] / 256
// let C1(y) = 16a + y(d-a)
// let C2(y) = 16b + y(c-b)
// B(x,y) = [ (16-x) C1(y) + x C2(y) ] / 256
//
// B(x+1,y) - B(x,y) = [ C2(y) - C1(y) ] / 256
//
// so B(0,0) = a
//    B(x+1,y) = B(x,y) + [ C2(y) - C1(y) ] / 256
//    B(0,y) = [ 16 C1(y) ] / 256
//
//    C1(0) = 16a
//    C1(y+1) = C1(y) + (d-a)
//    C2(0) = 16b
//    C2(y+1) = C2(y) + (c-b)

//------------ BEGIN OF FUNCTION IMGfogRemap16x16 -----------
//
// draw a fogged 16x16 square by remapping
//
// input :
// EDI = destination
// ESI = colorRemapArray
// cornera = north west corner of 16x16
// cornerb = north east corner of 16x16
// cornerc = south east corner of 16x16
// cornerd = south west corner of 16x16
//
//----------------------------------------------------------

void IMGfogRemap16x16(char* imageBuf, int pitch, int x, int y, char**colorTableArray, unsigned char a, unsigned char b, unsigned char c, unsigned char d)
{
	int c1 = 16*a;		// C1(0)
	int c2 = 16*b;		// C2(0)
	int bxy;			// B uses fixed point 24.8 arithmetic
	int C2subC1;
	int remap;
	int dest = y*pitch + x;
	for ( int j=0; j<16; ++j, dest+=pitch, c1+=(d-a), c2+=(c-b) )
	{
		bxy = (16*c1);		// into 24.8: (c1<<4)
		C2subC1 = c2-c1;
		for (int i=0; i<16; ++i, bxy+=C2subC1)
		{
			remap = bxy / 256 / 8		// bxy >> 11, 256 for fix point, 8 for visibility levels
				- (MAX_VISIBILITY-1);	// there are -10 < x < 10 entries in the colorTableArray
			imageBuf[ dest + i ] = colorTableArray[ remap ][ ((unsigned char*)imageBuf)[dest+i] ];
		}
	}
}
//------------ END OF FUNCTION IMGfogRemap16x16 -----------

//------------ BEGIN OF FUNCTION IMGbar16x16 -----------
//
// draw a black 16x16 square
//
// input :
// EDI = destination
//
//----------------------------------------------------------

void IMGbar16x16(char* imageBuf, int pitch, int x, int y, int color)
{
	int dest = y*pitch + x;
	for (int j=0; j<16; ++j, dest+=pitch)
	{
		memset( &imageBuf[dest], color, 16 );
	}
}
//------------ END OF FUNCTION IMGbar16x16 -----------

//------------ BEGIN OF FUNCTION IMGbarRemap16x16 -----------
//
// draw a black 16x16 square
//
// input :
// EDI = destination
// EBX = colorTable
//
//----------------------------------------------------------

void IMGbarRemap16x16(char* imageBuf, int pitch, int x, int y, char* table)
{
	int dest = y*pitch + x;
	for (int j=0; j<16; ++j, dest+=pitch)
	{
		for (int i=0; i<16; ++i)
		{
			imageBuf[dest+i] = table[ ((unsigned char*)imageBuf)[dest+i] ];
		}
	}
}
//------------ END OF FUNCTION IMGbarRemap16x16 -----------


//---------- BEGIN OF FUNCTION decideBarRemap -----------
//
// decide whether to call IMGfogRemap16x16, IMGbar16x16 or skip
//
// input cornera, cornerb, cornerc and cornerd
// ESI = colorRemapArray
// EDI = destination
//
//-------------------------------------------------------

void decideBarRemap(char*imageBuf, int pitch, int x, int y, char**colorTableArray, unsigned char cornera, unsigned char cornerb, unsigned char cornerc, unsigned char cornerd)
{
	unsigned char cornera_lvl = cornera>>3;
	unsigned char cornerb_lvl = cornerb>>3;
	unsigned char cornerc_lvl = cornerc>>3;
	unsigned char cornerd_lvl = cornerd>>3;
	if ( (cornera_lvl==cornerb_lvl) && (cornerc_lvl==cornerd_lvl) && (cornera_lvl==cornerc_lvl) )
	{
		// no interpolation needed
		if (cornera_lvl == MAX_VISIBILITY)	// MAX visibility
		{
			// (don't do anything)
		}
		else if (cornera_lvl == 0)		// invisible, draw black
		{
			IMGbar16x16(imageBuf, pitch, x, y, 0);		// black = 0
		}
		else
		{
			// visibility < 0 : darker, so subtract MAX_VISIBILITY
			IMGbarRemap16x16( imageBuf, pitch, x, y, colorTableArray[cornera_lvl-(MAX_VISIBILITY-1)] );
		}
	}
	else
	{
		IMGfogRemap16x16(imageBuf, pitch, x, y, colorTableArray, cornera, cornerb, cornerc, cornerd);
	}
}
//------------ END OF FUNCTION decideBarRemap -----------



//---------- BEGIN OF FUNCTION IMGfogRemap32x32 -------
//
// Smooth the area in the fog square
//
// Syntax : IMGfogRemap32x32( imageBuf, pitch, x, y, colorTableArray, northRow, thisRow, southRow)
//
// char *imageBuf - the pointer to the display surface buffer
// int  pitch     - the pitch of the display surface buffer
// int  x,y       - where to put the image on the surface buffer
// char **colorTableArray - array of start of each remapping table
// unsigned char *northRow - B value of adjacent location
//                  byte 0 - north east square
//                  byte 1 - north square
//                  byte 2 - north west square
// thisRow        - byte 0 - east square
//                  byte 1 - this square
//                  byte 2 - west square
// southRow       - byte 0 - south east square
//                  byte 1 - south square
//                  byte 2 - south west square
//
//--------------------------------------------------------

void _stdcall IMGfogRemap32x32(char *imageBuf, int pitch, int x, int y, char**colorTableArray, unsigned char*northRow, unsigned char*thisRow, unsigned char*southRow)
{
	decideBarRemap(imageBuf, pitch, x, y, colorTableArray, northRow[2], northRow[1], thisRow[1], thisRow[2]);
	decideBarRemap(imageBuf, pitch, x+16, y, colorTableArray, northRow[1], northRow[0], thisRow[0], thisRow[1]);
	decideBarRemap(imageBuf, pitch, x, y+16, colorTableArray, thisRow[2], thisRow[1], southRow[1], southRow[2]);
	decideBarRemap(imageBuf, pitch, x+16, y+16, colorTableArray, thisRow[1], thisRow[0], southRow[0], southRow[1]);
}
//------------ END OF FUNCTION IMGfogRemap32x32 -------
