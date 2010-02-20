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
 *Filename    : I_EREMAP.ASM
 *Description : Blt exploration remap of 32x32 to the display surface buffer
 * a modified version, 16 mask
 * second modification, 8-bit remap table selector instead of 1-bit tone mask
 *
 * converted to C++
 */


#include <IMGFUN.h>

// ------------ Define constant ------------

#define MASKCOLOUR		0

#define EAST_BIT_MASK	1
#define CENTRE_BIT_MASK 2
#define WEST_BIT_MASK	4

#define NORTH1_MASK_OFFS	0x000
#define SOUTH1_MASK_OFFS	0x100
#define WEST1_MASK_OFFS		0x200
#define EAST1_MASK_OFFS		0x300
#define NW_MASK_OFFS		0x400
#define NE_MASK_OFFS		0x500
#define SW_MASK_OFFS		0x600
#define SE_MASK_OFFS		0x700
#define XNW_MASK_OFFS		0x800
#define XNE_MASK_OFFS		0x900
#define XSW_MASK_OFFS		0xa00
#define XSE_MASK_OFFS		0xb00
#define NORTH2_MASK_OFFS	0xc00
#define SOUTH2_MASK_OFFS	0xd00
#define WEST2_MASK_OFFS		0xe00
#define EAST2_MASK_OFFS		0xf00

// bit 0 = north sqaure, bit 1 = north west square, bit 2 = west square
int RNW_SQR_DECISION[] = {XSE_MASK_OFFS, WEST1_MASK_OFFS, XSE_MASK_OFFS, WEST1_MASK_OFFS,
	NORTH1_MASK_OFFS, NW_MASK_OFFS, NORTH1_MASK_OFFS, -1};

// bit 0 = east square, bit 1 = north east square, bit 2 = north square
int RNE_SQR_DECISION[] = {XSW_MASK_OFFS, NORTH2_MASK_OFFS, XSW_MASK_OFFS, NORTH2_MASK_OFFS,
	EAST1_MASK_OFFS, NE_MASK_OFFS, EAST1_MASK_OFFS, -1};

// bit 0 = south square, bit 1 = south west square, bit 2 = west square
int RSW_SQR_DECISION[] = {XNE_MASK_OFFS, WEST2_MASK_OFFS, XNE_MASK_OFFS, WEST2_MASK_OFFS,
	SOUTH1_MASK_OFFS, SW_MASK_OFFS, SOUTH1_MASK_OFFS, -1};

// bit 0 = east square, bit 1 = south east square, bit 2 = south square
int RSE_SQR_DECISION[] = {XNW_MASK_OFFS, SOUTH2_MASK_OFFS, XNW_MASK_OFFS, SOUTH2_MASK_OFFS,
	EAST2_MASK_OFFS, SE_MASK_OFFS, EAST2_MASK_OFFS, -1};


//------------ BEGIN OF FUNCTION IMGremap16x16 -----------
//
// draw a mask color over a 16x16 quadrant
// char *imageBuf - the pointer to the display surface buffer
// int pitch      - the pitch of the display surface buffer
// int  x,y       - where to put the image on the surface buffer
// char *maskPtr  - pointer to the bit mask (from inside 'EXPLMASK.BIN')
void IMGremap16x16 (char *imageBuf, int pitch, int x, int y,  char* maskPtr, char**colorTableArray)
{
	int offset = y*pitch + x;
	int srcline = 0;
	int al;
	char* table;
	for (int j=0; j<16; ++j, offset+=pitch, srcline+=16)
	{
		for (int i=0; i<16; ++i)
		{
			table = colorTableArray[ maskPtr[ srcline + i ] ];
			imageBuf[ offset + i ] = table[ imageBuf[ offset + i ] ];
		}
	}
}
//------------ END OF FUNCTION IMGremap16x16 -----------


//---------- BEGIN OF FUNCTION IMGexploreRemap32x32 -------
//
// Smooth the area between explore and unexplored square
//
// Syntax : IMGexploreRemap32x32( imageBuf, pitch, x, y, maskPtr, colorTableArray, northRow, thisRow, southRow)
//
// char *imageBuf - the pointer to the display surface buffer
// int pitch      - the pitch of the display surface buffer
// int  x,y       - where to put the image on the surface buffer
// char *maskPtr  - pointer of masks, (address of 'EXPLMASK.BIN' is loaded)
// char **colorTableArray - a list of color tables, which are indexed by the mask
// northRow       - explored_flag of adjacent location
//                  bit 0 - north east square (0=unexplored, 1=explored)
//                  bit 1 - north square
//                  bit 2 - north west square
// thisRow        - bit 0 - east square
//                  bit 1 - this square
//                  bit 2 - west square
// southRow       - bit 0 - south east square
//                  bit 1 - south square
//                  bit 2 - south west square
//
//--------------------------------------------------------
// [alex] TODO: Merge this with I_EMASK, since they are so similar
// TODO: multiply the decision by the size of the masks, rather than
//   defining separate arrays for I_EMASK & I_EREMAP
void _stdcall IMGexploreRemap32x32( char *imageBuf,int pitch, int x, int y, char *maskPtr, char **colorTableArray, int northRow, int thisRow, int southRow)
{
	int maskLookup;
	// NORTH-WEST quadrant decision
	maskLookup = ((northRow & (CENTRE_BIT_MASK | WEST_BIT_MASK))>>1) | (thisRow & WEST_BIT_MASK);
	if (maskLookup < 7)
	{
		IMGremap16x16(imageBuf, pitch, x, y, &maskPtr[RNW_SQR_DECISION[maskLookup]], colorTableArray);
	}
	// NORTH-EAST quadrant decision
	maskLookup = ((northRow & (CENTRE_BIT_MASK | EAST_BIT_MASK))<<1) | (thisRow & EAST_BIT_MASK);
	if (maskLookup < 7)
	{
		IMGremap16x16(imageBuf, pitch, x+16, y, &maskPtr[RNE_SQR_DECISION[maskLookup]], colorTableArray);
	}
	// SOUTH-WEST quadrant decision
	maskLookup = ((southRow & (CENTRE_BIT_MASK | WEST_BIT_MASK))>>1) | (thisRow & WEST_BIT_MASK);
	if (maskLookup < 7)
	{
		IMGremap16x16(imageBuf, pitch, x, y+16, &maskPtr[RSW_SQR_DECISION[maskLookup]], colorTableArray);
	}
	// SOUTH-EAST quadrant decision
	maskLookup = ((southRow & (CENTRE_BIT_MASK | EAST_BIT_MASK))<<1) | (thisRow & EAST_BIT_MASK);
	if (maskLookup < 7)
	{
		IMGremap16x16(imageBuf, pitch, x+16, y+16, &maskPtr[RSE_SQR_DECISION[maskLookup]], colorTableArray);
	}
}
//------------ END OF FUNCTION IMGexploreRemap32x32 -------
