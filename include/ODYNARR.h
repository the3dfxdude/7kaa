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

//Filename    :: ODYNARR.H
//Description :: Dynamic Array Object

#ifndef __ODYNARR_H
#define __ODYNARR_H

#ifndef __ALL_H
#include <ALL.h>
#endif

#ifndef __STRING_H
#include <string.h>
#endif


//--------- Define constant ------------//

#define DEF_DYNARRAY_BLOCK_SIZE 30	// default allocation block size (no. of unities each block has) 

//---------- Define sort types --------//

enum { SORT_INT=1,
		 SORT_SHORT,
		 SORT_CHAR,
		 SORT_CHAR_PTR,
		 SORT_CHAR_STR };


//-------- BEGIN OF CLASS DynArrary ---------//

class DynArray
{
public :

   int  ele_num;            // the size of the whole array
   int  block_num;          // the number of element of each block
   int  cur_pos;            // current position
   int  last_ele;           // last element position ( the array is not fully used )
   int  ele_size;           // the size of each element

   int  sort_offset;
   char sort_type;

   char* body_buf;	    // cur_pos and last_ele are start from 1 (not 0)

   //----------------------------------------------//

public :

   DynArray(int,int=DEF_DYNARRAY_BLOCK_SIZE);
   ~DynArray();

   void  resize(int);

   void  linkin(const void*);
	void  linkin_unique(const void*);
	void  linkout(int= -1);
   void  update(const void*, int= -1);
   void  insert(const void*);
	void  insert_at(int,const void*);
	void  add_blank(int);

   void  init_sort(int,char);
	void  linkin_sort_scan_from_bottom(void*);
// void  resort(int);

   void* get();
   void* get(int);
   void* get_ptr() const;
   void* get_ptr(int) const;
	void  read(void*);

   int   check_pos();

   void  push(const void*);
   void  pop(void* =0);

   void  start();
   void  end();
   int   fwd();
   int   bkwd();

   void  jump(int);
   void  go(int);
   int   recno();
   int   size() const;

   int   is_start();
   int   is_end();

   int   scan_whole(void*);
   int   scan(void*,int,char,int=0);
   int   compare(void*,int,char);

   void  quick_sort( int(*cmpFun)(const void*, const void*) );

   void  clean_up(int* =0);
   void  free_ptr(void*,int*);
   void  zap(int resizeFlag=1);

   template <typename T, typename Visitor>
   void accept_visitor_as_value_array(Visitor* v, void (*visit_obj)(Visitor* v, T* obj), int recordSize);

protected:
   template <typename T, typename Visitor>
   void do_visit_as_value_array(Visitor* v, void (*visit_obj)(Visitor* v, T* obj), int elementRecordSize);
};

//--------- END OF CLASS DynArray ---------//


//--------- BEGIN OF FUNCTION DynArray::get -----------//
//
// Return : the memory pointer to the body_buf of current element
//          NULL if the record no. is invalid.
//
inline void* DynArray::get()
{
   if( cur_pos == 0 )
      return NULL;

   return (void*) (body_buf+(cur_pos-1)*ele_size);
}

inline void* DynArray::get(int specRec)
{
   if( specRec<1 || specRec>last_ele )
      return NULL;

   return (void*) (body_buf+(specRec-1)*ele_size);
}

//--------- END OF FUNCTION DynArray::get -----------//


//--------- BEGIN OF FUNCTION DynArray::get_ptr -----------//
//
// The content of the entry is a pointer, return the content
// is a pointer
//
// Return : the pointer
//          NULL if the record no. is invalid.

inline void* DynArray::get_ptr() const
{
   if( cur_pos == 0 )
      return NULL;

   return (void*) *((char**)(body_buf+(cur_pos-1)*ele_size));
}

inline void* DynArray::get_ptr(int specRec) const
{
   if( specRec < 1 || specRec > last_ele )
      return NULL;

   return (void*) *((char**)(body_buf+(specRec-1)*ele_size));
}

//--------- END OF FUNCTION DynArray::get_ptr -----------//


//--------- BEGIN OF FUNCTION DynArray::read -----------//
//
// Read current record into the given buffer
//
inline void DynArray::read(void* ent)
{
   if( ent )
      memcpy(ent, get(), ele_size );
}
//---------- END OF FUNCTION DynArray::read -----------//



//--------- BEGIN OF FUNCTIONO DynArray::push,pop -----------//

// <void*> ent = the address of the entity to be linked into the array

inline void DynArray::push(const void* ent)
{
   linkin(ent);
}

// [void*] ent = the address of the entity to be overwritten by current element

inline void DynArray::pop(void* ent)
{
   end();
   read(ent);
   linkout();
}

//----------- END OF FUNCTION DynArray::push,pop ----------//


//-------- BEGIN OF FUNCTIONO DynArray::start,end,fwd,bkwd -------------//
//
inline void DynArray::start()
{
   cur_pos = MIN(1,last_ele);
}

inline void DynArray::end()
{
   cur_pos = last_ele;
}

inline int DynArray::fwd()
{
   if (cur_pos < last_ele )
   {
      cur_pos++;
      return 1;
   }
   else
      return 0;
}

inline int DynArray::bkwd()
{
   if (cur_pos > 1)
   {
      cur_pos--;
      return 1;
   }
   else
      return 0;
}

//---------- END OF FUNCTION DynArray::start,end,fwd,bkwd ---------//


//--------- BEGIN OF FUNCTION DynArray::jump,go,pos,size ----------//


inline void DynArray::jump(int step)
{
   cur_pos+=step;

   if ( cur_pos < 0 )
      cur_pos = MIN(1,last_ele) ;

   if ( cur_pos > last_ele )
      cur_pos = last_ele;
}


inline void DynArray::go(int desPos)
{
   if ( desPos >= 1 && desPos <= last_ele )
      cur_pos = desPos;
}

inline int DynArray::recno()
{
   return cur_pos;
}

inline int DynArray::size() const
{
   return last_ele;
}

//----------- END OF FUNCTION DynArray::jump,go,pos,size ---------//


//-------- BEGIN OF FUNCTION DynArray::isstart,isend ------//

inline int DynArray::is_start()
{
   return( cur_pos <= 1 );
}

inline int DynArray::is_end()
{
   return( cur_pos >= last_ele );
}

//-------- END OF FUNCTION DynArray::isstart,isend --------//



#endif
