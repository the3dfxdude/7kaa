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
#include <stdio.h>
#include <stdlib.h>

#include <file_util.h>

bool seek(File *file, long off, int whence)
{
   long target;
   long pos;

   switch (whence)
   {
      case SEEK_SET: target = off; break;
      case SEEK_CUR: target = file->file_pos() + off; break;
      case SEEK_END: target = file->file_size() + off; break;
      default: abort();
   }

   pos = file->file_seek(off, whence);
   return (pos == target);
}
