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

//Filename    : OGFILE_ARRAYS.CPP
//Description : Object Game file, save game and restore game, part 3, various arrays (NewsArray, SiteArray, etc.)

#include <OGFILE.h>
#include <OGF_V1.h>
#include <ONEWS.h>
#include <OREBEL.h>
#include <OREGION.h>
#include <OREGIONS.h>
#include <OSITE.h>
#include <OSNOWG.h>
#include <OSPY.h>
#include <file_io_visitor.h>
#include <OGFILE_DYNARRAY.inl>
#include <OGFILE_DYNARRAYB.inl>
#include <dbglog.h>

using namespace FileIOVisitor;

DBGLOG_DEFAULT_CHANNEL(GameFile);


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


//*****//


template <typename Visitor>
static void visit_site_members(Visitor* v, Site* c)
{
	visit<int16_t>(v, &c->site_recno);
	visit<int8_t>(v, &c->site_type);
	visit<int16_t>(v, &c->object_id);
	visit<int32_t>(v, &c->reserve_qty);
	visit<int8_t>(v, &c->has_mine);
	visit<int16_t>(v, &c->map_x_loc);
	visit<int16_t>(v, &c->map_y_loc);
	visit<uint8_t>(v, &c->region_id);
}

template <typename Visitor>
static void visit_site_array_members(Visitor* v, SiteArray* c)
{
	visit<int16_t>(v, &c->selected_recno);
	visit<int16_t>(v, &c->untapped_raw_count);
	visit<int16_t>(v, &c->scroll_count);
	visit<int16_t>(v, &c->gold_coin_count);
	visit<int16_t>(v, &c->std_raw_site_count);
}

template <typename Visitor>
static void visit_site_array(Visitor* v, SiteArray* c)
{
	enum { SITE_RECORD_SIZE = 15 };

	visit_site_array_members(v, c);
	c->accept_visitor_as_value_array(v, visit_site_members<Visitor>, SITE_RECORD_SIZE);
}

//-------- Start of function SiteArray::write_file -------------//
//
int SiteArray::write_file(File* filePtr)
{
	FileWriterVisitor v(filePtr);
	visit_site_array(&v, this);
	return v.good();
}
//--------- End of function SiteArray::write_file ---------------//

//-------- Start of function SiteArray::read_file -------------//
//
int SiteArray::read_file(File* filePtr)
{
	FileReaderVisitor v(filePtr);
	visit_site_array(&v, this);
	return v.good();
}
//--------- End of function SiteArray::read_file ---------------//


//*****//

template <typename Visitor>
static void visit_spy_members(Visitor* v, Spy* c)
{
	visit<int16_t>(v, &c->spy_recno);
	visit<int8_t>(v, &c->spy_place);
	visit<int16_t>(v, &c->spy_place_para);
	visit<int8_t>(v, &c->spy_skill);
	visit<int8_t>(v, &c->spy_loyalty);
	visit<int8_t>(v, &c->true_nation_recno);
	visit<int8_t>(v, &c->cloaked_nation_recno);
	visit<int8_t>(v, &c->notify_cloaked_nation_flag);
	visit<int8_t>(v, &c->exposed_flag);
	visit<int8_t>(v, &c->race_id);
	visit<uint16_t>(v, &c->name_id);
	visit<int8_t>(v, &c->action_mode);
}

enum { SPY_RECORD_SIZE = 15 };

//-------- Start of function SpyArray::write_file -------------//
//
int SpyArray::write_file(File* filePtr)
{
	FileWriterVisitor v(filePtr);
	accept_visitor_as_value_array(&v, visit_spy_members<FileWriterVisitor>, SPY_RECORD_SIZE);
	return v.good();
}
//--------- End of function SpyArray::write_file ---------------//


//-------- Start of function SpyArray::read_file -------------//
//
int SpyArray::read_file(File* filePtr)
{
	FileReaderVisitor v(filePtr);
	accept_visitor_as_value_array(&v, visit_spy_members<FileReaderVisitor>, SPY_RECORD_SIZE);
	return v.good();
}
//--------- End of function SpyArray::read_file ---------------//


//*****//

template <typename Visitor>
static void visit_snow_ground_array(Visitor* v, SnowGroundArray* c)
{
	visit<uint32_t>(v, &c->seed);
	visit<int32_t>(v, &c->snow_thick);
	visit<int32_t>(v, &c->snow_pattern);
}

