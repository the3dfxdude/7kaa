//Filename    :: ODYNARR.CPP
//Description :: Dynamic Array object

#include <stdlib.h>
#include <string.h>

#include <ODYNARR.H>

//--------- BEGIN OF FUNCTION DynArray Constructor -------//
//
// <int> eleSize  = size of each element
// [int] blockNum = number of entity of each block of element
//                       increased ( default : 30 )

DynArray::DynArray(int eleSize,int blockNum)
{
   ele_size  = eleSize;
   block_num = blockNum;

   body_buf = mem_add( ele_size*block_num );

   cur_pos=0;
   last_ele=0;
   ele_num= block_num;

   sort_offset = -1;
}

//----------- END OF FUNCTION DynArray Constructor -----//


//--------- BEGIN OF FUNCTION DynArray Destructor ------//

DynArray::~DynArray()
{
   mem_del( body_buf );
}

//---------- END OF FUNCTION DynArray Destructor --------//


//--------- BEGIN OF FUNCTION DynArray::resize ---------//
//
// change the size of the storage block - this will always be an increase
//
void DynArray::resize( int newNum )
{
   //-------------------------------------------------------//
   //
   // The Mem::resize() and realloc() may not function properly in
   // some case when the memory block has a considerable size.
   //
   // Calling function resize_keep_data will do additional effort
   // to preserve the original data.
   //
   //-------------------------------------------------------//

	body_buf = mem_resize( body_buf, newNum*ele_size );	// give both the original data size and the new data size

   ele_num = newNum;
}

//--------- END OF FUNCTION DynArray::resize -----------//


//--------- BEGIN OF FUNCTION DynArray::zap ---------//
//
// Zap the whole dynamic array, clear all elements
//
// [int] resizeFlag - whether resize the array to its initial size
//							 or keep its current size.
//							 (default:1)
//
void DynArray::zap(int resizeFlag)
{
	if( resizeFlag )
	{
		if( ele_num != block_num )			// if the current record no. is already block_num, no resizing needed
		{
			ele_num  = block_num;
			body_buf = mem_resize(body_buf, ele_size*ele_num );
		}
	}

	cur_pos=0;
	last_ele=0;
}
//--------- END OF FUNCTION DynArray::zap -----------//


//---------- BEGIN OF FUNCTION DynArray::linkin -----------//
//
// Link a record at the END of the array
//
// WARNING : After calling linkin() all pointers to the linklist body
//           should be updated, because mem_resize() will move the body memory
//
void DynArray::linkin(void* ent)
{
   last_ele++;
   cur_pos=last_ele;

   if ( last_ele > ele_num ) // not enough empty element left to hold the new entity
      resize( ele_num + block_num );

   if ( ent )
      memcpy(body_buf+(cur_pos-1)*ele_size, ent, ele_size );
   else
      *(body_buf+(cur_pos-1)*ele_size) = NULL;
}

//---------- END OF FUNCTION DynArray::linkin ------------//


//---------- BEGIN OF FUNCTION DynArray::linkin_unique -----------//
//
// Linkin - unique mode. If duplicated, don't link into the array.
//
void DynArray::linkin_unique(void* ent)
{
	int i;

	for( i=0 ; i<last_ele ; i++ )
	{
		if( memcmp( body_buf+i*ele_size, ent, ele_size )==0 )
			return;
	}

	linkin(ent);
}
//---------- END OF FUNCTION DynArray::linkin_unique ------------//


//---------- BEGIN OF FUNCTION DynArray::insert -----------//
//
// Link into the position BEFORE the current one
//
// Warning : DynArrayB (version B) can't use this function
//
void DynArray::insert(void* ent)
{
	if( size()==0 )
	{
		linkin(ent);
		return;
	}

	last_ele++;

   if ( last_ele > ele_num ) // not enough empty element left to hold the new entity
      resize( ele_num + block_num );

   memmove( body_buf+cur_pos*ele_size,body_buf+(cur_pos-1)*ele_size, (last_ele-cur_pos)*ele_size );

   if ( ent )
      memcpy(body_buf+(cur_pos-1)*ele_size, ent, ele_size );
   else
      *(body_buf+(cur_pos-1)*ele_size) = NULL;
}

