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
#include <file_writer.h>

FileWriter::FileWriter()
{
   this->file = NULL;
   this->ok = true;
}

FileWriter::~FileWriter()
{
   this->deinit();
}

bool FileWriter::init(File *file)
{
   this->deinit();

   if (!this->os.open(file))
      return false;

   this->file = file;
   this->ok = true;
   this->original_type = this->file->file_type;

   /* We need raw file access */
   if (this->file->file_type != File::FLAT)
      this->file->file_type = File::FLAT;

   return true;
}

void FileWriter::deinit()
{
   if (this->file == NULL)
      return;

   this->file->file_type = this->original_type;
   this->os.close();
   this->file = NULL;
}

bool FileWriter::good() const
{
   return this->ok;
}

bool FileWriter::skip(size_t len)
{
   const char *chars = "\xc0\xde\xba\xbe";

   if (!this->ok)
      return false;

   for (size_t n = 0; n < len; n++)
   {
      if (!this->write<int8_t>(chars[n & 3]))
	 break;
   }

   return this->ok;
}

bool FileWriter::write_record_size(uint16_t size)
{
   if (!this->ok)
      return false;

   if (this->original_type != File::STRUCTURED)
      return true;

   return this->write<uint16_t>(size);
}

/* vim: set ts=8 sw=3: */
