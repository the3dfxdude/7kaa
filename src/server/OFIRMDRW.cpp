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

//Filename    : OFIRMDRW.CPP
//Description : Firm drawing routines

#include <COLCODE.h>
#include <OSYS.h>
#include <OVGA.h>
#include <ONATION.h>
#include <OSPRITE.h>
#include <OINFO.h>
#include <ORAWRES.h>
#include <OPOWER.h>
#include <OGAME.h>
#include <OANLINE.h>
#include <OIMGRES.h>
#include <OWORLD.h>
#include <OF_BASE.h>
#include <OSE.h>
#ifdef USE_DPLAY
#include <OREMOTE.h>
#endif

//------- define static vars -------//

struct Point
{
	short	x;
	short y;
};

static Point slot_point_array[] =
{
	{  1, 86 },
	{  8, 84 },
	{ 15, 86 },
	{  1, 79 },
	{  8, 77 },
	{ 15, 79 },
	{  1, 72 },
	{  8, 70 },
	{ 15, 72 },
};

//------- Begin of function Firm::draw -----------//
//
// Draw the firm on the map
//
// [int] displayLayer   : 1 = same layer with units (default : 1)
//                      : 2 = layer above units
//                      : 4 = layer below units
//
void Firm::draw(int displayLayer)
{
	FirmBuild* firmBuild = firm_res.get_build(firm_build_id);

	// if in construction, don't draw ground unless the last construction frame
	if( firmBuild->ground_bitmap_recno &&
		(!under_construction || construction_frame() >= firmBuild->under_construction_bitmap_count-1))
	{
		firm_res.get_bitmap(firmBuild->ground_bitmap_recno)
			->draw_at(loc_x1*ZOOM_LOC_WIDTH, loc_y1*ZOOM_LOC_HEIGHT, NULL, displayLayer);
	}

	if( firmBuild->animate_full_size )
	{
		draw_full_size(displayLayer);
	}
	else
	{
		if( under_construction )
		{
			draw_full_size(displayLayer);
		}
		else if( !is_operating() )
		{
			FirmBuild* firmBuild = firm_res.get_build(firm_build_id);
			if( firm_res.get_bitmap(firmBuild->idle_bitmap_recno) )
				draw_full_size(displayLayer);
			else
			{
				draw_frame(1, displayLayer);
				draw_frame(2, displayLayer);
			}
		}
		else
		{
			draw_frame(1, displayLayer);				// the first frame is the common frame for multi-segment bitmaps
			draw_frame(cur_frame, displayLayer);
		}
	}
}
//--------- End of function Firm::draw -----------//