//---------- END OF FUNCTION DynArray::insert ------------//


//---------- BEGIN OF FUNCTION DynArray::insert_at -----------//
//
// Link into the position BEFORE the current one
//
// Warning : DynArrayB (version B) can't use this function
//
// <int>   insertPos - the recno to insert the new entity.
// <void*> ent		   - pointer to the record entity. If NULL, blank record.
//
void DynArray::insert_at(int insertPos, void* ent)
{
	if( size()==0 || insertPos>last_ele )
	{
		linkin(ent);
		return;
	}

	err_when( insertPos<1 || insertPos > last_ele );

	last_ele++;

	if ( last_ele > ele_num ) // not enough empty element left to hold the new entity
		resize( ele_num + block_num );

	memmove( body_buf+insertPos*ele_size, body_buf+(insertPos-1)*ele_size, (last_ele-insertPos)*ele_size );

	if ( ent )
		memcpy(body_buf+(insertPos-1)*ele_size, ent, ele_size );
	else
		*(body_buf+(insertPos-1)*ele_size) = NULL;
}

//---------- END OF FUNCTION DynArray::insert_at ------------//


//----------- BEGIN OF FUNCTION DynArray::linkout ---------//
//
// Linkout the current object and then point to next object
//
// [int] delPos = the position (recno) of the item to be deleted
//                ( default : recno() current record no. )
//
void DynArray::linkout(int delPos)
{
	if( delPos < 0 )
		delPos = cur_pos;

	if( delPos == 0 || delPos > last_ele )
		return;

	if( delPos != last_ele )
		memmove( body_buf+(delPos-1)*ele_size, body_buf+delPos*ele_size, (last_ele-delPos)*ele_size );

   last_ele--;

   if( cur_pos > last_ele )
      cur_pos = last_ele;

   if ( last_ele < ele_num - block_num*2 )      // shrink the size if two empty block of element are left
      resize( ele_num - block_num );
}

//------------ END OF FUNCTION DynArray::linkout ----------//



//---------- BEGIN OF FUNCTION DynArray::update ---------//
//
// <void*> bodyPtr = the pointer to the content body
// [int]   recNo   = no. of the record to be updated
//                   ( default : current record no. )
//
void DynArray::update(void* bodyPtr, int recNo)
{
   if( recNo < 0 )
      recNo = cur_pos;

   if( recNo <= 0 )
      return;

   if( bodyPtr )
      memcpy(body_buf+(recNo-1)*ele_size, bodyPtr, ele_size );
   else
      *(body_buf+(recNo-1)*ele_size) = NULL;
}
//----------- END OF FUNCTION DynArray::update ---------//


//---------- BEGIN OF FUNCTION DynArray::add_blank -----------//
//
// Add a specified number of blank records at the END of the array
//
// <int> blankNum = no. of blank records
//
void DynArray::add_blank(int blankNum)
{
   cur_pos=last_ele+1;
   last_ele+=blankNum;

   if ( last_ele > ele_num ) // not enough empty element left to hold the new entity
      resize( last_ele+block_num );

   memset( body_buf+(cur_pos-1)*ele_size, 0, blankNum*ele_size );
}

//---------- END OF FUNCTION DynArray::add_blank ------------//


//------ BEGIN OF FUNCTION DynArray::scan_whole ----------//
//
// Description : scan a sorted or unsorted link list for matching
//               the Whole Structure
//
// Syntax      : scan(<void*>)
//
// <void*> = pointer to the data structure
//
// Return : 0 if not found
//          if found, return the position found
//
int DynArray::scan_whole(void* structPtr)
{
   for ( cur_pos=1 ; cur_pos<=last_ele ; cur_pos++ )
   {
      if( memcmp( structPtr, get(), ele_size ) == 0 )
         return cur_pos;
   }

   return 0;
}
//-------- END OF FUNCTION DynArray::scan_whole ----------//



