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

//Filename:    OUNIT2.CPP
//Description: Unit functions

#include <OWORLD.h>
#include <ONATION.h>
#include <OFIRM.h>
#include <OTOWN.h>
#include <OUNIT.h>


//------- Begin of function Unit::think_aggressive_action --------//
//
// This function is called when the unit is in aggressive mode.
//
int Unit::think_aggressive_action()
{
	//------ think about resuming the original action -----//

	if( original_action_mode && cur_action!=SPRITE_ATTACK )	// if it is currently attacking somebody, don't think about resuming the original action
	{
		return think_resume_original_action();
	}

	//---- think about attacking nearby units if this unit is attacking a town or a firm ---//

	if( aggressive_mode && unit_mode==0 && cur_action==SPRITE_ATTACK )	// only in attack mode, as if the unit is still moving the target may be far away from the current position
	{
		//--- only when the unit is currently attacking a firm or a town ---//

		if( action_mode2 == ACTION_ATTACK_FIRM ||
			 action_mode2 == ACTION_ATTACK_TOWN )
		{
			if( info.game_date%5==0 )				// check once every 5 days
				return think_change_attack_target();
		}
	}

	return 0;
}
//-------- End of function Unit::think_aggressive_action --------//


//--------- Begin of function Unit::think_resume_original_action --------//
//
// If this unit is chasing a target to attack. Stop the chase
// if the target is being far away from its original location.
//
int Unit::think_resume_original_action()
{
	if( !is_visible() )		// if the unit is no longer visible, cancel the saved orignal action
	{
		original_action_mode = 0;
		return 0;
	}

	//---- if the unit is in defense mode now, don't do anything ----//

	if( in_any_defense_mode() )
		return 0;

	//----------------------------------------------------//
	//
	// If the action has been changed or the target unit has been deleted,
	// stop the chase right and move back to the original position
	// before the auto guard attack.
	//
	//----------------------------------------------------//

	if(action_mode2!=ACTION_ATTACK_UNIT || unit_array.is_deleted(action_para2))
	{
		resume_original_action();
		return 1;
	}

	//-----------------------------------------------------//
	//
	// Stop the chase if the target is being far away from
	// its original location and move back to its original
	// position before the auto guard attack.
	//
	//-----------------------------------------------------//

	#define AUTO_GUARD_CHASE_ATTACK_DISTANCE	5

	Unit* targetUnit = unit_array[action_para2];

	int curDistance = m.points_distance( targetUnit->next_x_loc(), targetUnit->next_y_loc(),
							original_target_x_loc, original_target_y_loc );

	if( curDistance > AUTO_GUARD_CHASE_ATTACK_DISTANCE )
	{
		resume_original_action();
		return 1;
	}

	return 0;
}
//---------- End of function Unit::think_resume_original_action --------//


//------- Begin of function Unit::think_change_attack_target -------//
//
// When the unit is attacking a firm or town, look out for enemy units
// to attack. Enemy units should be attacked first.
//
int Unit::think_change_attack_target()
{
	err_when( !nation_recno );		// only for nation units

	//----------------------------------------------//

	int attackRange	  = MAX(attack_range, 8);
	int attackScanRange = attackRange*2+1;

	int		 xOffset, yOffset;
	int		 xLoc, yLoc;
	Location* locPtr;
	int		 curXLoc = next_x_loc(), curYLoc = next_y_loc();
	BYTE	 	 regionId = world.get_region_id(curXLoc, curYLoc);

	for( int i=2 ; i<attackScanRange*attackScanRange ; i++ )
	{
		m.cal_move_around_a_point(i, attackScanRange, attackScanRange, xOffset, yOffset);

		xLoc = curXLoc + xOffset;
		yLoc = curYLoc + yOffset;

		xLoc = MAX(0, xLoc);
		xLoc = MIN(MAX_WORLD_X_LOC-1, xLoc);

		yLoc = MAX(0, yLoc);
		yLoc = MIN(MAX_WORLD_Y_LOC-1, yLoc);

		locPtr = world.get_loc(xLoc, yLoc);

		if( locPtr->region_id != regionId )
			continue;

		//----- if there is a unit on the location ------//

		if( locPtr->has_unit(UNIT_LAND) )
		{
			int unitRecno = locPtr->unit_recno(UNIT_LAND);

			if( unit_array.is_deleted(unitRecno) )
				continue;

			if( unit_array[unitRecno]->nation_recno != nation_recno &&
				 idle_detect_unit_checking(unitRecno) )
			{
				save_original_action();

				original_target_x_loc = xLoc;
				original_target_y_loc = yLoc;

				attack_unit(xLoc, yLoc);
				return 1;
			}
		}
	}

	return 0;
}
//---------- End of function Unit::think_change_attack_target ------//


