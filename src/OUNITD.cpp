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

//Filename    : OUNITD.CPP
//Description : Object Unit defense functions
//Owner		  : Alex

#include <ALL.h>
#include <OUNIT.h>
#include <OWORLD.h>
#include <OFIRM.h>
#include <ONATION.h>
#include <OTOWN.h>
#include <OF_CAMP.h>
#include <OF_MONS.h>

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

//=================================================================================================//
// Unit's defend mode generalized functions
//=================================================================================================//

//--------- Begin of function Unit::in_any_defense_mode ---------//
// check whether the unit is in defense mode
//
// return 1 if it is
// return 0 otherwise
//
int Unit::in_any_defense_mode()
{
	return (action_mode2>=ACTION_AUTO_DEFENSE_ATTACK_TARGET && action_mode2<=ACTION_MONSTER_DEFEND_BACK_FIRM);
}
//----------- End of function Unit::in_any_defense_mode -----------//


//--------- Begin of function Unit::general_defend_mode_detect_target ---------//
// call the appropriate defend function for the current type of defend
//
// <int> checkDefendMode	-	flag to check defend mode or not
//
void Unit::general_defend_mode_detect_target(int checkDefendMode)
{
	stop();
	switch(action_mode2)
	{
		case ACTION_AUTO_DEFENSE_ATTACK_TARGET:
				defense_detect_target();
				break;

		case ACTION_DEFEND_TOWN_ATTACK_TARGET:
				defend_town_detect_target();
				break;

		case ACTION_MONSTER_DEFEND_ATTACK_TARGET:
				monster_defend_detect_target();
				break;

		default: if(checkDefendMode)
						err_here();
					break;
	}
}
//----------- End of function Unit::general_defend_mode_detect_target -----------//


//--------- Begin of function Unit::general_defend_mode_process_attack_target ---------//
// process unit defense action. If target is dead, action_mode changes to detect_mode
//
// return 1 if action mode changes to detect mode
// return 0 otherwise
//
int Unit::general_defend_mode_process_attack_target()
{
	Location *locPtr;
	Unit *unitPtr;
	Town *townPtr;
	Firm *firmPtr;
	SpriteInfo	*spriteInfo;
	FirmInfo		*firmInfo;
	int clearToDetect = 0;

	//------------------------------------------------------------------------------//
	// if the unit's action mode is in defensive attack action, process the corresponding
	// checking.
	//------------------------------------------------------------------------------//
	switch(action_mode)
	{
		case ACTION_ATTACK_UNIT:
				if(unit_array.is_deleted(action_para2))
					clearToDetect++;
				else
				{
					err_when(unit_array.is_deleted(action_para2));
					unitPtr = unit_array[action_para2];
					
					//if(unitPtr->cur_action==SPRITE_IDLE)
					//	clearToDetect++;

					if(!nation_can_attack(unitPtr->nation_recno)) // cannot attack this nation
						clearToDetect++;
				}
				break;

		case ACTION_ATTACK_FIRM:
				if(firm_array.is_deleted(action_para2))
					clearToDetect++;
				else
				{
					err_when(firm_array.is_deleted(action_para2));
					firmPtr = firm_array[action_para2];
					
					if(!nation_can_attack(firmPtr->nation_recno)) // cannot attack this nation
						clearToDetect++;
				}
				break;

		case ACTION_ATTACK_TOWN:
				if(town_array.is_deleted(action_para2))
					clearToDetect++;
				else
				{
					err_when(town_array.is_deleted(action_para2));
					townPtr = town_array[action_para2];
					
					if(!nation_can_attack(townPtr->nation_recno)) // cannot attack this nation
						clearToDetect++;
				}
				break;

		case ACTION_ATTACK_WALL:
				locPtr = world.get_loc(action_x_loc2, action_y_loc2);
				
				if(!locPtr->is_wall() || !nation_can_attack(locPtr->power_nation_recno))
					clearToDetect++;
				break;

		default: clearToDetect++;
					break;
	}

	//------------------------------------------------------------------------------//
	// suitation changed to defensive detecting mode
	//------------------------------------------------------------------------------//
	if(clearToDetect)
	{
		//----------------------------------------------------------//
		// target is dead, change to detect state for another target
		//----------------------------------------------------------//
		reset_action_para();
		return 1;
	}
	else if(waiting_term<ATTACK_WAITING_TERM)
		waiting_term++;
	else
	{
		//------------------------------------------------------------------------------//
		// process the corresponding attacking procedure.
		//------------------------------------------------------------------------------//
		waiting_term = 0;
		switch(action_mode)
		{
			case ACTION_ATTACK_UNIT:
				err_when(unit_array.is_deleted(action_para2) || !unitPtr);
				spriteInfo = unitPtr->sprite_info;
				
				//-----------------------------------------------------------------//
				// attack the target if able to reach the target surrounding, otherwise
				// continue to wait
				//-----------------------------------------------------------------//
				action_x_loc2 = unitPtr->next_x_loc(); // update target location
				action_y_loc2 = unitPtr->next_y_loc();
				if(space_for_attack(action_x_loc2, action_y_loc2, unitPtr->mobile_type, spriteInfo->loc_width, spriteInfo->loc_height))
					attack_unit(unitPtr->sprite_recno);
				break;

			case ACTION_ATTACK_FIRM:
				err_when(firm_array.is_deleted(action_para2) || !firmPtr);
				firmInfo = firm_res[firmPtr->firm_id];
				
				//-----------------------------------------------------------------//
				// attack the target if able to reach the target surrounding, otherwise
				// continue to wait
				//-----------------------------------------------------------------//
				attack_firm(action_x_loc2, action_y_loc2);

				if(!is_in_surrounding(move_to_x_loc, move_to_y_loc, sprite_info->loc_width, action_x_loc2, action_y_loc2,
					firmInfo->loc_width, firmInfo->loc_height))
					waiting_term = 0;
				break;

			case ACTION_ATTACK_TOWN:
				err_when(town_array.is_deleted(action_para2) || !townPtr);

				//-----------------------------------------------------------------//
				// attack the target if able to reach the target surrounding, otherwise
				// continue to wait
				//-----------------------------------------------------------------//
				attack_town(action_x_loc2, action_y_loc2);

				if(!is_in_surrounding(move_to_x_loc, move_to_y_loc, sprite_info->loc_width, action_x_loc2, action_y_loc2,
					STD_TOWN_LOC_WIDTH, STD_TOWN_LOC_HEIGHT))
					waiting_term = 0;
				break;

			case ACTION_ATTACK_WALL:
				err_when(action_para || action_para2);

				attack_wall(action_x_loc2, action_y_loc2);
				if(!is_in_surrounding(move_to_x_loc, move_to_y_loc, sprite_info->loc_width, action_x_loc2, action_y_loc2, 1, 1))
					waiting_term = 0;
				break;

			default:
				err_here();
				break;
		}
	}

	return 0;
}
//----------- End of function Unit::general_defend_mode_process_attack_target -----------//


