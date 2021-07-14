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

//Filename	  : OSYS2.CPP
//Description : System resource management object

#include <OVGA.h>
#include <vga_util.h>
#include <OMOUSE.h>
#include <OFONT.h>
#include <OBUTT3D.h>
#include <OVBROWSE.h>
#include <ONATION.h>
#include <OFIRM.h>
#include <OTOWN.h>
#include <OSITE.h>
#include <OUNIT.h>
#include <OHELP.h>
#include <OTUTOR.h>
#include <ONEWS.h>
#include <OBULLET.h>
#include <OREBEL.h>
#include <OREMOTE.h>
#include <OSPY.h>
#include <OINFO.h>
#include <OGAME.h>
#include <OWORLD.h>
#include <OSYS.h>
#include <ORAWRES.h>
#include <OTALKRES.h>
#include <OWEATHER.h>
#include <OANLINE.h>
#include <OTORNADO.h>
#include <OSE.h>
#include <OSNOWG.h>
#include <OROCK.h>
#include <OEFFECT.h>
#include <OLOG.h>
#include <OCONFIG.h>
#include <OPOWER.h>
#include <OSERES.h>
#include <OIMGRES.h>
#include <OWARPT.h>
#include <OMOUSECR.h>
#include <OFIRMDIE.h>
#include <OOPTMENU.h>
#include <OINGMENU.h>
#include <CmdLine.h>
#include <gettext.h>


//---------- define static variables ----------//

static int		  report_disp_frame_no;
static Button3D  button_menu;

//-------- Begin of function Sys::detect --------//
//
void Sys::detect()
{
	//--- only detect at the even frames when in report mode ---//

	if( view_mode != MODE_NORMAL &&
		// ###### begin Gilbert 5/11 ######//
		 !report_disp_frame_no )
		// ###### end Gilbert 5/11 ######//
	{
		return;
	}

	//--------------------------------------//

	mouse.get_event();

	if( option_menu.is_active() )
	{
		option_menu.detect();
		return;
	}

	if( in_game_menu.is_active() )
	{
		in_game_menu.detect();
		return;
	}

	if( mouse.is_key_event() )
	{
		process_key(mouse.scan_code, mouse.event_skey_state);
	}

	detect_button();		// detect main buttons on the screen

	detect_view();
}
//--------- End of function Sys::detect ---------//


