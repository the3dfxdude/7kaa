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

#include <file_reader.h>
#include <file_writer.h>

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


class FileWriterVisitor
{
protected:
   FileWriter *writer;

public:
   FileWriterVisitor()
   {
      this->writer = NULL;
   }

   ~FileWriterVisitor()
   {
      this->deinit();
   }

   void init(FileWriter *writer)
   {
      this->writer = writer;
   }

   void deinit()
   {
      this->writer = NULL;
   }

   bool skip(size_t len)
   {
      return this->writer->skip(len);
   }

   template <typename FileT, typename MemT>
   bool visit(MemT *v)
   {
      return this->writer->write<FileT, MemT>(*v);
   }

   template <typename T>
   bool visit(T **v)
   {
      return this->writer->write<T>(const_cast<const T *>(*v));
   }

   template <typename FileT, typename MemT>
   bool visit_array(MemT *array, size_t len)
   {
      return this->writer->write_array<FileT, MemT>(array, len);
   }
};

namespace FileIOVisitor
{
   template <typename FileT, typename MemT, typename Visitor>
   bool visit(Visitor *vis, MemT *val)
   {
      return vis->template visit<FileT, MemT>(val);
   }

   template <typename FileT, typename MemT, typename Visitor>
   bool visit_array(Visitor *vis, MemT *array, size_t len)
   {
      return vis->template visit_array<FileT, MemT>(array, len);
   }

   template <typename T, typename Visitor>
   bool visit_pointer(Visitor *vis, T **ptr)
   {
      return vis->template visit(ptr);
   }

   template <typename T>
   bool write_with_record_size(File *file, T *obj,
			       void (*visit_obj)(FileWriterVisitor *v, T *obj),
			       uint16_t rec_size)
   {
      FileWriter w;
      FileWriterVisitor v;

      if (!w.init(file))
	 return false;

      w.write_record_size(rec_size);
      v.init(&w);
      visit_obj(&v, obj);

      return w.good();
   }

   template <typename T>
   bool read_with_record_size(File *file, T *obj,
			      void (*visit_obj)(FileReaderVisitor *v, T *obj),
			      uint16_t expected_rec_size)
   {
      FileReader r;
      FileReaderVisitor v;

      if (!r.init(file))
	 return false;

      r.check_record_size(expected_rec_size);
      v.init(&r);
      visit_obj(&v, obj);

      return r.good();
   }
} /* namespace FileIOVisitor */

/* vim: set ts=8 sw=3: */
#endif
