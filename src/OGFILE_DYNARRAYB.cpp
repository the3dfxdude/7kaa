/*
* Seven Kingdoms: Ancient Adversaries
*
* Copyright 1997,1998 Enlight Software Ltd.
* Copyright 2017 Richard Dijk <microvirus.multiplying@gmail.com>
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

#include <ODYNARRB.h>
#include <file_io_visitor.h>

#include <vector>

using namespace FileIOVisitor;


template <typename Visitor>
static void visit_dyn_array_b(Visitor *v, DynArrayB *dab)
{
	/* DynArray */
	visit<int32_t>(v, &dab->ele_num);
	visit<int32_t>(v, &dab->block_num);
	visit<int32_t>(v, &dab->cur_pos);
	visit<int32_t>(v, &dab->last_ele);
	visit<int32_t>(v, &dab->ele_size);
	visit<int32_t>(v, &dab->sort_offset);
	visit<int8_t>(v, &dab->sort_type);
	v->skip(4); /* dab->body_buf */

	/* Not reading DynArrayB members */
}

enum { DYN_ARRAY_B_RECORD_SIZE = 29 };

//---------- Begin of function DynArrayB::write_file -------------//
//
// Write current dynamic array into file,
// read_file() can be used to retrieve it.
//
// <File*> writeFile = the pointer to the writing file
//
// Return : 1 - write successfully
//          0 - writing error
//
int DynArrayB::write_file(File* filePtr)
{
	if (!visit_with_record_size<FileWriterVisitor>(filePtr, this, &visit_dyn_array_b<FileWriterVisitor>, DYN_ARRAY_B_RECORD_SIZE))
		return 0;

	//---------- write body_buf ---------//

	if( last_ele > 0 )
	{
		if( !filePtr->file_write( body_buf, ele_size*last_ele ) )
			return 0;
	}

	//---------- write empty_room_array ---------//

	write_empty_room(filePtr);

	return 1;
}
//------------- End of function DynArrayB::write_file --------------//


//---------- Begin of function DynArrayB::read_file -------------//
//
// Read a saved dynamic array from file, it must be saved with write_file()
//
// <File*> readFile = the pointer to the writing file
//
// Return : 1 - read successfully
//          0 - writing error
//
int DynArrayB::read_file(File* filePtr)
{
	if (!visit_with_record_size<FileReaderVisitor>(filePtr, this, &visit_dyn_array_b<FileReaderVisitor>, DYN_ARRAY_B_RECORD_SIZE))
		return 0;

	//---------- read body_buf ---------//

	this->body_buf = mem_resize(this->body_buf, this->ele_num*this->ele_size);

	if( last_ele > 0 )
	{
		if( !filePtr->file_read( body_buf, ele_size*last_ele ) )
			return 0;
	}

	//---------- read empty_room_array ---------//

	read_empty_room(filePtr);

	//------------------------------------------//

	start();    // go top

	return 1;
}
//------------- End of function DynArrayB::read_file --------------//


//---------- Begin of function DynArrayB::write_empty_room -------------//
//
// Write current dynamic array into file,
// read_file() can be used to retrieve it.
//
// <File*> writeFile = the pointer to the writing file
//
// Return : 1 - write successfully
//          0 - writing error
//
int DynArrayB::write_empty_room(File* filePtr)
{
	filePtr->file_put_short( empty_room_count );

	//---------- write empty_room_array ---------//

	if( empty_room_count > 0 )
	{
		if( !filePtr->file_write( empty_room_array,
			sizeof(EmptyRoom) * empty_room_count ) )
		{
			return 0;
		}
	}

	return 1;
}
//------------- End of function DynArrayB::write_empty_room --------------//


//---------- Begin of function DynArrayB::read_empty_room -------------//
//
// Read a saved dynamic array from file, it must be saved with write_file()
//
// <File*> readFile = the pointer to the writing file
//
// Return : 1 - read successfully
//          0 - writing error
//
int DynArrayB::read_empty_room(File* filePtr)
{
	empty_room_num = empty_room_count = filePtr->file_get_short();		// set both to the same

																		//---------- read empty_room_array ---------//

	if( empty_room_count > 0 )
	{
		empty_room_array = (EmptyRoom*) mem_resize( empty_room_array,
			sizeof(EmptyRoom) * empty_room_count );

		if( !filePtr->file_read( empty_room_array,
			sizeof(*empty_room_array) * empty_room_count ) )
		{
			return 0;
		}
	}
	else // when empty_room_count == 0
	{
		if( empty_room_array )
		{
			mem_del( empty_room_array );
			empty_room_array = NULL;
		}
	}

	//------------------------------------------//

	return 1;
}
//------------- End of function DynArrayB::read_empty_room --------------//