//-------- Begin of function Sys::process --------//
//
void Sys::process()
{
	//------- update frame count and is_sync_frame --------//

	frame_count++;
	is_sync_frame = frame_count%3==0;	// check if sychronization should take place at this frame (for handling one sync per n frames)

	//--------- process objects -----------//

	LOG_MSG(misc.get_random_seed());
	LOG_MSG("begin unit_array.process()");
	unit_array.process();
	seek_path.reset_total_node_avail();	// reset node for seek_path
	LOG_MSG("end unit_array.process()");
	LOG_MSG(misc.get_random_seed());

	LOG_MSG("begin firm_array.process()");
	firm_array.process();
	LOG_MSG("end firm_array.process()");
	LOG_MSG(misc.get_random_seed());

	LOG_MSG("begin town_array.process()");
	town_array.process();
	LOG_MSG("end town_array.process()");
	LOG_MSG(misc.get_random_seed());

	LOG_MSG("begin nation_array.process()");
	nation_array.process();
	LOG_MSG("end nation_array.process()");
	LOG_MSG(misc.get_random_seed());

	LOG_MSG("begin bullet_array.process()");
	bullet_array.process();
	LOG_MSG("end bullet_array.process()");
	LOG_MSG(misc.get_random_seed());

	LOG_MSG("begin world.process()");
	world.process();
	LOG_MSG("end world.process()");
	LOG_MSG(misc.get_random_seed());

	LOG_MSG("begin tornado_array.process()");
	tornado_array.process();
	LOG_MSG("begin tornado_array.process()");
	LOG_MSG(misc.get_random_seed());

	LOG_MSG("begin snow_ground_array.process()");
	snow_ground_array.process();
	LOG_MSG("end snow_ground_array.process()");
	LOG_MSG(misc.get_random_seed());

	LOG_MSG("begin rock_array.process()");
	rock_array.process();
	LOG_MSG("end rock_array.process()");
	LOG_MSG(misc.get_random_seed());

	LOG_MSG("begin dirt_array.process()");
	dirt_array.process();
	LOG_MSG("end dirt_array.process()");
	LOG_MSG(misc.get_random_seed());

	LOG_MSG("begin effect_array.process()");
	effect_array.process();
	LOG_MSG("end effect_array.process()");
	LOG_MSG(misc.get_random_seed());

	LOG_MSG("begin war_point_array.process()");
	war_point_array.process();
	LOG_MSG("end war_point_array.process()");

	LOG_MSG("begin firm_die.process()");
	firm_die_array.process();
	LOG_MSG("end firm_die.process()");

	//------ check if it's time for the next day ------//

	if( ++day_frame_count > FRAMES_PER_DAY )
	{
		LOG_MSG("begin info.next_day()");
		info.next_day();
		LOG_MSG("end info.next_day()");
		LOG_MSG(misc.get_random_seed());

		LOG_MSG("begin world.next_day()");
		world.next_day();
		LOG_MSG("end world.next_day()");
		LOG_MSG(misc.get_random_seed());

		LOG_MSG("begin site_array.next_day()");
		site_array.next_day();
		LOG_MSG("end site_array.next_day()");
		LOG_MSG(misc.get_random_seed());

		LOG_MSG("begin rebel_array.next_day()");
		rebel_array.next_day();
		LOG_MSG("end rebel_array.next_day()");
		LOG_MSG(misc.get_random_seed());

		LOG_MSG("begin spy_array.next_day()");
		spy_array.next_day();
		LOG_MSG("end spy_array.next_day()");
		LOG_MSG(misc.get_random_seed());

		LOG_MSG("begin sprite_res.update_speed()");
		if( config.weather_effect)
			sprite_res.update_speed();
		LOG_MSG("end sprite_res.update_speed()");
		LOG_MSG(misc.get_random_seed());

		LOG_MSG("begin raw_res.next_day()");
		raw_res.next_day();
		LOG_MSG("end raw_res.next_day()");
		LOG_MSG(misc.get_random_seed());

		LOG_MSG("begin talk_res.next_day()");
		talk_res.next_day();
		LOG_MSG("end talk_res.next_day()");
		LOG_MSG(misc.get_random_seed());

		LOG_MSG("begin region_array.next_day()");
		region_array.next_day();
		LOG_MSG("end region_array.next_day()");
		LOG_MSG(misc.get_random_seed());

		day_frame_count = 0;
	}

	//------ display the current frame ------//

	LOG_MSG("begin sys.disp_frame");
	misc.lock_seed();
	if( cmd_line.enable_if )
		disp_frame();
	misc.unlock_seed();
	LOG_MSG("end sys.disp_frame");
	LOG_MSG(misc.get_random_seed() );

	//-----------------------------------------//

	/*
	#ifdef DEBUG
		//------- debug codes used to count town's defender number -------//
		Unit *unitPtr;
		Town *townPtr;
		for(int i=unit_array.size(); i>0; i--)
		{
			if(unit_array.SpriteArray::is_deleted(i))
				continue;

			unitPtr = unit_array[i];
			
			if(unitPtr->unit_mode==UNIT_MODE_DEFEND_TOWN)
			{
				if(town_array.is_deleted(unitPtr->unit_mode_para))
					continue;

				townPtr = town_array[unitPtr->unit_mode_para];
				if(townPtr->nation_recno==unitPtr->nation_recno)
					townPtr->debug_defender_count++;
			}
		}

		for(i=town_array.size(); i>0; i--)
		{
			if(town_array.is_deleted(i))
				continue;

			townPtr = town_array[i];
			err_when(townPtr->debug_defender_count != townPtr->town_defender_count);
			townPtr->debug_defender_count = 0;
		}
	#endif
	*/
	//-*********** simulate aat ************-//
	#ifdef DEBUG
		if(debug_sim_game_type==2)
		{
			//if(misc.random(20)==0)
			if(misc.random(50)==0)
			{
				#define MAX_ATTACKER	30
				int arraySize = unit_array.size();
				int i, j, i2, j2;
				short nationRecno;
				Unit *unitPtr, *targetPtr;

				short attackerArray[MAX_ATTACKER];
				short attackerCount = 0;
				
				//----------- select attackers -----------//
				//for(i=misc.random(arraySize)+1, j=1; j<=arraySize; j++, i++)
				for(i=int(misc.get_random_seed()%arraySize)+1, j=1; j<=arraySize; j++, i++)
				{
					if(i>arraySize)
						i = 1;

					if(unit_array.is_deleted(i))
						continue;

					unitPtr = (Unit*) unit_array[i];
					if(!unitPtr->is_visible())
						continue;

					if(attackerCount)
					{
						if(unitPtr->nation_recno!=nationRecno)
							continue;
					}
					else
						nationRecno = unitPtr->nation_recno;

					err_when(i!=unitPtr->sprite_recno);
					err_when(i>arraySize);

					if(misc.random(4)==0)
						attackerArray[attackerCount++] = i;

					if(attackerCount>=MAX_ATTACKER/2)
						break; // array full
				}

				//--------- selecet victim ----------//
				//if(day_frame_count==9 && frame_count==6082)
				//	int debug  =0;
				if(misc.random(10))
				{
					//for(i2=misc.random(arraySize)+1, j2=1; j2<=arraySize; j2++, i2++)
					for(i2=int(misc.get_random_seed()%arraySize)+1, j2=1; j2<=arraySize; j2++, i2++)
					{
						if(i2>arraySize)
							i2 = 1;

						if(unit_array.is_deleted(i2))
							continue;

						targetPtr = (Unit*) unit_array[i2];
						if(!targetPtr->is_visible())
							continue;

						if(targetPtr->nation_recno==unitPtr->nation_recno)
							continue;

						err_when(i2>arraySize);
						unit_array.attack(targetPtr->next_x_loc(), targetPtr->next_y_loc(), 0, attackerArray, attackerCount, 0, 0);
						break;
					}
				}
				else
					unit_array.move_to(misc.random(MAX_WORLD_X_LOC), misc.random(MAX_WORLD_Y_LOC), 0, attackerArray, attackerCount, 1);
			}
		}
	#endif
	//-*********** simulate aat ************-//
}
//--------- End of function Sys::process ---------//


