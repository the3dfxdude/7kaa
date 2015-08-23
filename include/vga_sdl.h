/*
 * Seven Kingdoms: Ancient Adversaries
 *
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

//Filename    : vga_sdl.h
//Description : Header file for class OVGA (SDL version)

#ifndef __VGA_SDL_H
#define __VGA_SDL_H

#include <OCOLTBL.h>
#include <vga_base.h>

#include <SDL.h>

#define VGA_PALETTE_SIZE 256

//-------- Define class VgaSDL ----------------//

class VgaSDL : public VgaBase
{
private:
	SDL_Window*    window;
	SDL_Renderer*  renderer;
	SDL_Texture*   texture;
	SDL_Surface*   front;
	SDL_Surface*   target;
	SDL_Color      game_pal[VGA_PALETTE_SIZE];
	SDL_Color*     custom_pal;

public:
	ColorTable*    vga_color_table;
	unsigned char  gray_remap_table[VGA_PALETTE_SIZE];

public:
	VgaSDL();
	~VgaSDL();

	int    init();
	void   deinit();

	char   is_inited()  { return front != NULL; }

	int    set_custom_palette(char*);
	void   free_custom_palette();
	void   adjust_brightness(int changeValue);

	void   handle_messages();
	void   flag_redraw();
	int    is_full_screen();
	int    is_input_grabbed();
	void   set_full_screen_mode(int mode);
	void   set_window_grab(int mode);
	void   flip();

private:
	int    init_front(VgaBuf*);
	int    init_back(VgaBuf*, unsigned long =0, unsigned long =0);
	int    init_pal(const char* fileName);
	void   activate_pal(VgaBuf*);
	void   refresh_palette();

};

typedef VgaSDL Vga;

//--------------------------------------------//

#endif
