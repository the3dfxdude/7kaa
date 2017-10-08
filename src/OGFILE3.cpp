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

//Filename    : OGFILE3.CPP
//Description : Object Game file, save game and restore game, part 3

#include <OGFILE.h>
#include <OGF_V1.h>
#include <ONATION.h>
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
	enum {SITE_RECORD_SIZE = 15 };

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


//-------- Start of function SpyArray::write_file -------------//
//
int SpyArray::write_file(File* filePtr)
{
	FileWriterVisitor v(filePtr);
	accept_visitor_as_value_array(&v, visit_raw<FileWriterVisitor, Spy>, sizeof(Spy));
	return v.good();
}
//--------- End of function SpyArray::write_file ---------------//


//-------- Start of function SpyArray::read_file -------------//
//
int SpyArray::read_file(File* filePtr)
{
	FileReaderVisitor v(filePtr);
	accept_visitor_as_value_array(&v, visit_raw<FileReaderVisitor, Spy>, sizeof(Spy));
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
static void visit_region_array(Visitor *v, RegionArray *ra)
{
	visit<int32_t>(v, &ra->init_flag);
	visit_pointer(v, &ra->region_info_array);
	visit<int32_t>(v, &ra->region_info_count);
	visit_pointer(v, &ra->region_stat_array);
	visit<int32_t>(v, &ra->region_stat_count);
	visit_pointer(v, &ra->connect_bits);
	visit_array<uint8_t>(v, ra->region_sorted_array);
}

enum { REGION_ARRAY_RECORD_SIZE = 279 };

//-------- Start of function RegionArray::write_file -------------//
//
int RegionArray::write_file(File* filePtr)
{
	if (!visit_with_record_size<FileWriterVisitor>(filePtr, this, &visit_region_array<FileWriterVisitor>,
										 REGION_ARRAY_RECORD_SIZE))
		return 0;

	if( !filePtr->file_write( region_info_array, sizeof(RegionInfo)*region_info_count ) )
		return 0;

	//-------- write RegionStat ----------//

	filePtr->file_put_short( region_stat_count );

	if( !filePtr->file_write( region_stat_array, sizeof(RegionStat)*region_stat_count ) )
		return 0;

	//--------- write connection bits ----------//

	int connectBit = (region_info_count -1) * (region_info_count) /2;
	int connectByte = (connectBit +7) /8;

	if( connectByte > 0)
	{
		if( !filePtr->file_write(connect_bits, connectByte) )
			return 0;
	}

	return 1;
}
//--------- End of function RegionArray::write_file ---------------//


//-------- Start of function RegionArray::read_file -------------//
//
int RegionArray::read_file(File* filePtr)
{
	if (!visit_with_record_size<FileReaderVisitor>(filePtr, this, &visit_region_array<FileReaderVisitor>,
										REGION_ARRAY_RECORD_SIZE))
		return 0;

   if( region_info_count > 0 )
      region_info_array = (RegionInfo *) mem_add(sizeof(RegionInfo)*region_info_count);
   else
      region_info_array = NULL;

   if( !filePtr->file_read( region_info_array, sizeof(RegionInfo)*region_info_count))
      return 0;

	//-------- read RegionStat ----------//

	region_stat_count = filePtr->file_get_short();

	region_stat_array = (RegionStat*) mem_add( region_stat_count * sizeof(RegionStat) );

	if( !filePtr->file_read( region_stat_array, sizeof(RegionStat)*region_stat_count ) )
		return 0;

	//--------- read connection bits ----------//

	int connectBit = (region_info_count -1) * (region_info_count) /2;
	int connectByte = (connectBit +7) /8;

	if( connectByte > 0)
	{
		connect_bits = (unsigned char *)mem_add(connectByte);
		if( !filePtr->file_read(connect_bits, connectByte) )
			return 0;
	}
	else
	{
		connect_bits = NULL;
	}

	return 1;
}
//--------- End of function RegionArray::read_file ---------------//

//*****//

//-------- Start of function NewsArray::write_file -------------//
//
int NewsArray::write_file(File* filePtr)
{
   //----- save news_array parameters -----//

   filePtr->file_write( news_type_option, sizeof(news_type_option) );

   filePtr->file_put_short(news_who_option);
   filePtr->file_put_long (last_clear_recno);

   //---------- save news data -----------//

   FileWriterVisitor v(filePtr);
   accept_visitor_as_value_array(&v, visit_raw<FileWriterVisitor, News>, sizeof(News));
   return v.good();
}
//--------- End of function NewsArray::write_file ---------------//


//-------- Start of function NewsArray::read_file -------------//
//
int NewsArray::read_file(File* filePtr)
{
   //----- read news_array parameters -----//

   filePtr->file_read( news_type_option, sizeof(news_type_option) );

   news_who_option   = (char) filePtr->file_get_short();
   last_clear_recno  = filePtr->file_get_long();

   //---------- read news data -----------//

   FileReaderVisitor v(filePtr);
   accept_visitor_as_value_array(&v, visit_raw<FileReaderVisitor, News>, sizeof(News));
   return v.good();
}
//--------- End of function NewsArray::read_file ---------------//
/* vim:set ts=3 sw=3: */
