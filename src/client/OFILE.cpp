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

//Filename    : OFILE.CPP
//Description : Object File

#include <windows.h>
#include <stdio.h>
#include <OBOX.h>
#include <ALL.h>
#include <OFILE.h>

//--------- Define static variables -----------//

static const char *path_array[] = { "" };	// multiple search path

//-------- Begin of function File::file_open ----------//
//
// Open an existing file for reading
// If the file is not found, call err.run()
//
// <char*> fileName      = name of the file
// [int]   handleError   = handle error
//                         ( if 1, when error happen, err.run() will be called
//                           immediately )
//                         ( default : 1 )
// [int]   allowVarySize = allow the writing size and the read size to be different
//                         ( default : 0 )
//
// return : 1-success, 0-fail
//
int File::file_open(const char* fileName, int handleError, int allowVarySize)
{
	if( strlen(fileName) > MAX_PATH )
      err.run( "File : file name is too long." );

   if( file_handle != 0 )
      file_close();

   strcpy( file_name, fileName );

   handle_error    = handleError;
   allow_vary_size = allowVarySize;

   //--------- search all path to open the file ------//

   for( int i=0 ; i<sizeof(path_array)/sizeof(path_array[0]) ; i++ )
   {
      char    filePath[MAX_PATH];
      strcpy( filePath, path_array[i] );
      strcat( filePath, fileName );

      /*
       * This replaces
       *
       * file_handle = CreateFile(filePath,
			   GENERIC_READ,
			   FILE_SHARE_READ,
			   (LPSECURITY_ATTRIBUTES) NULL,
			   OPEN_EXISTING,
			   FILE_ATTRIBUTE_NORMAL,
			   (HANDLE) NULL);
      */
      file_handle = fopen(filePath, "rb");

      if( file_handle != 0)
			return TRUE;
   }

	err.run( "Error opening file %s.", file_name );

   return FALSE;
}
//---------- End of function File::file_open ----------//


//-------- Begin of function File::file_create ----------//
//
// Create a new file for writing (reading is also permitted)
//
// <char*> fileName      = name of the file
// [int]   handleError   = handle error
//                         ( if 1, when error happen, err.run() will be called
//                           immediately )
//                         ( default : 1 )
// [int]   allowVarySize = allow the writing size and the read size to be different
//                         ( default : 0 )
//
//
// return : 1-success, 0-fail
//
int File::file_create(const char* fileName, int handleError, int allowVarySize)
{
   if( strlen(fileName) > MAX_PATH )
      err.run( "File : file name is too long." );

   strcpy( file_name, fileName );

   handle_error    = handleError;
   allow_vary_size = allowVarySize;

   // cannot use creat() because it can't specify Binary file type

   file_handle = fopen(fileName, "wb+");
   /* 
    * This replaces
    *
    file_handle = CreateFile(fileName,
			   GENERIC_WRITE,
			   0,
			   (LPSECURITY_ATTRIBUTES) NULL,
			   CREATE_ALWAYS,
			   FILE_ATTRIBUTE_NORMAL,
			   (HANDLE) NULL);
   */

   if( file_handle == 0)
		err.run( "Error creating file %s", file_name );

   return TRUE;
}
//---------- End of function File::file_create ----------//

//### begin alex 24/7 ###//
//-------- Begin of function File::file_append ----------//
//
// Open an existing file for reading
// If the file is not found, call err.run()
//
// <char*> fileName      = name of the file
// [int]   handleError   = handle error
//                         ( if 1, when error happen, err.run() will be called
//                           immediately )
//                         ( default : 1 )
// [int]   allowVarySize = allow the writing size and the read size to be different
//                         ( default : 0 )
//
// return : 1-success, 0-fail
//
int File::file_append(const char* fileName, int handleError, int allowVarySize)
{
	if( strlen(fileName) > MAX_PATH )
      err.run( "File : file name is too long." );

   if( file_handle != 0 )
      file_close();

   strcpy( file_name, fileName );

   handle_error    = handleError;
   allow_vary_size = allowVarySize;

   //--------- search all path to open the file ------//

   for( int i=0 ; i<sizeof(path_array)/sizeof(path_array[0]) ; i++ )
   {
      char    filePath[MAX_PATH];
      strcpy( filePath, path_array[i] );
      strcat( filePath, fileName );

      file_handle = fopen(filePath, "ab+");
      /*         
       * This replaces
       *
       file_handle = CreateFile(filePath,
			   GENERIC_WRITE,
			   FILE_SHARE_WRITE,
			   (LPSECURITY_ATTRIBUTES) NULL,
			   OPEN_EXISTING,
			   FILE_ATTRIBUTE_NORMAL,
			   (HANDLE) NULL);
      */

      if( file_handle != 0)
			return TRUE;
   }

	err.run( "Error opening file %s.", file_name );

   return FALSE;
}
//---------- End of function File::file_append ----------//
//#### end alex 24/7 ####//

