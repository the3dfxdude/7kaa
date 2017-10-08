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

//Filename    :: ODYNARRB.H
//Description :: Dynamic Array Object Version B

#ifndef __ODYNARRB_H
#define __ODYNARRB_H

#ifndef __ODYNARR_H
#include <ODYNARR.h>
#endif

class FileReaderVisitor;
class FileWriterVisitor;

//------------ Define Constant -------------//

#define DEFAULT_REUSE_INTERVAL_DAYS		3

//----------- Define variable type -----------//

typedef char* (*CreateEleFP)();

//--------- Define struct EmptyRoom -----------//

struct EmptyRoom
{
	short	recno;
	int   deleted_game_date;
};

//---------- Define class DynArrayB -----------//

class DynArrayB : public DynArray
{
public:
	EmptyRoom*  empty_room_array;
	short  		empty_room_num;  	// rooms allocated
	short  		empty_room_count;	// rooms used
	short			reuse_interval_days;

public:
	DynArrayB(int,int=DEF_DYNARRAY_BLOCK_SIZE,int reuseIntervalDays=0);
   ~DynArrayB();

   // packed_size()  is the size when the array is packed (deleted record are actually removed)
   // packed_recno() is the recno when the array is packed 

   int  packed_size() const  { return size() - empty_room_count; }
   int  packed_recno(int) const;	// Given the recno unpacked, it returns the recno packed.

   void linkin(const void*);
   void linkout(int= -1);
   void zap();

	template <typename T, typename Visitor>
	void accept_visitor_as_value_array(Visitor* v, void (*visitObj)(Visitor* v, T* obj), int recordSize);

	template <typename T, typename Visitor>
	void accept_visitor_as_ptr_array(Visitor* v, short (*getObjectId) (T* obj), T* (*createObj)(short), void (*visitObj)(Visitor* v, T* obj), int objectRecordSize);

	// Note: the below two functions are helpers for accept_visitor_as_ptr_array and, unfortunately, also called by UnitArray, because it uniquely has visits between the size() and the array visits.
	template <typename Visitor>
	void visit_array_size(Visitor* v);
	template <typename T, typename Visitor>
	void visit_ptr_array(Visitor* v, short (*getObjectId) (T* obj), T* (*createObj)(short), void (*visitObj)(Visitor* v, T* obj), int objectRecordSize);

	template <typename Visitor>
	void visit_empty_room_array(Visitor* v);
};

//---------------------------------------------//

#endif


