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

// Filename    : CRC.H
// Description : header of assembly function crc.asm

#ifndef __CRC_H
#define __CRC_H

#include <windows.h>

typedef unsigned char CRC_TYPE;
const unsigned int CRC_LEN = sizeof(CRC_TYPE);

extern "C"
{
	CRC_TYPE _stdcall crc8(unsigned char *, int) asm("_crc8");
}


#endif