//-------- Begin of function Sys::disp_button --------//
//
void Sys::disp_button()
{
	vga.use_back();

	button_menu.paint( 720, 6, "MENU-U", "MENU-D" );
	button_menu.set_help_code( "GAMEMENU" );

	vga.use_front();
}
//--------- End of function Sys::disp_button ---------//


//-------- Begin of function Sys::detect_button --------//
//
void Sys::detect_button()
{
	//--------- detect menu button -------//

	if( button_menu.detect() )
	{
		// ##### begin Gilbert 5/11 #######//
		// game.in_game_menu();
		in_game_menu.enter(!remote.is_enable());
		// ##### end Gilbert 5/11 #######//
		return;
	}

	//------- detect view mode buttons -----//

	#define VIEW_MODE_BUTTON_WIDTH   58
	#define VIEW_MODE_BUTTON_HEIGHT  16
	#define VIEW_MODE_BUTTON_X_SPACE  5

	int i, x=6, y=8;

	static char viewModeArray[] =
	{
		MODE_NATION, MODE_TOWN, MODE_ECONOMY, MODE_TRADE, MODE_MILITARY, MODE_TECH, MODE_SPY, MODE_RANK
	};

	for( i=0 ; i<8 ; i++, x+=VIEW_MODE_BUTTON_WIDTH+VIEW_MODE_BUTTON_X_SPACE )
	{
		if( i==4 )		// the second row
		{
			x=12;
			y=29;
		}

		if( nation_array.player_recno==0 && i<7 )		// when the player has lost the game, the only report available is ranking report only
			continue;

		if( nation_array.nation_count==0 && i==7 )		// unless there are no nations at all, then the ranking report is also disabled
			continue;

		if( mouse.single_click( x, y, x+VIEW_MODE_BUTTON_WIDTH-1, y+VIEW_MODE_BUTTON_HEIGHT-1 ) )
		{
			int newMode = viewModeArray[i];

			if( view_mode == newMode )       	// when click on the same mode again, go to the normal mode
				set_view_mode(MODE_NORMAL);
			else
				set_view_mode(newMode);

			break;
		}
	}
}
//--------- End of function Sys::detect_button ---------//


