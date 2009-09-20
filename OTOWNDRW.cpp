//Filename    : OTOWNDRW.CPP
//Description : Town drawing routines

#include <OVGA.h>
#include <OSYS.h>
#include <OFONT.h>
#include <OMOUSE.h>
#include <OBUTTON.h>
#include <OINFO.h>
#include <OIMGRES.h>
#include <OPOWER.h>
#include <OWORLD.h>
#include <OGAME.h>
#include <OREMOTE.h>
#include <OANLINE.h>
#include <OPLANT.h>
#include <ONATION.h>
#include <OFIRM.h>
#include <OTOWN.h>
#include <OSE.h>


//------- Begin of function Town::draw -----------//
//
// Draw the town section on the zoom map
//
// [int] displayLayer   : 1 = normal layer          (default : 1)
//                      : 2 = layer above the town
//                      : 4 = layer below the town
//
void Town::draw(int displayLayer)
{
	TownLayout* townLayout = town_res.get_layout(layout_id);
	TownSlot*   townSlot   = town_res.get_slot(townLayout->first_slot_recno);

	int sectionX1 = loc_x1 * ZOOM_LOC_WIDTH;
	int sectionY1 = loc_y1 * ZOOM_LOC_HEIGHT;

	if( displayLayer==4 )
	{
		int townX1 = ZOOM_X1 + (loc_x1-world.zoom_matrix->top_x_loc) * ZOOM_LOC_WIDTH;
		int townY1 = ZOOM_Y1 + (loc_y1-world.zoom_matrix->top_y_loc) * ZOOM_LOC_HEIGHT;

		townX1 += (STD_TOWN_LOC_WIDTH  * ZOOM_LOC_WIDTH  - get_bitmap_width(townLayout->ground_bitmap_ptr))/2;		// adjust offset
		townY1 += (STD_TOWN_LOC_HEIGHT * ZOOM_LOC_HEIGHT - get_bitmap_height(townLayout->ground_bitmap_ptr))/2;	// adjust offset

		world.zoom_matrix->put_bitmap_remap_clip( townX1, townY1, townLayout->ground_bitmap_ptr );
		return;
	}

	//-------- draw plants, buildings and flags --------//

	for( int i=0 ; i<townLayout->slot_count ; i++, townSlot++ )
	{
		//----- build_type==0 if plants -----//

		switch(townSlot->build_type)
		{
			//----- build_type>0 if town buildings -----//

			case TOWN_OBJECT_HOUSE:
				town_res.get_build( slot_object_id_array[i] )
					->draw(town_recno, sectionX1+townSlot->base_x, sectionY1+townSlot->base_y );
				break;

			case TOWN_OBJECT_PLANT:
				plant_res.get_bitmap( slot_object_id_array[i] )
					->draw_at(sectionX1+townSlot->base_x, sectionY1+townSlot->base_y);
				break;

			case TOWN_OBJECT_FARM:
				draw_farm(sectionX1+townSlot->base_x, sectionY1+townSlot->base_y, townSlot->build_code );
				break;

			case TOWN_OBJECT_FLAG:
				if( nation_recno )
					draw_flag(sectionX1+townSlot->base_x, sectionY1+townSlot->base_y);
				break;
		}
	}
}
//-------- End of function Town::draw -----------//


//------- Begin of function Town::draw_flag -----------//
//
// Draw the town section on the zoom map
//
void Town::draw_flag(int absBaseX, int absBaseY)
{
	char flagName[] = "FLAG-1";

	flagName[5] = '1' + (char) ((sys.frame_count+town_recno)%8) / 2;

	char* colorRemapTable = game.get_color_remap_table(nation_recno, 0);

	int drawX = absBaseX - world.view_top_x + ZOOM_X1 - 9;
	int drawY = absBaseY - world.view_top_y + ZOOM_Y1 - 97;

	world.zoom_matrix->put_bitmap_remap_clip(drawX, drawY, image_spict.get_ptr(flagName), colorRemapTable, 1);	// 1-the bitmap is compressed
}
//-------- End of function Town::draw_flag -----------//


//------- Begin of function Town::draw_farm -----------//
//
// Draw farming field.
//
void Town::draw_farm(int absBaseX, int absBaseY, int farmId)
{
	err_when( farmId<1 || farmId>9 );

	char farmName[] = "FARM-1";

	farmName[5] = '0' + farmId;

	int drawX = absBaseX - world.view_top_x + ZOOM_X1;
	int drawY = absBaseY - world.view_top_y + ZOOM_Y1;

	world.zoom_matrix->put_bitmap_clip(drawX, drawY, image_spict.get_ptr(farmName), 1);	// 1-the bitmap is compressed
}
//-------- End of function Town::draw_farm -----------//


