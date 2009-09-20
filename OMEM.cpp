//Filename    : OMEM.CPP
//Description : Object Memory Management (Debug Version)

#include <string.h>
#include <stdlib.h>

#ifndef NO_MEM_CLASS

#include <stdio.h>
#include <ALL.h>

//--------- Define Constants -----------//

// We will write the PRE_CHK_VAL before every allocated memory block
// and POST_CHK_VAL after them, so when freeing them, we could
// know whether they have been underrun/overun or not

#define CHK_VAL_SIZE    sizeof(long)

#define PRE_CHK_VAL     0x12345678      // value to detect underrun
#define POST_CHK_VAL    0x87654321      // value to detect overrun

// The followings should are selected to give maximum probability that
// pointers loaded with these values will cause an obvious crash.
// MALLOCVAL is the value to set malloc'd data to.

#define BAD_VAL         0xFF     // set to this value for freed memory (memory that we no longer occupy)
#define ALLOC_VAL       0xEE     // set to this value for memory just allocated


//---------- define constant and structure --------//

#define SPOOL_MEM  50         // 50 bytes spool memory for mem_add(),

struct MemInfo
{
   void     *ptr;       // this pointer directly point to useable buffer
   unsigned size;       // bypassing the PRE_CHK_VAL

   char     *file_name;
   int      file_line;
};


//-------- BEGIN OF FUNCTION Mem::Mem ------------//

Mem::Mem()
{
   info_array = (MemInfo *)malloc(sizeof(MemInfo) * 100);

   if ( info_array == NULL )
      err.mem();

   ptr_num  = 100 ;
   ptr_used = 0;
}
//---------- END OF FUNCTION Mem::Mem ------------//


//--------- BEGIN OF FUNCTION Mem::add --------------//
//
// <unsigned> memNum   = the size of the memory to be allocated
// <char*>    fileName = file from which the client function calls
// <int>      fileLine = line number of the client function in the file
//
char* Mem::add(unsigned memSize, char* fileName, int fileLine)
{
	// ###### begin Gilbert 29/8 ######//
	//err_when( memSize > 1000000 );		//**BUGHERE, for temporary debugging only
	err_when( memSize > 0x800000 );
	// ###### end Gilbert 29/8 ######//

   //----------- build up memory pointer table ---------//

   if ( ptr_used == ptr_num )
   {
      ptr_num += 100 ;
      if ( ptr_num > 10000 )
         err.run( " Mem::add() - Too many pointers " );

      info_array = (MemInfo*) realloc( info_array, sizeof(MemInfo) * ptr_num ) ;

      if ( info_array == NULL )
         err.mem();
   }

   //----------- actually allocate memory -------------//

   char *allocPtr;

   allocPtr = (char *)malloc(sizeof(char)*( memSize + CHK_VAL_SIZE*2 )); // Pre-check & Post-check

   if ( allocPtr == NULL )
   {
      err.mem();
      return NULL;
   }
   else
   {
      // set check value before and after the allocated block,
      // so Mem::del() can use these check value to detect
      // underrun && overrun

      *((long*)allocPtr)                        = PRE_CHK_VAL;
      *((long*)(allocPtr+CHK_VAL_SIZE+memSize)) = POST_CHK_VAL;

      // fill the allocated block with a value, which may
      // have chance to reveal some hiden bugs

      memset( allocPtr+CHK_VAL_SIZE, ALLOC_VAL, memSize );

      info_array[ptr_used].ptr       = allocPtr+CHK_VAL_SIZE;
      info_array[ptr_used].size      = memSize;
      info_array[ptr_used].file_name = fileName;
      info_array[ptr_used].file_line = fileLine;
      ptr_used++;

      return allocPtr+CHK_VAL_SIZE;
   }
}
//---------- END OF FUNCTION Mem::add ---------------//


