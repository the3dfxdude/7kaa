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

//Filename    : OF_BASE.CPP
//Description : Firm Base

#include <OINFO.h>
#include <OVGA.h>
#include <ODATE.h>
#include <OMOUSE.h>
#include <OIMGRES.h>
#include <OHELP.h>
#include <OSTR.h>
#include <OFONT.h>
#include <OCONFIG.h>
#include <OUNIT.h>
#include <OSPY.h>
#include <OGAME.h>
#include <OBUTT3D.h>
#include <ONATION.h>
#include <ORACERES.h>
#include <OWALLRES.h>
#include <OGODRES.h>
#include <OPOWER.h>
#include <OTOWN.h>
#include <OWORLD.h>
#include <OF_BASE.h>
#ifdef USE_DPLAY
#include <OREMOTE.h>
#endif
#include <OSERES.h>
#include <OSE.h>

//----------- Define static vars -------------//

static Button3D   button_invoke, button_reward;

//--------- Begin of function FirmBase::FirmBase ---------//
//
FirmBase::FirmBase()
{
	firm_skill_id = SKILL_PRAYING;
}
//----------- End of function FirmBase::FirmBase -----------//


//--------- Begin of function FirmBase::~FirmBase ---------//
//
FirmBase::~FirmBase()
{
	err_when( race_id<1 || race_id>MAX_RACE );

	nation_array[nation_recno]->base_count_array[race_id-1]--;

	err_when( nation_array[nation_recno]->base_count_array[race_id-1] < 0 );
}
//----------- End of function FirmBase::~FirmBase -----------//


//--------- Begin of function FirmBase::init_derived ---------//
//
void FirmBase::init_derived()
{
	pray_points = (float) 0;

	//------ increase NationBase::base_count_array[] -----//

	err_when( race_id<1 || race_id>MAX_RACE );

	nation_array[nation_recno]->base_count_array[race_id-1]++;

	//---------- set the god id. ----------//

	god_id = 0 ;
	god_unit_recno = 0;

	for( int i=1 ; i<=god_res.god_count ; i++ )
	{
		if( god_res[i]->race_id == race_id &&
			 god_res[i]->is_nation_know(nation_recno) )
		{
			god_id = i;
			break;
		}
	}

	err_when( god_id==0 );
}
//----------- End of function FirmBase::init_derived -----------//


//--------- Begin of function FirmBase::assign_unit ---------//
//
void FirmBase::assign_unit(int unitRecno)
{
	Unit* unitPtr = unit_array[unitRecno];

	//### begin alex 18/9 ###//
	/*//------ only assign units of the right race ------//

	if( unitPtr->race_id != race_id )
		return;*/
	//#### end alex 18/9 ####//

	//------- if this is a construction worker -------//

	if( unitPtr->skill.skill_id == SKILL_CONSTRUCTION )
	{
		set_builder(unitRecno);
		return;
	}

	//### begin alex 18/9 ###//
	//------ only assign units of the right race ------//

	if( unitPtr->race_id != race_id )
		return;
	//#### end alex 18/9 ####//

	//-------- assign the unit ----------//

	int rankId = unit_array[unitRecno]->rank_id;

	if( rankId == RANK_GENERAL || rankId==RANK_KING )
	{
		assign_overseer(unitRecno);
	}
	else
	{
		assign_worker(unitRecno);
	}
}
//----------- End of function FirmBase::assign_unit -----------//


//--------- Begin of function FirmBase::assign_overseer ---------//
//
void FirmBase::assign_overseer(int overseerRecno)
{
	//---- reset the team member count of the general ----//

	if( overseerRecno )
	{
		Unit* unitPtr = unit_array[overseerRecno];

		err_when( !unitPtr->race_id );
		err_when( unitPtr->rank_id != RANK_GENERAL && unitPtr->rank_id != RANK_KING );
		err_when( !unitPtr->team_info );

		unitPtr->team_info->member_count = 0;
	}

	//----- assign the overseer now -------//

	Firm::assign_overseer(overseerRecno);
}
//----------- End of function FirmBase::assign_overseer -----------//


