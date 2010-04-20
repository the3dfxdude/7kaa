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

//Filename    : OWALLRES.CPP
//Description : Wall resource object
//Ownership   : Gilbert

#include <OSYS.h>
#include <OVGA.h>
#include <OFONT.h>
#include <OGAMESET.h>
#include <OWORLD.h>
#include <ONATION.h>
#include <WALLTILE.h>
#include <OWALLRES.h>

//---------- #define constant ------------//

#define WALL_DB   		"WALL"

//------- Begin of function WallRes::WallRes -----------//

WallRes::WallRes()
{
	init_flag=0;

	selected_x_loc = -1;
	selected_y_loc = -1;
}
//--------- End of function WallRes::WallRes -----------//


//---------- Begin of function WallRes::init -----------//
//
// This function must be called after a map is generated.
//
void WallRes::init()
{
	deinit();

	//----- open wall material bitmap resource file -------//

	String str;

	str  = DIR_RES;
	str += "I_WALL.RES";

	res_bitmap.init_imported(str,1);  // 1-read all into buffer

	//------- load database information --------//

	load_wall_info();

	init_flag=1;
}
//---------- End of function WallRes::init -----------//


//---------- Begin of function WallRes::deinit -----------//

void WallRes::deinit()
{
	if( init_flag )
	{
		mem_del(wall_info_array);
		mem_del(wall_index);
		init_flag=0;
	}
}
//---------- End of function WallRes::deinit -----------//


//------- Begin of function WallRes::load_wall_info -------//
//
void WallRes::load_wall_info()
{
	WallRec		*wallRec;
	WallInfo		*wallInfo;
	int			i;
	long			bitmapOffset;
	
	max_wall_id = 0;

	//---- read in wall count and initialize wall info array ----//

	Database *dbWall = game_set.open_db(WALL_DB);	// only one database can be opened at a time, so we read FIRM.DBF first

	wall_count      = (short) dbWall->rec_count();
	wall_info_array = (WallInfo*) mem_add( sizeof(WallInfo)*wall_count );

	memset( wall_info_array, 0, sizeof(WallInfo) * wall_count );

	//---------- read in WALL.DBF ---------//

	for( i=0 ; i<wall_count ; i++ )
	{
		wallRec  = (WallRec*) dbWall->read(i+1);
		wallInfo = wall_info_array+i;

		wallInfo->wall_id = m.atoi( wallRec->wall_id, wallRec->WALL_ID_LEN);
		wallInfo->flags = 0;
		if( wallRec->gate_flag == 'Y' || wallRec->gate_flag == 'y')
			wallInfo->set_gate();
		if( wallRec->trans_flag == 'Y' || wallRec->trans_flag == 'y')
			wallInfo->set_trans();
		wallInfo->offset_x = m.atoi( wallRec->offset_x, wallRec->OFFSET_LEN);
		wallInfo->offset_y = m.atoi( wallRec->offset_y, wallRec->OFFSET_LEN);
		wallInfo->loc_off_x = m.atoi( wallRec->loc_off_x, wallRec->LOC_OFF_LEN);
		wallInfo->loc_off_y = m.atoi( wallRec->loc_off_y, wallRec->LOC_OFF_LEN);

		wallInfo->draw_wall_id = m.atoi( wallRec->draw_wall, wallRec->WALL_ID_LEN);

		memcpy( &bitmapOffset, wallRec->bitmap_ptr, sizeof(long) );
		wallInfo->bitmap_ptr	= res_bitmap.read_imported(bitmapOffset);

		if( wallInfo->wall_id > max_wall_id)
			max_wall_id = wallInfo->wall_id;
	}

	// --------- build wall_index ---------//
	if( max_wall_id > 0)
	{
		wall_index = (WallInfo **) mem_add(sizeof(WallInfo *) * max_wall_id);
		memset( wall_index, 0, sizeof(WallInfo *) * max_wall_id);
		for( i=0 ; i<wall_count ; i++ )
		{
			wall_index[wall_info_array[i].wall_id -1] = &wall_info_array[i];
		}
	}
	else
	{
		err.run("No wall database found");
	}
}
//--------- End of function WallRes::load_wall_info ---------//