//=================================================================================================//
// Unit's defense mode
//=================================================================================================//

//--------- Begin of function Unit::defense_attack_unit ---------//
// defensive attack units
//
// <short>	targetRecno	-	recno of the target
//
void Unit::defense_attack_unit(short targetRecno)
{
	action_mode2 = ACTION_AUTO_DEFENSE_ATTACK_TARGET;
	attack_unit(targetRecno);
}
//----------- End of function Unit::defense_attack_unit -----------//


//--------- Begin of function Unit::defense_attack_firm ---------//
// defensive attack firm
//
// <short>	targetXLoc	-	x location of the firm	
// <short>	targetYLoc	-	y location of the firm
//
void Unit::defense_attack_firm(int targetXLoc, int targetYLoc)
{
	action_mode2 = ACTION_AUTO_DEFENSE_ATTACK_TARGET;
	attack_firm(targetXLoc, targetYLoc);
}
//----------- End of function Unit::defense_attack_firm -----------//


//--------- Begin of function Unit::defense_attack_town ---------//
// defensive attack town
//
// <short>	targetXLoc	-	x location of the town
// <short>	targetYLoc	-	y location of the town
//
void Unit::defense_attack_town(int targetXLoc, int targetYLoc)
{
	action_mode2 = ACTION_AUTO_DEFENSE_ATTACK_TARGET;
	attack_town(targetXLoc, targetYLoc);
}
//----------- End of function Unit::defense_attack_town -----------//


//--------- Begin of function Unit::defense_attack_wall ---------//
// defensive attack wall
//
// <short>	targetXLoc	-	x location of wall
// <short>	targetYLoc	-	y location of wall
//
void Unit::defense_attack_wall(int targetXLoc, int targetYLoc)
{
	action_mode2 = ACTION_AUTO_DEFENSE_ATTACK_TARGET;
	attack_wall(targetXLoc, targetYLoc);
}
//----------- End of function Unit::defense_attack_wall -----------//


//--------- Begin of function Unit::defense_detect_target ---------//
// set parameters for unit's defensive mode
//
void Unit::defense_detect_target()
{
	action_mode2 = ACTION_AUTO_DEFENSE_DETECT_TARGET;
	action_para2 = AUTO_DEFENSE_DETECT_COUNT;
	action_x_loc2 = -1;
	action_y_loc2 = -1;
}
//----------- End of function Unit::defense_detect_target -----------//


