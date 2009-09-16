//Filename    : ODB.H
//Description : Header file of ODB.CPP object Database

#ifndef __ODB_H
#define __ODB_H

#ifndef __OFILE_H
#include <OFILE.h>
#endif


//-------- Define class Database ------------//

class Database : public File
{
private:

   struct DbfHeader
   {
      char     dbf_id;
      char     last_update[3];
      long     last_rec;
      unsigned short data_offset;
      unsigned short rec_size;
   };

   struct DbfRec
   {
      char     field_name[11];
      char     field_type;
      long     dummy[4];
      union
      {
        unsigned char_len;
        struct
        {
           char len;
           char dec;
	} num_size;
      }len_info;

      char filler[14];
   };

private:

   DbfHeader dbf_header;
   char*     rec_buf;
   long      cur_recno;
   long      last_read_recno;

   char*     dbf_buf;     	// buffer for reading in the whole dbf
   char      dbf_buf_allocated; // whether we allocated the buffer or only take from extern pointer

public:

   Database(char* =0, int=0);
   ~Database();

   void  open(char*, int=0);
   void  open_from_buf(char*);

	char* read(long=0);
	void  go(long recNo)    { cur_recno=recNo; }
	void  close();

	long  rec_count()       { return dbf_header.last_rec; }
   long  recno()           { return cur_recno; }
};

//----------------------------------------------//

extern Database db_movie_hdr, db_movie_dtl;

//----------------------------------------------//

#endif
