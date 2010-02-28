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

#include <windows.h>

// Align up the asm symbols between jwasm and gcc object files
// This is not needed when we build without jwasm
#ifdef NO_ASM
#define __asmsym__(s)
#else
#define __asmsym__(s) __asm__(s)
#endif

//------- Declare external functions ---------//

extern "C"
{
	// not used : void _stdcall IMGinit(int,int);
	void _stdcall IMGbar(char*,int pitch,int,int,int,int,int) __asmsym__("_IMGbar");
	void _stdcall IMGread(char*,int pitch,int,int,int,int,char*) __asmsym__("_IMGread");
	void _stdcall IMGblack32x32(char*,int pitch,int,int) __asmsym__("_IMGblack32x32");

	void _stdcall IMGblt(char*,int pitch,int,int,char*) __asmsym__("_IMGblt");
	void _stdcall IMGblt2(char*,int pitch,int,int,int,int,char*) __asmsym__("_IMGblt2");
	void _stdcall IMGblt32x32(char*,int pitch,int,int,char*) __asmsym__("_IMGblt32x32");
	void _stdcall IMGbltDW(char*,int pitch,int,int,char*) __asmsym__("_IMGbltDW");
	void _stdcall IMGbltTrans(char*,int pitch,int,int,char*) __asmsym__("_IMGbltTrans");
	void _stdcall IMGbltTransHMirror(char*,int pitch,int,int,char*);
	void _stdcall IMGbltTransVMirror(char*,int pitch,int,int,char*);
	void _stdcall IMGbltTransHVMirror(char*,int pitch,int,int,char*);
	void _stdcall IMGbltTransRemap(char*,int pitch,int,int,char*,char*) __asmsym__("_IMGbltTransRemap");
	void _stdcall IMGbltArea(char* imageBuf,int pitch, int desX, int desY, char* bitmapBuf, int srcX1, int srcY1, int srcX2, int srcY2) __asmsym__("_IMGbltArea");
	void _stdcall IMGbltAreaTrans(char* imageBuf, int pitch, int desX, int desY, char* bitmapBuf, int srcX1, int srcY1, int srcX2, int srcY2) __asmsym__("_IMGbltAreaTrans");
	void _stdcall IMGbltAreaTransHMirror(char* imageBuf, int pitch, int desX, int desY, char* bitmapBuf, int srcX1, int srcY1, int srcX2, int srcY2);

	void _stdcall IMGbltTransDecompressHMirror(char* imageBuf, int pitch, int desX, int desY, char* bitmapBuf) __asmsym__("_IMGbltTransDecompressHMirror");
	void _stdcall IMGbltAreaTransDecompressHMirror(char* imageBuf, int pitch, int desX, int desY, char* bitmapBuf, int srcX1, int srcY1, int srcX2, int srcY2) __asmsym__("_IMGbltAreaTransDecompressHMirror");

	void _stdcall IMGremapDecompress(char* desPtr, char* srcPtr, char* colorTable);
	void _stdcall IMGbltTransDecompress(char* imageBuf, int pitch, int desX, int desY, char* bitmapBuf) __asmsym__("_IMGbltTransDecompress");
	void _stdcall IMGbltTransRemapDecompress(char* imageBuf, int pitch, int desX, int desY, char* bitmapBuf, char* colorTable) __asmsym__("_IMGbltTransRemapDecompress");
	void _stdcall IMGbltTransRemapDecompressHMirror(char* imageBuf, int pitch, int desX, int desY, char* bitmapBuf, char* colorTable) __asmsym__("_IMGbltTransRemapDecompressHMirror");
	void _stdcall IMGbltAreaTransDecompress(char* imageBuf, int pitch, int desX, int desY, char* bitmapBuf, int srcX1, int srcY1, int srcX2, int srcY2) __asmsym__("_IMGbltAreaTransDecompress");
	void _stdcall IMGbltAreaTransRemapDecompress(char* imageBuf, int pitch, int desX, int desY, char* bitmapBuf, int srcX1, int srcY1, int srcX2, int srcY2, char* colorTable) __asmsym__("_IMGbltAreaTransRemapDecompress");
	void _stdcall IMGbltAreaTransRemapDecompressHMirror(char* imageBuf, int pitch, int desX, int desY, char* bitmapBuf, int srcX1, int srcY1, int srcX2, int srcY2, char* colorTable) __asmsym__("_IMGbltAreaTransRemapDecompressHMirror");

	void _stdcall IMGjoinTrans(char* imageBuf, int imgPitch, char* backBuf, int backPitch, int x, int y, char* bitmapPtr) __asmsym__("_IMGjoinTrans");
	void _stdcall IMGcopy(char* imageBuf, int imgPitch, char* backBuf, int backPitch, int x1, int y1, int x2, int y2) __asmsym__("_IMGcopy");
	void _stdcall IMGcopyRemap(char*, int imgPitch,char*, int backPitch,int,int,int,int,unsigned char*) __asmsym__("_IMGcopyRemap");

	// used in wall
	void _stdcall IMGbltRemap(char* imageBuf, int pitch, int desX, int desY, char* bitmapBuf, char* colorTable) __asmsym__("_IMGbltRemap");
	void _stdcall IMGbltAreaTransRemap(char* imageBuf, int pitch, int desX, int desY, char* bitmapBuf, int srcX1, int srcY1, int srcX2, int srcY2, char* colorTable) __asmsym__("_IMGbltAreaTransRemap");
	void _stdcall IMGbltAreaRemap(char* imageBuf, int pitch, int desX, int desY, char* bitmapBuf, int srcX1, int srcY1, int srcX2, int srcY2, char* colorTable) __asmsym__("_IMGbltAreaRemap");

	void _stdcall IMGputBitFont(char*,int pitch,int,int,int,int,int,int,char*,int,int);
	void _stdcall IMGline(char*,int pitch,int w, int h, int,int,int,int,int) __asmsym__("_IMGline");
	void _stdcall IMGxor(char*,int pitch,int,int,int,int);

	void _stdcall IMGdarken(char*,int pitch,int,int,int,int,int);
	void _stdcall IMGtile(char*,int pitch,int,int,int,int,char*);
	void _stdcall IMGpixel32x32(char*,int pitch,int,int,int) __asmsym__("_IMGpixel32x32");

	void _stdcall IMGsnow32x32(char*,int pitch,int,int,int,int) __asmsym__("_IMGsnow32x32");
	void _stdcall IMGexploreMask32x32( char *,int pitch, int, int, char *, int, int, int) __asmsym__("_IMGexploreMask32x32");;
	void _stdcall IMGexploreRemap32x32( char *,int pitch, int, int, char *, char **,int, int, int) __asmsym__("_IMGexploreRemap32x32");
	void _stdcall IMGfogRemap32x32( char *,int pitch, int, int, char**, unsigned char*, unsigned char*, unsigned char*) __asmsym__("_IMGfogRemap32x32");

	// ----- colour remapping functions ------//
	void _stdcall IMGremapBar(char*,int pitch,int,int,int,int,unsigned char*) __asmsym__("_IMGremapBar");
	void _stdcall IMGremap(char*,int pitch,int,int,char*,unsigned char**) __asmsym__("_IMGremap");
	void _stdcall IMGremapHMirror(char*,int pitch,int,int,char*,unsigned char**) __asmsym__("_IMGremapHMirror");
	void _stdcall IMGremapArea(char*,int pitch,int,int,char*,unsigned char**,int,int,int,int) __asmsym__("_IMGremapArea");
	void _stdcall IMGremapAreaHMirror(char*,int pitch,int,int,char*,unsigned char**,int,int,int,int) __asmsym__("_IMGremapAreaHMirror");
};

//-------------------------------------------//

#endif