//------- Begin of function Town::is_in_zoom_win -----------//
//
// Whether the town section is in the current zoom window.
//
int Town::is_in_zoom_win()
{
	int x1=world.zoom_matrix->top_x_loc;
	int y1=world.zoom_matrix->top_y_loc;
	int x2=x1+world.zoom_matrix->disp_x_loc-1;
	int y2=y1+world.zoom_matrix->disp_y_loc-1;

	return m.is_touch( x1, y1, x2, y2, loc_x1, loc_y1, loc_x2, loc_y2 );
}
//--------- End of function Town::is_in_zoom_win -----------//


//------- Begin of function Town::draw_selected -----------//
//
// Draw a square around the town section on the map.
//
void Town::draw_selected()
{
	draw_detect_link_line(0);		// 0-the action is draw only, not detecting
}
//--------- End of function Town::draw_selected -----------//


//------- Begin of function Town::draw_detect_link_line ---------//
//
// [int] actionDetect - 0 - this is a draw action
//								1 - this is a detect action
//                      (default: 0)
//
// return: <int> 1 - detected
//					  0 - not detected
//
int Town::draw_detect_link_line(int actionDetect)
{
	int 	i, firmX, firmY, townX, townY;
	Firm* firmPtr;
	Town* townPtr;

	//-------- set source points ----------//

	int srcX = ( ZOOM_X1 + (loc_x1-world.zoom_matrix->top_x_loc) * ZOOM_LOC_WIDTH
				  + ZOOM_X1 + (loc_x2-world.zoom_matrix->top_x_loc+1) * ZOOM_LOC_WIDTH ) / 2;

	int srcY = ( ZOOM_Y1 + (loc_y1-world.zoom_matrix->top_y_loc) * ZOOM_LOC_HEIGHT
				  + ZOOM_Y1 + (loc_y2-world.zoom_matrix->top_y_loc+1) * ZOOM_LOC_HEIGHT ) / 2;

	//------ draw lines to linked firms ---------//

	char* bitmapPtr;
	char  goodLinkName[9] = "GOODLINK";

	goodLinkName[7] = '1'+(char)(sys.frame_count%3);

	for( i=0 ; i<linked_firm_count ; i++ )
	{
		firmPtr = firm_array[linked_firm_array[i]];

		firmX = ( ZOOM_X1 + (firmPtr->loc_x1-world.zoom_matrix->top_x_loc) * ZOOM_LOC_WIDTH
				  + ZOOM_X1 + (firmPtr->loc_x2-world.zoom_matrix->top_x_loc+1) * ZOOM_LOC_WIDTH ) / 2;

		firmY = ( ZOOM_Y1 + (firmPtr->loc_y1-world.zoom_matrix->top_y_loc) * ZOOM_LOC_HEIGHT
				  + ZOOM_Y1 + (firmPtr->loc_y2-world.zoom_matrix->top_y_loc+1) * ZOOM_LOC_HEIGHT ) / 2;

		anim_line.draw_line(&vga_back, srcX, srcY, firmX, firmY, linked_firm_enable_array[i]==LINK_EE );

		if( !can_toggle_firm_link(linked_firm_array[i]) )
			continue;

		//------ draw the link icon and detect mouse action -----//

		bitmapPtr = power.get_link_icon( linked_firm_enable_array[i], nation_recno==firmPtr->nation_recno );

		if( actionDetect )
		{
			int rc = world.zoom_matrix->detect_bitmap_clip( firmX-11, firmY-11, bitmapPtr );

			if( rc )
				mouse.reset_click();		// reset queued mouse click for fast single clicking

			//------ left clicking to toggle link -------//

			if( rc==1 && nation_recno==nation_array.player_recno )
			{
				if( linked_firm_enable_array[i] & LINK_ED )
				{
					toggle_firm_link( i+1, 0, COMMAND_PLAYER );
					// ###### begin Gilbert 25/9 #######//
					se_ctrl.immediate_sound("TURN_OFF");
					// ###### end Gilbert 25/9 #######//
				}
				else
				{
					toggle_firm_link( i+1, 1, COMMAND_PLAYER );
					// ###### begin Gilbert 25/9 #######//
					se_ctrl.immediate_sound("TURN_ON");
					// ###### end Gilbert 25/9 #######//

				}

				// ######## begin Gilbert 23/9 ##########//
				if( firmPtr->firm_id == FIRM_CAMP && !remote.is_enable())
				// ######## end Gilbert 23/9 ##########//
				{
					if( nation_recno )
						update_target_loyalty();
					else
						update_target_resistance();

               update_camp_link();
				}

				return 1;
			}

			//------ right clicking to recruit soldiers ------//

			else if( rc==2 )
			{
				if( firmPtr->nation_recno == nation_recno && !firmPtr->under_construction &&
					 firmPtr->worker_array && firmPtr->worker_count < MAX_WORKER )
				{
					firmPtr->pull_town_people(town_recno, COMMAND_PLAYER, browse_selected_race_id(), 1);			// last 1-force pulling people from the town to the firm
					// ###### begin Gilbert 25/9 #######//
					se_ctrl.immediate_sound("PULL_MAN");
					// ###### end Gilbert 25/9 #######//
				}
			}
		}
		else
		{
			if( nation_recno == nation_array.player_recno )
				world.zoom_matrix->put_bitmap_clip( firmX-11, firmY-11, bitmapPtr );
		}
	}

	//------ draw lines to linked towns ---------//

	int townRecno;

	for( i=0 ; i<linked_town_count ; i++ )
	{
		townRecno = linked_town_array[i];

		townPtr = town_array[townRecno];

		townX = ( ZOOM_X1 + (townPtr->loc_x1-world.zoom_matrix->top_x_loc) * ZOOM_LOC_WIDTH
				  + ZOOM_X1 + (townPtr->loc_x2-world.zoom_matrix->top_x_loc+1) * ZOOM_LOC_WIDTH ) / 2;

		townY = ( ZOOM_Y1 + (townPtr->loc_y1-world.zoom_matrix->top_y_loc) * ZOOM_LOC_HEIGHT
				  + ZOOM_Y1 + (townPtr->loc_y2-world.zoom_matrix->top_y_loc+1) * ZOOM_LOC_HEIGHT ) / 2;

		anim_line.draw_line(&vga_back, srcX, srcY, townX, townY, linked_town_enable_array[i]==LINK_EE );

		//------- detect on the migration icon -------//

		if( nation_recno == nation_array.player_recno &&
			 nation_recno == townPtr->nation_recno &&
			 can_migrate(townRecno) )
		{
			bitmapPtr = image_icon.get_ptr("MIGRATE");

			if( actionDetect )
			{
				if( world.zoom_matrix->detect_bitmap_clip( townX-11, townY-11, bitmapPtr ) )
				{
					mouse.reset_click();		// reset queued mouse click for fast single clicking

					err_when(town_array[townRecno]->population>MAX_TOWN_POPULATION);
					migrate_to(townRecno, COMMAND_PLAYER);
					// ###### begin Gilbert 25/9 #######//
					se_ctrl.immediate_sound("PULL_MAN");
					// ###### end Gilbert 25/9 #######//
					return 1;
				}
			}
			else
			{
				world.zoom_matrix->put_bitmap_clip( townX-11, townY-11, bitmapPtr );
			}
		}
	}

	return 0;
}
//------- End of function Town::draw_detect_link_line ---------//


//------- Begin of function Town::can_toggle_firm_link ---------//
//
// Return whether this town can toggle the link to the given
// firm.
// 
int Town::can_toggle_firm_link(int firmRecno)
{
	if( !nation_recno )		// cannot toggle for independent town
		return 0;

	//------------------------------------//

	Firm* firmPtr;

	for( int i=0 ; i<linked_firm_count ; i++ )
	{
		if( linked_firm_array[i] != firmRecno )
			continue;

		firmPtr = firm_array[linked_firm_array[i]];

		switch( firmPtr->firm_id )
		{
			//-- you can only toggle a link to a camp if the camp is yours --//

			case FIRM_CAMP:
				return firmPtr->nation_recno == nation_recno;

			//--- town to market link is governed by trade treaty and cannot be toggled ---//

			case FIRM_MARKET:		
				return 0;			// !nation_array[nation_recno]->get_relation(firmPtr->nation_recno)->trade_treaty;

			default:
				return firm_res[firmPtr->firm_id]->is_linkable_to_town;
		}
	}

	return 0;
}
//------- Begin of function Town::can_toggle_firm_link ---------//