//--------- Begin of function Unit::defense_back_camp ---------//
// set parameters for unit's to return camp
//
// <int>	firmXLoc	-	x location of the firm
// <int>	firmYLoc -	y location of the firm
//
void Unit::defense_back_camp(int firmXLoc, int firmYLoc)
{
	err_when(firm_array[world.get_loc(firmXLoc, firmYLoc)->firm_recno()]->firm_id!=FIRM_CAMP);
	err_when(firm_array[world.get_loc(firmXLoc, firmYLoc)->firm_recno()]->firm_recno!=action_misc_para);
	
	assign(firmXLoc, firmYLoc);
	action_mode2 = ACTION_AUTO_DEFENSE_BACK_CAMP;
}
//----------- End of function Unit::defense_back_camp -----------//


//--------- Begin of function Unit::process_auto_defense_attack_target ---------//
// process the action for unit's defensive attack
//
void Unit::process_auto_defense_attack_target()
{
	err_when(action_mode2!=ACTION_AUTO_DEFENSE_ATTACK_TARGET);
	if(general_defend_mode_process_attack_target())
	{
		defense_detect_target();
		err_when(action_mode!=ACTION_STOP || action_para || action_x_loc!=-1 || action_y_loc!=-1 || cur_action==SPRITE_ATTACK);
	}
}
//----------- End of function Unit::process_auto_defense_attack_target -----------//


//--------- Begin of function Unit::process_auto_defense_detect_target ---------//
// process action for unit's defensive detecet target
//
void Unit::process_auto_defense_detect_target()
{
	err_when(action_mode2!=ACTION_AUTO_DEFENSE_DETECT_TARGET);
	err_when(action_mode!=ACTION_STOP);
	err_when(action_para!=0);
	err_when(action_x_loc!=-1 || action_y_loc!=-1 || action_x_loc!=-1 || action_y_loc2!=-1);

	//----------------------------------------------------------------//
	// no target or target is out of detect range, so change state to
	// back camp
	//----------------------------------------------------------------//
	if(!action_para2)
	{
		err_when(action_misc!=ACTION_MISC_DEFENSE_CAMP_RECNO || !action_misc_para);

		if(firm_array.is_deleted(action_misc_para))
		{
			process_auto_defense_back_camp();
			return;
		}

		Firm *firmPtr = firm_array[action_misc_para];
		if(firmPtr->firm_id!=FIRM_CAMP || firmPtr->nation_recno!=nation_recno)
		{
			process_auto_defense_back_camp();
			return;
		}

		FirmCamp *campPtr = firmPtr->cast_to_FirmCamp();
		campPtr = (FirmCamp*) firm_array[action_misc_para];
		if(unit_array.is_deleted(campPtr->defend_target_recno))
		{
			process_auto_defense_back_camp();
			return;
		}

		Unit *targetPtr = unit_array[campPtr->defend_target_recno];
		if(targetPtr->action_mode!=ACTION_ATTACK_FIRM || targetPtr->action_para!=campPtr->firm_recno)
		{
			process_auto_defense_back_camp();
			return;
		}

		//action_mode2 = ACTION_AUTO_DEFENSE_DETECT_TARGET;
		action_para2 = AUTO_DEFENSE_DETECT_COUNT;
		return;
	}

	//----------------------------------------------------------------//
	// defense_detecting target algorithm
	//----------------------------------------------------------------//
	int startLoc;
	int dimension;

	switch(action_para2%GAME_FRAMES_PER_DAY)
	{
		case 3:	startLoc = 2;// 1-7, check 224 = 15^2-1
					//### begin alex 3/10 ###//
					dimension = 7;
					//#### end alex 3/10 ####//
					break;

		case 2:	startLoc = 122;// 6-8, check 168 = 17^2-11^2
					dimension = 8;
					break;

		case 1:	startLoc = 170;// 7-9, check 192 = 19^2-13^2
					dimension = EFFECTIVE_AUTO_DEFENSE_DISTANCE;
					break;

		default: action_para2--;
					return;
	}

	//---------------------------------------------------------------//
	// attack the target if target detected, or change the detect region
	//---------------------------------------------------------------//
	if(!idle_detect_attack(startLoc, dimension, 1)) // defense mode is on
		action_para2--;
}
//----------- End of function Unit::process_auto_defense_detect_target -----------//


