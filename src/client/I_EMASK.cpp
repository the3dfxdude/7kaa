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
 *Filename    : I_EMASK.ASM
 *Description : Blt exploration mask of 32x32 to the display surface buffer
 * a modified version, 16 mask
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
#define SOUTH1_MASK_OFFS	0x020
#define WEST1_MASK_OFFS		0x040
#define EAST1_MASK_OFFS		0x060
#define NW_MASK_OFFS		0x080
#define NE_MASK_OFFS		0x0a0
#define SW_MASK_OFFS		0x0c0
#define SE_MASK_OFFS		0x0e0
#define XNW_MASK_OFFS		0x100
#define XNE_MASK_OFFS		0x120
#define XSW_MASK_OFFS		0x140
#define XSE_MASK_OFFS		0x160
#define NORTH2_MASK_OFFS	0x180
#define SOUTH2_MASK_OFFS	0x1a0
#define WEST2_MASK_OFFS		0x1c0
#define EAST2_MASK_OFFS		0x1e0

// bit 0 = north sqaure, bit 1 = north west square, bit 2 = west square
int NW_SQR_DECISION[] = {XSE_MASK_OFFS, WEST1_MASK_OFFS, XSE_MASK_OFFS, WEST1_MASK_OFFS,
	NORTH1_MASK_OFFS, NW_MASK_OFFS, NORTH1_MASK_OFFS, -1};

// bit 0 = east square, bit 1 = north east square, bit 2 = north square
int NE_SQR_DECISION[] = {XSW_MASK_OFFS, NORTH2_MASK_OFFS, XSW_MASK_OFFS, NORTH2_MASK_OFFS,
	EAST1_MASK_OFFS, NE_MASK_OFFS, EAST1_MASK_OFFS, -1};

// bit 0 = south square, bit 1 = south west square, bit 2 = west square
int SW_SQR_DECISION[] = {XNE_MASK_OFFS, WEST2_MASK_OFFS, XNE_MASK_OFFS, WEST2_MASK_OFFS,
	SOUTH1_MASK_OFFS, SW_MASK_OFFS, SOUTH1_MASK_OFFS, -1};

// bit 0 = east square, bit 1 = south east square, bit 2 = south square
int SE_SQR_DECISION[] = {XNW_MASK_OFFS, SOUTH2_MASK_OFFS, XNW_MASK_OFFS, SOUTH2_MASK_OFFS,
	EAST2_MASK_OFFS, SE_MASK_OFFS, EAST2_MASK_OFFS, -1};



//------------ BEGIN OF FUNCTION IMGmask16x16 -----------
//
// draw a mask color over a 16x16 quadrant
// char *imageBuf - the pointer to the display surface buffer
// int pitch      - the pitch of the display surface buffer
// int  x,y       - where to put the image on the surface buffer
// char *maskPtr  - pointer to the bit mask (from inside 'EXPLMASK.BIN')
void IMGmask16x16 (char *imageBuf, int pitch, int x, int y,  char* maskPtr)
{
	int offset = y*pitch + x;
	int mask;
	for (int j=0; j<16; ++j, offset+=pitch)
	{
		// lodsw: binary compatible, endian agnostic, somewhat ugly
		mask = (((unsigned char*)maskPtr)[j*2 + 1] << 8) | (((unsigned char*)maskPtr)[j*2 + 0]);
		for (int i=0; i<16; ++i, mask>>=1)
		{
			if ( (mask&0x1) == 0)
			{
				imageBuf[ offset + i ] = MASKCOLOUR;
			}
		}
	}
}
//------------ END OF FUNCTION IMGmask16x16 -----------


//---------- BEGIN OF FUNCTION IMGexploreMask32x32 -------
//
// Smooth the area between explore and unexplored square
//
// Syntax : IMGexploreMask32x32( imageBuf, pitch, x, y, maskPtr, northRow, thisRow, southRow)
//
// char *imageBuf - the pointer to the display surface buffer
// int pitch      - the pitch of the display surface buffer
// int  x,y       - where to put the image on the surface buffer
// char *maskPtr  - pointer of masks, (address of 'EXPLMASK.BIN' is loaded)
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

void _stdcall IMGexploreMask32x32(char *imageBuf,int pitch, int x, int y, char *maskPtr, int northRow, int thisRow, int southRow)
{
	int maskLookup;
	// NORTH-WEST quadrant decision
	maskLookup = ((northRow & (CENTRE_BIT_MASK | WEST_BIT_MASK))>>1) | (thisRow & WEST_BIT_MASK);
	if (maskLookup < 7)
	{
		IMGmask16x16(imageBuf, pitch, x, y, &maskPtr[NW_SQR_DECISION[maskLookup]]);
	}
	// NORTH-EAST quadrant decision
	maskLookup = ((northRow & (CENTRE_BIT_MASK | EAST_BIT_MASK))<<1) | (thisRow & EAST_BIT_MASK);
	if (maskLookup < 7)
	{
		IMGmask16x16(imageBuf, pitch, x+16, y, &maskPtr[NE_SQR_DECISION[maskLookup]]);
	}
	// SOUTH-WEST quadrant decision
	maskLookup = ((southRow & (CENTRE_BIT_MASK | WEST_BIT_MASK))>>1) | (thisRow & WEST_BIT_MASK);
	if (maskLookup < 7)
	{
		IMGmask16x16(imageBuf, pitch, x, y+16, &maskPtr[SW_SQR_DECISION[maskLookup]]);
	}
	// SOUTH-EAST quadrant decision
	maskLookup = ((southRow & (CENTRE_BIT_MASK | EAST_BIT_MASK))<<1) | (thisRow & EAST_BIT_MASK);
	if (maskLookup < 7)
	{
		IMGmask16x16(imageBuf, pitch, x+16, y+16, &maskPtr[SE_SQR_DECISION[maskLookup]]);
	}
}

//------------ END OF FUNCTION IMGexploreMask32x32 -------
