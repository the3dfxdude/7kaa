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
   bool good() const { return ok; }
   bool skip(size_t len);
   bool check_record_size(uint16_t expected_size);

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
};

/* vim: set ts=8 sw=3: */
#endif
