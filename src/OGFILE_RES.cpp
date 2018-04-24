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


template <typename Visitor>
void visit_race_res(Visitor* v, RaceRes* c)
{
	for (int i = 0; i < c->race_count; ++i)
	{
		visit<int16_t>(v, &c->race_info_array[i].town_name_used_count);
	}
	v->with_record_size(c->name_count);
	for (int i = 0; i < c->name_count; ++i)
	{
		visit<int8_t>(v, &c->name_used_array[i]);
	}
}

void visit_version_1_race_res(FileReaderVisitor* v, RaceRes* c)
{
	for (int i = 0; i < c->race_count; ++i)
	{
		if (i < VERSION_1_MAX_RACE)
		{
			visit<int16_t>(v, &c->race_info_array[i].town_name_used_count);
		}
		else
		{
			c->race_info_array[i].town_name_used_count = 0;
		}
	}
	v->with_record_size(VERSION_1_RACERES_NAME_COUNT);
	for (int i = 0; i < c->name_count; ++i)
	{
		if (i < VERSION_1_RACERES_NAME_COUNT)
		{
			visit<int8_t>(v, &c->name_used_array[i]);
		}
		else
		{
			c->name_used_array[i] = 0;
		}
	}
}

//-------- Start of function RaceRes::write_file -------------//
//
int RaceRes::write_file(File* filePtr)
{
	FileWriterVisitor v(filePtr);
	visit_race_res(&v, this);
	return v.good();
}
//--------- End of function RaceRes::write_file ---------------//


//-------- Start of function RaceRes::read_file -------------//
//
int RaceRes::read_file(File* filePtr)
{
	FileReaderVisitor v(filePtr);
	if (GameFile::read_file_same_version)
	{
		visit_race_res(&v, this);
	}
	else
	{
		visit_version_1_race_res(&v, this);
	}
	return v.good();
}
//--------- End of function RaceRes::read_file ---------------//


//***//

template <typename Visitor>
void visit_unit_res(Visitor* v, UnitRes* c)
{
	visit<int16_t>(v, &c->mobile_monster_count);
	for (UnitInfo* unitInfo = c->unit_info_array; unitInfo < c->unit_info_array + c->unit_info_count; ++unitInfo)
	{
		v->with_record_size(sizeof(unitInfo->nation_tech_level_array));
		visit_array<int8_t>(v, unitInfo->nation_tech_level_array);
		v->with_record_size(sizeof(unitInfo->nation_unit_count_array));
		visit_array<int16_t>(v, unitInfo->nation_unit_count_array);
		v->with_record_size(sizeof(unitInfo->nation_general_count_array));
		visit_array<int16_t>(v, unitInfo->nation_general_count_array);
	}
}

template <typename Visitor>
void visit_version_1_unit_res(Visitor* v, UnitRes* c)
{
	visit<int16_t>(v, &c->mobile_monster_count);
	for (int i = 0; i < c->unit_info_count; ++i)
	{
		UnitInfo* unitInfo = c->unit_info_array + i;
		if (i < VERSION_1_UNITRES_UNIT_INFO_COUNT)
		{
			v->with_record_size(sizeof(unitInfo->nation_tech_level_array));
			visit_array<int8_t>(v, unitInfo->nation_tech_level_array);
			v->with_record_size(sizeof(unitInfo->nation_unit_count_array));
			visit_array<int16_t>(v, unitInfo->nation_unit_count_array);
			v->with_record_size(sizeof(unitInfo->nation_general_count_array));
			visit_array<int16_t>(v, unitInfo->nation_general_count_array);
		}
		else
		{
			memset(unitInfo->nation_tech_level_array, 0, sizeof(unitInfo->nation_tech_level_array));
			memset(unitInfo->nation_unit_count_array, 0, sizeof(unitInfo->nation_unit_count_array));
			memset(unitInfo->nation_general_count_array, 0, sizeof(unitInfo->nation_general_count_array));
		}
	}
}

//-------- Start of function UnitRes::write_file -------------//
//
int UnitRes::write_file(File* filePtr)
{
	FileWriterVisitor v(filePtr);
	visit_unit_res(&v, this);
	return v.good();
}
//--------- End of function UnitRes::write_file ---------------//


