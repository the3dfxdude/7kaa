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

//Filename    : ORESDB.CPP
//Description : Resource library with database index reading object

#include <string.h>
#include <stdint.h>
#include <OSYS.h>
#include <ODB.h>
#include <ORESDB.h>

//-------------------------------------------------------//
//
// Resource library reading object
//
// Files required :
//
// - A resource file build by LIBDB.EXE, it is always in .RES extension
// - A database file with Index Pointer field, the values of the field
//   are automatically calculated by LIBDB.EXE
//
//-------------------------------------------------------//

//---------- Begin of function ResourceDb::deinit ---------//
//
void ResourceDb::deinit()
{
   if( init_flag )
   {
      if( !use_common_buf && data_buf )
      {
			 mem_del(data_buf);
			 data_buf = NULL;
      }

      if( !read_all )
			 file_close();

      init_flag = false;
   }
}
//----------- End of function ResourceDb::deinit ----------//

//---------- Begin of function ResourceDb::init_imported ----------//
//
// If the whole database has been read into memory, then only no need to
// tell ResourceDb the database name and the index offset
//
// <char*> resName   = name of the resource file (e.g. "GIF.RES")
// <int>   readAll   = whether read all data into the buffer or read one each time
// [int]   useCommonBuf = whether use the vga common buffer to store the data or not
//                     (default:0)
//
void ResourceDb::init_imported(const char* resName, int cacheWholeFile, int useCommonBuf)
{
   deinit();

   read_all = cacheWholeFile;

   file_open( resName );

	if( read_all )
	{
		data_buf_size = file_size();

		data_buf = mem_add( data_buf_size );
		file_read( data_buf, data_buf_size );
		file_close();

		use_common_buf = 0;  // don't use vga buffer if read all
	}
	else
	{
		use_common_buf = useCommonBuf;

      if( use_common_buf )
         data_buf = sys.common_data_buf;
      else
         data_buf = NULL;
   }

   init_flag = true;
}
//----------- End of function ResourceDb::init_imported -------------//



//---------- Begin of function ResourceDb::read_imported ----------//
//
// If ResourceDb is initialized using init_imported(),
// then use read_imported to read the record
//
// <long> offset = offset to the data in the resource file
//
// Return : <char*> data pointer
//          NULL    if the record has not index to data
//
char* ResourceDb::read_imported(long offset)
{
	err_when(!init_flag);

	//-------- read from buffer ---------//
	// ##### begin Gilbert 4/10 #######//
	if( read_all )
	{
		err_when(offset < 0 || offset >= data_buf_size);
		return data_buf + offset + sizeof(int32_t);  // bypass the long parameters which is the size of the data
	}
	// ##### end Gilbert 4/10 #######//

	//---------- read from file ---------//

	// ##### begin Gilbert 2/10 ######//
	err_when(offset >= file_size());
	// ##### end Gilbert 2/10 ######//
	file_seek( offset );

	data_buf_size = file_get_long();

	err_when(use_common_buf && data_buf_size > COMMON_DATA_BUF_SIZE);

   if( !use_common_buf )
		data_buf = mem_resize( data_buf, data_buf_size );

	file_read( data_buf, data_buf_size );

   return data_buf;
}
//----------- End of function ResourceDb::read_imported -------------//
