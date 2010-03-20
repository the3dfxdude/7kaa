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

// Filename    : OROCKRES.CPP
// Description : rock resource
// Owner       : Gilbert


#include <OROCKRES.h>
#include <OGAMESET.h>
#include <OMISC.h>
#include <OWORLD.h>
#include <OVGABUF.h>
#include <OTERRAIN.h>
#include <OCONFIG.h>
#include <string.h>

// ---------- Define constant ------------//
//#define ROCK_DB        "ROCK"
//#define ROCK_BLOCK_DB  "ROCKBLK"
//#define ROCK_BITMAP_DB "ROCKBMP"
//#define ROCK_ANIM_DB   "ROCKANIM"

// one rock is composed of one or many rock blocks
// each rock block has one or many frames
// each rock has its own animation sequence
// any animation sequence can change to one of the two sequences


// ------------ begin of function RockRes::RockRes -----------//
RockRes::RockRes()
{
	init_flag = 0;

	rock_info_count = 0;
	rock_info_array = NULL;

	rock_block_count = 0;
	rock_block_array = NULL;

	rock_bitmap_count = 0;
	rock_bitmap_array = NULL;

	rock_anim_count = 0;
	rock_anim_array = NULL;
}
// ------------ end of function RockRes::RockRes -----------//


// ------------ begin of function RockRes::~RockRes -----------//
RockRes::~RockRes()
{
	deinit();
}
// ------------ end of function RockRes::~RockRes -----------//


// ------------ begin of function RockRes::init -----------//
void RockRes::init()
{
	deinit();

	//----- open rock bitmap resource file -------//

	String str;

	str  = DIR_RES;
	// str += "I_ROCK.RES";
	str += "I_ROCK";
	str += config.terrain_set;
	str += ".RES";

	res_bitmap.init_imported(str,1);  // 1-read all into buffer

	//------- load database information --------//

	load_info();
	load_bitmap_info();
	load_block_info();
	load_anim_info();

   //----------------------------------------//

	init_flag=1;
}
// ------------ end of function RockRes::init -----------//


// ------------ begin of function RockRes::deinit -----------//
void RockRes::deinit()
{
	if(init_flag)
	{
		mem_del(rock_info_array);
		mem_del(rock_block_array);
		mem_del(rock_bitmap_array);
		mem_del(rock_anim_array);
		rock_info_count = 0;
		rock_block_count = 0;
		rock_bitmap_count = 0;
		rock_anim_count = 0;

		res_bitmap.deinit();

		init_flag = 0;
	}
}
// ------------ end of function RockRes::deinit -----------//


// ------------ begin of function RockRes::load_info -----------//
void RockRes::load_info()
{
	RockRec  	 *rockRec;
	RockInfo 	 *rockInfo;
	int      	 i;

	//---- read in rock count and initialize rock info array ----//

	String rockDbName;
	rockDbName  = DIR_RES;
	rockDbName += "ROCK";
	rockDbName += config.terrain_set;
	rockDbName += ".RES";
	Database rockDbObj(rockDbName, 1);
	//Database *dbRock = game_set.open_db(ROCK_DB);	// only one database can be opened at a time, so we read ROCK.DBF first
	Database *dbRock = &rockDbObj;

	rock_info_count = (short) dbRock->rec_count();
	rock_info_array = (RockInfo*) mem_add( sizeof(RockInfo)*rock_info_count );

	memset( rock_info_array, 0, sizeof(RockInfo) * rock_info_count );

	//---------- read in ROCK.DBF ---------//

	for( i=0 ; i<rock_info_count ; i++ )
	{
		rockRec  = (RockRec*) dbRock->read(i+1);
		rockInfo = rock_info_array+i;

		m.rtrim_fld( rockInfo->rock_name, rockRec->rock_id, rockRec->ROCKID_LEN );
		rockInfo->rock_type   	 	  = rockRec->rock_type;
		rockInfo->loc_width          = m.atoi(rockRec->loc_width, rockRec->LOC_LEN);
		rockInfo->loc_height         = m.atoi(rockRec->loc_height, rockRec->LOC_LEN);
		// ###### begin Gilbert 2/5 ##########//
		if( rockRec->terrain_1 == 0 || rockRec->terrain_1 == ' ')
			rockInfo->terrain_1 = 0;
		else
			rockInfo->terrain_1       = TerrainRes::terrain_code(rockRec->terrain_1);
		if( rockRec->terrain_2 == 0 || rockRec->terrain_2 == ' ')
			rockInfo->terrain_2 = 0;
		else
			rockInfo->terrain_2       = TerrainRes::terrain_code(rockRec->terrain_2);
		// ###### end Gilbert 2/5 ##########//
		rockInfo->first_anim_recno   = m.atoi(rockRec->first_anim_recno, rockRec->RECNO_LEN);
		if(rockInfo->first_anim_recno)
			rockInfo->max_frame          = m.atoi(rockRec->max_frame, rockRec->MAX_FRAME_LEN);
		else
			rockInfo->max_frame          = 1;			// unanimated, rock anim recno must be -1

		rockInfo->first_block_recno  = 0;
	}
}
// ------------ end of function RockRes::load_info -----------//


