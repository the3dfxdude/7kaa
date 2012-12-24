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

//Filename        : ALL.H
//Description : General-purpose header file

#ifndef __ALL_H
#define __ALL_H

//--------- Include other headers -------//

#include <win32_compat.h>
#include <OMISC.h>
#include <OSTR.h>
#include <OFILE.h>
#include <GAMEDEF.h>
#include <OERROR.h>


//-------- Game Version -----------//
#define SKVERSION "2.14.4"
#define SKVERMAJ 2
#define SKVERMED 14
#define SKVERMIN 4


//-------- Define macro functions -------//

#define MAX(a,b)        (((a) > (b)) ? (a) : (b))
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))

//---------- define class Mem ----------//

struct MemInfo;

class Mem
{
public :
        MemInfo* info_array;
        short    ptr_num;
        short    ptr_used;

public :
        Mem();
        ~Mem();

    char* add(unsigned, const char*, int);
    char* add_clear(unsigned, const char*,int);
    char* resize(void*,unsigned, const char*,int);
    char* resize_keep_data(void *orgPtr, unsigned orgSize, unsigned newSize, const char* fileName, int fileLine);
    void  del(void*,const char*,int);

    int get_mem_size(void *memPtr);
};

char* mem_resize_keep_data(void*,unsigned,unsigned);    // called when DEBUG mode is off

extern Mem mem;

//------ memory allocation functions --------//

#ifndef NO_MEM_CLASS

	#define mem_add(memSize)             mem.add(memSize, __FILE__, __LINE__)
	#define mem_add_clear(memSize)       mem.add_clear(memSize, __FILE__, __LINE__)
	#define mem_resize(orgPtr,newSize)   mem.resize(orgPtr, newSize, __FILE__, __LINE__)
	#define mem_del(memPtr)              mem.del(memPtr, __FILE__, __LINE__)

	#define mem_resize_keep_data(orgPtr, orgSize, newSize) \
								  mem.resize_keep_data(orgPtr, orgSize, newSize, __FILE__, __LINE__)
#else

	#include <stdlib.h>

	#define mem_add(memSize)            ((char*)malloc(memSize))
	#define mem_add_clear(memSize)      ((char*)calloc(1,memSize))
	#define mem_resize(orgPtr, newSize) ((char*)realloc(orgPtr,newSize))
	#define mem_del(memPtr)             free(memPtr)

	// mem_resize_keep_data() will be called directly when not in DEBUG mode

#endif

//--------------------------------------------//

#endif
