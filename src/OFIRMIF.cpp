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

//Filename    : OFIRMIF.CPP
//Description : Firm interface routines

#include <OSTR.h>
#include <KEY.h>
#include <OVGA.h>
#include <vga_util.h>
#include <OHELP.h>
#include <OMOUSE.h>
#include <OFONT.h>
#include <OBUTT3D.h>
#include <OBUTTON.h>
#include <OSLIDER.h>
#include <ONATION.h>
#include <OINFO.h>
#include <ORACERES.h>
#include <OUNIT.h>
#include <OSPY.h>
#include <OTOWN.h>
#include <OWORLD.h>
#include <OFIRM.h>
#include <OREMOTE.h>
#include <OIMGRES.h>
#include <OGAME.h>
#include <OSYS.h>
#include "gettext.h"

//---------- Define static variables ------------//

static Button3D button_sell, button_destruct, button_builder, button_request_builder;
static short  	 pop_disp_y1;
static char     worker_id_array[MAX_WORKER];
static Firm*    cur_firm_ptr;

//---------- Declare static function ------------//

static void disp_worker_hit_points(int x1, int y1, int x2, int hitPoints, int maxHitPoints);
static int sort_worker_id_function(const void *a, const void *b);

//--------- Begin of function Firm::disp_info_both ---------//
//
void Firm::disp_info_both(int refreshFlag)
{
	static char  lastUnderConstruction;
	static short lastFirmRecno;
	static char  lastShouldShowInfo;
	static short lastPlayerSpyCount;

	//------ check if under_construction has been changed -----//

	if( lastUnderConstruction != under_construction )
	{
		lastUnderConstruction = under_construction;

		if( refreshFlag == INFO_UPDATE )
			info.disp();							// refresh the display
	}

	//--------------------------------------------//

	if( under_construction )
	{
		disp_basic_info(INFO_Y1, refreshFlag);

		if( refreshFlag == INFO_REPAINT )
			font_san.d3_put( INFO_X1, INFO_Y1+54, INFO_X2, INFO_Y1+74, _("Under Construction") );
	}
	else
	{
		//------ check if should_show_info() has been changed -----//

		int shouldShowInfo = should_show_info();

		if( shouldShowInfo   != lastShouldShowInfo ||
			 player_spy_count != lastPlayerSpyCount )
		{
			lastShouldShowInfo = shouldShowInfo;
			lastPlayerSpyCount = player_spy_count;

			if( refreshFlag == INFO_UPDATE )
			{
				info.disp();
				return;
			}
		}

		//----------------------------------------//

		if( lastFirmRecno != firm_recno ||
			 (firm_menu_mode != FIRM_MENU_ASSASSINATE_RESULT &&
			  player_spy_count==0 && bribe_result==BRIBE_NONE) )		// don't refresh mode if the player spy in the firm has just failed the bribing and executed, and we now need to display the failure message
		{
			firm_menu_mode = FIRM_MENU_MAIN;
			lastFirmRecno = firm_recno;
			bribe_result  = BRIBE_NONE;
		}

		switch( firm_menu_mode )
		{
			case FIRM_MENU_MAIN:
				put_info(refreshFlag);
				break;

			case FIRM_MENU_SPY:
			case FIRM_MENU_SELECT_BRIBER:
				disp_spy_menu(refreshFlag);
				break;

			case FIRM_MENU_SET_BRIBE_AMOUNT:
				disp_bribe_menu(refreshFlag);
				break;

			case FIRM_MENU_VIEW_SECRET:
				spy_array.disp_view_secret_menu(action_spy_recno, refreshFlag);
				break;

			case FIRM_MENU_ASSASSINATE_RESULT:
				disp_assassinate_result(refreshFlag);
				break;
		}

		#ifdef DEBUG
		if(debug2_enable_flag)
		{
			font_san.d3_put( INFO_X1, INFO_Y2-30, INFO_X2, INFO_Y2, "" );
			font_san.field( INFO_X1+10, INFO_Y2-20, " ", INFO_X1+20, firm_recno, 1, INFO_X2-10, refreshFlag);
			font_san.field( INFO_X1+40, INFO_Y2-20, " ", INFO_X1+50, loc_x1, 1, INFO_X2-10, refreshFlag);
			font_san.field( INFO_X1+70, INFO_Y2-20, " ", INFO_X1+80, loc_y1, 1, INFO_X2-10, refreshFlag);
			font_san.field( INFO_X1+100, INFO_Y2-20, " ", INFO_X1+110, ai_link_checked, 1, INFO_X2-10, refreshFlag);
		}
		#endif
	}
}
//----------- End of function Firm::disp_info_both -----------//


