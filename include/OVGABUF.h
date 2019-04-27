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

//Filename    : VGABUF.H
//Description : Header file for class VgaBuffer, SDL_Surface version

#ifndef __VGABUF_H
#define __VGABUF_H

#include <IMGFUN.h>
#include <SDL.h>

//-------- Define class VgaBuf ----------------//

class File;

class VgaBuf
{
public:
	SDL_Surface*					surface;
	char*						cur_buf_ptr;
	int						buf_locked;			// whether the and back buffers have been locked or not.
	char						is_front;			// whether it's the front buffer or not
	char                                            save_locked_flag;

public:
	//--------- back buffer ----------//

	char* buf_ptr()             { return cur_buf_ptr; }
	char* buf_ptr(int x, int y) { return cur_buf_ptr + surface->pitch*y + x;  }
	int   buf_pitch()           { return surface->pitch; }
	int   buf_size()            { return surface->h * surface->w; }
	int   buf_width()           { return surface->w; }
	int   buf_height()          { return surface->h; }

	//---- GUI colors -----//

	static char color_light;      // color of the light panel side
	static char color_dark;       // color of the dark panel side
	static char color_up;         // color of up_panel surface
	static char color_down;       // color of down_panel surface
	static char color_push;       // color of pushed button surface
	static char color_border;     // color of color

public:
	VgaBuf();
	~VgaBuf();

	//---------- system functions ----------//

	void            init(char front, int w=0, int h=0);
	void		deinit();

	int		is_buf_lost()					 { return 0; }
	int		restore_buf()					 { return 1; }

	void		activate_pal(SDL_Color *pal);

	void		lock_buf();
	void		unlock_buf();

	void		temp_unlock();
	void		temp_restore_lock();

	void		set_buf_ptr(char* bufPtr)			{ cur_buf_ptr = bufPtr; }
	void		set_default_buf_ptr()				{ cur_buf_ptr = (char*)surface->pixels; }

	int		write_bmp_file(char* fileName)			{ return !SDL_SaveBMP(surface, fileName); }

	//---------- painting functions ----------//

	void		bar(int x1,int y1,int x2,int y2,int colorCode);
	void		bar_up(int x1,int y1,int x2,int y2)		{ bar( x1, y1, x2, y2, color_up ); }
	void		bar_down(int x1,int y1,int x2,int y2)		{ bar( x1, y1, x2, y2, color_down ); }

	void		pixelize(int x1,int y1,int x2,int y2,int colorCode);
	// ####### begin Gilbert 7/7 ##########//
	void		draw_pixel(int x1,int y1,int colorCode)
				{ *(buf_ptr()+buf_pitch()*y1+x1) = colorCode; }
	// ####### end Gilbert 7/7 ##########//

	void  	separator(int,int,int,int);
	void		line(int x1,int y1,int x2,int y2,int colorCode);
	void  	thick_line(int x1,int y1,int x2,int y2,int colorCode);

	void  	indicator(int,int,int,int,float,float,int,int= -1);
	void		indicator(int barType, int x,int y,float,float,int colorScheme);
	void  	v_indicator(int,int,int,int,float,float,int,int= -1);

	void  	rect(int,int,int,int,int,int);
	void  	d3_rect(int,int,int,int);

	void  	d3_panel_up(int,int,int,int,int=2,int=1);
	void  	d3_panel_down(int,int,int,int,int=2,int=1);
	void  	d3_panel_up_clear(int,int,int,int,int=2);
	void  	d3_panel_down_clear(int,int,int,int,int=2);

	void  	tile(int,int,int,int,char*);
	void 		adjust_brightness(int x1,int y1,int x2,int y2,int adjustDegree);
	void 		draw_d3_up_border(int x1,int y1,int x2,int y2);
	void 		draw_d3_down_border(int x1,int y1,int x2,int y2);

	void 		convert_gray(int x1, int y1, int x2, int y2);

	//-------- buffer saving functions --------//

	char* 	save_area(int,int,int,int,char* =0L);
	void  	rest_area(char*,int=1 );

	void  	save_area_common_buf(int,int,int,int);
	void  	rest_area_common_buf();

	//------- bitmap displaying functions -------//

	void		put_bitmap_trans(int x,int y,char* bitmapPtr);
	void		put_bitmap_remap(int desX, int desY, char* bitmapPtr, char *colorTable);
	void 		put_large_bitmap(int x1, int y1, File* filePtr, int useStretch=0);
	void		put_bitmap(int x,int y,char* bitmapPtr);

	//---------- assembly bitmap manipulation functions ----------//