//--------- Begin of function Unit::process_auto_defense_back_camp ---------//
// process action for the units to return camp
//
void Unit::process_auto_defense_back_camp()
{
	int	clearDefenseMode = 0;
	if(action_mode!=ACTION_ASSIGN_TO_FIRM) // the unit may become idle or unable to reach firm, reactivate it
	{
		Firm	*firmPtr;
		if(action_misc!=ACTION_MISC_DEFENSE_CAMP_RECNO || !action_misc_para || firm_array.is_deleted(action_misc_para))
			clearDefenseMode++;
		else
		{
			firmPtr = firm_array[action_misc_para];
			if(firmPtr->firm_id!=FIRM_CAMP || firmPtr->nation_recno!=nation_recno)
				clearDefenseMode++;
			else
			{
				defense_back_camp(firmPtr->loc_x1, firmPtr->loc_y1); // go back to the military camp
				err_when(action_mode2!=ACTION_AUTO_DEFENSE_BACK_CAMP);
				return;
			}
		}
	}
	else if(cur_action==SPRITE_IDLE)
	{
		if(firm_array.is_deleted(action_misc_para))
			clearDefenseMode++;
		else
		{
			Firm *firmPtr = firm_array[action_misc_para];
			defense_back_camp(firmPtr->loc_x1, firmPtr->loc_y1);
			err_when(action_mode2!=ACTION_AUTO_DEFENSE_BACK_CAMP);
			return;
		}
	}

	err_when(!clearDefenseMode);
	//----------------------------------------------------------------//
	// clear order if the camp is deleted
	//----------------------------------------------------------------//
	stop2();
	reset_action_misc_para();
	err_when(in_auto_defense_mode());
}
//----------- End of function Unit::process_auto_defense_back_camp -----------//


//------------- Begin of function Unit::in_auto_defense_mode --------------//
// check whether the units in auto defense mode
//
int Unit::in_auto_defense_mode()
{
	return (action_mode2>=ACTION_AUTO_DEFENSE_ATTACK_TARGET && action_mode2<=ACTION_AUTO_DEFENSE_BACK_CAMP);
}
//----------- End of function Unit::in_auto_defense_mode -----------//


//------------- Begin of function Unit::defense_follow_target --------------//
// decide whether to follow the target
//
// return 0 if aborting attack the current target and go back to military camp
// return 1 otherwise
//
int Unit::defense_follow_target()
{
	#define PROB_HOSTILE_RETURN	10
	#define PROB_FRIENDLY_RETURN	20
	#define PROB_NEUTRAL_RETURN	30
	
	if(unit_array.is_deleted(action_para))
		return 1;

	if(cur_action==SPRITE_ATTACK)
		return 1;

	Unit *targetPtr = unit_array[action_para];
	Location *locPtr = world.get_loc(action_x_loc, action_y_loc);
	if(!locPtr->has_unit(targetPtr->mobile_type))
		return 1; // the target may be dead or invisible
	
	int returnFactor, abortAction = 0;
	
	//-----------------------------------------------------------------//
	// calculate the chance to go back to military camp in following the
	// target
	//-----------------------------------------------------------------//
	if(locPtr->power_nation_recno==nation_recno)
		return 1; // target within our nation
	else if(!locPtr->power_nation_recno) // is neutral
		returnFactor = PROB_NEUTRAL_RETURN;
	else
	{
		Nation *locNationPtr = nation_array[locPtr->power_nation_recno];
		if(locNationPtr->get_relation_status(nation_recno)==NATION_HOSTILE)
			returnFactor = PROB_HOSTILE_RETURN;
		else
			returnFactor = PROB_FRIENDLY_RETURN;
	}

	if(!abortAction)
	{
		SpriteInfo *targetSpriteInfo = targetPtr->sprite_info;

		//-----------------------------------------------------------------//
		// if the target moves faster than this unit, it is more likely for
		// this unit to go back to military camp.
		//-----------------------------------------------------------------//
		//-**** should also consider the combat level and hit_points of both unit ****-//
		if(targetSpriteInfo->speed > sprite_info->speed)
			returnFactor -= 5;

		if(misc.random(returnFactor)==0) // return to camp if true
			abortAction++;
		else
			return 1;
	}

	err_when(!abortAction);
	err_when(action_mode==ACTION_ASSIGN_TO_FIRM); // if so, process_auto_defense_back_camp() cannot process successfully
	process_auto_defense_back_camp();	
	return 0; // cancel attack
}
//----------- End of function Unit::defense_follow_target -----------//


//------------- Begin of function Unit::clear_unit_defense_mode --------------//
// clear defensive mode
//
void Unit::clear_unit_defense_mode()
{
	//------- cancel defense mode and continue the current action -------//
	action_mode2 = action_mode;
	action_para2 = action_para;
	action_x_loc2 = action_x_loc;
	action_y_loc2 = action_y_loc;

	reset_action_misc_para();

	if( unit_mode == UNIT_MODE_DEFEND_TOWN )
		set_mode(0);		// reset unit mode 
}
//----------------- End of function Unit::clear_unit_defense_mode ----------------//


//=================================================================================================//
// Town unit's defend mode, eg rebel
//=================================================================================================//

//--------- Begin of function Unit::defend_town_attack_unit ---------//
// set to defensive mode
//
// <short>	targetRecno	-	record of target
//
void Unit::defend_town_attack_unit(short targetRecno)
{
	action_mode2 = ACTION_DEFEND_TOWN_ATTACK_TARGET;
	attack_unit(targetRecno);
}
//----------- End of function Unit::defend_town_attack_unit -----------//


