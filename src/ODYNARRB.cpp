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

//Filename    : ODYNARRB.CPP
//Description : Object Dynamic Array Version B

#include <OINFO.h>
#include <ODYNARRB.h>
#include <dbglog.h>
#include <file_io_visitor.h>

using namespace FileIOVisitor;

DBGLOG_DEFAULT_CHANNEL(DynArray);

//----------------------------------------------------------//
//
// Version B is different from Version A as :
//
// - when linkout() it doesn't not physically shift the memory
//   upwards.
//
// - when linkin() it will search for empty rooms in the array
//   before appending space at the end of the array.
//
//----------------------------------------------------------//


#define EMPTY_ROOM_ALLOC_STEP    5


//--------- BEGIN OF FUNCTION DynArrayB::DynArrayB -------//
//
// <int> eleSize  = size of each element
// [int] blockNum = number of entity of each block of element
//                       increased ( default : 30 )
// [int] reuseIntervalDays = no. of game days deleted records needed to be kept before reusing them.
//									  (default: 0)
//
DynArrayB::DynArrayB(int eleSize,int blockNum,int reuseIntervalDays) : DynArray(eleSize, blockNum)
{
	empty_room_array = NULL;
	empty_room_num   = 0;
	empty_room_count = 0;

	reuse_interval_days = reuseIntervalDays;
}
//----------- END OF FUNCTION DynArrayB::DynArrayB -----//


//--------- BEGIN OF FUNCTION DynArrayB::~DynArrayB -------//
//
DynArrayB::~DynArrayB()
{
	if( empty_room_array )
		mem_del( empty_room_array );
}
//----------- END OF FUNCTION DynArrayB::DynArrayB -----//


//---------- BEGIN OF FUNCTION DynArrayB::linkin -----------//
//
// <void*> ent
// <int>
//
// - when linkin() it will search for empty rooms in the array
//   before appending space at the end of the array.
//
// - If found, then it will use that room
//
// - Otherwise, it will link a record at the END of the array
//
// WARNING : After calling linkin() all pointers to the linklist body
//           should be updated, because mem_resize() will move the body memory
//
void DynArrayB::linkin(const void* ent)
{
	//------- detect for empty rooms --------//

	int reusedFlag=0;

	if( empty_room_count > 0 )
	{
		if( reuse_interval_days )
		{
			//------ first in, first out approach -----//

			if( info.game_date >= empty_room_array[0].deleted_game_date + reuse_interval_days )
			{
				cur_pos = empty_room_array[0].recno;

				memmove( empty_room_array, empty_room_array+1, sizeof(empty_room_array[0]) * (empty_room_count-1) );

				empty_room_count--;
				reusedFlag = 1;
			}
		}
		else
		{
			//------ last in, first out approach -----//

			cur_pos = empty_room_array[empty_room_count-1].recno;

			empty_room_count--;
			reusedFlag = 1;
		}
	}

	if( !reusedFlag )
	{
		last_ele++;
		cur_pos=last_ele;
	}

	//---------- regular link in -----------//

	if ( last_ele > ele_num ) // not enough empty element left to hold the new entity
		resize( ele_num + block_num );

	if ( ent )
		memcpy(body_buf+(cur_pos-1)*ele_size, ent, ele_size );
	else
		*(body_buf+(cur_pos-1)*ele_size) = '\0';
}
//---------- END OF FUNCTION DynArrayB::linkin ------------//



//----------- BEGIN OF FUNCTION DynArrayB::linkout ---------//
//
// - when linkout() it doesn't not physically shift the memory
//   upwards.
//
// - it record the address of this empty room at empty room list
//
// [int] delPos = the position (recno) of the item to be deleted
//                ( default : recno() current record no. )
//
void DynArrayB::linkout(int delPos)
{
	if( delPos < 0 )
		delPos = cur_pos;

	if( delPos == 0 || delPos > last_ele )
		return;

	//-------- add to the empty room list ---------//

	if( ++empty_room_count > empty_room_num )
	{
		empty_room_array = (EmptyRoom*) mem_resize( empty_room_array,
								 (empty_room_num+EMPTY_ROOM_ALLOC_STEP) * sizeof(*empty_room_array) );

		empty_room_num  += EMPTY_ROOM_ALLOC_STEP;
	}

	empty_room_array[empty_room_count-1].recno = delPos;
	empty_room_array[empty_room_count-1].deleted_game_date = info.game_date;

	memset( body_buf+(delPos-1)*ele_size, 0, ele_size );
}
//------------ END OF FUNCTION DynArrayB::linkout ----------//


//---------- Begin of function DynArrayB::packed_recno -------------//
//
// Given the recno unpacked, it returns the recno packed.
//
// packed_recno() is the recno when the array is packed
//                (deleted record are actually removed)
//
// <int> recNo = given the recno unpacked
//
// return : <int> the recno when packed
//
int DynArrayB::packed_recno(int recNo) const
{
	int i, packedRecno = recNo;

	for( i=0 ; i<empty_room_count ; i++ )
	{
		if( empty_room_array[i].recno < recNo )
			packedRecno--;
	}

	return packedRecno;
}
//------------- End of function DynArrayB::packed_recno --------------//


//---------- Begin of function DynArrayB::zap -------------//
//
// Zap the whole dynamic array, clear all elements
//
void DynArrayB::zap()
{
	DynArray::zap();

	empty_room_count=0;	    // reset empty rooms
}
//------------- End of function DynArrayB::zap --------------//
