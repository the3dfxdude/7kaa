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

using namespace FileIOVisitor;


template <typename Visitor>
static void visit_tornado(Visitor *v, Tornado *t)
{
	visit_sprite_members(v, t);
	visit<float>(v, &t->attack_damage);
	visit<int16_t>(v, &t->life_time);
	visit<int16_t>(v, &t->dmg_offset_x);
	visit<int16_t>(v, &t->dmg_offset_y);
}

enum { TORNADO_RECORD_SIZE = 44 };

//--------- Begin of function Tornado::write_file ---------//
//
int Tornado::write_file(File* filePtr)
{
	return visit_with_record_size<FileWriterVisitor>(filePtr, this, &visit_tornado<FileWriterVisitor>,
		TORNADO_RECORD_SIZE);
}
//----------- End of function Tornado::write_file ---------//

//--------- Begin of function Tornado::read_file ---------//
//
int Tornado::read_file(File* filePtr)
{
	if (!visit_with_record_size<FileReaderVisitor>(filePtr, this, &visit_tornado<FileReaderVisitor>,
		TORNADO_RECORD_SIZE))
		return 0;

	//------------ post-process the data read ----------//

	sprite_info = sprite_res[sprite_id];
	sprite_info->load_bitmap_res();

	return 1;
}
//----------- End of function Tornado::read_file ---------//



//-------- Start of function TornadoArray::write_file -------------//
//
int TornadoArray::write_file(File* filePtr)
{
	filePtr->file_put_short(restart_recno);  // variable in SpriteArray

	int    i;
	Tornado *tornadoPtr;

	filePtr->file_put_short( size() );  // no. of tornados in tornado_array

	for( i=1; i<=size() ; i++ )
	{
		tornadoPtr = (Tornado*) get_ptr(i);

		//----- write tornadoId or 0 if the tornado is deleted -----//

		if( !tornadoPtr )    // the tornado is deleted
		{
			filePtr->file_put_short(0);
		}
		else
		{
			filePtr->file_put_short(1);      // there is a tornado in this record

											 //------ write data in the base class ------//

			if( !tornadoPtr->write_file(filePtr) )
				return 0;
		}
	}

	//------- write empty room array --------//

	{
		FileWriterVisitor v(filePtr);
		visit_empty_room_array(&v);
	}

	return 1;
}
//--------- End of function TornadoArray::write_file -------------//


//-------- Start of function TornadoArray::read_file -------------//
//
int TornadoArray::read_file(File* filePtr)
{
	restart_recno    = filePtr->file_get_short();

	int     i, tornadoRecno, tornadoCount;
	Tornado* tornadoPtr;

	tornadoCount = filePtr->file_get_short();  // get no. of tornados from file

	for( i=1 ; i<=tornadoCount ; i++ )
	{
		if( filePtr->file_get_short() == 0 )
		{
			add_blank(1);     // it's a DynArrayB function
		}
		else
		{
			//----- create tornado object -----------//

			tornadoRecno = tornado_array.create_tornado();
			tornadoPtr   = tornado_array[tornadoRecno];

			//----- read data in base class --------//

			if( !tornadoPtr->read_file( filePtr ) )
				return 0;
		}
	}

	//-------- linkout() those record added by add_blank() ----------//
	//-- So they will be marked deleted in DynArrayB and can be -----//
	//-- undeleted and used when a new record is going to be added --//

	for( i=size() ; i>0 ; i-- )
	{
		DynArrayB::go(i);             // since TornadoArray has its own go() which will call GroupArray::go()

		if( get_ptr() == NULL )       // add_blank() record
			linkout();
	}

	//------- read empty room array --------//

	{
		FileReaderVisitor v(filePtr);
		visit_empty_room_array(&v);
	}

	return 1;
}
//--------- End of function TornadoArray::read_file ---------------//
