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

//Filename    : OTOWNIF.CPP
//Description : Town interface routines

#include <OINFO.h>
#include <OBOX.h>
#include <OVGA.h>
#include <OSYS.h>
#include <OHELP.h>
#include <OSPY.h>
#include <OSTR.h>
#include <OFONT.h>
#include <OMOUSE.h>
#include <OVBROWIF.h>
#include <OGAME.h>
#include <ONATION.h>
#include <OBUTT3D.h>
#include <OIMGRES.h>
#include <ORAWRES.h>
#include <ORACERES.h>
#include <OWORLD.h>
#include <OUNIT.h>
#include <OTOWN.h>
#ifdef USE_DPLAY
#include <OREMOTE.h>
#endif
#include <OSE.h>
#include <OSERES.h>
#include <OBUTTCUS.h>


//------------- Define coordinations -----------//

enum { RACE_BROWSE_X1 = INFO_X1,
		 RACE_BROWSE_Y1 = INFO_Y1+48,
		 RACE_BROWSE_X2 = INFO_X2,
		 RACE_BROWSE_Y2 = RACE_BROWSE_Y1+130,
	  };

enum { BUTTON_X1 = INFO_X1,
		 BUTTON_Y1 = RACE_BROWSE_Y2+28,
		 BUTTON_X2 = INFO_X2,
		 BUTTON_Y2 = BUTTON_Y1+50,
     };

//---------- Define constant ------------//

enum { TOWN_MENU_MAIN,
		 TOWN_MENU_TRAIN,
		 TOWN_MENU_SPY,
		 TOWN_MENU_VIEW_SECRET,
		 TOWN_MENU_SET_AUTO_COLLECT_TAX,
		 TOWN_MENU_SET_AUTO_GRANT,
	  };

#define BUTTON_LOYALTY_COUNT	8
#define COUNT_BUTTON_OFFSET_X	165
#define COUNT_BUTTON_OFFSET_Y 5
#define COUNT_BUTTON_WIDTH		32
#define COUNT_BUTTON_HEIGHT	32

//----------- Define static variables ----------//

static VBrowseIF  browse_race, browse_spy;
static Button3D   button_recruit, button_train, button_tax, button_grant;
static Button3D   button_spy, button_cancel, button_spy_mobilize,
						button_spy_reward, button_spy_action, button_spy_view_secret;
static Button3D	button_cancel_training;
static ButtonCustom  button_cancel3;
static Button		button_loyalty_array[BUTTON_LOYALTY_COUNT];
static Button		button_loyalty_disabled;
static Button		button_cancel2;
static ButtonCustom button_skill[MAX_TRAINABLE_SKILL];
static ButtonCustom button_queue_skill[MAX_TRAINABLE_SKILL];
static short      browse_race_recno=1, browse_race_town_recno=0;		// the town which the browse_race displays its info
static short      recruit_race_count;
static short		spy_count;
static short      last_town_recno=0, last_rebel_recno=0;
static char 		last_has_linked_own_camp;
static char       town_menu_mode=TOWN_MENU_MAIN;
static char       disable_refresh=0;
static short 		action_spy_recno;	// recno of the spy that is doing the bribing or viewing secret reports of other nations

//-------- Define static class member var ------//

short  Town::if_town_recno = 0;

//----------- Define static functions ----------//

static int  race_filter(int recNo=0);
static int  spy_filter(int recNo=0);
static void put_race_rec(int recNo, int x, int y, int refreshFlag);
static void put_spy_rec(int recNo, int x, int y, int refreshFlag);
// ###### begin Gilbert 12/9 ########//
static void i_disp_skill_button(ButtonCustom *button, int);
static void i_disp_queue_skill_button(ButtonCustom *button, int);
// ###### end Gilbert 12/9 ########//


//--------- Begin of function Town::disp_info ---------//
//
void Town::disp_info(int refreshFlag)
{
	if_town_recno = town_recno;

	if( town_recno != last_town_recno ||
		 (refreshFlag==INFO_REPAINT && !disable_refresh) )
	{
		if( town_recno != last_town_recno )
			browse_race_recno = 1;

		town_menu_mode  = TOWN_MENU_MAIN;
		last_town_recno = town_recno;
	}

	//-----------------------------------------//

	int needRepaint=0;

	if( last_rebel_recno != rebel_recno )
	{
		last_rebel_recno = rebel_recno;
		needRepaint = 1;
	}

	if( last_has_linked_own_camp != has_linked_own_camp )
	{
		last_has_linked_own_camp = has_linked_own_camp;
		needRepaint = 1;
	}

	if( needRepaint && refreshFlag == INFO_UPDATE )
	{
		info.disp();
		return;
	}

	//-----------------------------------//

	switch( town_menu_mode )
	{
		case TOWN_MENU_MAIN:
			disp_main_menu(refreshFlag);
			break;

		case TOWN_MENU_TRAIN:
			disp_train_menu(refreshFlag);
			break;

		case TOWN_MENU_SPY:
			disp_spy_menu(refreshFlag);
			break;

		case TOWN_MENU_VIEW_SECRET:
			spy_array.disp_view_secret_menu(action_spy_recno, refreshFlag);
			break;

		case TOWN_MENU_SET_AUTO_COLLECT_TAX:
			if( refreshFlag == INFO_REPAINT )
				disp_auto_menu(1);
			break;

		case TOWN_MENU_SET_AUTO_GRANT:
			if( refreshFlag == INFO_REPAINT )
				disp_auto_menu(0);
			break;
	}
}
//----------- End of function Town::disp_info -----------//


//--------- Begin of function Town::detect_info ---------//
//
void Town::detect_info()
{
	if_town_recno = town_recno;

	switch( town_menu_mode )
	{
		case TOWN_MENU_MAIN:
			detect_main_menu();
			break;

		case TOWN_MENU_TRAIN:
			detect_train_menu();
			break;

		case TOWN_MENU_SPY:
			detect_spy_menu();
			break;

		case TOWN_MENU_VIEW_SECRET:
			if( spy_array.detect_view_secret_menu(action_spy_recno, nation_recno) )
			{
				town_menu_mode = TOWN_MENU_MAIN;
				info.disp();
			}
			break;

		case TOWN_MENU_SET_AUTO_COLLECT_TAX:
			detect_auto_menu(1);
			break;

		case TOWN_MENU_SET_AUTO_GRANT:
			detect_auto_menu(0);
			break;
	}
}
//----------- End of function Town::detect_info -----------//