//-------- Begin of function File::file_close ----------//
//
void File::file_close()
{
   if( file_handle != 0 )
   {
      fclose(file_handle);
      file_handle = 0;
   }
}
//---------- End of function File::file_close ----------//


//-------- Begin of function File::~File ----------//
//
File::~File()
{
   file_close();
}
//---------- End of function File::~File ----------//


//-------- Begin of function File::file_write ----------//
//
// Write a block of data to the file
//
// <void*>    dataBuf = pointer to data buffer to be written to the file
// <unsigned> dataSize = length of the data (must < 64K)
//
// return : 1-success, 0-fail
//
int File::file_write(void* dataBuf, unsigned dataSize)
{
   err_when( file_handle == 0 );       // not initialized

   if( allow_vary_size )        // allow the writing size and the read size to be different
   {
      if( dataSize > 0xFFFF )
         file_put_unsigned_short(0);    // if exceed the unsigned short limit, don't write it
      else
         file_put_unsigned_short( dataSize );
   }


   fwrite(dataBuf, 1, dataSize, file_handle);
  /*    
   * This replaces
   *
   int rc = WriteFile(  file_handle, dataBuf, dataSize, &actualWritten, NULL); 
   */
		
   if( ferror(file_handle) && handle_error )
      err.run( "Error writing file %s", file_name );

   return ferror(file_handle) == 0? 1 : 0;
}
//---------- End of function File::file_write ----------//


//-------- Begin of function File::file_read ----------//
//
// Read a block of data from the file
//
// <void*>    dataBuf = pointer to data buffer to be written to the file
// <unsigned> dataSize = length of the data (must < 64K)
//
// return : 1-success, 0-fail
//
int File::file_read(void* dataBuf, unsigned dataSize)
{
	#define MAX_READ_SIZE 0xFFF0

	err_when( file_handle == 0 );       // not initialized

	int curPos=file_pos();

  start_read:

   unsigned readSize=dataSize, writtenSize=dataSize;

   if( allow_vary_size )        // allow the writing size and the read size to be different
   {
      writtenSize = file_get_unsigned_short();

      if( writtenSize )    // writtenSize==0, if the size > 0xFFFF
         readSize = min(dataSize, writtenSize);  // the read size is the minimum of the written size and the supposed read size
   }


   fread(dataBuf, 1, dataSize, file_handle);
   /*
    * This replaces
    *
   int rc=ReadFile( file_handle, dataBuf, dataSize, &actualRead, NULL);
    */		

   //-------- if the data size has been reduced ----------//

   if( readSize < writtenSize )
      file_seek( writtenSize-readSize, FILE_CURRENT );

   //---- if the data size has been increased, reset the unread area ---//

   if( readSize < dataSize )
      memset( (char*)dataBuf+readSize, 0, dataSize-readSize );

   //----- if reading error, popup box and ask for retry -----//

	if( ferror(file_handle) && handle_error )
	{
		char msgStr[100];

		sprintf( msgStr, "Error reading file %s, Retry ?", file_name );

		if( box.ask(msgStr) )
		{
			file_seek( curPos );
			goto start_read;
		}
	}

	return ferror(file_handle) == 0? 1 : 0;
}
//---------- End of function File::file_read ----------//


//-------- Begin of function File::file_put_short ----------//
//
// Put a short integer to the file
//
// <short int> = the short integer
//
// return : 1-success, 0-fail
//
int File::file_put_short(short value)
{
   err_when( file_handle == 0 );       // not initialized

   /*
    * This replaces
    *
   WriteFile( file_handle, &value, sizeof(short), &actualWritten, NULL ) ;
    */
    fwrite( &value, 1, sizeof(short), file_handle);

	if( !ferror(file_handle) )
		return TRUE;
	else
	{
		if( handle_error )
			err.run( "Error writing file %s", file_name );

		return FALSE;
	}
}
//---------- End of function File::file_put_short ----------//