enum {SNOW_GROUND_ARRAY_RECORD_SIZE = 12};

//-------- Start of function SnowGroundArray::write_file -------------//
//
int SnowGroundArray::write_file(File* filePtr)
{
	return visit_with_record_size(filePtr, this, visit_snow_ground_array<FileWriterVisitor>, SNOW_GROUND_ARRAY_RECORD_SIZE);
}
//--------- End of function SnowGroundArray::write_file ---------------//


//-------- Start of function SnowGroundArray::read_file -------------//
//
int SnowGroundArray::read_file(File* filePtr)
{
	return visit_with_record_size(filePtr, this, visit_snow_ground_array<FileReaderVisitor>, SNOW_GROUND_ARRAY_RECORD_SIZE);
}
//--------- End of function SnowGroundArray::read_file ---------------//

//*****//

template <typename Visitor>
static void visit_region_info(Visitor* v, RegionInfo* c)
{
	visit<uint8_t>(v, &c->region_id);
	visit<uint8_t>(v, &c->region_stat_id);
	visit_enum(v, &c->region_type);
	visit<int32_t>(v, &c->adj_offset_bit);
	visit<int32_t>(v, &c->region_size);
	visit<int16_t>(v, &c->center_x);
	visit<int16_t>(v, &c->center_y);
}

template <typename Visitor>
static void visit_region_path(Visitor* v, RegionPath* c)
{
	visit<uint8_t>(v, &c->sea_region_id);
	visit<uint8_t>(v, &c->land_region_stat_id);
}

template <typename Visitor>
static void visit_region_stat(Visitor* v, RegionStat* c)
{
	visit<uint8_t>(v, &c->region_id);
	visit_array<int8_t>(v, c->nation_is_present_array);
	visit<int8_t>(v, &c->nation_presence_count);
	visit_array<int16_t>(v, c->firm_type_count_array);
	visit_array<int16_t>(v, c->firm_nation_count_array);
	visit_array<int16_t>(v, c->camp_nation_count_array);
	visit_array<int16_t>(v, c->mine_nation_count_array);
	visit_array<int16_t>(v, c->harbor_nation_count_array);
	visit<int16_t>(v, &c->total_firm_count);
	visit_array<int16_t>(v, c->town_nation_count_array);
	visit_array<int16_t>(v, c->base_town_nation_count_array);
	visit<int16_t>(v, &c->independent_town_count);
	visit<int16_t>(v, &c->total_town_count);
	visit_array<int16_t>(v, c->nation_population_array);
	visit_array<int16_t>(v, c->nation_jobless_population_array);
	visit_array<int16_t>(v, c->unit_nation_count_array);
	visit<int16_t>(v, &c->independent_unit_count);
	visit<int16_t>(v, &c->total_unit_count);
	visit<int16_t>(v, &c->site_count);
	visit<int16_t>(v, &c->raw_count);
	visit_array(v, c->reachable_region_array, visit_region_path<Visitor>);
	visit<int8_t>(v, &c->reachable_region_count);
}

// Visit a raw, dynamically allocated array of POD types, where the size is stored as an int16_t before the array data chunk.
template <typename Visitor, typename T, typename SizeT>
static void visit_raw_value_array(Visitor* v, T*& rawArray, void (*visitValue) (Visitor*, T*), SizeT& arrayElementCount, int valueRecordSize)
{
	visit_property<SizeT, int16_t>(v, [arrayElementCount]() {return arrayElementCount;},
		[&arrayElementCount, &rawArray](SizeT visitSize) {
		arrayElementCount = visitSize;
		rawArray = (T*) mem_add(visitSize * sizeof(T));
	});

	v->with_record_size(arrayElementCount * valueRecordSize);
	for (SizeT i = 0; i < arrayElementCount; ++i) {
		visitValue(v, &rawArray[i]);
	}
}

