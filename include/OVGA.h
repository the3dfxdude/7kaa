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

//Filename    : OVGA.H
//Description : Video display management class

#ifndef __OVGA_H
#define __OVGA_H

#include <COLOR.h>

#if defined(USE_SDLVIDEO)
#include <vga_sdl.h>
typedef VgaSDL Vga;
#elif defined(USE_NOVIDEO)
#include <vga_none.h>
typedef VgaNone Vga;
#else
#error "A video backend must be specified."
#endif

#define VGA_WIDTH             800
#define VGA_HEIGHT            600

#define MAX_BRIGHTNESS_ADJUST_DEGREE 10

#define IF_LIGHT_BORDER_COLOR     V_WHITE
#define IF_DARK_BORDER_COLOR      V_BLACK
#define IF_UP_BRIGHTNESS_ADJUST   5
#define IF_DOWN_BRIGHTNESS_ADJUST 6

//-------- Define macro functions ---------//

#define get_bitmap_width(bitmapPtr)  (*(short*)bitmapPtr)
#define get_bitmap_height(bitmapPtr) (*((short*)bitmapPtr+1))


extern Vga vga;

#endif
