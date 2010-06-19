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

//Filename    : surface_sdl.h
//Description : Header file for an SDL Surface.

#ifndef __SURFACE_SDL_H
#define __SURFACE_SDL_H

#include <stdlib.h>
#include <SDL/SDL.h>

class SurfaceSDL
{
private:
	SDL_Surface *surface;
	void *save_buf;

protected:

public:
	SurfaceSDL(SDL_Surface *s);
	~SurfaceSDL();

	char* buf_ptr()             { return (char *)surface->pixels; }
	char* buf_ptr(int x, int y) { return (char *)surface->pixels + surface->pitch * y + x;  }
	int   buf_pitch()           { return surface->pitch; }
	int   buf_size()            { return surface->h * surface->w; }
	int   buf_width()           { return surface->w; }
	int   buf_height()          { return surface->h; }

	int   lock_buf();
	int   unlock_buf();

	void  set_buf_ptr(char* bufPtr) { if (!save_buf) { save_buf = surface->pixels; surface->pixels = bufPtr; } }
	void  set_default_buf_ptr()     { if (save_buf) { surface->pixels = save_buf; save_buf = NULL; } }

	int   write_bmp_file(char *fileName);

	// Only use these in VgaSDL
	void  blt_virtual_buf(SurfaceSDL *src);
	int   activate_pal(SDL_Color *pal, int firstcolor, int ncolors);
        int   is_buf_lost();
        int   restore_buf();
};

typedef SurfaceSDL Surface;

#endif // __SURFACE_SDL_H
