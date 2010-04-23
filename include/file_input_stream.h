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
#ifndef FILE_INPUT_STREAM_H
#define FILE_INPUT_STREAM_H

#include <OFILE.h>
#include <input_stream.h>

class FileInputStream: public InputStream
{
private:
   File *file;

public:
   FileInputStream();
   ~FileInputStream();
   bool open(File *file);
   bool open(const char *file_name);
   long read(void *buffer, long length);
   bool seek(long offset, int whence);
   long tell();
   void close();
};

#endif