//--------- Begin of function Firm::detect_info_both ---------//
//
// Called by Info::detect(). Detect both cases when the firm is
// under construction and is normal. 
//
void Firm::detect_info_both()
{
	if( under_construction )
		detect_basic_info();
	else
	{
		switch( firm_menu_mode )
		{
			case FIRM_MENU_MAIN:
				if( detect_info() )
					return;
				break;

			case FIRM_MENU_SPY:
			case FIRM_MENU_SELECT_BRIBER:
				detect_spy_menu();
				break;

			case FIRM_MENU_SET_BRIBE_AMOUNT:
				detect_bribe_menu();
				break;

			case FIRM_MENU_VIEW_SECRET:
				if( spy_array.detect_view_secret_menu(action_spy_recno, nation_recno) )
				{
					firm_menu_mode = FIRM_MENU_MAIN;
					info.disp();
				}
				break;

			case FIRM_MENU_ASSASSINATE_RESULT:
				detect_assassinate_result();
				break;
		}
	}

	if( ISKEY(KEYEVENT_OBJECT_PREV) )
	{
		firm_array.disp_next(-1, 0);    // previous same object type of any nation
		return;
	}

	if( ISKEY(KEYEVENT_OBJECT_NEXT) )
	{
		firm_array.disp_next(1, 0);     // next same object type of any nation
		return;
	}

	if( ISKEY(KEYEVENT_NATION_OBJECT_PREV) )
	{
		firm_array.disp_next(-1, 1);    // prevous same object type of the same nation
		return;
	}

	if( ISKEY(KEYEVENT_NATION_OBJECT_NEXT) )
	{
		firm_array.disp_next(1, 1);     // next same object type of the same nation
		return;
	}
}
//----------- End of function Firm::detect_info_both -----------//


//--------- Begin of function Firm::disp_basic_info ---------//
//
void Firm::disp_basic_info(int dispY1, int refreshFlag)
{
	//------- display the name of the firm --------//

	if( refreshFlag == INFO_REPAINT )
	{
		vga_util.d3_panel_up( INFO_X1, dispY1, INFO_X2, dispY1+21 );

		if( nation_recno )
		{
			font_san.center_put( INFO_X1+21, dispY1, INFO_X2-2, dispY1+21, firm_name() );

			char *nationPict = image_button.get_ptr("V_COLCOD");

			vga_front.put_bitmap_remap(INFO_X1+3, dispY1+2, nationPict, game.get_color_remap_table(nation_recno, 0) );
		}
		else
		{
			font_san.center_put( INFO_X1, dispY1, INFO_X2-2, dispY1+21, firm_name() );
		}
	}

	dispY1+=23;

	//------- display hit points and buttons -------//

	int sliderX1, sliderX2;

	if( under_construction )
		sliderX1 = INFO_X1+34;		// there is only one button in the contruction mode, so the slider is longer
	else
		sliderX1 = INFO_X1+64;

	sliderX2 = INFO_X2-64;

	int showRepairIcon = builder_recno && !under_construction && should_show_info();
	int showReqRepairIcon = !builder_recno && !under_construction && should_show_info() && own_firm() && find_idle_builder(0);
	err_when( showRepairIcon && showReqRepairIcon );

	if( refreshFlag == INFO_REPAINT )
	{
		button_sell.reset();
		button_destruct.reset();

		vga_util.d3_panel_up( INFO_X1, dispY1, INFO_X2, dispY1+26 );

		if( nation_array.player_recno &&
			 nation_recno == nation_array.player_recno )
		{
			if( under_construction || !can_sell() )
			{
				button_destruct.paint( INFO_X1+4, dispY1+1, "V_DEM-U", "V_DEM-D" );	// Destruct

				if( under_construction )
					button_destruct.set_help_code( "CANCELBLD" );
				else
					button_destruct.set_help_code( "DESTFIRM" );
			}

			if( !under_construction && can_sell() )
			{
				button_sell.paint( INFO_X1+4, dispY1+1, "V_SEL-U", "V_SEL-D" );	// Sell
				button_sell.set_help_code( "SELLFIRM" );
			}
		}

		if( showRepairIcon )
		{
			button_request_builder.init_flag = 0;
			button_builder.paint( INFO_X1+30, dispY1+1, "REPAIRU", "REPAIRD" );	// Builder
			button_builder.set_help_code( "REPAIR" );
		}

		else if( showReqRepairIcon )
		{
			button_builder.init_flag = 0;
			button_request_builder.paint( INFO_X1+30, dispY1+1, "REPAIRQU", "REPAIRQD" );
			button_request_builder.set_help_code( "REPAIRQ" );
		}
	}
	else	//--------- INFO_UPDATE --------//
	{
		if( showRepairIcon )
		{
			button_request_builder.hide();
			button_request_builder.reset();

			if( !button_builder.init_flag || !button_builder.enable_flag )
			{
				button_builder.paint( INFO_X1+30, dispY1+1, "REPAIRU", "REPAIRD" );	// Builder
				button_builder.set_help_code( "REPAIR" );
			}
		}

		else if( showReqRepairIcon )
		{
			button_builder.hide();
			button_builder.reset();

			if( !button_request_builder.init_flag || !button_request_builder.enable_flag )
			{
				button_request_builder.paint( INFO_X1+30, dispY1+1, "REPAIRQU", "REPAIRQD" );
				button_request_builder.set_help_code( "REPAIRQ" );
			}
		}

		else
		{
			button_builder.hide();
			button_request_builder.hide();
		}

	}

	disp_hit_point(dispY1);
}
//----------- End of function Firm::disp_basic_info -----------//