//------- Begin of function WallRes::draw_selected -------//
//
void WallRes::draw_selected()
{
	if( selected_x_loc < 0 )
		return;

	int x = selected_x_loc * ZOOM_LOC_WIDTH;
	int y = selected_y_loc * ZOOM_LOC_HEIGHT;

	x = x - World::view_top_x + ZOOM_X1;
	y = y - World::view_top_y + ZOOM_Y1;

	//------------ draw the square frame now ------------//

	if( m.is_touch( x, y, x, x, ZOOM_X1, ZOOM_Y1, ZOOM_X2, ZOOM_Y2 ) )
	{
		vga_back.rect( x, y, x+ZOOM_LOC_WIDTH-1, y+ZOOM_LOC_HEIGHT-1, 1, OWN_SELECT_FRAME_COLOR );
	}
}
//--------- End of function WallRes::draw_selected ---------//


//------- Begin of function WallRes::disp_info -------//
//
void WallRes::disp_info(int refreshFlag)
{
	if( selected_x_loc < 0 )
		return;

	if( refreshFlag == INFO_REPAINT )
	{
		font_san.d3_put( INFO_X1, INFO_Y1, INFO_X2, INFO_Y1+17, "Defense Wall" );
		vga_front.d3_panel_up( INFO_X1, INFO_Y1+20, INFO_X2, INFO_Y1+40, 1 );
	}

	int x=INFO_X1+3, y=INFO_Y1+23;
	Location* locPtr = world.get_loc( selected_x_loc, selected_y_loc );

	String str;

	str  = locPtr->wall_abs_hit_point();
	str += " / 100";

	font_san.field( x, y, "Hit Points",  x+80, str, INFO_X2-4, refreshFlag );
}
//--------- End of function WallRes::disp_info ---------//


//---------- Begin of function WallRes::operator[] -----------//

WallInfo* WallRes::operator[](int wallId)
{
	err_if( wallId<1 || wallId>max_wall_id )
		err_now( "WallRes::operator[]" );

	return wall_index[wallId-1];
}
//------------ End of function WallRes::operator[] -----------//

//------- Begin of function WallInfo::draw -----------//
//
// Draw the current wall on the map
//
void WallInfo::draw(int xLoc, int yLoc, char *remapTbl)
{
	//----------- calculate absolute positions ------------//
	//-------- check if the wall is within the view area --------//

	int x1 = xLoc*ZOOM_LOC_WIDTH + offset_x - World::view_top_x;
	int x2 = x1 + bitmap_width() -1;
	if( x2 < 0 || x1 >= ZOOM_WIDTH )	// out of the view area, not even a slight part of it appears in the view area
		return;

	int y1 = yLoc*ZOOM_LOC_HEIGHT +offset_y - World::view_top_y;
	int y2 = y1 + bitmap_height() -1;
	if( y2 < 0 || y1 >= ZOOM_HEIGHT )
		return;

	//------- decide which approach to use for displaying -----//
	//---- only portion of the sprite is inside the view area ------//

	if( x1 < 0 || x2 >= ZOOM_WIDTH || y1 < 0 || y2 >= ZOOM_HEIGHT )
	{
		if(is_trans())
		{
			if( remapTbl)
				vga_back.put_bitmap_area_trans_remap( x1+ZOOM_X1, y1+ZOOM_Y1, bitmap_ptr,
					MAX(0,x1)-x1, MAX(0,y1)-y1, MIN(ZOOM_WIDTH-1,x2)-x1, MIN(ZOOM_HEIGHT-1,y2)-y1,
					remapTbl);
			else
				vga_back.put_bitmap_area_trans( x1+ZOOM_X1, y1+ZOOM_Y1, bitmap_ptr,
					MAX(0,x1)-x1, MAX(0,y1)-y1, MIN(ZOOM_WIDTH-1,x2)-x1, MIN(ZOOM_HEIGHT-1,y2)-y1);
		}
		else
		{
			if( remapTbl)
				vga_back.put_bitmap_area_remap( x1+ZOOM_X1, y1+ZOOM_Y1, bitmap_ptr,
					MAX(0,x1)-x1, MAX(0,y1)-y1, MIN(ZOOM_WIDTH-1,x2)-x1, MIN(ZOOM_HEIGHT-1,y2)-y1,
					remapTbl);
			else
				vga_back.put_bitmap_area( x1+ZOOM_X1, y1+ZOOM_Y1, bitmap_ptr,
					MAX(0,x1)-x1, MAX(0,y1)-y1, MIN(ZOOM_WIDTH-1,x2)-x1, MIN(ZOOM_HEIGHT-1,y2)-y1);
		}
	}

	//---- the whole sprite is inside the view area ------//
	else
	{
		if(is_trans() )
		{
			if( remapTbl)
				vga_back.put_bitmap_trans_remap( x1+ZOOM_X1, y1+ZOOM_Y1, bitmap_ptr, remapTbl);
			else
				vga_back.put_bitmap_trans( x1+ZOOM_X1, y1+ZOOM_Y1, bitmap_ptr);
		}
		else
		{
			if( remapTbl)
				vga_back.put_bitmap_remap( x1+ZOOM_X1, y1+ZOOM_Y1, bitmap_ptr, remapTbl );
			else
				vga_back.put_bitmap( x1+ZOOM_X1, y1+ZOOM_Y1, bitmap_ptr);
		}
	}
}
//--------- End of function WallInfo::draw -----------//