// ------------ begin of function RockRes::load_bitmap_info -----------//
void RockRes::load_bitmap_info()
{
	RockBitmapRec  	 *rockBitmapRec;
	RockBitmapInfo 	 *rockBitmapInfo;
	int      	 i;

	//---- read in rock count and initialize rock info array ----//

	String rockDbName;
	rockDbName  = DIR_RES;
	rockDbName += "ROCKBMP";
	rockDbName += config.terrain_set;
	rockDbName += ".RES";
	Database rockDbObj(rockDbName, 1);
	// Database *dbRock = game_set.open_db(ROCK_BITMAP_DB);	// only one database can be opened at a time, so we read ROCK.DBF first
	Database *dbRock = &rockDbObj;

	rock_bitmap_count = (short) dbRock->rec_count();
	rock_bitmap_array = (RockBitmapInfo*) mem_add( sizeof(RockBitmapInfo)*rock_bitmap_count );

	memset( rock_bitmap_array, 0, sizeof(RockBitmapInfo) * rock_bitmap_count );

	//---------- read in ROCKBMP.DBF ---------//

	for( i=0 ; i<rock_bitmap_count ; i++ )
	{
		rockBitmapRec  = (RockBitmapRec*) dbRock->read(i+1);
		rockBitmapInfo = rock_bitmap_array+i;

		rockBitmapInfo->loc_x = m.atoi(rockBitmapRec->loc_x, rockBitmapRec->LOC_LEN);
		rockBitmapInfo->loc_y = m.atoi(rockBitmapRec->loc_y, rockBitmapRec->LOC_LEN);
		rockBitmapInfo->frame = m.atoi(rockBitmapRec->frame, rockBitmapRec->FRAME_NO_LEN);

		long bitmapOffset;
		memcpy( &bitmapOffset, rockBitmapRec->bitmap_ptr, sizeof(long) );
		rockBitmapInfo->bitmap_ptr = res_bitmap.read_imported(bitmapOffset);
	}
}
// ------------ end of function RockRes::load_bitmap_info -----------//


