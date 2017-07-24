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

//Filename    : OUNITIF.CPP
//Description : Unit Interface Routines

#include <KEY.h>
#include <OVGA.h>
#include <vga_util.h>
#include <OMOUSE.h>
#include <OFONT.h>
#include <OMOUSE.h>
#include <OHELP.h>
#include <OINFO.h>
#include <OFIRMRES.h>
#include <OIMGRES.h>
#include <OBUTTON.h>
#include <OBUTT3D.h>
#include <OPOWER.h>
#include <OSPY.h>
#include <OSYS.h>
#include <OGAME.h>
#include <ONATION.h>
#include <OWORLD.h>
#include <OU_VEHI.h>
#include <OREMOTE.h>
#include <OSE.h>
#include <OSERES.h>
#include <OBUTTCUS.h>

#include <OU_MARI.h>
#include "gettext.h"

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

//---------- Define constant ------------//

enum { UNIT_MENU_MAIN,
		 UNIT_MENU_BUILD,
	  };

#define BURN_COMBAT_LEVEL		80

#define SPY_CLOAK_WIDTH			21

//---------- Define static variables ----------//

static Button3D 		button_build;
static ButtonCustom 	button_build_array[MAX_FIRM_TYPE];
static char 			button_build_flag[MAX_FIRM_TYPE];
static Button3D 		button_settle;
static Button3D 		button_assign;
static Button3D 		button_promote, button_demote;
static Button3D 		button_reward;
static Button3D 		button_return_camp;
static Button3D 		button_cancel;
static Button   		button_cancel2;
static Button3D 		button_resign;
static Button3D 		button_succeed_king;
static Button3D 		button_spy_notify;
static Button3D 		button_spy_drop_identity;
static Button3D		button_aggressive_mode;
static Button   		button_change_color;

static short unit_menu_mode=UNIT_MENU_MAIN, last_unit_recno=0;
// ##### begin Gilbert 3/10 #######//
static short build_firm_button_order[MAX_FIRM_TYPE] =
{
	FIRM_CAMP,
	FIRM_MINE,
	FIRM_FACTORY,
	FIRM_MARKET,
	FIRM_RESEARCH,
	FIRM_WAR_FACTORY,
	FIRM_INN,
	FIRM_HARBOR,
	FIRM_BASE,
	FIRM_MONSTER,
};
// ##### end Gilbert 3/10 #######//

static char button_build_hotkey[MAX_FIRM_TYPE] =
{
	'F', // fort
	'R', // mine (raw)
	'A', // factory
	'M', // market
	'T', // tower of science
	'W', // war factory
	'I', // inn
	'H', // harbour
	'P', // seat of power
	 0, // monster
};


//--------- Declare static functions ---------//

static void disp_debug_info(Unit* unitPtr, int dispY1, int refreshFlag);
static void disp_firm_button(ButtonCustom *, int);

static void group_resign();
static void group_change_aggressive_mode();
static void group_reward();
static void group_change_spy_notify_flag();
static void group_drop_spy_identity();

//--------- Begin of function Unit::disp_info ---------//
//
void Unit::disp_info(int refreshFlag)
{
	if( sprite_recno != last_unit_recno )
	{
		unit_menu_mode  = UNIT_MENU_MAIN;
		last_unit_recno = sprite_recno;
	}

	switch( unit_menu_mode )
	{
		case UNIT_MENU_MAIN:
			switch( power.command_id )
			{
				case COMMAND_BUILD_FIRM:
					disp_build(refreshFlag);
					break;

				case COMMAND_SETTLE:
					disp_settle(refreshFlag);
					break;

				default:
					disp_main_menu(refreshFlag);
			}
			break;

		case UNIT_MENU_BUILD:
			disp_build_menu(refreshFlag);
			break;
	}
}
//----------- End of function Unit::disp_info -----------//


//--------- Begin of function Unit::detect_info ---------//
//
void Unit::detect_info()
{
	switch( unit_menu_mode )
	{
		case UNIT_MENU_MAIN:
			switch( power.command_id )
			{
				case COMMAND_BUILD_FIRM:
					detect_build();
					break;

				case COMMAND_SETTLE:
					detect_settle();
					break;

				default:
					detect_main_menu();
			}
			break;

		case UNIT_MENU_BUILD:
			detect_build_menu();
			break;
	}
}
//----------- End of function Unit::detect_info -----------//


//--------- Begin of function Unit::is_in_build_menu ---------//
// Returns true if a unit is currently in build mode.
// Only reliable if this unit is the selected unit.
// Used by Info to detect if the build mode is opened.
//
bool Unit::is_in_build_menu()
{
	return unit_menu_mode == UNIT_MENU_BUILD;
}
//----------- End of function Unit::is_in_build_menu -----------//


//--------- Begin of function Unit::disp_main_menu ---------//
//
void Unit::disp_main_menu(int refreshFlag)
{
	disp_basic_info(INFO_Y1, refreshFlag);
	disp_unit_profile( INFO_Y1+52, refreshFlag );

	if( !should_show_info() )
		return;

	//---------------------------------//

	int y=INFO_Y1+98;

	if( race_id )
	{
		disp_unit_info( y, refreshFlag );
		y += 89;
	}

	//---------------------------------//

	if( is_own_spy() )
	{
		disp_spy_menu(y, refreshFlag);
		y+=spy_menu_height()+3;
	}

	if( is_own() )
	{
		if( refreshFlag == INFO_REPAINT )
			disp_button(y);

		y+=BUTTON_ACTION_HEIGHT;
	}

	#ifdef DEBUG
		if( sys.debug_session || sys.testing_session )
			disp_debug_info(this, INFO_Y2-68, refreshFlag);
	#endif
}
//----------- End of function Unit::disp_main_menu -----------//


//-------- Begin of function Unit::should_show_info ------//
//
int Unit::should_show_info()
{
	if( config.show_ai_info || is_own() )
		return 1;

	//---- if there is a phoenix of the player over this firm ----//

	if( nation_array.player_recno &&
		 (~nation_array)->revealed_by_phoenix(next_x_loc(), next_y_loc()) )
	{
		return 1;
	}

	return 0;
}
//---------- End of function Unit::should_show_info --------//


