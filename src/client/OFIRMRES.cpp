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

//Filename    : OFIRMRES.CPP
//Description : Firm material resource object

#include <OSYS.h>
#include <OGAMESET.h>
#include <OWORLD.h>
#include <OUNIT.h>
#include <ONATION.h>
#include <OFIRMRES.h>
#if(defined(GERMAN) || defined(FRENCH) || defined(SPANISH))
#include <OTRANSL.h>
#endif

//---------- #define constant ------------//

#define FIRM_DB   		"FIRM"
#define FIRM_BUILD_DB 	"FBUILD"
#define FIRM_FRAME_DB 	"FFRAME"
#define FIRM_BITMAP_DB 	"FBITMAP"

//------- Begin of function FirmRes::FirmRes -----------//

FirmRes::FirmRes()
{
	init_flag=0;
}
//--------- End of function FirmRes::FirmRes -----------//


//---------- Begin of function FirmRes::init -----------//
//
// This function must be called after a map is generated.
//
void FirmRes::init()
{
	deinit();

	//----- open firm material bitmap resource file -------//

	String str;

	str  = DIR_RES;
	str += "I_FIRM.RES";

	res_bitmap.init_imported(str,1);  // 1-read all into buffer

	//------- load database information --------//

	load_firm_bitmap();		// call load_firm_bitmap() first as load_firm_info() will need info loaded by load_firm_bitmap()
	load_firm_build();
	load_firm_info();

	//------------ set firm skill ------------//

	firm_res[FIRM_BASE]->firm_skill_id = SKILL_LEADING;
	firm_res[FIRM_CAMP]->firm_skill_id = SKILL_LEADING;
	firm_res[FIRM_MINE]->firm_skill_id = SKILL_MINING;
	firm_res[FIRM_FACTORY]->firm_skill_id = SKILL_MFT;
	firm_res[FIRM_RESEARCH]->firm_skill_id = SKILL_RESEARCH;
	firm_res[FIRM_WAR_FACTORY]->firm_skill_id = SKILL_MFT;

   //----------------------------------------//

	init_flag=1;
}
//---------- End of function FirmRes::init -----------//


//---------- Begin of function FirmRes::deinit -----------//

void FirmRes::deinit()
{
	if( init_flag )
	{
		mem_del(firm_info_array);
		mem_del(firm_build_array);
		mem_del(firm_bitmap_array);

		res_bitmap.deinit();

		init_flag=0;
	}
}
//---------- End of function FirmRes::deinit -----------//


//------- Begin of function FirmRes::load_firm_bitmap -------//
//
// Read in information of FBITMAP.DBF into memory array
//
void FirmRes::load_firm_bitmap()
{
	FirmBitmapRec  *firmBitmapRec;
	FirmBitmap     *firmBitmap;
	int      		i;
	long				bitmapOffset;
	Database 		*dbFirmBitmap = game_set.open_db(FIRM_BITMAP_DB);

	firm_bitmap_count = (short) dbFirmBitmap->rec_count();
	firm_bitmap_array = (FirmBitmap*) mem_add( sizeof(FirmBitmap)*firm_bitmap_count );

	//------ read in firm bitmap info array -------//

	memset( firm_bitmap_array, 0, sizeof(FirmBitmap) * firm_bitmap_count );

	for( i=0 ; i<firm_bitmap_count ; i++ )
	{
		firmBitmapRec = (FirmBitmapRec*) dbFirmBitmap->read(i+1);
		firmBitmap    = firm_bitmap_array+i;

		memcpy( &bitmapOffset, firmBitmapRec->bitmap_ptr, sizeof(long) );

		firmBitmap->bitmap_ptr = res_bitmap.read_imported(bitmapOffset);
		firmBitmap->width  	  = *((short*)firmBitmap->bitmap_ptr);
		firmBitmap->height 	  = *(((short*)firmBitmap->bitmap_ptr)+1);

		firmBitmap->offset_x = m.atoi( firmBitmapRec->offset_x, firmBitmapRec->OFFSET_LEN );
		firmBitmap->offset_y = m.atoi( firmBitmapRec->offset_y, firmBitmapRec->OFFSET_LEN );

		firmBitmap->loc_width  = m.atoi( firmBitmapRec->loc_width , firmBitmapRec->LOC_LEN );
		firmBitmap->loc_height = m.atoi( firmBitmapRec->loc_height, firmBitmapRec->LOC_LEN );
		firmBitmap->display_layer = firmBitmapRec->layer - '0';
	}
}
//--------- End of function FirmRes::load_firm_bitmap ---------//


