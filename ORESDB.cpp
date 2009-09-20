//Filename    : ORESDB.CPP
//Description : Resource library with database index reading object

#include <string.h>
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


//---------- Begin of function ResourceDb::init ---------//
//
// <char*>     resName			= name of the resource file (e.g. "GIF.RES")
// <Database*> dbObj				= name of the database      (e.g. Database db("PFILE.DBF"))
// <int>       indexOffset		= offset of the index field
// [int]       useCommonBuf   = whether use the vga common buffer to store the data or not
//										  (default:0)
//
void ResourceDb::init(char* resName, Database* dbObj, int indexOffset, int useCommonBuf)
{
   deinit();

   file_open( resName );

   db_obj             = dbObj;
   index_field_offset = indexOffset;
   use_common_buf     = useCommonBuf;

   if( use_common_buf )
      data_buf = sys.common_data_buf;
   else
      data_buf = NULL;

   err_if( db_obj == NULL )
      err_now("db_obj is NULL");

   init_flag = 1;
}
//----------- End of function ResourceDb::init ------------//


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

      init_flag=0;
   }
}
//----------- End of function ResourceDb::deinit ----------//


//---------- Begin of function ResourceDb::read ----------//
//
// Read in data from the resource file and store in an the buffer of this class
//
// The index field of the current record in the database object is
// used to locate the data in the resource file.
//
// Syntax : read(int recNo)
//
// [int] recNo = the record no. in the database.
//               (default : current record no.)
//
// Return : <char*> data pointer
//          NULL    if the record has not index to data
//
char* ResourceDb::read(int recNo)
{
   err_when( !init_flag || !db_obj );

   long* indexFieldPtr;
	char* recPtr;

   if( (recPtr = db_obj->read(recNo)) == NULL )
      return NULL;

   indexFieldPtr = (long*) (recPtr+index_field_offset);

   if( memcmp( indexFieldPtr, "    ", 4 ) == 0 )
      return NULL;      // no sample screen for this file

   file_seek( *indexFieldPtr );
	data_buf_size = file_get_long();

	err_when( use_common_buf && data_buf_size > COMMON_DATA_BUF_SIZE );

   if( !use_common_buf )
		data_buf = mem_resize( data_buf, data_buf_size );

	file_read( data_buf, data_buf_size );

   return data_buf;
}
//----------- End of function ResourceDb::read -------------//


//---------- Begin of function ResourceDb::get_file ----------//
//
// Position the file pointer to the beginning of the data and
// return the file stream
//
// Syntax : get_file()
//
// Return : <FILE*> the file stream
//          NULL    if the record has not index to data
//
File* ResourceDb::get_file()
{
   err_when( !init_flag || !db_obj );

   long* indexFieldPtr;
   char  emptyField[] = "        ";
   char* recPtr;

   if( (recPtr = db_obj->read()) == NULL )
      return NULL;

   indexFieldPtr = (long*) (recPtr+index_field_offset);

   if( memcmp( indexFieldPtr, emptyField, 8 ) == 0 )
      return NULL;      // no sample screen for this file

   file_seek( *indexFieldPtr + sizeof(long) );

   return this;
}
//----------- End of function ResourceDb::get_file -------------//


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
void ResourceDb::init_imported(char* resName, int readAll, int useCommonBuf)
{
   deinit();

   db_obj             = NULL;
   index_field_offset = NULL;

   read_all = readAll;

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

   init_flag = 1;
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
	err_when( !init_flag );

	// #### begin Gilbert 4/10 ######//
//	err_if( offset<0 || offset>=data_buf_size )
//		err_here();
	// #### end Gilbert 4/10 ######//

	//-------- read from buffer ---------//

	// ##### begin Gilbert 4/10 #######//
	if( read_all )
	{
		err_when( offset<0 || offset>=data_buf_size );
		return data_buf + offset + sizeof(long);  // by pass the long parameters which is the size of the data
	}
	// ##### end Gilbert 4/10 #######//

	//---------- read from file ---------//

	// ##### begin Gilbert 2/10 ######//
	err_when( offset >= file_size() );
	// ##### end Gilbert 2/10 ######//
	file_seek( offset );

	data_buf_size = file_get_long();

	err_when( use_common_buf && data_buf_size > COMMON_DATA_BUF_SIZE);

   if( !use_common_buf )
		data_buf = mem_resize( data_buf, data_buf_size );

	file_read( data_buf, data_buf_size );

   return data_buf;
}
//----------- End of function ResourceDb::read_imported -------------//

