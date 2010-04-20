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

//Filename    : OUNITDRW.CPP
//Description : Object Unit Drawing routines

#include <OVGA.h>
#include <OMOUSE.h>
#include <OFONT.h>
#include <OINFO.h>
#include <OFIRMRES.h>
#include <OIMGRES.h>
#include <OPOWER.h>
#include <OSYS.h>
#include <OGAME.h>
#include <ONATION.h>
#include <OUNIT.h>
#include <OCONFIG.h>
#include <dbglog.h>

#ifdef NO_DEBUG_UNIT
#undef err_when
#undef err_here
#undef err_if
#undef err_else
#undef err_now
#define err_when(cond)
#define err_here()
#define err_if(cond)
#define err_else
#define err_now(msg)
#undef DEBUG
#endif

DBGLOG_DEFAULT_CHANNEL(Graphics);

//--------- Begin of function Unit::draw ---------//
//
// update Unit::draw_outlined() also
//
void Unit::draw()
{
	//--------- draw sprite on the zoom window ---------//

	int needMirror;
	SpriteFrame* spriteFrame = cur_sprite_frame(&needMirror);
	update_abs_pos(spriteFrame);

    if (!sprite_info->res_bitmap.initialized())
        ERR("[Unit::draw] trying to read from uninitialized resource\n");

	char* bitmapPtr = sprite_info->res_bitmap.read_imported(spriteFrame->bitmap_offset);

	//-------- check if the sprite is inside the view area --------//

	int x1 = abs_x1-World::view_top_x;  // the sprite's position in the view window

	if( x1 <= -spriteFrame->width || x1 >= ZOOM_WIDTH )   // out of the view area, not even a slight part of it appears in the view area
		return;

	int y1 = abs_y1-World::view_top_y;

	if( y1 <= -spriteFrame->height || y1 >= ZOOM_HEIGHT )
		return;

	//------- decide which approach to use for displaying -----//

	int x2 = abs_x2-World::view_top_x;
	int y2 = abs_y2-World::view_top_y;

	//------- get the color remap table for this sprite ------//

	char* colorRemapTable = game.get_color_remap_table(nation_recno, 0); 		// selected_flag);

	//---- only portion of the sprite is inside the view area ------//

	if( x1 < 0 || x2 >= ZOOM_WIDTH || y1 < 0 || y2 >= ZOOM_HEIGHT )
	{
		if( needMirror )	// if this direction needed to be mirrored
		{
			vga_back.put_bitmap_area_trans_remap_decompress_hmirror( x1+ZOOM_X1, y1+ZOOM_Y1, bitmapPtr,
				MAX(0,x1)-x1, MAX(0,y1)-y1, MIN(ZOOM_WIDTH-1,x2)-x1, MIN(ZOOM_HEIGHT-1,y2)-y1, colorRemapTable );
		}
		else
		{
			vga_back.put_bitmap_area_trans_remap_decompress( x1+ZOOM_X1, y1+ZOOM_Y1, bitmapPtr,
				MAX(0,x1)-x1, MAX(0,y1)-y1, MIN(ZOOM_WIDTH-1,x2)-x1, MIN(ZOOM_HEIGHT-1,y2)-y1, colorRemapTable );
		}
	}

	//---- the whole sprite is inside the view area ------//

	else
	{
		//------ mirror-bilting for certain directions ------//

		if( needMirror )  // if this direction needed to be mirrored
		{
			vga_back.put_bitmap_trans_remap_decompress_hmirror( x1+ZOOM_X1, y1+ZOOM_Y1,
												bitmapPtr, colorRemapTable );
		}
		else
		{
			vga_back.put_bitmap_trans_remap_decompress( x1+ZOOM_X1, y1+ZOOM_Y1,
												bitmapPtr, colorRemapTable );
		}
	}

	//---- display additional info for selected units ----//

	if( selected_flag )
	{
		draw_selected();

		//---- also draw square on units that this unit is attacking ----//

		if( action_mode==ACTION_ATTACK_UNIT )
		{
			if( !unit_array.is_deleted(action_para) )
				unit_array[action_para]->draw_selected();
		}
		else if( action_mode==ACTION_ATTACK_FIRM )
		{
			if( !firm_array.is_deleted(action_para) )
				firm_array[action_para]->draw_selected();
		}
	}

	//--------- if the unit is a spy ---------//

	else if( spy_recno &&
			  (true_nation_recno() == nation_array.player_recno ||
				config.show_ai_info) )
	{
		int dispX1 = ZOOM_X1 + cur_x - World::view_top_x;
		int dispY1 = ZOOM_Y1 + cur_y - World::view_top_y - 20;
		int maxHitBarWidth = ZOOM_LOC_WIDTH * sprite_info->loc_width - 11;

		world.zoom_matrix->put_bitmap_clip(dispX1+maxHitBarWidth+1,
			dispY1-3, image_icon.get_ptr("U_SPY") );
	}

	if( (is_own() || config.show_ai_info || skill.skill_id == SKILL_LEADING ) )		// || rank_id > RANK_SOLDIER) )
	{
		if( !selected_flag && config.show_all_unit_icon )
			draw_skill_icon();
	}
}
//----------- End of function Unit::draw -----------//


