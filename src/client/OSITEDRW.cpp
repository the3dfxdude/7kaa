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

//Filename    : OSITE2.CPP
//Description : Object Site Material - display info functions

#include <OVGA.h>
#include <vga_util.h>
#include <OINFO.h>
#include <OFONT.h>
#include <OWORLD.h>
#include <OIMGRES.h>
#include <ORAWRES.h>
#include <OGODRES.h>
#include <ORACERES.h>
#include <OSITE.h>

//--------- Begin of function Site::disp_info ---------//
//
void Site::disp_info(int refreshFlag)
{
	if( refreshFlag != INFO_REPAINT )
		return;

   //------- natural resource site ------//

	if( site_type == SITE_RAW )
	{
		font_san.d3_put( INFO_X1, INFO_Y1, INFO_X2, INFO_Y1+17, "Natural Resource" );
		vga_util.d3_panel_up( INFO_X1, INFO_Y1+20, INFO_X2, INFO_Y1+59, 1 );

		int x=INFO_X1+4, y=INFO_Y1+24;

		font_san.put_field( x, y	, "Resource", x+70, raw_res[object_id]->name );
		font_san.put_field( x, y+16, "Reserve" , x+70, reserve_qty, 1 );
	}

	//--------- scroll of power --------//

	else if( site_type == SITE_SCROLL )
	{
		font_san.d3_put( INFO_X1, INFO_Y1, INFO_X2, INFO_Y1+17, "Scroll of Power" );
		vga_util.d3_panel_up( INFO_X1, INFO_Y1+20, INFO_X2, INFO_Y1+59, 1 );

		int x=INFO_X1+4, y=INFO_Y1+24;

		GodInfo* godInfo = god_res[object_id];

		font_san.put_field( x, y	, "Nationalty", x+82, race_res[godInfo->race_id]->name );
		font_san.put_field( x, y+16, "Invoke"    , x+82, unit_res[godInfo->unit_id]->name );
	}

	//----------- gold coins -----------//

	else if( site_type == SITE_GOLD_COIN )
	{
		font_san.d3_put( INFO_X1, INFO_Y1, INFO_X2, INFO_Y1+17, "Treasure" );
		vga_util.d3_panel_up( INFO_X1, INFO_Y1+20, INFO_X2, INFO_Y1+43, 1 );

		int x=INFO_X1+4, y=INFO_Y1+24;

		font_san.put_field( x, y, "Worth", x+60, object_id, 2 );
	}
}
//----------- End of function Site::disp_info -----------//


//--------- Begin of function Site::detect_info ---------//
//
void Site::detect_info()
{
}
//----------- End of function Site::detect_info -----------//


//--------- Begin of function Site::draw ---------//
//
void Site::draw(int x, int y)
{
	char* bmpPtr;

	switch( site_type )
	{
		case SITE_RAW:
			bmpPtr = raw_res.large_raw_icon(object_id);
			break;

		case SITE_SCROLL:
		{
			char iconName[]="SCROLL-0";
			iconName[7] = race_res[object_id]->code[0];
			bmpPtr = image_spict.get_ptr(iconName);
			break;
		}

		case SITE_GOLD_COIN:
		{
			// ##### begin Gilbert 30/8 ######//
			#define MAX_COINS_TYPE  8
			// ##### end Gilbert 30/8 ######//

			char iconName[]="COINS-0";
			iconName[6] = '1' + object_id % MAX_COINS_TYPE;
			bmpPtr = image_spict.get_ptr(iconName);
			break;
		}
	}

	vga_back.put_bitmap_trans( x, y, bmpPtr );
}
//----------- End of function Site::draw -----------//


//------- Begin of function Site::draw_selected -----------//
//
// Draw a square around the raw material site on the map.
//
void Site::draw_selected()
{
	Location* locPtr = world.get_loc(map_x_loc, map_y_loc);

	if( locPtr->is_firm() || locPtr->is_town() )	// do not draw the selection frame if there is a firm or town built on the top of the site
		return;

	//------ calculate frame coordinations ---------//

	int x1 = map_x_loc * ZOOM_LOC_WIDTH;
	int y1 = map_y_loc * ZOOM_LOC_HEIGHT;
	int x2 = x1 + ZOOM_LOC_WIDTH  - 1;
	int y2 = y1 + ZOOM_LOC_HEIGHT - 1;

	x1 = x1 - World::view_top_x + ZOOM_X1;
	y1 = y1 - World::view_top_y + ZOOM_Y1;
	x2 = x2 - World::view_top_x + ZOOM_X1;
	y2 = y2 - World::view_top_y + ZOOM_Y1;

	//------------ draw the square frame now ------------//

	if( x1>=ZOOM_X1 && y1>=ZOOM_Y1 && x2<=ZOOM_X2 && y2<=ZOOM_Y2 )
	{
		vga_back.rect( x1, y1, x2, y2, 1, OWN_SELECT_FRAME_COLOR );
	}
}
//--------- End of function Site::draw_selected -----------//


//--------- Begin of function SiteArray::draw_dot ---------//
//
// Draw 2x2 tiny squares on map window representing the
// location of raw materials sites.
//
void SiteArray::draw_dot()
{
	char*	  vgaBufPtr = vga_back.buf_ptr();
	char*	  writePtr;
	int	  i, mapX, mapY;
	Site*	  rawPtr;

	// ###### begin Gilbert 7/7 #######//
	int		vgaBufPitch = vga_back.buf_pitch();
	// ###### end Gilbert 7/7 #######//

	for(i=1; i <=size() ; i++)
	{
		if( is_deleted(i) )
			continue;

		rawPtr = operator[](i);

		mapX = MAP_X1 + rawPtr->map_x_loc;
		mapY = MAP_Y1 + rawPtr->map_y_loc;

		if( mapX == MAP_WIDTH-1 )
			mapX = MAP_WIDTH-2;

		if( mapY == MAP_HEIGHT-1 )
			mapY = MAP_HEIGHT-2;

		// ###### begin Gilbert 2/9 #######//
		writePtr = vgaBufPtr + mapY*vgaBufPitch + mapX;

		if( writePtr[-vgaBufPitch-1] != UNEXPLORED_COLOR )
			writePtr[-vgaBufPitch-1] = (char) SITE_COLOR;

		if( writePtr[-vgaBufPitch] != UNEXPLORED_COLOR )
			writePtr[-vgaBufPitch] = (char) SITE_COLOR;

		if( writePtr[-vgaBufPitch+1] != UNEXPLORED_COLOR )
			writePtr[-vgaBufPitch+1] = (char) SITE_COLOR;

		if( writePtr[-1] != UNEXPLORED_COLOR )
			writePtr[-1] = (char) SITE_COLOR;

		if( writePtr[0] != UNEXPLORED_COLOR )
			writePtr[0] = (char) SITE_COLOR;

		if( writePtr[1] != UNEXPLORED_COLOR )
			writePtr[1] = (char) SITE_COLOR;

		if( writePtr[vgaBufPitch-1] != UNEXPLORED_COLOR )
			writePtr[vgaBufPitch-1] = (char) SITE_COLOR;

		if( writePtr[vgaBufPitch] != UNEXPLORED_COLOR )
			writePtr[vgaBufPitch] = (char) SITE_COLOR;

		if( writePtr[vgaBufPitch+1] != UNEXPLORED_COLOR )
			writePtr[vgaBufPitch+1] = (char) SITE_COLOR;

		// ###### end Gilbert 2/9 #######//
	}
}
//----------- End of function SiteArray::draw_dot -----------//

