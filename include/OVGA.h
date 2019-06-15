/*
 * Seven Kingdoms: Ancient Adversaries
 *
 * Copyright 1997,1998 Enlight Software Ltd.
 * Copyright 2010,2015 Jesse Allen
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
//Description : VGA management class (SDL version)

#ifndef __OVGA_H
#define __OVGA_H

#include <OVGABUF.h>
#include <COLOR.h>
#include <SDL.h>

//----------- define constants ----------//

#define VGA_WIDTH             800
#define VGA_HEIGHT            600
#define VGA_BPP                 8
#define VGA_PALETTE_SIZE      256

#define MAX_BRIGHTNESS_ADJUST_DEGREE 10

#define IF_LIGHT_BORDER_COLOR     V_WHITE
#define IF_DARK_BORDER_COLOR      V_BLACK
#define IF_UP_BRIGHTNESS_ADJUST   5
#define IF_DOWN_BRIGHTNESS_ADJUST 6

//-------- Define macro functions ---------//

#define get_bitmap_width(bitmapPtr)  (*(short*)bitmapPtr)
#define get_bitmap_height(bitmapPtr) (*((short*)bitmapPtr+1))

//-------- Define modes --------//

enum MouseInputMode
{
	MOUSE_INPUT_ABS,
	MOUSE_INPUT_REL,
	MOUSE_INPUT_REL_WARP,
};

enum WinGrab
{
	WINGRAB_OFF,
	WINGRAB_ON,
	WINGRAB_TOGGLE,
	WINGRAB_FORCE,
	WINGRAB_RESTORE,
};

//-------- Define class Vga ----------------//

class ColorTable;

class Vga
{
private:
	SDL_Window*    window;
	SDL_Renderer*  renderer;
	SDL_Texture*   texture;
	SDL_Surface*   target;
	SDL_Color      game_pal[VGA_PALETTE_SIZE];
	SDL_Color*     custom_pal;

	int win_grab_forced;
	int win_grab_user_mode;
	int bound_x1, bound_y1, bound_x2, bound_y2;
	int boundary_set;

public:
	ColorTable*    vga_color_table;
	unsigned char  gray_remap_table[VGA_PALETTE_SIZE];

	static VgaBuf* active_buf;
	static char    use_back_buf;
	static char    opaque_flag;

	MouseInputMode mouse_mode;

public:
	Vga();
	~Vga();

	int    init();
	void   deinit();

	char   is_inited()  { return window != NULL; }

	int    load_pal(const char* fileName);
	void   activate_pal(VgaBuf*);

	int    set_custom_palette(char*);
	void   free_custom_palette();
	void   adjust_brightness(int changeValue);

	void use_front() { use_back_buf=0; active_buf = &vga_front; }
	void use_back()  { use_back_buf=1; active_buf = &vga_back;  }

	void   handle_messages();
	void   flag_redraw();
	int    is_full_screen();
	int    is_input_grabbed();
	void   update_boundary();
	void   update_mouse_pos();
	void   set_full_screen_mode(int mode);
	void   set_mouse_mode(MouseInputMode mode);
	void   set_window_grab(WinGrab mode);
	void   flip();
	void   save_status_report();

private:
	void   get_window_scale(float *xscale, float *yscale);
};

extern Vga vga;

//--------------------------------------------//

#endif
