/*
 * Seven Kingdoms: Ancient Adversaries
 *
 * Copyright 1997,1998 Enlight Software Ltd.
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

//Filename    : OHILLRES.CPP
//Description : Hill resource object
//Onwership   : Gilbert

#include <OSYS.h>
#include <OVGA.h>
#include <OGAMESET.h>
#include <OWORLD.h>
#include <OHILLRES.h>
#include <OCONFIG.h>
#include <ALL.h>

//---------- #define constant ------------//

// #define HILL_DB   		"HILL"

//------- Begin of function HillRes::HillRes -----------//

HillRes::HillRes()
{
	init_flag=0;
}
//--------- End of function HillRes::HillRes -----------//


//---------- Begin of function HillRes::init -----------//
//
// This function must be called after a map is generated.
//
void HillRes::init()
{
	deinit();

	//----- open hill material bitmap resource file -------//

	String str;

	str  = DIR_RES;
	// str += "I_HILL.RES";
	str += "I_HILL";
	str += config.terrain_set;
	str += ".RES";

	res_bitmap.init_imported(str,1);  // 1-read all into buffer

	//------- load database information --------//

	load_hill_block_info();

	init_flag=1;
}
//---------- End of function HillRes::init -----------//


//---------- Begin of function HillRes::deinit -----------//

void HillRes::deinit()
{
	if( init_flag )
	{
		mem_del(first_block_index);
		mem_del(hill_block_info_array);
		init_flag=0;
	}
}
//---------- End of function HillRes::deinit -----------//


//------- Begin of function HillRes::load_hill_info -------//
//
void HillRes::load_hill_block_info()
{
	HillBlockRec		*hillBlockRec;
	HillBlockInfo		*hillBlockInfo;
	int			i;
	long			bitmapOffset;
	
	//---- read in hill count and initialize hill block info array ----//

	String hillDbName;
	hillDbName = DIR_RES;
	hillDbName += "HILL";
	hillDbName += config.terrain_set;
	hillDbName += ".RES";
	Database hillDbObj(hillDbName, 1);
	// Database *dbHill = game_set.open_db(HILL_DB);	// only one database can be opened at a time
	Database *dbHill = &hillDbObj;

	hill_block_count      = (short) dbHill->rec_count();
	hill_block_info_array = (HillBlockInfo*) mem_add( sizeof(HillBlockInfo)*hill_block_count );

	memset( hill_block_info_array, 0, sizeof(HillBlockInfo) * hill_block_count );
	max_pattern_id = 0;

	//---------- read in HILL.DBF ---------//

	for( i=0 ; i<hill_block_count ; i++ )
	{
		hillBlockRec  = (HillBlockRec*) dbHill->read(i+1);
		hillBlockInfo = hill_block_info_array+i;
		hillBlockInfo->block_id = i + 1;
		hillBlockInfo->pattern_id = (char) m.atoi( hillBlockRec->pattern_id, hillBlockRec->PATTERN_ID_LEN);
		if( hillBlockInfo->pattern_id > max_pattern_id)
			max_pattern_id = hillBlockInfo->pattern_id;
		hillBlockInfo->sub_pattern_id = (char) m.atoi(hillBlockRec->sub_pattern_id, hillBlockRec->SUB_PATTERN_ID_LEN);
		hillBlockInfo->special_flag = hillBlockRec->special_flag;
		if( hillBlockRec->special_flag == ' ')
			hillBlockInfo->special_flag = 0;
		hillBlockInfo->layer = hillBlockRec->layer - '0';

		hillBlockInfo->priority = (char) m.atoi( hillBlockRec->priority, hillBlockRec->PRIORITY_LEN);
		hillBlockInfo->bitmap_type = hillBlockRec->bitmap_type;
		hillBlockInfo->offset_x = m.atoi(hillBlockRec->offset_x, hillBlockRec->OFFSET_LEN);
		hillBlockInfo->offset_y = m.atoi(hillBlockRec->offset_y, hillBlockRec->OFFSET_LEN);

		memcpy( &bitmapOffset, hillBlockRec->bitmap_ptr, sizeof(long) );
		hillBlockInfo->bitmap_ptr = res_bitmap.read_imported(bitmapOffset);
	}

	//------ build index for the first block of each pattern -------//
	// e.g first block id of pattern 1 is 1
	//     first block id of pattern 3 is 4
	//     first block id of pattern 4 is 7
	//     last block id (which is pattern 4) is 10
	// first_block_index is { 1, 4, 4, 7 };
	// such that, blocks which are pattern 1 are between [1,4)
	//                                     2 are between [4,4) i.e. not found
	//                                     3 are between [4,7)
	//                                     4 are between [7,11)
	// see also first_block()
	//
	first_block_index = (short *) mem_add(sizeof(short) * max_pattern_id);
	memset( first_block_index, 0, sizeof(short) * max_pattern_id);
	int patternMarked = 0;
	for(i = 0, hillBlockInfo = hill_block_info_array; i < hill_block_count;
		++i, ++hillBlockInfo)
	{
		err_when( hillBlockInfo->pattern_id < patternMarked);
		while(patternMarked < hillBlockInfo->pattern_id)
		{
			first_block_index[patternMarked] = i+1;
			patternMarked++;
		}
	}

}
//--------- End of function HillRes::load_hill_info ---------//


//---------- Begin of function HillRes::operator[] -----------//

HillBlockInfo* HillRes::operator[](int hillBlockId)
{
	err_if( hillBlockId<1 || hillBlockId>hill_block_count )
		err_now( "HillRes::operator[]" );

	return hill_block_info_array+(hillBlockId-1);
}
//------------ End of function HillRes::operator[] -----------//


//---------- Begin of function HillRes::first_block -----------//
short HillRes::first_block(int hillPatternId)
{
	err_when(hillPatternId<1);
	
	if(hillPatternId>max_pattern_id)
		return hill_block_count+1;		// return last block+1
	else
		return first_block_index[ hillPatternId-1];
}
//------------ End of function HillRes::first_block -----------//


// ####### begin Gilbert 28/1 #######//
//------------ Begin of function HillRes::locate -----------//
short HillRes::locate(char patternId, char subPattern, char searchPriority, char specialFlag)
{
	err_when(patternId < 1 || patternId > max_pattern_id);

	// ------- find the range which patternId may exist ------//
	// find the start of this pattern and next pattern
	short startBlockIdx = first_block(patternId);
	short endBlockIdx = first_block(patternId+1);
	for(short j = startBlockIdx; j < endBlockIdx; ++j)
	{
		HillBlockInfo *hillBlockInfo = hill_block_info_array+j-1;
		if( hillBlockInfo->pattern_id == patternId &&
			hillBlockInfo->sub_pattern_id == subPattern &&
			hillBlockInfo->priority == searchPriority &&
			hillBlockInfo->special_flag == specialFlag)
		{
			return j;
		}
	}
	return 0;	// not found

}
//------------ End of function HillRes::locate -----------//


//------------ Begin of function HillRes::scan -----------//
short HillRes::scan(char patternId, char searchPriority, char specialFlag, char findFirst)
{
	err_when(patternId < 1 || patternId > max_pattern_id);

	// ------- find the range which patternId may exist ------//
	// find the start of this pattern and next pattern
	short startBlockIdx = first_block(patternId);
	short endBlockIdx = first_block(patternId+1);
	short foundBlockId = 0;
	short foundCount = 0;

	for(short j = startBlockIdx; j < endBlockIdx; ++j)
	{
		HillBlockInfo *hillBlockInfo = hill_block_info_array+j-1;
		if( hillBlockInfo->pattern_id == patternId &&
			hillBlockInfo->priority == searchPriority &&
			hillBlockInfo->special_flag == specialFlag)
		{
			if( findFirst)
				return j;
			if( m.random(++foundCount) == 0)
			{
				foundBlockId = j;
			}
		}
	}
	return foundBlockId;	// not found
}
//------------ End of function HillRes::scan -----------//
// ####### end Gilbert 28/1 #######//


//------- Begin of function HillInfo::draw -----------//
//
// Draw the current hill on the map
//
void HillBlockInfo::draw(int xLoc, int yLoc, int layerMask)
{
	if(!(layerMask & layer))
		return;

	//----------- calculate absolute positions ------------//

	int absX1   = xLoc*ZOOM_LOC_WIDTH + offset_x;
	int absY1   = yLoc*ZOOM_LOC_HEIGHT + offset_y;

	//-------- check if the firm is within the view area --------//

	int x1 = absX1 - World::view_top_x;

	if( x1 <= -bitmap_width() || x1 >= ZOOM_WIDTH )	// out of the view area, not even a slight part of it appears in the view area
		return;

	int y1 = absY1 - World::view_top_y;

	if( y1 <= -bitmap_height() || y1 >= ZOOM_HEIGHT )
		return;

	if( bitmap_type == 'W')
	{
		vga_back.put_bitmap_32x32(x1+ZOOM_X1,y1+ZOOM_Y1, bitmap_ptr);
		return;
	}

	//------- decide which approach to use for displaying -----//

	int x2 = absX1 + bitmap_width() - 1 - World::view_top_x;
	int y2 = absY1 + bitmap_height()- 1 - World::view_top_y;

	//---- only portion of the sprite is inside the view area ------//

	if( x1 < 0 || x2 >= ZOOM_WIDTH || y1 < 0 || y2 >= ZOOM_HEIGHT )
	{
		vga_back.put_bitmap_area_trans_decompress( x1+ZOOM_X1, y1+ZOOM_Y1, bitmap_ptr,
			MAX(0,x1)-x1, MAX(0,y1)-y1, MIN(ZOOM_WIDTH-1,x2)-x1, MIN(ZOOM_HEIGHT-1,y2)-y1 );
	}

	//---- the whole sprite is inside the view area ------//

	else
	{
		vga_back.put_bitmap_trans_decompress( x1+ZOOM_X1, y1+ZOOM_Y1, bitmap_ptr );
	}

}
//--------- End of function HillBlockInfo::draw -----------//


//------- Begin of function HillBlockInfo::draw_at -----------//
//
// Draw the hill on the zoom map, given the exact pixel position to put
// the bitmap.
//
// <int> absX1, absY1 - the absolute base (center-bottom) coordination
//										of the building.
//
// Draw the current plant on the map
//
void HillBlockInfo::draw_at(int absX1, int absY1, int layerMask)
{
	if(!(layerMask & layer))
		return;

	//-------- check if the firm is within the view area --------//

	absX1 += offset_x;
	absY1 += offset_y;

	int x1 = absX1 - World::view_top_x;

	if( x1 <= -bitmap_width() || x1 >= ZOOM_WIDTH )	// out of the view area, not even a slight part of it appears in the view area
		return;

	int y1 = absY1 - World::view_top_y;

	if( y1 <= -bitmap_height() || y1 >= ZOOM_HEIGHT )
		return;

	if( bitmap_type == 'W')
	{
		vga_back.put_bitmap_32x32(x1+ZOOM_X1,y1+ZOOM_Y1, bitmap_ptr);
		return;
	}

	//------- decide which approach to use for displaying -----//

	int x2 = absX1 + bitmap_width() - 1 - World::view_top_x;
	int y2 = absY1 + bitmap_height()- 1 - World::view_top_y;

	//---- only portion of the sprite is inside the view area ------//

	if( x1 < 0 || x2 >= ZOOM_WIDTH || y1 < 0 || y2 >= ZOOM_HEIGHT )
	{
		vga_back.put_bitmap_area_trans_decompress( x1+ZOOM_X1, y1+ZOOM_Y1, bitmap_ptr,
			MAX(0,x1)-x1, MAX(0,y1)-y1, MIN(ZOOM_WIDTH-1,x2)-x1, MIN(ZOOM_HEIGHT-1,y2)-y1 );
	}

	//---- the whole sprite is inside the view area ------//

	else
	{
		vga_back.put_bitmap_trans_decompress( x1+ZOOM_X1, y1+ZOOM_Y1, bitmap_ptr );
	}

}
//--------- End of function HillBlockInfo::draw_at -----------//