//----------- Begin of function Unit::save_original_action -------//
void Unit::save_original_action()
{
	if( original_action_mode==0 )
	{
		original_action_mode  = action_mode2;
		original_action_para  = action_para2;
		original_action_x_loc = action_x_loc2;
		original_action_y_loc = action_y_loc2;
	}
}
//----------- End of function Unit::save_original_action -------//


//----------- Begin of function Unit::resume_original_action -------//

void Unit::resume_original_action()
{
	if( !original_action_mode )
		return;

	//--------- If it is an attack action ---------//

	if( original_action_mode == ACTION_ATTACK_UNIT ||
		 original_action_mode == ACTION_ATTACK_FIRM ||
		 original_action_mode == ACTION_ATTACK_TOWN )
	{
		resume_original_attack_action();
		return;
	}

	//--------------------------------------------//

	if( original_action_x_loc<0 || original_action_x_loc>=MAX_WORLD_X_LOC ||
		 original_action_y_loc<0 || original_action_y_loc>=MAX_WORLD_Y_LOC )
	{
		original_action_mode = 0;
		return;
	}

	short selectedArray[1];
	selectedArray[0] = sprite_recno;		// use unit_array.attack() instead of unit.attack_???() as we are unsure about what type of object the target is.

	Location* locPtr = world.get_loc(original_action_x_loc, original_action_y_loc);

	//--------- resume assign to town -----------//

	if( original_action_mode == ACTION_ASSIGN_TO_TOWN && locPtr->is_town() )
	{
		if( locPtr->town_recno() == original_action_para &&
			 town_array[original_action_para]->nation_recno == nation_recno )
		{
			unit_array.assign( original_action_x_loc, original_action_y_loc, 0,
									 COMMAND_AUTO, selectedArray, 1 );
		}
	}

	//--------- resume assign to firm ----------//

	else if( original_action_mode == ACTION_ASSIGN_TO_FIRM && locPtr->is_firm() )
	{
		if( locPtr->firm_recno() == original_action_para &&
			 firm_array[original_action_para]->nation_recno == nation_recno )
		{
			unit_array.assign( original_action_x_loc, original_action_y_loc, 0,
									 COMMAND_AUTO, selectedArray, 1 );
		}
	}

	//--------- resume build firm ---------//

	else if( original_action_mode == ACTION_BUILD_FIRM )
	{
		if( world.can_build_firm( original_action_x_loc, original_action_y_loc,
										  original_action_para, sprite_recno ) )
		{
			build_firm( original_action_x_loc, original_action_y_loc,
							original_action_para, COMMAND_AUTO );
		}
	}

	//--------- resume settle ---------//

	else if( original_action_mode == ACTION_SETTLE )
	{
		if( world.can_build_town( original_action_x_loc, original_action_y_loc, sprite_recno ) )
		{
			unit_array.settle( original_action_x_loc, original_action_y_loc,
									 0, COMMAND_AUTO, selectedArray, 1 );
		}
	}

	//--------- resume move ----------//

	else if( original_action_mode == ACTION_MOVE )
	{
		unit_array.move_to( original_action_x_loc, original_action_y_loc, 0,
								  selectedArray, 1, COMMAND_AUTO );
	}

	original_action_mode = 0;
}
//----------- End of function Unit::resume_original_action -------//