//------------- Begin of function Unit::defend_town_detect_target --------------//
// set to detect mode
//
void Unit::defend_town_detect_target()
{
	action_mode2 = ACTION_DEFEND_TOWN_DETECT_TARGET;
	action_para2 = UNIT_DEFEND_TOWN_DETECT_COUNT;
	action_x_loc2 = -1;
	action_y_loc2 = -1;
}
//----------- End of function Unit::defend_town_detect_target -----------//


//------------- Begin of function Unit::process_defend_town_attack_target --------------//
// defend town units is only allowed to attack unit
//
void Unit::process_defend_town_attack_target()
{
	err_when(action_mode2!=ACTION_DEFEND_TOWN_ATTACK_TARGET);
	if(general_defend_mode_process_attack_target())
	{
		action_mode2 = ACTION_DEFEND_TOWN_DETECT_TARGET;
		action_para2 = UNIT_DEFEND_TOWN_DETECT_COUNT;
		action_x_loc2 = action_y_loc2 = -1;
		err_when(action_mode!=ACTION_STOP || action_para || action_x_loc!=-1 || action_y_loc!=-1 || cur_action==SPRITE_ATTACK);
	}
}
//----------------- End of function Unit::process_defend_town_attack_target ----------------//


//------------- Begin of function Unit::process_defend_town_detect_target --------------//
// process detect mode
//
void Unit::process_defend_town_detect_target()
{
	err_when(action_mode2!=ACTION_DEFEND_TOWN_DETECT_TARGET);
	err_when(action_mode!=ACTION_STOP);
	err_when(action_para!=0);
	err_when(action_x_loc!=-1 || action_y_loc!=-1 || action_x_loc2!=-1 || action_y_loc2!=-1);

	//----------------------------------------------------------------//
	// no target or target is out of detect range, so change state to
	// back camp
	//----------------------------------------------------------------//
	if(!action_para2)
	{
		err_when(action_misc!=ACTION_MISC_DEFEND_TOWN_RECNO || !action_misc_para);
		err_when(unit_mode!=UNIT_MODE_DEFEND_TOWN || !unit_mode_para);
		int back = 0;
		Town *townPtr;
		Unit *targetPtr;

		if(town_array.is_deleted(action_misc_para))
			back++;
		else
		{
			townPtr = town_array[action_misc_para];
			if(unit_array.is_deleted(townPtr->defend_target_recno))
				back++;
			else
			{
				targetPtr = unit_array[townPtr->defend_target_recno];
				if(targetPtr->action_mode!=ACTION_ATTACK_TOWN || targetPtr->action_para!=townPtr->town_recno)
					back++;
			}
		}

		if(!back)
		{
			//action_mode2 = ACTION_DEFEND_TOWN_DETECT_TARGET;
			action_para2 = UNIT_DEFEND_TOWN_DETECT_COUNT;
			return;
		}

		process_defend_town_back_town();
		return;
	}

	//----------------------------------------------------------------//
	// defense_detecting target algorithm
	//----------------------------------------------------------------//
	int startLoc;
	int dimension;

	switch(action_para2%GAME_FRAMES_PER_DAY)
	{
		case 3:	startLoc = 2;// 1-7, check 224 = 15^2-1
					//### begin alex 3/10 ###//
					dimension = 7;
					//#### end alex 3/10 ####//
					break;

		case 2:	startLoc = 122;// 6-8, check 168 = 17^2-11^2
					dimension = 8;
					break;

		case 1:	startLoc = 170;// 7-9, check 192 = 19^2-13^2
					dimension = EFFECTIVE_DEFEND_TOWN_DISTANCE;
					break;

		default: action_para2--;
					return;
	}

	//---------------------------------------------------------------//
	// attack the target if target detected, or change the detect region
	//---------------------------------------------------------------//

	if(!idle_detect_attack(startLoc, dimension, 1)) // defense mode is on
		action_para2--;
}
//----------------- End of function Unit::process_defend_town_detect_target ----------------//


