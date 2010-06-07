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

//Filename    : asmfun.h
//Description : Header file for asm compilation

#ifndef _ASMFUN_H
#define _ASMFUN_H


#if (defined(NO_ASM))

#define __asmsym__(s)
#define IMGcall

#else // ASM Requires Wine or Win32 and JWasm!!

#include <windows.h>

#define __asmsym__(s) __asm__(s)
#define IMGcall _stdcall

#endif


#endif // _ASMFUN_H