//--------- Begin of function Town::disp_main_menu ---------//
//
void Town::disp_main_menu(int refreshFlag)
{
	static short lastTownNationRecno;

	//--- if the town's owner nation has just been changed ---//

	if( lastTownNationRecno != nation_recno )
	{
		lastTownNationRecno = nation_recno;
		info.disp();
		return;
	}

	//--------- display basic info --------//

	disp_basic_info(refreshFlag);

	//---------- paint controls -----------//

	if( refreshFlag == INFO_REPAINT )
	{
		recruit_race_count = race_filter();

		//------ display browser field description -------//

		int x=RACE_BROWSE_X1+2;
		int y=RACE_BROWSE_Y1-23;

		vga.d3_panel_up( RACE_BROWSE_X1, y, RACE_BROWSE_X2, RACE_BROWSE_Y1-3 );

		font_san.put( x+2  , y+4, "Population" );
		font_san.put( x+70 , y+4, "Peasants" );

		if( nation_recno )      // only display loyalty if this town is controlled by a nation
			font_san.put( x+132, y+4, "Loyalty" );
		else
		{
			#ifdef GERMAN
				font_san.put( x+128, y+4, "Resistance" );
			#else
				font_san.put( x+132, y+4, "Resistance" );
			#endif
		}

		//------------ create browser ------------//

		browse_race.init( RACE_BROWSE_X1, RACE_BROWSE_Y1, RACE_BROWSE_X2, RACE_BROWSE_Y2,
								0, 25, recruit_race_count, put_race_rec );

		browse_race.open(browse_race_recno);

		browse_race_town_recno = town_recno;		// the town which browse_race displays

		//---------- paint total section ----------//

		vga.d3_panel_up( RACE_BROWSE_X1, RACE_BROWSE_Y2+3, RACE_BROWSE_X2, RACE_BROWSE_Y2+23 );

		font_san.put( RACE_BROWSE_X1+5, RACE_BROWSE_Y2+7, "Total" );
		font_san.put( RACE_BROWSE_X1+128, RACE_BROWSE_Y2+7, "Avg" );
	}
	else
	{
		//---------- update controls -----------//

		if( recruit_race_count != race_filter() )
		{
			info.disp();
			return;
		}

		browse_race.update();
	}

	browse_race_recno = browse_race.recno();

	//----------- display total -----------//

	font_mid.put( RACE_BROWSE_X1+52, RACE_BROWSE_Y2+6, population, 1 );
	font_mid.put( RACE_BROWSE_X1+94, RACE_BROWSE_Y2+6, jobless_population, 1 );

	if( nation_recno )
		font_mid.put( RACE_BROWSE_X1+165, RACE_BROWSE_Y2+6, average_loyalty(), 1 );
	else
		font_mid.put( RACE_BROWSE_X1+165, RACE_BROWSE_Y2+6, average_resistance(nation_array.player_recno), 1 );

	//------ if this town is controlled by a rebel group -----//

	int x=BUTTON_X1, y=BUTTON_Y1;

	if( rebel_recno )
	{
		if( refreshFlag == INFO_REPAINT )
			font_san.d3_put( BUTTON_X1, y-1, BUTTON_X2, y+19, "Controlled by Rebels" );

		y+=23;
	}

	//----------- create the paint button ----------//

	if( nation_recno==nation_array.player_recno )
	{
		if( refreshFlag == INFO_REPAINT )
		{
			button_recruit.paint( BUTTON_X1, y, 'A', "RECRUIT" );

			if( has_linked_own_camp )
			{
				button_train.paint( BUTTON_X1+BUTTON_ACTION_WIDTH, y, 'A', "TRAIN" );
				button_tax.paint( BUTTON_X1+BUTTON_ACTION_WIDTH*2, y, 'A', "COLLTAX" );
				button_grant.paint( BUTTON_X1+BUTTON_ACTION_WIDTH*3, y, 'A', "GRANT" );

				disp_auto_loyalty();
			}
			else
			{
				button_train.reset();
				button_tax.reset();
				button_grant.reset();
			}

			#ifdef DEBUG
			if(debug2_enable_flag)
			{
				font_san.d3_put( INFO_X1, INFO_Y2-30, INFO_X2, INFO_Y2, "" );
				font_san.field( INFO_X1+10, INFO_Y2-20, " ", INFO_X1+20, town_recno, 1, INFO_X2-10, refreshFlag);
				font_san.field( INFO_X1+40, INFO_Y2-20, " ", INFO_X1+50, loc_x1, 1, INFO_X2-10, refreshFlag);
				font_san.field( INFO_X1+70, INFO_Y2-20, " ", INFO_X1+80, loc_y1, 1, INFO_X2-10, refreshFlag);
				font_san.field( INFO_X1+100, INFO_Y2-20, " ", INFO_X1+110, ai_link_checked, 1, INFO_X2-10, refreshFlag);
			}
			#endif
		}

		if( has_linked_own_camp )		 // a whole row is used for displaying buttons, so additional buttons will be displayed in the next row
			y += BUTTON_ACTION_HEIGHT;
		else
			x += BUTTON_ACTION_WIDTH;	 // only one button "Recruit", new button displayed next to it.

		//-------- enable/disable the train button -----------//

		int raceId = race_filter(browse_race.recno());

		if( can_recruit(raceId) )
			button_recruit.enable();
		else
			button_recruit.disable();

		if( button_train.init_flag )
		{
			if( can_train(raceId) )
				button_train.enable();
			else
				button_train.disable();
		}

		if( button_tax.init_flag )
		{
			// ###### patch begin Gilbert 5/8 ######//
			// if( average_loyalty() >= 1 )
			if( average_loyalty() > COLLECT_TAX_LOYALTY_DECREASE )
			// ###### end begin Gilbert 5/8 ######//
				button_tax.enable();
			else
				button_tax.disable();
		}

		if( button_grant.init_flag )
		{
			if( nation_array[nation_recno]->cash > 0 )
				button_grant.enable();
			else
				button_grant.disable();
		}

		//--------- display train info --------//

		if( train_unit_recno )			// display the progress of the current training process
			disp_train_info(refreshFlag);
	}

	//------ grant to an independent town ------//

	else if( nation_array.player_recno &&
				can_grant_to_non_own_town(nation_array.player_recno) )
	{
		if( refreshFlag == INFO_REPAINT )
			button_grant.paint( BUTTON_X1, y, 'A', "GRANT2" );

		if( button_grant.init_flag )
		{
			if( nation_array[nation_array.player_recno]->cash > 0 )
				button_grant.enable();
			else
				button_grant.disable();
		}

		x += BUTTON_ACTION_WIDTH;
	}

	//---------- display the spy button ----------//

	int spyFlag = spy_filter() > 0;

	if( refreshFlag == INFO_REPAINT )
	{
		if( spyFlag )				// only display the spy button for non-player towns
			button_spy.paint( x, y, 'A', "SPYMENU" );
		else
			button_spy.reset();
	}
	else
	{
		if( spyFlag != button_spy.init_flag )		// if the button availability has just changed
		{
			if(spyFlag)		// only display the spy button for non-player towns
				button_spy.paint( x, y, 'A', "SPYMENU" );
			else				// remove the button from the screen
				button_spy.hide();
		}
	}

	//-------- display debug info ----------//

	if( sys.debug_session || sys.testing_session )
		disp_debug_resistance(refreshFlag);
}
//----------- End of function Town::disp_main_menu -----------//


//--------- Begin of function Town::disp_auto_loyalty ---------//
//
void Town::disp_auto_loyalty()
{
	if( auto_collect_tax_loyalty )
	{
		vga_front.bar( button_tax.x1+8, button_tax.y1+10, button_tax.x2-12, button_tax.y2-15, V_WHITE );
		vga_front.rect( button_tax.x1+8, button_tax.y1+10, button_tax.x2-12, button_tax.y2-15, 1, V_BLACK );

		font_mid.center_put( button_tax.x1+8, button_tax.y1+10, button_tax.x2-12, button_tax.y2-15,
									m.format(auto_collect_tax_loyalty) );
	}

	if( auto_grant_loyalty )
	{
		vga_front.bar( button_grant.x1+8, button_grant.y1+10, button_grant.x2-12, button_grant.y2-15, V_WHITE );
		vga_front.rect( button_grant.x1+8, button_grant.y1+10, button_grant.x2-12, button_grant.y2-15, 1, V_BLACK );

		font_mid.center_put( button_grant.x1+8, button_grant.y1+10, button_grant.x2-12, button_grant.y2-15,
									m.format(auto_grant_loyalty) );
	}
}
//----------- End of function Town::disp_auto_loyalty -----------//


