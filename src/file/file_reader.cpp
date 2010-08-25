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
#include <file_reader.h>

FileReader::FileReader()
{
   this->file = NULL;
   this->ok = true;
}

FileReader::~FileReader()
{
   this->deinit();
}

bool FileReader::init(File *file)
{
   this->deinit();

   if (!this->is.open(file, false))
      return false;

   this->file = file;
   this->ok = true;
   this->original_type = this->file->file_type;

   /* We need raw file access */
   if (this->file->file_type != File::FLAT)
      this->file->file_type = File::FLAT;

   return true;
}

void FileReader::deinit()
{
   if (this->file == NULL)
      return;

   this->file->file_type = this->original_type;
   this->is.close();
   this->file = NULL;
}

bool FileReader::good() const
{
   return this->ok;
}

bool FileReader::skip(size_t len)
{
   if (!this->ok)
      return false;

   if (!this->is.seek(len, SEEK_CUR))
      this->ok = false;

   return this->ok;
}

/* vim: set ts=8 sw=3: */