//--------- Begin of function Unit::draw_selected ---------//
//
// Draw a square around the sprite if it has been selected.
//
void Unit::draw_selected()
{
	//------- determine the hit bar type -------//

	#define HIT_BAR_TYPE_COUNT  3

	int  hit_bar_color_array[HIT_BAR_TYPE_COUNT] = { 0xA8, 0xB4, 0xAC };
	int  hit_bar_max_array[HIT_BAR_TYPE_COUNT] 	= { 50, 100, 200 };
	char hitBarColor;
	int  hitBarMax;

	for( int i=0 ; i<HIT_BAR_TYPE_COUNT ; i++ )
	{
		if( max_hit_points <= hit_bar_max_array[i] || i==HIT_BAR_TYPE_COUNT-1 )
		{
			hitBarColor = hit_bar_color_array[i];
			hitBarMax   = max_hit_points; 	// MAX( max_hit_points, hit_bar_max_array[i] );
			break;
		}
	}

	//----- draw the hit point bar in a buffer -----//

	enum { HIT_BAR_HEIGHT=3 };

	enum { HIT_BAR_LIGHT_BORDER = 0,
			 HIT_BAR_DARK_BORDER  = 3,
			 HIT_BAR_BODY 		    = 1 };

	enum { NO_BAR_LIGHT_BORDER = 0x40+11,
			 NO_BAR_DARK_BORDER  = 0x40+3,
			 NO_BAR_BODY 		   = 0x40+7 };

	//----- set the display coordination and width -----//

	int maxHitBarWidth, dispX1, dispY1;

	if( mobile_type == UNIT_LAND )
	{
		if( unit_res[unit_id]->unit_class == UNIT_CLASS_HUMAN )
			maxHitBarWidth = ZOOM_LOC_WIDTH - 11;
		else
			maxHitBarWidth = ZOOM_LOC_WIDTH;

		dispX1 = ZOOM_X1 + cur_x - World::view_top_x;
		dispY1 = ZOOM_Y1 + cur_y - World::view_top_y - 20;
	}
	else
	{
		maxHitBarWidth = ZOOM_LOC_WIDTH*2 - 22;

		dispX1 = ZOOM_X1 + cur_x - World::view_top_x - ZOOM_LOC_WIDTH/2 + 11;
		dispY1 = ZOOM_Y1 + cur_y - World::view_top_y - 20;

		if( mobile_type == UNIT_AIR )
			dispY1 -= 20;
	}

	//----------- set other vars -----------//

	char* dataPtr = sys.common_data_buf;
	int   curBarWidth = maxHitBarWidth * max_hit_points / hitBarMax;
	int   pointX = (curBarWidth-1) * (int) hit_points / max_hit_points;	// the separating point between the area with hit point and the area without

	err_when( max_hit_points > hitBarMax );
	err_when( hit_points > max_hit_points );

	*((short*)dataPtr) 	  = curBarWidth;
	*(((short*)dataPtr)+1) = HIT_BAR_HEIGHT;

	dataPtr += sizeof(short) * 2;

	//--------- draw the hit bar ------------//

	// IMGinit( curBarWidth, HIT_BAR_HEIGHT );

	IMGbar( dataPtr, curBarWidth, 0, 0, pointX, 0, hitBarColor+HIT_BAR_LIGHT_BORDER );		// top - with hit

	if( pointX < curBarWidth-1 )
		IMGbar( dataPtr, curBarWidth, pointX+1, 0, curBarWidth-1, 0, NO_BAR_LIGHT_BORDER );		// top - without hit

	IMGbar( dataPtr, curBarWidth, 0, 0, 0, 2, hitBarColor+HIT_BAR_LIGHT_BORDER );					// left - with hit

	IMGbar( dataPtr, curBarWidth, 1, 2, pointX, 2, hitBarColor+HIT_BAR_DARK_BORDER );	// bottom - with hit

	if( pointX < curBarWidth-1 )
		IMGbar( dataPtr, curBarWidth, pointX+1, 2, curBarWidth-1, 2, NO_BAR_DARK_BORDER );		// bottom - without hit

	if( pointX == curBarWidth - 1 )
		IMGbar( dataPtr, curBarWidth, curBarWidth-1, 1, curBarWidth-1, 1, hitBarColor+HIT_BAR_DARK_BORDER );	// right -with hit
	else
		IMGbar( dataPtr, curBarWidth, curBarWidth-1, 1, curBarWidth-1, 1, NO_BAR_DARK_BORDER );				// right -without hit

	IMGbar( dataPtr, curBarWidth, 1, 1, MIN(pointX,curBarWidth-2), 1, hitBarColor+HIT_BAR_BODY );	// bar body

	if( pointX < curBarWidth - 2 )
		IMGbar( dataPtr, curBarWidth, pointX+1, 1, curBarWidth-2, 1, NO_BAR_BODY );	// bar body

	// IMGinit( VGA_WIDTH, VGA_HEIGHT );

	//--------- display the bar now ---------//

	world.zoom_matrix->put_bitmap_clip(dispX1, dispY1, sys.common_data_buf);

	//----- display skill/rank icon (only for own units) -----//

	if( is_own() || config.show_ai_info || skill.skill_id == SKILL_LEADING || rank_id > RANK_SOLDIER )
	{
		draw_skill_icon();
	}
}
//----------- End of function Unit::draw_selected -----------//