// ------------ begin of function RockRes::load_block_info -----------//
void RockRes::load_block_info()
{
	RockBlockRec  	 *rockBlockRec;
	RockBlockInfo 	 *rockBlockInfo;
	int              i;

	//---- read in rock count and initialize rock info array ----//

	String rockDbName;
	rockDbName  = DIR_RES;
	rockDbName += "ROCKBLK";
	rockDbName += config.terrain_set;
	rockDbName += ".RES";
	Database rockDbObj(rockDbName, 1);
	// Database *dbRock = game_set.open_db(ROCK_BLOCK_DB);	// only one database can be opened at a time, so we read ROCK.DBF first
	Database *dbRock = &rockDbObj;

	rock_block_count = (short) dbRock->rec_count();
	rock_block_array = (RockBlockInfo*) mem_add( sizeof(RockBlockInfo)*rock_block_count );

	memset( rock_block_array, 0, sizeof(RockBlockInfo) * rock_block_count );

	//---------- read in ROCKBLK.DBF ---------//

	for( i=0 ; i<rock_block_count ; i++ )
	{
		rockBlockRec  = (RockBlockRec*) dbRock->read(i+1);
		rockBlockInfo = rock_block_array+i;

		rockBlockInfo->loc_x          = m.atoi(rockBlockRec->loc_x, rockBlockRec->LOC_LEN);
		rockBlockInfo->loc_y          = m.atoi(rockBlockRec->loc_y, rockBlockRec->LOC_LEN);
		rockBlockInfo->rock_recno     = m.atoi(rockBlockRec->rock_recno, rockBlockRec->RECNO_LEN);
		rockBlockInfo->first_bitmap   = m.atoi(rockBlockRec->first_bitmap, rockBlockRec->RECNO_LEN);

		// ------- validate rock_recno --------//
		err_when( rockBlockInfo->rock_recno <= 0 || rockBlockInfo->rock_recno > rock_info_count);
		RockInfo* rockInfo = rock_info_array+rockBlockInfo->rock_recno-1;
		err_when( strncmp(rockBlockRec->rock_id, rockInfo->rock_name, strlen(rockInfo->rock_name)));
		if(rockInfo->first_block_recno == 0)
			rockInfo->first_block_recno = i + 1;

		// ------- set block_offset in rockInfo ----------//
		if( rockBlockInfo->loc_x < MAX_ROCK_WIDTH &&
			rockBlockInfo->loc_y < MAX_ROCK_HEIGHT)
		{
			// store the rockBlockRecno (i.e. i+1) in rockInfo->block_offset
			// in order to find a rock block from rock recno and x offset, y offset
			// thus make RockRes::locate_block() faster
			rockInfo->block_offset[rockBlockInfo->loc_y][rockBlockInfo->loc_x] = i + 1;
		}

#ifdef DEBUG
		// -------- validate loc_x and loc_y ----------//
		err_when( rockBlockInfo->loc_x >= rockInfo->loc_width );
		err_when( rockBlockInfo->loc_y >= rockInfo->loc_height );

		// -------- validate rockBitmapInfo -------//
		int blockFrame = rockInfo->max_frame;
		for(int f = 1; f <= blockFrame; ++f)
		{
			RockBitmapInfo *rockBitmapInfo = rock_bitmap_array + (rockBlockInfo->first_bitmap-1) + (f-1);
			err_when( rockBlockInfo->loc_x != rockBitmapInfo->loc_x);
			err_when( rockBlockInfo->loc_y != rockBitmapInfo->loc_y);
			err_when( f != rockBitmapInfo->frame);
		}
#endif
	}
}
// ------------ end of function RockRes::load_block_info -----------//