//--------- Begin of function FirmBase::put_info ---------//
//
void FirmBase::put_info(int refreshFlag)
{
	disp_basic_info(INFO_Y1, refreshFlag);

	if( !should_show_info() )
		return;

	disp_base_info(INFO_Y1+54, refreshFlag);
	disp_worker_list(INFO_Y1+104, refreshFlag);
	disp_worker_info(INFO_Y1+168, refreshFlag);
	disp_god_info(INFO_Y1+226, refreshFlag);

	//------ display button -------//

	int x, y=INFO_Y1+279;

	if( own_firm() )
	{
		if( refreshFlag==INFO_REPAINT )
		{
			button_invoke.paint( INFO_X1, y, 'A', "INVOKE" );
			button_reward.paint( INFO_X1+BUTTON_ACTION_WIDTH, y, 'A', "REWARDSP" );
		}

		if( can_invoke() )
			button_invoke.enable();
		else
			button_invoke.disable();

		if( nation_array[nation_recno]->cash >= REWARD_COST &&
			 ( (overseer_recno && unit_array[overseer_recno]->rank_id != RANK_KING)
			  || selected_worker_id ) )
		{
			button_reward.enable();
		}
		else
		{
			button_reward.disable();
		}

		x=INFO_X1+BUTTON_ACTION_WIDTH*2;
	}
	else
		x=INFO_X1;

	disp_spy_button(x, y, refreshFlag);
}
//----------- End of function FirmBase::put_info -----------//


//--------- Begin of function FirmBase::detect_info ---------//
//
void FirmBase::detect_info()
{
	if( detect_basic_info() )
		return;

	if( !should_show_info() )
		return;

	//------ detect the overseer button -----//

	int rc = mouse.single_click(INFO_X1+6, INFO_Y1+58,
				INFO_X1+5+UNIT_LARGE_ICON_WIDTH, INFO_Y1+57+UNIT_LARGE_ICON_HEIGHT, 2 );

	if( rc==1 )		// display this overseer's info
	{
		selected_worker_id = 0;
		disp_base_info(INFO_Y1+54, INFO_UPDATE);
		disp_worker_list(INFO_Y1+104, INFO_UPDATE);
		disp_worker_info(INFO_Y1+168, INFO_UPDATE);
	}

	//--------- detect soldier info ---------//

	if( detect_worker_list() )
	{
		disp_base_info(INFO_Y1+54, INFO_UPDATE);
		disp_worker_list(INFO_Y1+104, INFO_UPDATE);
		disp_worker_info(INFO_Y1+168, INFO_UPDATE);
	}

	//---------- detect spy button ----------//

	detect_spy_button();

	if( !own_firm() )
		return;

	//------ detect the overseer button -----//

	if( rc==2 )
	{
#ifdef USE_DPLAY
		if(remote.is_enable())
		{
			// packet structure : <firm recno>
			short *shortPtr=(short *)remote.new_send_queue_msg(MSG_FIRM_MOBL_OVERSEER, sizeof(short));
			shortPtr[0] = firm_recno;
		}
		else
#endif
		{
			assign_overseer(0);		// the overseer quits the camp
		}
	}

	//----------- detect invoke -----------//

	if( button_invoke.detect() )
	{
#ifdef USE_DPLAY
		if(remote.is_enable())
		{
			// ##### begin Gilbert 14/10 #######//
			// packet structure : <firm recno>
			short *shortPtr=(short *)remote.new_send_queue_msg(MSG_F_BASE_INVOKE_GOD, sizeof(short));
			shortPtr[0] = firm_recno;
			// ##### end Gilbert 14/10 #######//
		}
		else
#endif
		{
			invoke_god();
		}
	}

	//----------- detect reward -----------//

	if( button_reward.detect() )
	{
		reward(selected_worker_id, COMMAND_PLAYER);
		// ##### begin Gilbert 26/9 ########//
		se_ctrl.immediate_sound("TURN_ON");
		// ##### end Gilbert 26/9 ########//
	}
}
//----------- End of function FirmBase::detect_info -----------//


