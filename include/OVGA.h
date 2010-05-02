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

//Filename    : OVGA.H
//Description : Header file for class OVGA (Direct Draw version)

#ifndef __OVGA_H
#define __OVGA_H

#ifndef __OVGABUF_H
#include <OVGABUF.h>
#endif

#ifndef __COLOR_H
#include <COLOR.h>
#endif

//----------- define constants ----------//

#define VGA_WIDTH             800
#define VGA_HEIGHT            600
#define VGA_BPP                 8

#define MAX_BRIGHTNESS_ADJUST_DEGREE	10

//----------- Define constant -------------//

#define IF_LIGHT_BORDER_COLOR     V_WHITE
#define IF_DARK_BORDER_COLOR		 V_BLACK
#define IF_UP_BRIGHTNESS_ADJUST	 5
#define IF_DOWN_BRIGHTNESS_ADJUST 6

//-------- Define macro functions ---------//

#define get_bitmap_width(bitmapPtr)  (*(short*)bitmapPtr)
#define get_bitmap_height(bitmapPtr) (*((short*)bitmapPtr+1))

//-------- Vga surface types ---------------//

enum vga_surface_type {
	VGA_FRONT,
	VGA_BACK
};

//-------- Define class Vga ----------------//

class ColorTable;

class Vga
{
public:
		  LPDIRECTDRAW2        dd_obj;
		  LPDIRECTDRAWPALETTE  dd_pal;

		  PALETTEENTRY game_pal[256];
		  LPPALETTEENTRY custom_pal;

		  ColorTable*			  vga_color_table;
		  unsigned char		  gray_remap_table[256];

		  static VgaBuf*		  active_buf;
		  static char			  use_back_buf;
		  static char			  opaque_flag;

public:
		  Vga();
		  ~Vga();

		  BOOL   init();
		  BOOL   init_dd();
		  BOOL   set_mode();
		  void   deinit();

		  char	is_inited() 	{ return dd_obj!=NULL; }

		  void  init_surface(VgaBuf* surface, enum vga_surface_type t);
		  BOOL  init_pal(const char* fileName);
		  void  refresh_palette();
		  void	init_gray_remap_table();

		  void   activate_pal(VgaBuf*);
		  int    set_custom_palette(char*);
		  void   free_custom_palette();
		  void   release_pal();

		  void 	d3_panel_up(int x1,int y1,int x2,int y2,int vgaFrontOnly=0,int drawBorderOnly=0);
		  void 	d3_panel_down(int x1,int y1,int x2,int y2,int vgaFrontOnly=0,int drawBorderOnly=0);
		  void	d3_panel2_up(int x1,int y1,int x2,int y2,int vgaFrontOnly=0,int drawBorderOnly=0);
		  void	d3_panel2_down(int x1,int y1,int x2,int y2,int vgaFrontOnly=0,int drawBorderOnly=0);
		  void 	separator(int x1, int y1, int x2, int y2);

		  void	adjust_brightness(int changeValue);

		  void 	use_front()	{ use_back_buf=0; active_buf = &vga_front; }
		  void 	use_back()	{ use_back_buf=1; active_buf = &vga_back;  }

		  BOOL   blt_buf(int x1, int y1, int x2, int y2, int putMouseCursor=1);

		  void 	disp_image_file(const char* fileName,int x1=0, int y1=0);
		  void 	finish_disp_image_file();

private:
		  void	init_color_table();
};

extern Vga vga;

//--------------------------------------------//

#endif
