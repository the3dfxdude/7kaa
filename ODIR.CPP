//Filename    : ODIR.CPP
//Description : Object Directory

#include <string.h>
#include <windows.h>
#include <ODIR.H>


//----------- Define static function ------------//

static int sort_file_function( const void *a, const void *b );


//------- Begin of function Directory::Directory -------//

Directory::Directory() : DynArray( sizeof(FileInfo), 20 )
{
}

//-------- End of function Directory::Directory -------//


//------- Begin of function Directory::read -------//
//
// Read in the file list of the specified file spec.
//
// <char*> fileSpec = the file spec of the directory
// [int]   sortName = sort the file list by file name
//                    (default : 0)
//
// return : <int> the no. of files matched the file spec.
//
int Directory::read(char *fileSpec, int sortName)
{
   FileInfo				fileInfo;
	WIN32_FIND_DATA	findData;
   
   //----------- get the file list -------------//

   HANDLE findHandle = FindFirstFile( fileSpec, &findData );

   while(findHandle!=INVALID_HANDLE_VALUE)
   {
      m.extract_file_name( fileInfo.name, findData.cFileName );		// get the file name only from a full path string 

      fileInfo.size = findData.nFileSizeLow;
      fileInfo.time = findData.ftLastWriteTime; 

      linkin( &fileInfo );

      if( !FindNextFile( findHandle, &findData ) )
			break;
   }

	FindClose(findHandle);

   //------ the file list by file name ---------//

   if( sortName )
      quick_sort( sort_file_function );

   return size();       // DynArray::size()
}
//-------- End of function Directory::read -------//


//------ Begin of function sort_file_function ------//
//
static int sort_file_function( const void *a, const void *b )
{
	return strcmpi( ((FileInfo*)a)->name, ((FileInfo*)b)->name );
}
//------- End of function sort_file_function ------//
