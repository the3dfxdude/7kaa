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

#include <OREBEL.h>
#include <file_io_visitor.h>
#include <OGFILE_DYNARRAYB.inl>

#include <vector>

using namespace FileIOVisitor;


template <typename Visitor>
static void visit_rebel(Visitor* v, Rebel* c)
{
	visit<int16_t>(v, &c->rebel_recno);
	visit<int16_t>(v, &c->leader_unit_recno);
	visit<int8_t>(v, &c->action_mode);
	visit<int16_t>(v, &c->action_para);
	visit<int16_t>(v, &c->action_para2);
	visit<int16_t>(v, &c->mobile_rebel_count);
	visit<int16_t>(v, &c->town_recno);
	visit<int8_t>(v, &c->hostile_nation_bits);
}

enum { REBEL_RECORD_SIZE = 14 };

static Rebel* create_rebel_func(short)
{
	return new Rebel;
}

//-------- Start of function RebelArray::write_file -------------//
//
int RebelArray::write_file(File* filePtr)
{
	FileWriterVisitor v(filePtr);
	accept_visitor_as_ptr_array(&v, yes_or_no_object_id<Rebel>, create_rebel_func, visit_rebel<FileWriterVisitor>, REBEL_RECORD_SIZE);
	return v.good();
}
//--------- End of function RebelArray::write_file ---------------//


//-------- Start of function RebelArray::read_file -------------//
//
int RebelArray::read_file(File* filePtr)
{
	FileReaderVisitor v(filePtr);
	accept_visitor_as_ptr_array(&v, yes_or_no_object_id<Rebel>, create_rebel_func, visit_rebel<FileReaderVisitor>, REBEL_RECORD_SIZE);
	return v.good();
}
//--------- End of function RebelArray::read_file ---------------//
