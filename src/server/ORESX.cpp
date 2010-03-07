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

//Filename    : ORESX.CPP
//Description : Object of resource library with name index

#include <string.h>

#include <ALL.h>
#include <OSYS.h>
#include <ORESX.h>


//--------- Format of RES file ------------//
//
// In the resource file, contain many data unit.
//
// <int>  = no. of data records in this RES file
//
// Index area
// char[8] = data record name
// <long>  = pointer of corresponding data unit in this file
//   .
//   .
// char[9] = the last index is a virtual index which is used to calculate
// <long>    the size of the last data unit
//
//
// Data area
// <char..>  = data
//
//---------------------------------------------
//
// size of data unit = next offset - current offset
//
//---------------------------------------------//


//---------- Begin of function void ResourceIdx::init ---------//
//
// <char*> resName		= name of the resource file (e.g. "GIF.RES")
// <int>   readAll		= whether read all data into the buffer or read one each time
//									(default:0)
// [int]   useCommonBuf = whether use the common buffer to store the data or not
//									(default:0)
//
void ResourceIdx::init(char* resName, int readAll, int useCommonBuf)
{
   long dataSize;

   if( init_flag )	// if a resource is already opened, close it first
      deinit();

   //------------- open resource file ----------//

   file_open( resName );

   //------------ Init vars ---------------------//

   read_all      = readAll;
   use_common_buf   = useCommonBuf;
   data_buf      = NULL;
   data_buf_size = 0;

   rec_count     = file_get_short();
	cur_rec_no    = -1;

	user_data_buf 		  = NULL;
	user_data_buf_size  = 0;
	user_start_read_pos = 0;

   //---------- Read in record index -------------//

   index_buf = (ResIndex*) mem_add( (rec_count+1) * sizeof(ResIndex) );

   // rec_count+1 is the last index pointer for calculating last record size

   file_read( index_buf, sizeof(ResIndex) * (rec_count+1) );

   //---------- Read in record data -------------//

	if( read_all )
   {
      dataSize = index_buf[rec_count].pointer - index_buf[0].pointer;

      data_buf = mem_add( dataSize );

      file_read( data_buf, dataSize );
      file_close();
   }
   else
   {
      if( use_common_buf )
         data_buf = sys.common_data_buf;
   }

   init_flag = 1;
}
//----------- End of function ResourceIdx::init ------------//


//---------- Begin of function ResourceIdx::deinit ---------//
//
void ResourceIdx::deinit()
{
   if( init_flag )
   {
      if( index_buf )
      {
			mem_del(index_buf);
			index_buf = NULL;
      }

		if( data_buf && data_buf != sys.common_data_buf )
      {
			mem_del(data_buf);
			data_buf = NULL;
      }

      if( !read_all )
			file_close();

      init_flag=0;
   }
}
//----------- End of function ResourceIdx::~ResourceIdx ------------//


//---------- Begin of function ResourceIdx::read_into_user_buf ----------//
//
// Read a block of data into the buffer specified by the user.
//
// <char*> dataName    		 = name of the data going to read
// <char*> userBuf     		 = pointer to the user buffer
// <int>   userBufSize 		 = size of the user buffer.
// [int]   userStartReadPos = the starting position of the data
//										to be read into the buffer.
//										(default:0)
//
// Return : <int> 1 - succeeded
//						0 - failed
//
int ResourceIdx::read_into_user_buf(char* dataName, char* userBuf, int userBufSize, int userStartReadPos)
{
	err_when( !init_flag || !dataName );

	int indexId = get_index(dataName);

	if( !indexId )
		return 0;

	//----- set the user buffer and read into the data -----//

	set_user_buf(userBuf, userBufSize, userStartReadPos);

	int rc = get_data(indexId)!=NULL;		// ==NULL if actual data size > buffer size

	reset_user_buf();

	return rc;
}
//----------- End of function ResourceIdx::read_into_user_buf -------------//


//---------- Begin of function ResourceIdx::read ----------//
//
// Read in data from the resource file and store in an the buffer of this class
//
// The index field of the current record in the database object is
// used to locate the data in the resource file.
//
// Syntax : read(char* dataName)
//
// <char*> dataName = name of the data going to read
//
// Return : <char*> data pointer
//          NULL    if the record has not index to data
//
char* ResourceIdx::read(const char* dataName)
{
   err_when( !init_flag || !dataName );

   int indexId = get_index(dataName);

	if( indexId )
      return get_data(indexId);
   else
      return NULL;
}
//----------- End of function ResourceIdx::read -------------//


//---------- Begin of function ResourceIdx::get_index ----------//
//
// Get the index of the data in the resource file,
// and later can retrieve the data using this index by calling get_data()
//
// <char*> dataName = name of the data going to get_index
//
// Return : <int> index of the data in the resource file
//          0     if the name is not found in the resource file
//
int ResourceIdx::get_index(const char* dataName)
{
	err_when( !init_flag || !dataName );

	int i;

	for( i=0 ; i<rec_count ; i++ )
	{
		if( strcmp( index_buf[i].name, dataName ) == 0 )
			return i+1;
	}

   return 0;
}
//----------- End of function ResourceIdx::get_index -------------//