//--------- BEGIN OF FUNCTION Mem::add_clear --------------//
//
// Allocate the memory and set all the memory content to byte 0.
//
// <unsigned> memNum   = the size of the memory to be allocated
// <char*>    fileName = file from which the client function calls
// <int>      fileLine = line number of the client function in the file
//
char* Mem::add_clear(unsigned memSize, char* fileName, int fileLine)
{
	err_when( memSize > 1000000 );		//**BUGHERE, for temporary debugging only

	//----------- build up memory pointer table ---------//

	if ( ptr_used == ptr_num )
	{
		ptr_num += 100 ;
		if ( ptr_num > 10000 )
			err.run( " Mem::add_clear() - Too many pointers " );

		info_array = (MemInfo*) realloc( info_array, sizeof(MemInfo) * ptr_num ) ;

		if ( info_array == NULL )
			err.mem();
	}

	//----------- actually allocate memory -------------//

	char *allocPtr;

	allocPtr = (char*)malloc(sizeof(char)*( memSize + CHK_VAL_SIZE*2 )); // Pre-check & Post-check

	if ( allocPtr == NULL )
	{
		err.mem();
		return NULL;
	}
	else
	{
		// set check value before and after the allocated block,
		// so Mem::del() can use these check value to detect
		// underrun && overrun

		*((long*)allocPtr)                        = PRE_CHK_VAL;
		*((long*)(allocPtr+CHK_VAL_SIZE+memSize)) = POST_CHK_VAL;

		// fill the allocated block with a value, which may
		// have chance to reveal some hiden bugs

		memset( allocPtr+CHK_VAL_SIZE, 0, memSize );

		info_array[ptr_used].ptr       = allocPtr+CHK_VAL_SIZE;
      info_array[ptr_used].size      = memSize;
      info_array[ptr_used].file_name = fileName;
      info_array[ptr_used].file_line = fileLine;
      ptr_used++;

      return allocPtr+CHK_VAL_SIZE;
   }
}
//---------- END OF FUNCTION Mem::add_clear ---------------//


//-------- BEGIN OF FUNCTION Mem::resize_keep_data ----------//
//
// The Mem::resize() and realloc() may not function properly in
// some case when the memory block has a considerable size.
//
// Calling this function resize_keep_data will do additional effort
// to preserve the original data.
//
// <void*>    orgPtr    = the original memory data pointer
// <unsigned> orgSize   = the original data size
// <unsigned> newSize   = new size of memory required
// <char*>    fileName  = file from which the client function calls
// <int>      fileLine  = line number of the client function in the file
//
// Returns : NULL    - not enough memory
//           <char*> - pointer to the allocated memory
//
char* Mem::resize_keep_data(void *orgPtr, unsigned orgSize, unsigned newSize, char* fileName, int fileLine)
{
   if( orgPtr == NULL )
      return add( newSize, fileName, fileLine);

   if( newSize <= orgSize )
      return resize(orgPtr, newSize, fileName, fileLine);

   //-------- save the original data first ------//

   char* saveBuf = (char*)malloc(sizeof(char)*(orgSize));

   memcpy( saveBuf, orgPtr, orgSize );

   //------ reallocate the memory --------//

   char* newPtr = resize(orgPtr, newSize, fileName, fileLine);

   //----- store the original data to the new buf -------//
   //
   // if the new pointer is the same as the orginal pointer
   // the original data should be kept without any change
   //
   //----------------------------------------------------//

   if( newPtr != orgPtr )
      memcpy( newPtr, saveBuf, orgSize );

   free(saveBuf);

   return newPtr;
}
//----------- END OF FUNCTION Mem::resize_keep_data ---------------//


//-------- BEGIN OF FUNCTION Mem::resize ----------//
//
// <void*>    orgPtr = the original memory data pointer
// <unsigned> memSize   = new size of memory required
// <char*>    fileName  = file from which the client function calls
// <int>      fileLine  = line number of the client function in the file
//
// Returns : NULL    - not enough memory
//           <char*> - pointer to the allocated memory
//
// Note : resize() must actually call realloc(), it can't call mem()
//        and add(), because some clients want to keep the content on the
//        existing buffer. (e.g. DynArray)
//
char* Mem::resize(void *orgPtr, unsigned memSize, char* fileName, int fileLine)
{
	err_when( memSize > 1000000 );		//**BUGHERE, for temporary debugging only

   if( orgPtr == NULL )
      return add( memSize, fileName, fileLine);

   //-------------------------------------------//

   char *newPtr;
   int  i;

   for( i=ptr_used-1; i>=0; i-- )
   {
      if( info_array[i].ptr == orgPtr )
      {
         if( info_array[i].size != memSize )
         {
            // Remember : MemInfo::ptr points directly to client buffer,
            //            bypassing the PRE_CHK_VAL

            newPtr = (char*) realloc( (char*)orgPtr-CHK_VAL_SIZE, memSize+CHK_VAL_SIZE*2 );

            if( newPtr == NULL )
               err.mem();

            // set the POST_CHK_VAL again as the size of it has changed

				*((long*)newPtr)                        = PRE_CHK_VAL;
            *((long*)(newPtr+CHK_VAL_SIZE+memSize)) = POST_CHK_VAL;

            info_array[i].ptr  = newPtr + CHK_VAL_SIZE;
            info_array[i].size = memSize;
         }

         return (char*) info_array[i].ptr;
      }
   }

   err.run( "Mem::resize - Original memory pointer not found.\n"
            "File name : %s, line no. : %d \n", fileName, fileLine );

   return NULL;
}
//----------- END OF FUNCTION Mem::resize ---------------//