//------ BEGIN OF FUNCTION DynArray::scan ----------//
//
// Description : scan a sorted or unsorted link list for matching
//               the specified element in the structure
//
// Syntax      : scan(<void*>,<int>,<char>,[int])
//
// <void*> = the data to be scanned for
// <int>   = the offset of the structure containing the variable to be compared
// <char>  = the type of the variable in the structure
//           'C' = char[]
//           'P' = char*
//           'd' = double
//           'i' = integer
//           'c' = char
// [int]  = restore the original position      ( default : FALSE )
//
// Return : 0 if not found
//          if found, return the position found
//
int DynArray::scan(void* varChar,int varOff,char varType,int restPos)
{
   int oldPos,ret;

   oldPos = cur_pos;

   for ( cur_pos=1 ; cur_pos<=last_ele ; cur_pos++ )
   {
      if ( compare(varChar,varOff,varType) )
      {
         ret = cur_pos;
         if (restPos)
            cur_pos = oldPos;
         return ret;
      }
   }

   return 0;
}
//-------- END OF FUNCTION DynArray::scan ----------//


//------- BEGIN OF FUNCTION DynArray::compare --------//
//
// <char*> = the character string to be scanned for
// <int>   = the offset of the structure containing the variable to be compared
// <char>  = the type of the variable in the structure
//           'C' = char[]
//           'P' = char*
//           'd' = double
//           'i' = integer
//           'c' = char

int DynArray::compare(void* varChar,int varOff,char varType)
{
   char *bodyPtr,*bodyStr;

   bodyPtr = (char*) get();

   switch( varType )
   {
      case 'C' :
      case 'P' :
         if ( varType == 'C' )
            bodyStr = bodyPtr + varOff;
         else
            bodyStr = *( (char**)(bodyPtr+varOff) );

         if( bodyStr == (char*) varChar )   // the pointer is the same
            return 1;
         else
            return m.str_cmp(bodyStr, (char*) varChar);    // m1strcmp with exact set off

      case 'c' :
         return *(bodyPtr + varOff ) == *((char*)varChar);

      case 'i' :
         return *((int*)(bodyPtr+varOff)) == *((int*)varChar);

      case 'd' :
         return *((double*)(bodyPtr+varOff)) == *((double*)varChar);
   }

   return NULL;
}

//-------- END OF FUNCTION DynArray::compare ----------//


//---------- BEGIN OF FUNCTION DynArray::clean_up --------------//
//
// Description : clear up the whole dynamic array
//
// Syntax      : clean_up(<int[]>)
//
// <int[]> = the array of offset of the char*
//

void DynArray::clean_up(int *stringOffset)
{
   if ( stringOffset )
   {
      int i;

      for ( i = 0 ; i<last_ele ; i++ )
         free_ptr( body_buf+i*ele_size, stringOffset );
   }

   last_ele=0;
   cur_pos =0;

   resize( block_num );
}

//---------- END OF FUNCTION DynArray::clean_up ---------//


//------- BEGIN OF FUNCTION DynArray::free_ptr ----------//

void DynArray::free_ptr( void* freebody, int *stringOffset )
{
    int    i,stringNum;
    char** ptrPtr;
    char*  charPtr;

    stringNum = stringOffset[0];

    for(i=1 ; i<=stringNum ; i++)            // write char* allocated string
    {
        ptrPtr  = (char**) ( (char*)freebody + stringOffset[i] );
        charPtr = (char*)  *ptrPtr;

        if ( charPtr )
           mem_del(charPtr);
    }

}
//----------- END OF FUNCTION DynArray::free_ptr -------------//