//-------- Start of function UnitRes::read_file -------------//
//
int UnitRes::read_file(File* filePtr)
{
	FileReaderVisitor v(filePtr);
	if (GameFile::read_file_same_version)
	{
		visit_unit_res(&v, this);
	}
	else
	{
		visit_version_1_unit_res(&v, this);
	}
	return v.good();
}
//--------- End of function UnitRes::read_file ---------------//

//***//

template <typename Visitor>
static void visit_firm_info_members(Visitor* v, FirmInfo* c)
{
	v->skip(79); // Skip all non-game variables.
	// Persist game variables
	visit<int16_t>(v, &c->total_firm_count);
	visit_array<int16_t>(v, c->nation_firm_count_array);
	visit_array<int8_t>(v, c->nation_tech_level_array);
}

template <typename Visitor>
static void visit_firm_res(Visitor* v, FirmRes* c)
{
	enum { FIRM_INFO_RECORD_SIZE = 102 };
	v->with_record_size(c->firm_count * FIRM_INFO_RECORD_SIZE);
	for (int i = 0; i < c->firm_count; ++i)
	{
		visit_firm_info_members(v, &c->firm_info_array[i]);
	}
}

//-------- Start of function FirmRes::write_file -------------//
//
int FirmRes::write_file(File* filePtr)
{
	FileWriterVisitor v(filePtr);
	visit_firm_res(&v, this);
	return v.good();
}
//--------- End of function FirmRes::write_file ---------------//


//-------- Start of function FirmRes::read_file -------------//
//
int FirmRes::read_file(File* filePtr)
{
	FileReaderVisitor v(filePtr);
	visit_firm_res(&v, this);
	return v.good();
}
//--------- End of function FirmRes::read_file ---------------//

//***//

template <typename Visitor>
static void visit_town_res(Visitor* v, TownRes* c)
{
	v->with_record_size(c->town_name_count * sizeof(uint8_t));
	for (int i = 0; i < c->town_name_count; ++i)
	{
		visit<uint8_t>(v, &c->town_name_used_array[i]);
	}
}

template <typename Visitor>
static void visit_version_1_town_res(Visitor* v, TownRes* c)
{
	v->with_record_size(VERSION_1_TOWNRES_TOWN_NAME_COUNT * sizeof(uint8_t));
	for (int i = 0; i < c->town_name_count; ++i)
	{
		if (i < VERSION_1_TOWNRES_TOWN_NAME_COUNT)
		{
			visit<uint8_t>(v, &c->town_name_used_array[i]);
		}
		else
		{
			c->town_name_used_array[i] = 0;
		}
	}
}

//-------- Start of function TownRes::write_file -------------//
//
int TownRes::write_file(File* filePtr)
{
	FileWriterVisitor v(filePtr);
	visit_town_res(&v, this);
	return v.good();
}
//--------- End of function TownRes::write_file ---------------//


//-------- Start of function TownRes::read_file -------------//
//
int TownRes::read_file(File* filePtr)
{
	FileReaderVisitor v(filePtr);
	if (GameFile::read_file_same_version)
	{
		visit_town_res(&v, this);
	}
	else
	{
		visit_version_1_town_res(&v, this);
	}
	return v.good();
}
//--------- End of function TownRes::read_file ---------------//

//***//

template <typename Visitor>
static void visit_tech_class_members(Visitor* v, TechClass* c)
{
	v->skip(8);
	visit_array<int16_t>(v, c->nation_research_firm_recno_array);
}


template <typename Visitor>
static void visit_tech_info_members(Visitor* v, TechInfo* c)
{
	v->skip(19);
	visit_array<int8_t>(v, c->nation_tech_level_array);
	visit_array<int8_t>(v, c->nation_is_researching_array);
	visit_array<float>(v, c->nation_research_progress_array);
}

enum { TECH_CLASS_RECORD_SIZE = 22, TECH_INFO_RECORD_SIZE = 61 };