//---------- Begin of function ResourceIdx::get_data ----------//
//
// Read in data from the resource file and store in an the buffer of this class
//
// The index field of the current record in the database object is
// used to locate the data in the resource file.
//
// <int> indexId = index of the data pointing to the resource file
//
// Return : <char*> data pointer
//
char* ResourceIdx::get_data(int indexId)
{
	err_when( !init_flag );

	unsigned dataSize;

	err_when( indexId < 1 || indexId > rec_count );

	indexId--;     // make it start from 0, instead of starting from 1 for easy array accessing

	//------ all data pre-loaded to memory ------//

	if( read_all )
		return data_buf + index_buf[indexId].pointer - index_buf[0].pointer;

	//------ all data NOT pre-loaded to memory -------//

	if( indexId == cur_rec_no && !use_common_buf )
		return data_buf;

	dataSize = index_buf[indexId+1].pointer - index_buf[indexId].pointer;

	file_seek( index_buf[indexId].pointer );

	//--- if the user has custom assigned a buffer, read into that buffer ---//

	if( user_data_buf )
	{
		if( user_start_read_pos > 0 )
		{
			file_seek(user_start_read_pos,FILE_CURRENT);			// skip the width and height info
			dataSize -= user_start_read_pos;
		}

		if( dataSize > user_data_buf_size )
			return NULL;

		file_read( user_data_buf, dataSize );

		return user_data_buf;
	}
	else
	{
		err_when( use_common_buf && dataSize > COMMON_DATA_BUF_SIZE );

		if( !use_common_buf && data_buf_size < dataSize )
			data_buf = mem_resize( data_buf, dataSize );

		data_buf_size = dataSize;

		file_read( data_buf, dataSize );

		return data_buf;
	}
}
//----------- End of function ResourceIdx::get_data -------------//


//---------- Begin of function ResourceIdx::get_file ----------//
//
// Position the file pointer to the beginning of the data and
// return the file stream
//
// Syntax : get_file(char* dataName)
//
// <char*> dataName = name of the data going to read
// <int&>  dataSIze = for returning the data size of the resource record
//
// Return : <FILE*> the file stream
//          NULL    if the record has not index to data
//
File* ResourceIdx::get_file(const char* dataName, int& dataSize)
{
   err_when( !init_flag || !dataName || read_all);

   int i;

   //-------- Search for data name ----------//

	for( i=0 ; i<rec_count ; i++ )
   {
      if( strcmp( index_buf[i].name, dataName ) == 0 )
      {
	 file_seek( index_buf[i].pointer );

	 dataSize = index_buf[i+1].pointer - index_buf[i].pointer;

	 return this;
      }
   }

   return NULL;
}
//----------- End of function ResourceIdx::get_file -------------//


//---------- Begin of function ResourceIdx::get_file ----------//
//
// Position the file pointer to the beginning of the data and
// return the file stream
//
// Syntax : get_file(char* dataName)
//
// <char*> dataName = name of the data going to read
// <int&>  dataSIze = for returning the data size of the resource record
//
// Return : <FILE*> the file stream
//          NULL    if the record has not index to data
//
File* ResourceIdx::get_file(int bitmapId, int& dataSize)
{
   err_when( !init_flag || !bitmapId || read_all );

   //-------- Search for data name ----------//

	file_seek( index_buf[bitmapId-1].pointer );

	dataSize = index_buf[bitmapId].pointer - index_buf[bitmapId-1].pointer;

	return this;
}
//----------- End of function ResourceIdx::get_file -------------//


//---------- Begin of function ResourceIdx::set_user_buf ----------//
//
// Set custom user data buffer. When it is set, data will be loaded
// into the user buffer instead of ResourceIdx's own buffer.
//
// <char*> userDataBuf 		 - pointer user data buffer
// <int>   bufSize     		 - size of the data bufferr
// <int>   userStartReadPos - the starting position of the data
//										to be read into the buffer.
//
void ResourceIdx::set_user_buf(char* userDataBuf, int bufSize, int userStartReadPos)
{
	user_data_buf 		  = userDataBuf;
	user_data_buf_size  = bufSize;
	user_start_read_pos = userStartReadPos;
}
//----------- End of function ResourceIdx::set_user_buf -------------//


//---------- Begin of function ResourceIdx::reset_user_buf ----------//
//
// Cancel using custom user data buffer.
//
void ResourceIdx::reset_user_buf()
{
	user_data_buf 		 = NULL;
	user_data_buf_size = 0;
}
//----------- End of function ResourceIdx::reset_user_buf -------------//


//---------- Begin of function ResourceIdx::data_name ----------//
char *ResourceIdx::data_name(int i)
{
	if( i < 1 || i > rec_count)
		return NULL;

	return( index_buf[i-1].name );
}
//---------- End of function ResourceIdx::data_name ----------//
