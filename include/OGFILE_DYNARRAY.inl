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

#include <ODYNARR.h>
#include <file_io_visitor.h>
#include <visitor_functions.h>
#include <visit_dyn_array.h>


template <typename T, typename Visitor>
void DynArray::do_visit_as_value_array(Visitor* v, void (*visit_obj)(Visitor* v, T* obj), int elementRecordSize)
{
	using namespace FileIOVisitor;

	v->with_record_size(DYN_ARRAY_RECORD_SIZE);
	visit_dyn_array(v, this, elementRecordSize);

	if (is_reader_visitor(v))
	{
		body_buf = mem_resize(body_buf, ele_num * ele_size);
	}

	if (last_ele > 0)
	{
		v->with_record_size(last_ele * elementRecordSize);
		for (int i = 1; i <= last_ele; ++i)
		{
			visit_obj(v, static_cast<T*>(get(i)));
		}
	}
}

template <typename T, typename Visitor>
void DynArray::accept_visitor_as_value_array(Visitor* v, void (*visit_obj)(Visitor* v, T* obj), int recordSize)
{
	using namespace FileIOVisitor;

	do_visit_as_value_array(v, visit_obj, recordSize);

	if (is_reader_visitor(v))
		start();    // go top
}

#endif