//------- Begin of function Firm::draw_full_size -----------//
//
// Draw the firm on the map
//
void Firm::draw_full_size(int displayLayer)
{
	FirmBuild* firmBuild = firm_res.get_build(firm_build_id);

	//-------- check if the firm is within the view area --------//

	int x1 = abs_x1 - World::view_top_x;

	if( x1 <= -firmBuild->max_bitmap_width || x1 >= ZOOM_WIDTH )	// out of the view area, not even a slight part of it appears in the view area
		return;

	int y1 = abs_y1 - World::view_top_y;

	if( y1 <= -firmBuild->max_bitmap_height || y1 >= ZOOM_HEIGHT )
		return;

	//------- get the color remap table for this bitmap ------//

	char* colorRemapTable = game.get_color_remap_table(nation_recno, firm_array.selected_recno == firm_recno);

	// ######## begin Gilbert 29/10 #######//
	// ------ draw flags behind the building -------//
	if( under_construction )
	{
#define FLAG_WIDTH 9
#define FLAG_HEIGHT 25
		char *flagBitmapPtr = image_spict.get_ptr("FLAG-S0");
		int drawX = loc_x1 * ZOOM_LOC_WIDTH - world.view_top_x + ZOOM_X1;
		int drawY = loc_y1 * ZOOM_LOC_HEIGHT - world.view_top_y + ZOOM_Y1;
		world.zoom_matrix->put_bitmap_remap_clip(drawX, drawY, flagBitmapPtr, colorRemapTable, 1);	// 1-the bitmap is compressed

		drawX = (loc_x2+1)*ZOOM_LOC_WIDTH - FLAG_WIDTH - world.view_top_x + ZOOM_X1;
		world.zoom_matrix->put_bitmap_remap_clip(drawX, drawY, flagBitmapPtr, colorRemapTable, 1);	// 1-the bitmap is compressed
	}
	// ######## end Gilbert 29/10 #######//

	//---------- get the bitmap pointer ----------//

	FirmBitmap* firmBitmap = NULL;

	if( under_construction )
	{
		int buildFraction = construction_frame();
		firmBitmap = firm_res.get_bitmap(firmBuild->under_construction_bitmap_recno
			+ buildFraction);
	}

	else if( !is_operating() )                      // is_operating() is a virtual function
		firmBitmap = firm_res.get_bitmap(firmBuild->idle_bitmap_recno);

	else
		firmBitmap = firm_res.get_bitmap(firmBuild->first_bitmap(cur_frame));

	// ------ check if the display layer is correct ---------//
	if( !firmBitmap || !(firmBitmap->display_layer & displayLayer) )
		return;

	//-------- check if the firm is within the view area --------//

	x1 = loc_x1 * ZOOM_LOC_WIDTH - World::view_top_x + firmBitmap->offset_x;

	if( x1 <= -firmBitmap->width || x1 >= ZOOM_WIDTH )	// out of the view area, not even a slight part of it appears in the view area
		return;

	y1 = loc_y1 * ZOOM_LOC_HEIGHT - World::view_top_y + firmBitmap->offset_y;

	if( y1 <= -firmBitmap->height || y1 >= ZOOM_HEIGHT )
		return;

	//------- decide which approach to use for displaying -----//

	int x2 = x1 + firmBitmap->width - 1;
	int y2 = y1 + firmBitmap->height - 1;

	//------- if the firm is under construction ------//

	if( 0 && under_construction )
	{
		err_when( (abs_x2-abs_x1+1) * (abs_y2-abs_y1+1) > COMMON_DATA_BUF_SIZE );

		//---------- decompress the image ---------//

		IMGremapDecompress(sys.common_data_buf, firmBitmap->bitmap_ptr, colorRemapTable);

		//---------- pixelize the image -----------//

		char* pixelPtr = sys.common_data_buf + sizeof(short) * 2;

		int y, bitmapWidth=abs_x2-abs_x1+1, bitmapHeight=abs_y2-abs_y1+1;
		int lineCount;
		int solidLineCount = bitmapHeight / 2 * (int) hit_points / (int) max_hit_points;
		int solidLinePixel = bitmapHeight / 2 * bitmapWidth * (int) hit_points / (int) max_hit_points -
									solidLineCount * bitmapWidth;

		int hitPerPixel = (int) max_hit_points / bitmapWidth / bitmapHeight / 2;

		for( lineCount=1, y=bitmapHeight-2 ; y>=0 ; y-=2, lineCount++ )
		{
			 if( lineCount > solidLineCount )
				 memset( pixelPtr+y*bitmapWidth, TRANSPARENT_CODE, bitmapWidth );

			 if( lineCount==solidLineCount+1 )		// the current progressing line
				 memset( pixelPtr+y*bitmapWidth, TRANSPARENT_CODE, solidLinePixel );
		}

		//---- only portion of the sprite is inside the view area ------//

		if( x1 < 0 || x2 >= ZOOM_WIDTH || y1 < 0 || y2 >= ZOOM_HEIGHT )
		{
			vga_back.put_bitmap_area_trans( x1+ZOOM_X1, y1+ZOOM_Y1, sys.common_data_buf,
				MAX(0,x1)-x1, MAX(0,y1)-y1, MIN(ZOOM_WIDTH-1,x2)-x1, MIN(ZOOM_HEIGHT-1,y2)-y1 );
		}

		//---- the whole sprite is inside the view area ------//

		else
		{
			vga_back.put_bitmap_trans( x1+ZOOM_X1, y1+ZOOM_Y1, sys.common_data_buf );
		}
	}
	else	//----- display the normal image (not under construction) ----//
	{
		//---- only portion of the sprite is inside the view area ------//

		if( x1 < 0 || x2 >= ZOOM_WIDTH || y1 < 0 || y2 >= ZOOM_HEIGHT )
		{
			vga_back.put_bitmap_area_trans_remap_decompress( x1+ZOOM_X1, y1+ZOOM_Y1, firmBitmap->bitmap_ptr,
				MAX(0,x1)-x1, MAX(0,y1)-y1, MIN(ZOOM_WIDTH-1,x2)-x1, MIN(ZOOM_HEIGHT-1,y2)-y1, colorRemapTable );
		}

		//---- the whole sprite is inside the view area ------//

		else
		{
			vga_back.put_bitmap_trans_remap_decompress( x1+ZOOM_X1, y1+ZOOM_Y1, firmBitmap->bitmap_ptr, colorRemapTable );
		}
	}

	// ######## begin Gilbert 29/10 #######//
	// ------ draw flags in front of the building -------//
	if( under_construction )
	{
		char *flagBitmapPtr = image_spict.get_ptr("FLAG-S0");
		int drawX = loc_x1 * ZOOM_LOC_WIDTH - world.view_top_x + ZOOM_X1;
		int drawY = (loc_y2+1) * ZOOM_LOC_HEIGHT - FLAG_HEIGHT - world.view_top_y + ZOOM_Y1;
		world.zoom_matrix->put_bitmap_remap_clip(drawX, drawY, flagBitmapPtr, colorRemapTable, 1);	// 1-the bitmap is compressed

		drawX = (loc_x2+1)*ZOOM_LOC_WIDTH - FLAG_WIDTH - world.view_top_x + ZOOM_X1;
		world.zoom_matrix->put_bitmap_remap_clip(drawX, drawY, flagBitmapPtr, colorRemapTable, 1);	// 1-the bitmap is compressed
	}
	// ######## end Gilbert 29/10 #######//
}
//--------- End of function Firm::draw_full_size -----------//


