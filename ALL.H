//Filename        : ALL.H
//Description : General-purpose header file

#ifndef __ALL_H
#define __ALL_H

//--------- Include other headers -------//

#ifndef __WINDOWS_
#include <windows.h>
#endif

#ifndef __OMISC_H
#include <OMISC.H>
#endif

#ifndef __OSTR_H
#include <OSTR.H>
#endif

#ifndef __OFILE_H
#include <OFILE.H>
#endif

#ifndef __GAMEDEF_H
#include <GAMEDEF.H>
#endif

//-------- Define macro functions -------//

#define max(a,b)        (((a) > (b)) ? (a) : (b))
#define min(a,b)        (((a) < (b)) ? (a) : (b))

//---------- Debug functions -------//

#ifdef DEBUG
void __cdecl debug_msg( char* fmt, ... );
#endif

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

        char*  add(unsigned,char*,int);
		  char*  add_clear(unsigned,char*,int);

		  char*  resize(void*,unsigned,char*,int);
		  char*  resize_keep_data(void *orgPtr, unsigned orgSize, unsigned newSize, char* fileName, int fileLine);

		  void   del(void*,char*,int);

			int 	get_mem_size(void *memPtr);
};

char* mem_resize_keep_data(void*,unsigned,unsigned);    // called when DEBUG mode is off

extern Mem mem;

//------- Define Class Error ------------//

typedef void (*ExtraHandler)();

class Error
{
private:
   ExtraHandler extra_handler;          // extra error handler

public:
   Error();

   void internal(char*,char*,int);
   void mem();
        void msg(char*,...);
   void run(char*,...);

   void set_extra_handler(ExtraHandler extraHandler) { extra_handler = extraHandler; }
};

extern Error err;

//-------- error handling functions ----------//

#ifdef DEBUG
   #define err_when(cond)   if(cond) err.internal(NULL,__FILE__, __LINE__)
   #define err_here()       err.internal(NULL,__FILE__, __LINE__)
   #define err_if(cond)     if(cond)
   #define err_else         else
   #define err_now(msg)     err.run(msg)        // internal error

   // always use err_if(cond) together with err_now(), so when debug is turn off, these two statement will turn off
#else
   #define err_when(cond)
   #define err_here()
   #define err_if(cond)
   #define err_else
   #define err_now(msg)
#endif


//------ memory allocation functions --------//

#ifndef NO_MEM_CLASS

	#define mem_add(memSize)             mem.add(memSize, __FILE__, __LINE__)
	#define mem_add_clear(memSize)       mem.add_clear(memSize, __FILE__, __LINE__)
	#define mem_resize(orgPtr,newSize)   mem.resize(orgPtr, newSize, __FILE__, __LINE__)
	#define mem_del(memPtr)              mem.del(memPtr, __FILE__, __LINE__)

	#define mem_resize_keep_data(orgPtr, orgSize, newSize) \
								  mem.resize_keep_data(orgPtr, orgSize, newSize, __FILE__, __LINE__)
#else

	#define mem_add(memSize)            ((char*)malloc(memSize))
	#define mem_add_clear(memSize)      ((char*)calloc(1,memSize))
	#define mem_resize(orgPtr, newSize) ((char*)realloc(orgPtr,newSize))
	#define mem_del(memPtr)             free(memPtr)

	// mem_resize_keep_data() will be called directly when not in DEBUG mode

#endif

//--------------------------------------------//

#endif