//--------- Begin of function Unit::detect_main_menu ---------//
//
void Unit::detect_main_menu()
{
	if( detect_basic_info() )
		return;

	if( detect_unit_profile() )
		return;

	if( is_own_spy() )
		detect_spy_menu(INFO_Y1+187);

	if( is_own() )
		detect_button();
}
//----------- End of function Unit::detect_main_menu -----------//


//--------- Begin of function Unit::disp_basic_info ---------//
//
void Unit::disp_basic_info(int dispY1, int refreshFlag)
{
	//------- display the name of the unit --------//

	if( refreshFlag == INFO_REPAINT )
	{
		int dispName = unit_id == UNIT_CARAVAN || unit_res[unit_id]->unit_class == UNIT_CLASS_SHIP;		// only display names for caravans and ships as their names are not displayed in the other part of the interface

		vga_util.d3_panel_up( INFO_X1, dispY1, INFO_X2, dispY1+21 );

		if( nation_recno )
		{
			int dispTitle = is_own() || nation_recno==0;		// only display title for own units or independent units (which is for rebel leader only)

			if( dispName )
				font_san.center_put( INFO_X1+21, dispY1, INFO_X2-2, dispY1+21, unit_name(dispTitle) );

			vga_util.d3_panel_down( INFO_X1+3, dispY1+3, INFO_X1+20, dispY1+18 );

			//------- display the nation color box -------//

			char *nationPict = image_button.get_ptr("V_COLCOD");

			vga_front.put_bitmap_remap(INFO_X1+3, dispY1+2, nationPict, game.get_color_remap_table(nation_recno, 0) );
		}
		else
		{
			if( dispName )
				font_san.center_put( INFO_X1, dispY1, INFO_X2-2, dispY1+21, unit_name() );
		}
	}

	dispY1+=23;

	//-------- display the hit points bar --------//

	if( refreshFlag == INFO_REPAINT )
	{
		vga_util.d3_panel_up( INFO_X1, dispY1, INFO_X2, dispY1+27 );

		if( nation_recno == nation_array.player_recno &&
			 can_resign())
		{
			button_resign.paint( INFO_X1+4, dispY1+1, "V_X-U", "V_X-D" );
			button_resign.set_help_code( "RESIGN" );
		}
	}

	//-------- display hit points in numbers --------//

	disp_hit_point(dispY1);
}
//----------- End of function Unit::disp_basic_info -----------//


//--------- Begin of function Unit::detect_basic_info ---------//
//
int Unit::detect_basic_info()
{
	//--- detect pressing on the name to center the unit on the screen ---//

	if( is_visible() && mouse.single_click( INFO_X1, INFO_Y1, INFO_X2, INFO_Y1+21 ) )
	{
		world.go_loc( next_x_loc(), next_y_loc() );
		return 1;
	}

	//-------- detect resign button ----------//

	// ###### patch begin Alex 10/1 ########//
	// if( is_own() && rank_id != RANK_KING )
	if( nation_recno == nation_array.player_recno && rank_id != RANK_KING )
	{
		if( button_resign.detect(KEY_DEL) )
		{
			group_resign();
			return 1;
		}
	}
	// ###### patch end Alex 10/1 ########//

	return 0;
}
//----------- End of function Unit::detect_basic_info -----------//