//--------- Begin of function Firm::detect_basic_info ---------//
//
int Firm::detect_basic_info()
{
	//--- click on the name area to center the map on it ---//

	if( mouse.single_click(INFO_X1, INFO_Y1, INFO_X2, INFO_Y1+21) )
	{
		world.go_loc( center_x, center_y );
		return 1;
	}

	//----------- Mobilize the builder ---------//

	int detectBuilder = builder_recno && !under_construction &&
							  unit_array[builder_recno]->is_own();			// this is your unit in your firm or it is a spy of yours in an enemy firm

	if( detectBuilder && button_builder.detect(0, 0, 1) )		// 1-detect right button also
	{
		if( !remote.is_enable() )
			set_builder(0);
		else
		{
			// packet structure : <firm recno>
			short *shortPtr = (short *)remote.new_send_queue_msg(MSG_FIRM_MOBL_BUILDER, sizeof(short));
			*shortPtr = firm_recno;
		}

		return 1;
	}

	else if( button_request_builder.init_flag && button_request_builder.detect(0, 0, 1) )
	{
		send_idle_builder_here(COMMAND_PLAYER);
	}

	//---------------------------------------//

	if( !own_firm() )
		return 0;

	//---------- "Destruct" button -----------//

	if( button_destruct.detect(KEY_DEL) )
	{
		if( under_construction )
			cancel_construction(COMMAND_PLAYER);
		else
			destruct_firm(COMMAND_PLAYER);

		return 1;
	}

	//------------ "Sell" button -------------//

	if( button_sell.detect(KEY_DEL) )
	{
		sell_firm(COMMAND_PLAYER);
		return 1;
	}

	return 0;
}
//----------- End of function Firm::detect_basic_info -----------//