//---------- Begin of function DynArray::quick_sort -------------//
//
// Perform a quick sort on the array.
//
// int(*fcmp)(const void*, const void*) cmpFun = the pointer to the comparsion function
//
void DynArray::quick_sort( int(*cmpFun)(const void*, const void*) )
{
   qsort( body_buf, last_ele, ele_size, cmpFun );

}
//------------- End of function DynArray::quick_sort --------------//



//---------- Begin of function DynArray::write_file -------------//
//
// Write current dynamic array into file,
// read_file() can be used to retrieve it.
//
// <File*> writeFile = the pointer to the writing file
//
// Return : 1 - write successfully
//          0 - writing error
//
int DynArray::write_file(File* filePtr)
{
   if( !filePtr->file_write( this, sizeof(DynArray) ) )
       return 0;

   if( last_ele > 0 )
   {
      if( !filePtr->file_write( body_buf, ele_size*last_ele ) )
         return 0;
   }

   return 1;
}
//------------- End of function DynArray::write_file --------------//



//---------- Begin of function DynArray::read_file -------------//
//
// Read a saved dynamic array from file, it must be saved with write_file()
//
// <File*> readFile = the pointer to the writing file
//
// Return : 1 - read successfully
//          0 - writing error
//
int DynArray::read_file(File* filePtr)
{
   char* bodyBuf = body_buf;     // preserve body_buf which has been allocated

   if( !filePtr->file_read( this, sizeof(DynArray) ) )
      return 0;

   body_buf = mem_resize( bodyBuf, ele_num*ele_size );

	if( last_ele > 0 )
	{
		if( !filePtr->file_read( body_buf, ele_size*last_ele ) )
			return 0;
	}

	start();    // go top

	return 1;
}
//------------- End of function DynArray::read_file --------------//


//--------- Begin of function DynArray::init_sort ---------//
//
// Initialize sorting parameters before using linkin_sort & resort
//
// <int>  sortOffset : offset of the sorting variable
// <char> sortType   : SORT_CHAR_PTR = <char*>
//                     SORT_CHAR_STR = <char[]>
//                     SORT_INT		 = <int>
//							  SORT_SHORT    = <short>
//							  SORT_CHAR     = <char>
//
void DynArray::init_sort(int sortOffset, char sortType)
{
	sort_offset = sortOffset;
	sort_type   = sortType;
}
//----------- End of function DynArray::init_sort ---------//



//------ BEGIN OF FUNCTION DynArray::linkin_sort_scan_from_bottom ----------//
//
// Description : linkin a entry to a sorted link list
//
// Note    : init_sort() must be called first, before using sorting function
// Warning : DynArrayB (version B) can't use this function
//
// Syntax      : linkin_sort_scan_from_bottom(<void*>,<int>,<char>)
//
// <void*> = the structure buffer pointer
//
// Note : use DynArray::seek() to search quickly if the whole DynArray is
//        built up with linkin_sort_scan_from_bottom()
//
// WARNING : After calling linkin() all pointers to the linklist body
//           should be updated, because mem_resize() will move the body memory
//
void DynArray::linkin_sort_scan_from_bottom(void* varPtr)
{
	err_when( sort_offset < 0 );

	int 	cmpRet;
	char* varData,*bodyChar;

	for( int recNo=last_ele ; recNo>0 ; recNo-- )
	{
		//-------- comparsion ---------//

		switch( sort_type )
		{
			case SORT_INT:				// <int>
				cmpRet   = *((int*)((char*)varPtr+sort_offset)) >
							  *((int*)((char*)get()+sort_offset));
				break;

			case SORT_SHORT:			// <short>
				cmpRet   = *((short*)((char*)varPtr+sort_offset)) >
							  *((short*)((char*)get()+sort_offset));
				break;

			case SORT_CHAR: 			// <char>
				cmpRet   = *((char*)((char*)varPtr+sort_offset)) >
							  *((char*)((char*)get()+sort_offset));
				break;

			case SORT_CHAR_PTR:     // <char*>
				varData  = *( (char**)((char*)varPtr + sort_offset) );
				bodyChar = *( (char**)((char*)get() + sort_offset) );

				err_when( !varData || !bodyChar );

				cmpRet   = strcmp( varData, bodyChar );
				break;

			case SORT_CHAR_STR:		// <char[]>
				varData  = (char*)varPtr + sort_offset ;
				bodyChar = (char*)get() + sort_offset ;

				err_when( !varData || !bodyChar );

				cmpRet   = strcmp( varData, bodyChar );
				break;
		}

		//---- if equal then linkin -----------//

		if( cmpRet >= 0 )
		{
			insert_at( last_ele+1, varPtr) ;
			return;
		}
	}

	insert_at( 1, varPtr);		// insert at the top
}
//--------- END OF FUNCTION DynArray::linkin_sort_scan_from_bottom ---------//