	void		fast_put_bitmap(int x, int y, char* bitmapPtr)
				{ IMGblt(buf_ptr(), buf_pitch(), x, y, bitmapPtr); }
	void		put_bitmap2_s(int x, int y, int bitmapWidth, int bitmapHeight, char* bitmapPtr)
				{
					IMGblt3(buf_ptr(), buf_pitch(), buf_height(), x, y, bitmapWidth, bitmapHeight, bitmapPtr);
				}
	void		put_bitmap2(int x,int y,int bitmapWidth, int bitmapHeight, char* bitmapPtr)
				{ 
					IMGblt2(buf_ptr(), buf_pitch(), x, y, bitmapWidth, bitmapHeight, bitmapPtr); 
				}

	void		put_bitmap_32x32(int x,int y,char* bitmapPtr)
				{ IMGblt32x32(buf_ptr(), buf_pitch(), x, y, bitmapPtr); }

	void		put_bitmap_dw(int x,int y,char* bitmapPtr)
				{ IMGbltDW(buf_ptr(), buf_pitch(), x, y, bitmapPtr); }

	void		xor_buf(int x1,int y1,int x2,int y2)
				{ IMGxor(buf_ptr(), buf_pitch(), x1, y1, x2, y2); }

	void		put_bitmap_area(int desX,int desY,char* bitmapPtr,int srcX1, int srcY1, int srcX2, int srcY2)
				{ IMGbltArea(buf_ptr(), buf_pitch(), desX, desY, bitmapPtr, srcX1, srcY1, srcX2, srcY2); }

	void		put_bitmap_area_trans(int desX,int desY,char* bitmapPtr,int srcX1, int srcY1, int srcX2, int srcY2)
				{ IMGbltAreaTrans(buf_ptr(), buf_pitch(), desX, desY, bitmapPtr, srcX1, srcY1, srcX2, srcY2); }

	void		put_bitmap_trans_hmirror(int x,int y,char* bitmapPtr)
				{ IMGbltTransHMirror(buf_ptr(), buf_pitch(), x, y, bitmapPtr); }

	void		put_bitmap_trans_vmirror(int x,int y,char* bitmapPtr)
				{ IMGbltTransVMirror(buf_ptr(), buf_pitch(), x, y, bitmapPtr); }

	void		put_bitmap_trans_hvmirror(int x,int y,char* bitmapPtr)
				{ IMGbltTransHVMirror(buf_ptr(), buf_pitch(), x, y, bitmapPtr); }

	void		put_bitmap_area_trans_hmirror(int desX,int desY,char* bitmapPtr,int srcX1, int srcY1, int srcX2, int srcY2)
				{ IMGbltAreaTransHMirror(buf_ptr(), buf_pitch(), desX, desY, bitmapPtr, srcX1, srcY1, srcX2, srcY2); }

	void		put_bitmap_trans_remap(int x,int y,char* bitmapPtr,char* colorTable)
				{ IMGbltTransRemap(buf_ptr(), buf_pitch(), x, y, bitmapPtr, colorTable); }

	void		put_bitmap_area_remap(int desX,int desY,char* bitmapPtr,int srcX1, int srcY1, int srcX2, int srcY2, char *colorTable)
				{ IMGbltAreaRemap(buf_ptr(), buf_pitch(), desX, desY, bitmapPtr, srcX1, srcY1, srcX2, srcY2, colorTable); }

	void		put_bitmap_area_trans_remap(int desX,int desY,char* bitmapPtr,int srcX1, int srcY1, int srcX2, int srcY2, char *colorTable)
				{ IMGbltAreaTransRemap(buf_ptr(), buf_pitch(), desX, desY, bitmapPtr, srcX1, srcY1, srcX2, srcY2, colorTable); }

	//-------- functions with compression ---------//

	void		remap_decompress(char* desPtr, char* srcPtr, char* colorTable)
				{ IMGremapDecompress(desPtr, srcPtr, colorTable); }

	void		put_bitmap_trans_decompress(int x,int y,char* bitmapPtr)
				{ IMGbltTransDecompress(buf_ptr(), buf_pitch(), x, y, bitmapPtr); }

	void		put_bitmap_area_trans_decompress(int x,int y,char* bitmapPtr,int srcX1,int srcY1,int srcX2,int srcY2)
				{ IMGbltAreaTransDecompress(buf_ptr(), buf_pitch(), x, y, bitmapPtr, srcX1, srcY1, srcX2, srcY2); }

	void		put_bitmap_trans_decompress_hmirror(int x,int y,char* bitmapPtr)
				{ IMGbltTransDecompressHMirror(buf_ptr(), buf_pitch(), x, y, bitmapPtr); }

	void		put_bitmap_area_trans_decompress_hmirror(int x,int y,char* bitmapPtr, int srcX1, int srcY1, int srcX2, int srcY2)
				{ IMGbltAreaTransDecompressHMirror(buf_ptr(), buf_pitch(), x, y, bitmapPtr, srcX1, srcY1, srcX2, srcY2); }