//--------- Begin of function Firm::disp_worker_list ---------//
//
void Firm::disp_worker_list(int dispY1, int refreshFlag)
{
	pop_disp_y1 = dispY1;

	//---------------- paint the panel --------------//

	if( refreshFlag == INFO_REPAINT )
		vga_util.d3_panel_up( INFO_X1, dispY1, INFO_X2, dispY1+60 );

	//----------- display populatin distribution ---------//

	int overseerRaceId=0;

	if( overseer_recno )
		overseerRaceId = unit_array[overseer_recno]->race_id;

	if( selected_worker_id > worker_count )
		selected_worker_id = worker_id_array[worker_count-1];

	//------ display population composition -------//

	int	  x, y;
	Worker* workerPtr = worker_array;
	static  char last_race_id_array[MAX_WORKER];
	static  char last_unit_id_array[MAX_WORKER];

	dispY1+=1;

	for( int i=0 ; i<MAX_WORKER ; i++ )
	{
		workerPtr = &worker_array[worker_id_array[i]-1];
		x = INFO_X1+4+i%4*50;
		y = dispY1+i/4*29;

		if( i<worker_count )
		{
			if( refreshFlag==INFO_REPAINT ||
				 last_race_id_array[i] != workerPtr->race_id ||
				 last_unit_id_array[i] != workerPtr->unit_id )
			{
				vga_front.put_bitmap(x+2, y+2, workerPtr->small_icon_ptr());
			}

			//----- highlight the selected worker -------//

			if( selected_worker_id == worker_id_array[i] )
				vga_front.rect( x, y, x+27, y+23, 2, V_YELLOW );
			else
				vga_front.rect( x, y, x+27, y+23, 2, vga_front.color_up );

			//------ display hit points bar --------//

			disp_worker_hit_points( x+2, y+24, x+25, workerPtr->hit_points, workerPtr->max_hit_points() );

			//----- display combat or skill level ------//

			const char* spyIconName=NULL;

			if( workerPtr->spy_recno )
			{
				Spy* spyPtr = spy_array[workerPtr->spy_recno];

				//------ if this is the player's spy -------//

				if( nation_array.player_recno &&
					 spyPtr->true_nation_recno == nation_array.player_recno )
				{
					spyIconName = "U_SPY";
				}

				//--------------------------------------------//
				//
				// If this is an enemy spy and this firm belongs
				// to the player and there is a player's phoenix
				// over this firm and the spying skill of the spy
				// is low (below 40)
				//
				//--------------------------------------------//
/*
				else if( spyPtr->spy_skill < 40 &&
							nation_recno == nation_array.player_recno &&
							nation_array.player_recno &&
					 (~nation_array)->revealed_by_phoenix(loc_x1, loc_y1) )
				{
					spyIconName = "ENEMYSPY";
				}
*/
			}

			//--------------------------------------//

			if( spyIconName )
			{
				vga_front.put_bitmap( x+30, y+6, image_icon.get_ptr(spyIconName) );
				vga_util.blt_buf( x+40, y+6, x+49, y+15, 0 );
				vga_util.blt_buf( x+30, y+16, x+49, y+26, 0 );
			}
			else
			{
				if( firm_id == FIRM_CAMP )
					font_san.disp(x+30, y+6, workerPtr->combat_level, 1, x+49);
				else
					font_san.disp(x+30, y+6, workerPtr->skill_level, 1, x+49);
			}

			last_race_id_array[i] = workerPtr->race_id;
			last_unit_id_array[i] = workerPtr->unit_id;

			//------- set help parameters ---------//

			if( mouse.in_area(x, y, x+27, y+23) )
				help.set_unit_help( workerPtr->unit_id, 0, x, y, x+27, y+23 );
		}
		else
		{
			if( last_race_id_array[i] != 0 || last_unit_id_array[i] != 0 )
			{
				vga_util.blt_buf( x, y, x+49, y+27, 0 );
				last_race_id_array[i] = 0;
				last_unit_id_array[i] = 0;
			}
		}
	}
}
//----------- End of function Firm::disp_worker_list -----------//


//--------- Begin of function Firm::detect_worker_list ---------//
//
int Firm::detect_worker_list()
{
	if( !should_show_info() )
		return 0;

	//------- detect buttons on hiring firm workers -------//

	int i, x, y;
	int liveInTown = firm_res[firm_id]->live_in_town;

	for( i=0 ; i<worker_count ; i++ )
	{
		x = INFO_X1+6+i%4*50;
		y = pop_disp_y1+1+i/4*29;

		switch( mouse.any_click(x, y, x+27, y+23, LEFT_BUTTON) ? 1 : (mouse.any_click(x, y, x+27, y+23, RIGHT_BUTTON) ? 2 : 0) )
		{
			case 1:         // left button to select worker
				if (selected_worker_id == worker_id_array[i])
					selected_worker_id = 0;
				else
					selected_worker_id = worker_id_array[i];
				return 1;

			case 2:
				if( own_firm() )		// only if this is our own firm
				{
					//--- if the town where the unit lives belongs to the nation of this firm ---//

					mobilize_worker(worker_id_array[i], COMMAND_PLAYER);
					return 1;
				}
				break;
		}
	}

	return 0;
}
//----------- End of function Firm::detect_worker_list -----------//


