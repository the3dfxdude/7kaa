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

#include <OTOWNRES.h>
#include <OTECHRES.h>
#include <OTALKRES.h>
#include <ORAWRES.h>
#include <ORACERES.h>
#include <OMONSRES.h>
#include <OGODRES.h>
#include <OUNITRES.h>
#include <OFIRMRES.h>

#include <OGFILE.h>
#include <OGF_V1.h>

#include <file_io_visitor.h>
#include <OGFILE_DYNARRAY.inl>
#include <OGFILE_DYNARRAYB.inl>

using namespace FileIOVisitor;


//-------- Start of function RaceRes::write_file -------------//
//
int RaceRes::write_file(File* filePtr)
{
	//------- write RaceInfo -------//

	RaceInfo* raceInfo = race_info_array;

	for( int i=1 ; i<=race_res.race_count ; i++, raceInfo++ )
	{
		filePtr->file_put_short( raceInfo->town_name_used_count );
	}

	return filePtr->file_write( name_used_array, sizeof(name_used_array[0]) * name_count );
}
//--------- End of function RaceRes::write_file ---------------//


//-------- Start of function RaceRes::read_file -------------//
//
int RaceRes::read_file(File* filePtr)
{
	//------- read RaceInfo -------//

	RaceInfo* raceInfo = race_info_array;

	for( int i=1 ; i<=race_res.race_count ; i++, raceInfo++ )
	{
		raceInfo->town_name_used_count = (!GameFile::read_file_same_version && i>VERSION_1_MAX_RACE) ?
			0 : filePtr->file_get_short();
	}

	if(!GameFile::read_file_same_version)
	{
		memset(name_used_array, 0, sizeof(name_used_array[0]) * name_count);
		return filePtr->file_read( name_used_array, sizeof(name_used_array[0]) * VERSION_1_RACERES_NAME_COUNT );
	}
	else
		return filePtr->file_read( name_used_array, sizeof(name_used_array[0]) * name_count );
}
//--------- End of function RaceRes::read_file ---------------//

//***//

//-------- Start of function UnitRes::write_file -------------//
//
int UnitRes::write_file(File* filePtr)
{
	filePtr->file_put_short(mobile_monster_count);

	UnitInfo* unitInfo = unit_info_array;

	for( int i=1 ; i<=unit_res.unit_info_count ; i++, unitInfo++ )
	{
		if( !filePtr->file_write( unitInfo->nation_tech_level_array, sizeof(unitInfo->nation_tech_level_array) ) )
			return 0;

		if( !filePtr->file_write( unitInfo->nation_unit_count_array, sizeof(unitInfo->nation_unit_count_array) ) )
			return 0;

		if( !filePtr->file_write( unitInfo->nation_general_count_array, sizeof(unitInfo->nation_general_count_array) ) )
			return 0;
	}

	return 1;
}
//--------- End of function UnitRes::write_file ---------------//


//-------- Start of function UnitRes::read_file -------------//
//
int UnitRes::read_file(File* filePtr)
{
	mobile_monster_count = filePtr->file_get_short();

	UnitInfo* unitInfo = unit_info_array;

	for( int i=1 ; i<=unit_res.unit_info_count ; i++, unitInfo++ )
	{
		if(!GameFile::read_file_same_version && i > VERSION_1_UNITRES_UNIT_INFO_COUNT)
		{
			memset(unitInfo->nation_tech_level_array, 0, sizeof(unitInfo->nation_tech_level_array));
			memset(unitInfo->nation_unit_count_array, 0, sizeof(unitInfo->nation_unit_count_array));
			memset(unitInfo->nation_general_count_array, 0, sizeof(unitInfo->nation_general_count_array));
			continue;
		}

		if( !filePtr->file_read( unitInfo->nation_tech_level_array, sizeof(unitInfo->nation_tech_level_array) ) )
			return 0;

		if( !filePtr->file_read( unitInfo->nation_unit_count_array, sizeof(unitInfo->nation_unit_count_array) ) )
			return 0;

		if( !filePtr->file_read( unitInfo->nation_general_count_array, sizeof(unitInfo->nation_general_count_array) ) )
			return 0;
	}

	return 1;
}
//--------- End of function UnitRes::read_file ---------------//

//***//

//-------- Start of function FirmRes::write_file -------------//
//
int FirmRes::write_file(File* filePtr)
{
	return filePtr->file_write( firm_info_array, firm_count * sizeof(FirmInfo) );
}
//--------- End of function FirmRes::write_file ---------------//