template <typename Visitor>
void visit_tech_res(Visitor* v, TechRes* c)
{
	v->with_record_size(c->tech_class_count * TECH_CLASS_RECORD_SIZE);
	for (TechClass* techClass = c->tech_class_array; techClass < c->tech_class_array + c->tech_class_count; ++techClass)
	{
		visit_tech_class_members(v, techClass);
	}

	v->with_record_size(c->tech_count * TECH_INFO_RECORD_SIZE);
	for (TechInfo* techInfo = c->tech_info_array; techInfo < c->tech_info_array + c->tech_count; ++techInfo)
	{
		visit_tech_info_members(v, techInfo);
	}
}

template <typename Visitor>
void visit_version_1_tech_res(Visitor* v, TechRes* c)
{
	v->with_record_size(c->tech_class_count * TECH_CLASS_RECORD_SIZE);
	for (TechClass* techClass = c->tech_class_array; techClass < c->tech_class_array + c->tech_class_count; ++techClass)
	{
		visit_tech_class_members(v, techClass);
	}

	v->with_record_size(VERSION_1_TECH_COUNT * TECH_INFO_RECORD_SIZE);
	for (int i = 0; i < c->tech_count; ++i)
	{
		TechInfo* techInfo = c->tech_info_array + i;
		if (i < VERSION_1_TECH_COUNT)
		{
			visit_tech_info_members(v, techInfo);
		}
		else
		{
			memset(techInfo->nation_tech_level_array, 0, sizeof(techInfo->nation_tech_level_array));
			memset(techInfo->nation_is_researching_array, 0, sizeof(techInfo->nation_is_researching_array));
			memset(techInfo->nation_research_progress_array, 0, sizeof(techInfo->nation_research_progress_array));
		}
	}
}

//-------- Start of function TechRes::write_file -------------//
//
int TechRes::write_file(File* filePtr)
{
	FileWriterVisitor v(filePtr);
	visit_tech_res(&v, this);
	return v.good();
}
//--------- End of function TechRes::write_file ---------------//