//------------- Begin of function Unit::process_defend_town_back_town --------------//
// process action to go back town
//
void Unit::process_defend_town_back_town()
{
	int	clearDefenseMode = 0;
	if(action_mode!=ACTION_ASSIGN_TO_TOWN) // the unit may become idle or unable to reach town, reactivate it
	{
		Town	*townPtr;
		if(action_misc!=ACTION_MISC_DEFEND_TOWN_RECNO || !action_misc_para || town_array.is_deleted(action_misc_para))
			clearDefenseMode++;
		else
		{
			townPtr = town_array[action_misc_para];
			if(townPtr->nation_recno!=nation_recno)
				clearDefenseMode++;
			else
			{
				defend_town_back_town(action_misc_para); // go back to the town
				err_when(action_mode2!=ACTION_DEFEND_TOWN_BACK_TOWN);
				return;
			}
		}
	}
	else if(cur_action==SPRITE_IDLE)
	{
		if(town_array.is_deleted(action_misc_para))
			clearDefenseMode++;
		else
		{
			defend_town_back_town(action_misc_para);
			err_when(action_mode2!=ACTION_DEFEND_TOWN_BACK_TOWN);
			return;
		}
	}

	err_when(!clearDefenseMode);
	//----------------------------------------------------------------//
	// clear order if the town is deleted
	//----------------------------------------------------------------//
	stop2();
	reset_action_misc_para();
	err_when(in_defend_town_mode());
}
//----------------- End of function Unit::process_defend_town_back_town ----------------//


//------------- Begin of function Unit::in_defend_town_mode --------------//
// check whether the unit is in defend town mode
//
int Unit::in_defend_town_mode()
{
	return (action_mode2>=ACTION_DEFEND_TOWN_ATTACK_TARGET && action_mode2<=ACTION_DEFEND_TOWN_BACK_TOWN);
}
//----------------- End of function Unit::in_defend_town_mode ----------------//


//--------- Begin of function Unit::defend_town_back_town ---------//
// set action to back town
//
// <int>	townRecno	-	recno of the town
//
void Unit::defend_town_back_town(short townRecno)
{
	err_when(town_array.is_deleted(townRecno));
	Town *townPtr = town_array[townRecno];
	
	assign(townPtr->loc_x1, townPtr->loc_y1);
	action_mode2 = ACTION_DEFEND_TOWN_BACK_TOWN;
}
//----------- End of function Unit::defend_town_back_town -----------//


//------------- Begin of function Unit::defend_town_follow_target --------------//
// check the unit should follow the target or not
//
// return 0 if aborting attack the current target and go back to military camp
// return 1 otherwise
//
int Unit::defend_town_follow_target()
{
	err_when(unit_mode!=UNIT_MODE_DEFEND_TOWN);

	if(cur_action==SPRITE_ATTACK)
		return 1;

	if(town_array.is_deleted(unit_mode_para))
	{
		stop2(); //**** BUGHERE
		set_mode(0); //***BUGHERE
		return 0;
	}

	int curXLoc = next_x_loc();
	int curYLoc = next_y_loc();

	Town *townPtr = town_array[unit_mode_para];
	if((curXLoc<townPtr->center_x-UNIT_DEFEND_TOWN_DISTANCE) || (curXLoc>townPtr->center_x+UNIT_DEFEND_TOWN_DISTANCE) ||
		(curYLoc<townPtr->center_y-UNIT_DEFEND_TOWN_DISTANCE) || (curYLoc>townPtr->center_y+UNIT_DEFEND_TOWN_DISTANCE))
	{
		defend_town_back_town(unit_mode_para);
		return 0;
	}

	return 1;
}
//----------- End of function Unit::defend_town_follow_target -----------//


//------------- Begin of function Unit::clear_town_defend_mode --------------//
// clear defend mode
//
void Unit::clear_town_defend_mode()
{
	//------- cancel defense mode and continue the current action -------//
	action_mode2 = action_mode;
	action_para2 = action_para;
	action_x_loc2 = action_x_loc;
	action_y_loc2 = action_y_loc;

	reset_action_misc_para();
}
//----------------- End of function Unit::clear_town_defend_mode ----------------//


//=================================================================================================//
// Monster unit's defend mode
//=================================================================================================//

//--------- Begin of function Unit::monster_defend_attack_unit ---------//
// set to attack target
//
// <short> targetRecno	-	recno of the target
//
void Unit::monster_defend_attack_unit(short targetRecno)
{
	action_mode2 = ACTION_MONSTER_DEFEND_ATTACK_TARGET;
	attack_unit(targetRecno);
}
//----------- End of function Unit::monster_defend_attack_unit -----------//


//--------- Begin of function Unit::monster_defend_attack_firm ---------//
// set monster to attack firm
//
// <int>	targetXLoc	-	x location of firm
// <int>	targetYLoc	-	y location of firm
//
void Unit::monster_defend_attack_firm(int targetXLoc, int targetYLoc)
{
	action_mode2 = ACTION_MONSTER_DEFEND_ATTACK_TARGET;
	attack_firm(targetXLoc, targetYLoc);
}
//----------- End of function Unit::monster_defend_attack_firm -----------//


//--------- Begin of function Unit::monster_defend_attack_town ---------//
// set monster to attack town
//
// <int>	targetXLoc	-	x location of the town
// <int>	targetYLoc	-	y location of the town
//
void Unit::monster_defend_attack_town(int targetXLoc, int targetYLoc)
{
	action_mode2 = ACTION_MONSTER_DEFEND_ATTACK_TARGET;
	attack_town(targetXLoc, targetYLoc);
}
//----------- End of function Unit::monster_defend_attack_town -----------//


