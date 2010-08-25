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
#ifndef FILE_IO_VISITOR_H
#define FILE_IO_VISITOR_H

#include "file_reader.h"

class FileReaderVisitor
{
protected:
   FileReader *reader;

public:
   FileReaderVisitor()
   {
      this->reader = NULL;
   }

   ~FileReaderVisitor()
   {
      this->deinit();
   }

   void init(FileReader *reader)
   {
      this->reader = reader;
   }

   void deinit()
   {
      this->reader = NULL;
   }

   bool skip(size_t len)
   {
      return this->reader->skip(len);
   }

   template <typename FileT, typename MemT>
   bool visit(MemT *v)
   {
      return this->reader->read<FileT, MemT>(v);
   }

   template <typename T>
   bool visit(T **v)
   {
      return this->reader->read<T>(v);
   }

   template <typename FileT, typename MemT>
   bool visit_array(MemT *array, size_t len)
   {
      return this->reader->read_array<FileT, MemT>(array, len);
   }
};

/* vim: set ts=8 sw=3: */
#endif