//------- Begin of function Firm::draw_frame -----------//
//
// Draw a specific frame of the firm.
//
void Firm::draw_frame(int frameId, int displayLayer)
{
	//---------- draw animation now ------------//

	FirmBuild*  firmBuild = firm_res.get_build(firm_build_id);
	FirmBitmap* firmBitmap;

	int bitmapRecno, i;
	int firstBitmap = firmBuild->first_bitmap(frameId);
	int bitmapCount = firmBuild->bitmap_count(frameId);

	char* colorRemapTable = game.get_color_remap_table(nation_recno, firm_array.selected_recno==firm_recno);

	for( i=0, bitmapRecno=firstBitmap ; i<bitmapCount ; i++, bitmapRecno++ )
	{
		firmBitmap = firm_res.get_bitmap(bitmapRecno);

		if( firmBitmap )
			firmBitmap->draw_at(loc_x1*ZOOM_LOC_WIDTH, loc_y1*ZOOM_LOC_HEIGHT, colorRemapTable, displayLayer);
	}
}
//--------- End of function Firm::draw_frame -----------//


//------- Begin of function Firm::draw_detect_link_line ---------//
//
// [int] actionDetect - 0 - this is a draw action
//								1 - this is a detect action
//                      (default: 0)
//
// return: <int> 1 - detected
//					  0 - not detected
//
int Firm::draw_detect_link_line(int actionDetect)
{
	if( firm_id == FIRM_INN ) 	// FirmInn's link is only for scan for neighbor inns quickly, the link line is not displayed
		return 0;

	//--------------------------------------//

	int 	i, firmX, firmY, townX, townY;
	Firm* firmPtr;
	Town* townPtr;
	FirmInfo* firmInfo = firm_res[firm_id];

	//-------- set source points ----------//

	int srcX = ( ZOOM_X1 + (loc_x1-world.zoom_matrix->top_x_loc) * ZOOM_LOC_WIDTH
				  + ZOOM_X1 + (loc_x2-world.zoom_matrix->top_x_loc+1) * ZOOM_LOC_WIDTH ) / 2;

	int srcY = ( ZOOM_Y1 + (loc_y1-world.zoom_matrix->top_y_loc) * ZOOM_LOC_HEIGHT
				  + ZOOM_Y1 + (loc_y2-world.zoom_matrix->top_y_loc+1) * ZOOM_LOC_HEIGHT ) / 2;

	//------ draw lines to linked firms ---------//

	int	lineType;
	char* bitmapPtr;

	for( i=0 ; i<linked_firm_count ; i++ )
	{
		firmPtr = firm_array[linked_firm_array[i]];

		firmX = ( ZOOM_X1 + (firmPtr->loc_x1-world.zoom_matrix->top_x_loc) * ZOOM_LOC_WIDTH
				  + ZOOM_X1 + (firmPtr->loc_x2-world.zoom_matrix->top_x_loc+1) * ZOOM_LOC_WIDTH ) / 2;

		firmY = ( ZOOM_Y1 + (firmPtr->loc_y1-world.zoom_matrix->top_y_loc) * ZOOM_LOC_HEIGHT
				  + ZOOM_Y1 + (firmPtr->loc_y2-world.zoom_matrix->top_y_loc+1) * ZOOM_LOC_HEIGHT ) / 2;

		anim_line.draw_line(&vga_back, srcX, srcY, firmX, firmY, linked_firm_enable_array[i]==LINK_EE );

		//----- check if this firm can toggle link or not -----//

		if( !can_toggle_firm_link(firmPtr->firm_recno) )
			continue;

		//------ if the link is switchable -------//

		bitmapPtr = power.get_link_icon( linked_firm_enable_array[i], nation_recno==firmPtr->nation_recno );

		if( actionDetect )
		{
			if( own_firm() && world.zoom_matrix->detect_bitmap_clip( firmX-11, firmY-11, bitmapPtr ) )
			{
				if( linked_firm_enable_array[i] & LINK_ED )
				{
					toggle_firm_link( i+1, 0, COMMAND_PLAYER );
					se_ctrl.immediate_sound("TURN_OFF");
				}
				else
				{
					toggle_firm_link( i+1, 1, COMMAND_PLAYER );
					se_ctrl.immediate_sound("TURN_ON");
				}
				return 1;
			}
		}
		else
		{
			if( nation_recno == nation_array.player_recno )
				world.zoom_matrix->put_bitmap_clip( firmX-11, firmY-11, bitmapPtr );
		}
	}

	//------ draw lines to linked towns ---------//

	for( i=0 ; i<linked_town_count ; i++ )
	{
		townPtr = town_array[linked_town_array[i]];

		townX = ( ZOOM_X1 + (townPtr->loc_x1-world.zoom_matrix->top_x_loc) * ZOOM_LOC_WIDTH
				  + ZOOM_X1 + (townPtr->loc_x2-world.zoom_matrix->top_x_loc+1) * ZOOM_LOC_WIDTH ) / 2;

		townY = ( ZOOM_Y1 + (townPtr->loc_y1-world.zoom_matrix->top_y_loc) * ZOOM_LOC_HEIGHT
				  + ZOOM_Y1 + (townPtr->loc_y2-world.zoom_matrix->top_y_loc+1) * ZOOM_LOC_HEIGHT ) / 2;

		if( worker_array && selected_worker_id &&
			 worker_array[selected_worker_id-1].town_recno == townPtr->town_recno )
		{
			lineType = -1;
			anim_line.thick_line(&vga_back, srcX, srcY, townX, townY, linked_town_enable_array[i]==LINK_EE, lineType );
		}
		else
		{
			lineType = 0;
			anim_line.draw_line(&vga_back, srcX, srcY, townX, townY, linked_town_enable_array[i]==LINK_EE, lineType );
		}

		//----- check if this firm can toggle link or not -----//

		if( !can_toggle_town_link() )
			continue;

		//--------- draw link symbol -----------//

		bitmapPtr = power.get_link_icon( linked_town_enable_array[i], nation_recno==townPtr->nation_recno );

		if( actionDetect )
		{
			int rc = world.zoom_matrix->detect_bitmap_clip( townX-11, townY-11, bitmapPtr );

			//------ left clicking to toggle link -------//

			if( rc==1 && own_firm() )
			{
				if( linked_town_enable_array[i] & LINK_ED )
				{
					toggle_town_link( i+1, 0, COMMAND_PLAYER );
					se_ctrl.immediate_sound("TURN_OFF");
				}
				else
				{
					toggle_town_link( i+1, 1, COMMAND_PLAYER );
					se_ctrl.immediate_sound("TURN_ON");
				}

				//
				// update RemoteMsg::firm_toggle_link_town()
				//
#ifdef USE_DPLAY
				if( firm_id == FIRM_CAMP && !remote.is_enable())
#else
				if( firm_id == FIRM_CAMP )
#endif
				{
					if( townPtr->nation_recno )
						townPtr->update_target_loyalty();
					else
						townPtr->update_target_resistance();

					townPtr->update_camp_link();
				}

				return 1;
			}

			//------ right clicking to move workers ------//

			else if( rc==2 && selected_worker_id > 0 )
			{
				//--- only when this worker is ours ----//

				if( firm_res[firm_id]->live_in_town &&
					 worker_array[selected_worker_id-1].is_nation(firm_recno, nation_array.player_recno) )
				{
					if(townPtr->population>=MAX_TOWN_POPULATION)
						return 0;

					set_worker_home_town(townPtr->town_recno, COMMAND_PLAYER);
					se_ctrl.immediate_sound("PULL_MAN");
					return 1;
				}
			}
		}
		else
		{
			if( nation_recno == nation_array.player_recno )
				world.zoom_matrix->put_bitmap_clip( townX-11, townY-11, bitmapPtr );
		}
	}

	return 0;
}
//------- End of function Firm::draw_detect_link_line ---------//