//-------- Begin of function Sys::set_view_mode --------//
//
// <int> viewMode 			 - id. of the view mode.
// [int] viewingNationRecno - which nation the player is viewing at with the reports.
//										(default: the player nation)
// [int] viewingSpyRecno 	 - >0 if the spy is viewing secret reports of other nations
//
void Sys::set_view_mode(int viewMode, int viewingNationRecno, int viewingSpyRecno)
{
	if( view_mode == viewMode )
		return;

	//---- if the player's kingdom has been destroyed ----//

	err_when( viewingNationRecno && nation_array.is_deleted(viewingNationRecno) );

	if( nation_array.is_deleted(info.default_viewing_nation_recno) )
	{
		if( viewMode != MODE_NORMAL && viewMode != MODE_RANK )		// other reports are not available except the normal and rank report
			return;
	}

	//---- a spy is exposed when he has finished viewing the secrets ----//

	if( info.viewing_spy_recno )
	{
		if( !spy_array.is_deleted(info.viewing_spy_recno) )
		{
			Spy* spyPtr = spy_array[info.viewing_spy_recno];

			int needViewSecretSkill = spy_array.needed_view_secret_skill(info.viewing_spy_recno);
			int escapeChance = spyPtr->spy_skill - needViewSecretSkill;
			int killFlag = 0;

			if( escapeChance > 0 )
			{
				if( misc2.random( escapeChance/15 )==0  )		// use m2 instead of m to maintain mulitplayer sync
					killFlag = 1;
			}

			if( killFlag )
				spyPtr->set_exposed(COMMAND_PLAYER);
		}

		info.viewing_spy_recno = 0;
	}

	//----------------------------------------------------//

	if( viewMode == MODE_NORMAL )
	{
		info.viewing_nation_recno = info.default_viewing_nation_recno;
	}
	else
	{
		if( viewingNationRecno )
			info.viewing_nation_recno = viewingNationRecno;
		else
			info.viewing_nation_recno = info.default_viewing_nation_recno;

		info.viewing_spy_recno = viewingSpyRecno;
	}
	if( viewMode == MODE_NATION && info.nation_report_mode == NATION_REPORT_CHAT )
		SDL_StartTextInput();
	else
		SDL_StopTextInput();

	view_mode = viewMode;
	disp_view_mode();

	disp_view();
}
//--------- End of function Sys::set_view_mode ---------//


//-------- Begin of function Sys::disp_frame --------//

void Sys::disp_frame()
{
	if( sys.signal_exit_flag )
		return;

	if( option_menu.is_active() )
	{
		// ##### begin Gilbert 3/11 ######//
		option_menu.disp(need_redraw_flag);
		// ##### end Gilbert 3/11 ######//
		blt_virtual_buf();
	}
	else
	{
		// -------- re-draw the whole screen if needed, such as after task switching ---------//

		if( need_redraw_flag )
		{
			info.disp_panel();
			world.paint();
			disp_button();
			world.refresh();
			disp_view();

			if( in_game_menu.is_active() )
			{
				vga.use_back();
				in_game_menu.disp();
				vga.use_front();
			}

			vga_util.blt_buf(0,0, VGA_WIDTH-1, VGA_HEIGHT-1, 0);
			// ###### begin Gilbert 4/11 ######//
			disp_view_mode();
			// ###### end Gilbert 4/11 ######//

			info.disp();
		}
		else
		{
			update_view();
			info.update();
		}

		//--------- display the map and info area --------//

		disp_map();

		blt_virtual_buf();

		//---------- display help ----------//

		if( !remote.is_enable() )		// help is only available in a single player game as it has to pause the game
			help.disp();
	}
	// ####### end Glbert 24/10 #######//

	anim_line.inc_phase();		// originally in Sys::process()

	need_redraw_flag = 0;
}
//-------- End of function Sys::disp_frame --------//


