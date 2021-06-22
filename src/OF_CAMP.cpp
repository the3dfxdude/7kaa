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

//Filename    : OF_CAMP.CPP
//Description : Firm Military Camp

#include <OINFO.h>
#include <OVGA.h>
#include <vga_util.h>
#include <ODATE.h>
#include <OIMGRES.h>
#include <OHELP.h>
#include <OSYS.h>
#include <OSTR.h>
#include <OFONT.h>
#include <OMOUSE.h>
#include <OBUTTON.h>
#include <OBUTT3D.h>
#include <OPOWER.h>
#include <OUNIT.h>
#include <OINFO.h>
#include <OGAME.h>
#include <OSPY.h>
#include <ONATION.h>
#include <ORACERES.h>
#include <OTERRAIN.h>
#include <OWORLD.h>
#include <OF_CAMP.h>
#include <OREMOTE.h>
#include <OSERES.h>
#include <OSE.h>
#include "gettext.h"


//----------- Define static vars -------------//

static Button3D button_patrol, button_reward, button_defense;

//--------- Declare static functions ---------//

static void disp_debug_info(FirmCamp* firmPtr, int refreshFlag);

//--------- Begin of function FirmCamp::FirmCamp ---------//
//
FirmCamp::FirmCamp()
{
	firm_skill_id = SKILL_LEADING;

	employ_new_worker = 1;

	memset(defense_array, 0, sizeof(DefenseUnit)*(MAX_WORKER+1));

	defend_target_recno = 0;
	defense_flag = 1;

	is_attack_camp = 0;
}
//----------- End of function FirmCamp::FirmCamp -----------//


//--------- Begin of function FirmCamp::deinit ---------//
//
void FirmCamp::deinit()
{
	int firmRecno = firm_recno;		// save the firm_recno first for reset_unit_home_camp()

	Firm::deinit();

	//-------------------------------------------//

	int saveOverseerRecno = overseer_recno;

	overseer_recno = 0;     // set overseer_recno to 0 when calling update_influence(), so this base is disregarded.

	update_influence();

	overseer_recno = saveOverseerRecno;

	clear_defense_mode(firmRecno);

	//---- reset all units whose home_camp_firm_recno is this firm ----//

	reset_unit_home_camp(firmRecno);		// this must be called at last as Firm::deinit() will create new units.

	//--- if this camp is in the Nation::attack_camp_array[], remove it now ---//

	reset_attack_camp(firmRecno);
}
//----------- End of function FirmCamp::deinit -----------//


//--------- Begin of function FirmCamp::reset_unit_home_camp ---------//
//
void FirmCamp::reset_unit_home_camp(int firmRecno)
{
	//---- reset all units whose home_camp_firm_recno is this firm ----//

	Unit* unitPtr;

	for( int i=unit_array.size() ; i>0 ; i-- )
	{
		if( unit_array.is_deleted(i) )
			continue;

		unitPtr = unit_array[i];

		if( unitPtr->home_camp_firm_recno == firmRecno )
			unitPtr->home_camp_firm_recno = 0;
	}
}
//----------- End of function FirmCamp::reset_unit_home_camp -----------//


//--------- Begin of function FirmCamp::reset_attack_camp ---------//
//
void FirmCamp::reset_attack_camp(int firmRecno)
{
	//--- if this camp is in the Nation::attack_camp_array[], remove it now ---//

	if( firm_ai )
	{
		Nation* nationPtr = nation_array[nation_recno];

		for( int i=0 ; i<nationPtr->attack_camp_count ; i++ )
		{
			if( nationPtr->attack_camp_array[i].firm_recno == firmRecno )
			{
				misc.del_array_rec(nationPtr->attack_camp_array, nationPtr->attack_camp_count, sizeof(AttackCamp), i+1 );
				nationPtr->attack_camp_count--;
				break;
			}
		}
	}
}
//----------- End of function FirmCamp::reset_attack_camp -----------//


//--------- Begin of function FirmCamp::clear_defense_mode ---------//
void FirmCamp::clear_defense_mode(int firmRecno)
{
	//------------------------------------------------------------------//
	// change defense unit's to non-defense mode
	//------------------------------------------------------------------//
	Unit *unitPtr;
	for(int i=unit_array.size(); i>=1; --i)
	{
		if(unit_array.is_deleted(i))
			continue;

		unitPtr = unit_array[i];
		if(!unitPtr)
			continue;

		err_when(unitPtr->cur_action==SPRITE_DIE || unitPtr->action_mode==ACTION_DIE || unitPtr->hit_points<=0);
		if(unitPtr->in_auto_defense_mode() && unitPtr->action_misc==ACTION_MISC_DEFENSE_CAMP_RECNO &&
			unitPtr->action_misc_para==firmRecno)
			unitPtr->clear_unit_defense_mode();
	}

	memset(defense_array, 0, sizeof(DefenseUnit)*(MAX_WORKER+1));
}
//----------- End of function FirmCamp::clear_defense_mode -----------//


