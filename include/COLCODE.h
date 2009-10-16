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

// Filename    : COLCODE.H
// Description : C include file for defining special color
//               see/update also COLCODE.INC for assembly

// for transparent code repeated for 1 to UNIQUE_REPEAT_CODE_NUM times,
// write FEW_TRANSPARENT_CODE(repeated_times)
// for transparent code repeated for UNIQUE_REPEAT_CODE_NUM+1 to 255 times,
// write two bytes, MANY_TRANSPARENT_CODE and repeated_times

#define TRANSPARENT_CODE			255
#define UNIQUE_REPEAT_CODE_NUM	  7		// total no. of bytes used by transparent pixels and compressed transparent pixels is 7+1 (the last 1 is the first byte of the 2 bytes compression code)
#define FEW_TRANSPARENT_CODE(n) (0xFF-n+1)
#define MANY_TRANSPARENT_CODE   0xf8
#define MIN_TRANSPARENT_CODE    0xf8
#define MAX_TRANSPARENT_CODE    0xff

#define SHADOW_CODE      		  0x00
#define OUTLINE_CODE            0xf2
#define OUTLINE_SHADOW_CODE     0xf3