//-------- Start of function TechRes::read_file -------------//
//
int TechRes::read_file(File* filePtr)
{
	FileReaderVisitor v(filePtr);
	if (GameFile::read_file_same_version)
	{
		visit_tech_res(&v, this);
	}
	else
	{
		visit_version_1_tech_res(&v, this);
	}
	return v.good();
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
static void visit_talk_res_members(Visitor *v, TalkRes *tr)
{
	visit<int8_t>(v, &tr->init_flag);
	visit<int16_t>(v, &tr->reply_talk_msg_recno);
	visit_talk_msg(v, &tr->cur_talk_msg);
	v->skip(8); /* choice_question, choice_question_second_line */	
	v->skip(2 + 20 * 6); /* talk_choice_count, talk_choice_array */
	visit_array<int8_t>(v, tr->available_talk_id_array);
	visit<int16_t>(v, &tr->cur_choice_id);
	visit<int8_t>(v, &tr->save_view_mode);
	visit<int8_t>(v, &tr->msg_add_nation_color);
	v->skip(39); /* &tr->talk_msg_array */
}

template <typename Visitor>
static void visit_talk_msg_members(Visitor* v, TalkMsg* c)
{
	visit<int16_t>(v, &c->talk_id);
	visit<int16_t>(v, &c->talk_para1);
	visit<int16_t>(v, &c->talk_para2);
	visit<int32_t>(v, &c->date);
	visit<int8_t>(v, &c->from_nation_recno);
	visit<int8_t>(v, &c->to_nation_recno);
	visit<int8_t>(v, &c->reply_type);
	visit<int32_t>(v, &c->reply_date);
	visit<int8_t>(v, &c->relation_status);
}

template <typename Visitor>
static void visit_talk_res(Visitor *v, TalkRes *tr)
{
	enum { TALK_RES_RECORD_SIZE = 214, TALK_MSG_RECORD_SIZE = 18 };

	v->with_record_size(TALK_RES_RECORD_SIZE);
	visit_talk_res_members(v, tr);
	tr->talk_msg_array.accept_visitor_as_value_array(v, visit_talk_msg_members<Visitor>, TALK_MSG_RECORD_SIZE);

	if (is_reader_visitor(v))
	{
		tr->choice_question = nullptr;
		tr->choice_question_second_line = nullptr;
		tr->talk_choice_count = 0;
	}
}


//-------- Start of function TalkRes::write_file -------------//
//
int TalkRes::write_file(File* filePtr)
{
	FileWriterVisitor v(filePtr);
	visit_talk_res(&v, this);
	return v.good();
}
//--------- End of function TalkRes::write_file ---------------//


//-------- Start of function TalkRes::read_file -------------//
//
int TalkRes::read_file(File* filePtr)
{
	FileReaderVisitor v(filePtr);
	visit_talk_res(&v, this);
	return v.good();
}
//--------- End of function TalkRes::read_file ---------------//

//***//

namespace {
	template <typename Visitor>
	void visit_supply_firm_array_element(Visitor* v, short* c)
	{
		visit<int16_t>(v, c);
	}
}

template <typename Visitor>
static void visit_raw_res(Visitor *v, RawRes *c)
{
	for (int i = 0; i < MAX_RAW; i++)
	{
		c->raw_info_array[i].raw_supply_firm_array.accept_visitor_as_value_array(v, visit_supply_firm_array_element<Visitor>, sizeof(int16_t));
		c->raw_info_array[i].product_supply_firm_array.accept_visitor_as_value_array(v, visit_supply_firm_array_element<Visitor>, sizeof(int16_t));
	}
}

//-------- Start of function RawRes::write_file -------------//
//
int RawRes::write_file(File* filePtr)
{
	FileWriterVisitor v(filePtr);
	visit_raw_res(&v, this);
	return v.good();
}
//--------- End of function RawRes::write_file ---------------//


//-------- Start of function RawRes::read_file -------------//
//
int RawRes::read_file(File* filePtr)
{
	FileReaderVisitor v(filePtr);
	visit_raw_res(&v, this);
	return v.good();
}
//--------- End of function RawRes::read_file ---------------//

//***//

template <typename Visitor>
static void visit_god_info_members(Visitor* v, GodInfo* c)
{
	v->skip(9);
	visit_array<int8_t>(v, c->nation_know_array);
}

enum { GOD_INFO_RECORD_SIZE = 16 };

template <typename Visitor>
static void visit_god_res(Visitor* v, GodRes* c)
{
	v->with_record_size(c->god_count * GOD_INFO_RECORD_SIZE);
	for (int i = 0; i < c->god_count; ++i)
	{
		visit_god_info_members(v, &c->god_info_array[i]);
	}
}

template <typename Visitor>
static void visit_version_1_god_res(Visitor* v, GodRes* c)
{
	v->with_record_size(c->god_count * VERSION_1_GODRES_GOD_COUNT);
	for (int i = 0; i < c->god_count; ++i)
	{
		if (i < VERSION_1_GODRES_GOD_COUNT)
		{
			visit_god_info_members(v, &c->god_info_array[i]);
		}
		else
		{
			memset(&c->god_info_array[i], 0, sizeof(GodInfo));
		}
	}
}

//-------- Start of function GodRes::write_file -------------//
//
int GodRes::write_file(File* filePtr)
{
	FileWriterVisitor v(filePtr);
	visit_god_res(&v, this);
	return v.good();
}
//--------- End of function GodRes::write_file ---------------//


//-------- Start of function GodRes::read_file -------------//
//
int GodRes::read_file(File* filePtr)
{
	FileReaderVisitor v(filePtr);
	if(GameFile::read_file_same_version)
	{
		visit_god_res(&v, this);
	}
	else
	{
		visit_version_1_god_res(&v, this);
	}
	return v.good();
}
//--------- End of function GodRes::read_file ---------------//

//***//

template <typename Visitor>
static void visit_monster_res(Visitor* v, MonsterRes* c)
{
	visit_array<int16_t>(v, c->active_monster_array);
}

enum { MONSTER_RES_RECORD_SIZE = 6 };

//-------- Start of function MonsterRes::write_file -------------//
//
int MonsterRes::write_file(File* filePtr)
{
	return visit_with_record_size(filePtr, this, visit_monster_res<FileWriterVisitor>, MONSTER_RES_RECORD_SIZE);
}
//--------- End of function MonsterRes::write_file ---------------//


//-------- Start of function MonsterRes::read_file -------------//
//
int MonsterRes::read_file(File* filePtr)
{
	return visit_with_record_size(filePtr, this, visit_monster_res<FileReaderVisitor>, MONSTER_RES_RECORD_SIZE);
}
//--------- End of function MonsterRes::read_file ---------------//