//----------- Begin of function Unit::resume_original_attack_action -------//
//
void Unit::resume_original_attack_action()
{
	if( !original_action_mode )
		return;

	if( original_action_mode != ACTION_ATTACK_UNIT &&
		 original_action_mode != ACTION_ATTACK_FIRM &&
		 original_action_mode != ACTION_ATTACK_TOWN )
	{
		original_action_mode = 0;
		return;
	}

	//--------------------------------------------//

	err_when( original_action_x_loc<0 || original_action_x_loc>=MAX_WORLD_X_LOC );
	err_when( original_action_y_loc<0 || original_action_y_loc>=MAX_WORLD_Y_LOC );

	Location* locPtr = world.get_loc(original_action_x_loc, original_action_y_loc);
	int		 targetNationRecno = -1;

	if( original_action_mode == ACTION_ATTACK_UNIT && locPtr->has_unit(UNIT_LAND) )
	{
		int unitRecno = locPtr->unit_recno(UNIT_LAND);

		if( unitRecno == original_action_para )
			targetNationRecno = unit_array[unitRecno]->nation_recno;
	}
	else if( original_action_mode == ACTION_ATTACK_FIRM && locPtr->is_firm() )
	{
		int firmRecno = locPtr->firm_recno();

		if( firmRecno == original_action_para )
			targetNationRecno = firm_array[firmRecno]->nation_recno;
	}
	else if( original_action_mode == ACTION_ATTACK_TOWN && locPtr->is_town() )
	{
		int townRecno = locPtr->town_recno();

		if( townRecno == original_action_para )
			targetNationRecno = town_array[townRecno]->nation_recno;
	}

	//----- the original target is no longer valid ----//

	if( targetNationRecno == -1 )
	{
		original_action_mode = 0;
		return;
	}

	//---- only resume attacking the target if the target nation is at war with us currently ---//

	if( !targetNationRecno || (targetNationRecno &&
		 targetNationRecno != nation_recno &&
		 nation_array[nation_recno]->get_relation_status(targetNationRecno) == NATION_HOSTILE ))
	{
		short selectedArray[1];
		selectedArray[0] = sprite_recno;		// use unit_array.attack() instead of unit.attack_???() as we are unsure about what type of object the target is.

		// #### begin Gilbert 5/8 ########//
		unit_array.attack(original_action_x_loc, original_action_y_loc, 0, selectedArray, 1, COMMAND_AI, 0 );
		// #### end Gilbert 5/8 ########//
	}

	original_action_mode = 0;
}
//----------- End of function Unit::resume_original_attack_action -------//

//------- Begin of function Unit::ask_team_help_attack --------//
//
// It returns whether any of the co-member of this unit in a troop
// is under attack.
//
// <Unit*> attackerUnit - the unit pointer of the attacker
//
void Unit::ask_team_help_attack(Unit* attackerUnit)
{
	//--- if the attacking unit is our unit (this can happen if the unit is porcupine) ---//

	if( attackerUnit->nation_recno == nation_recno )
		return;

	//-----------------------------------------//

	int leaderUnitRecno=0;

	if( leader_unit_recno )		// if the current unit is a soldier, get its leader's recno
		leaderUnitRecno = leader_unit_recno;

	else if( team_info )			// this unit is the commander
		leaderUnitRecno = sprite_recno;

	if( leaderUnitRecno )
	{
		TeamInfo* teamInfo = unit_array[leaderUnitRecno]->team_info;

		err_when( !teamInfo );

		for( int i=teamInfo->member_count-1 ; i>=0 ; i-- )
		{
			int unitRecno = teamInfo->member_unit_array[i];

			if( unit_array.is_deleted(unitRecno) )
				continue;

			Unit* unitPtr = unit_array[ unitRecno ];

			if( unitPtr->cur_action==SPRITE_IDLE && unitPtr->is_visible() )
			{
				unitPtr->attack_unit(attackerUnit->sprite_recno);
				return;
			}
		}
	}
}
//-------- End of function Unit::ask_team_help_attack --------//

