/*
* Seven Kingdoms: Ancient Adversaries
*
* Copyright 1997,1998 Enlight Software Ltd.
* Copyright 2010 Unavowed <unavowed@vexillium.org>
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


#include <OFIRM.h>
#include <OGFILE.h>
#include <OGF_V1.h>
#include <file_io_visitor.h>
#include <dbglog.h>

using namespace FileIOVisitor;

DBGLOG_DEFAULT_CHANNEL(GameFile);


template <typename Visitor>
static void visit_firm_members(Visitor *v, Firm *f)
{
	v->skip(4); /* virtual table pointer */

	visit<int8_t>(v, &f->firm_id);
	visit<int16_t>(v, &f->firm_build_id);
	visit<int16_t>(v, &f->firm_recno);
	visit<int8_t>(v, &f->firm_ai);
	visit<int8_t>(v, &f->ai_processed);
	visit<int8_t>(v, &f->ai_status);
	visit<int8_t>(v, &f->ai_link_checked);
	visit<int8_t>(v, &f->ai_sell_flag);
	visit<int8_t>(v, &f->race_id);
	visit<int16_t>(v, &f->nation_recno);
	visit<int16_t>(v, &f->closest_town_name_id);
	visit<int16_t>(v, &f->firm_name_instance_id);
	visit<int16_t>(v, &f->loc_x1);
	visit<int16_t>(v, &f->loc_y1);
	visit<int16_t>(v, &f->loc_x2);
	visit<int16_t>(v, &f->loc_y2);
	visit<int16_t>(v, &f->abs_x1);
	visit<int16_t>(v, &f->abs_y1);
	visit<int16_t>(v, &f->abs_x2);
	visit<int16_t>(v, &f->abs_y2);
	visit<int16_t>(v, &f->center_x);
	visit<int16_t>(v, &f->center_y);
	visit<uint8_t>(v, &f->region_id);
	visit<int8_t>(v, &f->cur_frame);
	visit<int8_t>(v, &f->remain_frame_delay);
	visit<float>(v, &f->hit_points);
	visit<float>(v, &f->max_hit_points);
	visit<int8_t>(v, &f->under_construction);
	visit<int8_t>(v, &f->firm_skill_id);
	visit<int16_t>(v, &f->overseer_recno);
	visit<int16_t>(v, &f->overseer_town_recno);
	visit<int16_t>(v, &f->builder_recno);
	visit<uint8_t>(v, &f->builder_region_id);
	visit<float>(v, &f->productivity);
	visit_pointer(v, &f->worker_array);
	visit<int8_t>(v, &f->worker_count);
	visit<int8_t>(v, &f->selected_worker_id);
	visit<int8_t>(v, &f->player_spy_count);
	visit<uint8_t>(v, &f->sabotage_level);
	visit<int8_t>(v, &f->linked_firm_count);
	visit<int8_t>(v, &f->linked_town_count);
	visit_array<int16_t>(v, f->linked_firm_array);
	visit_array<int16_t>(v, f->linked_town_array);

	visit_array<int8_t>(v, f->linked_firm_enable_array);

	visit_array<int8_t>(v, f->linked_town_enable_array);

	visit<float>(v, &f->last_year_income);
	visit<float>(v, &f->cur_year_income);
	visit<int32_t>(v, &f->setup_date);
	visit<int8_t>(v, &f->should_set_power);
	visit<int32_t>(v, &f->last_attacked_date);
	visit<int8_t>(v, &f->should_close_flag);
	visit<int8_t>(v, &f->no_neighbor_space);
	visit<int8_t>(v, &f->ai_should_build_factory_count);
}

void Firm::accept_file_visitor(FileReaderVisitor* v)
{
	visit_firm_members(v, this);
}

void Firm::accept_file_visitor(FileWriterVisitor* v)
{
	visit_firm_members(v, this);
}

enum { FIRM_RECORD_SIZE = 254 };

template <typename Visitor>
static bool visit_firm(File* file, Firm* firm, uint16_t record_size)
{
	Visitor v(file);
	v.with_record_size(record_size);
	firm->accept_file_visitor(&v);

	return v.good();
}

