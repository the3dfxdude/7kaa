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
#ifndef FILE_READER_H
#define FILE_READER_H

#include <file_input_stream.h>

class FileReader
{
protected:
   FileInputStream is;
   File::FileType original_type;
   File *file;
   bool ok;

public:
   FileReader();
   ~FileReader();
   bool init(File *file);
   void deinit();
   bool read(void *buf, size_t len);
   bool good() const;
   bool skip(size_t len);

   template <typename FileT, typename MemT>
   bool read(MemT *v)
   {
      FileT val;

      if (!this->ok)
	 return false;

      if (read_le(&this->is, &val))
	 *v = val;
      else
	 this->ok = false;

      return this->ok;
   }

   template <typename T>
   bool read(T **v)
   {
      uint32_t p;

      if (!this->read<uint32_t>(&p))
	 return false;

      if (p != 0)
	 *v = reinterpret_cast<T *>(uintptr_t(0xdeadbeefUL));
      else
	 *v = NULL;

      return true;
   }

   template <typename FileT, typename MemT>
   bool read_array(MemT *array, size_t len)
   {
      for (size_t n = 0; n < len; n++)
      {
	 if (!this->read<FileT>(&array[n]))
	    break;
      }

      return this->ok;
   }
};

/* vim: set ts=8 sw=3: */
#endif