//--------- Begin of function Town::detect_main_menu ---------//
//
void Town::detect_main_menu()
{
	//--- detect clicking on the name area to center the map on it ---//

	if( mouse.single_click(INFO_X1, INFO_Y1, INFO_X2, INFO_Y1+21) )
	{
		world.go_loc( center_x, center_y );
		return;
	}

	//-------- detect browsers ---------//

	if( browse_race.detect() )
	{
		browse_race_recno = browse_race.recno();
		// ##### begin patch Gilbert 21/1 #######//
		if( sys.debug_session || sys.testing_session )
			disp_debug_resistance(INFO_UPDATE);
		// ##### end patch Gilbert 21/1 #######//
	}

	if( button_spy.detect() )	// switch to the spy menu
	{
		town_menu_mode  = TOWN_MENU_SPY;
		disable_refresh = 1;    // static var for disp_info() only
		info.disp();
		disable_refresh = 0;
		return;
	}

	//----- detect granting to an independent town ---//

	if( nation_array.player_recno &&
		 can_grant_to_non_own_town(nation_array.player_recno) )
	{
		if( button_grant.detect() )
		{
			se_ctrl.immediate_sound("TURN_ON");

			grant_to_non_own_town(nation_array.player_recno, COMMAND_PLAYER);
		}
	}

	//---------- buttons for player town only --------//

	if( nation_recno!=nation_array.player_recno )
		return;

	//------ update button status ------//

	if( browse_race.recno() > race_filter() )
		return;

	int raceId = race_filter(browse_race.recno());

	if( can_recruit(raceId) )
		button_recruit.enable();
	else
		button_recruit.disable();

	if( can_train(raceId) )
		button_train.enable();
	else
		button_train.disable();

	//------- detect buttons --------//

	if( button_recruit.detect() )
		recruit(-1, 0, COMMAND_PLAYER);

	if( button_train.detect() )
	{
		town_menu_mode = TOWN_MENU_TRAIN;
		disable_refresh = 1;    // static var for disp_info() only
		info.disp();
		disable_refresh = 0;
		return;
	}

	int rc;

	if( (rc=button_tax.detect(0, 0, 1)) > 0 )		// 1-detect right-clicking
	{
		disp_auto_loyalty();

		// ##### begin Gilbert 26/9 ########//
		se_ctrl.immediate_sound("TURN_ON");
		// ##### end Gilbert 26/9 ########//

		if( rc==1 )
		{
			town_array[town_recno]->collect_tax(COMMAND_PLAYER);
		}
		else if( rc==2 )		// right click
		{
			town_menu_mode  = TOWN_MENU_SET_AUTO_COLLECT_TAX;
			disable_refresh = 1;    // static var for disp_info() only
			info.disp();
			disable_refresh = 0;
		}
	}

	if( (rc=button_grant.detect(0, 0, 1)) > 0 )
	{
		disp_auto_loyalty();

		// ##### begin Gilbert 26/9 ########//
		se_ctrl.immediate_sound("TURN_ON");
		// ##### end Gilbert 26/9 ########//

		if( rc==1 )
		{
			town_array[town_recno]->reward(COMMAND_PLAYER);
		}
		else if( rc==2 ) 		// right click
		{
			town_menu_mode  = TOWN_MENU_SET_AUTO_GRANT;
			disable_refresh = 1;    // static var for disp_info() only
			info.disp();
			disable_refresh = 0;
		}
	}

	if(train_unit_recno)
	{
		if((rc = button_cancel_training.detect()))
		{
#ifdef USE_DPLAY
			if( !remote.is_enable() )
			{
#endif
				cancel_train_unit();
				info.disp();
#ifdef USE_DPLAY
			}
			else
			{
				short *shortPtr = (short *)remote.new_send_queue_msg(MSG_TOWN_SKIP_RECRUIT, sizeof(short));
				shortPtr[0] = town_recno;
			}
#endif
		}
	}
}
//----------- End of function Town::detect_main_menu -----------//


//--------- Begin of function Town::disp_basic_info ---------//
//
void Town::disp_basic_info(int refreshFlag)
{
	if( refreshFlag == INFO_REPAINT )
	{
		vga.d3_panel_up( INFO_X1, INFO_Y1, INFO_X2, INFO_Y1+21 );

		if( nation_recno )
		{
			font_san.center_put( INFO_X1+21, INFO_Y1, INFO_X2-2, INFO_Y1+21, town_name() );

			char *nationPict = image_button.get_ptr("V_COLCOD");

			vga_front.put_bitmap_trans_remap_decompress(INFO_X1+3, INFO_Y1+2, nationPict, game.get_color_remap_table(nation_recno, 0) );
		}
		else
		{
			font_san.center_put( INFO_X1, INFO_Y1, INFO_X2-2, INFO_Y1+21, town_name() );
		}
	}
}
//----------- End of function Town::disp_basic_info -----------//


//--------- Begin of function Town::disp_train_info ---------//
//
void Town::disp_train_info(int refreshFlag)
{
	if( !train_unit_recno || nation_recno!=nation_array.player_recno )
		return;

	int dispY1 = INFO_Y1+26;

	Unit* unitPtr = unit_array[train_unit_recno];
	int	x=MSG_X1+4, y=MSG_Y1+4;

	if( refreshFlag == INFO_REPAINT )
	{
		vga.d3_panel_up( MSG_X1, MSG_Y1, MSG_X2, MSG_Y2 );

		vga.d3_panel_down(x, y, x+RACE_ICON_WIDTH+3, y+RACE_ICON_HEIGHT+3 );
		vga_front.put_bitmap(x+2, y+2, race_res[unitPtr->race_id]->icon_bitmap_ptr );

		// vga.d3_panel_down(x+RACE_ICON_WIDTH+6, y, MSG_X2-4, MSG_Y2-4 );
	}

	int totalDays;

	if( config.fast_build && nation_recno==nation_array.player_recno )
		totalDays = TOTAL_TRAIN_DAYS/2;
	else
		totalDays = TOTAL_TRAIN_DAYS;

	vga_front.indicator( 0, x+RACE_ICON_WIDTH+6, y, float(sys.frame_count-start_train_frame_no),
		float(totalDays * FRAMES_PER_DAY), VGA_GRAY );

	button_cancel_training.paint(MSG_X2-27, MSG_Y1+2, "V_X-U", "V_X-D");
	button_cancel_training.set_help_code( "CANCELTRA" );
}
//----------- End of function Town::disp_train_info -----------//


//------ Begin of function Town::browse_selected_race_id ------//
//
// Return the id. of the race selected in the town's race browser.
//
int Town::browse_selected_race_id()
{
	err_when( town_recno != town_array.selected_recno );		// the current town must be the selected one when this function is called

	if( browse_race_town_recno != town_recno )		// the browser still hasn't displayed this town yet. This happens when the selected town is just changed, and this function is called before the town interface is refreshed
		return 0;

	if( browse_race.none_record )
		return 0;

	int raceCount = race_filter();

	if( raceCount != browse_race.total_rec() ||
		 raceCount != recruit_race_count )
	{
		info.disp();
	}

	err_when( browse_race.recno() < 1 );
	err_when( browse_race.recno() > raceCount );
	err_when( browse_race.recno() > browse_race.total_rec() );

	return race_filter(browse_race.recno());
}
//------ End of function Town::browse_selected_race_id ------//


//--------- Begin of function race_filter ---------//
//
// Filter those races that are in the town and can be trained.
//
static int race_filter(int recNo)
{
	err_when( recNo && recNo < 1 );
	err_when( recNo && recNo > recruit_race_count );
	err_when( recNo && recNo > browse_race.total_rec() );

	int 	i, recCount=0;
	Town* townPtr = town_array[Town::if_town_recno];

	for( i=0 ; i<MAX_RACE ; i++ )
	{
		if( townPtr->race_pop_array[i] > 0 )
			recCount++;

		if( recNo && recCount==recNo )
			return i+1;
	}

	err_when( recNo );

	return recCount;
}
//----------- End of function race_filter -----------//


//-------- Begin of static function put_race_rec --------//
//
static void put_race_rec(int recNo, int x, int y, int refreshFlag)
{
	//-------- display race icon -------//

	int       raceId   = race_filter(recNo);
	RaceInfo* raceInfo = race_res[raceId];

	if( refreshFlag == INFO_REPAINT )
	{
		vga.d3_panel_down(x+1, y+1, x+RACE_ICON_WIDTH+4, y+RACE_ICON_HEIGHT+4 );
		vga_front.put_bitmap(x+3, y+3, raceInfo->icon_bitmap_ptr);
	}

	//--------- set help parameters --------//

	if( mouse.in_area(x+1, y+1, x+RACE_ICON_WIDTH+4, y+RACE_ICON_HEIGHT+4) )
		help.set_unit_help( raceInfo->basic_unit_id, 0, x+1, y+1, x+RACE_ICON_WIDTH+4, y+RACE_ICON_HEIGHT+4 );

	//-------- display race name --------//

	Town* townPtr = town_array[Town::if_town_recno];

	font_mid.put( x+46, y+6, townPtr->race_pop_array[raceId-1],1, x+87 );
	font_mid.put( x+88, y+6, townPtr->jobless_race_pop_array[raceId-1], 1, x+129 );

	//---- only display loyalty if this town is controlled by a nation ----//

	int curLoyalty, targetLoyalty;
	int x2 = x+browse_race.rec_width-1;

	if( townPtr->nation_recno )
	{
		curLoyalty    = (int) townPtr->race_loyalty_array[raceId-1];
		targetLoyalty = (int) townPtr->race_target_loyalty_array[raceId-1];
	}
	else
	{
		curLoyalty    = (int) townPtr->race_resistance_array[raceId-1][nation_array.player_recno-1];
		targetLoyalty = (int) townPtr->race_target_resistance_array[raceId-1][nation_array.player_recno-1];

		if( targetLoyalty > curLoyalty )		// resistance only decrease, won't increase
			targetLoyalty = -1;					// don't display the decrease target
	}

	//---------- display loyalty/resistance ------------//

	int    dispArrow=0;
	String str;

	if( curLoyalty == targetLoyalty || targetLoyalty == -1 )					// only display up and down arrow for independent town's resistance
	{
		str = curLoyalty;
	}
	else
	{
		str  = curLoyalty;
		str += " ";
		str += targetLoyalty;

		dispArrow=1;
	}

	x2 = font_mid.center_put( x+110, y+6, x2, y+5+font_mid.height(), str, 1 );

	//--------- display up/down arrow -----------//

	if( dispArrow )
	{
		x = x2-font_mid.text_width( m.format(targetLoyalty) ) - 8;

		if( (int) targetLoyalty > (int) curLoyalty )
			image_icon.put_join( x+1, y+9, "ARROWUP" );

		else if( (int) targetLoyalty < (int) curLoyalty )
			image_icon.put_join( x+1, y+9, "ARROWDWN" );
	}
}
//----------- End of static function put_race_rec -----------//


