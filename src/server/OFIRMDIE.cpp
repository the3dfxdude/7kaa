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

// Filename    : OFIRMDIE.CPP
// Description : destruting firm


#include <OSTR.h>
#include <OFIRMRES.h>
#include <OFIRMDIE.h>
#include <OGAMESET.h>
#include <OGAME.h>
#include <ORESDB.h>
#include <ALL.h>


#define FIRM_BUILD_DB 	"FDBUILD"
#define FIRM_FRAME_DB 	"FDFRAME"
#define FIRM_BITMAP_DB 	"FDBITMAP"


struct FirmDieBitmap : public FirmBitmap
{
	int	bitmap_offset;		// fill bitmap_ptr, width and height before draw()
};

FirmDieRes::FirmDieRes()
{
	firm_build_count = 0;
	firm_bitmap_count = 0;
	firm_build_array = NULL;
	firm_bitmap_array = NULL;
	init_flag = 0;
}

FirmDieRes::~FirmDieRes()
{
	deinit();
}

void FirmDieRes::init()
{
	deinit();

	//----- open firm material bitmap resource file -------//

	String str;

	str  = DIR_RES;
	str += "I_FIRMDI.RES";

	res_bitmap.init_imported(str,0,1);		// 0 - do not read all data into buffer

	//------- load database information --------//

	load_bitmap_info();		// call load_firm_bitmap() first as load_firm_info() will need info loaded by load_firm_bitmap()
	load_build_info();

   //----------------------------------------//

	init_flag=1;
}

void FirmDieRes::deinit()
{
	if(init_flag)
	{
		mem_del(firm_build_array);
		mem_del(firm_bitmap_array);

		res_bitmap.deinit();

		init_flag = 0;
	}
}


//------- Begin of function FirmDieRes::load_firm_bitmap -------//
//
// Read in information of FDBITMAP.DBF into memory array
//
void FirmDieRes::load_bitmap_info()
{
	FirmBitmapRec  *firmBitmapRec;
	FirmDieBitmap	*firmBitmap;
	int      		i;
	long				bitmapOffset;
	Database 		*dbFirmBitmap = game_set.open_db(FIRM_BITMAP_DB);

	firm_bitmap_count = (short) dbFirmBitmap->rec_count();
	firm_bitmap_array = (FirmDieBitmap*) mem_add( sizeof(FirmDieBitmap)*firm_bitmap_count );

	//------ read in firm bitmap info array -------//

	memset( firm_bitmap_array, 0, sizeof(FirmDieBitmap) * firm_bitmap_count );

	for( i=0 ; i<firm_bitmap_count ; i++ )
	{
		firmBitmapRec = (FirmBitmapRec*) dbFirmBitmap->read(i+1);
		firmBitmap    = firm_bitmap_array+i;

		memcpy( &bitmapOffset, firmBitmapRec->bitmap_ptr, sizeof(long) );

		// BUGHERE : bitmap is not yet loaded into memory, fill them before draw()
		firmBitmap->bitmap_ptr = NULL;
		firmBitmap->width  	  = 0;
		firmBitmap->height 	  = 0;

		firmBitmap->offset_x = m.atoi( firmBitmapRec->offset_x, firmBitmapRec->OFFSET_LEN );
		firmBitmap->offset_y = m.atoi( firmBitmapRec->offset_y, firmBitmapRec->OFFSET_LEN );

		firmBitmap->loc_width  = m.atoi( firmBitmapRec->loc_width , firmBitmapRec->LOC_LEN );
		firmBitmap->loc_height = m.atoi( firmBitmapRec->loc_height, firmBitmapRec->LOC_LEN );
		firmBitmap->display_layer = firmBitmapRec->layer - '0';

		firmBitmap->bitmap_offset = bitmapOffset;
	}
}
//--------- End of function FirmDieRes::load_firm_bitmap ---------//


//------- Begin of function FirmDieRes::load_firm_build -------//
//
// Read in information of FIRM.DBF into memory array
//
void FirmDieRes::load_build_info()
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

	//---------- read in FDBUILD.DBF ---------//

	for( i=0 ; i<firm_build_count ; i++ )
	{
		firmBuildRec = (FirmBuildRec*) dbFirmBuild->read(i+1);
		firmBuild	 = firm_build_array+i;

		m.rtrim_fld( firmBuild->build_code, firmBuildRec->race_code, firmBuild->BUILD_CODE_LEN );

		firmBuild->animate_full_size = firmBuildRec->animate_full_size=='1';

		firmBuild->race_id 	  = m.atoi( firmBuildRec->race_id	 , firmBuildRec->RACE_ID_LEN );
		firmBuild->frame_count = m.atoi( firmBuildRec->frame_count, firmBuildRec->FRAME_COUNT_LEN );

		firmBuild->under_construction_bitmap_recno = m.atoi(firmBuildRec->under_construction_bitmap_recno, firmBuildRec->BITMAP_RECNO_LEN);
		firmBuild->idle_bitmap_recno 					 = m.atoi(firmBuildRec->idle_bitmap_recno, firmBuildRec->BITMAP_RECNO_LEN);
		firmBuild->ground_bitmap_recno             = m.atoi(firmBuildRec->ground_bitmap_recno, firmBuildRec->BITMAP_RECNO_LEN);

		err_when( firmBuild->frame_count > MAX_FIRM_FRAME );

		firstFrameArray[i] = m.atoi( firmBuildRec->first_frame, firmBuildRec->FIRST_FRAME_LEN );

		// BUGHERE : need to compare same Firm name and build code in firm database
	}

	//-------- read in FDFRAME.DBF --------//

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

		if( firmBuild->under_construction_bitmap_recno==0 )
			firmBuild->under_construction_bitmap_recno =	bitmapRecno;

		if( firmBuild->idle_bitmap_recno==0 )
			firmBuild->idle_bitmap_recno = bitmapRecno;
	}

	//------ free up the temporary array -------//

	mem_del( firstFrameArray );
}
//--------- End of function FirmDieRes::load_firm_build ---------//


