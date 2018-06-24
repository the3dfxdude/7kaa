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

//Filename    : surface.h
//Description : Registers the selected backend for the surface class


#ifndef __SURFACE_H
#define __SURFACE_H

#if defined(USE_SDLVIDEO)
#include <surface_sdl.h>
#elif defined(USE_NOVIDEO)
#include <surface_none.h>
#else
#error "A video backend must be specified."
#endif

#endif // __SURFACE_H
