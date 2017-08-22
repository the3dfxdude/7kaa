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

#ifndef OGFILE_DYNARRAY_H_INCLUDED
#define OGFILE_DYNARRAY_H_INCLUDED

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

template <typename Visitor, typename T>
void DynArrayB::accept_visitor_as_ptr_array(Visitor* v, T* (*create_obj)(), void (*visit_obj)(Visitor* v, T* obj), int objectRecordSize)
{
	using namespace FileIOVisitor;
	visit_property<int, int16_t>(v, this, &DynArray::size,
		[this](int size) {
			err_when(this->size() != 0); // i.e. visit should only be done on a 'fresh' instance
			add_blank(size);
		});

	for (int i = 1; i <= size(); ++i)
	{
		bool present;
		visit_property<bool, int16_t>(v, [=, &present]() {return (present = (get_ptr(i) != nullptr));}, [&present](bool visitPresent) {present = visitPresent;});

		if (present)
		{
			if (is_reader_visitor(v))
			{
				*static_cast<T**>(get(i)) = create_obj();
			}
			T* object = static_cast<T*>(get_ptr(i));
			v->with_record_size(objectRecordSize);
			visit_obj(v, object);
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

#endif