// ------------ begin of function RockRes::load_anim_info -----------//
void RockRes::load_anim_info()
{
	RockAnimRec  	 *rockAnimRec;
	RockAnimInfo 	 *rockAnimInfo;
	int      	 i;

	//---- read in rock count and initialize rock info array ----//

	String rockDbName;
	rockDbName  = DIR_RES;
	rockDbName += "ROCKANI";
	rockDbName += config.terrain_set;
	rockDbName += ".RES";
	Database rockDbObj(rockDbName, 1);
	// Database *dbRock = game_set.open_db(ROCK_ANIM_DB);	// only one database can be opened at a time, so we read ROCK.DBF first
	Database *dbRock = &rockDbObj;

	rock_anim_count = (short) dbRock->rec_count();
	rock_anim_array = (RockAnimInfo*) mem_add( sizeof(RockAnimInfo)*rock_anim_count );

	memset( rock_anim_array, 0, sizeof(RockAnimInfo) * rock_anim_count );

	//---------- read in ROCKANIM.DBF ---------//

	for( i=0 ; i<rock_anim_count ; i++ )
	{
		rockAnimRec  = (RockAnimRec*) dbRock->read(i+1);
		rockAnimInfo = rock_anim_array+i;

		rockAnimInfo->frame   	 = m.atoi(rockAnimRec->frame, rockAnimRec->FRAME_NO_LEN);
		rockAnimInfo->delay      = m.atoi(rockAnimRec->delay, rockAnimRec->DELAY_LEN);
		rockAnimInfo->next_frame = m.atoi(rockAnimRec->next_frame, rockAnimRec->FRAME_NO_LEN);
		rockAnimInfo->alt_next   = m.atoi(rockAnimRec->alt_next, rockAnimRec->FRAME_NO_LEN);
		if( rockAnimInfo->alt_next == 0)
			rockAnimInfo->alt_next = rockAnimInfo->next_frame;

#ifdef DEBUG
		char rockName[rockAnimRec->ROCKID_LEN+1];
		m.rtrim_fld( rockName, rockAnimRec->rock_id, rockAnimRec->ROCKID_LEN );
		// temporary set init_flag =1;
		init_flag = 1;
		short rockId = locate(rockName);
		init_flag = 0;
		err_when(!rockId);
		
		RockInfo *rockInfo = rock_info_array+rockId-1;

		// -------- validate rockAnimFrame -------//
		err_when( rockAnimInfo->frame <= 0 || rockAnimInfo->frame > rockInfo->max_frame);

		// --------- validate next_frame ---------//
		err_when( rockAnimInfo->next_frame <= 0 || rockAnimInfo->next_frame > rockInfo->max_frame);

		// --------- valudate alt_next -----------//
		err_when( rockAnimInfo->alt_next <= 0 || rockAnimInfo->alt_next > rockInfo->max_frame);
#endif
	}
}
// ------------ end of function RockRes::load_anim_info -----------//


// ------------ begin of function RockRes::get_rock_info -----------//
RockInfo *RockRes::get_rock_info(short rockRecno)
{
	err_when(rockRecno <= 0 || rockRecno > rock_info_count );
	return rock_info_array + rockRecno - 1;
}
// ------------ end of function RockRes::get_rock_info -----------//


// ------------ begin of function RockRes::get_block_info -----------//
RockBlockInfo *RockRes::get_block_info(short rockBlockRecno)
{
	err_when(rockBlockRecno <= 0 || rockBlockRecno > rock_block_count);
	return rock_block_array + rockBlockRecno -1;
}
// ------------ end of function RockRes::get_block_info -----------//


// ------------ begin of function RockRes::get_bitmap_info -----------//
RockBitmapInfo *RockRes::get_bitmap_info(short rockBitmapRecno)
{
	err_when( rockBitmapRecno <= 0 || rockBitmapRecno > rock_bitmap_count);
	return rock_bitmap_array + rockBitmapRecno - 1;
}
// ------------ end of function RockRes::get_bitmap_info -----------//


// ------------ begin of function RockRes::get_anim_info -----------//
RockAnimInfo *RockRes::get_anim_info(short rockAnimRecno )
{
	static RockAnimInfo unanimatedInfo = { 1, 99, 1, 1, };
	if( rockAnimRecno == -1 )			// non-animated rock
	{
		return &unanimatedInfo;
	}
	err_when( rockAnimRecno <= 0 || rockAnimRecno > rock_anim_count );
	return rock_anim_array + rockAnimRecno - 1;
}
// ------------ end of function RockRes::get_anim_info -----------//