template <typename Visitor>
static void visit_region_array(Visitor *v, RegionArray *ra)
{
	enum {REGION_INFO_RECORD_SIZE = 18};
	enum {REGION_STAT_RECORD_SIZE = 190};

	visit<int32_t>(v, &ra->init_flag);
	visit_pointer(v, &ra->region_info_array);
	visit<int32_t>(v, &ra->region_info_count);
	visit_pointer(v, &ra->region_stat_array);
	visit<int32_t>(v, &ra->region_stat_count);
	visit_pointer(v, &ra->connect_bits);
	visit_array<uint8_t>(v, ra->region_sorted_array);

	// Note: region_info_count, the size of region_info_array, is already read before.
	if (is_reader_visitor(v))
	{
		ra->region_info_array = ra->region_info_count > 0 ? (RegionInfo *) mem_add(sizeof(RegionInfo)*ra->region_info_count) : nullptr;
	}
	v->with_record_size(ra->region_info_count * REGION_INFO_RECORD_SIZE);
	for (int i = 0; i < ra->region_info_count; ++i)
	{
		visit_region_info(v, &ra->region_info_array[i]);
	}

	// Note: region_stat_count, the size of region_stat_array, is read twice.
	visit_raw_value_array(v, ra->region_stat_array, visit_region_stat<Visitor>, ra->region_stat_count, REGION_STAT_RECORD_SIZE);

	//--------- read connection bits ----------//

	int connectBitCount = (ra->region_info_count - 1) * (ra->region_info_count) / 2;
	int connectByteCount = (connectBitCount + 7) / 8;

	if (is_reader_visitor(v))
	{
		ra->connect_bits = connectByteCount > 0 ? (unsigned char *)mem_add(connectByteCount) : nullptr;
	}
	if (connectByteCount > 0)
	{
		v->with_record_size(connectByteCount);
		for (int i = 0; i < connectByteCount; ++i)
		{
			visit<uint8_t>(v, &ra->connect_bits[i]);
		}
	}
}

enum { REGION_ARRAY_RECORD_SIZE = 279 };

//-------- Start of function RegionArray::write_file -------------//
//
int RegionArray::write_file(File* filePtr)
{
	return visit_with_record_size(filePtr, this, &visit_region_array<FileWriterVisitor>, REGION_ARRAY_RECORD_SIZE);
}
//--------- End of function RegionArray::write_file ---------------//


//-------- Start of function RegionArray::read_file -------------//
//
int RegionArray::read_file(File* filePtr)
{
	return visit_with_record_size(filePtr, this, &visit_region_array<FileReaderVisitor>, REGION_ARRAY_RECORD_SIZE);
}
//--------- End of function RegionArray::read_file ---------------//

//*****//

template <typename Visitor>
static void visit_news_members(Visitor* v, News* c)
{
	visit<int8_t>(v, &c->id);
	visit<int8_t>(v, &c->type);
	visit<int32_t>(v, &c->news_date);
	visit<int8_t>(v, &c->nation_color1);
	visit<int8_t>(v, &c->nation_color2);
	visit<int8_t>(v, &c->nation_race_id1);
	visit<int8_t>(v, &c->nation_race_id2);
	visit<int32_t>(v, &c->nation_name_id1);
	visit<int32_t>(v, &c->nation_name_id2);
	visit<int16_t>(v, &c->short_para1);
	visit<int16_t>(v, &c->short_para2);
	visit<int16_t>(v, &c->short_para3);
	visit<int16_t>(v, &c->short_para4);
	visit<int16_t>(v, &c->short_para5);
	visit<int8_t>(v, &c->loc_type);
	visit<int16_t>(v, &c->loc_type_para);
	visit<uint16_t>(v, &c->loc_type_para2);
	visit<int16_t>(v, &c->loc_x);
	visit<int16_t>(v, &c->loc_y);
}

template <typename Visitor>
void visit_news_array(Visitor* v, NewsArray* c) {
	enum { NEWS_RECORD_SIZE = 37 };

	v->with_record_size(sizeof(int8_t) * sizeof(c->news_type_option)/sizeof(c->news_type_option[0]));
	visit_array<int8_t>(v, c->news_type_option);
	visit<int16_t>(v, &c->news_who_option);
	visit<int32_t>(v, &c->last_clear_recno);

	c->accept_visitor_as_value_array(v, visit_news_members<Visitor>, NEWS_RECORD_SIZE);
}

//-------- Start of function NewsArray::write_file -------------//
//
int NewsArray::write_file(File* filePtr)
{
	FileWriterVisitor v(filePtr);
	visit_news_array(&v, this);
	return v.good();
}
//--------- End of function NewsArray::write_file ---------------//


//-------- Start of function NewsArray::read_file -------------//
//
int NewsArray::read_file(File* filePtr)
{
	FileReaderVisitor v(filePtr);
	visit_news_array(&v, this);
	return v.good();
}
//--------- End of function NewsArray::read_file ---------------//