//--------- Begin of function Unit::monster_defend_attack_wall ---------//
// set monster to attack wall
//
// <int>	targetXLoc	-	x location of wall
// <int>	targetYLoc	-	y location of wall
//
void Unit::monster_defend_attack_wall(int targetXLoc, int targetYLoc)
{
	action_mode2 = ACTION_MONSTER_DEFEND_ATTACK_TARGET;
	attack_wall(targetXLoc, targetYLoc);
}
//----------- End of function Unit::monster_defend_attack_wall -----------//


//--------- Begin of function Unit::monster_defend_detect_target ---------//
// set to detect mode
//
void Unit::monster_defend_detect_target()
{
	action_mode2 = ACTION_MONSTER_DEFEND_DETECT_TARGET;
	action_para2 = MONSTER_DEFEND_DETECT_COUNT;
	action_x_loc2 = -1;
	action_y_loc2 = -1;
}
//----------- End of function Unit::monster_defend_detect_target -----------//


//--------- Begin of function Unit::monster_defend_back_firm ---------//
// set to return mode
//
void Unit::monster_defend_back_firm(int firmXLoc, int firmYLoc)
{
	err_when(firm_array[world.get_loc(firmXLoc, firmYLoc)->firm_recno()]->firm_id!=FIRM_MONSTER);
	
	assign(firmXLoc, firmYLoc);
	action_mode2 = ACTION_MONSTER_DEFEND_BACK_FIRM;
}
//----------- End of function Unit::monster_defend_back_firm -----------//


//--------- Begin of function Unit::process_monster_defend_attack_target ---------//
// proces attack mode
//
void Unit::process_monster_defend_attack_target()
{
	err_when(action_mode2!=ACTION_MONSTER_DEFEND_ATTACK_TARGET);
	if(general_defend_mode_process_attack_target())
	{
		monster_defend_detect_target();
		err_when(action_mode!=ACTION_STOP || action_para || action_x_loc!=-1 || action_y_loc!=-1 || cur_action==SPRITE_ATTACK);
	}
}
//----------- End of function Unit::process_monster_defend_attack_target -----------//


//--------- Begin of function Unit::process_monster_defend_detect_target ---------//
// process detect mode
//
void Unit::process_monster_defend_detect_target()
{
	err_when(action_mode2!=ACTION_MONSTER_DEFEND_DETECT_TARGET);
	err_when(action_mode!=ACTION_STOP);
	err_when(action_para!=0);
	err_when(action_x_loc!=-1 || action_y_loc!=-1 || action_x_loc!=-1 || action_y_loc2!=-1);

	//----------------------------------------------------------------//
	// no target or target is out of detect range, so change state to
	// back camp
	//----------------------------------------------------------------//
	if(!action_para2)
	{
		err_when(action_misc!=ACTION_MISC_MONSTER_DEFEND_FIRM_RECNO || !action_misc_para);
		err_when(unit_mode!=UNIT_MODE_MONSTER || !unit_mode_para);
		int back = 0;
		Unit *targetPtr;
		FirmMonster *firmMonsterPtr;

		if(firm_array.is_deleted(action_misc_para))
			back++;
		else
		{
			firmMonsterPtr = (FirmMonster*) firm_array[action_misc_para];
			if(unit_array.is_deleted(firmMonsterPtr->defend_target_recno))
				back++;
			else
			{
				targetPtr = unit_array[firmMonsterPtr->defend_target_recno];
				if(targetPtr->action_mode!=ACTION_ATTACK_FIRM || targetPtr->action_para!=firmMonsterPtr->firm_recno)
					back++;
			}
		}

		if(!back)
		{
			//action_mode2 = ACTION_MONSTER_DEFEND_DETECT_TARGET;
			action_para2 = MONSTER_DEFEND_DETECT_COUNT;
			return;
		}

		process_monster_defend_back_firm();
		return;
	}

	//----------------------------------------------------------------//
	// defense_detecting target algorithm
	//----------------------------------------------------------------//
	int startLoc;
	int dimension;

	switch(action_para2%GAME_FRAMES_PER_DAY)
	{
		case 3:	startLoc = 2;// 1-7, check 224 = 15^2-1
					//### begin alex 3/10 ###//
					dimension = 7;
					//#### end alex 3/10 ####//
					break;

		case 2:	startLoc = 122;// 6-8, check 168 = 17^2-11^2
					dimension = 8;
					break;

		case 1:	startLoc = 170;// 7-9, check 192 = 19^2-13^2
					dimension = EFFECTIVE_MONSTER_DEFEND_FIRM_DISTANCE;
					break;

		default: action_para2--;
					return;
	}

	//---------------------------------------------------------------//
	// attack the target if target detected, or change the detect region
	//---------------------------------------------------------------//
	if(!idle_detect_attack(startLoc, dimension, 1)) // defense mode is on
		action_para2--;
}
//----------- End of function Unit::process_monster_defend_detect_target -----------//


