/*
 * Seven Kingdoms: Ancient Adversaries
 * File utilities
 *
 * Copyright 2010 Unavowed <unavowed@vexillium.org>
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

#ifndef FILE_UTIL_H
#define FILE_UTIL_H

#include <OFILE.h>

/* Reads a little-endian integer */
template <typename T>
bool read_le(File *file, T *valp)
{
	T val = T();
	unsigned char c;

	for (int n = 0; n < sizeof(T); n++)
	{
		if (!file->file_read(&c, 1))
			return false;

		val |= static_cast<T>(c) << (8 * n);
	}

	return true;
}

bool seek(File *file, long off, int whence);

#endif