//-------- BEGIN OF FUNCTION Mem::del ----------//
//
// <void*>    freePtr   = the memory data pointer to be freed
// <char*>    fileName  = file from which the client function calls
// <int>      fileLine  = line number of the client function in the file
//
void Mem::del(void *freePtr, char* fileName, int fileLine)
{
   int   i ;
   char* truePtr;

   for( i=ptr_used-1; i>=0; i-- )
   {
      if( info_array[i].ptr == freePtr )
      {
         // truePtr is the pointer actually point to the start of the allocated block, including PRE_CHK_VAL

         truePtr = (char*) freePtr - CHK_VAL_SIZE;

         //---- Check for Underwrite and Overwrite error ---//

         if( *((long*)truePtr) != PRE_CHK_VAL )
            err.run( "Mem::del - Memory Underwritten, File name:%s, line no.:%d\n", fileName, fileLine );

         if( *((long*)(truePtr+CHK_VAL_SIZE+info_array[i].size)) != POST_CHK_VAL )
            err.run( "Mem::del - Memory Overwritten, File name:%s, line no.:%d\n", fileName, fileLine );

         // fill the to be freed block with a value, which may
         // have chance to reveal some hiden bugs

         memset( truePtr+CHK_VAL_SIZE, BAD_VAL, info_array[i].size );

         //--------- free it up --------------//

         free(truePtr);

         memmove( info_array+i, info_array+i+1, sizeof(MemInfo) * (ptr_used-i-1) ) ;
         ptr_used-- ;
         return ;
      }
   }

   err.run( "Mem::del - Free value not found, File name:%s, line no.:%d\n", fileName, fileLine );
}
//----------- END OF FUNCTION Mem::del ---------------//


//-------- BEGIN OF FUNCTION Mem::get_mem_size ----------//
//
// This function is mainly for debugging only.
//
// <void*> memPtr   = the memory data pointer to be freed
//
int Mem::get_mem_size(void *memPtr)
{
	for( int i=ptr_used-1; i>=0; i-- )
	{
		if( info_array[i].ptr == memPtr )
			return info_array[i].size;
	}

	err.run( "Error: Mem::get_mem_size()." );

	return 0;
}
//----------- END OF FUNCTION Mem::get_mem_size ---------------//


//-------- BEGIN OF FUNCTION Mem::~Mem ------------//

Mem::~Mem()
{
   if ( ptr_used > 0 )
   {
      int i;

      for ( i=0; i< ptr_used ; i++ )
		{
         err.msg( "Memory not freed. File name : %s, line no. : %d \n",
                  info_array[i].file_name, info_array[i].file_line );
		}
   }

   free(info_array);
}

//---------- END OF FUNCTION Mem::~Mem ------------//

/*

//-------- Begin of Overload "new" operator --------//

void* operator new(size_t memSize)
{
   void *memPtr = malloc(memSize);

   return memPtr;
}

//--------- End of Overload "new" operator ---------//


//-------- Begin of Overload "delete" operator --------//

void operator delete(void *memPtr)
{
   free(memPtr);
}

//--------- End of Overload "delete" operator ---------//

*/

#else

//-------- BEGIN OF FUNCTION mem_resize_keep_data ----------//
//
// This is the non-DEBUG version of Mem::resize_keep_data()
//
// <void*>    orgPtr    = the original memory data pointer
// <unsigned> orgSize   = the original data size
// <unsigned> newSize   = new size of memory required
//
// Returns : NULL    - not enough memory
//           <char*> - pointer to the allocated memory
//
char* mem_resize_keep_data(void *orgPtr, unsigned orgSize, unsigned newSize)
{
   if( orgPtr == NULL )
      return (char*) malloc(newSize);

   if( newSize <= orgSize )
      return (char*) realloc(orgPtr, newSize);

   //-------- save the original data first ------//

   char* saveBuf = (char*)malloc(sizeof(char)*(orgSize));

   memcpy( saveBuf, orgPtr, orgSize );

   //------ reallocate the memory --------//

   char* newPtr = (char*) realloc(orgPtr, newSize);

   //----- store the original data to the new buf -------//

   if( newPtr != orgPtr )       // only when the pointer has been changed
      memcpy( newPtr, saveBuf, orgSize );

   free(saveBuf);

   return newPtr;
}
//----------- END OF FUNCTION mem_resize_keep_data ---------------//


#endif