// ------------ begin of function RockRes::get_bitmap_recno -----------//
// return rockBitmapRecno
short RockRes::get_bitmap_recno(short rockBlockRecno, char curFrame)
{
	RockBlockInfo *rockBlockInfo = get_block_info(rockBlockRecno);

#ifdef DEBUG
	// --------- validate curFrame -------------//
	RockInfo *rockInfo = get_rock_info(rockBlockInfo->rock_recno);
	err_when( curFrame <= 0 || curFrame > rockInfo->max_frame);
#endif

	short rockBitmapRecno = rockBlockInfo->first_bitmap+curFrame-1;
#ifdef DEBUG
	// --------- validate loc_x and loc_y ----------//
	RockBitmapInfo *rockBitmapInfo = get_bitmap_info(rockBitmapRecno);
	err_when( rockBlockInfo->loc_x != rockBitmapInfo->loc_x);
	err_when( rockBlockInfo->loc_y != rockBitmapInfo->loc_y);
#endif

	return rockBitmapRecno;
}
// ------------ end of function RockRes::get_bitmap_recno -----------//


// ------------ begin of function RockRes::choose_next -----------//
// choose the next frame
// <short> rockRecno     rock recno
// <char> curFrame       the current frame no.
// <long> path           a random number, related to the probability of choosing alt_next
//                       eg. choose_next(..,.., m.random(x)); prob of using alt_next is 1/x
// return next frame no.
char RockRes::choose_next(short rockRecno, char curFrame, long path)
{
	// -------- validate rockRecno ---------//
	RockInfo *rockInfo = get_rock_info(rockRecno);

	// -------- validate curFrame ----------//
	err_when(curFrame <= 0 || curFrame > rockInfo->max_frame);
	RockAnimInfo *rockAnimInfo = get_anim_info(get_anim_recno(rockRecno, curFrame) );

	// -------- validate frame, next_frame and alt_next in rockAnimInfo -------/
	err_when(rockAnimInfo->frame != curFrame);
	err_when(get_anim_info(get_anim_recno(rockRecno, rockAnimInfo->next_frame))->frame != rockAnimInfo->next_frame);
	err_when(get_anim_info(get_anim_recno(rockRecno, rockAnimInfo->alt_next))->frame != rockAnimInfo->alt_next);

	return rockAnimInfo->choose_next(path);
}
// ------------ end of function RockRes::choose_next -----------//


// ------------ begin of function RockRes::draw -----------//
// draw the whole rock at location (xLoc, yLoc)
// <short> rockRecno			rock no.
// <short> xLoc,yLoc       where the rock is drawn
// <char> curFrame         frame no.
void RockRes::draw(short rockRecno, short xLoc, short yLoc, char curFrame)
{
	int yc, xc;
	RockInfo *rockInfo = rock_res.get_rock_info(rockRecno);
	short rockBlockRecno, rockBitmapRecno;

	for( yc = 0; yc < rockInfo->loc_height; ++yc)
	{
		for( xc = 0; xc < rockInfo->loc_width; ++xc)
		{
			if( (rockBlockRecno = locate_block(rockRecno, xc, yc)) != 0 
				&& (rockBitmapRecno = get_bitmap_recno(rockBlockRecno, curFrame)) != 0 )
			{
				get_bitmap_info(rockBitmapRecno)->draw(xLoc+xc, yLoc+yc);
			}
		}
	}
}
// ------------ end of function RockRes::draw -----------//


