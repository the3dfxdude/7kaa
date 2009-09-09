//Filename    : ORES.CPP
//Description : Resource library handling object

#include <string.h>

#include <ALL.H>			
#include <OSYS.H>
#include <ORES.H>

//------------ Format of RES file -------------//
//
// In the resource file, contain many data unit.
//
// <int>  = no. of data records in this RES file
//
// Index area
// <long> = offset of corresponding data unit in this file
//   .
//   .
// <long> = the last index is a virtual index which is used to calculate
//          the size of the last data unit
//
// Data area
// <char..>  = data
//
//---------------------------------------------
//
// size of data unit = next offset - current offset
//
//---------------------------------------------//


//---------- Begin of function Resource::init ----------//
//
// <char*> resName   = name of the resource file (e.g. "GIF.RES")
// <int>   readAll   = whether read all data into the buffer or read one each time
// [int]   useCommonBuf = whether use the common buffer to store the data or not
//                     (default:0)
//
void Resource::init(char* resName, int readAll, int useCommonBuf)
{
   if( init_flag )
      deinit();

   //-------------------------------------------//

   long dataSize;

   file_open(resName);

   read_all      = readAll;
   use_common_buf   = useCommonBuf;
   data_buf      = NULL;
   data_buf_size = 0 ;

   rec_count  = file_get_short();
   cur_rec_no = -1;

   //---------- Read in record index -------------//

   index_buf = (long*) mem_add( (rec_count+1) * sizeof(long) );

   // rec_count+1 is the last index pointer for calculating last record size

   file_read( index_buf, sizeof(long) * (rec_count+1) );

   //---------- Read in record data -------------//

   if( read_all )
   {
      dataSize = index_buf[rec_count] - index_buf[0];

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
//----------- End of function Resource::init -------------//


//---------- Begin of function Resource::deinit ---------//
//
void Resource::deinit()
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
			data_buf=NULL;
      }

      if( !read_all )
         file_close();

      init_flag=0;
   }
}
//----------- End of function Resource::deinit ------------//


//---------- Begin of function Resource::read ----------//
//
// Read in data from the resource file and store in an the buffer of this class
//
// The index field of the current record in the database object is
// used to locate the data in the resource file.
//
// Syntax : read(int recNo)
//
// [int] recNo = the record no of the data going to read
//               (default : current record no.)
//
// Return : <char*> data pointer
//          NULL    if the record has not index to data
//
char* Resource::read(int recNo)
{
   err_when( !init_flag );

   unsigned dataSize;

   if( recNo < 1 )
      recNo = cur_rec_no;

   err_when( recNo < 1 || recNo > rec_count );

   if( recNo < 1 || recNo > rec_count )	   // when no in debug mode, err_when() will be removed
      return NULL;

   //------ all data pre-loaded to memory ------//

   if( read_all )
      return data_buf + index_buf[recNo-1] - index_buf[0];

   //------ all data NOT pre-loaded to memory -------//

   if( recNo == cur_rec_no && !use_common_buf )
      return data_buf;

   dataSize = index_buf[recNo] - index_buf[recNo-1];

   err_when( use_common_buf && dataSize > COMMON_DATA_BUF_SIZE );

   if( !use_common_buf && data_buf_size < dataSize )
      data_buf = mem_resize( data_buf, dataSize );

   data_buf_size = dataSize;

   //------------ read data ------------//

   file_seek( index_buf[recNo-1] );

   file_read( data_buf, dataSize );

   return data_buf;
}
//----------- End of function Resource::read -------------//


//---------- Begin of function Resource::get_file ----------//
//
// Position the file pointer to the beginning of the data and
// return the file stream
//
// <int> recNo = the record no of the data going to read
//               (default : current record no.)
//
// <int&> dataSIze = for returning the data size of the resource record
//
//
// Return : <FILE*> the file stream
//          NULL    if the record has not index to data/all data read into memory
//
File* Resource::get_file(int recNo, int& dataSize)
{
   err_when( !init_flag || read_all );	 // if read_all, all data in file has been read in and the file has been closed

   err_when( recNo < 1 || recNo > rec_count );

   if( recNo < 1 || recNo > rec_count )	   // when no in debug mode, err_when() will be removed
      return NULL;

   file_seek( index_buf[recNo-1] );

   return this;
}
//----------- End of function Resource::get_file -------------//

