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
#include <ODIR.h>

#ifdef NO_WINDOWS
#include <dirent.h>
#include <ctype.h>
#include <sys/stat.h>
#endif

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
   FileInfo				fileInfo;
#ifndef NO_WINDOWS
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

#else

   char dirname[MAX_PATH];
   char search[MAX_PATH];
   struct dirent **namelist;
   int n;

   char *slash = strrchr((char*)fileSpec, '\\');
   if (slash)
   {
      char *s = (char*)fileSpec;
      char *d = dirname;
      int i = 0;
      while (s != slash && i < MAX_PATH - 1)
      {
         if (*d == '\\')
            *d = '/';
         else
            *d = tolower(*s);
         d++;
         s++;
         i++;
      }
      *d = 0;

      i = 0;
      d = search;
      s++;
      while (*s && i < MAX_PATH - 1)
      {
         if (*s == '*')
         {
            s++;
            i++;
            continue;
         }
         else if (*s == '.')
         {
            *d = *s;
         }
         else
         {
            *d = tolower(*s);
         }
         d++;
         s++;
         i++;
      }
      *d = 0;
   } else {
      char *s = (char*)fileSpec;
      char *d = search;
      int i = 0;

      while (*s && i < MAX_PATH - 1)
      {
         if (*s == '*')
         {
            s++;
            i++;
            continue;
         }
         else if (*s == '.')
         {
            *d = *s;
         }
         else
         {
            *d = tolower(*s);
         }
         d++;
         s++;
         i++;
      }
      *d = 0;


      dirname[0] = '.';
      dirname[1] = 0;
   }

   n = scandir(dirname, &namelist, 0, alphasort);
   for (int i = 0; i < n; i++)
   {
      char filename[MAX_PATH];
      char *s = namelist[i]->d_name;
      char *d = filename;
      int j = 0;
      while (*s && j < MAX_PATH - 1)
      {
         if (isalpha(*s))
            *d = tolower(*s);
         else
            *d = *s;
         d++;
         s++;
         j++;
      }
      *d = 0;

      if (strstr(filename, search))
      {
         char full_path[MAX_PATH];
         struct stat file_stat;

         full_path[0] = 0;
         strcat(full_path, dirname);
         strcat(full_path, "/");
         strcat(full_path, namelist[i]->d_name);

         stat(full_path, &file_stat);

         strncpy(fileInfo.name, namelist[i]->d_name, MAX_PATH - 2);

         fileInfo.size = file_stat.st_size;
         fileInfo.time.dwLowDateTime = 0;
         fileInfo.time.dwHighDateTime = 0;

         linkin( &fileInfo );
      }
      free(namelist[i]);
   }
   if (n > -1)
     free(namelist);

#endif

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