/* this function has bugs and is commented out.

//------ BEGIN OF FUNCTION DynArray::linkin_sort ----------//
//
// Description : linkin a entry to a sorted link list
//
// Note    : init_sort() must be called first, before using sorting function
// Warning : DynArrayB (version B) can't use this function
//
// Syntax      : linkin_sort(<void*>,<int>,<char>)
//
// <void*> = the structure buffer pointer
//
// Note : use DynArray::seek() to search quickly if the whole DynArray is
//        built up with linkin_sort()
//
// WARNING : After calling linkin() all pointers to the linklist body
//           should be updated, because mem_resize() will move the body memory
//
void DynArray::linkin_sort(void* varPtr)
{
	err_when( sort_offset < 0 );

	register int jumpStep,cmpRet;

	char* lastVar,*lastVar2;
   char* varData,*bodyChar;

   jumpStep=last_ele/2+1;

   go(jumpStep);

   lastVar = lastVar2 = NULL;

   //------- for binary comparsion linkin_sort -------------//

   for (;; )
   {
      //-------- comparsion ---------//

      switch( sort_type )
      {
         case 'I':
            cmpRet   = *((int*)((char*)varPtr+sort_offset)) ==
							  *((int*)((char*)get()+sort_offset));
				break;

         case 'P':       // char*
            varData  = *( (char**)((char*)varPtr + sort_offset) );
            bodyChar = *( (char**)((char*)get() + sort_offset) );

            err_when( !varData || !bodyChar );

            cmpRet   = strcmp( varData, bodyChar );
            break;

         case 'C':
            varData  = (char*)varPtr + sort_offset ;
            bodyChar = (char*)get() + sort_offset ;

            err_when( !varData || !bodyChar );

            cmpRet   = strcmp( varData, bodyChar );
				break;
      }

      //---- if equal then linkin -----------//

      if ( cmpRet ==0 )
      {
         linkin(varPtr) ;
         break;
		}

      //----- reach the end of the binary tree ----//

      if ( lastVar2 == bodyChar )
      {
         if ( cmpRet < 0 )
             bkwd();
         linkin(varPtr);
         break;
      }

		//----- search through the binary tree -----//

      if ( jumpStep%2 == 1 )
         jumpStep=jumpStep/2+1;
      else
			jumpStep/=2;

		if ( jumpStep < 1 )
			jumpStep = 1;

		if ( cmpRet < 0 )
			jump( -jumpStep );
		else
			jump( jumpStep );

		lastVar2 = lastVar;
		lastVar  = bodyChar;
	}

}
//--------- END OF FUNCTION DynArray::linkin_sort ---------//

//---------- Begin of function DynArray::resort -------------//
//
// When the content of a sorted recno is updated,
// call this function for renewing the sorting order
//
// <int> resortRecno = the pointer to the writing file
//
void DynArray::resort(int resortRecno)
{
   linkin_sort(get(resortRecno));       // linkin the record for a second time, for sorting

   go(resortRecno);             // delete the original record
   linkout(resortRecno);
}
//------------- End of function DynArray::resort --------------//

*/