//------- Begin of function FirmRes::load_firm_build -------//
//
// Read in information of FIRM.DBF into memory array
//
void FirmRes::load_firm_build()
{
	FirmBuildRec *firmBuildRec;
	FirmFrameRec *firmFrameRec;
	FirmBuild 	 *firmBuild;
	FirmBitmap   *firmBitmap;
	int      	 i, j, k, frameRecno, bitmapRecno;
	short			 *firstFrameArray;

	//---- read in firm count and initialize firm info array ----//

	Database *dbFirmBuild = game_set.open_db(FIRM_BUILD_DB);	// only one database can be opened at a time, so we read FIRM.DBF first

	firm_build_count = (short) dbFirmBuild->rec_count();
	firm_build_array = (FirmBuild*) mem_add( sizeof(FirmBuild)*firm_build_count );

	memset( firm_build_array, 0, sizeof(FirmBuild) * firm_build_count );

	//------ allocate an array for storing firstFrameRecno -----//

	firstFrameArray = (short*) mem_add( sizeof(short) * firm_build_count );

	//---------- read in FBUILD.DBF ---------//

	for( i=0 ; i<firm_build_count ; i++ )
	{
		firmBuildRec = (FirmBuildRec*) dbFirmBuild->read(i+1);
		firmBuild	 = firm_build_array+i;

		m.rtrim_fld( firmBuild->build_code, firmBuildRec->race_code, firmBuild->BUILD_CODE_LEN );

		firmBuild->animate_full_size = firmBuildRec->animate_full_size=='1';

		firmBuild->race_id 	  = m.atoi( firmBuildRec->race_id	 , firmBuildRec->RACE_ID_LEN );
		firmBuild->frame_count = m.atoi( firmBuildRec->frame_count, firmBuildRec->FRAME_COUNT_LEN );

		firmBuild->under_construction_bitmap_recno = m.atoi(firmBuildRec->under_construction_bitmap_recno, firmBuildRec->BITMAP_RECNO_LEN);
		// ##### begin Gilbert 18/10 ########//
		firmBuild->under_construction_bitmap_count =
			m.atoi(firmBuildRec->under_construction_bitmap_count, firmBuildRec->FRAME_COUNT_LEN);
		// ##### end Gilbert 18/10 ########//
		firmBuild->idle_bitmap_recno 					 = m.atoi(firmBuildRec->idle_bitmap_recno, firmBuildRec->BITMAP_RECNO_LEN);
		firmBuild->ground_bitmap_recno             = m.atoi(firmBuildRec->ground_bitmap_recno, firmBuildRec->BITMAP_RECNO_LEN);

		err_when( firmBuild->frame_count > MAX_FIRM_FRAME );

		firstFrameArray[i] = m.atoi( firmBuildRec->first_frame, firmBuildRec->FIRST_FRAME_LEN );
	}

	//-------- read in FFRAME.DBF --------//

	Database 	*dbFirmFrame = game_set.open_db(FIRM_FRAME_DB);
	int 			minOffsetX, minOffsetY;
	int			maxX2, maxY2;

	for( i=0 ; i<firm_build_count ; i++ )
	{
		firmBuild  = firm_build_array+i;
		frameRecno = firstFrameArray[i];

		minOffsetX = minOffsetY = 0xFFFF;
		maxX2 = maxY2 = 0;

		for( j=0 ; j<firmBuild->frame_count ; j++, frameRecno++ )
		{
			firmFrameRec = (FirmFrameRec*) dbFirmFrame->read(frameRecno);

			//------ following animation frames, bitmap sections -----//

			firmBuild->first_bitmap_array[j] = m.atoi( firmFrameRec->first_bitmap, firmFrameRec->FIRST_BITMAP_LEN );
			firmBuild->bitmap_count_array[j] = m.atoi( firmFrameRec->bitmap_count, firmFrameRec->BITMAP_COUNT_LEN );

			firmBuild->frame_delay_array[j] = m.atoi( firmFrameRec->delay, firmFrameRec->DELAY_LEN );

			//---- get the MIN offset_x, offset_y and MAX width, height ----//
			//
			// So we can get the largest area of all the frames in this building
			// and this will serve as a normal size setting for this building,
			// with variation from frame to frame
			//
			//--------------------------------------------------------------//

			firmBitmap = firm_bitmap_array + firmBuild->first_bitmap_array[j] - 1;

			for( k=firmBuild->bitmap_count_array[j] ; k>0 ; k--, firmBitmap++ )
			{
				if( firmBitmap->offset_x < minOffsetX )
					minOffsetX = firmBitmap->offset_x;

				if( firmBitmap->offset_y < minOffsetY )
					minOffsetY = firmBitmap->offset_y;

				if( firmBitmap->offset_x + firmBitmap->width > maxX2 )
					maxX2 = firmBitmap->offset_x + firmBitmap->width;

				if( firmBitmap->offset_y + firmBitmap->height > maxY2 )
					maxY2 = firmBitmap->offset_y + firmBitmap->height;
			}
		}

		//------- set FirmBuild Info -------//

		bitmapRecno = firmBuild->first_bitmap_array[0];

		//----- get the info of the first frame bitmap ----//

		firmBitmap = firm_bitmap_array + bitmapRecno - 1;

		firmBuild->loc_width  = firmBitmap->loc_width;
		firmBuild->loc_height = firmBitmap->loc_height;

		firmBuild->min_offset_x = minOffsetX;
		firmBuild->min_offset_y = minOffsetY;

		firmBuild->max_bitmap_width  = maxX2 - minOffsetX;
		firmBuild->max_bitmap_height = maxY2 - minOffsetY;

		//------ set firmBuild's under construction and idle bitmap recno -----//

		// ####### begin Gilbert 18/10 #######//
		if( firmBuild->under_construction_bitmap_recno==0 )
		{
			firmBuild->under_construction_bitmap_recno = bitmapRecno;
			firmBuild->under_construction_bitmap_count = 1;
		}

		err_when(firmBuild->under_construction_bitmap_count == 0);
		// ####### end Gilbert 18/10 #######//

		if( firmBuild->idle_bitmap_recno==0 )
			firmBuild->idle_bitmap_recno = bitmapRecno;
	}

	//------ free up the temporary array -------//

	mem_del( firstFrameArray );
}
//--------- End of function FirmRes::load_firm_build ---------//