//----------- Begin of function Unit::draw_skill_icon ----------//
void Unit::draw_skill_icon()
{
	if( !race_id )		// only for human units
		return;

	int maxHitBarWidth, dispX1, dispY1;

	if( mobile_type == UNIT_LAND )
	{
		if( unit_res[unit_id]->unit_class == UNIT_CLASS_HUMAN )
			maxHitBarWidth = ZOOM_LOC_WIDTH - 11;
		else
			maxHitBarWidth = ZOOM_LOC_WIDTH;

		dispX1 = ZOOM_X1 + cur_x - World::view_top_x;
		dispY1 = ZOOM_Y1 + cur_y - World::view_top_y - 20;
	}
	else
	{
		maxHitBarWidth = ZOOM_LOC_WIDTH*2 - 22;

		dispX1 = ZOOM_X1 + cur_x - World::view_top_x - ZOOM_LOC_WIDTH/2 + 11;
		dispY1 = ZOOM_Y1 + cur_y - World::view_top_y - 20;

		if( mobile_type == UNIT_AIR )
			dispY1 -= 20;
	}

	String str;

	switch( rank_id )
	{
		case RANK_KING:
			str = "U_KING";
			break;

		case RANK_GENERAL:
			str = "U_GENE";
			break;

		case RANK_SOLDIER:
			if( skill.skill_id )
			{
				str  = "U_";
				str += skill.skill_code_array[skill.skill_id-1];
			}
			else
				str = "";
			break;
	}

	int y=dispY1-3;

	if(*str)
	{
		world.zoom_matrix->put_bitmap_clip(dispX1+maxHitBarWidth+1,
			dispY1-3, image_icon.get_ptr(str) );

		y=dispY1+10;
	}

	if( spy_recno &&
		 (true_nation_recno() == nation_array.player_recno ||
		  config.show_ai_info) )
	{
		world.zoom_matrix->put_bitmap_clip(dispX1+maxHitBarWidth+1,
			y, image_icon.get_ptr("U_SPY") );
	}

	//### begin alex 7/10 ###//
	/*( force_move_flag )
	{
		if(is_own() || config.show_ai_info)
			world.zoom_matrix->put_bitmap_clip(dispX1,
					dispY1+5, image_icon.get_ptr("MEDAL1") );
	}*/
	//#### end alex 7/10 ####//
}
//----------- End of function Unit::draw_skill_icon ----------//


//----------- Begin of function Unit::is_visible ----------//
int Unit::is_shealth()
{
	return config.fog_of_war && world.get_loc(next_x_loc(), next_y_loc())->visibility() < unit_res[unit_id]->shealth;
}
//----------- End of function Unit::is_visible ----------//