//--------- Begin of function disp_worker_hit_points ---------//
//
static void disp_worker_hit_points(int x1, int y1, int x2, int hitPoints, int maxHitPoints)
{
	//------- determine the hit bar type -------//

	#define HIT_BAR_TYPE_COUNT  3

	int  hit_bar_color_array[HIT_BAR_TYPE_COUNT] = { 0xA8, 0xB4, 0xAC };
	int  hit_bar_max_array[HIT_BAR_TYPE_COUNT] 	= { 50, 100, 200 };
	char hitBarColor;

	for( int i=0 ; i<HIT_BAR_TYPE_COUNT ; i++ )
	{
		if( maxHitPoints <= hit_bar_max_array[i] || i==HIT_BAR_TYPE_COUNT-1 )
		{
			hitBarColor = hit_bar_color_array[i];
			break;
		}
	}

	//------- draw the hit points bar -------//

	enum { HIT_BAR_DARK_BORDER = 3,
			 HIT_BAR_BODY 		   = 1 };

	int barWidth = (x2-x1+1) * hitPoints / MAX(hitPoints, maxHitPoints);

	vga_front.bar( x1, y1, x1+barWidth-1, y1+1, hitBarColor + HIT_BAR_BODY );

	if( x1+barWidth <= x2 )
		vga_util.blt_buf( x1+barWidth, y1, x2, y1+1, 0 );

	y1+=2;

	vga_front.bar( x1, y1, x1+barWidth-1, y1, hitBarColor + HIT_BAR_DARK_BORDER );
	vga_front.bar( x1+barWidth, y1, x1+barWidth, y1, V_BLACK );

	if( x1+barWidth+1 <= x2 )
		vga_util.blt_buf( x1+barWidth+1, y1, x2, y1, 0 );

	y1++;

	vga_front.bar( x1+1, y1, x1+barWidth, y1, V_BLACK );

	if( x1+barWidth+1 <= x2 )
		vga_util.blt_buf( x1+barWidth+1, y1, x2, y1, 0 );
}
//----------- End of function disp_worker_hit_points -----------//


//--------- Begin of function Firm::disp_worker_info ---------//
//
void Firm::disp_worker_info(int dispY1, int refreshFlag)
{
	static int lastSelected;

	if( selected_worker_id > worker_count )
		selected_worker_id = worker_id_array[worker_count-1];

	if( lastSelected != selected_worker_id > 0 )
	{
		lastSelected = selected_worker_id > 0;
		info.disp();			// redisplay the interface
		return;
	}

	//------ if selected_worker_id==0, display overseer info -----//

	if( selected_worker_id==0 )		// display overseer info
	{
		disp_overseer_info(dispY1, refreshFlag);
		return;
	}

	//---------------- paint the panel --------------//

	if( refreshFlag == INFO_REPAINT )
	{
		if( firm_id == FIRM_CAMP )		// the command base has one more field
			vga_util.d3_panel_up( INFO_X1, dispY1, INFO_X2, dispY1+71 );
		else
			vga_util.d3_panel_up( INFO_X1, dispY1, INFO_X2, dispY1+55 );
	}

	if( selected_worker_id > 0 )
	{
		int x=INFO_X1+4, y=dispY1+4;

		Worker* workerPtr = worker_array + selected_worker_id - 1;

		if( workerPtr->town_recno )		// FirmInfo::live_in_town is 1
		{
			Town* townPtr = town_array[workerPtr->town_recno];
			font_san.field( x, y, _("Residence"), x+100, townPtr->town_name(), INFO_X2-2, refreshFlag);
			y+=16;

			if( town_array[workerPtr->town_recno]->nation_recno == nation_recno &&
				 workerPtr->race_id )
			{
				info.disp_loyalty( x, y, x+100, workerPtr->loyalty(), workerPtr->target_loyalty(firm_recno), nation_recno, refreshFlag );
			}
			else
				font_san.field( x, y, _("Loyalty"), x+100, _("N/A"), INFO_X2-2, refreshFlag );		// no loyalty because it does not belong to your empire
		}
		else										// FirmInfo::live_in_town is 0
		{
			if( workerPtr->race_id )
				info.disp_loyalty( x, y, x+100, workerPtr->loyalty(), workerPtr->target_loyalty(firm_recno), nation_recno, refreshFlag );
			else
				font_san.field( x, y, _("Loyalty"), x+100, _("N/A"), INFO_X2-2, refreshFlag );		// no loyalty because it does not belong to your empire
		}

		y+=16;

		//----------------------------------------//

		String str;

		if( workerPtr->race_id )
			str = misc.format(workerPtr->skill_level, 1);
		else
			str = _("N/A");

		font_san.field( x, y, _(Skill::skill_str_array[workerPtr->skill_id-1]),
			x+100, str, INFO_X2-2, refreshFlag);

		y+=16;

		//----------------------------------------//

		if( firm_id == FIRM_CAMP )
		{
			if( workerPtr->race_id )
				str = misc.format(workerPtr->combat_level, 1);
			else
				str = _("N/A");

			font_san.field( x, y, _("Combat"), x+100, str, INFO_X2-2, refreshFlag);
			y+=16;

			//---------------------------------------//

			str  = workerPtr->hit_points;
			str += "/";
			str += workerPtr->max_hit_points();

			font_san.field( x, y, _("Hit Points"), x+100, str, INFO_X2-2, refreshFlag);
		}
	}
}
//----------- End of function Firm::disp_worker_info -----------//