//------- Begin of function FirmRes::load_firm_info -------//
//
// Read in information of FIRM.DBF into memory array
//
void FirmRes::load_firm_info()
{
	FirmRec  	 *firmRec;
	FirmInfo 	 *firmInfo;
	FirmBuild	 *firmBuild;
	int      	 i;

	//---- read in firm count and initialize firm info array ----//

	Database *dbFirm = game_set.open_db(FIRM_DB);	// only one database can be opened at a time, so we read FIRM.DBF first

	firm_count      = (short) dbFirm->rec_count();
	firm_info_array = (FirmInfo*) mem_add( sizeof(FirmInfo)*firm_count );

	memset( firm_info_array, 0, sizeof(FirmInfo) * firm_count );

	//---------- read in FIRM.DBF ---------//

	for( i=0 ; i<firm_count ; i++ )
	{
		firmRec  = (FirmRec*) dbFirm->read(i+1);
		firmInfo = firm_info_array+i;

		m.rtrim_fld( firmInfo->name, firmRec->name, firmRec->NAME_LEN );
		m.rtrim_fld( firmInfo->short_name, firmRec->short_name, firmRec->SHORT_NAME_LEN );

		m.rtrim_fld( firmInfo->overseer_title, firmRec->overseer_title, firmRec->TITLE_LEN );
		m.rtrim_fld( firmInfo->worker_title  , firmRec->worker_title  , firmRec->TITLE_LEN );
#if(defined(GERMAN) || defined(FRENCH) || defined(SPANISH))
		translate.multi_to_win(firmInfo->name, firmInfo->NAME_LEN);
		translate.multi_to_win(firmInfo->short_name, firmInfo->SHORT_NAME_LEN);
		translate.multi_to_win(firmInfo->overseer_title, firmInfo->TITLE_LEN);
		translate.multi_to_win(firmInfo->worker_title, firmInfo->TITLE_LEN);
#endif

		firmInfo->firm_id     	 	  = i+1;
		firmInfo->tera_type   	 	  = firmRec->tera_type-'0';
		firmInfo->live_in_town   	  = firmRec->live_in_town=='1';

		firmInfo->max_hit_points 	  = m.atoi( firmRec->hit_points, firmRec->HIT_POINTS_LEN );

		firmInfo->first_build_id = m.atoi( firmRec->first_build, firmRec->FIRST_BUILD_LEN );
		firmInfo->build_count	 = m.atoi( firmRec->build_count, firmRec->BUILD_COUNT_LEN );

		firmInfo->need_overseer  = firmInfo->overseer_title[0] && firmInfo->overseer_title[0] != ' ';
		firmInfo->need_worker  	 = firmInfo->worker_title[0]   && firmInfo->worker_title[0]   != ' ';

		firmInfo->is_linkable_to_town = firmRec->is_linkable_to_town=='1';

		firmInfo->setup_cost		 = m.atoi( firmRec->setup_cost, firmRec->COST_LEN );
		firmInfo->year_cost		 = m.atoi( firmRec->year_cost, firmRec->COST_LEN );

		firmInfo->buildable		 = firmInfo->setup_cost > 0;

		if( firmRec->all_know=='1' )
			memset( firmInfo->nation_tech_level_array, 1, sizeof(firmInfo->nation_tech_level_array) );

		//------- set loc_width & loc_height in FirmInfo --------//

		firmBuild = firm_build_array+firmInfo->first_build_id-1;

		firmInfo->loc_width  = firmBuild->loc_width;
		firmInfo->loc_height = firmBuild->loc_height;

		//------------- set firm_race_id --------------//

		if( firmInfo->build_count==1 )       	  // if only one building style for this firm, take the race id. of the building as the race of the firm
			firmInfo->firm_race_id = (char) firmBuild->race_id;
	}
}
//--------- End of function FirmRes::load_firm_info ---------//


