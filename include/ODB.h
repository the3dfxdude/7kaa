/*
 * Seven Kingdoms: Ancient Adversaries
 *
 * Copyright 1997,1998 Enlight Software Ltd.
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