//--------- Begin of function Unit::disp_button ---------//
//
void Unit::disp_button(int dispY1)
{
	int x=INFO_X1;

	//---- if currently in the mode of selecting a unit to succeed the king ----//

	if( nation_array.player_recno &&
		 nation_recno == nation_array.player_recno &&
		 (~nation_array)->king_unit_recno == 0 )
	{
		if( race_id )
			button_succeed_king.paint( x, dispY1, 'A', "SUCCEED" );

		return;
	}

	//------- display aggressive mode button ------//

	button_aggressive_mode.paint( x, dispY1, 'A', aggressive_mode ? (char*)"AGGRESS1" : (char*)"AGGRESS0" );
	x += BUTTON_ACTION_WIDTH;

	//---------- only for human units ---------//

	// Reset all buttons, and activate them as-needed
	button_build.reset();
	button_settle.reset();
	button_promote.reset();
	button_demote.reset();
	button_reward.reset();
	button_return_camp.reset();
	// button_burn.reset();
	// button_assign.reset();

	if( unit_res[unit_id]->unit_class == UNIT_CLASS_HUMAN && race_id )
	{
		int firmId;
		for( firmId=1; firmId<=MAX_FIRM_TYPE ; firmId++ )
		{
			if( firm_res[firmId]->can_build(sprite_recno) )
				break;
		}

		if( firmId<=MAX_FIRM_TYPE &&
			 nation_recno == nation_array.player_recno ) 	// a spy cannot build structure for another nation
		{
			button_build.paint( x, dispY1, 'A', "BUILD" );
			x += BUTTON_ACTION_WIDTH;
		}

		//-------- settle button ----------//

		if( mobile_type==UNIT_LAND && rank_id != RANK_KING )
		{
			button_settle.paint( x, dispY1, 'A', "SETTLE" );
			x += BUTTON_ACTION_WIDTH;
		}

		//-------- promote/demote button --------//


		if( nation_recno == nation_array.player_recno )		// you can't promote your spy in other nation
		{
			if( rank_id==RANK_SOLDIER && skill.skill_id==SKILL_LEADING )
			{
				if(unit_array.selected_count==1)
				{
					button_promote.paint( x, dispY1, 'A', "PROMOTE" );
					x += BUTTON_ACTION_WIDTH;
				}
			}
			else if( rank_id == RANK_GENERAL )
			{
				if( unit_array.selected_count==1 )
				{
					button_demote.paint( x, dispY1, 'A', "DEMOTE"  );
					x += BUTTON_ACTION_WIDTH;
				}
			}
		}

		if( x+BUTTON_ACTION_WIDTH-5 > INFO_X2 )
		{
			x  = INFO_X1;
			dispY1 += BUTTON_ACTION_HEIGHT;
		}

		//------------ "reward" button ---------//

		if( nation_array.player_recno && is_own() &&	// Can only reward if the player is still alive. Can reward own spies (even when cloaked).
			 rank_id != RANK_KING )
		{
			button_reward.paint( x, dispY1, 'A', "REWARD" );
			x += BUTTON_ACTION_WIDTH;

			if( x+BUTTON_ACTION_WIDTH-5 > INFO_X2 )
			{
				x  = INFO_X1;
				dispY1 += BUTTON_ACTION_HEIGHT;
			}
		}

		/*
		//-------- burn button ----------//

		if( skill.combat_level > BURN_COMBAT_LEVEL && mobile_type==UNIT_LAND  )
		{
			button_burn.paint_text( x, dispY1, "Burn" );
			x += 60;
		}

		//-------- assign to firm button --------//

		if( mobile_type==UNIT_LAND )
		{
			button_assign.paint_text( x, dispY1, "Assign" );
			x+=60;
		}
		*/
	}

	//------ "Return Camp" button -------//

	if( home_camp_firm_recno &&
		 (unit_res[unit_id]->unit_class == UNIT_CLASS_HUMAN || unit_res[unit_id]->unit_class == UNIT_CLASS_WEAPON) &&
		 firm_array[home_camp_firm_recno]->region_id == region_id() )
	{
		button_return_camp.paint( x, dispY1, 'A', "RETCAMP" );
		x += BUTTON_ACTION_WIDTH;

		if( x+BUTTON_ACTION_WIDTH-5 > INFO_X2 )
		{
			x  = INFO_X1;
			dispY1 += BUTTON_ACTION_HEIGHT;
		}
	}

	//------- spy notify button ------//

	if( spy_recno && true_nation_recno() == nation_array.player_recno )
	{
		int notifyFlag = spy_array[spy_recno]->notify_cloaked_nation_flag;

		button_spy_notify.paint( x, dispY1, 'A', notifyFlag ? (char*)"SPYNOTI1" : (char*)"SPYNOTI0" );
		x += BUTTON_ACTION_WIDTH;

		if( x+BUTTON_ACTION_WIDTH-5 > INFO_X2 )
		{
			x  = INFO_X1;
			dispY1 += BUTTON_ACTION_HEIGHT;
		}

		button_spy_drop_identity.paint( x, dispY1, 'A', "NOSPY" );
	}
	else
	{
		button_spy_notify.reset();
		button_spy_drop_identity.reset();
	}

	//---- display button for changing nation color scheme ----//

	if( sys.debug_session )
		button_change_color.paint_text( INFO_X1, INFO_Y2-20, "Change Nation Color" );
}
//----------- End of function Unit::disp_button -----------//


//--------- Begin of function Unit::detect_button ---------//
//
void Unit::detect_button()
{
	//---- if currently in the mode of selecting a unit to succeed the king ----//

	if( nation_array.player_recno &&
		 nation_recno == nation_array.player_recno &&
		 (~nation_array)->king_unit_recno == 0 )
	{
		if( race_id && button_succeed_king.detect() )
		{
			if( !remote.is_enable() )
			{
				(~nation_array)->succeed_king(sprite_recno);
				info.disp();
			}
			else
			{
				// packet structure : <unit recno> <nation recno>
				short *shortPtr = (short *)remote.new_send_queue_msg(MSG_UNIT_SUCCEED_KING, 2*sizeof(short));
				*shortPtr = sprite_recno;
				shortPtr[1] = nation_array.player_recno;
			}
		}

		return;
	}

	//--------- "return camp" button ---------//

	if( home_camp_firm_recno && button_return_camp.detect('R') )
	{
		// sound effect
		se_res.far_sound(next_x_loc(), next_y_loc(), 1, 'S', sprite_id, "ACK");

		unit_array.return_camp(COMMAND_PLAYER);
		return;
	}

	//------- toggle unit aggressive mode button ------//

	if( button_aggressive_mode.detect() )
	{
		group_change_aggressive_mode();
	}

	//-------- build button --------//

	if( button_build.detect('B') )
	{
		unit_menu_mode = UNIT_MENU_BUILD;
		info.disp();
	}

	//-------- settle button ---------//

	if( button_settle.detect('T') )
	{
		power.issue_command(COMMAND_SETTLE, sprite_recno);
		info.disp();
		return;
	}

	//-------- detect promote/demote button --------//

	if( rank_id == RANK_SOLDIER )
	{
		if( button_promote.detect() )
		{
			if(!remote.is_enable() )
			{
				set_rank(RANK_GENERAL);
			}
			else
			{
				// packet structure : <unit recno> <new rank>
				short *shortPtr = (short *)remote.new_send_queue_msg(MSG_UNIT_SET_RANK, 2*sizeof(short));
				*shortPtr = sprite_recno;
				shortPtr[1] = RANK_GENERAL;
			}

			se_ctrl.immediate_sound("TURN_ON");
		}
	}
	else if( rank_id == RANK_GENERAL )
	{
		if( button_demote.detect() )
		{
			if( !remote.is_enable() )
			{
				set_rank(RANK_SOLDIER);
			}
			else
			{
				// packet structure : <unit recno> <new rank>
				short *shortPtr = (short *)remote.new_send_queue_msg(MSG_UNIT_SET_RANK, 2*sizeof(short));
				*shortPtr = sprite_recno;
				shortPtr[1] = RANK_SOLDIER;
			}

			se_ctrl.immediate_sound("TURN_OFF");
		}
	}

	//------------ detect assign to firm button ------------//

	if( button_assign.detect() )
	{
		power.issue_command(COMMAND_ASSIGN, sprite_recno);
	}

	//------------ "reward" button ---------//

	if( button_reward.detect() )
	{
		group_reward();
	}

	//------- spy notify button ------//

	if( spy_recno && button_spy_notify.detect() )
	{
		group_change_spy_notify_flag();
	}

	//------- drop spy identity button ------//

	if( spy_recno && button_spy_drop_identity.detect() )
	{
		group_drop_spy_identity();
	}

/*
	//--------- detect button for changing nation color -------//

	if( button_change_color.detect() )
	{
		Nation* nationPtr = nation_array[nation_recno];

		if( ++nationPtr->color_scheme_id > MAX_COLOR_SCHEME )
			nationPtr->color_scheme_id = 1;

		nationPtr->nation_color	= game.color_remap_array[nationPtr->color_scheme_id-1].main_color;
	}
*/
}
//----------- End of function Unit::detect_button -----------//