//---------- Begin of function FirmRes::operator[] -----------//

FirmInfo* FirmRes::operator[](int firmId)
{
	err_if( firmId<1 || firmId>firm_count )
		err_now( "FirmRes::operator[]" );

	return firm_info_array+firmId-1;
}
//------------ End of function FirmRes::operator[] -----------//


//-------- Start of function FirmInfo::can_build -------------//
//
// Whether unit of this race can build this firm or not.
//
// <int> unitRecno - check whether this unit knows how to build
//						   this firm.
//
int FirmInfo::can_build(int unitRecno)
{
	if( !buildable )
		return 0;

	Unit* unitPtr = unit_array[unitRecno];

	if( !unitPtr->nation_recno )
		return 0;

	if( !(get_nation_tech_level(unitPtr->nation_recno) > 0) )
		return 0;

	//------ fortress of power ------//

	if( firm_id == FIRM_BASE )	// only if the nation has acquired the myth to build it
	{
		if( unitPtr->rank_id == RANK_GENERAL ||
			 unitPtr->rank_id == RANK_KING ||
			 unitPtr->skill.skill_id == SKILL_PRAYING ||
			 unitPtr->skill.skill_id == SKILL_CONSTRUCTION )
		{
			//----- each nation can only build one seat of power -----//

			if( unitPtr->nation_recno > 0 &&
				 unitPtr->race_id > 0 &&
				 nation_array[unitPtr->nation_recno]->base_count_array[unitPtr->race_id-1] == 0 )
			{
				//--- if this nation has acquired the needed scroll of power ---//

				return nation_array[unitPtr->nation_recno]->know_base_array[unitPtr->race_id-1];
			}
		}

		return 0;
	}

	//------ a king or a unit with construction skill knows how to build all buildings -----//

	if( firm_race_id == 0 )
	{
		if( unitPtr->rank_id == RANK_KING || unitPtr->skill.skill_id == SKILL_CONSTRUCTION )
			return 1;
	}

	//----- if the firm is race specific, if the unit is right race, return true ----//

	if( firm_race_id == unitPtr->race_id )
		return 1;

	//---- if the unit has the skill needed by the firm or the unit has general construction skill ----//

	if( firm_skill_id && firm_skill_id == unitPtr->skill.skill_id )
		return 1;

	return 0;
}
//--------- End of function FirmInfo::can_build ---------------//


