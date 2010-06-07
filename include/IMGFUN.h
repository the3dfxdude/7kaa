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
	void IMGcall IMGbar(char*,int pitch,int,int,int,int,int) __asmsym__("_IMGbar");
	void IMGcall IMGread(char*,int pitch,int,int,int,int,char*) __asmsym__("_IMGread");
	void IMGcall IMGblack32x32(char*,int pitch,int,int) __asmsym__("_IMGblack32x32");

	void IMGcall IMGblt(char*,int pitch,int,int,char*) __asmsym__("_IMGblt");
	void IMGcall IMGblt2(char*,int pitch,int,int,int,int,char*) __asmsym__("_IMGblt2");
	void IMGcall IMGblt32x32(char*,int pitch,int,int,char*) __asmsym__("_IMGblt32x32");
	void IMGcall IMGbltDW(char*,int pitch,int,int,char*) __asmsym__("_IMGbltDW");
	void IMGcall IMGbltTrans(char*,int pitch,int,int,char*) __asmsym__("_IMGbltTrans");
	void IMGcall IMGbltTransHMirror(char*,int pitch,int,int,char*);
	void IMGcall IMGbltTransVMirror(char*,int pitch,int,int,char*);
	void IMGcall IMGbltTransHVMirror(char*,int pitch,int,int,char*);
	void IMGcall IMGbltTransRemap(char*,int pitch,int,int,char*,char*) __asmsym__("_IMGbltTransRemap");
	void IMGcall IMGbltArea(char* imageBuf,int pitch, int desX, int desY, char* bitmapBuf, int srcX1, int srcY1, int srcX2, int srcY2) __asmsym__("_IMGbltArea");
	void IMGcall IMGbltAreaTrans(char* imageBuf, int pitch, int desX, int desY, char* bitmapBuf, int srcX1, int srcY1, int srcX2, int srcY2) __asmsym__("_IMGbltAreaTrans");
	void IMGcall IMGbltAreaTransHMirror(char* imageBuf, int pitch, int desX, int desY, char* bitmapBuf, int srcX1, int srcY1, int srcX2, int srcY2);

	void IMGcall IMGbltTransDecompressHMirror(char* imageBuf, int pitch, int desX, int desY, char* bitmapBuf) __asmsym__("_IMGbltTransDecompressHMirror");
	void IMGcall IMGbltAreaTransDecompressHMirror(char* imageBuf, int pitch, int desX, int desY, char* bitmapBuf, int srcX1, int srcY1, int srcX2, int srcY2) __asmsym__("_IMGbltAreaTransDecompressHMirror");

	void IMGcall IMGremapDecompress(char* desPtr, char* srcPtr, char* colorTable);
	void IMGcall IMGbltTransDecompress(char* imageBuf, int pitch, int desX, int desY, char* bitmapBuf) __asmsym__("_IMGbltTransDecompress");
	void IMGcall IMGbltTransRemapDecompress(char* imageBuf, int pitch, int desX, int desY, char* bitmapBuf, char* colorTable) __asmsym__("_IMGbltTransRemapDecompress");
	void IMGcall IMGbltTransRemapDecompressHMirror(char* imageBuf, int pitch, int desX, int desY, char* bitmapBuf, char* colorTable) __asmsym__("_IMGbltTransRemapDecompressHMirror");
	void IMGcall IMGbltAreaTransDecompress(char* imageBuf, int pitch, int desX, int desY, char* bitmapBuf, int srcX1, int srcY1, int srcX2, int srcY2) __asmsym__("_IMGbltAreaTransDecompress");
	void IMGcall IMGbltAreaTransRemapDecompress(char* imageBuf, int pitch, int desX, int desY, char* bitmapBuf, int srcX1, int srcY1, int srcX2, int srcY2, char* colorTable) __asmsym__("_IMGbltAreaTransRemapDecompress");
	void IMGcall IMGbltAreaTransRemapDecompressHMirror(char* imageBuf, int pitch, int desX, int desY, char* bitmapBuf, int srcX1, int srcY1, int srcX2, int srcY2, char* colorTable) __asmsym__("_IMGbltAreaTransRemapDecompressHMirror");

	void IMGcall IMGjoinTrans(char* imageBuf, int imgPitch, char* backBuf, int backPitch, int x, int y, char* bitmapPtr) __asmsym__("_IMGjoinTrans");
	void IMGcall IMGcopy(char* imageBuf, int imgPitch, char* backBuf, int backPitch, int x1, int y1, int x2, int y2) __asmsym__("_IMGcopy");
	void IMGcall IMGcopyRemap(char*, int imgPitch,char*, int backPitch,int,int,int,int,unsigned char*) __asmsym__("_IMGcopyRemap");

	// used in wall
	void IMGcall IMGbltRemap(char* imageBuf, int pitch, int desX, int desY, char* bitmapBuf, char* colorTable) __asmsym__("_IMGbltRemap");
	void IMGcall IMGbltAreaTransRemap(char* imageBuf, int pitch, int desX, int desY, char* bitmapBuf, int srcX1, int srcY1, int srcX2, int srcY2, char* colorTable) __asmsym__("_IMGbltAreaTransRemap");
	void IMGcall IMGbltAreaRemap(char* imageBuf, int pitch, int desX, int desY, char* bitmapBuf, int srcX1, int srcY1, int srcX2, int srcY2, char* colorTable) __asmsym__("_IMGbltAreaRemap");

	void IMGcall IMGputBitFont(char*,int pitch,int,int,int,int,int,int,char*,int,int);
	void IMGcall IMGline(char*,int pitch,int w, int h, int,int,int,int,int) __asmsym__("_IMGline");
	void IMGcall IMGxor(char*,int pitch,int,int,int,int);

	void IMGcall IMGdarken(char*,int pitch,int,int,int,int,int);
	void IMGcall IMGtile(char*,int pitch,int,int,int,int,char*);
	void IMGcall IMGpixel32x32(char*,int pitch,int,int,int) __asmsym__("_IMGpixel32x32");

	void IMGcall IMGsnow32x32(char*,int pitch,int,int,int,int) __asmsym__("_IMGsnow32x32");
	void IMGcall IMGexploreMask32x32( char *,int pitch, int, int, char *, int, int, int) __asmsym__("_IMGexploreMask32x32");;
	void IMGcall IMGexploreRemap32x32( char *,int pitch, int, int, char *, char **,int, int, int) __asmsym__("_IMGexploreRemap32x32");
	void IMGcall IMGfogRemap32x32( char *,int pitch, int, int, char**, unsigned char*, unsigned char*, unsigned char*) __asmsym__("_IMGfogRemap32x32");

	// ----- colour remapping functions ------//
	void IMGcall IMGremapBar(char*,int pitch,int,int,int,int,unsigned char*) __asmsym__("_IMGremapBar");
	void IMGcall IMGremap(char*,int pitch,int,int,char*,unsigned char**) __asmsym__("_IMGremap");
	void IMGcall IMGremapHMirror(char*,int pitch,int,int,char*,unsigned char**) __asmsym__("_IMGremapHMirror");
	void IMGcall IMGremapArea(char*,int pitch,int,int,char*,unsigned char**,int,int,int,int) __asmsym__("_IMGremapArea");
	void IMGcall IMGremapAreaHMirror(char*,int pitch,int,int,char*,unsigned char**,int,int,int,int) __asmsym__("_IMGremapAreaHMirror");
};

//-------------------------------------------//

#endif