//-------- Start of function FirmRes::read_file -------------//
//
int FirmRes::read_file(File* filePtr)
{
	int arraySize = firm_count * sizeof(FirmInfo);

	//----- save the firm names, so that it won't be overwritten by the saved game file ----//

	FirmInfo* oldFirmInfoArray = (FirmInfo*) mem_add(arraySize);

	memcpy( oldFirmInfoArray, firm_info_array, arraySize );

	int rc = filePtr->file_read( firm_info_array, arraySize );

	for( int i=0 ; i<firm_count ; i++ )
	{
		memcpy( firm_info_array[i].name			  , oldFirmInfoArray[i].name			  , FirmInfo::NAME_LEN+1 );
		memcpy( firm_info_array[i].short_name	  , oldFirmInfoArray[i].short_name	  , FirmInfo::SHORT_NAME_LEN+1 );
		memcpy( firm_info_array[i].overseer_title, oldFirmInfoArray[i].overseer_title, FirmInfo::TITLE_LEN+1 );
		memcpy( firm_info_array[i].worker_title  , oldFirmInfoArray[i].worker_title  , FirmInfo::TITLE_LEN+1 );

		// ###### patch begin Gilbert 11/3 ########//
		firm_info_array[i].first_build_id = oldFirmInfoArray[i].first_build_id;
		firm_info_array[i].build_count = oldFirmInfoArray[i].build_count;
		// ###### patch end Gilbert 11/3 ########//
	}

	mem_del( oldFirmInfoArray );

	return rc;
}
//--------- End of function FirmRes::read_file ---------------//

//***//

//-------- Start of function TownRes::write_file -------------//
//
int TownRes::write_file(File* filePtr)
{
	return filePtr->file_write( town_name_used_array, sizeof(town_name_used_array[0]) * town_name_count );
}
//--------- End of function TownRes::write_file ---------------//


//-------- Start of function TownRes::read_file -------------//
//
int TownRes::read_file(File* filePtr)
{
	if(!GameFile::read_file_same_version)
	{
		memset(town_name_used_array, 0, sizeof(town_name_used_array));
		return filePtr->file_read( town_name_used_array, sizeof(town_name_used_array[0]) * VERSION_1_TOWNRES_TOWN_NAME_COUNT );
	}
	else
		return filePtr->file_read( town_name_used_array, sizeof(town_name_used_array[0]) * town_name_count );
}
//--------- End of function TownRes::read_file ---------------//

//***//

//-------- Start of function TechRes::write_file -------------//
//
int TechRes::write_file(File* filePtr)
{
	if( !filePtr->file_write( tech_class_array, tech_class_count * sizeof(TechClass) ) )
		return 0;

	if( !filePtr->file_write( tech_info_array, tech_count * sizeof(TechInfo) ) )
		return 0;

	return 1;
}
//--------- End of function TechRes::write_file ---------------//


//-------- Start of function TechRes::read_file -------------//
//
int TechRes::read_file(File* filePtr)
{
	if( !filePtr->file_read( tech_class_array, tech_class_count * sizeof(TechClass) ) )
		return 0;

	if(!GameFile::read_file_same_version)
	{
		if(!filePtr->file_read( tech_info_array, VERSION_1_TECH_COUNT * sizeof(TechInfo) ) )
			return 0;

		TechInfo *techInfoPtr = tech_info_array + VERSION_1_TECH_COUNT;
		for(int i=VERSION_1_TECH_COUNT; i<tech_count; ++i, techInfoPtr++)
		{
			memset(techInfoPtr->nation_tech_level_array, 0, sizeof(techInfoPtr->nation_tech_level_array));
			memset(techInfoPtr->nation_is_researching_array, 0, sizeof(techInfoPtr->nation_is_researching_array));
			memset(techInfoPtr->nation_research_progress_array, 0, sizeof(techInfoPtr->nation_research_progress_array));
		}
	}
	else
	{
		if( !filePtr->file_read( tech_info_array, tech_count * sizeof(TechInfo) ) )
			return 0;
	}

	return 1;
}
//--------- End of function TechRes::read_file ---------------//

//***//

template <typename Visitor>
static void visit_talk_msg(Visitor *v, TalkMsg *tm)
{
	visit<int16_t>(v, &tm->talk_id);
	visit<int16_t>(v, &tm->talk_para1);
	visit<int16_t>(v, &tm->talk_para2);
	visit<int32_t>(v, &tm->date);
	visit<int8_t>(v, &tm->from_nation_recno);
	visit<int8_t>(v, &tm->to_nation_recno);
	visit<int8_t>(v, &tm->reply_type);
	visit<int32_t>(v, &tm->reply_date);
	visit<int8_t>(v, &tm->relation_status);
}

template <typename Visitor>
static void visit_talk_choice(Visitor *v, TalkChoice *tc)
{
	visit_pointer(v, &tc->str);
	visit<int16_t>(v, &tc->para);
}