//-------- Start of function FirmInfo::is_linkable_to_firm -------------//
//
int FirmInfo::is_linkable_to_firm(int linkFirmId)
{
	switch(firm_id)
	{
		case FIRM_FACTORY:
			return linkFirmId==FIRM_MINE || linkFirmId==FIRM_MARKET || linkFirmId==FIRM_HARBOR;

		case FIRM_MINE:
			return linkFirmId==FIRM_FACTORY || linkFirmId==FIRM_MARKET || linkFirmId==FIRM_HARBOR;

		case FIRM_MARKET:
			return linkFirmId==FIRM_MINE || linkFirmId==FIRM_FACTORY || linkFirmId==FIRM_HARBOR;

		case FIRM_INN:                   // for an inn to scan for neighbor inns quickly, the link line is not displayed
			return linkFirmId==FIRM_INN;

		case FIRM_HARBOR:
			return linkFirmId==FIRM_MARKET || linkFirmId==FIRM_MINE || linkFirmId==FIRM_FACTORY;

		default:
			return 0;
	}
}
//--------- End of function FirmInfo::is_linkable_to_firm ---------------//


//------ Start of function FirmInfo::default_link_status --------//
//
// return: <int> 1 - the default status is <enable>
//					  0 - the default status is <disable>
//
int FirmInfo::default_link_status(int linkFirmId)
{
	int rc;

	switch(firm_id)
	{
		case FIRM_MINE:
			rc = (linkFirmId!=FIRM_MARKET);
			break;

		case FIRM_FACTORY:
			rc = (linkFirmId==FIRM_MARKET) || (linkFirmId==FIRM_MINE);
			break;

		case FIRM_MARKET:
			rc = (linkFirmId==FIRM_FACTORY) || (linkFirmId==FIRM_HARBOR);
			break;

		case FIRM_HARBOR:
			rc = (linkFirmId==FIRM_MARKET) || (linkFirmId==FIRM_MINE) ||
				  (linkFirmId==FIRM_FACTORY);
			break;

		default:
			rc = 1;
	}

	if( rc )
		return LINK_EE;
	else
		return LINK_DD;
}
//------- End of function FirmInfo::default_link_status -------//