//-------- Begin of function Sys::disp_view --------//
//
// Display the view area.
//
void Sys::disp_view()
{
	disp_zoom();
	// ###### begin Gilbert 5/11 ########//
	report_disp_frame_no = 0;		// 0 - mean report can be drawn, clear after disp_zoom, set after display report
	// ###### end Gilbert 5/11 ########//

	//---- if in report mode, convert the view to gray scale ----//

	if( view_mode!=MODE_NORMAL )
	{
		// ###### begin Gilbert 5/11 ########//
		// report_disp_frame_no = frame_count;		// the frame no which this report is first displayed
		// ###### end Gilbert 5/11 ########//

		//------- blt the zoom area to the front screen --------//

		vga.use_back();
		Vga::opaque_flag = config.opaque_report;

		switch( view_mode )
		{
			case MODE_TRADE:
				info.disp_trade(INFO_REPAINT);
				break;

			case MODE_MILITARY:
				info.disp_military(INFO_REPAINT);
				break;

			case MODE_ECONOMY:
				info.disp_economy(INFO_REPAINT);
				break;

			case MODE_TOWN:
				info.disp_town(INFO_REPAINT);
				break;

			case MODE_NATION:
				info.disp_nation(INFO_REPAINT);
				break;

			case MODE_TECH:
				info.disp_tech(INFO_REPAINT);
				break;

			case MODE_SPY:
				info.disp_spy(INFO_REPAINT);
				break;

			case MODE_RANK:
				info.disp_rank(INFO_REPAINT);
				break;

			case MODE_NEWS_LOG:
				info.disp_news_log(INFO_REPAINT);
				break;

			case MODE_AI_ACTION:
				info.disp_ai_action(INFO_REPAINT);
				break;
		}

		vga.use_front();
		Vga::opaque_flag = 0;

		// ###### begin Gilbert 5/11 ########//
		report_disp_frame_no = 1;
		// ###### end Gilbert 5/11 ########//
	}
}
//-------- End of function Sys::disp_view --------//


//-------- Begin of function Sys::update_view --------//
//
// Display the view area.
//
void Sys::update_view()
{
	if( view_mode==MODE_NORMAL )
	{
		disp_zoom();
		// ####### begin Gilbert 5/11 #######//
		report_disp_frame_no = 0;
		// ####### end Gilbert 5/11 #######//

		//------ display tutorial text -------//

		if( game.game_mode == GAME_TUTORIAL )
			tutor.disp();

		//----------- draw profile information -----------//

		if( config.show_ai_info )
		{
			vga.use_back();
/*
			char* germanStr = "d ü    ä    ß    ö    Ä    Ü    Ö";

			vga_back.bar( ZOOM_X1, ZOOM_Y1, ZOOM_X1+300, ZOOM_Y1+150, VGA_LIGHT_GREEN );

			font_san.put( ZOOM_X1+10, ZOOM_Y1+30, germanStr );
			font_news.put( ZOOM_X1+10, ZOOM_Y1+50, germanStr );
			font_bible.put( ZOOM_X1+10, ZOOM_Y1+70, germanStr );
*/
			nation_array.draw_profile();
			firm_array.draw_profile();
			town_array.draw_profile();
			unit_array.draw_profile();

			vga.use_front();
		}

		if( in_game_menu.is_active() )
		{
			vga.use_back();
			in_game_menu.disp();
			vga.use_front();
		}

		//------------------------------------//

		vga_util.blt_buf(ZOOM_X1, ZOOM_Y1, ZOOM_X2, ZOOM_Y2);
	}
	else
	{
		//-------------------------------------------//
		//
		// In report mode, display the background view in odd
		// number frames and the report in even number frames.
		//
		//-------------------------------------------//

		// ####### begin Gilbert 5/11 #######//
		if( report_disp_frame_no )
		{
			disp_zoom();
			report_disp_frame_no = 0;
		}
		else
		{
			vga.use_back();
			Vga::opaque_flag = config.opaque_report;

			switch( view_mode )
			{
				case MODE_TRADE:
					info.disp_trade(INFO_UPDATE);
					break;

				case MODE_MILITARY:
					info.disp_military(INFO_UPDATE);
					break;

				case MODE_ECONOMY:
					info.disp_economy(INFO_UPDATE);
					break;

				case MODE_TOWN:
					info.disp_town(INFO_UPDATE);
					break;

				case MODE_NATION:
					info.disp_nation(INFO_UPDATE);
					break;

				case MODE_TECH:
					info.disp_tech(INFO_UPDATE);
					break;

				case MODE_SPY:
					info.disp_spy(INFO_UPDATE);
					break;

				case MODE_RANK:
					info.disp_rank(INFO_UPDATE);
					break;

				case MODE_NEWS_LOG:
					info.disp_news_log(INFO_UPDATE);
					break;

				case MODE_AI_ACTION:
					info.disp_ai_action(INFO_UPDATE);
					break;
			}

			if( in_game_menu.is_active() )
			{
				in_game_menu.disp();
			}

			vga.use_front();
			Vga::opaque_flag = 0;

			vga_util.blt_buf(ZOOM_X1, ZOOM_Y1, ZOOM_X2, ZOOM_Y2);

			// ###### begin Gilbert 5/11 #######//
			report_disp_frame_no = 1;
			// ###### end Gilbert 5/11 #######//
		}
	}
}
//-------- End of function Sys::update_view --------//