//----- Begin of static function Unit::can_resign -----//
int Unit::can_resign()
{
  return rank_id != RANK_KING;
}
//------ End of static function Unit::can_resign ----//


//----- Begin of static function group_change_aggressive_mode -----//
//
static void group_change_aggressive_mode()
{
	Unit* unitPtr = unit_array[unit_array.selected_recno];
	int newAggressiveMode = !unitPtr->aggressive_mode;

	//------ group chaning spy notify flag -------//

	for( int i=unit_array.size() ; i>0 ; i-- )
	{
		if( unit_array.is_deleted(i) )
			continue;

		unitPtr = unit_array[i];

		//------ if this is a player spy --------//

		if( unitPtr->selected_flag && unitPtr->is_own() )
		{
			if( !remote.is_enable() )
			{
				unitPtr->aggressive_mode = newAggressiveMode;
			}
			else
			{
				// packet structure : <unit no> <new aggressive mode>
				short *shortPtr = (short *)remote.new_send_queue_msg(MSG_UNIT_CHANGE_AGGRESSIVE_MODE, sizeof(short)*2);
				*shortPtr = i;
				shortPtr[1] = newAggressiveMode;
			}
		}
	}

	if( newAggressiveMode )
		se_ctrl.immediate_sound("TURN_ON");
	else
		se_ctrl.immediate_sound("TURN_OFF");

	button_aggressive_mode.update_bitmap( newAggressiveMode ? (char*)"AGGRESS1" : (char*)"AGGRESS0" );
}
//------ End of static function group_change_aggressive_mode ----//


//----- Begin of static function group_resign -----//
//
static void group_resign()
{
	Unit* unitPtr;

	//------ group chaning spy notify flag -------//

	for( int i=unit_array.size() ; i>0 ; i-- )
	{
		if( unit_array.is_deleted(i) )
			continue;

		unitPtr = unit_array[i];

		//------ if this is a player spy --------//

		if( unitPtr->selected_flag && unitPtr->is_own() && unitPtr->can_resign())
			unitPtr->resign(COMMAND_PLAYER);
	}
}
//------ End of static function group_resign ----//


//----- Begin of static function group_reward -----//
//
static void group_reward()
{
	Unit* unitPtr;

	//------ group rewarding -------//

	for( int i=unit_array.size() ; i>0 ; i-- )
	{
		if( unit_array.is_deleted(i) )
			continue;

		unitPtr = unit_array[i];

		//------ if this is a player unit (and not a weapon) --------//

		if( unitPtr->selected_flag && unitPtr->race_id && unitPtr->is_own() )
		{
			if( !remote.is_enable() )
				unitPtr->reward(nation_array.player_recno);
			else
			{
				// packet structure : <unit no> + <rewarding nation recno>
				short *shortPtr = (short *)remote.new_send_queue_msg(MSG_UNIT_REWARD,sizeof(short)*2);
				*shortPtr = i;
				shortPtr[1] = nation_array.player_recno;
			}
		}
	}

	se_ctrl.immediate_sound("TURN_ON");
}
//------ End of static function group_reward ----//


//----- Begin of static function group_change_spy_notify_flag -----//
//
static void group_change_spy_notify_flag()
{
	Unit* unitPtr = unit_array[unit_array.selected_recno];
	Spy* spyPtr = spy_array[ unitPtr->spy_recno ];
	char newNotifyFlag = !spyPtr->notify_cloaked_nation_flag;

	//------ group chaning spy notify flag -------//

	for( int i=unit_array.size() ; i>0 ; i-- )
	{
		if( unit_array.is_deleted(i) )
			continue;

		unitPtr = unit_array[i];

		//------ if this is a player spy --------//

		if( unitPtr->selected_flag && unitPtr->is_own_spy() )
		{
			if( !remote.is_enable() )
			{
				spy_array[unitPtr->spy_recno]->notify_cloaked_nation_flag = newNotifyFlag;
			}
			else
			{
				// packet structure : <spy recno> <new notify_cloaked_nation_flag>
				short *shortPtr = (short *)remote.new_send_queue_msg(MSG_SPY_CHANGE_NOTIFY_FLAG, sizeof(short)*2);
				*shortPtr = unitPtr->spy_recno;
				shortPtr[1] = newNotifyFlag;
			}
		}
	}

	//--------- update the spy bitmap ----------//

	button_spy_notify.update_bitmap( newNotifyFlag ? (char*)"SPYNOTI1" : (char*)"SPYNOTI0" );

	if( newNotifyFlag )
		se_ctrl.immediate_sound("TURN_ON");
	else
		se_ctrl.immediate_sound("TURN_OFF");
}
//------ End of static function group_change_spy_notify_flag ----//


//------ Begin of static function group_drop_spy_identity -------//
//
static void group_drop_spy_identity()
{
	Unit* unitPtr;

	//------ group drop spy identity -------//

	for( int i=unit_array.size() ; i>0 ; i-- )
	{
		if( unit_array.is_deleted(i) )
			continue;

		unitPtr = unit_array[i];

		//------ if this is a player spy --------//

		if( unitPtr->selected_flag && unitPtr->is_own_spy() )
		{
			if( !remote.is_enable() )
			{
				spy_array[unitPtr->spy_recno]->drop_spy_identity();
			}
			else
			{
				// packet structure : <spy recno>
				short *shortPtr = (short *)remote.new_send_queue_msg(MSG_SPY_DROP_IDENTITY, sizeof(short));
				shortPtr[0] = unitPtr->spy_recno;
			}
		}
	}

	se_ctrl.immediate_sound("TURN_OFF");
}
//------- End of static function group_drop_spy_identity -------//