//-------- Start of function FirmInfo::get_build_id -------------//
//
// Give the build code and return the build id.
//
// <char*> buildCode - the build code
//
int FirmInfo::get_build_id(const char* buildCode)
{
	err_when( build_count<1 );		

	if( build_count==1 )		// if this firm has only one building type
		return first_build_id;

	int firmBuildId = first_build_id;

	for( int i=0 ; i<build_count ; i++, firmBuildId++ )              // if this firm has one graphics for each race
	{
		if( strcmp(buildCode, firm_res.get_build(firmBuildId)->build_code)==0 )
			return firmBuildId;
	}

	err_here();

	return 0;
}
//--------- End of function FirmInfo::get_build_id ---------------//


//---- Begin of function FirmInfo::inc_nation_firm_count ----//

void FirmInfo::inc_nation_firm_count(int nationRecno)
{
	err_when( nationRecno<1 || nationRecno>nation_array.size() );

	nation_firm_count_array[nationRecno-1]++;

   nation_array[nationRecno]->total_firm_count++;
}
//----- End of function FirmInfo::inc_nation_firm_count -----//


//---- Begin of function FirmInfo::dec_nation_firm_count ----//

void FirmInfo::dec_nation_firm_count(int nationRecno)
{
	nation_firm_count_array[nationRecno-1]--;

	nation_array[nationRecno]->total_firm_count--;

	err_when( nation_firm_count_array[nationRecno-1] < 0 );

	if( nation_firm_count_array[nationRecno-1] < 0 )		// run-time bug fixing
		nation_firm_count_array[nationRecno-1] = 0;
}
//----- End of function FirmInfo::dec_nation_firm_count -----//


//--------- begin of function FirmBitmap::draw_at ---------//

void FirmBitmap::draw_at(int absX, int absY, char *colorRemapTable, int displayLayer)
{
	// -------- skip if display layer is not correct --------//

	if( !(displayLayer & display_layer))
		return;

	//-------- check if the firm is within the view area --------//

	int x1 = absX + offset_x - World::view_top_x;
	int x2 = x1 + width -1;
	if( x1 >= ZOOM_WIDTH || x2 < 0)
		return;
	int y1 = absY + offset_y - World::view_top_y;
	int y2 = y1 + height -1;
	if( y1 >= ZOOM_HEIGHT || y2 < 0)
		return;

	//---- only portion of the sprite is inside the view area ------//

	if( x1 < 0 || x2 >= ZOOM_WIDTH || y1 < 0 || y2 >= ZOOM_HEIGHT )
	{
		int srcX1 = x1<0 ? -x1 : 0;
		int srcY1 = y1<0 ? -y1 : 0;
		int srcX2 = x2>=ZOOM_WIDTH ? ZOOM_WIDTH-1-x1 : width-1;
		int srcY2 = y2>=ZOOM_HEIGHT ? ZOOM_HEIGHT-1-y1 : height-1;

		if( colorRemapTable )
		{
			vga_back.put_bitmap_area_trans_remap_decompress( x1+ZOOM_X1, y1+ZOOM_Y1,
				bitmap_ptr, srcX1, srcY1, srcX2, srcY2, colorRemapTable );
		}
		else
		{
			vga_back.put_bitmap_area_trans_decompress( x1+ZOOM_X1, y1+ZOOM_Y1,
				bitmap_ptr, srcX1, srcY1, srcX2, srcY2 );
		}
	}

	//---- the whole sprite is inside the view area ------//

	else
	{
		if( colorRemapTable )
		{
			vga_back.put_bitmap_trans_remap_decompress( x1+ZOOM_X1, y1+ZOOM_Y1, bitmap_ptr, colorRemapTable );
		}
		else
		{
			vga_back.put_bitmap_trans_decompress( x1+ZOOM_X1, y1+ZOOM_Y1, bitmap_ptr );
		}
	}
}
// --------- end of function FirmBitmap::draw_at ---------//