//--------- Begin of function FirmBase::disp_base_info ---------//
//
void FirmBase::disp_base_info(int dispY1, int refreshFlag)
{
	//---------------- paint the panel --------------//

	if( refreshFlag == INFO_REPAINT )
		vga.d3_panel_up( INFO_X1, dispY1, INFO_X2, dispY1+46);

	if( !overseer_recno )
		return;

	//------------ display overseer info -------------//

	Unit* overseerUnit = unit_array[overseer_recno];

	int x=INFO_X1+6, y=dispY1+4, x1=x+UNIT_LARGE_ICON_WIDTH+8;

	if( selected_worker_id == 0 )
	{
		vga_front.rect( x-2, y-2, x+UNIT_LARGE_ICON_WIDTH+1, y+UNIT_LARGE_ICON_HEIGHT+1, 2, V_YELLOW );
	}
	else
	{
		vga.blt_buf( x-2, y-2, x+UNIT_LARGE_ICON_WIDTH+1, y-1, 0 );
		vga.blt_buf( x-2, y+UNIT_LARGE_ICON_HEIGHT+1, x+UNIT_LARGE_ICON_WIDTH+1, y+UNIT_LARGE_ICON_HEIGHT+2, 0 );
		vga.blt_buf( x-2, y-2, x-1, y+UNIT_LARGE_ICON_HEIGHT+2, 0 );
		vga.blt_buf( x+UNIT_LARGE_ICON_WIDTH, y-2, x+UNIT_LARGE_ICON_WIDTH+1, y+UNIT_LARGE_ICON_HEIGHT+2, 0 );
	}

	//-------------------------------------//

	if( refreshFlag == INFO_REPAINT )
	{
		vga_front.put_bitmap(x, y, unit_res[overseerUnit->unit_id]->get_large_icon_ptr(overseerUnit->rank_id) );
	}

	//-------- set help parameters --------//

	if( mouse.in_area(x, y, x+UNIT_LARGE_ICON_WIDTH+3, y+UNIT_LARGE_ICON_HEIGHT+3) )
		help.set_unit_help( overseerUnit->unit_id, overseerUnit->rank_id, x, y, x+UNIT_LARGE_ICON_WIDTH+3, y+UNIT_LARGE_ICON_HEIGHT+3);

	//-------------------------------------//

	if( overseerUnit->rank_id == RANK_KING )
	{
		if( refreshFlag == INFO_REPAINT )
			font_san.put( x1, y, "King" );

		y+=14;
	}

	if( refreshFlag == INFO_REPAINT )
		font_san.put( x1, y, overseerUnit->unit_name(0), 0, INFO_X2-2 );		// 0-ask unit_name() not to return the title of the unit

	y+=14;

	//------- display leadership -------//

	String str;

	str  = translate.process("Leadership");
	str += ": ";
	str += overseerUnit->skill.get_skill(SKILL_LEADING);

	font_san.disp( x1, y, str, INFO_X2-10 );
	y+=14;

	//--------- display loyalty ----------//

	if( overseerUnit->rank_id != RANK_KING )
	{
		x1 = font_san.put( x1, y, "Loyalty:" );

		int x2 = info.disp_loyalty( x1, y-1, x1, overseerUnit->loyalty, overseerUnit->target_loyalty, nation_recno, refreshFlag );

		if( overseerUnit->spy_recno )
		{
			//------ if this is the player's spy -------//

			if( overseerUnit->true_nation_recno() == nation_array.player_recno )
			{
				vga_front.put_bitmap( x2+5, y+1, image_icon.get_ptr("U_SPY") );
				x2 += 15;
			}
		}

		vga.blt_buf( x2, y-1, INFO_X2-2, dispY1+44, 0 );
	}
}
//----------- End of function FirmBase::disp_base_info -----------//


//--------- Begin of function FirmBase::next_day ---------//
//
void FirmBase::next_day()
{
	//----- call next_day() of the base class -----//

	Firm::next_day();

	//--------------------------------------//

	calc_productivity();

	//--------------------------------------//

	if( info.game_date%15 == firm_recno%15 )			// once a week
	{
		train_unit();
		recover_hit_point();
	}

	//------- increase pray points --------//

	if( overseer_recno && pray_points < MAX_PRAY_POINTS )
	{
		// ###### patch begin Gilbert 21/1 #######//
		if( config.fast_build )
			pray_points += productivity/10;
		else
			pray_points += productivity/100;
		// ###### patch end Gilbert 21/1 #######//

		if( pray_points > MAX_PRAY_POINTS )
			pray_points = (float) MAX_PRAY_POINTS;
	}

	//------ validate god_unit_recno ------//

	if( god_unit_recno )
	{
		if( unit_array.is_deleted(god_unit_recno) )
			god_unit_recno = 0;

	#ifdef DEBUG
		if( god_unit_recno )
		{
			err_when( !unit_array[god_unit_recno]->is_visible() );
			err_when( unit_array[god_unit_recno]->nation_recno != nation_recno );
		}
	#endif
	}
}
//----------- End of function FirmBase::next_day -----------//


//------- Begin of function FirmBase::change_nation ---------//
//
void FirmBase::change_nation(int newNationRecno)
{
	//--- update the UnitInfo vars of the workers in this firm ---//

	for( int i=0 ; i<worker_count ; i++ )
		unit_res[ worker_array[i].unit_id ]->unit_change_nation(newNationRecno, nation_recno, worker_array[i].rank_id );

	//------ update base_count_array[] --------//

	err_when( race_id<1 || race_id>MAX_RACE );

	nation_array[nation_recno]->base_count_array[race_id-1]--;

	err_when( nation_array[nation_recno]->base_count_array[race_id-1] < 0 );

	nation_array[newNationRecno]->base_count_array[race_id-1]++;

	//----- change the nation recno of the god invoked by the base if there is any ----//

	if( god_unit_recno && !unit_array.is_deleted(god_unit_recno) )
		unit_array[god_unit_recno]->change_nation(newNationRecno);

	//-------- change the nation of this firm now ----------//

	Firm::change_nation(newNationRecno);
}
//-------- End of function FirmBase::change_nation ---------//