	//-------- functions with compression & color remapping ---------//

	void		put_bitmap_trans_remap_decompress(int x,int y,char* bitmapPtr,char* colorTable)
				{ IMGbltTransRemapDecompress(buf_ptr(), buf_pitch(), x, y, bitmapPtr, colorTable); }

	void		put_bitmap_trans_remap_decompress_hmirror(int x,int y,char* bitmapPtr,char* colorTable)
				{ IMGbltTransRemapDecompressHMirror(buf_ptr(), buf_pitch(), x, y, bitmapPtr, colorTable); }

	void		put_bitmap_area_trans_remap_decompress(int x,int y,char* bitmapPtr, int srcX1, int srcY1, int srcX2, int srcY2, char* colorTable)
				{ IMGbltAreaTransRemapDecompress(buf_ptr(), buf_pitch(), x, y, bitmapPtr, srcX1, srcY1, srcX2, srcY2, colorTable); }

	void		put_bitmap_area_trans_remap_decompress_hmirror(int x,int y,char* bitmapPtr, int srcX1, int srcY1, int srcX2, int srcY2, char* colorTable)
				{ IMGbltAreaTransRemapDecompressHMirror(buf_ptr(), buf_pitch(), x, y, bitmapPtr, srcX1, srcY1, srcX2, srcY2, colorTable); }

	//---------- read bitmap from buffer ----------//

	void		read_bitmap(int x1,int y1,int x2,int y2,char* bitmapPtr)
				{ IMGread(buf_ptr(), buf_pitch(), x1, y1, x2, y2, bitmapPtr); }

	//---------- other assembly functions ---------//

	void		black_32x32(int x, int y)
				{ IMGblack32x32(buf_ptr(), buf_pitch(), x, y); }

	void		pixelize_32x32(int x, int y, int color)
				{ IMGpixel32x32(buf_ptr(), buf_pitch(), x, y, color); }

	void		snow_32x32(int x, int y, int randomSeed, int baseLevel)
				{ IMGsnow32x32(buf_ptr(), buf_pitch(), x, y, randomSeed, baseLevel); }

	void		explore_mask(int x, int y, char *maskBitmap, int northRow, int thisRow, int southRow)
				{ IMGexploreMask32x32(buf_ptr(), buf_pitch(), x, y, maskBitmap, northRow, thisRow, southRow); }

	void		explore_remap(int x, int y, char *remapBitmap, char **colorTableArray, int northRow, int thisRow, int southRow)
				{ IMGexploreRemap32x32(buf_ptr(), buf_pitch(), x, y, remapBitmap, colorTableArray, northRow, thisRow, southRow); }

	void		fog_remap(int x, int y, char **colorTableArray, unsigned char *northRow, unsigned char *thisRow, unsigned char *southRow)
				{ IMGfogRemap32x32( buf_ptr(), buf_pitch(), x, y, colorTableArray, northRow, thisRow, southRow); }

	// ----- color remapping functions ------//
	void		remap_bar(int x1, int y1, int x2, int y2, unsigned char *colorTable)
				{ IMGremapBar(buf_ptr(), buf_pitch(), x1, y1, x2, y2, colorTable); }

	void		remap_bitmap(int x, int y, char *bitmapPtr, unsigned char **colorTableArray)
				{ IMGremap(buf_ptr(), buf_pitch(), x, y, bitmapPtr, colorTableArray);}

	void		remap_bitmap_hmirror(int x, int y, char *bitmapPtr, unsigned char **colorTableArray)
				{ IMGremapHMirror(buf_ptr(), buf_pitch(), x, y, bitmapPtr, colorTableArray);}
  
	void		remap_bitmap_area(int x, int y, char *bitmapPtr, unsigned char **colorTableArray,
				int srcX1, int srcY1, int srcX2, int srcY2)
				{ IMGremapArea(buf_ptr(), buf_pitch(), x, y, bitmapPtr, colorTableArray, srcX1, srcY1, srcX2, srcY2); }

	void     remap_bitmap_area_hmirror(int x, int y, char *bitmapPtr, unsigned char **colorTableArray,
				int srcX1, int srcY1, int srcX2, int srcY2)
				{ IMGremapAreaHMirror(buf_ptr(), buf_pitch(), x, y, bitmapPtr, colorTableArray, srcX1, srcY1, srcX2, srcY2); }

	// --------- VgaBuf to VgaBuf copy ------------ //
	void		blt_buf( VgaBuf *srcBuf, int x1, int y1 );
	void 		blt_virtual_buf( VgaBuf *source ) { }
};

extern VgaBuf vga_front, vga_back, vga_true_front;

//--------------------------------------------//

#endif
