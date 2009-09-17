//Filename    : VGAFUN.H
//Description : Header file for image manipulation functions

#ifndef __VGAFUN_H
#define __VGAFUN_H

//------- Declare external functions ---------//

extern "C"
{
	// not used : void _stdcall IMGinit(int,int);
	void _stdcall IMGbar(char*,int pitch,int,int,int,int,int) __asm__("_IMGbar");
	void _stdcall IMGread(char*,int pitch,int,int,int,int,char*) __asm__("_IMGread");
	void _stdcall IMGblack32x32(char*,int pitch,int,int) __asm__("_IMGblack32x32");

	void _stdcall IMGblt(char*,int pitch,int,int,char*) __asm__("_IMGblt");
	void _stdcall IMGblt2(char*,int pitch,int,int,int,int,char*) __asm__("_IMGblt2");
	void _stdcall IMGblt32x32(char*,int pitch,int,int,char*) __asm__("_IMGblt32x32");
	void _stdcall IMGbltDW(char*,int pitch,int,int,char*) __asm__("_IMGbltDW");
	void _stdcall IMGbltTrans(char*,int pitch,int,int,char*) __asm__("_IMGbltTrans");
	void _stdcall IMGbltTransHMirror(char*,int pitch,int,int,char*);
	void _stdcall IMGbltTransVMirror(char*,int pitch,int,int,char*);
	void _stdcall IMGbltTransHVMirror(char*,int pitch,int,int,char*);
	void _stdcall IMGbltTransRemap(char*,int pitch,int,int,char*,char*) __asm__("_IMGbltTransRemap");
	void _stdcall IMGbltArea(char* imageBuf,int pitch, int desX, int desY, char* bitmapBuf, int srcX1, int srcY1, int srcX2, int srcY2) __asm__("_IMGbltArea");
	void _stdcall IMGbltAreaTrans(char* imageBuf, int pitch, int desX, int desY, char* bitmapBuf, int srcX1, int srcY1, int srcX2, int srcY2) __asm__("_IMGbltAreaTrans");
	void _stdcall IMGbltAreaTransHMirror(char* imageBuf, int pitch, int desX, int desY, char* bitmapBuf, int srcX1, int srcY1, int srcX2, int srcY2);

	void _stdcall IMGbltTransDecompressHMirror(char* imageBuf, int pitch, int desX, int desY, char* bitmapBuf) __asm__("_IMGbltTransDecompressHMirror");
	void _stdcall IMGbltAreaTransDecompressHMirror(char* imageBuf, int pitch, int desX, int desY, char* bitmapBuf, int srcX1, int srcY1, int srcX2, int srcY2) __asm__("_IMGbltAreaTransDecompressHMirror");

	void _stdcall IMGremapDecompress(char* desPtr, char* srcPtr, char* colorTable);
	void _stdcall IMGbltTransDecompress(char* imageBuf, int pitch, int desX, int desY, char* bitmapBuf) __asm__("_IMGbltTransDecompress");
	void _stdcall IMGbltTransRemapDecompress(char* imageBuf, int pitch, int desX, int desY, char* bitmapBuf, char* colorTable) __asm__("_IMGbltTransRemapDecompress");
	void _stdcall IMGbltTransRemapDecompressHMirror(char* imageBuf, int pitch, int desX, int desY, char* bitmapBuf, char* colorTable) __asm__("_IMGbltTransRemapDecompressHMirror");
	void _stdcall IMGbltAreaTransDecompress(char* imageBuf, int pitch, int desX, int desY, char* bitmapBuf, int srcX1, int srcY1, int srcX2, int srcY2) __asm__("_IMGbltAreaTransDecompress");
	void _stdcall IMGbltAreaTransRemapDecompress(char* imageBuf, int pitch, int desX, int desY, char* bitmapBuf, int srcX1, int srcY1, int srcX2, int srcY2, char* colorTable) __asm__("_IMGbltAreaTransRemapDecompress");
	void _stdcall IMGbltAreaTransRemapDecompressHMirror(char* imageBuf, int pitch, int desX, int desY, char* bitmapBuf, int srcX1, int srcY1, int srcX2, int srcY2, char* colorTable) __asm__("_IMGbltAreaTransRemapDecompressHMirror");

	void _stdcall IMGjoinTrans(char* imageBuf, int imgPitch, char* backBuf, int backPitch, int x, int y, char* bitmapPtr) __asm__("_IMGjoinTrans");
	void _stdcall IMGcopy(char* imageBuf, int imgPitch, char* backBuf, int backPitch, int x1, int y1, int x2, int y2) __asm__("_IMGcopy");
	void _stdcall IMGcopyRemap(char*, int imgPitch,char*, int backPitch,int,int,int,int,unsigned char*) __asm__("_IMGcopyRemap");

	// used in wall
	void _stdcall IMGbltRemap(char* imageBuf, int pitch, int desX, int desY, char* bitmapBuf, char* colorTable) __asm__("_IMGbltRemap");
	void _stdcall IMGbltAreaTransRemap(char* imageBuf, int pitch, int desX, int desY, char* bitmapBuf, int srcX1, int srcY1, int srcX2, int srcY2, char* colorTable) __asm__("_IMGbltAreaTransRemap");
	void _stdcall IMGbltAreaRemap(char* imageBuf, int pitch, int desX, int desY, char* bitmapBuf, int srcX1, int srcY1, int srcX2, int srcY2, char* colorTable) __asm__("_IMGbltAreaRemap");

	void _stdcall IMGputBitFont(char*,int pitch,int,int,int,int,int,int,char*,int,int);
	void _stdcall IMGline(char*,int pitch,int w, int h, int,int,int,int,int) __asm__("_IMGline");
	void _stdcall IMGxor(char*,int pitch,int,int,int,int);

	void _stdcall IMGdarken(char*,int pitch,int,int,int,int,int);
	void _stdcall IMGtile(char*,int pitch,int,int,int,int,char*);
	void _stdcall IMGpixel32x32(char*,int pitch,int,int,int) __asm__("_IMGpixel32x32");

	void _stdcall IMGsnow32x32(char*,int pitch,int,int,int,int) __asm__("_IMGsnow32x32");
	void _stdcall IMGexploreMask32x32( char *,int pitch, int, int, char *, int, int, int) __asm__("_IMGexploreMask32x32");;
	void _stdcall IMGexploreRemap32x32( char *,int pitch, int, int, char *, char **,int, int, int) __asm__("_IMGexploreRemap32x32");
	void _stdcall IMGfogRemap32x32( char *,int pitch, int, int, char**, unsigned char*, unsigned char*, unsigned char*) __asm__("_IMGfogRemap32x32");

	// ----- colour remapping functions ------//
	void _stdcall IMGremapBar(char*,int pitch,int,int,int,int,unsigned char*) __asm__("_IMGremapBar");
	void _stdcall IMGremap(char*,int pitch,int,int,char*,unsigned char**) __asm__("_IMGremap");
	void _stdcall IMGremapHMirror(char*,int pitch,int,int,char*,unsigned char**) __asm__("_IMGremapHMirror");
	void _stdcall IMGremapArea(char*,int pitch,int,int,char*,unsigned char**,int,int,int,int) __asm__("_IMGremapArea");
	void _stdcall IMGremapAreaHMirror(char*,int pitch,int,int,char*,unsigned char**,int,int,int,int) __asm__("_IMGremapAreaHMirror");
};

//-------------------------------------------//

#endif

