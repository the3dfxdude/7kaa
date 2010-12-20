/*
 * Seven Kingdoms: Ancient Adversaries
 *
 * Copyright 1997,1998 Enlight Software Ltd.
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

// Filename    : MPTYPES.H
// Description : Multiplayer header, define symbols


#ifndef __MPTYPES_H
#define __MPTYPES_H

#include <stdint.h>

#if defined(USE_NONETPLAY)
	#define PID_TYPE uint32_t
	#define BROADCAST_PID 0
	#define MultiPlayerType MultiPlayerNone
	#define mp_obj mp_none
	#define SessionIdType uint32_t
	#define PlayerDesc NonePlayer
#elif defined(USE_SDLNET)
	#define PID_TYPE uint32_t
	#define BROADCAST_PID 0
	#define MultiPlayerType MultiPlayerSDL
	#define mp_obj mp_sdl
	#define SessionIdType uint32_t
	#define PlayerDesc SDLPlayer
#else
	#error "A netplay backend must be specified."
#endif

#endif

