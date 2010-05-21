/*
 * Seven Kingdoms: Ancient Adversaries
 *
 * Copyright 1997,1998 Enlight Software Ltd.
 * Copyright 2010 Jesse Allen
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

//Filename    : vga_ddraw.h
//Description : Header file for class OVGA (Direct Draw version)

#ifndef __VGA_DDRAW_H
#define __VGA_DDRAW_H

#include <vga_base.h>

//-------- Define class VgaDDraw ----------------//

class ColorTable;

class VgaDDraw : public VgaBase
{
public:
	LPDIRECTDRAW2        dd_obj;
	LPDIRECTDRAWPALETTE  dd_pal;

	PALETTEENTRY   game_pal[256];
	LPPALETTEENTRY custom_pal;

	ColorTable*    vga_color_table;
	unsigned char  gray_remap_table[256];

public:
	VgaDDraw();
	~VgaDDraw();

	BOOL   init();
	void   deinit();

	char   is_inited()  { return dd_obj!=NULL; }

	void   init_surface(VgaBuf* surface, enum vga_surface_type t);
	BOOL   init_pal(const char* fileName);
	void   refresh_palette();

	void   activate_pal(VgaBuf*);
	int    set_custom_palette(char*);
	void   free_custom_palette();
	void   adjust_brightness(int changeValue);

private:
	BOOL   init_dd();
	void   init_gray_remap_table();
        void   init_color_table();
	BOOL   set_mode();
	void   release_pal();
};

typedef VgaDDraw Vga;

//--------------------------------------------//

#endif
