#ifndef FILE_IO_VISITOR_H
#define FILE_IO_VISITOR_H

#include "file_reader.h"

class FileReaderVisitor
{
protected:
   FileReader *reader;

public:
   FileReaderVisitor(FileReader *fr)
   {
      this->reader = fr;
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