//-------- Start of function FirmArray::write_file -------------//
//
int FirmArray::write_file(File* filePtr)
{
	int  i;
	Firm *firmPtr;

	filePtr->file_put_short( size()  );  // no. of firms in firm_array
	filePtr->file_put_short( process_recno );
	filePtr->file_put_short( selected_recno );

	filePtr->file_put_short( Firm::firm_menu_mode );
	filePtr->file_put_short( Firm::action_spy_recno );
	filePtr->file_put_short( Firm::bribe_result );
	filePtr->file_put_short( Firm::assassinate_result );

	for( i=1; i<=size() ; i++ )
	{
		firmPtr = (Firm*) get_ptr(i);

		//----- write firmId or 0 if the firm is deleted -----//

		if( !firmPtr )    // the firm is deleted
		{
			filePtr->file_put_short(0);
		}
		else
		{
			//--------- write firm_id -------------//

			filePtr->file_put_short(firmPtr->firm_id);

			//------ write data in base class --------//

			if (!visit_firm<FileWriterVisitor>(filePtr, firmPtr, FIRM_RECORD_SIZE))
				return 0;

			//--------- write worker_array ---------//

			if( firmPtr->worker_array )
			{
				if( !filePtr->file_write( firmPtr->worker_array, MAX_WORKER*sizeof(Worker) ) )
					return 0;
			}

			//------ write data in derived class ------//

			if( !firmPtr->write_derived_file(filePtr) )
				return 0;
		}
	}

	//------- write empty room array --------//

	write_empty_room(filePtr);

	return 1;
}
//--------- End of function FirmArray::write_file ---------------//

//-------- Start of function FirmArray::read_file -------------//
//
int FirmArray::read_file(File* filePtr)
{
	Firm*   firmPtr;
	int     i, firmId, firmRecno;

	int firmCount      = filePtr->file_get_short();  // get no. of firms from file
	process_recno      = filePtr->file_get_short();
	selected_recno     = filePtr->file_get_short();

	Firm::firm_menu_mode  	 = (char) filePtr->file_get_short();
	Firm::action_spy_recno   = filePtr->file_get_short();
	Firm::bribe_result    	 = (char) filePtr->file_get_short();
	Firm::assassinate_result = (char) filePtr->file_get_short();

	for( i=1 ; i<=firmCount ; i++ )
	{
		firmId = filePtr->file_get_short();

		if( firmId==0 )  // the firm has been deleted
		{
			add_blank(1);     // it's a DynArrayB function
		}
		else
		{
			//----- create firm object -----------//

			firmRecno = create_firm( firmId );
			firmPtr   = firm_array[firmRecno];

			if (!visit_firm<FileReaderVisitor>(filePtr, firmPtr, FIRM_RECORD_SIZE))
				return 0;

			//---- read data in base class -----//

			if(!GameFile::read_file_same_version && firmPtr->firm_id > FIRM_BASE)
				firmPtr->firm_build_id += MAX_RACE - VERSION_1_MAX_RACE;

			//--------- read worker_array ---------//

			if( firm_res[firmId]->need_worker )
			{
				firmPtr->worker_array = (Worker*) mem_add( MAX_WORKER*sizeof(Worker) );

				if( !filePtr->file_read( firmPtr->worker_array, MAX_WORKER*sizeof(Worker) ) )
					return 0;
			}

			//----- read data in derived class -----//

			if( !firmPtr->read_derived_file( filePtr ) )
				return 0;
		}
	}

	//-------- linkout() those record added by add_blank() ----------//
	//-- So they will be marked deleted in DynArrayB and can be -----//
	//-- undeleted and used when a new record is going to be added --//

	for( i=size() ; i>0 ; i-- )
	{
		DynArrayB::go(i);             // since FirmArray has its own go() which will call GroupArray::go()

		if( get_ptr() == NULL )       // add_blank() record
			linkout();
	}

	//------- read empty room array --------//

	read_empty_room(filePtr);

	return 1;
}
//--------- End of function FirmArray::read_file ---------------//


//--------- Begin of function Firm::write_derived_file ---------//
//
// Write data in derived class.
//
// If the derived Firm don't have any special data,
// just use Firm::write_file(), otherwise make its own derived copy of write_file()
//
int Firm::write_derived_file(File* filePtr)
{
	//--- write data in derived class -----//

	int writeSize = firm_array.firm_class_size(firm_id)-sizeof(Firm);

	if( writeSize > 0 )
	{
		if( !filePtr->file_write( (char*) this + sizeof(Firm), writeSize ) )
			return 0;
	}

	return 1;
}
//----------- End of function Firm::write_derived_file ---------//


//--------- Begin of function Firm::read_derived_file ---------//
//
// Read data in derived class.
//
// If the derived Firm don't have any special data,
// just use Firm::read_file(), otherwise make its own derived copy of read_file()
//
int Firm::read_derived_file(File* filePtr)
{
	//--- read data in derived class -----//

	int readSize = firm_array.firm_class_size(firm_id)-sizeof(Firm);

	if( readSize > 0 )
	{
		MSG(__FILE__":%d: file_read(this, ...);\n", __LINE__);

		if( !filePtr->file_read( (char*) this + sizeof(Firm), readSize ) )
			return 0;
	}

	return 1;
}
//----------- End of function Firm::read_derived_file ---------//