//--------- Begin of function FirmDieRes::get_build ---------//
FirmBuild*	FirmDieRes::get_build(int buildId)
{
	err_when( buildId < 1 || buildId > firm_build_count);
	return firm_build_array+buildId-1;
}
//--------- End of function FirmDieRes::get_build ---------//


//--------- Begin of function FirmDieRes::get_bitmap ---------//
FirmDieBitmap* FirmDieRes::get_bitmap(int bitmapId)
{
	err_when( bitmapId < 1 || bitmapId > firm_bitmap_count);
	return firm_bitmap_array+bitmapId-1;
}
//--------- End of function FirmDieRes::get_bitmap ---------//


//--------- Begin of function FirmDie::init --------//
void FirmDie::init(short firmId, short firmBuildId, short nationRecno,
	short	locX1, short locY1, short locX2, short locY2)
{
	firm_id = firmId;
	firm_build_id = firmBuildId;
	nation_recno = nationRecno;
	loc_x1 = locX1;
	loc_y1 = locY1;
	loc_x2 = locX2;
	loc_y2 = locY2;
	frame = 1;
}

// add before delete the firm
void FirmDie::init(Firm *firmPtr)
{
	firm_id = firmPtr->firm_id;
	firm_build_id = firmPtr->firm_build_id;
	nation_recno = firmPtr->nation_recno;
	loc_x1 = firmPtr->loc_x1;
	loc_y1 = firmPtr->loc_y1;
	loc_x2 = firmPtr->loc_x2;
	loc_y2 = firmPtr->loc_y2;
	frame = 1;
	frame_delay_count = 0;
}



void FirmDie::pre_process()
{
	//nothing
}

int FirmDie::process()
{
	FirmBuild *firmBuild = firm_die_res.get_build(firm_build_id);
	if( ++frame_delay_count > firmBuild->frame_delay_array[frame-1])
	{
		frame_delay_count = 0;
		if( ++frame > 	firmBuild->frame_count)
		{
			return 1;
		}
	}
	return 0;
}


void FirmDie::draw(int displayLayer)
{
	// get ground dirt from firm_res
	FirmBuild* firmBuild = firm_res.get_build(firm_build_id);

	if( firmBuild->ground_bitmap_recno )
	{
//		firm_res.get_bitmap(firmBuild->ground_bitmap_recno)
//			->draw_at(loc_x1*ZOOM_LOC_WIDTH, loc_y1*ZOOM_LOC_HEIGHT, NULL, displayLayer);
	}

	//---------- draw animation now ------------//

	firmBuild = firm_die_res.get_build(firm_build_id);
	FirmDieBitmap* firmBitmap;

	int bitmapRecno, i;
	int firstBitmap = firmBuild->first_bitmap(frame);
	int bitmapCount = firmBuild->bitmap_count(frame);

	char* colorRemapTable = game.get_color_remap_table(nation_recno, 0);

	for( i=0, bitmapRecno=firstBitmap ; i<bitmapCount ; i++, bitmapRecno++ )
	{
		firmBitmap = firm_die_res.get_bitmap(bitmapRecno);

		// BUGHERE : need to load bitmap into memory
		if( firmBitmap )
		{
			char *bitmapPtr;
			firmBitmap->bitmap_ptr = bitmapPtr = firm_die_res.res_bitmap.read_imported(firmBitmap->bitmap_offset);
			firmBitmap->width = *(short *)bitmapPtr;
			firmBitmap->height= *(1+(short *)bitmapPtr);
			firmBitmap->draw_at(loc_x1*ZOOM_LOC_WIDTH, loc_y1*ZOOM_LOC_HEIGHT, colorRemapTable, displayLayer);
		}
	}
}


FirmDieArray::FirmDieArray() : DynArrayB(sizeof(FirmDie),10, DEFAULT_REUSE_INTERVAL_DAYS)
{
	// nothing
}

FirmDieArray::~FirmDieArray()
{
   deinit();
}

void FirmDieArray::init()
{
	zap();
}

void FirmDieArray::deinit()
{
	// nothing
}

void FirmDieArray::process()
{
	int i, j;

	for( i=1, j=size(); j; --j, ++i)
	{
		if( is_deleted(i) )
			continue;

		FirmDie *firmDiePtr = this->operator[](i);

		if( firmDiePtr->process() )
		{
			del(i);
			continue;
		}
	}
}



int FirmDieArray::add(FirmDie *r)
{
	linkin(r);
	return recno();
}

void FirmDieArray::del(int i)
{
	linkout(i);
}


FirmDie *FirmDieArray::operator[](int recNo)
{
   FirmDie* firmDiePtr = (FirmDie*) get(recNo);

   if( !firmDiePtr || is_deleted(recNo) )
      err.run( "FirmDieArray[] is deleted" );

   return firmDiePtr;
}

int FirmDieArray::is_deleted(int recNo)
{
	FirmDie* firmDiePtr = (FirmDie*) get(recNo);
	if( !firmDiePtr || firmDiePtr->firm_id == 0)
		return 1;
	return 0;
}