//-------- Begin of function File::file_get_short ----------//
//
// Get a short integer from the file
//
// return : the short integer
//
short File::file_get_short()
{
   err_when( file_handle == 0 );       // not initialized

	short value;

    /*
     * This replaces
     *
   ReadFile( file_handle, &value, sizeof(short), &actualRead, NULL );
     */
    fread(&value, 1, sizeof(short), file_handle);

	if(!ferror(file_handle))
		return value;
	else
	{
		if( handle_error )
			err.run( "Error reading file %s", file_name );

		return FALSE;
	}
}
//---------- End of function File::file_get_short ----------//


//-------- Begin of function File::file_put_unsigned_short ----------//
//
// Put a unsigned short integer to the file
//
// <unsigned short> = the short integer
//
// return : 1-success, 0-fail
//
int File::file_put_unsigned_short(unsigned short value)
{
   err_when( file_handle == 0 );       // not initialized

   /*
    * This replaces
    *
 WriteFile( file_handle, &value, sizeof(unsigned short), &actualWritten, NULL )
    */
   fwrite(&value, 1, sizeof(unsigned short), file_handle);

	if(!ferror(file_handle))
		return TRUE;
	else
	{
		if( handle_error )
			err.run( "Error writing file %s", file_name );

		return FALSE;
	}
}
//---------- End of function File::file_put_unsigned_short ----------//


//-------- Begin of function File::file_get_unsigned_short ----------//
//
// Get a short integer from the file
//
// return : the short integer
//
unsigned short File::file_get_unsigned_short()
{
   err_when( file_handle == 0 );       // not initialized

	unsigned short value;
    /*   
     * This replaces
     *
 ReadFile( file_handle, &value, sizeof(unsigned short), &actualRead, NULL ) 
     */
    fread(&value, 1, sizeof(unsigned short), file_handle);

	if(!ferror(file_handle))
		return value;
	else
	{
		if( handle_error )
			err.run( "Error reading file %s", file_name );
		
		return 0;
	}
}
//---------- End of function File::file_get_unsigned_short ----------//


//-------- Begin of function File::file_put_long ----------//
//
// Put a long integer to the file
//
// <long int> = the long integer
//
// return : 1-success, 0-fail
//
int File::file_put_long(long value)
{
   err_when( file_handle == 0 );       // not initialized

   /*
    * This replaces
    *
  WriteFile( file_handle, &value, sizeof(long), &actualWritten, NULL ) 
    */
   fwrite(&value, 1, sizeof(long), file_handle);

   if(!ferror(file_handle))
		return TRUE;
	else
	{
		if( handle_error )
			err.run( "Error writing file %s", file_name );

		return FALSE;
	}
}
//---------- End of function File::file_put_long ----------//


//-------- Begin of function File::file_get_long ----------//
//
// Get a long integer from the file
//
// return : the long integer
//
long File::file_get_long()
{
   err_when( file_handle == 0 );       // not initialized

	long  value;

   /*
    * This replaces
    *
   ReadFile( file_handle, &value, sizeof(long), &actualRead, NULL ) 
    */
    fread(&value, 1, sizeof(long), file_handle);

    if(!ferror(file_handle))
		return value;
	else
	{
		if( handle_error )
			err.run( "Error reading file %s", file_name );

		return FALSE;
	}
}
//---------- End of function File::file_get_long ----------//


//---------- Start of function File::file_seek ---------//
//
// <long> offset = seek offset
// [int]  whence = FILE_BEGIN, FILE_CURRENT, FILE_END
//                 (default : FILE_BEGIN)
//
// return : the offset of the pointer's new position, measured in
//          bytes from the file beginning.
//
long File::file_seek(long offset, int whence)
{
	switch(whence){
        case FILE_BEGIN:
            whence = SEEK_SET;
            break;
        case FILE_CURRENT:
            whence = SEEK_CUR;
            break;
        case FILE_END:
            whence = SEEK_END;
            break;
        default:
            whence = SEEK_SET;
            break;
    }

    fseek(file_handle, offset, whence);

	return ftell(file_handle);
}
//------------ End of function File::file_seek ----------//


//---------- Start of function File::file_pos ---------//
//
// Get the position of current file pointer
//
// return : the position of current file pointer
//
long File::file_pos()
{
   return ftell(file_handle);
}

//------------ End of function File::file_pos ----------//


//---------- Start of function File::file_size ---------//

long File::file_size()
{
    long actual = ftell(file_handle);
    fseek(file_handle, 0, SEEK_END);

    long size = ftell(file_handle);

    fseek(file_handle, actual, SEEK_SET);
	return size;
}
//------------ End of function File::file_size ----------//

