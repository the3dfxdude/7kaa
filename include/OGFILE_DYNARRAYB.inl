/*
* Seven Kingdoms: Ancient Adversaries
*
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

#ifndef OGFILE_DYNARRAYB_H_INCLUDED
#define OGFILE_DYNARRAYB_H_INCLUDED

#include <ODYNARRB.h>
#include <file_io_visitor.h>
#include <visitor_functions.h>


template <typename Visitor>
void visit_empty_room(Visitor* v, EmptyRoom* c)
{
	using namespace FileIOVisitor;
	visit<int16_t>(v, &c->recno);
	visit<int32_t>(v, &c->deleted_game_date);
}

template <typename Visitor>
void DynArrayB::visit_empty_room_array(Visitor* v)
{
	using namespace FileIOVisitor;
	visit_property<int, int16_t>(v, [this]() {return empty_room_count;},
		[this](int count) {
			empty_room_count = count;
			empty_room_num = count;
			empty_room_array = (EmptyRoom*)mem_resize(empty_room_array, count * sizeof(EmptyRoom));
		});

	if (empty_room_count > 0) {
		enum { EMPTY_ROOM_RECORD_SIZE = 6 };
		v->with_record_size(empty_room_count * EMPTY_ROOM_RECORD_SIZE);
		for (int i = 0; i < empty_room_count; ++i) {
			visit_empty_room(v, &empty_room_array[i]);
		}
	}
}

template <typename T, typename Visitor>
void DynArrayB::accept_visitor_as_value_array(Visitor* v, void (*visitObj)(Visitor* v, T* obj), int recordSize)
{
	using namespace FileIOVisitor;
	do_visit_as_value_array(v, visitObj, recordSize);

	visit_empty_room_array(v);

	if (is_reader_visitor(v))
		start();    // go top
}

template <typename T, typename Visitor>
void DynArrayB::accept_visitor_as_ptr_array(Visitor* v, short (*getObjectId) (T* obj), T* (*createObj)(short), void (*visitObj)(Visitor* v, T* obj), int objectRecordSize)
{
	visit_array_size(v);
	visit_ptr_array(v, getObjectId, createObj, visitObj, objectRecordSize);
}

template <typename T, typename Visitor>
void DynArrayB::visit_ptr_array(Visitor* v, short (*getObjectId) (T* obj), T* (*createObj)(short), void (*visitObj)(Visitor* v, T* obj), int objectRecordSize)
{
	using namespace FileIOVisitor;

	for (int i = 1; i <= size(); ++i)
	{
		int objectId;
		visit_property<int, int16_t>(v,
			[=, &objectId]() {
				void* const ptr = this->get_ptr(i);
				objectId = ptr ? getObjectId(static_cast<T*>(ptr)) : 0;
				return objectId;
			},
			[&objectId](int visitObjectId) {objectId = visitObjectId;});

		if (objectId)
		{
			if (is_reader_visitor(v))
			{
				*static_cast<T**>(get(i)) = createObj(objectId);
			}
			T* object = static_cast<T*>(get_ptr(i));
			v->with_record_size(objectRecordSize);
			visitObj(v, object);
		}
	}

	if (is_reader_visitor(v))
	{
		//-------- linkout() those record added by add_blank() ----------//
		//-- So they will be marked deleted in DynArrayB and can be -----//
		//-- undeleted and used when a new record is going to be added --//

		for (int i = size(); i > 0; --i)
		{
			if (get_ptr(i) == nullptr) // (add_blank() record)
				linkout(i);
		}
	}

	visit_empty_room_array(v);
}

template <typename Visitor>
void DynArrayB::visit_array_size(Visitor* v)
{
	using namespace FileIOVisitor;
	visit_property<int, int16_t>(v, this, &DynArray::size,
		[this](int size) {
		err_when(this->size() != 0); // i.e. visit should only be done on a 'fresh' instance
		add_blank(size);
	});
}


// Helper for getObjectId argument of accept_visitor_as_ptr_array, for DynArrayB's that just need to know if the object was present or not,
// rather than what ID the object has, in order to recreate the array values.
template <typename T>
short yes_or_no_object_id (T* obj)
{
	return !!obj;
}

#endif
