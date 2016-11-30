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

//Filename    : ODB.DBF
//Description : Object Database handling, it read DBF files

#include <string.h>

#include <ALL.h>
#include <ODB.h>


//-------- Begin of function Database constructor ------//
//
// [char*] fileName  = the name of the DBF file to be opened
// [int]   bufferAll = read the whole database into memory or not
//
Database::Database(const char* dbFileName, int bufferAll)
{
   dbf_buf  = NULL;
   rec_buf  = NULL;

   last_read_recno = -1;

   if( dbFileName )
      open( dbFileName, bufferAll );
}
//----------- End of function Database constructor ------//


//-------- Begin of function Database destructor ------//

Database::~Database()
{
   Database::close();
}

//----------- End of function Database destructor ------//


//-------- Begin of function Database::open --------//
//
// Database::open( char* fileName )
//
// <char*> fileName  = the name of the DBF file to be opened
// [int]   bufferAll = read the whole database into memory or not
//      	       (default : 0)
//
// return 1 : opened successfully
//        0 : opening failure
//
void Database::open( const char* fileName, int bufferAll )
{
   close();        // if there is a opened file attached to current database, close it first

   file_open(fileName);
   file_read( &dbf_header, sizeof(DbfHeader) );

   //..........................................//

   if( bufferAll )      // read the whole database into memory or not
   {
      dbf_buf = mem_add( dbf_header.rec_size * dbf_header.last_rec );

      file_seek( 1 + dbf_header.data_offset );
      file_read( dbf_buf, dbf_header.rec_size*dbf_header.last_rec );
      file_close();

      dbf_buf_allocated = 1;	// we allocated the buffer
   }
   else
      rec_buf = mem_add( dbf_header.rec_size );

   cur_recno = 1;
}
//--------- End of function Database::open ---------//


//-------- Begin of function Database::open_from_buf --------//
//
// Open the database from an buffer with the database pre-read in
//
// <char*> dataPtr = the pointer to the pre-read database in the memory
//
void Database::open_from_buf(char* dataPtr)
{
   close();        // if there is a open_from_bufed file attached to current database, close it first

   //------- set data pointers ----------//

   memcpy( &dbf_header, dataPtr, sizeof(DbfHeader) );

   dbf_buf = dataPtr + 1 + dbf_header.data_offset;

   dbf_buf_allocated = 0;	// we didn't allocate the buffer, so don't bother to free it in deinit()

   cur_recno = 1;
}
//--------- End of function Database::open_from_buf ---------//


//--------- Begin of function Database::read --------//
//
// Database::read( long recNum )
//
// [recNum] = the record number of the record to be read
//	      (default : current record no.)
//
// return : <char*> success, the pointer to the reading buffer
//          <NULL>  fail
//
char* Database::read( long recNo )
{
   if( recNo <= 0 )
      recNo = cur_recno;

   if( recNo < 1 || static_cast<uint32_t>(recNo) > dbf_header.last_rec )
      return NULL;

   if( dbf_buf )        // the whole database has been read into memory
   {
      return dbf_buf + dbf_header.rec_size * (recNo-1);
   }
   else            // only a portion of the database is read into the memory at a time
   {
      if( recNo == last_read_recno )    // the record to be read is the same as one in buffer, simply return it
	 return rec_buf;

      file_seek( 1+dbf_header.data_offset + dbf_header.rec_size * (recNo-1) );
      file_read( rec_buf, dbf_header.rec_size );

      last_read_recno = recNo;

      return rec_buf;
   }
}
//----------- End of function Database::read ---------//



//---------- Begin of function Database::close -------//
//
// Database::close()
//
void Database::close()
{
   if( rec_buf )
   {
      mem_del(rec_buf);
      rec_buf = NULL;
   }

   if( dbf_buf && dbf_buf_allocated )
   {
      mem_del( dbf_buf );
      dbf_buf = NULL;
   }
}
//----------- End of function Database::close ----------//