// ------------ begin of function RockRes::draw_block -----------//
// draw (offsetX, offsetY) of rockRecno at location (xLoc, yLoc)
// <short> rockRecno			  rock no.
// <short> offsetX, offsetY  which cell of a rock
// <short> xLoc,yLoc         where the rock is drawn
// <char> curFrame           frame no.
void RockRes::draw_block(short rockRecno, short xLoc, short yLoc, 
	short offsetX, short offsetY, char curFrame)
{
	RockInfo *rockInfo = rock_res.get_rock_info(rockRecno);
	short rockBlockRecno, rockBitmapRecno;
	err_when( !rockInfo );
	err_when( offsetX < 0 || offsetX >= rockInfo->loc_width );
	err_when( offsetY < 0 || offsetY >= rockInfo->loc_height );

	if( (rockBlockRecno = locate_block(rockRecno, offsetX, offsetY)) != 0 
		&& (rockBitmapRecno = get_bitmap_recno(rockBlockRecno, curFrame)) != 0 )
	{
		get_bitmap_info(rockBitmapRecno)->draw(xLoc, yLoc);
	}
}
// ------------ end of function RockRes::draw_block -----------//


// ------------ begin of function RockRes::search -----------//
//
// search which rock id has the specified criteria
//
// <char*> rockTypes   : a string of 'R', 'D', 'E', or NULL for any
// <short> minWidth    : minimum loc_width of RockInfo
// <short> maxWidth    : maximum loc_width of RockInfo
// <short> minHeight   : minimum loc_height of RockInfo
// <short> maxHeight   : maximum loc_height of RockInfo
// <int> animatedFlag  : 0 for non-animated (i.e. max_frame==1),
//                       +ve for animated( max_frame > 1),
//                       -ve for animated or non-animated    (default : -1)
// <int> findFirst     : whether to find the first match     (default : 0)
//
// return rock recno or 0 (for not found)
//
short RockRes::search(const char *rockTypes, short minWidth, short maxWidth, short minHeight, short maxHeight,
	int animatedFlag, int findFirst, char terrainType )
{
	// -------- search a rock by rock_type, width and height ---------//
	short rockRecno = 0;
	int	findCount = 0;
	RockInfo *rockInfo;
	int	i;

	for( i = 1, rockInfo = rock_info_array; i <= rock_info_count; ++i, ++rockInfo)
	{
		if( (!rockTypes || strchr(rockTypes,rockInfo->rock_type) )
			&& (terrainType == 0 || rockInfo->valid_terrain(terrainType) )
			&& rockInfo->loc_width >= minWidth && rockInfo->loc_width <= maxWidth
			&& rockInfo->loc_height >= minHeight && rockInfo->loc_height <= maxHeight
			&& (animatedFlag < 0 || animatedFlag == 0 && rockInfo->max_frame == 1 ||
				animatedFlag > 0 && rockInfo->max_frame > 1) )
		{
			++findCount;
			if( findFirst )
			{
				rockRecno = i;
				break;
			}
			else if( m.random(findCount) == 0)
			{
				rockRecno = i;
			}
		}
	}

	return rockRecno;
}
// ------------ end of function RockRes::search -----------//


// ------------ begin of function RockRes::locate -----------//
// find a rock by name
//
// <char *> rockName     : name of rock to find
//
// return rock recno or 0 (for not found)
//
short RockRes::locate(char *rockName)
{
	// -------- search a rock by rock_type, width and height ---------//
	short rockRecno = 0;
	RockInfo *rockInfo;
	int	i;

	for( i = 1, rockInfo = rock_info_array; i <= rock_info_count; ++i, ++rockInfo)
	{
		if( strcmp(rockName, rockInfo->rock_name) == 0 )
		{
			rockRecno = i;
			break;
		}
	}

	return rockRecno;
}
// ------------ end of function RockRes::locate -----------//


