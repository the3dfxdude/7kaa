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


#ifndef __SURFACE_BASE_H
#define __SURFACE_BASE_H

class SurfaceBase
{
	virtual char* buf_ptr() =0;
	virtual char* buf_ptr(int x, int y) =0;
	virtual int   buf_pitch() =0;
	virtual int   buf_size() =0;
	virtual int   buf_width() =0;
	virtual int   buf_height() =0;

	// Drawing routines
	virtual void  blt(Surface *src, int x = 0, int y = 0) =0;
};

#endif // __SURFACE_BASE_H
