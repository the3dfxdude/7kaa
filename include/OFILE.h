//Filename    : OFILE.H
//Description : Header file for file object

#ifndef __OFILE_H
#define __OFILE_H

#ifndef __ALL_H
#include <ALL.h>
#endif

#ifndef __WINDOWS_
#include <windows.h>
#endif

//--------------------------------------//

class File
{
public:
   char file_name[MAX_PATH+1];

   HANDLE file_handle;
   int    handle_error;

   char   allow_vary_size;    // allow the writing size and the read size to be different

public:
   File()     { file_handle=INVALID_HANDLE_VALUE; }
   ~File();

   int   file_open(char*, int=1, int=0);
   int   file_create(char*, int=1, int=0);
	//### begin alex 24/7 ###//
	int	file_append(char*, int=1, int=0);
	//#### end alex 24/7 ####//
   void  file_close();

   long  file_size();
   long  file_seek(long, int= -1);
   long  file_pos();

   int   file_read(void*, unsigned);
   int   file_write(void*, unsigned);

   short file_get_short();
   int   file_put_short(short);

   unsigned short file_get_unsigned_short();
   int   file_put_unsigned_short(unsigned short);

   long  file_get_long();
   int   file_put_long(long);
};

#endif