//--------- Begin of function Unit::draw_outlined ---------//
//
void Unit::draw_outlined()
{
	//--------- draw sprite on the zoom window ---------//

	int needMirror;
	SpriteFrame* spriteFrame = cur_sprite_frame(&needMirror);
	update_abs_pos(spriteFrame);

    if (!sprite_info->res_bitmap.initialized())
        ERR("[Unit::draw_outlined] trying to read from uninitialized resource\n");

	char* bitmapPtr = sprite_info->res_bitmap.read_imported(spriteFrame->bitmap_offset);

	//-------- check if the sprite is inside the view area --------//

	int x1 = abs_x1-World::view_top_x;  // the sprite's position in the view window

	if( x1 <= -spriteFrame->width || x1 >= ZOOM_WIDTH )   // out of the view area, not even a slight part of it appears in the view area
		return;

	int y1 = abs_y1-World::view_top_y;

	if( y1 <= -spriteFrame->height || y1 >= ZOOM_HEIGHT )
		return;

	//------- decide which approach to use for displaying -----//

	int x2 = abs_x2-World::view_top_x;
	int y2 = abs_y2-World::view_top_y;

	//------- get the color remap table for this sprite ------//

	char* colorRemapTable = game.get_color_remap_table(nation_recno, 1);

	//---- only portion of the sprite is inside the view area ------//

	if( x1 < 0 || x2 >= ZOOM_WIDTH || y1 < 0 || y2 >= ZOOM_HEIGHT )
	{
		if( needMirror )	// if this direction needed to be mirrored
		{
			vga_back.put_bitmap_area_trans_remap_decompress_hmirror( x1+ZOOM_X1, y1+ZOOM_Y1, bitmapPtr,
				MAX(0,x1)-x1, MAX(0,y1)-y1, MIN(ZOOM_WIDTH-1,x2)-x1, MIN(ZOOM_HEIGHT-1,y2)-y1, colorRemapTable );
		}
		else
		{
			vga_back.put_bitmap_area_trans_remap_decompress( x1+ZOOM_X1, y1+ZOOM_Y1, bitmapPtr,
				MAX(0,x1)-x1, MAX(0,y1)-y1, MIN(ZOOM_WIDTH-1,x2)-x1, MIN(ZOOM_HEIGHT-1,y2)-y1, colorRemapTable );
		}
	}

	//---- the whole sprite is inside the view area ------//

	else
	{
		//------ mirror-bilting for certain directions ------//

		if( needMirror )  // if this direction needed to be mirrored
		{
			vga_back.put_bitmap_trans_remap_decompress_hmirror( x1+ZOOM_X1, y1+ZOOM_Y1,
												bitmapPtr, colorRemapTable );
		}
		else
		{
			vga_back.put_bitmap_trans_remap_decompress( x1+ZOOM_X1, y1+ZOOM_Y1,
												bitmapPtr, colorRemapTable );
		}
	}

	//---- display additional info for selected units ----//

	if( selected_flag )
	{
		draw_selected();

		//---- also draw square on units that this unit is attacking ----//

		if( action_mode==ACTION_ATTACK_UNIT )
		{
			if( !unit_array.is_deleted(action_para) )
				unit_array[action_para]->draw_selected();
		}
		else if( action_mode==ACTION_ATTACK_FIRM )
		{
			if( !firm_array.is_deleted(action_para) )
				firm_array[action_para]->draw_selected();
		}
	}

	//--------- if the unit is a spy ---------//

	else if( spy_recno && true_nation_recno() == nation_array.player_recno )
	{
		int dispX1 = ZOOM_X1 + cur_x - World::view_top_x;
		int dispY1 = ZOOM_Y1 + cur_y - World::view_top_y - 20;
		int maxHitBarWidth = ZOOM_LOC_WIDTH * sprite_info->loc_width - 11;

		world.zoom_matrix->put_bitmap_clip(dispX1+maxHitBarWidth+1,
			dispY1-3, image_icon.get_ptr("U_SPY") );
	}

	if( (is_own() || config.show_ai_info || skill.skill_id == SKILL_LEADING || rank_id > RANK_SOLDIER) )
	{
		if( !selected_flag )
		{
			draw_skill_icon();
		}
	}
}
//----------- End of function Unit::draw_outlined -----------//