//--------- Begin of function Unit::disp_build_menu ---------//
//
void Unit::disp_build_menu(int refreshFlag)
{
	//---------- paint buttons ------------//

	if( refreshFlag == INFO_REPAINT )
	{
		// ####### begin Gilbert 3/10 ########//
		int firmId, x=INFO_X1, y=INFO_Y1, addedCount=0;
		String str;

		for( int i = 0; i < MAX_FIRM_TYPE; ++i )
		{
			firmId = build_firm_button_order[i];

			//---- check if the unit is the right race to build this firm ----//

			button_build_flag[i] = 0;

			if( firm_res[firmId]->can_build(sprite_recno) )
			{
				// button_build_array[firmId-1].paint( x, y, str, "F-DOWN");

				button_build_array[i].paint( x, y, x+99-1, y+60-1,
					disp_firm_button, ButtonCustomPara( game.get_color_remap_table(nation_recno,0) ,
					(unit_id << 16) + (race_id << 8) + firmId) );

				button_build_flag[i] = 1;
				addedCount++;

				if( addedCount & 1)
				{
					// old number, increase in x
					x += 102;
				}
				else
				{
					// even number, increase in y, reset x
					x -= 102;
					y += 63;
				}
			}
		}
		// ####### end Gilbert 3/10 ########//

		button_cancel.paint( x, y, 'A', "CANCEL" );
	}
}
//----------- End of function Unit::disp_build_menu -----------//


//--------- Begin of function Unit::detect_build_menu ---------//
//
void Unit::detect_build_menu()
{
	//----------- detect build buttons ------------//

	int firmId, addedCount=0, rc=0;

	// ##### begin Gilbert 3/10 #######//
	for( int i=0; i<MAX_FIRM_TYPE ; i++ )
	{
		firmId = build_firm_button_order[i];

		//---- check if the unit is the right race to build this firm ----//

		if( button_build_flag[i] && firm_res[firmId]->can_build(sprite_recno) )
		{
			if( button_build_array[i].detect(button_build_hotkey[i]) )
			{
				power.issue_command(COMMAND_BUILD_FIRM, sprite_recno, firmId);
				rc = 1;
				break;
			}

			addedCount++;
		}
	}
	// ##### end Gilbert 3/10 #######//

	//---------- detect cancel button ----------//

	if( button_cancel.detect() )
	{
		// ###### begin Gilbert 26/9 ######//
		se_ctrl.immediate_sound("TURN_OFF");
		// ###### end Gilbert 26/9 ######//
		rc = 1;
	}

	if( rc )
	{
		unit_menu_mode = UNIT_MENU_MAIN;
		info.disp();
	}
}
//----------- End of function Unit::detect_build_menu -----------//


//--------- Begin of function Unit::disp_build ---------//
//
// Display the info when the player has selected the type of
// structure to build.
//
void Unit::disp_build(int refreshFlag)
{
	if( refreshFlag == INFO_REPAINT )
	{
		vga_util.d3_panel_up( INFO_X1, INFO_Y1, INFO_X2, INFO_Y1+42 );

		String str;

		// TRANSLATORS: Please select a location to build the <Firm>.
		snprintf( str, MAX_STR_LEN+1, _("Please select a location to build the %s."), _(firm_res[power.command_para]->name) );

// FRENCH
//		font_san.put_paragraph( INFO_X1, INFO_Y1, INFO_X2, INFO_Y2, str, 0 );
		font_san.put_paragraph( INFO_X1+7, INFO_Y1+5, INFO_X2-7, INFO_Y2-5, str );

		button_cancel2.paint_text( INFO_X1, INFO_Y1+45, INFO_X2, INFO_Y1+70, _("Cancel") );
	}
}
//----------- End of function Unit::disp_build -----------//


//--------- Begin of function Unit::detect_build ---------//
//
void Unit::detect_build()
{
	if( button_cancel2.detect() )
	{
		// ###### begin Gilbert 26/9 ######//
		se_ctrl.immediate_sound("TURN_OFF");
		// ###### end Gilbert 26/9 ######//
		power.command_id = 0;
		info.disp();
	}
}
//----------- End of function Unit::detect_build -----------//


//--------- Begin of function Unit::disp_settle ---------//
//
// Display the info when the player has selected the type of
// structure to build.
//
void Unit::disp_settle(int refreshFlag)
{
	if( refreshFlag == INFO_REPAINT )
	{
		vga_util.d3_panel_up( INFO_X1, INFO_Y1, INFO_X2, INFO_Y1+42 );

		font_san.put_paragraph( INFO_X1+7, INFO_Y1+5, INFO_X2-7, INFO_Y2-5,
										_("Please select a location to settle.") );

		button_cancel2.paint_text( INFO_X1, INFO_Y1+45, INFO_X2, INFO_Y1+70, _("Cancel") );
	}
}
//----------- End of function Unit::disp_settle -----------//


//--------- Begin of function Unit::detect_settle ---------//
//
void Unit::detect_settle()
{
	if( button_cancel2.detect() )
	{
		// ###### begin Gilbert 26/9 ######//
		se_ctrl.immediate_sound("TURN_OFF");
		// ###### end Gilbert 26/9 ######//
		power.command_id = 0;
		info.disp();
	}
}
//----------- End of function Unit::detect_settle -----------//