//--------- Begin of function Town::disp_train_menu ---------//
//
void Town::disp_train_menu(int refreshFlag)
{
	// ####### begin Gilbert 13/9 ########//
	if( refreshFlag == INFO_UPDATE )
	{
		for( int i=1; i<=MAX_TRAINABLE_SKILL; i++)
		{
			button_skill[i-1].paint(-1,0);
			// button_queue_skill[i] is called by automatically
		}
	}
	else if( refreshFlag == INFO_REPAINT )
	{
		font_san.d3_put( INFO_X1, INFO_Y1, INFO_X2, INFO_Y1+18, "Train (Cost:$30, Skill:20)" );
		int x=INFO_X1, y=INFO_Y1+24;

		for(int i=1; i<=MAX_TRAINABLE_SKILL; i++)
		{
			//disp_train_button(y, i, 1);
			button_queue_skill[i-1].create(INFO_X1+COUNT_BUTTON_OFFSET_X,
			y+COUNT_BUTTON_OFFSET_Y,
			INFO_X1+COUNT_BUTTON_OFFSET_X+COUNT_BUTTON_WIDTH-1,
			y+COUNT_BUTTON_OFFSET_Y+COUNT_BUTTON_HEIGHT-1,
			i_disp_queue_skill_button, ButtonCustomPara(this,i));

			button_skill[i-1].paint(INFO_X1, y, 
			INFO_X2, y+BUTTON_ACTION_HEIGHT-1,
			i_disp_skill_button, ButtonCustomPara(&button_queue_skill[i-1],i) );

			y += BUTTON_ACTION_HEIGHT;
		}

		button_cancel3.paint( INFO_X1, y, INFO_X2, y+BUTTON_ACTION_HEIGHT*3/4-1,
		ButtonCustom::disp_text_button_func, ButtonCustomPara((void*)"Done",0) );
	}
	// ####### end Gilbert 13/9 ########//
}
//----------- End of function Town::disp_train_menu -----------//

// ######### begin Gilbert 13/9 #########//

//-------- Begin of function i_disp_skill_button --------//
//
static void i_disp_skill_button(ButtonCustom *button, int repaintBody)
{
	int x1 = button->x1;
	int y1 = button->y1;
	int x2 = button->x2;
	int y2 = button->y2;
	if( !button->pushed_flag )
	{
		if( repaintBody )
		{
			vga.blt_buf(x1, y1, x2, y2, 0);
			vga.d3_panel2_up( x1, y1, x2, y2, 1 );
		}
		x2--;
		y2--;
	}
	else
	{
		if( repaintBody )
		{
			vga.blt_buf(x1, y1, x2, y2, 0);
			vga.d3_panel2_down( x1, y1, x2, y2, 1 );
		}
		x1++;
		y1++;
	}

	ButtonCustom *queueButton = (ButtonCustom *)button->custom_para.ptr;
	if( repaintBody)
	{
		// display skill large icon
		short skillId = button->custom_para.value;
		char str[9] = "U_";
		strcat( str, Skill::skill_code_array[skillId-1] );
		char *bitmapPtr = image_button.get_ptr(str);

		vga_front.put_bitmap_trans_decompress(x1, y1+4, bitmapPtr);

		// put name

		if( skillId == SKILL_MFT )
			font_bible.put(x1+50, y1+11, "Manufacturing" );		// the string in skill_str_array[] is "Manufacture".
		else
			font_bible.put(x1+50, y1+11, Skill::skill_str_array[skillId-1]);
	}

	// display small button
	queueButton->paint(-1, repaintBody);
}
//--------- End of static function i_disp_skill_button ---------//


/*
//--------- Begin of function Town::disp_queue_button ---------//
void Town::disp_queue_button(int y, int skillId, int buttonUp)
{
	//----- count the no. of units queued for this ship ------//
	int x=INFO_X1+2+COUNT_BUTTON_OFFSET_X;
	int trainCount=0;

	for(int i=0; i<train_queue_count; i++)
	{
		if(train_queue_skill_array[i] == skillId)
			trainCount++;
	}

	if(train_unit_recno)
	{
		Unit *unitPtr = unit_array[train_unit_recno];
		if(unitPtr->skill.skill_id==skillId)
			trainCount++;
	}

	if(buttonUp)
		vga.d3_panel_up(x, y, x+COUNT_BUTTON_WIDTH-1, y+COUNT_BUTTON_HEIGHT-1);
	else
	{
		vga.d3_panel_down(x, y, x+COUNT_BUTTON_WIDTH-1, y+COUNT_BUTTON_HEIGHT-1);
		x++;
		y++;
	}

	font_san.center_put(x, y, x+COUNT_BUTTON_WIDTH-1 , y+COUNT_BUTTON_HEIGHT-1, m.format(trainCount));
}
//----------- End of function Town::disp_queue_button -----------//
*/

//-------- Begin of static function i_disp_queue_skill_button --------//
//
static void i_disp_queue_skill_button(ButtonCustom *button, int repaintBody)
{
	Town *townPtr= (Town *)button->custom_para.ptr;

	int x1 = button->x1;
	int y1 = button->y1;
	int x2 = button->x2;
	int y2 = button->y2;
	if( !button->pushed_flag )
	{
		if( repaintBody )
		{
			vga.blt_buf(x1, y1, x2, y2, 0);
			vga.d3_panel2_up( x1, y1, x2, y2, 1, 1);
		}
		x2--;
		y2--;
	}
	else
	{
		if( repaintBody )
		{
			vga.blt_buf(x1, y1, x2, y2, 0);
			vga.d3_panel2_down( x1, y1, x2, y2, 1, 1);
		}
		x1++;
		y1++;
	}

	//----- count the no. of units queued for this skill ------//

	short skillId = button->custom_para.value;
	int queuedCount=0;
	for( int i=0 ; i<townPtr->train_queue_count ; i++ )
	{
		if( townPtr->train_queue_skill_array[i] == skillId )
			queuedCount++;
	}
	if(townPtr->train_unit_recno)
	{
		Unit *unitPtr = unit_array[townPtr->train_unit_recno];
		// ##### begin Gilbert 10/10 #######//
		if(unitPtr->skill.skill_id==skillId
			//### begin alex 17/3 ###//
			//|| (unitPtr->spy_recno && skillId == SKILL_SPYING) )
			|| (skillId == SKILL_SPYING && unitPtr->spy_recno && unitPtr->skill.skill_id == 0) ) // 0 for spying-training
			 //#### end alex 17/3 ####//
			queuedCount++;
		// ##### end Gilbert 10/10 #######//
	}

	font_mid.center_put( x1+3, y1+3, x2-3, y2-3, m.format(queuedCount), 1);
}
//--------- End of static function i_disp_queue_skill_button ---------//
// ######### end Gilbert 13/9 #########//