template <typename Visitor>
static void visit_talk_res(Visitor *v, TalkRes *tr)
{
	visit<int8_t>(v, &tr->init_flag);
	visit<int16_t>(v, &tr->reply_talk_msg_recno);
	visit_talk_msg(v, &tr->cur_talk_msg);
	visit_pointer(v, &tr->choice_question);
	visit_pointer(v, &tr->choice_question_second_line);
	visit<int16_t>(v, &tr->talk_choice_count);

	for (int n = 0; n < MAX_TALK_CHOICE; n++)
		visit_talk_choice(v, &tr->talk_choice_array[n]);

	visit_array<int8_t>(v, tr->available_talk_id_array);
	visit<int16_t>(v, &tr->cur_choice_id);
	visit<int8_t>(v, &tr->save_view_mode);
	visit<int8_t>(v, &tr->msg_add_nation_color);
	v->skip(39); /* &tr->talk_msg_array */
}

enum { TALK_RES_RECORD_SIZE = 214 };

//-------- Start of function TalkRes::write_file -------------//
//
int TalkRes::write_file(File* filePtr)
{
	if (!visit_with_record_size<FileWriterVisitor>(filePtr, this, &visit_talk_res<FileWriterVisitor>,
		TALK_RES_RECORD_SIZE))
		return 0;

	{
		FileWriterVisitor v(filePtr);
		talk_msg_array.accept_visitor_as_value_array(&v, visit_raw<FileWriterVisitor, TalkMsg>, sizeof(TalkMsg));
		if( !v.good() )
			return 0;
	}

	return 1;
}
//--------- End of function TalkRes::write_file ---------------//


//-------- Start of function TalkRes::read_file -------------//
//
int TalkRes::read_file(File* filePtr)
{
	if (!visit_with_record_size<FileReaderVisitor>(filePtr, this, &visit_talk_res<FileReaderVisitor>,
		TALK_RES_RECORD_SIZE))
		return 0;

	{
		FileReaderVisitor v(filePtr);
		talk_msg_array.accept_visitor_as_value_array(&v, visit_raw<FileReaderVisitor, TalkMsg>, sizeof(TalkMsg));
		if( !v.good() )
			return 0;
	}

	this->choice_question = NULL;
	this->choice_question_second_line = NULL;
	this->talk_choice_count = 0;

	return 1;
}
//--------- End of function TalkRes::read_file ---------------//

//***//

//-------- Start of function RawRes::write_file -------------//
//
int RawRes::write_file(File* filePtr)
{
	for( int i=0 ; i<MAX_RAW ; i++ )
	{
		{
			FileWriterVisitor v(filePtr);
			raw_info_array[i].raw_supply_firm_array.accept_visitor_as_value_array(&v, visit_raw<FileWriterVisitor, short>, sizeof(short));
			if (!v.good())
				return 0;
		}

		{
			FileWriterVisitor v(filePtr);
			raw_info_array[i].product_supply_firm_array.accept_visitor_as_value_array(&v, visit_raw<FileWriterVisitor, short>, sizeof(short));
			if (!v.good())
				return 0;
		}
	}

	return 1;
}
//--------- End of function RawRes::write_file ---------------//


//-------- Start of function RawRes::read_file -------------//
//
int RawRes::read_file(File* filePtr)
{
	for( int i=0 ; i<MAX_RAW ; i++ )
	{
		{
			FileReaderVisitor v(filePtr);
			raw_info_array[i].raw_supply_firm_array.accept_visitor_as_value_array(&v, visit_raw<FileReaderVisitor, short>, sizeof(short));
			if (!v.good())
				return 0;
		}

		{
			FileReaderVisitor v(filePtr);
			raw_info_array[i].product_supply_firm_array.accept_visitor_as_value_array(&v, visit_raw<FileReaderVisitor, short>, sizeof(short));
			if (!v.good())
				return 0;
		}
	}

	return 1;
}
//--------- End of function RawRes::read_file ---------------//

//***//

//-------- Start of function GodRes::write_file -------------//
//
int GodRes::write_file(File* filePtr)
{
	return filePtr->file_write( god_info_array, sizeof(GodInfo) * god_count );
}
//--------- End of function GodRes::write_file ---------------//


//-------- Start of function GodRes::read_file -------------//
//
int GodRes::read_file(File* filePtr)
{
	if(!GameFile::read_file_same_version)
	{
		memset(god_info_array, 0, sizeof(god_info_array));
		return filePtr->file_read( god_info_array, sizeof(GodInfo) * VERSION_1_GODRES_GOD_COUNT );
	}
	else
		return filePtr->file_read( god_info_array, sizeof(GodInfo) * god_count );
}
//--------- End of function GodRes::read_file ---------------//

//***//

//-------- Start of function MonsterRes::write_file -------------//
//
int MonsterRes::write_file(File* filePtr)
{
	return filePtr->file_write( active_monster_array, sizeof(active_monster_array) );
}
//--------- End of function MonsterRes::write_file ---------------//


//-------- Start of function MonsterRes::read_file -------------//
//
int MonsterRes::read_file(File* filePtr)
{
	return filePtr->file_read( active_monster_array, sizeof(active_monster_array) );
}
//--------- End of function MonsterRes::read_file ---------------//