//------- Begin of function Firm::is_in_zoom_win -----------//
//
// Whether the firm is in the current zoom window.
//
int Firm::is_in_zoom_win()
{
	FirmBuild* firmBuild = firm_res.get_build(firm_build_id);

	//-------- check if the firm is within the view area --------//

	int x1 = abs_x1 - World::view_top_x;

	if( x1 <= -firmBuild->max_bitmap_width || x1 >= ZOOM_WIDTH )	// out of the view area, not even a slight part of it appears in the view area
		return 0;

	int y1 = abs_y1 - World::view_top_y;

	if( y1 <= -firmBuild->max_bitmap_height || y1 >= ZOOM_HEIGHT )
		return 0;

	return 1;
}
//--------- End of function Firm::is_in_zoom_win -----------//



//------- Begin of function Firm::draw_selected -----------//
//
// Draw a square around the firm on the map.
//
void Firm::draw_selected()
{
/*
	//------ calculate frame coordinations ---------//

	FirmBuild* firmBuild = firm_res.get_build(firm_build_id);

	int x1 = loc_x1 * ZOOM_LOC_WIDTH;
	int y1 = loc_y1 * ZOOM_LOC_HEIGHT;
	int x2 = (loc_x1+firmBuild->loc_width)  * ZOOM_LOC_WIDTH  - 1;
	int y2 = (loc_y1+firmBuild->loc_height) * ZOOM_LOC_HEIGHT - 1;

	x1 = x1 - World::view_top_x + ZOOM_X1;
	y1 = y1 - World::view_top_y + ZOOM_Y1;
	x2 = x2 - World::view_top_x + ZOOM_X1;
	y2 = y2 - World::view_top_y + ZOOM_Y1;

	//------------- set frame color -------------//

	char frameColor;

	if( nation_recno == nation_array.player_recno )
		frameColor = OWN_SELECT_FRAME_COLOR;
	else
		frameColor = ENEMY_SELECT_FRAME_COLOR;

	//------------ draw the square frame now ------------//

	if( m.is_touch( x1, y1, x2, y2, ZOOM_X1, ZOOM_Y1, ZOOM_X2, ZOOM_Y2 ) )
	{
		//------- Only draw_selected the portion within the zoom window area ----//

		if( y1 >= ZOOM_Y1 )		// square top
			vga_back.bar( MAX(x1,ZOOM_X1), y1, MIN(x2,ZOOM_X2), y1, frameColor );

		if( y2 <= ZOOM_Y2 )		// square bottom
			vga_back.bar( MAX(x1,ZOOM_X1), y2, MIN(x2,ZOOM_X2), y2, frameColor );

		if( x1 >= ZOOM_X1 )		// square left
			vga_back.bar( x1, MAX(y1,ZOOM_Y1), x1, MIN(y2,ZOOM_Y2), frameColor );

		if( y1 <= ZOOM_X2 )		// square left
			vga_back.bar( x2, MAX(y1,ZOOM_Y1), x2, MIN(y2,ZOOM_Y2), frameColor );

      //------------ display hit point bar -----------//

		if( firm_res[firm_id]->buildable )
		{
			x1 = x1+1;
			y1 = MAX( y2-4, ZOOM_Y1 );
			x2 = x2-1;
			y2 = MIN( y2-1, ZOOM_Y2 );

			if( x1<=ZOOM_X2 && x2>=ZOOM_X1 && y1<=ZOOM_Y2 && y2>=ZOOM_Y1 )
			{
				int barWidth = (x2-x1+1) * hit_points / max_hit_points;

				if( hit_points > 0 && x1+barWidth-1 >= ZOOM_X1)
				{
					vga_back.bar( MAX(x1,ZOOM_X1), y1  , MIN(x1+barWidth-1,ZOOM_X2), y1, frameColor );
					vga_back.bar( MAX(x1,ZOOM_X1), y1+1, MIN(x1+barWidth-1,ZOOM_X2), y2, V_GREEN );
				}
			}
		}
	}
*/
	//------- draw lines connected to town ---------//

	draw_detect_link_line(0);
}
//--------- End of function Firm::draw_selected -----------//