//-------- Begin of function Sys::detect_view --------//
//
void Sys::detect_view()
{
	int enableAction;			// some action is not enabled, when paused.
	enableAction = config.frame_speed > 0 || !remote.is_enable();	// allow action when paused in single player

	if( enableAction )
		info.detect();

	vga.use_back();

	switch( view_mode )
	{
		case MODE_TRADE:
			info.detect_trade();
			break;

		case MODE_MILITARY:
			info.detect_military();
			break;

		case MODE_ECONOMY:
			info.detect_economy();
			break;

		case MODE_TOWN:
			info.detect_town();
			break;

		case MODE_NATION:
			info.detect_nation();
			break;

		case MODE_TECH:
			info.detect_tech();
			break;

		case MODE_SPY:
			info.detect_spy();
			break;

		case MODE_RANK:
			info.detect_rank();
			break;

		case MODE_NEWS_LOG:
			info.detect_news_log();
			break;

		case MODE_AI_ACTION:
			info.detect_ai_action();
			break;
	}

	vga.use_front();

	//------ detect tutorial controls -------//

	if( view_mode==MODE_NORMAL && game.game_mode==GAME_TUTORIAL )		// tutorial text is only displayed in non-report mode
	{
		if( tutor.detect() )
			return;
	}

	//---- no normal news when the game is displaying the news log ----//

	// ##### patch begin Gilbert 31/3 #####//
	if( news_array.detect() )
		return;
	// ##### patch end Gilbert 31/3 #####//

	//---- pressing right button in command mode -> cancel command mode ----//

	if( mouse.any_click(RIGHT_BUTTON) && power.command_id )
	{
		power.command_id = 0;
		mouse.reset_click();
		info.disp();
		return;
	}

	//------ detect selecting objects and laying tracks ------//

	//-------- detect world ----------//

	if( world.detect() )
		return;

	if( view_mode == MODE_NORMAL )
	{
		if( world.detect_firm_town() )
			return;

		//------ detect selecting objects and laying tracks ------//

		if( power.detect_frame() )
			return;
	}
	else
	{
		mouse_cursor.set_frame(0);
	}

	//----------- detect action ------------//

	if( enableAction && power.detect_action() )
	{
		if(unit_array.selected_recno && se_res.mark_command_time() )
		{
			Unit *unitPtr = unit_array[unit_array.selected_recno];
			se_res.far_sound( unitPtr->cur_x_loc(), unitPtr->cur_y_loc(), 1, 'S',
				unitPtr->sprite_id, "ACK");
		}
		return;
	}

	//----- detect right mouse button to select defined unit groups -----//

	if( !(mouse.event_skey_state & SHIFT_KEY_MASK) &&
		mouse.any_click(ZOOM_X1, ZOOM_Y1, ZOOM_X2, ZOOM_Y2, RIGHT_BUTTON) &&
		power.detect_select(mouse.click_x(RIGHT_BUTTON), mouse.click_y(RIGHT_BUTTON), 
			mouse.click_x(RIGHT_BUTTON),mouse.click_y(RIGHT_BUTTON), 1, 0) )		// 1 - recall group
	{
		return;
	}

	//-------- detect world ----------//

	// world.detect();
}
//-------- End of function Sys::detect_view --------//