//--------- Begin of function Town::detect_train_menu ---------//
//
void Town::detect_train_menu()
{
	int	x=INFO_X1+2, y=INFO_Y1+24, rc, quitFlag;

	for(int b=1; b<=MAX_TRAINABLE_SKILL; ++b)
	{
		// ###### begin Gilbert 10/9 ########//
		//------ detect pressing on the small queue count button -------//
		rc = 0;
		if( (rc = button_queue_skill[b-1].detect(0,0,2)) != 0)
		{
			quitFlag = 0;		// don't quit the menu right after pressing the button
		}
		//------ detect pressing on the big button -------//
		else if( (rc= button_skill[b-1].detect(0,0,2)) != 0)
		{
			quitFlag = 1;		// quit the menu right after pressing the button
		}
		// ###### end Gilbert 10/9 ########//

		//------- process the action --------//
		if( rc > 0 )
		{
			if( rc==1 )		// left button
			{
#ifdef USE_DPLAY
				if( remote.is_enable() )
				{
					// packet structure : <town recno> <skill id> <race id>
					short *shortPtr = (short *)remote.new_send_queue_msg(MSG_TOWN_RECRUIT, 3*sizeof(short) );
					shortPtr[0] = town_recno;
					shortPtr[1] = b;
					shortPtr[2] = race_filter(browse_race.recno());
				}
				else
#endif
					add_queue(b, race_filter(browse_race.recno()) );
				// ##### begin Gilbert 26/9 ########//
				se_ctrl.immediate_sound("TURN_ON");
				// ##### end Gilbert 26/9 ########//
			}
			else 				// right button - remove queue
			{
#ifdef USE_DPLAY
				if( remote.is_enable() )
				{
					// packet structure : <town recno> <skill id> <race id>
					short *shortPtr = (short *)remote.new_send_queue_msg(MSG_TOWN_RECRUIT, 3*sizeof(short) );
					shortPtr[0] = town_recno;
					shortPtr[1] = b;
					shortPtr[2] = -1;		// -1 race_id represent remove queue
				}
				else
#endif
					remove_queue(b);
				// ##### begin Gilbert 26/9 ########//
				se_ctrl.immediate_sound("TURN_OFF");
				// ##### end Gilbert 26/9 ########//
			}

			if( quitFlag )
				info.disp();		// info.disp() will call put_info() which will switch mode back to the main menu mode
			// ####### begin Gilbert 10/9 ######//
			else
				// disp_queue_button(y+COUNT_BUTTON_OFFSET_Y, b, 1);
				info.update();
			// ####### end Gilbert 10/9 ######//

			return;
		}

		y += BUTTON_ACTION_HEIGHT;
	}
	//------ detect the cancel button --------//

	if( button_cancel3.detect() || mouse.any_click(1) )		// press the cancel button or right click
	{
		// ##### begin Gilbert 26/9 ########//
		se_ctrl.immediate_sound("TURN_OFF");
		// ##### end Gilbert 26/9 ########//
      town_menu_mode = TOWN_MENU_MAIN;
		info.disp();
	}
}
//----------- End of function Town::detect_train_menu -----------//


//--------- Begin of function Town::disp_auto_menu ---------//
//
void Town::disp_auto_menu(int modeCollectTax)
{
	int curAutoLoyalty;

	Nation* nationPtr = nation_array[nation_recno];

	if( modeCollectTax )
		curAutoLoyalty =	auto_collect_tax_loyalty;
	else
		curAutoLoyalty =	auto_grant_loyalty;

	//---------- paint buttons ------------//

	const char* headingStr;

	if( modeCollectTax )
		headingStr = "Automatically Collect Tax from Villagers when their Loyalty reaches:";
	else
		headingStr = "Automatically Grant Money to Villagers when their Loyalty drops below:";

	const char* clickStr = "(Left-click below to apply to this village. Right-click below to apply to all your villages.)";

	vga.d3_panel_up( INFO_X1, INFO_Y1, INFO_X2, INFO_Y1+110 );

	font_san.put_paragraph( INFO_X1+7, INFO_Y1+8, INFO_X2-7, INFO_Y2-5, headingStr );

	font_san.put_paragraph( INFO_X1+7, INFO_Y1+58, INFO_X2-7, INFO_Y2-5, clickStr );

	int i, loyaltyLevel, y=INFO_Y1+114;

	for( i=0, loyaltyLevel=30 ; i<BUTTON_LOYALTY_COUNT ; loyaltyLevel+=10, i++, y+=20 )
		button_loyalty_array[i].paint_text( INFO_X1, y, INFO_X2, y+18, m.format(loyaltyLevel), 0, loyaltyLevel==curAutoLoyalty );

	button_loyalty_disabled.paint_text( INFO_X1, y, INFO_X2, y+18, "Disabled", 0, curAutoLoyalty==0 );
	y+=20;

	button_cancel2.paint_text( INFO_X1, y, INFO_X2, y+18, "Cancel" );
}
//----------- End of function Town::disp_auto_menu -----------//


//--------- Begin of function Town::detect_auto_menu ---------//
//
void Town::detect_auto_menu(int modeCollectTax)
{
	int i, rc=0, loyaltyLevel;

	for( i=0, loyaltyLevel=30 ; i<BUTTON_LOYALTY_COUNT ; loyaltyLevel+=10, i++ )
	{
		rc = button_loyalty_array[i].detect(0, 0, 1);

		if( rc )
			break;
	}

	if( !rc )
	{
		rc = button_loyalty_disabled.detect(0, 0, 1);
		loyaltyLevel = 0;
	}

	//------ set new settings now -------//

	if( rc==1 )
	{
		if( modeCollectTax )
		{
#ifdef USE_DPLAY
			if( !remote.is_enable() )
			{
#endif
				set_auto_collect_tax_loyalty(loyaltyLevel);
#ifdef USE_DPLAY
			}
			else
			{
				// packet structure <town recno> <loyalty level>
				short *shortPtr = (short *)remote.new_send_queue_msg(MSG_TOWN_AUTO_TAX, 2*sizeof(short) );
				*shortPtr = town_recno;
				shortPtr[1] = loyaltyLevel;
			}
#endif
		}
		else
		{
#ifdef USE_DPLAY
			if( !remote.is_enable() )
			{
#endif
				set_auto_grant_loyalty(loyaltyLevel);
#ifdef USE_DPLAY
			}
			else
			{
				// packet structure <town recno> <loyalty level>
				short *shortPtr = (short *)remote.new_send_queue_msg(MSG_TOWN_AUTO_GRANT, 2*sizeof(short) );
				*shortPtr = town_recno;
				shortPtr[1] = loyaltyLevel;
			}
#endif
		}
	}
	else if( rc==2 )
	{
		// ####### begin Gilbert 11/9 ########//
		//----- set the national policy -----//
#ifdef USE_DPLAY
		if( !remote.is_enable() )
		{
#endif
			Nation* nationPtr = nation_array[nation_recno];

			if( modeCollectTax )
				nationPtr->set_auto_collect_tax_loyalty(loyaltyLevel);
			else
				nationPtr->set_auto_grant_loyalty(loyaltyLevel);

			//----- update individual towns -----//

			Town* townPtr;

			for( i=town_array.size() ; i>0 ; i-- )
			{
				if( town_array.is_deleted(i) )
					continue;

				townPtr = town_array[i];
				if( townPtr->nation_recno == nation_recno )
				{
					if( modeCollectTax )
						townPtr->set_auto_collect_tax_loyalty(loyaltyLevel);
					else
						townPtr->set_auto_grant_loyalty(loyaltyLevel);
				}
			}
#ifdef USE_DPLAY
		}
		else
		{
			err_when(!nation_recno);
			if( modeCollectTax )
			{
				// packet structure <-nation recno> <loyalty level>
				short *shortPtr = (short *)remote.new_send_queue_msg(MSG_TOWN_AUTO_TAX, 2*sizeof(short) );
				*shortPtr = -nation_recno;
				shortPtr[1] = loyaltyLevel;
			}
			else
			{
				// packet structure <-nation recno> <loyalty level>
				short *shortPtr = (short *)remote.new_send_queue_msg(MSG_TOWN_AUTO_GRANT, 2*sizeof(short) );
				*shortPtr = -nation_recno;
				shortPtr[1] = loyaltyLevel;
			}
		}
#endif
		// ####### end Gilbert 11/9 ########//
	}

	//--------------------------------------//

	if( button_cancel2.detect() || rc )
	{
		// ##### begin Gilbert 26/9 ########//
		se_ctrl.immediate_sound("TURN_OFF");
		// ##### begin Gilbert 26/9 ########//
		town_menu_mode = TOWN_MENU_MAIN;
		info.disp();
	}
}
//----------- End of function Town::detect_auto_menu -----------//


