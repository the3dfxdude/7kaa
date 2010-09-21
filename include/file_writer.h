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
#ifndef FILE_WRITER_H
#define FILE_WRITER_H

#include <file_output_stream.h>

class FileWriter
{
protected:
   FileOutputStream os;
   File::FileType original_type;
   File *file;
   bool ok;

public:
   FileWriter();
   ~FileWriter();
   bool init(File *file);
   void deinit();
   bool good() const;
   bool skip(size_t len);
   bool write_record_size(uint16_t size);

   template <typename FileT, typename MemT>
   bool write(MemT val)
   {
      if (!this->ok)
	 return false;

      if (!write_le<FileT>(&this->os, val))
	 this->ok = false;

      return this->ok;
   }

   template <typename T>
   bool write(const T *v)
   {
      uint32_t p;

      if (v == NULL)
	 p = 0;
      else
	 p = 0xdeadbeef;

      return this->write<uint32_t>(p);
   }

   template <typename FileT, typename MemT>
   bool write_array(const MemT *array, size_t len)
   {
      for (size_t n = 0; n < len; n++)
      {
	 if (!this->write<FileT>(array[n]))
	    break;
      }

      return this->ok;
   }
};

/* vim: set ts=8 sw=3: */
#endif