//-------- Begin of function Sys::disp_map --------//

void Sys::disp_map()
{
	//------ draw and display the map -------//

	if( map_need_redraw )		// requested by other modules to redraw the pre-drawn map background
	{
		world.map_matrix->draw();
		map_need_redraw = 0;
	}

	world.map_matrix->disp();

	//-------- draw dots on the map ---------//

	firm_array.draw_dot();
	town_array.draw_dot();
	site_array.draw_dot();
	unit_array.draw_dot();
	war_point_array.draw_dot();
	tornado_array.draw_dot();

	world.map_matrix->draw_square();		// draw a square on the map for current zoomed area

	//------- blt the map area to the front screen --------//

	vga_util.blt_buf( MAP_X1, MAP_Y1 , MAP_X2 , MAP_Y2);
}
//-------- End of function Sys::disp_map --------//


//-------- Begin of function Sys::disp_zoom --------//

void Sys::disp_zoom()
{
	//--------- set zoom window ----------//

	ZoomMatrix* zoomMatrix = world.zoom_matrix;

	err_when(zoomMatrix->top_x_loc<0 || zoomMatrix->top_x_loc>=MAX_WORLD_X_LOC);
	err_when(zoomMatrix->top_y_loc<0 || zoomMatrix->top_y_loc>=MAX_WORLD_Y_LOC);

	World::view_top_x = zoomMatrix->top_x_loc * ZOOM_LOC_WIDTH;
	World::view_top_y = zoomMatrix->top_y_loc * ZOOM_LOC_HEIGHT;

	//--------- draw map area ---------//

	if( zoom_need_redraw )		// requested by other modules to redraw the pre-drawn zoom background
	{
		long backupSeed = misc.get_random_seed();
		
		world.zoom_matrix->draw();
		zoom_need_redraw = 0;
	}

	//-------- disp zoom area --------//

	world.zoom_matrix->disp();

	//---- draw sprite white sites if in debug mode ----//

	#ifdef DEBUG
	if(debug2_enable_flag)
		world.zoom_matrix->draw_white_site();
	#endif

	//------- draw foreground objects --------//

	world.zoom_matrix->draw_frame();

	//----- draw the frame of the selected firm/town -----//

	info.draw_selected();

	//-------- display news messages ---------//

	news_array.disp();

	//----- next frame, increase the frame counter -----//

	sys.frames_in_this_second++;		// no. of frames displayed in this second

	if( view_mode==MODE_NORMAL )
	{
		disp_frames_per_second();

		if( (remote.is_enable() || remote.is_replay()) && (remote.sync_test_level & 0x40) )
		{
			// Warn user we are out of sync
			vga.use_back();

			if( !(remote.sync_test_level & 1) )
				font_news.disp( ZOOM_X1+10, ZOOM_Y1+30, _("Multiplayer Random Seed Sync Error"), MAP_X2 );
			else if( !(remote.sync_test_level & 2) )
				font_news.disp( ZOOM_X1+10, ZOOM_Y1+30, _("Multiplayer Object Sync Error"), MAP_X2 );

			vga.use_front();
		}
	}
}
//-------- End of function Sys::disp_zoom --------//


//-------- Begin of function Sys::blt_virtual_buf --------//
//
void Sys::blt_virtual_buf()
{
	if( !sys.debug_session )
		return;

	//--- in a debug sesion, vga_front is not the true front buffer, now copy it to the true one ---//

	int frontLocked=0;

	if( vga_front.buf_locked )
	{
		vga_front.unlock_buf();
		frontLocked=1;
	}

	vga_true_front.blt_virtual_buf( &vga_front );

	if( frontLocked )
		vga_front.lock_buf();
}
//--------- End of function Sys::blt_virtual_buf ---------//