//--------- Begin of function Unit::process_monster_defend_back_firm ---------//
// process return mode
//
void Unit::process_monster_defend_back_firm()
{
	int	clearDefendMode = 0;
	if(action_mode!=ACTION_ASSIGN_TO_FIRM) // the unit may become idle or unable to reach firm, reactivate it
	{
		Firm	*firmPtr;
		if(action_misc!=ACTION_MISC_MONSTER_DEFEND_FIRM_RECNO || !action_misc_para || firm_array.is_deleted(action_misc_para))
			clearDefendMode++;
		else
		{
			firmPtr = firm_array[action_misc_para];
			if(firmPtr->firm_id!=FIRM_MONSTER || firmPtr->nation_recno!=nation_recno)
				clearDefendMode++;
			else
			{
				monster_defend_back_firm(firmPtr->loc_x1, firmPtr->loc_y1); // go back to the military camp
				err_when(action_mode2!=ACTION_MONSTER_DEFEND_BACK_FIRM);
				return;
			}
		}
	}
	else if(cur_action==SPRITE_IDLE)
	{
		if(firm_array.is_deleted(action_misc_para))
			clearDefendMode++;
		else
		{
			Firm *firmPtr = firm_array[action_misc_para];
			monster_defend_back_firm(firmPtr->loc_x1, firmPtr->loc_y1);
			err_when(action_mode2!=ACTION_MONSTER_DEFEND_BACK_FIRM);
			return;
		}
	}

	err_when(!clearDefendMode);
	//----------------------------------------------------------------//
	// clear order if the camp is deleted
	//----------------------------------------------------------------//
	stop2();
	reset_action_misc_para();
	err_when(in_monster_defend_mode());
}
//----------- End of function Unit::process_monster_defend_back_firm -----------//


//------------- Begin of function Unit::in_monster_defend_mode --------------//
// check whether the unit is in defend mode
//
int Unit::in_monster_defend_mode()
{
	return (action_mode2>=ACTION_MONSTER_DEFEND_ATTACK_TARGET && action_mode2<=ACTION_MONSTER_DEFEND_BACK_FIRM);
}
//----------- End of function Unit::in_monster_defend_mode -----------//


//------------- Begin of function Unit::monster_defend_follow_target --------------//
// make decision to choose to follow target or return
//
// return 0 if aborting attack the current target and go back to military camp
// return 1 otherwise
//
int Unit::monster_defend_follow_target()
{
//######## begin trevor 22/8 ##########//
	err_when( unit_mode!=UNIT_MODE_MONSTER || !unit_mode_para );

	if(cur_action==SPRITE_ATTACK)
		return 1;
/*
	if(firm_array.is_deleted(action_misc_para))
	{
		stop2(); //**** BUGHERE
		//set_mode(0); //***BUGHERE
		if(monster_array.is_deleted(unit_mode_para))
			return 0;

		Monster *monsterPtr = monster_array[unit_mode_para];
		monsterPtr->firm_recno = 0;
		return 0;
	}
*/
//######## end trevor 22/8 ##########//

	//--------------------------------------------------------------------------------//
	// choose to return to firm
	//--------------------------------------------------------------------------------//
	int curXLoc = next_x_loc();
	int curYLoc = next_y_loc();

	Firm *firmPtr = firm_array[action_misc_para];
	if((curXLoc<firmPtr->center_x-MONSTER_DEFEND_FIRM_DISTANCE) || (curXLoc>firmPtr->center_x+MONSTER_DEFEND_FIRM_DISTANCE) ||
		(curYLoc<firmPtr->center_y-MONSTER_DEFEND_FIRM_DISTANCE) || (curYLoc>firmPtr->center_y+MONSTER_DEFEND_FIRM_DISTANCE))
	{
		monster_defend_back_firm(firmPtr->loc_x1, firmPtr->loc_y1);
		return 0;
	}

	return 1;
}
//----------- End of function Unit::monster_defend_follow_target -----------//


//------------- Begin of function Unit::clear_monster_defend_mode --------------//
// clear defend mode
//
void Unit::clear_monster_defend_mode()
{
	err_when(!in_monster_defend_mode());

	//------- cancel defense mode and continue the current action -------//
	action_mode2 = action_mode;
	action_para2 = action_para;
	action_x_loc2 = action_x_loc;
	action_y_loc2 = action_y_loc;

	reset_action_misc_para();
}
//----------------- End of function Unit::clear_monster_defend_mode ----------------//
