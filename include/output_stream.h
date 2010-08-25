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
#ifndef OUTPUT_STREAM_H
#define OUTPUT_STREAM_H

#include <stdint.h>
#include <stdio.h>

class OutputStream
{
public:
   virtual ~OutputStream() {}
   virtual long write(const void *data, long length) = 0;
   virtual bool seek(long offset, int whence) = 0;
   virtual long tell() = 0;
   virtual void close() = 0;
};

/* Writes a little-endian integer */
template <typename T>
bool write_le_integer(OutputStream *os, T val)
{
   unsigned char c;

   for (int n = 0; n < static_cast<int>(sizeof(T)); n++)
   {
      c = val >> (8 * n);

      if (os->write(&c, 1) != 1)
	 return false;
   }

   return true;
}

template <typename T>
bool write_le(OutputStream *os, T val)
{
   return write_le_integer<T>(os, val);
}

template <> bool write_le<float>(OutputStream *os, float val);
template <> bool write_le<double>(OutputStream *os, double val);

/* vim: set ts=8 sw=3: */
#endif