//------- Begin of function FirmBase::train_unit -------//
//
// Increase the praying skills of the prayers.
//
void FirmBase::train_unit()
{
	if( !overseer_recno )
		return;

	Unit* overseerUnit = unit_array[overseer_recno];
	int 	overseerSkill = overseerUnit->skill.skill_level;
	int	incValue;

	//------- increase the commander's leadership ---------//

	if( worker_count > 0 && overseerUnit->skill.skill_level < 100 )
	{
		//-- the more soldiers this commander has, the higher the leadership will increase ---//

		incValue = 3 * worker_count
					  * (int) overseerUnit->hit_points / overseerUnit->max_hit_points
					  * (100+overseerUnit->skill.skill_potential*2) / 100;

		overseerUnit->skill.skill_level_minor += incValue;

		if( overseerUnit->skill.skill_level_minor >= 100 )
		{
			overseerUnit->skill.skill_level_minor -= 100;
			overseerUnit->skill.skill_level++;
		}
	}

	//------- increase the prayer's skill level ------//

	int	  levelMinor;
	Worker* workerPtr = worker_array;

	for( int i=0 ; i<worker_count ; i++, workerPtr++ )
	{
		//------- increase prayer skill -----------//

		if( workerPtr->skill_level < overseerSkill )
		{
			incValue = MAX(20, overseerSkill-workerPtr->skill_level)
						  * workerPtr->hit_points / workerPtr->max_hit_points()
						  * (100+workerPtr->skill_potential*2) / 100;

			levelMinor = workerPtr->skill_level_minor + incValue;		// with random factors, resulting in 75% to 125% of the original

			while( levelMinor >= 100 )
			{
				levelMinor -= 100;
				workerPtr->skill_level++;
			}

			workerPtr->skill_level_minor = levelMinor;
		}
	}
}
//-------- End of function FirmBase::train_unit --------//


//------- Begin of function FirmBase::recover_hit_point -------//
//
// Prayers recover their hit points.
//
// No need to recover the hit points of the general here as
// this is taken care in the Unit class function of the general.
//
void FirmBase::recover_hit_point()
{
	Worker* workerPtr = worker_array;

	for( int i=0 ; i<worker_count ; i++, workerPtr++ )
	{
		//------- increase worker hit points --------//

		if( workerPtr->hit_points < workerPtr->max_hit_points() )
			workerPtr->hit_points++;
	}
}
//------- End of function FirmBase::recover_hit_point -------//

//--------- Begin of function FirmBase::disp_god_info ---------//
//
void FirmBase::disp_god_info(int dispY1, int refreshFlag)
{
	//---------------- paint the panel --------------//

	if( refreshFlag == INFO_REPAINT )
		vga.d3_panel_up( INFO_X1, dispY1, INFO_X2, dispY1+50 );

	//-------- display the icon of the mythical creature -------//

	int 		 x=INFO_X1+4, y=dispY1+4;
	UnitInfo* unitInfo = unit_res[ god_res[god_id]->unit_id ];

	if( refreshFlag == INFO_REPAINT )
	{
		vga.d3_panel_down( x, y, x+UNIT_LARGE_ICON_WIDTH+3, y+UNIT_LARGE_ICON_HEIGHT+3, 2 );
		vga_front.put_bitmap( x+2, y+2, unitInfo->get_large_icon_ptr(0) );

		//----------- display text ------------//

		x += UNIT_LARGE_ICON_WIDTH+10;

		font_san.put( x, y+2, unitInfo->name );
	}
	else
	{
		x += UNIT_LARGE_ICON_WIDTH+10;
	}

	vga_front.indicator( 0x00, x-3, y+18, pray_points, (float) MAX_PRAY_POINTS, 0 );
}
//----------- End of function FirmBase::disp_god_info -----------//


//--------- Begin of function FirmBase::invoke_god ---------//
//
// Invoke God.
//
void FirmBase::invoke_god()
{
	god_unit_recno = god_res[god_id]->invoke(firm_recno, center_x, center_y);
}
//----------- End of function FirmBase::invoke_god -----------//


//--------- Begin of function FirmBase::can_invoke ---------//
//
int FirmBase::can_invoke()
{
	//----- if the base's god creature has been destroyed -----//

	if( god_unit_recno && unit_array.is_deleted(god_unit_recno) )
		god_unit_recno = 0;

	//---------------------------------------------------------//

	return !god_unit_recno &&		// one base can only support one god
			 overseer_recno &&
			 pray_points >= MAX_PRAY_POINTS/10;		// there must be at least 10% of the maximum pray points to cast a creature
}
//----------- End of function FirmBase::can_invoke -----------//