//--------- Begin of function Unit::disp_unit_info ---------//
//
// Display the skill information of the people in the town.
//
// <int> 	dispY1		 - the top y coordination of the info area
// <int>    refreshFlag  - refresh flag
//
void Unit::disp_unit_info(int dispY1, int refreshFlag)
{
	#ifdef DEBUG
		if(debug2_enable_flag)
		{
			if(unit_res[unit_id]->unit_class == UNIT_CLASS_MONSTER)
			{
				int 	 x=INFO_X1+4, y=dispY1+20;
				y+=20;
				font_san.field( x, y, " " , x+2, sprite_recno, 1, INFO_X2-2, refreshFlag);
				font_san.field( x+20, y, " " , x+22, next_x_loc(), 1, INFO_X2-2, refreshFlag);
				font_san.field( x+50, y, " " , x+52, next_y_loc(), 1, INFO_X2-2, refreshFlag);
				font_san.field( x+70, y, " " , x+72, nation_recno, 1, INFO_X2-2, refreshFlag);

				font_san.field( x+100, y, " " , x+102, action_mode, 1, INFO_X2-2, refreshFlag);
				font_san.field( x+120, y, " " , x+122, action_para, 1, INFO_X2-2, refreshFlag);
				font_san.field( x+140, y, " " , x+142, action_x_loc, 1, INFO_X2-2, refreshFlag);
				font_san.field( x+160, y, " " , x+162, action_y_loc, 1, INFO_X2-2, refreshFlag);
				y-=20;
				font_san.field( x+100, y, " " , x+102, action_mode2, 1, INFO_X2-2, refreshFlag);
				font_san.field( x+120, y, " " , x+122, action_para2, 1, INFO_X2-2, refreshFlag);
				font_san.field( x+140, y, " " , x+142, action_x_loc2, 1, INFO_X2-2, refreshFlag);
				font_san.field( x+160, y, " " , x+162, action_y_loc2, 1, INFO_X2-2, refreshFlag);
				y-=20;
				font_san.field( x+160, y, " " , x+162, cur_action, 1, INFO_X2-2, refreshFlag);
			}
		}
	#endif

	//--------------------------------------------//

	if( !race_id )		// if it's not a human unit, don't display anything
		return;

	if( refreshFlag==INFO_REPAINT )
		vga_util.d3_panel_up( INFO_X1, dispY1, INFO_X2, dispY1+87 );

	int 	 x=INFO_X1+4, y=dispY1+4;
	String str;

	//--------- display loyalty ---------//

	if( rank_id != RANK_KING && (nation_recno || spy_recno) )
	{
		if( spy_recno &&			// only display spy loyalty instead of unit loyalty if this is a spy and this spy is ours
			 true_nation_recno() == nation_array.player_recno )
		{
			font_san.field( x, y, _("Loyalty"), x+92, spy_array[spy_recno]->spy_loyalty, 1, INFO_X2-2, refreshFlag);
		}
		else if( nation_recno )
		{
			info.disp_loyalty( x, y, x+92, loyalty, target_loyalty, nation_recno, refreshFlag );
		}

		y+=16;
	}

	//--------- display combat level ----------//

	font_san.field( x, y, _("Combat") , x+92, skill.combat_level, 1, INFO_X2-2, refreshFlag);

	y+=16;

	//-------- display skill level ---------//

	if( skill.skill_id )
	{
		if( refreshFlag == INFO_REPAINT )
			font_san.field( x, y, skill.skill_des(), x+92, skill.skill_level , 1, INFO_X2-2, refreshFlag);
		else
			font_san.field( x, y, skill.skill_des(), x+92, skill.skill_level , 1, INFO_X2-2, refreshFlag);

		y+=16;
	}

	//------- display spying skill if the unit is a spy -----//

	if( spy_recno && spy_array[spy_recno]->true_nation_recno == nation_array.player_recno )		// only spies of the player's nation can see the spy skill details
	{
		font_san.field( x, y, _("Spying"), x+92, spy_array[spy_recno]->spy_skill, 1, INFO_X2-2, refreshFlag);

		y+=16;
	}

	//--------- display debug info ---------//

	if( !is_civilian() && rank_id != RANK_KING )
		font_san.field( x, y, _("Contribution"), x+92, nation_contribution, 1, INFO_X2-2, refreshFlag);
}
//----------- End of function Unit::disp_unit_info -----------//


//--------- Begin of function Unit::disp_unit_profile ---------//
//
// <int> dispY1 - the top y coordination of the info area
// <int> refreshFlag
//
void Unit::disp_unit_profile(int dispY1, int refreshFlag)
{
	//--------- set help parameters --------//

	int x=INFO_X1+4;

	if( mouse.in_area(x, dispY1+3, x+UNIT_LARGE_ICON_WIDTH-1, dispY1+UNIT_LARGE_ICON_HEIGHT+2) )
		help.set_unit_help( unit_id, rank_id, x, dispY1+3, x+UNIT_LARGE_ICON_WIDTH-1, dispY1+UNIT_LARGE_ICON_HEIGHT+2 );

	//-----------------------------------------//

	if( refreshFlag != INFO_REPAINT )		// only display in repaint mode
		return;

	//-----------------------------------------//

	const char *str=NULL;

	if( race_id )
	{
		if( rank_id == RANK_KING )
		{
			str = _("King");
		}
		else if( rank_id == RANK_GENERAL )
		{
			// ##### patch begin Gilbert 17/2 #####//
			if( unit_mode == UNIT_MODE_REBEL )
				str = _("Rebel Leader");
			else
				str = _("General");
			// ##### patch end Gilbert 17/2 #####//
		}
		else if( unit_mode == UNIT_MODE_DEFEND_TOWN )
		{
			str = _("Defending Villager");
		}
		else if( unit_mode == UNIT_MODE_REBEL )
		{
			str = _("Rebel");
		}
		else if( unit_res[unit_id]->unit_class == UNIT_CLASS_GOD )
		{
			str = _("Greater Being");
		}
		else
		{
			if( should_show_info() )
			{
				switch( skill.skill_id )
				{
					case SKILL_LEADING:
						str = _("Soldier");
						break;

					case SKILL_CONSTRUCTION:
						str  = _("Construction Worker");
						break;

					case SKILL_MINING:
						str = _("Miner");
						break;

					case SKILL_MFT:
						str = _("Worker");
						break;

					case SKILL_RESEARCH:
						str = _("Scientist");
						break;

					case SKILL_SPYING:
						str = _("Spy");
						break;

					default:
						str = _("Peasant");
						break;
				}
			}
			else	//--- don't display too much info on enemy units ---//
			{
				if( skill.skill_id == SKILL_LEADING )
					str = _("Soldier");

				else if( is_civilian() )
					str = _("Civilian");
			}
		}
	}

	//---------------- paint the panel --------------//

	vga_util.d3_panel_up( INFO_X1, dispY1, INFO_X2, dispY1+44);

	vga_front.put_bitmap( x, dispY1+3, unit_res[unit_id]->get_large_icon_ptr(rank_id) );

	//--------------------------------------//

	x += UNIT_LARGE_ICON_WIDTH;

	if( str )
	{
		font_san.center_put( x, dispY1+4, INFO_X2-2, dispY1+21, str );
		font_san.center_put( x, dispY1+22, INFO_X2-2, dispY1+40, unit_name(0) );		// 0-without title
	}
	else
	{
		font_san.center_put( x, dispY1, INFO_X2-2, dispY1+44, unit_name() );		// non-human units
	}
}
//----------- End of function Unit::disp_unit_profile -----------//