//--------- Begin of function Firm::disp_overseer_info ---------//
//
void Firm::disp_overseer_info(int dispY1, int refreshFlag)
{
	if( refreshFlag == INFO_REPAINT )
	{
		if( firm_id == FIRM_CAMP )
			vga_util.d3_panel_up( INFO_X1, dispY1, INFO_X2, dispY1+71 );
		else
			vga_util.d3_panel_up( INFO_X1, dispY1, INFO_X2, dispY1+55 );
	}

	if( !overseer_recno )
		return;

	int x=INFO_X1+4, y=dispY1+4;

	Unit* unitPtr = unit_array[overseer_recno];

	if( unitPtr->rank_id != RANK_KING )
	{
		info.disp_loyalty( x, y, x+100, unitPtr->loyalty, unitPtr->target_loyalty, nation_recno, refreshFlag );
		y+=16;
	}

	font_san.field( x, y, _(Skill::skill_str_array[unitPtr->skill.skill_id-1]),
		x+100, unitPtr->skill.skill_level , 1, INFO_X2-2, refreshFlag);

	y+=16;

	if( firm_id==FIRM_CAMP )		// only display combat level in camps and don't display it in seat of power
	{
		font_san.field( x, y, _("Combat") , x+100, unitPtr->skill.combat_level, 1, INFO_X2-2, refreshFlag);
		y+=16;
	}

	String str;
	str  = (int) unitPtr->hit_points;
	str += "/";
	str += unitPtr->max_hit_points;

	font_san.field( x, y, _("Hit Points"), x+100, str, INFO_X2-2, refreshFlag);
}
//----------- End of function Firm::disp_overseer_info -----------//


//----------- Begin of function Firm::disp_hit_point -----------//
void Firm::disp_hit_point(int dispY1)
{
	float hitPoints;

	if( hit_points > (float)0 && hit_points < (float)1 )
		hitPoints = (float) 1;		// display 1 for value between 0 and 1
	else
		hitPoints = hit_points;

	Vga::active_buf->indicator(0x0b, INFO_X1+58, dispY1+1, hitPoints, max_hit_points, 0);
}
//----------- End of function Firm::disp_hit_point -----------//


//--------- Begin of function Firm::sort_worker ---------//
//
// This is used to display the workers sorted by skill.
//
void Firm::sort_worker()
{
	if( firm_array.selected_recno != firm_recno )
		return;

	for( int i=0 ; i<MAX_WORKER ; i++ )
	{
		worker_id_array[i] = i+1;
	}

	if( worker_count > 1 )
	{
		cur_firm_ptr = this;
		qsort(worker_id_array, worker_count, sizeof(char), sort_worker_id_function);
	}
}
//----------- End of function Firm::sort_worker -----------//


//--------- Begin of function sort_worker_id_function ---------//
//
static int sort_worker_id_function(const void *a, const void *b)
{
	int workerId1 = *((char*)a);
	int workerId2 = *((char*)b);

	return cur_firm_ptr->worker_array[workerId2-1].skill_level -
			cur_firm_ptr->worker_array[workerId1-1].skill_level;
}
//----------- End of function sort_worker_id_function -----------//