// ------------ begin of function RockRes::locate_block -----------//
// find a rock block by rock recno and x, y offset
//
// return rock block recno or 0 for not found
short RockRes::locate_block(short rockRecno, short xLoc, short yLoc)
{
	if( xLoc < MAX_ROCK_WIDTH && yLoc < MAX_ROCK_HEIGHT)
	{
		// if xLoc and yLoc is small enough, the block of offset x and offset y
		// can be found in block_offset,
		return get_rock_info(rockRecno)->block_offset[yLoc][xLoc];
	}
	else
	{
		// otherwise, linear search
		short rockBlockRecno = get_rock_info(rockRecno)->first_block_recno;
		RockBlockInfo *rockBlockInfo;
		for( rockBlockInfo = get_block_info(rockBlockRecno); rockBlockRecno <= rock_block_count 
			&& rockBlockInfo->rock_recno == rockRecno; ++rockBlockRecno, ++rockBlockInfo)
		{
			if( rockBlockInfo->loc_x == xLoc && rockBlockInfo->loc_y == yLoc)
				return rockBlockRecno;
		}
		return 0;
	}
}
// ------------ end of function RockRes::locate_block -----------//


// ------------ begin of function RockRes::get_anim_recno -----------//
// get the rockAnimRecno for a frame of a rock
// <short> rockRecno     rock recno
// <char> curFrame       the current frame no.
//
// return rockAnim recno
//
short RockRes::get_anim_recno(short rockRecno, char curFrame)
{
	// -------- validate rockRecno ---------//
	RockInfo *rockInfo = get_rock_info(rockRecno);

	if( rockInfo->first_anim_recno )
	{
#ifdef DEBUG
	// -------- validate curFrame ----------//
	err_when(curFrame <= 0 || curFrame > rockInfo->max_frame);
	RockAnimInfo *rockAnimInfo = get_anim_info(rockInfo->first_anim_recno + (curFrame-1) );

	// -------- validate frame, next_frame and alt_next in rockAnimInfo -------/
	err_when(rockAnimInfo->frame != curFrame);
	err_when(get_anim_info(rockInfo->first_anim_recno+rockAnimInfo->next_frame-1)->frame != rockAnimInfo->next_frame);
	err_when(get_anim_info(rockInfo->first_anim_recno+rockAnimInfo->alt_next-1)->frame != rockAnimInfo->alt_next);
#endif
 
		return rockInfo->first_anim_recno + (curFrame-1);
	}
	else
		return -1;		// a special recno for non-animated rock
}
// ------------ end of function RockRes::get_anim_recno -----------//


// ------------ begin of function RockBitmapInfo::draw ---------//
void RockBitmapInfo::draw(short xLoc, short yLoc)
{
	//-------- check if the firm is within the view area --------//

	int x1 = xLoc * ZOOM_LOC_WIDTH - World::view_top_x;
	int x2 = x1 + width() -1;
	if( x1 >= ZOOM_WIDTH || x2 < 0)
		return;

	int y1 = yLoc * ZOOM_LOC_HEIGHT - World::view_top_y;
	int y2 = y1 + height() -1;
	if( y1 >= ZOOM_HEIGHT || y2 < 0)
		return;

	//---- only portion of the sprite is inside the view area ------//

	if( x1 < 0 || x2 >= ZOOM_WIDTH || y1 < 0 || y2 >= ZOOM_HEIGHT )
	{
		int srcX1 = x1<0 ? -x1 : 0;
		int srcY1 = y1<0 ? -y1 : 0;
		int srcX2 = (x2>=ZOOM_WIDTH ? ZOOM_WIDTH-1-x1 : width()-1);
		int srcY2 = (y2>=ZOOM_HEIGHT ? ZOOM_HEIGHT-1-y1 : height()-1);

		// ########## begin Gilbert 7/4 ###########//
		vga_back.put_bitmap_area_trans( x1+ZOOM_X1, y1+ZOOM_Y1,
			bitmap_ptr, srcX1, srcY1, srcX2, srcY2 );
		// ########## end Gilbert 7/4 ###########//
	}

	//---- the whole sprite is inside the view area ------//

	else
	{
		// ########## begin Gilbert 7/4 ###########//
		vga_back.put_bitmap_trans( x1+ZOOM_X1, y1+ZOOM_Y1, bitmap_ptr );
		// ########## end Gilbert 7/4 ###########//
	}
}
// ------------ end of function RockBitmapInfo::draw ---------//