//------- Begin of function Firm::draw_cargo -----------//
//
// Draw cargos of stock.
//
// <int>   cargoCount 	  - the no. of cargos to be drawn
// <char*> cargoBitmapPtr - bitmap ptr to the cargo
//
void Firm::draw_cargo(int cargoCount, char* cargoBitmapPtr)
{
	//-------- check if the firm is within the view area --------//

	if( loc_x1 < world.zoom_matrix->top_x_loc || loc_x2 >= world.zoom_matrix->top_x_loc+ZOOM_WIDTH )
		return;

	if( loc_y1 < world.zoom_matrix->top_y_loc || loc_y2 >= world.zoom_matrix->top_y_loc+ZOOM_HEIGHT )
		return;

	//------ display a pile of raw materials ------//

	int x = ZOOM_X1 + (loc_x1-world.zoom_matrix->top_x_loc) * ZOOM_LOC_WIDTH;
	int y = ZOOM_Y1 + (loc_y1-world.zoom_matrix->top_y_loc) * ZOOM_LOC_HEIGHT;

	for( int i=0 ; i<cargoCount ; i++ )
	{
		world.zoom_matrix->put_bitmap_clip( x+slot_point_array[i].x,
			y+slot_point_array[i].y, cargoBitmapPtr);
	}
}
//--------- End of function Firm::draw_cargo -----------//