//--------- Begin of function Unit::detect_unit_profile ---------//
//
int Unit::detect_unit_profile()
{
	if( is_visible() && mouse.single_click( INFO_X1, INFO_Y1+54, INFO_X2, INFO_Y1+97 ) )
	{
		world.go_loc( next_x_loc(), next_y_loc() );
		return 1;
	}

	return 0;
}
//----------- End of function Unit::detect_unit_profile -----------//


//--------- Begin of function Unit::disp_spy_menu ---------//
//
void Unit::disp_spy_menu(int dispY1, int refreshFlag)
{
	err_when( !spy_recno );

	static char  lastCanChangeCloak;
	static short lastSpyMenuHeight, lastNationCount;

	char  canChangeAnyCloak = can_spy_change_nation();
	short spyMenuHeight  = spy_menu_height();

	//-- if the spy can always change back to its original color. So if it is currently cloaked as another nation, it should be able to revert back any time ---//

	char canChangeOwnCloak = canChangeAnyCloak;		// change back to its own cloak

	if( true_nation_recno() != nation_recno )
		canChangeOwnCloak = 1;

	//---------------------------------------------//

	if( canChangeOwnCloak != lastCanChangeCloak ||
		 spyMenuHeight     != lastSpyMenuHeight ||
		 nation_array.nation_count != lastNationCount )
	{
		lastCanChangeCloak = canChangeOwnCloak;
		lastSpyMenuHeight  = spyMenuHeight;
		lastNationCount	 = nation_array.nation_count;

		info.disp();
		return;
	}

	//---- if enemy nearby and cannot change cloak right now ---//

	if( !canChangeOwnCloak )
	{
		if( refreshFlag==INFO_REPAINT )
		{
			vga_util.d3_panel_up( INFO_X1, dispY1, INFO_X2, dispY1+26 );
			font_san.center_put( INFO_X1, dispY1, INFO_X2-2, dispY1+26, _("Enemies Nearby") );
		}

		return;
	}

	//---------------------------------------------//

	if( refreshFlag==INFO_REPAINT )
	{
		vga_util.d3_panel_up( INFO_X1, dispY1, INFO_X2, dispY1+spyMenuHeight-1 );
		font_san.put( INFO_X1+6, dispY1+6, _("Spy Cloak:") );
	}

	Nation* nationPtr = nation_array[true_nation_recno()];
	int x=INFO_X1+80, y=dispY1+4, nationColor;

	for( int i=1 ; i<=nation_array.size()+1 ; i++ )
	{
		if( canChangeAnyCloak )
		{
			if( i <= nation_array.size() )		// exclude independent town
			{
				if( nation_array.is_deleted(i) || !nationPtr->get_relation(i)->has_contact )
					continue;
			}
		}
		else if( canChangeOwnCloak )
		{
			if( i!=nation_recno && i!=true_nation_recno() )	// only display the current cloaked nation and its true nation
				continue;
		}
		else
			err_here();

		//-----------------------------------//

		if( i>nation_array.size() )		// independent
		{
			nationColor = nation_array.nation_color_array[0];
		}
		else
		{
			nationColor = nation_array[i]->nation_color;
		}

		vga_front.bar( x+2, y+2, x+SPY_CLOAK_WIDTH-3, y+16, nationColor );

		if( i == nation_recno ||
			 (i==nation_array.size()+1 && nation_recno==0) )
		{
			vga_front.rect( x, y, x+SPY_CLOAK_WIDTH-1, y+18, 2, V_YELLOW );
		}
		else
			vga_front.rect( x, y, x+SPY_CLOAK_WIDTH-1, y+18, 2, VGA_GRAY+8 );

		x+=SPY_CLOAK_WIDTH+4;

		if( x+SPY_CLOAK_WIDTH > INFO_X2 )
		{
			x  = INFO_X1+80;
			y += 22;
		}
	}
}
//----------- End of function Unit::disp_spy_menu -----------//


//--------- Begin of function Unit::spy_menu_height ---------//
//
// Return the height of the spy menu.
//
int Unit::spy_menu_height()
{
	if( !can_spy_change_nation() )
		return 27;

	int cloakCount=1;
	Nation* nationPtr = nation_array[true_nation_recno()];

	for( int i=1 ; i<=nation_array.size() ; i++ )
	{
		if( nation_array.is_deleted(i) )
			continue;

		if( i==nation_recno || nationPtr->get_relation(i)->has_contact )
			cloakCount++;
	}

	if( cloakCount > 4 )
		return 47;
	else
		return 27;
}
//----------- End of function Unit::spy_menu_height -----------//


