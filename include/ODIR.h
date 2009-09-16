//Filename    : ODIR.H
//Description : Object Directory Header

#ifndef __ODIR_H
#define __ODIR_H

#ifndef __WINDOWS_
#include <windows.h>
#endif

#ifndef __ODYNARR_H
#include <ODYNARR.h>
#endif

//---------- Define struct FileInfo ----------//

struct FileInfo
{
    char				name[MAX_PATH+1];
    unsigned long size;
	 FILETIME		time;
};

//---------- Define class Directory ----------//

class Directory : public DynArray
{
public:
   Directory();

   int       read(char*, int=0);
   FileInfo* operator[](int recNo)  { return (FileInfo*) get(recNo); }
};

//--------------------------------------------//

#endif
