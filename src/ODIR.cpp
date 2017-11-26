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

#include <stdlib.h>
#include <string.h>
#include <ODATE.h>
#include <ODIR.h>
#include <FileSystem.h>

#ifdef USE_WINDOWS
#include <windows.h>
#endif
#ifdef USE_POSIX
#include <dirent.h>
#include <ctype.h>
#include <sys/stat.h>
#include <glob.h>
#include <time.h>
#endif

#include <posix_string_compat.h>

#include <dbglog.h>

DBGLOG_DEFAULT_CHANNEL(Directory);

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
int Directory::read(const char *fileSpec, int sortName)
{
    FileInfo fileInfo;
#ifdef USE_WINDOWS
    WIN32_FIND_DATA	findData;
   
   //----------- get the file list -------------//

   HANDLE findHandle = FindFirstFile( fileSpec, &findData );

   while(findHandle!=INVALID_HANDLE_VALUE)
   {
      strcpy(fileInfo.name, FileSystem::get_file_name(findData.cFileName)); // get the file name only from a full path string

      fileInfo.size = findData.nFileSizeLow;

      SYSTEMTIME sysTime;
      FILETIME localFileTime;
      FileTimeToLocalFileTime(&findData.ftLastWriteTime, &localFileTime);
      FileTimeToSystemTime(&localFileTime, &sysTime);
      fileInfo.time.year = sysTime.wYear;
      fileInfo.time.month = sysTime.wMonth;
      fileInfo.time.day = sysTime.wDay;
      fileInfo.time.hour = sysTime.wHour;
      fileInfo.time.minute = sysTime.wMinute;

      linkin( &fileInfo );

      if( !FindNextFile( findHandle, &findData ) )
          break;
   }

   FindClose(findHandle);
#endif
#ifdef USE_POSIX
   glob_t results;
   glob(fileSpec, sortName ? 0 : GLOB_NOSORT, NULL, &results);
   for( int i = 0; i < results.gl_pathc; i++ )
   {
      struct stat file_stat;

      if( stat(results.gl_pathv[i], &file_stat) )
      {
         // can't read, skip
         continue;
      }

      strcpy(fileInfo.name, FileSystem::get_file_name(results.gl_pathv[i]));

      fileInfo.size = file_stat.st_size;

      struct tm *time = localtime(&file_stat.st_mtime);
      fileInfo.time.year = time->tm_year+1900;
      fileInfo.time.month = time->tm_mon+1;
      fileInfo.time.day = time->tm_mday;
      fileInfo.time.hour = time->tm_hour;
      fileInfo.time.minute = time->tm_min;

      linkin(&fileInfo);
   }
   globfree(&results);
#endif

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