//--------- Begin of function Unit::detect_spy_menu ---------//
//
void Unit::detect_spy_menu(int dispY1)
{
	Nation* nationPtr = nation_array[true_nation_recno()];

	int x=INFO_X1+80, y=dispY1+4, nationRecno, changeFlag=0;

	char canChangeAnyCloak = can_spy_change_nation();
	char canChangeOwnCloak = canChangeAnyCloak;		// change back to its own cloak

	if( true_nation_recno() != nation_recno )
		canChangeOwnCloak = 1;

	if( !canChangeOwnCloak )
		return;

	int i;
	for( i=1 ; i<=nation_array.size()+1 ; i++ )
	{
		if( i > nation_array.size() )
		{
			nationRecno = 0;
		}
		else
		{
			if( canChangeAnyCloak )
			{
				if( nation_array.is_deleted(i) || !nationPtr->get_relation(i)->has_contact )
					continue;
			}
			else
			{
				if( i!=nation_recno && i!=true_nation_recno() )	// only display the current cloaked nation and its true nation
					continue;
			}

			nationRecno = i;
		}

		//---------------------------//

		if( mouse.single_click(x, y, x+SPY_CLOAK_WIDTH-1, y+22) )
		{
			changeFlag=1;
			break;
		}

		x+=SPY_CLOAK_WIDTH+4;

		if( x+SPY_CLOAK_WIDTH > INFO_X2 )
		{
			x  = INFO_X1+80;
			y += 22;
		}
	}

	if( !changeFlag )
		return;

	//--- group changing the cloaks of all of your spies currently selected ---//

	Unit* unitPtr;

	for( i=unit_array.size() ; i>0 ; i-- )
	{
		if( unit_array.is_deleted(i) )
			continue;

		unitPtr = unit_array[i];

		if( unitPtr->is_own_spy() && unitPtr->selected_flag )
			unitPtr->spy_change_nation(nationRecno, COMMAND_PLAYER);
	}

	disp_spy_menu(dispY1, INFO_UPDATE);
}
//----------- End of function Unit::detect_spy_menu -----------//


//----------- Begin of function Unit::disp_hit_point -----------//
void Unit::disp_hit_point(int dispY1)
{
	int hitPoints;

	if( hit_points > (float)0 && hit_points < (float)1 )
		hitPoints = 1;		// display 1 for value between 0 and 1
	else
		hitPoints = (int) hit_points;

	Vga::active_buf->indicator(0x0f, INFO_X1+30, dispY1+1, hit_points, max_hit_points, 0);
}
//----------- End of function Unit::disp_hit_point -----------//

#ifdef DEBUG

//----------- Begin of static function disp_debug_info -----------//

static void disp_debug_info(Unit* unitPtr, int dispY1, int refreshFlag)
{
	if( unitPtr->spy_recno &&
		 unitPtr->true_nation_recno() == nation_array.player_recno )
	{
		return;		// don't display debug info as it will overlap with the buttons
	}

	//---------------------------------------//

	static const char* action_mode_str_array[] =
	{
		"Stop",
		"Attack unit",
		"Attack firm",
		"Attack town",
		"Attack wall",
		"Assign to firm",
		"Assign to town",
		"Assign to vehicle",
		"Assign to ship",
		"Ship to beach",
		"Build firm",
		"Settle",
		"Burn",
		"Die",
		"Move",
		"Go cast power",

		"Auto defense attack target",
		"Auto defense detect target",
		"Auto defense back camp",
		"Defend town attack target",
		"Defend town detect target",
		"Defend town back town",
		"Monster defend attack target",
		"Monster defend detect target",
		"Monster defend back firm",
	};

	static const char* unit_mode_str_array[] =
	{
		"",
		"Oversee",
		"Defend town",
		"Construct",
		"Rebel",
		"Monster",
		"On Ship",
		"In Harbor",
	};

	static const char* cur_action_str_array[] =
	{
		"Idle",
		"Ready to move",
		"Move",
		"Wait",
		"Attack",
		"Turn",
		"Ship extra move",
		"Die",
	};

	if( refreshFlag == INFO_REPAINT )
		vga_util.d3_panel_up( INFO_X1, dispY1, INFO_X2, dispY1+65 );

	int x=INFO_X1+3, y=dispY1+3;

	font_san.disp( INFO_X2-80, y   , unitPtr->sprite_recno, 1, INFO_X2-41 );
	font_san.disp( INFO_X2-80, y+16, unitPtr->ai_action_id, 1, INFO_X2-41 );
	font_san.disp( INFO_X2-80, y+32, unitPtr->home_camp_firm_recno, 1, INFO_X2-41 );
	font_san.disp( INFO_X2-80, y+48, unitPtr->original_action_mode, 1, INFO_X2-41 );

	font_san.disp( INFO_X2-40, y   , unitPtr->leader_unit_recno, 1, INFO_X2-3 );

	font_san.disp( x, y	  , action_mode_str_array[unitPtr->action_mode] , INFO_X2-81 );
	font_san.disp( x, y+=16, action_mode_str_array[unitPtr->action_mode2], INFO_X2-81 );
	font_san.disp( x, y+=16, unit_mode_str_array  [unitPtr->unit_mode]   , INFO_X2-81 );
	font_san.disp( x, y+=16, cur_action_str_array [unitPtr->cur_action-1]  , INFO_X2-81 );
}
//----------- End of static function disp_debug_info -----------//

#endif


// ---------- begin of static function disp_firm_button --------//
static void disp_firm_button(ButtonCustom *button, int)
{
	// button->custom_para.ptr is the color remap table
	// button->custom_para.value is (unit_id << 16) + (race_id << 8) + firm_id
	int firmId = button->custom_para.value & 0xff;
	int raceId = (button->custom_para.value >> 8) & 0xff;

	// format is "F-d-n", d is firmId, n is build code for FIRM_BASE
	String str;
	str += "F-";
	str += firmId;
	if( firmId == FIRM_BASE )
	{
		str += "-";
		str += raceId;
	}

	char *bitmap = image_button.get_ptr(str);

	mouse.hide_area(button->x1, button->y1, button->x2, button->y2 );

	vga_front.put_bitmap_trans_remap_decompress(button->x1, button->y1,	bitmap, (char*) button->custom_para.ptr);

	if( button->pushed_flag )
	{
		image_button.put_front(button->x1, button->y1, "F-DOWN", 1);
	}

	mouse.show_area();
}
// ---------- end of static function disp_firm_button --------//
