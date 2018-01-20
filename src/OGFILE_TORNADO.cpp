/*
* Seven Kingdoms: Ancient Adversaries
*
* Copyright 1997,1998 Enlight Software Ltd.
* Copyright 2010 Unavowed <unavowed@vexillium.org>
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

#include <OTORNADO.h>
#include <file_io_visitor.h>
#include <visit_sprite.h>
#include <visitor_functions.h>
#include <OGFILE_DYNARRAYB.inl>

using namespace FileIOVisitor;


template <typename Visitor>
static void visit_tornado(Visitor *v, Tornado *t)
{
	visit_sprite_members(v, t);
	visit<float>(v, &t->attack_damage);
	visit<int16_t>(v, &t->life_time);
	visit<int16_t>(v, &t->dmg_offset_x);
	visit<int16_t>(v, &t->dmg_offset_y);

	if (is_reader_visitor(v))
	{
		t->sprite_info = sprite_res[t->sprite_id];
		t->sprite_info->load_bitmap_res();
	}
}

template <typename Visitor>
static void visit_tornado_array(Visitor *v, TornadoArray *c)
{
	enum { TORNADO_RECORD_SIZE = 44 };
	visit<int16_t>(v, &c->restart_recno);
	c->accept_visitor_as_ptr_array<Tornado>(v, yes_or_no_object_id<Tornado>, [](short) {return new Tornado;}, visit_tornado<Visitor>, TORNADO_RECORD_SIZE);
}

//-------- Start of function TornadoArray::write_file -------------//
//
int TornadoArray::write_file(File* filePtr)
{
	FileWriterVisitor v(filePtr);
	visit_tornado_array(&v, this);
	return v.good();
}
//--------- End of function TornadoArray::write_file -------------//


//-------- Start of function TornadoArray::read_file -------------//
//
int TornadoArray::read_file(File* filePtr)
{
	FileReaderVisitor v(filePtr);
	visit_tornado_array(&v, this);
	return v.good();
}
//--------- End of function TornadoArray::read_file ---------------//
