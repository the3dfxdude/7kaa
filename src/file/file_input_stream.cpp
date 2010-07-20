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
#include <file_input_stream.h>
#include <file_util.h>

FileInputStream::FileInputStream()
{
   this->file = NULL;
   this->own_file = false;
}

FileInputStream::~FileInputStream()
{
   this->close();
}

long FileInputStream::read(void *buffer, long length)
{
   if (this->file == NULL)
      return 0;

   if (!this->file->file_read(buffer, length))
		return 0;

	return length;
}

bool FileInputStream::seek(long offset, int whence)
{
   if (this->file == NULL)
      return false;

   return ::seek(this->file, offset, whence);
}

long FileInputStream::tell()
{
   if (this->file == NULL)
      return -1;

   return this->file->file_pos();
}

bool FileInputStream::open(File *file, bool own_file)
{
   this->close();
   this->file = file;
   this->own_file = own_file;
   return (file != NULL);
}

bool FileInputStream::open(const char *file_name)
{
   this->open(new File);

   if (!this->file->file_open(file_name))
   {
      this->close();
      return false;
   }

   return true;
}

void FileInputStream::close()
{
   if (this->own_file)
      delete this->file;

   this->file = NULL;
   this->own_file = false;
}
