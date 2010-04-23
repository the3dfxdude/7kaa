/*
 * Seven Kingdoms: Ancient Adversaries
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
#ifndef INPUT_STREAM_H
#define INPUT_STREAM_H

#include <stdio.h>

class InputStream
{
public:
   virtual ~InputStream() {}
   virtual long read(void *buffer, long length) = 0;
   virtual bool seek(long offset, int whence) = 0;
   virtual long tell() = 0;
   virtual void close() = 0;
};

/* Reads a little-endian integer */
template <typename T>
bool read_le(InputStream *is, T *valp)
{
   T val = T();
   unsigned char c;

   for (int n = 0; n < static_cast<int>(sizeof(T)); n++)
   {
      if (!is->read(&c, 1))
	 return false;

      val |= static_cast<T>(c) << (8 * n);
   }

   *valp = val;

   return true;
}

#endif
