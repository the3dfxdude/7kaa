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

//------------ Define Constant -------------//

#define DEFAULT_REUSE_INTERVAL_DAYS		3

//----------- Define variable type -----------//

typedef char* (*CreateEleFP)();

//--------- Define struct EmptyRoom -----------//

#pragma pack(1)
struct EmptyRoom
{
	short	recno;
	int   deleted_game_date;
};
#pragma pack()

//---------- Define class DynArrayB -----------//

#pragma pack(1)
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

   void linkin(void*);
   void linkout(int= -1);
   void zap();

	int  write_file(File*);    	// Write current dynamic array to file
	int  read_file(File*);   	  	// Read dynamic array from file

	int  write_empty_room(File*);    // Write current dynamic array to file
	int  read_empty_room(File*);     // Read dynamic array from file

	int  write_ptr_array(File*, int);
	int  read_ptr_array(File*, int, CreateEleFP);
};
#pragma pack()

//---------------------------------------------//

#endif