//--------- Begin of function Town::disp_spy_menu ---------//
//
void Town::disp_spy_menu(int refreshFlag)
{
	disp_basic_info(refreshFlag);

	//---------- paint controls -----------//

	if( refreshFlag == INFO_REPAINT )
	{
		spy_count = spy_filter();

		//------ display browser field description -------//

		int x=RACE_BROWSE_X1+2;
		int y=RACE_BROWSE_Y1-23;

		vga.d3_panel_up( RACE_BROWSE_X1, y, RACE_BROWSE_X2, RACE_BROWSE_Y1-3 );

		font_san.put( x+4  , y+4, "Spy Skill" );
		font_san.put( x+70 , y+4, "Loyalty" );
		font_san.put( x+130, y+4, "Action" );

		//------------ create browser ------------//

		browse_spy.init( RACE_BROWSE_X1, RACE_BROWSE_Y1, RACE_BROWSE_X2, RACE_BROWSE_Y2,
								0, 25, spy_count, put_spy_rec );

		browse_spy.open(1);
	}
	else
	{
		//---------- update controls -----------//

		if( spy_count != spy_filter() )
		{
			spy_count = spy_filter();

			if( spy_count>0 )
			{
				disable_refresh = 1;		// stay in the spy menu mode if disable_refresh is 1
				info.disp();
				disable_refresh = 0;
			}
			else
				info.disp();				// reset to the main menu mode if disable_refresh is 0

			return;
		}
		else
			browse_spy.update();
	}

	//----------- create the paint button ----------//

	if( refreshFlag == INFO_REPAINT )
	{
		int x=BUTTON_X1, y=RACE_BROWSE_Y2+5;

		button_spy_mobilize.paint( x, y, 'A', "MOBILSPY" );
		x+=BUTTON_ACTION_WIDTH;

		//--------- reward spy button --------//

		button_spy_reward.paint( x, y, 'A', "REWARD" );
		x+=BUTTON_ACTION_WIDTH;

		if( nation_recno != nation_array.player_recno )		// if the spy is in another nation's town
		{
			button_spy_action.paint( x, y, 'A', "SPYCHACT" );
			x+=BUTTON_ACTION_WIDTH;
		}

		if( nation_recno && nation_recno != nation_array.player_recno )
		{
			button_spy_view_secret.paint( x, y, 'A', "VSECRET" );
			x+=BUTTON_ACTION_WIDTH;

			if( x+BUTTON_ACTION_WIDTH-5 > INFO_X2 )
			{
				x  = BUTTON_X1;
				y += BUTTON_ACTION_HEIGHT;
			}
		}

		button_cancel.paint( x, y, 'A', "PREVMENU" );
	}
}
//----------- End of function Town::disp_spy_menu -----------//


//--------- Begin of function Town::detect_spy_menu ---------//
//
void Town::detect_spy_menu()
{
	browse_spy.detect();

	Spy* spyPtr = spy_array[ spy_filter( browse_spy.recno() ) ];

	//------- mobilize spy --------//

	if( button_spy_mobilize.detect() )
	{
#ifdef USE_DPLAY
		if( !remote.is_enable() )
		{
#endif
			if( spyPtr->mobilize_town_spy() )
			{
				spyPtr->notify_cloaked_nation_flag = 0;		// reset it so the player can control it 
				disp_spy_menu( INFO_UPDATE );
			}
#ifdef USE_DPLAY
		}
		else
		{
			// packet structure <spy recno>
			short *shortPtr = (short *)remote.new_send_queue_msg(MSG_SPY_LEAVE_TOWN, sizeof(short) );
			*shortPtr = spyPtr->spy_recno;
		}
#endif
	}

	//------ reward spy ---------//

	else if( button_spy_reward.detect() )
	{
		spyPtr->reward(COMMAND_PLAYER);
	}

	//----- change spy action --------//

	if( nation_recno != nation_array.player_recno )		// if the spy is in another nation's town
	{
		if( button_spy_action.detect() )		// set action mode
		{
#ifdef USE_DPLAY
			if( !remote.is_enable() )
			{
#endif
				spyPtr->set_next_action_mode();
				disp_spy_menu( INFO_UPDATE );
#ifdef USE_DPLAY
			}
			else
			{
				// packet structure <spy recno>
				short *shortPtr = (short *)remote.new_send_queue_msg(MSG_SPY_CYCLE_ACTION, sizeof(short) );
				*shortPtr = spyPtr->spy_recno;
			}
#endif
		}
	}

	//----- view secret report ------/

	if( nation_recno && nation_recno != nation_array.player_recno )
	{
		if( button_spy_view_secret.detect() )
		{
			action_spy_recno = spyPtr->spy_recno;
			town_menu_mode   = TOWN_MENU_VIEW_SECRET;
			disable_refresh = 1;
			info.disp();
			disable_refresh = 0;
		}
	}

	//--------- cancel -----------//

	if( button_cancel.detect() || mouse.any_click(1) )		// right click to cancel
		info.disp();
}
//----------- End of function Town::detect_spy_menu -----------//


//--------- Begin of function Town::has_player_spy ---------//
//
// Whether this town has any player spies.
//
int Town::has_player_spy()
{
	int i;
	for( i=0 ; i<MAX_RACE ; i++ )
	{
		if( race_spy_count_array[i] > 0 )
			break;
	}

	if( i==MAX_RACE )		// no spies in this nation
		return 0;

	//----- look for player spy in the spy_array -----//

	Spy* spyPtr;

	for( i=spy_array.size() ; i>=1 ; i-- )
	{
		if( spy_array.is_deleted(i) )
			continue;

		spyPtr = spy_array[i];

		if( spyPtr->spy_place==SPY_TOWN &&
			 spyPtr->spy_place_para==town_recno &&
			 spyPtr->true_nation_recno==nation_array.player_recno )
		{
			return 1;
		}
	}

	return 0;
}
//----------- End of function Town::has_player_spy -----------//


//--------- Begin of function spy_filter ---------//
//
static int spy_filter(int recNo)
{
	Spy* spyPtr;
	int  recCount=0;

	for( int i=spy_array.size() ; i>=1 ; i-- )
	{
		if( spy_array.is_deleted(i) )
			continue;

		spyPtr = spy_array[i];

		if( spyPtr->spy_place==SPY_TOWN &&
			 spyPtr->spy_place_para==Town::if_town_recno &&
			 spyPtr->true_nation_recno==nation_array.player_recno )
		{
			recCount++;
		}

		if( recNo && recCount==recNo )
			return i;
	}

	err_when( recNo );

	return recCount;
}
//----------- End of function spy_filter -----------//


//-------- Begin of static function put_spy_rec --------//
//
static void put_spy_rec(int recNo, int x, int y, int refreshFlag)
{
	int x2 = x+browse_spy.rec_width-1;

	//-------- display icon of the spy unit -----//

	Spy* spyPtr = spy_array[ spy_filter(recNo) ];

	if( refreshFlag == INFO_REPAINT )
	{
		vga.d3_panel_down(x+1, y+1, x+RACE_ICON_WIDTH+4, y+RACE_ICON_HEIGHT+4 );
		vga_front.put_bitmap(x+3, y+3, race_res[spyPtr->race_id]->icon_bitmap_ptr);
	}

	//--------- set help parameters --------//

	if( mouse.in_area(x+1, y+1, x+RACE_ICON_WIDTH+4, y+RACE_ICON_HEIGHT+4) )
	{
		int unitId = race_res[spyPtr->race_id]->basic_unit_id;

		help.set_unit_help( unitId, 0, x+1, y+1, x+RACE_ICON_WIDTH+4, y+RACE_ICON_HEIGHT+4 );
	}

	//-------- display spy skill -------//

	font_san.put( x+40, y+6, spyPtr->spy_skill, 1, x+66 );

	//-------- display spy loyalty -------//

	font_san.put( x+67, y+6, spyPtr->spy_loyalty, 1, x+94 );

	//------ display the action mode of the spy ------//

	vga.blt_buf( x+95, y+6, x2, y+5+font_san.height(), 0 );

	font_san.center_put( x+95, y+6, x2, y+5+font_san.height(), spyPtr->action_str() );
}
//----------- End of static function put_spy_rec -----------//