//------- Begin of function Firm::can_toggle_town_link ---------//
//
int Firm::can_toggle_town_link()
{
	return firm_id != FIRM_MARKET;		// only a market cannot toggle its link as it is
}
//------- Begin of function Firm::can_toggle_town_link ---------//


//------- Begin of function Firm::can_toggle_firm_link ---------//
//
int Firm::can_toggle_firm_link(int firmRecno)
{
	Firm* firmPtr = firm_array[firmRecno];

	//--- market to harbor link is determined by trade treaty ---//

	if( ( firm_id == FIRM_MARKET && firmPtr->firm_id == FIRM_HARBOR ) ||
		 ( firm_id == FIRM_HARBOR && firmPtr->firm_id == FIRM_MARKET ) )
	{
		return 0;
	}

	return firm_res[firm_id]->is_linkable_to_firm(firmPtr->firm_id);
}
//------- End of function Firm::can_toggle_firm_link ---------//


//------- Begin of function Firm::construction_frame ---------//
//
// return 0 to hitfirmBuild->under_construction_bitmap_count-1 
// according to hit_points and max_hitpoints
//
int Firm::construction_frame()
{
	err_when(!under_construction);
	FirmBuild* firmBuild = firm_res.get_build(firm_build_id);
	int r = int( hit_points * firmBuild->under_construction_bitmap_count / max_hit_points); 
	err_when( r < 0);
	if( r >= firmBuild->under_construction_bitmap_count )
		r = firmBuild->under_construction_bitmap_count-1;
	return r;
}
//------- End of function Firm::construction_frame ---------//
