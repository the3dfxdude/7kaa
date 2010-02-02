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

//Filename    : ODIR.CPP
//Description : Object Directory

#include <string.h>
#include <windows.h>
#include <ODIR.h>


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