//-------- Begin of function Sys::disp_frames_per_second --------//
//
void Sys::disp_frames_per_second()
{
	if( !config.show_ai_info && !sys.disp_fps_flag )// only display this in a debug session
		return;

	if( game.game_mode == GAME_TUTORIAL )		// don't display in tutorial mode as it overlaps with the tutorial text
		return;

	//------- get the curren system time ---------//

	unsigned long curTime = misc.get_time();		// in millisecond

	//----------- first time calling -------------//

	if( last_second_time==0 )
	{
		last_second_time  = curTime;
		frames_in_this_second = 0;		// no. of frames displayed in this second
		return;
	}

	//------ when passes to the next second -----//

	if( curTime >= last_second_time+1000 )  // 1 second = 1000 millisecond
	{
		frames_per_second = frames_in_this_second;

		//------ update var and reset counter -----//

		last_second_time += 1000;
		frames_in_this_second = 0;
	}

	//---------- display frame count -----------//

	String str;

	str  = "Frames per second: ";
	str += frames_per_second;

	if( frames_per_second < config.frame_speed-1 ) // -1 for rounding
	{
		str += " (expecting ";
		str += config.frame_speed;
		str += ")";
	}

	vga.use_back();

	font_news.disp( ZOOM_X1+10, ZOOM_Y1+10, str, MAP_X2);

	vga.use_front();
}
//--------- End of function Sys::disp_frames_per_second ---------//


//--------- Begin of funtion Sys::disp_view_mode ---------//
// <int> observeMode		- force observe mode display (darken view mode 1 - 7)
//									needed in Game::game_end, nation_array.player_recno has not yet set to 0
void Sys::disp_view_mode(int observeMode)
{
	// ------- display highlight ----------//
 	const int MIN_MODE_TO_DISPLAY = 1;
	const int MAX_MODE_TO_DISPLAY = 8;
	const int MODE_TO_DISPLAY_COUNT = MAX_MODE_TO_DISPLAY - MIN_MODE_TO_DISPLAY + 1;
	static short highLightX[MODE_TO_DISPLAY_COUNT] = {  0,  62, 124, 186,  7,  68, 129, 192};
	static short highLightY[MODE_TO_DISPLAY_COUNT] = {  0,   0,   0,   0, 19,  19,  19,  19};
	static short darkenX[MODE_TO_DISPLAY_COUNT] = {  7,  69, 132, 195,  13,  75, 139, 201};
	static short darkenY[MODE_TO_DISPLAY_COUNT] = {  8,   8,   8,   8,  29,  29,  29,  29};
	const int darkenWidth = 58;
	const int darkenHeight = 16;
	char scrollName[] = "SCROLL-0";

	// disable highlight of the mode before
	scrollName[7] = 'B';
	image_button.put_front( 0,0, scrollName);

	// highlight of the mode after
	if( view_mode >= MIN_MODE_TO_DISPLAY && view_mode <= MAX_MODE_TO_DISPLAY )
	{
		// find the size of that scroll
		scrollName[7] = '0' + view_mode;
		image_button.put_front( highLightX[view_mode-MIN_MODE_TO_DISPLAY], 
			highLightY[view_mode-MIN_MODE_TO_DISPLAY], scrollName);
	}

	// darken buttons of view mode 1-7 if nation_array.player_recno == 0
	if( observeMode || !nation_array.player_recno )
	{
		for( int j = 1; j <= 7; ++j )
		{
			//if( j == view_mode)
			//	continue;

			vga_front.adjust_brightness(
				darkenX[j-MIN_MODE_TO_DISPLAY], darkenY[j-MIN_MODE_TO_DISPLAY],
				darkenX[j-MIN_MODE_TO_DISPLAY]+darkenWidth-1, 
				darkenY[j-MIN_MODE_TO_DISPLAY]+darkenHeight-1, -8 );
		}
	}

	// darken view mode 8 if there are no nations to rank
	if( !nation_array.nation_count )
	{
			vga_front.adjust_brightness(
				darkenX[8-MIN_MODE_TO_DISPLAY], darkenY[8-MIN_MODE_TO_DISPLAY],
				darkenX[8-MIN_MODE_TO_DISPLAY]+darkenWidth-1,
				darkenY[8-MIN_MODE_TO_DISPLAY]+darkenHeight-1, -8 );
	}
}
//--------- End of funtion Sys::disp_view_mode ---------//