//--------- Begin of function Town::recruit ---------//
//
// <int> trainSkillId =  -1 - non-trained unit
//                      >=1 - skill id. of the unit to be trained.
//
// [int] raceId       = the race id. of the unit to be recruited
//                      (default: the currently selected race)
//
// return: <int> recno of the recruited unit
//
int Town::recruit(int trainSkillId, int raceId, char remoteAction)
{
	//---- we can't train a new one when there is one currently under training ---//

	if( trainSkillId >= 1 && train_unit_recno )
		return 0;

	//--------------------------------------------//

	if( !raceId )
	{
		if( browse_race.recno() > race_filter() )
			return 0;

		raceId = race_filter(browse_race.recno());
	}

#ifdef USE_DPLAY
	if( !remoteAction && remote.is_enable() )
	{
		// packet structure : <town recno> <skill id> <race id>
		short *shortPtr = (short *)remote.new_send_queue_msg(MSG_TOWN_RECRUIT, 3*sizeof(short));
		shortPtr[0] = town_recno;
		shortPtr[1] = trainSkillId;
		shortPtr[2] = raceId;
		return 0;
	}
#endif

	//---- check if there are units of the race ready for training ----//

	int recruitableCount = recruitable_race_pop(raceId, 1);

	if( recruitableCount == 0 )
		return 0;

	err_when( recruitableCount < 0 );		// 1-allow recruiting spies

	//-------- create an unit ------//

	int townRecno = town_recno;
	int nationRecno = nation_recno;        // save this town's info that is needed as promote_pop() will delete Town if all population of the town are promoted

	//--- if there are spies in this town, chances are that they will be mobilized ---//

	int shouldTrainSpy = race_spy_count_array[raceId-1] >= m.random(recruitableCount)+1;		// 1-allow recruiting spies

	//---- if we are trying to train an enemy to our spy, then... -----//

	if( shouldTrainSpy && trainSkillId == SKILL_SPYING )
	{
		//-- if there are other non-spy units in the town, then train the other and don't train the spy --//

		if( recruitableCount > race_spy_count_array[raceId-1] )
		{
			shouldTrainSpy = 0;
		}
		//--- if all remaining units are spies, when you try to train one, all of them will become mobilized ---//

		else
		{
			int spyRecno = spy_array.find_town_spy(town_recno, raceId, 1);

			err_when( !spyRecno );

			Spy* spyPtr = spy_array[spyRecno];

			if( !spyPtr->mobilize_town_spy() )
				return 0;

			spyPtr->change_cloaked_nation( spyPtr->true_nation_recno );

			return 0;
		}
	}

	//------- if we should train a spy --------//

	int unitRecno=0;

	if( shouldTrainSpy )
	{
		int  spyCount = spy_array.size();
		int  spyRecno = m.random(spyCount)+1;
		Spy* spyPtr;

		//-----------------------------------------------------//
		// Spies from other nations will first be mobilized,
		// when all peasants and spies are mobilized and
		// the only ones left in the town are spies from our
		// nation, then mobilize them finally.
		//-----------------------------------------------------//

		for( int mobileNationType=1 ; unitRecno==0 && mobileNationType<=2 ; mobileNationType++ )
		{
			if( mobileNationType==2 )	// only mobilize our own spies are there are the only ones in the town
			{
				if( recruitable_race_pop(raceId,1) > race_spy_count_array[raceId-1] )		// 1-allow recruiting spies
					break;
			}

			for( int i=0 ; i<spyCount ; i++ )
			{
				if( ++spyRecno > spyCount )
					spyRecno = 1;

				if( spy_array.is_deleted(spyRecno) )
					continue;

				spyPtr = spy_array[spyRecno];

				if( spyPtr->spy_place == SPY_TOWN
					 && spyPtr->spy_place_para == town_recno 
					 // ##### patch begin Gilbert 9/4 ######//
					 && spyPtr->race_id == raceId
					 // ##### patch end Gilbert 9/4 ######//
					 )
				{
					if( mobileNationType==1 )		// only mobilize spies from other nations, don't mobilize spies of our own nation
					{
						if( spyPtr->true_nation_recno == nation_recno )
								continue;
					}

					unitRecno = spyPtr->mobilize_town_spy(trainSkillId== -1);	// the parameter is whether decreasing the population immediately, if decrease immediately in recruit mode, not in training mode, 1-mobilize spies
					break;
				}
			}
		}
	}

	//-------- mobilize normal peasant units -------//

	if( !unitRecno )
		unitRecno = mobilize_town_people(raceId, trainSkillId== -1, 0 ); 	// the 2nd parameter is whether decreasing the population immediately, if decrease immediately in recruit mode, not in training mode, 2nd para 0-don't mobilize spies

	if( !unitRecno )
		return 0;

	err_when(unitRecno<=0 || unit_array.is_deleted(unitRecno));

	if( !unitRecno )
		return 0;

	Unit* unitPtr = unit_array[unitRecno];

	//-------- training skill -----------//

	if( trainSkillId > 0 )
	{
		if( trainSkillId == SKILL_SPYING )
		{
			unitPtr->spy_recno = spy_array.add_spy(unitRecno, TRAIN_SKILL_LEVEL);
		}
		else
		{
			if( trainSkillId == SKILL_LEADING )		// also increase the combat level for leadership skill training
				unitPtr->set_combat_level(TRAIN_SKILL_LEVEL);
		
			unitPtr->skill.skill_id    = trainSkillId;
			unitPtr->skill.skill_level = TRAIN_SKILL_LEVEL;
		}

		nation_array[nationRecno]->add_expense( EXPENSE_TRAIN_UNIT, (float) TRAIN_SKILL_COST );
	}
	else
	{
		//------ recruitment without training decreases loyalty --------//

		recruit_dec_loyalty(raceId);

		if( unitPtr->is_own() )
		{
			se_res.far_sound(unitPtr->cur_x_loc(), unitPtr->cur_y_loc(), 1,
				'S', unitPtr->sprite_id, "RDY" );
		}
	}

	//---- training solider or skilled unit takes time ----//

	if( trainSkillId >= 0 )
	{
		err_when(unitRecno<=0 || unit_array.is_deleted(unitRecno));

		err_when( train_unit_recno );		// if there is already a unit under training

		train_unit_recno = unitRecno;
		start_train_frame_no = sys.frame_count;	// as an offset for displaying the progress bar correctly

		unitPtr->deinit_sprite();
		unitPtr->unit_mode = UNIT_MODE_UNDER_TRAINING;
      unitPtr->unit_mode_para = town_recno;
	}

	//--- mobilize_pop() will delete the current Town if population goes down to 0 ---//

	if( town_recno == town_array.selected_recno )
	{
		if( town_array.is_deleted(townRecno) )
			info.disp();
	}

	return unitRecno;
}
//----------- End of function Town::recruit -----------//


//--------- Begin of function Town::recruit_dec_loyalty ---------//
//
// Decrease loyalty when an unit is recruited.
// This function is called by recruit() and Firm::pull_town_people()
//
// <int> raceId - the race to be recruited
// <int> decNow - decrease now, if it is 0, just return the
//						loyalty to be decreased without actual decreasing.
//						(default: 1)
//
// return: <int> - the loyalty decreased or to be decreased.
//
int Town::recruit_dec_loyalty(int raceId, int decNow)
{
	float loyaltyDec = MIN( 5, (float) MAX_TOWN_POPULATION / race_pop_array[raceId-1] );

	//------ recruitment without training decreases loyalty --------//

	if( decNow )
	{
		loyaltyDec += accumulated_recruit_penalty/5;

		loyaltyDec = MIN(loyaltyDec, 10);

		accumulated_recruit_penalty += 5;

		//-------------------------------------//

		race_loyalty_array[raceId-1] -= loyaltyDec;

		if( race_loyalty_array[raceId-1] < 0 )
			race_loyalty_array[raceId-1] = (float) 0;
	}

	return (int) loyaltyDec;
}
//----------- End of function Town::recruit_dec_loyalty -----------//