//-----------------------------------------------------//
//
// An article from Walter Bright's MEM Package which is
// very similar to our omem class
//
// WHAT MEM DOES:
// --------------
//
// 1.    ISO/ANSI verification:
//
// When Walter wrote MEM, compiler compliance with ANSI standards was still
// quite low.  MEM verifies ISO/ANSI compliance for situations such as passing
// NULL or size 0 to allocation/reallocation functions.
//
// 2.    Logging of all allocations and frees:
//
// All MEM's functions pass the __FILE__ and __LINE__ arguments.  During alloca-
// tion, MEM makes an entry into a linked list and stores the file and line
// information in the list for whichever allocation or free function is called.
//
// This linked list is the backbone of MEM.  When MEM detects a bug, it tells
// you where to look in which file to begin tracking the problem.
//
// 3.    Verification of frees:
//
// Since MEM knows about all allocations, when a pointer is freed, MEM can
// verify that the pointer was allocated originally.  Additionally, MEM will
// only allow a pointer to be freed once.
//
// Freed data is overwritten with a non-zero known value, flushing such problems
// as continuing to reference data after it's been freed.  The value written
// over the data is selected to maximize the probability of a segment fault or
// assertion failure if your application references it after it's been freed.
//
// MEM obviously can't directly detect "if" instances such as...
//
//       mem_free(p);
//       if (p) ...
//
// ...but by guaranteeing that `p' points to garbage after being freed, code
// like this will hopefully never work and will thus be easier to find.
//
// 4.    Detection of pointer over- and under-run:
//
// Pointer overrun occurs when a program stores data past the end of a buffer,
// e.g.
//
//       p = malloc(strlen(s));        /* No space for terminating NUL      */
//       strcpy(p,s);                  /* Terminating NUL clobber memory   */
//
// Pointer underrun occurs when a program stores data before the beginning of a
// buffer.  This error occurs less often than overruns, but MEM detects it
// anyway.  MEM does this by allocating a little extra at each end of every
// buffer, which is filled with a known value, called a sentinel. MEM detects
// overruns and underruns by verifying the sentinel value when the buffer is
// freed.
//
// 5.    Dependence on values in buffer obtained from malloc():
//
// When obtaining a buffer from malloc(), a program may develop erroneous and
// creeping dependencies on whatever random (and sometimes repeatable) values
// the buffer may contain.  The mem_malloc() function prevents this by always
// setting the data in a buffer to a known non-zero value before returning its
// pointer.  This also prevents another common error when running under MS-DOS
// which doesn't clear unused memory when loading a program.  These bugs are
// particularly nasty to find since correct program operation may depend on what
// was last run!
//
// 6.    Realloc problems:
//
// Common problems when using realloc() are: 1) depending on realloc() *not*
// shifting the location of the buffer in memory, and 2) depending on finding
// certain values in the uninitialized region of the realloc'ed buffer.
//
// MEM flushes these out by *always* moving the buffer and stomping on values
// past the initialized area.
//
// 7.    Memory leak detection:
//
// Memory "leaks" are areas that are allocated but never freed.  This can become
// a major problem in programs that must run for long periods without interrup-
// tion (e.g. BBS's).  If there are leaks, eventually the program will run out
// of memory and fail.
//
// Another form of memory leak occurs when a piece of allocated memory should
// have been added to some central data structure, but wasn't.
//
// MEM find memory leaks by keeping track of all allocations and frees.  When
// mem_term() is called, a list of all unfreed allocations is printed along with
// the files and line numbers where the allocations occurred.
//
// 8.    Pointer checking:
//
// Sometimes it's useful to be able to verify that a pointer is actually
// pointing into free store. MEM provides a function...
//
//       mem_checkptr(void *p);
//
// ...to do this.
//
// 9.    Consistency checking:
//
// Occasionally, even MEM's internal data structures get clobbered by a wild
// pointer.  When this happens, you can track it down by sprinkling your code
// temporarily with calls to mem_check(), which performs a consistency check on
// the free store.
//
// 10.   Out of memory handling:
//
// MEM can be set using mem_setexception() (see MEM.H) to handle out-of-memory
// conditions in any one of several predefined ways:
//
//       1.    Present an "Out of memory" message and terminate the program.
//       2.    Abort the program with no message.
//       3.    Mimic ISO/ANSI and return NULL.
//       4.    Call a user-specified function, perhaps involving virtual memory
//          or some other "emergency reserve".
//       5.    Retry (be careful to avoid infinite loops!)
//
// 11.   Companion techniques:
//
// Since MEM presets allocated and stomps on freed memory, this facilitates
// adding your own code to add tags to your data structures when debugging.  If
// the structures are invalid, you'll know it because MEM will have clobbered
// your verification tags.
//
//-----------------------------------------------------//
