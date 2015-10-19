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
 */

//Filename    : VGAFUN.H
//Description : Header file for image manipulation functions

#ifndef __VGAFUN_H
#define __VGAFUN_H

#include <asmfun.h>

//------- Declare external functions ---------//

extern "C"
{
	// not used : void IMGcall IMGinit(int,int);
	void IMGcall IMGbar(char*,int pitch,int,int,int,int,int);
	void IMGcall IMGread(char*,int pitch,int,int,int,int,char*);
	void IMGcall IMGblack32x32(char*,int pitch,int,int);

	void IMGcall IMGblt(char*,int pitch,int,int,char*);
	void IMGcall IMGblt2(char*,int pitch,int,int,int,int,char*);
	void IMGcall IMGblt32x32(char*,int pitch,int,int,char*);
	void IMGcall IMGbltDW(char*,int pitch,int,int,char*);
	void IMGcall IMGbltTrans(char*,int pitch,int,int,char*);
	void IMGcall IMGbltTransHMirror(char*,int pitch,int,int,char*);
	void IMGcall IMGbltTransVMirror(char*,int pitch,int,int,char*);
	void IMGcall IMGbltTransHVMirror(char*,int pitch,int,int,char*);
	void IMGcall IMGbltTransRemap(char*,int pitch,int,int,char*,char*);
	void IMGcall IMGbltArea(char* imageBuf,int pitch, int desX, int desY, char* bitmapBuf, int srcX1, int srcY1, int srcX2, int srcY2);
	void IMGcall IMGbltAreaTrans(char* imageBuf, int pitch, int desX, int desY, char* bitmapBuf, int srcX1, int srcY1, int srcX2, int srcY2);
	void IMGcall IMGbltAreaTransHMirror(char* imageBuf, int pitch, int desX, int desY, char* bitmapBuf, int srcX1, int srcY1, int srcX2, int srcY2);

	void IMGcall IMGbltTransDecompressHMirror(char* imageBuf, int pitch, int desX, int desY, char* bitmapBuf);
	void IMGcall IMGbltAreaTransDecompressHMirror(char* imageBuf, int pitch, int desX, int desY, char* bitmapBuf, int srcX1, int srcY1, int srcX2, int srcY2);

	void IMGcall IMGremapDecompress(char* desPtr, char* srcPtr, char* colorTable);
	void IMGcall IMGbltTransDecompress(char* imageBuf, int pitch, int desX, int desY, char* bitmapBuf);
	void IMGcall IMGbltTransRemapDecompress(char* imageBuf, int pitch, int desX, int desY, char* bitmapBuf, char* colorTable);
	void IMGcall IMGbltTransRemapDecompressHMirror(char* imageBuf, int pitch, int desX, int desY, char* bitmapBuf, char* colorTable);
	void IMGcall IMGbltAreaTransDecompress(char* imageBuf, int pitch, int desX, int desY, char* bitmapBuf, int srcX1, int srcY1, int srcX2, int srcY2);
	void IMGcall IMGbltAreaTransRemapDecompress(char* imageBuf, int pitch, int desX, int desY, char* bitmapBuf, int srcX1, int srcY1, int srcX2, int srcY2, char* colorTable);
	void IMGcall IMGbltAreaTransRemapDecompressHMirror(char* imageBuf, int pitch, int desX, int desY, char* bitmapBuf, int srcX1, int srcY1, int srcX2, int srcY2, char* colorTable);

	void IMGcall IMGjoinTrans(char* imageBuf, int imgPitch, char* backBuf, int backPitch, int x, int y, char* bitmapPtr);
	void IMGcall IMGcopy(char* imageBuf, int imgPitch, char* backBuf, int backPitch, int x1, int y1, int x2, int y2);
	void IMGcall IMGcopyRemap(char*, int imgPitch,char*, int backPitch,int,int,int,int,unsigned char*);

	// used in wall
	void IMGcall IMGbltRemap(char* imageBuf, int pitch, int desX, int desY, char* bitmapBuf, char* colorTable);
	void IMGcall IMGbltAreaTransRemap(char* imageBuf, int pitch, int desX, int desY, char* bitmapBuf, int srcX1, int srcY1, int srcX2, int srcY2, char* colorTable);
	void IMGcall IMGbltAreaRemap(char* imageBuf, int pitch, int desX, int desY, char* bitmapBuf, int srcX1, int srcY1, int srcX2, int srcY2, char* colorTable);

	void IMGcall IMGputBitFont(char*,int pitch,int,int,int,int,int,int,char*,int,int);
	void IMGcall IMGline(char*,int pitch,int w, int h, int,int,int,int,int);
	void IMGcall IMGxor(char*,int pitch,int,int,int,int);

	void IMGcall IMGdarken(char*,int pitch,int,int,int,int,int);
	void IMGcall IMGtile(char*,int pitch,int,int,int,int,char*);
	void IMGcall IMGpixel32x32(char*,int pitch,int,int,int);

	void IMGcall IMGsnow32x32(char*,int pitch,int,int,int,int);
	void IMGcall IMGexploreMask32x32( char *,int pitch, int, int, char *, int, int, int);
	void IMGcall IMGexploreRemap32x32( char *,int pitch, int, int, char *, char **,int, int, int);
	void IMGcall IMGfogRemap32x32( char *,int pitch, int, int, char**, unsigned char*, unsigned char*, unsigned char*);

	// ----- colour remapping functions ------//
	void IMGcall IMGremapBar(char*,int pitch,int,int,int,int,unsigned char*);
	void IMGcall IMGremap(char*,int pitch,int,int,char*,unsigned char**);
	void IMGcall IMGremapHMirror(char*,int pitch,int,int,char*,unsigned char**);
	void IMGcall IMGremapArea(char*,int pitch,int,int,char*,unsigned char**,int,int,int,int);
	void IMGcall IMGremapAreaHMirror(char*,int pitch,int,int,char*,unsigned char**,int,int,int,int);
};

//-------------------------------------------//

#endif

