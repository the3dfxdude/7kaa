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

//Filename    : surface_ddraw.h
//Description : Header file for a Direct Draw Surface.

#ifndef __SURFACE_DDRAW_H
#define __SURFACE_DDRAW_H

#include <windows.h>
#include <ddraw.h>

class SurfaceDDraw
{
private:
	DDSURFACEDESC        buf_des;
	char*                cur_buf_ptr;
	int                  buf_locked;

protected:
	LPDIRECTDRAWSURFACE2 dd_buf;

public:
	SurfaceDDraw(LPDIRECTDRAWSURFACE2 surface);
	~SurfaceDDraw();

	char* buf_ptr()             { return cur_buf_ptr; }
	char* buf_ptr(int x, int y) { return cur_buf_ptr + buf_des.lPitch*y + x; }
	int   buf_pitch()           { return buf_des.lPitch; }
	int   buf_size()            { return buf_des.dwWidth * buf_des.dwHeight; }
	int   buf_width()           { return buf_des.dwWidth; }
	int   buf_height()          { return buf_des.dwHeight; }

	int   lock_buf();
	int   unlock_buf();

	void  set_buf_ptr(char* bufPtr) { cur_buf_ptr = bufPtr; }
	void  set_default_buf_ptr()     { cur_buf_ptr = (char*)buf_des.lpSurface; }

	int   write_bmp_file(char *fileName);
	void  blt_virtual_buf(SurfaceDDraw *src);

	// Only use these in Vga or VgaBuf or SurfaceDDraw
	int   activate_pal(LPDIRECTDRAWPALETTE ddPalPtr);
        int   is_buf_lost();
        int   restore_buf();
};

typedef SurfaceDDraw Surface;

#endif // __SURFACE_DDRAW_H