//--------- Begin of function Town::process_train ---------//
void Town::process_train()
{
	err_when( !train_unit_recno );

	Unit* unitPtr = unit_array[train_unit_recno];
	int   raceId  = unitPtr->race_id;

	//---- if the unit being trained was killed -----//

	int cancelFlag = 0;

	err_when( jobless_race_pop_array[raceId-1] < 0 );

	if( jobless_race_pop_array[raceId-1]==0 )		// the unit being trained was killed
	{
		cancelFlag = 1;
	}

	//-----------------------------------------------------------------//
	//
	// If after start training the unit (non-spy unit), a unit has been
	// mobilized, resulting that the spy count >= jobless_race,
	// we must cancel the training, otherwise when training finishes,
	// and dec_pop is called, spy count will > jobless count and cause error.
	//
	//-----------------------------------------------------------------//

	err_when( race_spy_count_array[raceId-1] > jobless_race_pop_array[raceId-1] );

	if( race_spy_count_array[raceId-1] == jobless_race_pop_array[raceId-1] )
		cancelFlag = 1;

	if( cancelFlag )
	{
		unit_array.disappear_in_town(train_unit_recno, town_recno);
		train_unit_recno = 0;
		return;
	}

	//------------- process training ---------------//

	int totalDays;

	if( config.fast_build && nation_recno==nation_array.player_recno )
		totalDays = TOTAL_TRAIN_DAYS/2;
	else
		totalDays = TOTAL_TRAIN_DAYS;

	if( (int)(sys.frame_count-start_train_frame_no) / FRAMES_PER_DAY >= totalDays )
	{
		finish_train(unitPtr);
	}
}
//----------- End of function Town::process_train -----------//


//--------- Begin of function Town::finish_train ---------//

void Town::finish_train(Unit* unitPtr)
{
	err_when(train_unit_recno<=0 || unit_array.is_deleted(train_unit_recno));
	SpriteInfo*	spriteInfo = unitPtr->sprite_info;
	int 			xLoc=loc_x1; // xLoc & yLoc are used for returning results
	int 			yLoc=loc_y1;

	if( !world.locate_space(xLoc, yLoc, loc_x2, loc_y2, spriteInfo->loc_width, spriteInfo->loc_height) )
		return;

	unitPtr->init_sprite(xLoc, yLoc);

	if( unitPtr->is_own() )
		se_res.far_sound( xLoc, yLoc, 1, 'S', unitPtr->sprite_id, "RDY");

	unitPtr->unit_mode = 0;		// reset it to 0 from UNIT_MODE_UNDER_TRAINING
	train_unit_recno   = 0;

	int townRecno = town_recno;		// save the recno as it can be deleted in dec_pop()

	dec_pop(unitPtr->race_id, 0);		// decrease the population now as the recruit() does do so

	//---- if this trained unit is tied to an AI action ----//

	if( train_unit_action_id )
	{
		nation_array[nation_recno]->process_action_id(train_unit_action_id);
		train_unit_action_id = 0;
	}

	//----- refresh if this town is currently selected ------//

	if(townRecno==town_array.selected_recno)
	{
		if(town_menu_mode==TOWN_MENU_MAIN)
		{
			info.disp();
		}
		else
		{
			disable_refresh = 1;
			info.disp();
			disable_refresh = 0;
		}
	}
}
//----------- End of function Town::finish_train -----------//


//--------- Begin of function Town::process_queue ---------//
void Town::process_queue()
{
	if(train_queue_count==0)
		return;

	if(jobless_population==0)
		return;

	err_when(train_queue_count > MAX_TRAIN_QUEUE);
	
	char queueCount = train_queue_count;
	char skillId, raceId;
	char i;
	for(i=0; i<queueCount; ++i)
	{
		if(can_train(train_queue_race_array[i]))
		{
			skillId = train_queue_skill_array[i];
			raceId = train_queue_race_array[i];
			err_when(train_queue_count-i-1 < 0 || train_queue_count-i-1 > MAX_TRAIN_QUEUE);
			memmove(train_queue_skill_array, train_queue_skill_array+i+1,
						sizeof(train_queue_skill_array[0])*(train_queue_count-i-1));
			memmove(train_queue_race_array, train_queue_race_array+i+1,
						sizeof(train_queue_race_array[0])*(train_queue_count-i-1));
			train_queue_count -= i+1;
			recruit(skillId, raceId, COMMAND_AUTO);
			break;
		}
	}
	
	if(i==queueCount)
		train_queue_count = 0;

	if(town_menu_mode==TOWN_MENU_MAIN)
		info.disp();
}
//----------- End of function Town::process_queue -----------//


//--------- Begin of function Town::add_queue ---------//
void Town::add_queue(char skillId, char raceId)
{
	if(train_queue_count+(train_unit_recno>0)==MAX_TRAIN_QUEUE)
		return;

	train_queue_skill_array[train_queue_count] = skillId;
	train_queue_race_array[train_queue_count++] = raceId;

	if( !train_unit_recno )
		process_queue();
}
//----------- End of function Town::add_queue -----------//


//--------- Begin of function Town::remove_queue ---------//
void Town::remove_queue(char skillId)
{
	for(int i=train_queue_count-1; i>=0; i--)
	{
		if(train_queue_skill_array[i] == skillId)
		{
			err_when(train_queue_count > MAX_TRAIN_QUEUE);

			m.del_array_rec(train_queue_skill_array, train_queue_count, sizeof(train_queue_skill_array[0]), i+1);
			m.del_array_rec(train_queue_race_array, train_queue_count, sizeof(train_queue_race_array[0]), i+1);
			train_queue_count--;
			return;
		}
	}

	if(train_unit_recno)
	{
		Unit *unitPtr = unit_array[train_unit_recno];
		if((unitPtr->skill).skill_id==skillId)
			cancel_train_unit();
	}
}
//----------- End of function Town::remove_queue -----------//


//--------- Begin of function Town::disp_debug_resistance ---------//
//
void Town::disp_debug_resistance(int refreshFlag)
{
	if( nation_recno == nation_array.player_recno )		// not for player's own town
		return;

	if( refreshFlag==INFO_REPAINT )
		vga.d3_panel_up( INFO_X1, INFO_Y2-50, INFO_X2, INFO_Y2 );

	//------ display resistance (only for independent town) -----//

	int x=INFO_X1+10, y=INFO_Y2-47;

	if( nation_recno ==0 )
	{
		int raceId = race_filter(browse_race.recno());

		for( int i=1 ; i<=nation_array.size() ; i++, x+=28 )
		{
			if( nation_array.is_deleted(i) )
				continue;

			if( refreshFlag==INFO_REPAINT )
				vga_front.bar( x, y, x+18, y+16, nation_array[i]->nation_color );

			font_san.put( x, y+18, (int) race_resistance_array[raceId-1][i-1], 1, x+19 );
			font_san.put( x, y+32, (int) race_target_resistance_array[raceId-1][i-1], 1, x+19 );
		}
	}
	else
	{
		//------ if this town is the nation's base town -----//

		String str;
		
		str = "Base: ";
		str += is_base_town;

		font_san.put( INFO_X1+10, y, str );

		str = "Town recno: ";
		str += town_recno;

		font_san.put( INFO_X1+70, y, str );

		str = "no_neighbor_space: ";
		str += no_neighbor_space;

		font_san.put( INFO_X1+10, y+16, str );

		str = "quality of life: ";
		str += quality_of_life;

		font_san.disp( INFO_X1+10, y+32, str, INFO_X2-10 );
	}
}
//----------- End of function Town::disp_debug_resistance -----------//


//--------- Begin of function Town::get_elected_race ---------//
int Town::get_selected_race()
{
	if(browse_race.recno() > race_filter())
		return 0;

	return race_filter(browse_race.recno());
}
//----------- End of function Town::get_elected_race -----------//
