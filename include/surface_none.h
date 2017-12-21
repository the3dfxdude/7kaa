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

//Filename    : surface_none.h
//Description : Header file for dummy surfaces.

#ifndef __SURFACE_NONE_H
#define __SURFACE_NONE_H

#include <stdlib.h>

class SurfaceNone
{
private:

protected:

public:
	SurfaceNone();
	~SurfaceNone();

	char* buf_ptr()             { return NULL; }
	char* buf_ptr(int x, int y) { return NULL; }
	int   buf_pitch()           { return 0; }
	int   buf_size()            { return 0; }
	int   buf_width()           { return 0; }
	int   buf_height()          { return 0; }

	int   lock_buf();
	int   unlock_buf();

	void  set_buf_ptr(char* bufPtr) { }
	void  set_default_buf_ptr()     { }

	int   write_bmp_file(char *fileName);

	// Only use these in Vga
	void  blt_virtual_buf(SurfaceNone *src);
	int   activate_pal();
	int   is_buf_lost();
	int   restore_buf();
};

typedef SurfaceNone Surface;

#endif // __SURFACE_SDL_H
