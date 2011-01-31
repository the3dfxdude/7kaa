/*
 * Seven Kingdoms: Ancient Adversaries
 *
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

//Filename    : vga_none.h
//Description : Header file for class OVGA (Empty version)

#ifndef __VGA_NONE_H
#define __VGA_NONE_H

#include <OCOLTBL.h>
#include <vga_base.h>

//-------- Define class VgaNone ----------------//

class VgaNone : public VgaBase
{
private:

public:
	ColorTable*    vga_color_table;
	unsigned char  gray_remap_table[256];

public:
	VgaNone();
	~VgaNone();

	int    init();
	void   deinit();

	char   is_inited()  { return 0; }

	int    set_custom_palette(char*);
	void   free_custom_palette();
	void   adjust_brightness(int changeValue);

	void   handle_messages();
	void   flag_redraw();
	void   toggle_full_screen();
};

typedef VgaNone Vga;

//--------------------------------------------//

#endif