//--------- Begin of function FirmCamp::assign_unit ---------//
//
void FirmCamp::assign_unit(int unitRecno)
{
	Unit* unitPtr = unit_array[unitRecno];

	//------- if this is a construction worker -------//

	if( unitPtr->skill.skill_id == SKILL_CONSTRUCTION )
	{
		set_builder(unitRecno);
		return;
	}

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
//----------- End of function FirmCamp::assign_unit -----------//


//--------- Begin of function FirmCamp::assign_overseer ---------//
//
void FirmCamp::assign_overseer(int overseerRecno)
{
	//---- reset the team member count of the general ----//

	if( overseerRecno )
	{
		Unit* unitPtr = unit_array[overseerRecno];

		err_when( !unitPtr->race_id );
		err_when( unitPtr->rank_id != RANK_GENERAL && unitPtr->rank_id != RANK_KING );
		err_when( !unitPtr->team_info );

		unitPtr->team_info->member_count = 0;
		unitPtr->home_camp_firm_recno 	= 0;
	}

	//----- assign the overseer now -------//

	Firm::assign_overseer(overseerRecno);

	//------------- update influence -----------//

	update_influence();
}
//----------- End of function FirmCamp::assign_overseer -----------//


//--------- Begin of function FirmCamp::assign_worker --------//
//
// Increase armed unit count of the race of the worker assigned,
// as when a unit is assigned to a camp, Unit::deinit() will decrease
// the counter, so we need to increase it back here.
//
void FirmCamp::assign_worker(int workerUnitRecno)
{
	Firm::assign_worker(workerUnitRecno);

	//--- remove the unit from patrol_unit_array when it returns to the base ---//

	validate_patrol_unit();

	//-------- sort soldiers ---------//

	sort_worker();
}
//----------- End of function FirmCamp::assign_worker --------//


//------- Begin of function FirmCamp::validate_patrol_unit ---------//
//
void FirmCamp::validate_patrol_unit()
{
	int unitRecno;
	Unit* unitPtr;

	err_when( patrol_unit_count > 9 );

	for( int i=patrol_unit_count ; i>0 ; i-- )
	{
		unitRecno = patrol_unit_array[i-1];

		if( unit_array.is_deleted(unitRecno) ||
			 (unitPtr=unit_array[unitRecno])->is_visible()==0 ||
			 unitPtr->nation_recno != nation_recno )
		{
			err_when( patrol_unit_count > 9 );

			misc.del_array_rec( patrol_unit_array, patrol_unit_count, sizeof(patrol_unit_array[0]), i );

			err_when( patrol_unit_count==0 );		// it's already 0
			patrol_unit_count--;

			err_when( patrol_unit_count < 0 );
			err_when( patrol_unit_count > 9 );
		}
	}
}
//-------- End of function FirmCamp::validate_patrol_unit ---------//


//------- Begin of function FirmCamp::change_nation ---------//
//
void FirmCamp::change_nation(int newNationRecno)
{
	//--- update the UnitInfo vars of the workers in this firm ---//

	for( int i=0 ; i<worker_count ; i++ )
		unit_res[ worker_array[i].unit_id ]->unit_change_nation(newNationRecno, nation_recno, worker_array[i].rank_id );

	//----- reset unit's home camp to this firm -----//

	reset_unit_home_camp(firm_recno);

	//--- if this camp is in the Nation::attack_camp_array[], remove it now ---//

	reset_attack_camp(firm_recno);

	//---- reset AI vars --------//

	ai_capture_town_recno = 0;
	ai_recruiting_soldier = 0;

	//-------- change the nation of this firm now ----------//

	Firm::change_nation(newNationRecno);
}
//-------- End of function FirmCamp::change_nation ---------//


//--------- Begin of function FirmCamp::update_influence ---------//
//
// Update this camp's influence on neighbor towns.
//
void FirmCamp::update_influence()
{
	int   i;
	Town* townPtr;

	for( i=0 ; i<linked_town_count ; i++ )
	{
		if(town_array.is_deleted(linked_town_array[i]))
			continue;

		townPtr = town_array[linked_town_array[i]];

		if( linked_town_enable_array[i] == LINK_EE )
		{
			if( townPtr->nation_recno )
				townPtr->update_target_loyalty();
			else
				townPtr->update_target_resistance();
		}
	}
}
//----------- End of function FirmCamp::update_influence -----------//


//--------- Begin of function FirmCamp::put_info ---------//
//
void FirmCamp::put_info(int refreshFlag)
{
	disp_basic_info(INFO_Y1, refreshFlag);

	if( !should_show_info() )
		return;

	disp_camp_info(INFO_Y1+54, refreshFlag);
	disp_worker_list(INFO_Y1+104, refreshFlag);
	disp_worker_info(INFO_Y1+168, refreshFlag);

	//------ display button -------//

	int x;

	if( own_firm() )
	{
		if( refreshFlag==INFO_REPAINT )
		{
			button_patrol.paint( INFO_X1, INFO_Y1+242, 'A', "PATROL" );
			button_reward.paint( INFO_X1+BUTTON_ACTION_WIDTH, INFO_Y1+242, 'A', "REWARDCB" );
			button_defense.paint( INFO_X2-BUTTON_ACTION_WIDTH, INFO_Y1+242, 'A', defense_flag ? (char*)"DEFENSE1" : (char*)"DEFENSE0" );
		}

		if( overseer_recno || worker_count )
			button_patrol.enable();
		else
			button_patrol.disable();

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

	disp_spy_button(x, INFO_Y1+242, refreshFlag);

	#ifdef DEBUG
		if( sys.testing_session || sys.debug_session )
			disp_debug_info(this, refreshFlag);
	#endif
}
//----------- End of function FirmCamp::put_info -----------//


//--------- Begin of function FirmCamp::detect_info ---------//
//
int FirmCamp::detect_info()
{
	if( detect_basic_info() )
		return 1;

	if( !should_show_info() )
		return 0;

	//------ detect the overseer button -----//

	int rc = mouse.any_click(INFO_X1+6, INFO_Y1+58, INFO_X1+5+UNIT_LARGE_ICON_WIDTH, INFO_Y1+57+UNIT_LARGE_ICON_HEIGHT, LEFT_BUTTON) ? 1 
		: mouse.any_click(INFO_X1+6, INFO_Y1+58, INFO_X1+5+UNIT_LARGE_ICON_WIDTH, INFO_Y1+57+UNIT_LARGE_ICON_HEIGHT, RIGHT_BUTTON) ? 2 : 0;

	if( rc==1 )		// display this overseer's info
	{
		selected_worker_id = 0;
		disp_camp_info(INFO_Y1+54, INFO_UPDATE);
		disp_worker_list(INFO_Y1+104, INFO_UPDATE);
		disp_worker_info(INFO_Y1+168, INFO_UPDATE);
		return 1;
	}

	//--------- detect soldier info ---------//

	if( detect_worker_list() )
	{
		disp_camp_info(INFO_Y1+54, INFO_UPDATE);
		disp_worker_list(INFO_Y1+104, INFO_UPDATE);
		disp_worker_info(INFO_Y1+168, INFO_UPDATE);
		return 1;
	}

	//---------- detect spy button ----------//

	if( detect_spy_button() )
		return 1;

	if( !own_firm() )
		return 0;

	//------ detect the overseer button -----//

	if( rc==2 )
	{
		if(remote.is_enable())
		{
			// packet structure : <firm recno>
			short *shortPtr=(short *)remote.new_send_queue_msg(MSG_FIRM_MOBL_OVERSEER, sizeof(short));
			shortPtr[0] = firm_recno;
		}
		else
		{
			assign_overseer(0);		// the overseer quits the camp
		}
		return 1;
	}

	//----------- detect patrol -----------//

	if( button_patrol.detect(GETKEY(KEYEVENT_FIRM_PATROL)) )
	{
		if(remote.is_enable())
		{
			// packet structure : <firm recno>
			short *shortPtr=(short *)remote.new_send_queue_msg(MSG_F_CAMP_PATROL, sizeof(short));
			shortPtr[0] = firm_recno;
		}
		else
		{
			patrol();
		}
		return 1;
	}

	//----------- detect reward -----------//

	if( button_reward.detect() )
	{
		reward(selected_worker_id, COMMAND_PLAYER);
		// ##### begin Gilbert 25/9 ######//
		se_ctrl.immediate_sound("TURN_ON");
		// ##### end Gilbert 25/9 ######//
		return 1;
	}

	//----- detect defense mode button -------//

	if( button_defense.detect() )
	{
		// ##### begin Gilbert 25/9 ######//
		se_ctrl.immediate_sound( !defense_flag?(char*)"TURN_ON":(char*)"TURN_OFF");
		// ##### end Gilbert 25/9 ######//

		if( !remote.is_enable() )
		{
			// update RemoteMsg::toggle_camp_patrol()
			defense_flag = !defense_flag;
		}
		else
		{
			// packet structure : <firm recno> <defense_flag>
			short *shortPtr=(short *)remote.new_send_queue_msg(MSG_F_CAMP_TOGGLE_PATROL, 2*sizeof(short));
			shortPtr[0] = firm_recno;
			shortPtr[1] = !defense_flag;
		}

		button_defense.update_bitmap( defense_flag ? (char*)"DEFENSE1" : (char*)"DEFENSE0" );

		return 1;
	}

	return 0;
}
//----------- End of function FirmCamp::detect_info -----------//


//--------- Begin of function FirmCamp::disp_camp_info ---------//
//
void FirmCamp::disp_camp_info(int dispY1, int refreshFlag)
{
	//---------------- paint the panel --------------//

	if( refreshFlag == INFO_REPAINT )
		vga_util.d3_panel_up( INFO_X1, dispY1, INFO_X2, dispY1+46);

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
		vga_util.blt_buf( x-2, y-2, x+UNIT_LARGE_ICON_WIDTH+1, y-1, 0 );
		vga_util.blt_buf( x-2, y+UNIT_LARGE_ICON_HEIGHT+1, x+UNIT_LARGE_ICON_WIDTH+1, y+UNIT_LARGE_ICON_HEIGHT+2, 0 );
		vga_util.blt_buf( x-2, y-2, x-1, y+UNIT_LARGE_ICON_HEIGHT+2, 0 );
		vga_util.blt_buf( x+UNIT_LARGE_ICON_WIDTH, y-2, x+UNIT_LARGE_ICON_WIDTH+1, y+UNIT_LARGE_ICON_HEIGHT+2, 0 );
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
			font_san.put( x1, y, _("King") );

		y+=14;
	}

	if( refreshFlag == INFO_REPAINT )
		font_san.put( x1, y, overseerUnit->unit_name(0), 0, INFO_X2-2 );		// 0-ask unit_name() not to return the title of the unit

	y+=14;

	//------- display leadership -------//

	String str;

	str  = _("Leadership");
	str += ": ";
	str += overseerUnit->skill.get_skill(SKILL_LEADING);

	font_san.disp( x1, y, str, INFO_X2-10 );
	y+=14;

	//--------- display loyalty ----------//

	if( overseerUnit->rank_id != RANK_KING )
	{
		str  = _("Loyalty");
		str += ":";
		x1 = font_san.put( x1, y, str );

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

		vga_util.blt_buf( x2, y-1, INFO_X2-2, dispY1+44, 0 );
	}
}
//----------- End of function FirmCamp::disp_camp_info -----------//


//--------- Begin of function FirmCamp::next_day ---------//
//
void FirmCamp::next_day()
{
	//----- call next_day() of the base class -----//

	Firm::next_day();

	//----- update the patrol_unit_array -----//

	validate_patrol_unit();

	//--------------------------------------//

	if( info.game_date%15 == firm_recno%15 )			// once a week
	{
		train_unit();
		recover_hit_point();
	}

	//--- if there are weapons in the firm, pay for their expenses ---//

	pay_weapon_expense();
}
//----------- End of function FirmCamp::next_day -----------//


//------- Begin of function FirmCamp::train_unit -------//
//
// Increase the leadership and combat level of the general and the soldiers.
//
void FirmCamp::train_unit()
{
	if( !overseer_recno )
		return;

	Unit* overseerUnit = unit_array[overseer_recno];

	if( overseerUnit->skill.skill_id != SKILL_LEADING )
		return;

	int 		 overseerSkill = overseerUnit->skill.skill_level;
	RaceInfo* overseerRaceInfo = race_res[overseerUnit->race_id];
	int		 incValue;

	//------- increase the commander's leadership ---------//

	if( worker_count > 0 && overseerUnit->skill.skill_level < 100 )
	{
		//-- the more soldiers this commander has, the higher the leadership will increase ---//

		incValue = 5 * worker_count
					  * (int) overseerUnit->hit_points / overseerUnit->max_hit_points
					  * (100+overseerUnit->skill.skill_potential*2) / 100;

		overseerUnit->skill.skill_level_minor += incValue;

		if( overseerUnit->skill.skill_level_minor >= 100 )
		{
			overseerUnit->skill.skill_level_minor -= 100;
			overseerUnit->skill.skill_level++;
		}
	}

	//------- increase the commander's combat level ---------//

	if( overseerUnit->skill.combat_level < 100 )
	{
		incValue = 20 * (int) overseerUnit->hit_points / overseerUnit->max_hit_points
					  * (100+overseerUnit->skill.skill_potential*2) / 100;

		overseerUnit->skill.combat_level_minor += incValue;

		if( overseerUnit->skill.combat_level_minor >= 100 )
		{
			overseerUnit->skill.combat_level_minor -= 100;

			overseerUnit->set_combat_level(overseerUnit->skill.combat_level+1);
		}
	}

	//------- increase the solider's combat level -------//

	int	  levelMinor;
	Worker* workerPtr = worker_array;

	for( int i=0 ; i<worker_count ; i++, workerPtr++ )
	{
		if( !workerPtr->race_id )
			continue;

		//------- increase worker skill -----------//

		if( workerPtr->combat_level < overseerSkill )
		{
			incValue = MAX(20, overseerSkill-workerPtr->combat_level)
						  * workerPtr->hit_points / workerPtr->max_hit_points()
						  * (100+workerPtr->skill_potential*2) / 100;

			levelMinor = workerPtr->combat_level_minor + incValue;		// with random factors, resulting in 75% to 125% of the original

			while( levelMinor >= 100 )
			{
				levelMinor -= 100;
				workerPtr->combat_level++;
			}

			workerPtr->combat_level_minor = levelMinor;
		}

		//-- if the soldier has leadership potential, he learns leadership --//

		if( workerPtr->skill_potential > 0 && workerPtr->skill_level < 100 )
		{
			incValue = (int) MAX(50, overseerUnit->skill.skill_level-workerPtr->skill_level)
						  * workerPtr->hit_points / workerPtr->max_hit_points()
						  * workerPtr->skill_potential*2 / 100;

			workerPtr->skill_level_minor += incValue;

			err_when( workerPtr->skill_level >= 100 );

			if( workerPtr->skill_level_minor > 100 )
			{
				workerPtr->skill_level++;
				workerPtr->skill_level_minor -= 100;
			}
		}
	}

	sort_worker();
}
//-------- End of function FirmCamp::train_unit --------//


//------- Begin of function FirmCamp::recover_hit_point -------//
//
// Soldiers recover their hit points.
//
// No need to recover the hit points of the general here as
// this is taken care in the Unit class function of the general.
//
void FirmCamp::recover_hit_point()
{
	Worker* workerPtr = worker_array;

	for( int i=0 ; i<worker_count ; i++, workerPtr++ )
	{
		//------- increase worker hit points --------//

		if( workerPtr->hit_points < workerPtr->max_hit_points() )
			workerPtr->hit_points++;
	}
}
//------- End of function FirmCamp::recover_hit_point -------//


//------- Begin of function FirmCamp::pay_weapon_expense -------//
//
void FirmCamp::pay_weapon_expense()
{
	Worker* workerPtr = worker_array;
	Nation* nationPtr = nation_array[nation_recno];

	for( int i=1 ; i<=worker_count ; i++, workerPtr++ )
	{
		if( workerPtr->unit_id &&
			 unit_res[workerPtr->unit_id]->unit_class == UNIT_CLASS_WEAPON )
		{
			if( nationPtr->cash > 0 )
			{
				nationPtr->add_expense( EXPENSE_WEAPON, (float) unit_res[workerPtr->unit_id]->year_cost / 365, 1 );
			}
			else     // decrease hit points if the nation cannot pay the unit
			{
				if( workerPtr->hit_points > 0 )
					workerPtr->hit_points--;

				if( workerPtr->hit_points == 0 )
					kill_worker(i);		// if its hit points is zero, delete it

				err_when( workerPtr->hit_points < 0 );
			}
		}
	}
}
//------- End of function FirmCamp::pay_weapon_expense -------//



//--------- Begin of function FirmCamp::patrol ---------//
//
void FirmCamp::patrol()
{
	err_when( !overseer_recno && !worker_count );
	err_when( firm_ai && ai_capture_town_recno );		// ai_capture_town_recno is set after patrol() is called

	if( nation_recno == nation_array.player_recno )
		power.reset_selection();

	//------------------------------------------------------------//
	// If the commander in this camp has units under his lead
	// outside and he is now going to lead a new team, then
	// the old team members should be reset.
	//------------------------------------------------------------//

	if(overseer_recno)
	{
		TeamInfo* teamInfo = unit_array[overseer_recno]->team_info;

		if( worker_count>0 && teamInfo->member_count>0 )
		{
			int unitRecno;

			for( int i=0 ; i<teamInfo->member_count ; i++ )
			{
				unitRecno = teamInfo->member_unit_array[i];

				if( unit_array.is_deleted(unitRecno) )
					continue;

				unit_array[unitRecno]->leader_unit_recno = 0;
			}
		}
	}

	//------------------------------------------------------------//
	// mobilize workers first, then the overseer.
	//------------------------------------------------------------//

	short overseerRecno = overseer_recno;

	if(patrol_all_soldier() && overseer_recno)
	{
		Unit* unitPtr = unit_array[overseer_recno];

		err_when(unitPtr->rank_id!=RANK_GENERAL && unitPtr->rank_id!=RANK_KING);
		unitPtr->team_id = unit_array.cur_team_id-1;   // set it to the same team as the soldiers which are defined in mobilize_all_worker()

		if( nation_recno == nation_array.player_recno )
		{
			unitPtr->selected_flag = 1;
			unit_array.selected_recno = overseer_recno;
			unit_array.selected_count++;
		}
	}

	assign_overseer(0);

	//---------------------------------------------------//

	if(overseerRecno && !overseer_recno) // has overseer and the overseer is mobilized
	{
		Unit* overseerUnit = unit_array[overseerRecno];

		if(overseerUnit->is_own() )
		{
			se_res.sound( overseerUnit->cur_x_loc(), overseerUnit->cur_y_loc(), 1,
				'S', overseerUnit->sprite_id, "SEL");
		}

		err_when( patrol_unit_count > MAX_WORKER );

		//--- add the overseer into the patrol_unit_array[] of this camp ---//

		patrol_unit_array[patrol_unit_count++] = overseerRecno;

		err_when( patrol_unit_count > 9 );

		//------- set the team_info of the overseer -------//

		err_when( !overseerUnit->team_info );

		for( int i=0 ; i<patrol_unit_count ; i++ )
			overseerUnit->team_info->member_unit_array[i] = patrol_unit_array[i];

		overseerUnit->team_info->member_count = patrol_unit_count;
	}

	//-------- display info --------//

	if( nation_recno == nation_array.player_recno )		// for player's camp, patrol() can only be called when the player presses the button.
		info.disp();

	err_when( patrol_unit_count < 0 );
	err_when( patrol_unit_count > 9 );
}
//----------- End of function FirmCamp::patrol -----------//


//--------- Begin of function FirmCamp::patrol_all_soldier ---------//
//
// return 1 if there is enough space for patroling all soldiers
// return 0 otherwise
//
int FirmCamp::patrol_all_soldier()
{
	err_when(!worker_array);    // this function shouldn't be called if this firm does not need worker

	//------- detect buttons on hiring firm workers -------//

	err_when(worker_count>MAX_WORKER);
	
	#ifdef DEBUG
		int loopCount=0;
	#endif

	short unitRecno;
	int mobileWorkerId = 1;

	patrol_unit_count = 0;		// reset it, it will be increased later

	while(worker_count>0 && mobileWorkerId<=worker_count)
	{
		err_when(++loopCount > 100);

		if(worker_array[mobileWorkerId-1].skill_id==SKILL_LEADING)
		{
			unitRecno = mobilize_worker(mobileWorkerId, COMMAND_AUTO);

			patrol_unit_array[patrol_unit_count++] = unitRecno;
			err_when(patrol_unit_count>MAX_WORKER);
		}
		else
		{
			mobileWorkerId++;
			continue;
		}

		if(!unitRecno)
			return 0; // keep the rest workers as there is no space for creating the unit

		Unit* unitPtr = unit_array[unitRecno];

		unitPtr->team_id = unit_array.cur_team_id;   // define it as a team

		if(overseer_recno)
		{
			unitPtr->leader_unit_recno = overseer_recno;
			unitPtr->update_loyalty();							// the unit is just assigned to a new leader, set its target loyalty

			err_when( unit_array[overseer_recno]->rank_id != RANK_KING &&
					  unit_array[overseer_recno]->rank_id != RANK_GENERAL );
		}

		if( nation_recno == nation_array.player_recno )
		{
			unitPtr->selected_flag = 1;
			unit_array.selected_count++;
			if ( !unit_array.selected_recno )
				unit_array.selected_recno = unitRecno;       // set the first soldier as selected; this is also the soldier with the highest leadership (because of sorting)
		}
	}

	unit_array.cur_team_id++;
	return 1;
}
//----------- End of function FirmCamp::patrol_all_soldier -----------//


//--------- Begin of function FirmCamp::mobilize_overseer --------//
//
int FirmCamp::mobilize_overseer()
{
	int unitRecno = Firm::mobilize_overseer();

	//--- set the home camp firm recno of the unit for later return ---//

	if( unitRecno )
	{
		unit_array[unitRecno]->home_camp_firm_recno = firm_recno;
		return unitRecno;
	}
	else
		return 0;
}
//----------- End of function FirmCamp::mobilize_overseer --------//


//--------- Begin of function FirmCamp::mobilize_worker ---------//
//
int FirmCamp::mobilize_worker(int workerId, char remoteAction)
{
	int unitRecno = Firm::mobilize_worker(workerId, remoteAction);

	//--- set the home camp firm recno of the unit for later return ---//

	if( unitRecno )
	{
		unit_array[unitRecno]->home_camp_firm_recno = firm_recno;
		return unitRecno;
	}
	else
		return 0;
}
//----------- End of function FirmCamp::mobilize_worker -----------//


//--------- Begin of function FirmCamp::defense ---------//
//### begin alex 15/10 ###//
//void FirmCamp::defense(short targetRecno)
void FirmCamp::defense(short targetRecno, int useRangeAttack)
//#### end alex 15/10 ####//
{
	//### begin alex 15/10 ###//
	//--******* BUGHERE , please provide a reasonable condition to set useRangeAttack to 1
	if(unit_array[targetRecno]->mobile_type!=UNIT_LAND)
		useRangeAttack = 1;
	else
		useRangeAttack = 0;
	//#### end alex 15/10 ####//

	if( !defense_flag )
		return;

	//--------------- define parameters ------------------//

	DefenseUnit *defPtr, *defPtr2;
	Unit *unitPtr;
	short unitRecno;
	int numOfUnitInside = worker_count + (overseer_recno>0);
	int i, j;

	if(employ_new_worker)
	{
		//---------- reset unit's parameters in the previous defense -----------//
		defPtr = defense_array;
		for(int i=0; i<=MAX_WORKER; i++, defPtr++)
		{
			if(defPtr->status==OUTSIDE_CAMP && defPtr->unit_recno && !unit_array.is_deleted(defPtr->unit_recno))
			{
				unitPtr = unit_array[defPtr->unit_recno];
				if(unitPtr->nation_recno==nation_recno && unitPtr->action_misc==ACTION_MISC_DEFENSE_CAMP_RECNO &&
					unitPtr->action_misc_para==firm_recno)
				{
					unitPtr->clear_unit_defense_mode();
					err_when(unitPtr->in_auto_defense_mode());
				}
			}
		}
		memset(defense_array, 0, sizeof(DefenseUnit)*(MAX_WORKER+1));
	}

	//------------------------------------------------------------------//
	// check all the exist(not dead) units outside the camp and arrange
	// them in the front part of the array.
	//------------------------------------------------------------------//
	j = 0;
	defPtr2 = defense_array;

	if(!employ_new_worker)	// some soliders may be outside the camp
	{
		for(i=0, defPtr=defense_array; i<=MAX_WORKER; i++, defPtr++)
		{
			if(!defPtr->unit_recno)
				continue;	// a free slot

			if(unit_array.is_deleted(defPtr->unit_recno))
			{
				defPtr->unit_recno = 0;
				continue;	// unit is dead
			}

			//----------------------------------------------------------------//
			// arrange the recno in the array front part
			//----------------------------------------------------------------//
			if(defPtr->status==OUTSIDE_CAMP)
			{
				unitPtr = unit_array[defPtr->unit_recno];

				//-------------- ignore this unit if it is dead --------------------//
				if(unitPtr->is_unit_dead())
					continue;

				if(!unitPtr->in_auto_defense_mode())
					continue; // the unit is ordered by the player to do other thing, so cannot control it afterwards

				//--------------- the unit is in defense mode ----------------//
				defPtr2->unit_recno = defPtr->unit_recno;
				defPtr2->status = OUTSIDE_CAMP;
				j++;
				defPtr2++;
			}
		}

		err_when(defPtr2 + (MAX_WORKER-j+1) > defense_array + MAX_WORKER + 1);
		memset(defPtr2, 0, sizeof(DefenseUnit)*(MAX_WORKER-j+1));
	}
	
	set_employ_worker(0);

	//------------------------------------------------------------------//
	// the unit outside the camp should be in defense mode and ready to
	// attack new target
	//------------------------------------------------------------------//
	for(i=0, defPtr=defense_array; i<j; i++, defPtr++)
	{
		//------------------------------------------------------------------//
		// order those unit outside the camp to attack the target
		//------------------------------------------------------------------//
		unitPtr = unit_array[defPtr->unit_recno];
		defense_outside_camp(defPtr->unit_recno, targetRecno);
		unitPtr->action_misc = ACTION_MISC_DEFENSE_CAMP_RECNO;
		unitPtr->action_misc_para = firm_recno;
	}

	int mobilizePos = 0;
	//### begin alex 13/10 ###//
	//for(; i<MAX_WORKER && worker_count; i++, defPtr++)
	for(; i<MAX_WORKER && mobilizePos<worker_count; i++, defPtr++)
	//#### end alex 13/10 ####//
	{
		err_when(mobilizePos >= worker_count);

		//------------------------------------------------------------------//
		// order those soldier inside the firm to move to target for attacking
		// keep those unable to attack inside the firm
		//------------------------------------------------------------------//
		//### begin alex 13/10 ###//
		//if(worker_array[mobilizePos].unit_id==UNIT_EXPLOSIVE_CART)
		if(worker_array[mobilizePos].unit_id==UNIT_EXPLOSIVE_CART ||
			(useRangeAttack && worker_array[mobilizePos].max_attack_range()==1))
		//#### end alex 13/10 ####//
		{
			mobilizePos++;
			continue;
		}
				
		unitRecno = mobilize_worker(mobilizePos+1, COMMAND_AUTO);
		//unitRecno = mobilize_worker(1, COMMAND_AUTO);        // always record 1 as the workers info are moved forward from the back to the front
		if(!unitRecno)
			break;
		
		Unit* unitPtr = unit_array[unitRecno];
		unitPtr->team_id = unit_array.cur_team_id;   // define it as a team
		unitPtr->action_misc = ACTION_MISC_DEFENSE_CAMP_RECNO;
		unitPtr->action_misc_para = firm_recno; // store the firm_recno for going back camp

		if(overseer_recno)
		{
			unitPtr->leader_unit_recno = overseer_recno;
			unitPtr->update_loyalty();	// update target loyalty based on having a leader assigned

			err_when( unit_array[overseer_recno]->rank_id != RANK_KING &&
					  unit_array[overseer_recno]->rank_id != RANK_GENERAL );
		}

		defense_inside_camp(unitRecno, targetRecno);
		defPtr->unit_recno = unitRecno;
		defPtr->status = OUTSIDE_CAMP;
	}

	/*if(overseer_recno>0)
	{
		//------------------------------------------------------------------//
		// order those overseer inside the firm to move to target for attacking
		//------------------------------------------------------------------//
		unitPtr = unit_array[overseer_recno];
		assign_overseer(0);
		unitPtr->team_id = unit_array.cur_team_id;   // define it as a team
		unitPtr->action_misc = ACTION_MISC_DEFENSE_CAMP_RECNO;
		unitPtr->action_misc_para = firm_recno; // store the firm_recno for going back camp

		defense_inside_camp(unitPtr->sprite_recno, targetRecno);
		defPtr->unit_recno = unitPtr->sprite_recno;
		defPtr->status = OUTSIDE_CAMP;
	}*/

	unit_array.cur_team_id++;
}
//----------- End of function FirmCamp::defense -----------//


//--------- Begin of function FirmCamp::defense_inside_camp ---------//
void FirmCamp::defense_inside_camp(short unitRecno, short targetRecno)
{
	Unit *unitPtr = unit_array[unitRecno];
	unitPtr->defense_attack_unit(targetRecno);
	//	err_when(unitPtr->action_mode2!=ACTION_AUTO_DEFENSE_ATTACK_TARGET);
	
	if(unitPtr->action_mode==ACTION_STOP && !unitPtr->action_para && unitPtr->action_x_loc==-1 && unitPtr->action_y_loc==-1)
		unitPtr->defense_detect_target();
}
//----------- End of function FirmCamp::defense_inside_camp -----------//


//--------- Begin of function FirmCamp::defense_outside_camp ---------//
void FirmCamp::defense_outside_camp(short unitRecno, short targetRecno)
{
	Unit *unitPtr = unit_array[unitRecno];
	
	if(unitPtr->action_mode2==ACTION_AUTO_DEFENSE_DETECT_TARGET || unitPtr->action_mode2==ACTION_AUTO_DEFENSE_BACK_CAMP ||
		(unitPtr->action_mode2==ACTION_AUTO_DEFENSE_ATTACK_TARGET && unitPtr->cur_action==SPRITE_IDLE))
	{
		//----------------- attack new target now -------------------//
		unitPtr->defense_attack_unit(targetRecno);
		err_when(unitPtr->action_mode2!=ACTION_AUTO_DEFENSE_ATTACK_TARGET && unitPtr->action_mode2!=ACTION_AUTO_DEFENSE_DETECT_TARGET);

		if(unitPtr->action_mode==ACTION_STOP && !unitPtr->action_para && unitPtr->action_x_loc==-1 && unitPtr->action_y_loc==-1)
			unitPtr->defense_detect_target();
	}
}
//----------- End of function FirmCamp::defense_outside_camp -----------//


//--------- Begin of function FirmCamp::set_employ_worker ---------//
void FirmCamp::set_employ_worker(char flag)
{
	employ_new_worker = flag;
	
	if(!flag)
		ai_status = CAMP_IN_DEFENSE;
	else
		ai_status = FIRM_WITHOUT_ACTION;
/*
	//------- a button should exist for accept new worker or not ---------//
	//-*********** codes here **********-//
	Town *townPtr;
	for(int i=0; i<linked_town_count; i++)
	{
		err_when(!linked_town_array[i] || town_array.is_deleted(linked_town_array[i]));
		townPtr = town_array[linked_town_array[i]];

		if(nation_recno == townPtr->nation_recno)
			toggle_town_link(i+1, employ_new_worker, COMMAND_AUTO); // enable links if employ_new_worker is true, otherwise disable
	}
*/
}
//----------- End of function FirmCamp::set_employ_worker -----------//


//--------- Begin of function FirmCamp::update_defense_unit ---------//
void FirmCamp::update_defense_unit(short unitRecno)
{
	DefenseUnit *defPtr = defense_array;
	int allInCamp = 1;
	int found=0;

	for(int i=0; i<=MAX_WORKER; i++, defPtr++)
	{
		if(!defPtr->unit_recno)
			continue; // empty slot

		if(unit_array.is_deleted(defPtr->unit_recno))
		{
			defPtr->unit_recno = 0;
			defPtr->status = INSIDE_CAMP;
			continue;
		}

		if(defPtr->unit_recno==unitRecno)
		{
			defPtr->unit_recno = 0;
			defPtr->status = INSIDE_CAMP;
			Unit *unitPtr = unit_array[unitRecno];
			unitPtr->stop2();
			unitPtr->reset_action_misc_para();
			err_when(unitPtr->in_auto_defense_mode());
			found++;
		}
		else
			allInCamp = 0; // some units are still outside camp
	}

	if(allInCamp)
	{
		set_employ_worker(1);
		memset(defense_array, 0, sizeof(DefenseUnit)*(MAX_WORKER+1));
	}

//	err_when(!found);				//**BUGHERE
}
//----------- End of function FirmCamp::update_defense_unit -----------//


//-------- Begin of function FirmCamp::is_worker_full ------//
//
int FirmCamp::is_worker_full()
{
	return worker_count + patrol_unit_count + coming_unit_count >= MAX_WORKER;
}
//----------- End of function FirmCamp::is_worker_full ---------//


#ifdef DEBUG

//----------- Begin of static function disp_debug_info -----------//

static void disp_debug_info(FirmCamp* firmPtr, int refreshFlag)
{
	if( refreshFlag == INFO_REPAINT )
		vga_util.d3_panel_up( INFO_X1, INFO_Y2-40, INFO_X2, INFO_Y2 );

	int x=INFO_X1+3, y=INFO_Y2-37, x2=x+120;

	font_san.field( x, y   , "patrol unit count", x2, firmPtr->patrol_unit_count, 1, INFO_X2, refreshFlag);
	font_san.field( x, y+16, "coming unit count", x2, firmPtr->coming_unit_count, 1, INFO_X2, refreshFlag);

	font_san.put( x+180, y, firmPtr->firm_recno );
}
//----------- End of static function disp_debug_info -----------//

#endif