//------- Begin of function WallInfo::draw_at -----------//
//
// Draw the wall on the zoom map, given the exact pixel position to put
// the bitmap.
//
// <int> absBaseX, absBaseY - the absolute base (center-bottom) coordination
//										of the building.
//
// Draw the current plant on the map
//
void WallInfo::draw_at(int absBaseX, int absBaseY, char *remapTbl)
{
	//-------- check if the wall is within the view area --------//

	int x1 = absBaseX - World::view_top_x;
	int x2 = x1 + bitmap_width() -1;
	if( x2 < 0 || x1 >= ZOOM_WIDTH )	// out of the view area, not even a slight part of it appears in the view area
		return;

	int y1 = absBaseY - World::view_top_y;
	int y2 = y1 + bitmap_height() -1;
	if( y2 < 0 || y1 >= ZOOM_HEIGHT )
		return;

	//------- decide which approach to use for displaying -----//
	//---- only portion of the sprite is inside the view area ------//

	if( x1 < 0 || x2 >= ZOOM_WIDTH || y1 < 0 || y2 >= ZOOM_HEIGHT )
	{
		// no put_bitmap_area_remap
		if(is_trans())
		{
			if( remapTbl)
				vga_back.put_bitmap_area_trans_remap( x1+ZOOM_X1, y1+ZOOM_Y1, bitmap_ptr,
					MAX(0,x1)-x1, MAX(0,y1)-y1, MIN(ZOOM_WIDTH-1,x2)-x1, MIN(ZOOM_HEIGHT-1,y2)-y1,
					remapTbl);
			else
				vga_back.put_bitmap_area_trans( x1+ZOOM_X1, y1+ZOOM_Y1, bitmap_ptr,
					MAX(0,x1)-x1, MAX(0,y1)-y1, MIN(ZOOM_WIDTH-1,x2)-x1, MIN(ZOOM_HEIGHT-1,y2)-y1);
		}
		else
		{
			if( remapTbl)
				vga_back.put_bitmap_area_remap( x1+ZOOM_X1, y1+ZOOM_Y1, bitmap_ptr,
					MAX(0,x1)-x1, MAX(0,y1)-y1, MIN(ZOOM_WIDTH-1,x2)-x1, MIN(ZOOM_HEIGHT-1,y2)-y1,
					remapTbl);
			else
				vga_back.put_bitmap_area( x1+ZOOM_X1, y1+ZOOM_Y1, bitmap_ptr,
					MAX(0,x1)-x1, MAX(0,y1)-y1, MIN(ZOOM_WIDTH-1,x2)-x1, MIN(ZOOM_HEIGHT-1,y2)-y1);

		}
	}

	//---- the whole sprite is inside the view area ------//
	else
	{
		if(is_trans() )
		{
			if( remapTbl)
				vga_back.put_bitmap_trans_remap( x1+ZOOM_X1, y1+ZOOM_Y1, bitmap_ptr, remapTbl);
			else
				vga_back.put_bitmap_trans( x1+ZOOM_X1, y1+ZOOM_Y1, bitmap_ptr);
		}
		else
		{
			if( remapTbl)
				vga_back.put_bitmap_remap( x1+ZOOM_X1, y1+ZOOM_Y1, bitmap_ptr, remapTbl );
			else
				vga_back.put_bitmap( x1+ZOOM_X1, y1+ZOOM_Y1, bitmap_ptr);
		}
	}
}
//--------- End of function WallInfo::draw_at -----------//

