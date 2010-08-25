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
#include <file_output_stream.h>
#include <file_util.h>

FileOutputStream::FileOutputStream()
{
   this->file = NULL;
}

FileOutputStream::~FileOutputStream()
{
   this->close();
}

long FileOutputStream::write(const void *data, long length)
{
   if (this->file == NULL)
      return 0;

   if (!this->file->file_write(const_cast<void *>(data), length))
      return 0;

   return length;
}

bool FileOutputStream::seek(long offset, int whence)
{
   if (this->file == NULL)
      return false;

   return ::seek(this->file, offset, whence);
}

long FileOutputStream::tell()
{
   if (this->file == NULL)
      return -1;

   return this->file->file_pos();
}

bool FileOutputStream::open(File *file)
{
   this->close();
   this->file = file;
   return (file != NULL);
}

void FileOutputStream::close()
{
   this->file = NULL;
}

/* vim: set ts=8 sw=3: */
