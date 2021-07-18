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

//Filename    : OUNITAT2.CPP
//Description : Object Unit attack processing function
//Owner		  : Alex

#include <ALL.h>
#include <OWORLD.h>
#include <ONATION.h>
#include <OBULLET.h>
#include <OU_MARI.h>
#include <OFIRM.h>
#include <OGAME.h>
#include <OTOWN.h>
#include <OF_CAMP.h>
#include <OEFFECT.h>
#include <OU_CART.h>
#include <ConfigAdv.h>

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

//--------- Begin of function Unit::attack_unit ---------//
// overloaded function.
// the unit calling this function is to attack target by (1) default
// ordering or by (2) defense mode
//
// Note : (targetXLoc, targetYLoc) should be the upper left corner
//			 location of the target
//
// <int> targetXLoc			- target x location
// <int> targetYLoc			- target y location
// <int> xOffset, yOffset	- offset location to (targetXLoc, targetYLoc)
// <int> resetBlockedEdge	- reset blocked_edge if true
//
void Unit::attack_unit(int targetXLoc, int targetYLoc, int xOffset, int yOffset, int resetBlockedEdge)
{
	Location* locPtr = world.get_loc(targetXLoc, targetYLoc);

	//--- AI attacking a nation which its NationRelation::should_attack is 0 ---//

	int targetNationRecno = 0;

	if( locPtr->has_unit(UNIT_LAND) )
	{
		Unit* unitPtr = unit_array[ locPtr->unit_recno(UNIT_LAND) ];

		if( unitPtr->unit_id != UNIT_EXPLOSIVE_CART )	// attacking own porcupine is allowed
			targetNationRecno = unitPtr->nation_recno;
	}
	else if( locPtr->is_firm() )
	{
		targetNationRecno = firm_array[locPtr->firm_recno()]->nation_recno;
	}
	else if( locPtr->is_town() )
	{
		targetNationRecno = town_array[locPtr->town_recno()]->nation_recno;
	}

	if( nation_recno && targetNationRecno )
	{
		if( nation_array[nation_recno]->get_relation(targetNationRecno)->should_attack==0 )
			return;
	}

	//------------------------------------------------------------//
	// return if this unit cannot do the attack action, or die
	//------------------------------------------------------------//
	if(!can_attack())
	{
		stop2(KEEP_DEFENSE_MODE);
		return;
	}
	else if(is_unit_dead())
		return;

	err_when(!can_attack());

	locPtr = world.get_loc(targetXLoc, targetYLoc);
	err_when(!locPtr);

	char targetMobileType = (next_x_loc()==targetXLoc && next_y_loc()==targetYLoc) ?
									locPtr->has_any_unit(mobile_type) : locPtr->has_any_unit();

	if(targetMobileType)
	{
		Unit *targetUnit = unit_array[locPtr->unit_recno(targetMobileType)];
		attack_unit(targetUnit->sprite_recno, xOffset, yOffset, resetBlockedEdge);
	}

	//------ set ai_original_target_?_loc --------//

	if( ai_unit )
	{
		ai_original_target_x_loc = targetXLoc;
		ai_original_target_y_loc = targetYLoc;
	}
}
//----------- End of function Unit::attack_unit -----------//


//--------- Begin of function Unit::attack_unit ---------//
// overloaded function, 
//
// <int> targetRecno			- target recno
// <int> xOffset, yOffset	- offset location to (targetXLoc, targetYLoc)
// <int> resetBlockedEdge	- reset blocked_edge if true
//
void Unit::attack_unit(short targetRecno, int xOffset, int yOffset, int resetBlockedEdge)
{
	//------------------------------------------------------------//
	// return if this unit cannot do the attack action, or die
	//------------------------------------------------------------//
	if(!can_attack())
	{
		stop2(KEEP_DEFENSE_MODE);
		return;
	}
	else if(is_unit_dead())
		return;
	err_when(!can_attack());

	//----------------------------------------------------------------------------------//
	// Note for non-air unit,
	// 1) If target's mobile type == mobile_type and thir territory id are different,
	//		call move_to() instead of attacking.
	// 2) In the case, this unit is a land unit and the target is a sea unit, skip
	//		checking for range attacking.  It is because the ship may be in the coast side
	//		there this unit can attack it by close attack.  In other cases, a unit without
	//		the ability of doing range attacking cannot attack target with different mobile
	//		type to its.
	// 3) If the region_id of the target located is same as that of thus unit located,
	//		order this unit to move to it and process attacking
	// 4) If the unit can't reach a location there it can do range attack, call move_to()
	//		rather than resume the action.  The reason not to resume the action, even though
	//		the unit reach a location there can do range attack later, is that the action
	//		will be resumed in function idle_detect_target()
	//----------------------------------------------------------------------------------//
	int curXLoc = next_x_loc();
	int curYLoc = next_y_loc();
	err_when(unit_array.is_deleted(targetRecno));
	Unit	*targetUnit = unit_array[targetRecno];
	char	targetMobileType = targetUnit->mobile_type;
	int	targetXLoc = targetUnit->next_x_loc();
	int	targetYLoc = targetUnit->next_y_loc();
	int	diffTerritoryAttack = 0, maxRange=0;
	Location *locPtr = world.get_loc(targetUnit->next_x_loc(), targetUnit->next_y_loc());
	err_when(!locPtr);

	if(targetMobileType && mobile_type!=UNIT_AIR) // air unit can move to anywhere
	{
		//------------------------------------------------------------------------//
		// return if not feasible condition
		//------------------------------------------------------------------------//
		if((mobile_type!=UNIT_LAND || targetMobileType!=UNIT_SEA) && mobile_type!=targetMobileType)
		{
			if(!can_attack_different_target_type())
			{
				//-************ improve later **************-//
				//-******** should escape from being attacked ********-//
				if(in_any_defense_mode())
					general_defend_mode_detect_target();
				return;
			}
		}
	
		//------------------------------------------------------------------------//
		// handle the case the unit and the target are in different territory
		//------------------------------------------------------------------------//
		if(world.get_loc(curXLoc, curYLoc)->region_id!=locPtr->region_id)
		{
			maxRange = max_attack_range();
			Unit *unitPtr = unit_array[locPtr->unit_recno(targetMobileType)];
			if(!possible_place_for_range_attack(targetXLoc, targetYLoc, unitPtr->sprite_info->loc_width, unitPtr->sprite_info->loc_height, maxRange))
			{
				if(action_mode2!=ACTION_AUTO_DEFENSE_ATTACK_TARGET && action_mode2!=ACTION_DEFEND_TOWN_ATTACK_TARGET &&
					action_mode2!=ACTION_MONSTER_DEFEND_ATTACK_TARGET)
					move_to(targetXLoc, targetYLoc);
				else // in defend mode, but unable to attack target
					general_defend_mode_detect_target(1);

				return;
			}
			else // can reach
				diffTerritoryAttack = 1;
		}
	}

	//------------------------------------------------------------//
	// no unit there
	//------------------------------------------------------------//
	if(!targetMobileType)
	{
		if(action_mode2==ACTION_AUTO_DEFENSE_ATTACK_TARGET || action_mode2==ACTION_DEFEND_TOWN_ATTACK_TARGET ||
			action_mode2==ACTION_MONSTER_DEFEND_ATTACK_TARGET)
		{
			stop2(KEEP_DEFENSE_MODE);
			err_when((action_mode2!=ACTION_AUTO_DEFENSE_DETECT_TARGET || action_para2!=AUTO_DEFENSE_DETECT_COUNT) &&
						(action_mode2!=ACTION_DEFEND_TOWN_DETECT_TARGET || action_para2!=UNIT_DEFEND_TOWN_DETECT_COUNT) &&
						(action_mode2!=ACTION_MONSTER_DEFEND_DETECT_TARGET || action_para2!=MONSTER_DEFEND_DETECT_COUNT));
		}
		
		err_when(action_mode==ACTION_STOP && cur_action==SPRITE_ATTACK);
		err_when(action_mode==ACTION_ATTACK_UNIT || action_para || action_x_loc!=-1 || action_y_loc!=-1);
		return;
	}

	//------------------------------------------------------------//
	// cannot attack this nation
	//------------------------------------------------------------//
	err_when(targetUnit->next_x_loc()!=targetXLoc || targetUnit->next_y_loc()!=targetYLoc);
	if(!nation_can_attack(targetUnit->nation_recno) && targetUnit->unit_id!=UNIT_EXPLOSIVE_CART)
	{
		stop2(KEEP_DEFENSE_MODE);
		err_when(action_mode==ACTION_ATTACK_UNIT && !action_para);
		err_when(action_mode==ACTION_STOP && cur_action==SPRITE_ATTACK);
		return;
	}

	//----------------------------------------------------------------//
	// action_mode2: checking for equal action or idle action
	//----------------------------------------------------------------//
	if((action_mode2==ACTION_ATTACK_UNIT || action_mode2==ACTION_AUTO_DEFENSE_ATTACK_TARGET ||
		 action_mode2==ACTION_DEFEND_TOWN_ATTACK_TARGET || action_mode2==ACTION_MONSTER_DEFEND_ATTACK_TARGET) &&
		 action_para2==targetUnit->sprite_recno && action_x_loc2==targetXLoc && action_y_loc2==targetYLoc)
	{
		//------------ old order ------------//
		err_when(action_mode==ACTION_STOP && cur_action==SPRITE_ATTACK);
		err_when(action_mode==ACTION_ATTACK_UNIT && !action_para);

		if(cur_action!=SPRITE_IDLE)
		{
			//------- the old order is processing, return -------//
			#ifdef DEBUG
			if(action_mode==ACTION_ATTACK_UNIT)
			{
				err_when(action_mode2!=ACTION_ATTACK_UNIT && action_mode2!=ACTION_AUTO_DEFENSE_ATTACK_TARGET &&
							action_mode2!=ACTION_DEFEND_TOWN_ATTACK_TARGET && action_mode2!=ACTION_MONSTER_DEFEND_ATTACK_TARGET);
				err_when(action_para!=action_para2 || action_x_loc!=action_x_loc2 || action_y_loc!=action_y_loc2);
			}
			else
				err_when(action_mode!=ACTION_STOP || action_para || action_x_loc!=-1 || action_y_loc!=-1);
			#endif
			return;
		}//else the action becomes idle
	}
	else
	{
		//-------------- store new order ----------------//
		if(action_mode2!=ACTION_AUTO_DEFENSE_ATTACK_TARGET && action_mode2!=ACTION_DEFEND_TOWN_ATTACK_TARGET &&
			action_mode2!=ACTION_MONSTER_DEFEND_ATTACK_TARGET)
			action_mode2 = ACTION_ATTACK_UNIT;

		action_para2  = targetUnit->sprite_recno;
		action_x_loc2 = targetXLoc;
		action_y_loc2 = targetYLoc;
	}

   //-------------------------------------------------------------//
	// process new order
	//-------------------------------------------------------------//
	stop();
	cur_attack = 0;

	int attackDistance = cal_distance(targetXLoc, targetYLoc, targetUnit->sprite_info->loc_width, targetUnit->sprite_info->loc_height);
	choose_best_attack_mode(attackDistance, targetMobileType);

	AttackInfo* attackInfo = attack_info_array+cur_attack;
	if(attackInfo->attack_range<attackDistance) // need to move to target
	{
		err_when(attackDistance<2);
		int searchResult = 1;

		if(xOffset || yOffset)
		{
			int xLoc = targetXLoc+xOffset, yLoc = targetYLoc+yOffset;
			if(xLoc<0) xLoc = 0;
			else if(xLoc>=MAX_WORLD_X_LOC) xLoc = MAX_WORLD_X_LOC-1;
			if(yLoc<0) yLoc = 0;
			else if(yLoc>=MAX_WORLD_Y_LOC) yLoc = MAX_WORLD_Y_LOC-1;

			search(xLoc, yLoc, 1); // offset location is given, so move there directly
		}
		else
		{
			//if(mobile_type!=targetMobileType)
			if(diffTerritoryAttack)
			{
				//--------------------------------------------------------------------------------//
				// 1) different type from target, target located in different territory from this
				//		unit. But able to attack this target by range attacking
				//--------------------------------------------------------------------------------//
				move_to_range_attack(targetXLoc, targetYLoc, targetUnit->sprite_id, SEARCH_MODE_ATTACK_UNIT_BY_RANGE, maxRange);
			}
			else
			{
				//--------------------------------------------------------------------------------//
				// 1) same type of target,
				// 2) this unit is air unit, or
				// 3) different type from target, but target located in the same territory of this
				//		unit.
				//--------------------------------------------------------------------------------//
				searchResult = search(targetXLoc, targetYLoc, 1, SEARCH_MODE_TO_ATTACK, targetUnit->sprite_recno);
			}
		}

		//---------------------------------------------------------------//
		// initialize parameters for blocked edge handling in attacking
		//---------------------------------------------------------------//
		if(searchResult)
		{
			waiting_term = 0;
			if(resetBlockedEdge)
				memset(blocked_edge, 0, sizeof(char)*4);
		}
		else
			memset(blocked_edge, 0xff, sizeof(char)*4); // for reactivating idle action
	}
	else if(cur_action==SPRITE_IDLE) // the target is within attack range, attacks it now if the unit is idle
	{
		//---------------------------------------------------------------//
		// attack now
		//---------------------------------------------------------------//
		set_cur(next_x, next_y);
		set_attack_dir(curXLoc, curYLoc, targetXLoc, targetYLoc);
		if(is_dir_correct())
		{
			if(attackInfo->attack_range==1)
			{
				set_attack();
				turn_delay = 0;
			}
		}
		else
			set_turn();
	}

	action_mode  = ACTION_ATTACK_UNIT;
	action_para  = targetUnit->sprite_recno;
	action_x_loc = targetXLoc;
	action_y_loc = targetYLoc;

	//------ set ai_original_target_?_loc --------//

	if( ai_unit )
	{
		ai_original_target_x_loc = targetXLoc;
		ai_original_target_y_loc = targetYLoc;
	}

	err_when(action_mode==ACTION_ATTACK_UNIT && !action_para);
	err_when(action_mode!=action_mode2 && action_mode!=ACTION_ATTACK_UNIT);
	err_when(action_para!=action_para2 || !action_para);
	err_when(action_x_loc!=action_x_loc2 || action_y_loc!=action_y_loc2);
}
//----------- End of function Unit::attack_unit -----------//


//--------- Begin of function Unit::process_attack_unit ---------//
// process the action to attack a unit
//
void Unit::process_attack_unit()
{
	if(!can_attack())
	{
		stop2(KEEP_DEFENSE_MODE);
		return;
	}

	err_when(!action_para || action_mode!=ACTION_ATTACK_UNIT ||	!can_attack());	// unable to attack

	// ###### begin Gilbert 17/3 #######//
	//err_when(attack_info_array[cur_attack].attack_range != attack_range);
	// ###### end Gilbert 17/3 #######//

	//--------------------------------------------------------------------------//
	// stop if the targeted unit has been killed or target belongs our nation
	//--------------------------------------------------------------------------//
	int clearOrder = 0;
	Unit* targetUnit;

	if(unit_array.is_deleted(action_para) || action_para==sprite_recno)
		clearOrder++;
	else
	{
		targetUnit = unit_array[action_para];
		if(targetUnit->nation_recno && !nation_can_attack(targetUnit->nation_recno) &&
			targetUnit->unit_id!=UNIT_EXPLOSIVE_CART) // cannot attack this nation
			clearOrder++;
	}

	if(clearOrder)
	{
		//------------------------------------------------------------//
		// change to detect target if in defense mode
		//------------------------------------------------------------//
		/*if(action_mode2==ACTION_AUTO_DEFENSE_ATTACK_TARGET || action_mode2==ACTION_DEFEND_TOWN_ATTACK_TARGET ||
			action_mode2==ACTION_MONSTER_DEFEND_ATTACK_TARGET)
		{
			stop2(KEEP_DEFENSE_MODE);
			err_when((action_misc!=ACTION_MISC_DEFENSE_CAMP_RECNO && action_misc!=ACTION_MISC_DEFEND_TOWN_RECNO &&
						 action_misc!=ACTION_MISC_MONSTER_DEFEND_FIRM_RECNO) || !action_misc_para);
			return;
		}

		stop2(); // clear order
		err_when(cur_action==SPRITE_ATTACK && (move_to_x_loc!=next_x_loc() || move_to_y_loc!=next_y_loc()));
		err_when(action_mode==ACTION_ATTACK_UNIT && !action_para);
		err_when(cur_action==SPRITE_ATTACK && action_mode==ACTION_STOP);
		return;*/

		#ifdef DEBUG
			int inDefMode = in_any_defense_mode();
		#endif

		stop2(KEEP_DEFENSE_MODE);

		#ifdef DEBUG
			if(inDefMode)
			{
				err_when((action_misc!=ACTION_MISC_DEFENSE_CAMP_RECNO && action_misc!=ACTION_MISC_DEFEND_TOWN_RECNO &&
							 action_misc!=ACTION_MISC_MONSTER_DEFEND_FIRM_RECNO) || !action_misc_para);
			}
		#endif

		err_when(cur_action==SPRITE_ATTACK && (move_to_x_loc!=next_x_loc() || move_to_y_loc!=next_y_loc()));
		err_when(action_mode==ACTION_ATTACK_UNIT && !action_para);
		err_when(cur_action==SPRITE_ATTACK && action_mode==ACTION_STOP);
		return;
	}

	err_when(ai_unit && nation_recno== targetUnit->nation_recno); // ai_unit don't attack itself, but players can

	//--------------------------------------------------------------------------//
	// stop action if target goes into town/firm, ships (go to other territory)
	//--------------------------------------------------------------------------//
	if(!targetUnit->is_visible())
	{
		stop2(KEEP_DEFENSE_MODE);	// clear order
		err_when(action_mode==ACTION_ATTACK_UNIT && !action_para);
		err_when(cur_action==SPRITE_ATTACK && action_mode==ACTION_STOP);
		return;
	}

	//------------------------------------------------------------//
	// if the caravan is entered a firm, attack the firm
	//------------------------------------------------------------//
	if(targetUnit->caravan_in_firm())
	{
		//----- for caravan entering the market -------//
		err_when(targetUnit->unit_id!=UNIT_CARAVAN);
		if(firm_array.is_deleted(targetUnit->action_para))	// the current firm recno of the firm the caravan entered is stored in action_para
			stop2(KEEP_DEFENSE_MODE);	// clear order
		else
		{
			Firm* firmPtr = firm_array[targetUnit->action_para];
			attack_firm(firmPtr->loc_x1, firmPtr->loc_y1);
		}

		err_when(action_mode==ACTION_ATTACK_UNIT && !action_para);
		return;
	}
	
	//---------------- define parameters ---------------------//
	int   targetXLoc = targetUnit->next_x_loc();
	int   targetYLoc = targetUnit->next_y_loc();
	int	spriteXLoc = next_x_loc();
	int	spriteYLoc = next_y_loc();
	AttackInfo* attackInfo = attack_info_array+cur_attack;

	//----------------------------------------------------//
#ifdef DEBUG
	short x, y, h, w;
	SpriteInfo* targetSpriteInfo = targetUnit->sprite_info;

	for(h=0, y=spriteYLoc; h<sprite_info->loc_height; h++, y++)
	{
		for(w=0, x=spriteXLoc; w<sprite_info->loc_width; w++, x++)
			err_when(world.get_unit_recno(x,y,mobile_type)!=sprite_recno);
	}

	for(h=0, y=targetYLoc; h<targetSpriteInfo->loc_height; h++, y++)
	{
		for(w=0, x=targetXLoc; w<targetSpriteInfo->loc_width; w++, x++)
			err_when(world.get_unit_recno(x,y,targetUnit->mobile_type)!=targetUnit->sprite_recno);
	}
#endif

	//------------------------------------------------------------//
	// If this unit's target has moved, change the destination accordingly.
	//------------------------------------------------------------//
	if( targetXLoc != action_x_loc || targetYLoc != action_y_loc )
	{
		target_move(targetUnit);
		if(action_mode==ACTION_STOP)
			return;
	}

	//-----------------------------------------------------//
	// If the unit is currently attacking somebody.
	//-----------------------------------------------------//
	//if( cur_action==SPRITE_ATTACK && next_x==cur_x && next_y==cur_y)

	if(abs(cur_x-next_x)<=sprite_info->speed && abs(cur_y-next_y)<=sprite_info->speed)
	{
		if(cur_action==SPRITE_ATTACK)
		{
			err_when(!is_dir_correct());
			//###### begin alex 18/3 #######//
			err_when(attack_info_array[cur_attack].attack_range != attack_range);
			//###### end alex 18/3 #######//
			attack_target(targetUnit);
		}
		else
		{
			//-----------------------------------------------------//
			// If the unit is on its way to attack somebody and it
			// has got close next to the target, attack now
			//-----------------------------------------------------//
			#ifdef DEBUG
				if(on_way_to_attack(targetUnit))
					return; // skip the following error checking
			#else
				on_way_to_attack(targetUnit);
			#endif
		}
	}

	err_when(action_mode==ACTION_ATTACK_UNIT && !action_para);
	err_when(action_mode==ACTION_STOP && cur_action==SPRITE_ATTACK);
	err_when(cur_action==SPRITE_ATTACK && attack_range==1 && cal_distance(targetXLoc, targetYLoc,
				targetUnit->sprite_info->loc_width, targetUnit->sprite_info->loc_height)>1);
}
//---------- End of function Unit::process_attack_unit ----------//


//--------- Begin of function Unit::target_move ---------//
// called by process_attack_unit when the target is moved
//
// <Unit*> targetUnit	-	pointer to the target
//
void Unit::target_move(Unit* targetUnit)
{
	//------------------------------------------------------------------------------------//
	// chekcing whether ship can follow to attack target. It is always true if the unit is
	// not ship. 1 for allowing, 0 otherwise
	//------------------------------------------------------------------------------------//
	int allowMove = 1;
	if(sprite_info->sprite_sub_type=='M')
	{
		UnitInfo* unitInfo = unit_res[unit_id];
		if(unitInfo->carry_goods_capacity)
		{
			UnitMarine *shipPtr = (UnitMarine*) this;
			if(shipPtr->auto_mode && shipPtr->stop_defined_num>1)
				allowMove = 0;
		}
	}

	//---------------------------------------------------------//
	int targetXLoc	= targetUnit->next_x_loc();
	int targetYLoc = targetUnit->next_y_loc();
	SpriteInfo	*targetSpriteInfo = targetUnit->sprite_info;

	int attackDistance = cal_distance(targetXLoc, targetYLoc, targetSpriteInfo->loc_width, targetSpriteInfo->loc_height);
	action_x_loc2 = action_x_loc = targetXLoc; // update target location
	action_y_loc2 = action_y_loc = targetYLoc;

	//---------------------------------------------------------------------//
	// target is out of attacking range, move closer to it
	//---------------------------------------------------------------------//
	int curXLoc	= next_x_loc();
	int curYLoc	= next_y_loc();
	if(attackDistance>attack_range)
	{
		//---------------- stop all actions if not allow to move -----------------//
		if(!allowMove)
		{
			stop2();
			return;
		}

		err_when(action_mode!=ACTION_ATTACK_UNIT || action_para!=targetUnit->sprite_recno);
		//---------------------------------------------------------------------//
		// follow the target using the result_path_dist
		//---------------------------------------------------------------------//
		if(!update_attack_path_dist())
		{
			if(cur_action==SPRITE_MOVE || cur_action==SPRITE_WAIT || cur_action==SPRITE_READY_TO_MOVE)
				return;
		}

		if(move_try_to_range_attack(targetUnit))
		{
			err_when(action_x_loc!=targetXLoc || action_y_loc!=targetYLoc);

			//-----------------------------------------------------------------------//
			// reset attack parameters
			//-----------------------------------------------------------------------//
			range_attack_x_loc = range_attack_y_loc = -1;
			err_when(action_mode!=ACTION_ATTACK_UNIT);
			err_when(action_para!=targetUnit->sprite_recno);

			choose_best_attack_mode(attackDistance, targetUnit->mobile_type); // choose better attack mode to attack the target
			err_when(attackDistance>attack_range && cur_action==SPRITE_ATTACK);
		}
	}
	else // attackDistance <= attack_range
	{
		//-----------------------------------------------------------------------------//
		// although the target has moved, the unit can still attack it. no need to move
		//-----------------------------------------------------------------------------//
		if(abs(cur_x-next_x)>=sprite_info->speed || abs(cur_y-next_y)>=sprite_info->speed)
			return;	// return as moving

		if(attackDistance==1 && attack_range>1) // may change attack mode
			choose_best_attack_mode(attackDistance, targetUnit->mobile_type);

		if(attack_range>1) // range attack
		{
			//------------------ do range attack ----------------------//
			AttackInfo	*attackInfo = attack_info_array + cur_attack;
			if(bullet_array.add_bullet_possible(curXLoc, curYLoc, mobile_type, targetXLoc, targetYLoc, targetUnit->mobile_type, targetSpriteInfo->loc_width,
				targetSpriteInfo->loc_height, range_attack_x_loc, range_attack_y_loc, attackInfo->bullet_speed, attackInfo->bullet_sprite_id))	// range attack possible
			{
				set_cur(next_x, next_y);
				err_when(range_attack_x_loc==-1 || range_attack_y_loc==-1);

				set_attack_dir(curXLoc, curYLoc, range_attack_x_loc, range_attack_y_loc);
				if( config_adv.unit_target_move_range_cycle )
				{
					cycle_eqv_attack();
					attackInfo = attack_info_array + cur_attack;	// cur_attack may change
					cur_frame  = 1;
				}

				if(is_dir_correct())
					set_attack();
				else
					set_turn();
			}
			else // unable to do range attack, move to target
			{
				err_when(range_attack_x_loc!=-1 || range_attack_y_loc!=-1);
				if(!allowMove)
				{
					stop2();
					return;
				}

				if(move_try_to_range_attack(targetUnit))
				{
					err_when(action_x_loc!=targetXLoc || action_y_loc!=targetYLoc);
					//range_attack_x_loc = range_attack_y_loc = -1;
					err_when(action_mode!=ACTION_ATTACK_UNIT);
					err_when(action_para!=targetUnit->sprite_recno);
					choose_best_attack_mode(attackDistance, targetUnit->mobile_type);
				}
			}
		}
		else if(attackDistance==1) // close attack
		{
			err_when(attack_range>1);
			set_cur(next_x, next_y);
			set_attack_dir(curXLoc, curYLoc, targetXLoc, targetYLoc);
			cur_frame  = 1;

			if(is_dir_correct())
				set_attack();
			else
				set_turn();
		}
	}
}
//---------- End of function Unit::target_move ----------//


//----------------- Begin of function Unit::attack_target -----------------//
// called by process_attack_unit when the unit is now attacking the target
//
// <Unit*>	targetUnit	-	pointer to target unit
//
void Unit::attack_target(Unit* targetUnit)
{
	err_when(cur_x!=next_x || cur_y!=next_y);
	err_when(action_mode==ACTION_ATTACK_UNIT && !action_para);
	err_when(action_mode==ACTION_STOP && cur_action==SPRITE_ATTACK);
	err_when(!is_dir_correct());

	if(remain_attack_delay)
		return;
	
	int unitXLoc = next_x_loc();
	int unitYLoc = next_y_loc();

	if(attack_range>1) // use range attack
	{
		//---------------- use range attack -----------------//
		AttackInfo	*attackInfo = attack_info_array+cur_attack;

		if(cur_frame!=attackInfo->bullet_out_frame)
			return;	// wait for bullet_out_frame

		if(!bullet_array.bullet_path_possible(unitXLoc, unitYLoc, mobile_type, range_attack_x_loc, range_attack_y_loc, targetUnit->mobile_type, attackInfo->bullet_speed, attackInfo->bullet_sprite_id))
		{
			SpriteInfo	*targetSpriteInfo = targetUnit->sprite_info;
			if((targetSpriteInfo->loc_width>1 || targetSpriteInfo->loc_height>1) &&
				 !bullet_array.add_bullet_possible(unitXLoc, unitYLoc, mobile_type, action_x_loc, action_y_loc, targetUnit->mobile_type, targetSpriteInfo->loc_width, targetSpriteInfo->loc_height,
				 range_attack_x_loc, range_attack_y_loc, attackInfo->bullet_speed, attackInfo->bullet_sprite_id)) // seek for another possible point to attack if target size > 1x1
			{
				//------ no suitable location to attack target by bullet, move to target --------//
				if(!result_node_array || !result_node_count || !result_path_dist)
					if(!move_try_to_range_attack(targetUnit))
						return; // can't reach a location to attack target

				err_when(action_x_loc!=targetUnit->next_x_loc() || action_y_loc!=targetUnit->next_y_loc());
				err_when(action_mode!=ACTION_ATTACK_UNIT);
				err_when(action_para!=targetUnit->sprite_recno);
			}
		}

		bullet_array.add_bullet(this, targetUnit);
		// ####### begin Gilbert 14/7 ########//
		add_close_attack_effect();
		// ####### end Gilbert 14/7 ########//

		// ------- reduce power --------//
		cur_power -= attackInfo->consume_power;
		err_when(cur_power < 0);
		//### begin alex 28/10 ###//
		if(cur_power<0) // ***** BUGHERE
			cur_power = 0;
		//#### end alex 28/10 ####//
		set_remain_attack_delay();
		return; // bullet emits
	}
	else // close attack
	{
		//--------------------- close attack ------------------------//
		AttackInfo	*attackInfo = attack_info_array+cur_attack;
		err_when(action_x_loc!=action_x_loc2 || action_y_loc!=action_y_loc2);
		err_when((unitXLoc!=action_x_loc || unitYLoc!=action_y_loc) && final_dir!=(get_dir(unitXLoc, unitYLoc, action_x_loc, action_y_loc)));

		if(cur_frame==cur_sprite_attack()->frame_count)
		{
			if(targetUnit->unit_id == UNIT_EXPLOSIVE_CART && targetUnit->is_nation(nation_recno))
				((UnitExpCart *)targetUnit)->trigger_explode();
			else
				hit_target(this, targetUnit, actual_damage(), nation_recno);

			// ####### begin Gilbert 14/7 ########//
			add_close_attack_effect();
			// ####### end Gilbert 14/7 ########//

			//------- reduce power --------//
			cur_power -= attackInfo->consume_power;
			err_when(cur_power < 0);
			//### begin alex 28/10 ###//
			if(cur_power<0) // ***** BUGHERE
				cur_power = 0;
			//#### end alex 28/10 ####//
			set_remain_attack_delay();
		}
	}
}
//------------------ End of function Unit::attack_target ------------------//


//--------- Begin of function Unit::on_way_to_attack ---------//
// called by process_attack_unit when the unit is still on its
// way to attack the target.
//
// return 1 if new target is detected
// return 0 otherwise
//
// <Unit*>	targetUnit	-	pointer to target Unit
//
int Unit::on_way_to_attack(Unit* targetUnit)
{
	err_when(abs(cur_x-next_x)>sprite_info->speed || abs(cur_y-next_y)>sprite_info->speed);
	err_when(cur_action==SPRITE_ATTACK);

	if(mobile_type==UNIT_LAND)
	{
		if(attack_range==1)
		{
			//------------------------------------------------------------//
			// for close attack, the unit unable to attack the target if
			// it is not in the target surrounding
			//------------------------------------------------------------//
			if(result_path_dist > attack_range)
				return detect_surround_target();
		}
		else if(result_path_dist && cur_action!=SPRITE_TURN)
		{
			if(detect_surround_target())
				return 1; // detect surrounding target while walking
		}
	}

	int targetXLoc = targetUnit->next_x_loc();
	int targetYLoc = targetUnit->next_y_loc();
	SpriteInfo* targetSpriteInfo = targetUnit->sprite_info;

	int attackDistance = cal_distance(targetXLoc , targetYLoc, targetSpriteInfo->loc_width, targetSpriteInfo->loc_height);

	if(attackDistance<=attack_range) // able to attack target
	{
		if((attackDistance==1) && attack_range>1) // often false condition is checked first
			choose_best_attack_mode(1, targetUnit->mobile_type); // may change to close attack

		if(attack_range>1) // use range attack
		{
			set_cur(next_x, next_y);

			AttackInfo *attackInfo = attack_info_array+cur_attack;
			int curXLoc = next_x_loc();
			int curYLoc = next_y_loc();
			if(!bullet_array.add_bullet_possible(curXLoc, curYLoc, mobile_type, targetXLoc, targetYLoc, targetUnit->mobile_type, targetSpriteInfo->loc_width, targetSpriteInfo->loc_height,
				range_attack_x_loc, range_attack_y_loc, attackInfo->bullet_speed, attackInfo->bullet_sprite_id))
			{
				//------- no suitable location, move to target ---------//
				if(!result_node_array || !result_node_count || !result_path_dist) 
					if(!move_try_to_range_attack(targetUnit))
							return 0; // can't reach a location to attack target

				err_when(action_mode!=ACTION_ATTACK_UNIT || action_para!=targetUnit->sprite_recno);
				return 0;
			}

			//---------- able to do range attack ----------//
			err_when(range_attack_x_loc==-1 || range_attack_y_loc==-1);
			set_attack_dir(next_x_loc(), next_y_loc(), range_attack_x_loc, range_attack_y_loc);
			cur_frame  = 1;

			if(is_dir_correct())
				set_attack();
			else
				set_turn();
		}
		else // close attack
		{
			//---------- attack now ---------//
			set_cur(next_x, next_y);
			terminate_move();
			err_when(cur_frame!=1);
			set_attack_dir(next_x_loc(), next_y_loc(), targetXLoc, targetYLoc);

			if(is_dir_correct())
				set_attack();
			else
				set_turn();
		}

		err_when(cur_x!=next_x || cur_y!=next_y);
	}

	return 0;
}
//---------- End of function Unit::on_way_to_attack ----------//


//--------- Begin of function Unit::detect_surround_target ---------//
// This function is used to detect enemy in the surrounding of the unit
//
// return 1 if target is detected
// return 0 otherwise
//
int Unit::detect_surround_target()
{
	#define DIMENSION		3
	#define CHECK_SIZE	DIMENSION*DIMENSION

	int curXLoc = next_x_loc();
	int curYLoc = next_y_loc();
	int checkXLoc, checkYLoc, xShift, yShift;
	Unit *targetPtr;
	Location *locPtr;
	short targetRecno;
	
	for(int i=2; i<=CHECK_SIZE; ++i)
	{
		misc.cal_move_around_a_point(i, DIMENSION, DIMENSION, xShift, yShift);
      checkXLoc = curXLoc+xShift;
      checkYLoc = curYLoc+yShift;
      if(checkXLoc<0 || checkXLoc>=MAX_WORLD_X_LOC || checkYLoc<0 || checkYLoc>=MAX_WORLD_Y_LOC)
         continue;

      locPtr = world.get_loc(checkXLoc, checkYLoc);
		
		if(locPtr->has_unit(UNIT_LAND))
		{
			targetRecno = locPtr->unit_recno(UNIT_LAND);
			if(unit_array.is_deleted(targetRecno))
				continue;

			targetPtr = unit_array[targetRecno];
			if(targetPtr->nation_recno==nation_recno)
				continue;

			if(idle_detect_unit_checking(targetRecno))
			{
				attack_unit(targetRecno);
				return 1;
			}
		}
	}

	return 0;
}
//---------- End of function Unit::detect_surround_target ----------//


//--------- Begin of function Unit::attack_firm ---------//
// the unit calling this function is to attack target by (1) default
// ordering or by (2) defense mode
//
// Note : (firmXLoc, firmYLoc) should be the upper left corner location
//			 of the firm.
//
// <int>	firmXLoc				-	x location of the firm
// <int>	firmYLoc				-	y location of the firm
// <int>	xOffset				-	x offset to the firm
// <int>	yOffset				-	y offset to the firm
// <int> resetBlockedEdge	-	flag to variable blocked_edge
//
void Unit::attack_firm(int firmXLoc, int firmYLoc, int xOffset, int yOffset, int resetBlockedEdge)
{
	//------------------------------------------------------------//
	// return if this unit cannot do the attack action
	//------------------------------------------------------------//
	if(!can_attack())
	{
		stop2(KEEP_DEFENSE_MODE);
		return;
	}
	else if(is_unit_dead())
		return;
	err_when(!can_attack());

	Location *locPtr = world.get_loc(firmXLoc, firmYLoc);
	err_when(!locPtr);

	//------------------------------------------------------------//
	// no firm there
	//------------------------------------------------------------//
	if(!locPtr->is_firm())
	{
		if(action_mode2==ACTION_AUTO_DEFENSE_ATTACK_TARGET || action_mode2==ACTION_DEFEND_TOWN_ATTACK_TARGET ||
			action_mode2==ACTION_MONSTER_DEFEND_ATTACK_TARGET)
		{
			stop2(KEEP_DEFENSE_MODE);
			err_when((action_mode2!=ACTION_AUTO_DEFENSE_DETECT_TARGET || action_para2!=AUTO_DEFENSE_DETECT_COUNT) &&
						(action_mode2!=ACTION_DEFEND_TOWN_DETECT_TARGET || action_para2!=UNIT_DEFEND_TOWN_DETECT_COUNT) &&
						(action_mode2!=ACTION_MONSTER_DEFEND_DETECT_TARGET || action_para2!=MONSTER_DEFEND_DETECT_COUNT));
		}
		
		err_when(action_mode==ACTION_STOP && cur_action==SPRITE_ATTACK);
		err_when(action_mode==ACTION_ATTACK_FIRM || action_para || action_x_loc!=-1 || action_y_loc!=-1);
		return;
	}

	//------------------------------------------------------------//
	// cannot attack this nation
	//------------------------------------------------------------//
	Firm *firmPtr = firm_array[locPtr->firm_recno()];
	err_when(firmPtr->loc_x1!=firmXLoc || firmPtr->loc_y1!=firmYLoc);
	if(!nation_can_attack(firmPtr->nation_recno))
	{
		stop2(KEEP_DEFENSE_MODE);
		err_when(action_mode==ACTION_STOP && cur_action==SPRITE_ATTACK);
		err_when(action_mode==ACTION_ATTACK_FIRM && !action_para);
		return;
	}

	//------------------------------------------------------------------------------------//
	// move there if cannot reach the effective attacking region
	//------------------------------------------------------------------------------------//
	int diffTerritoryAttack = 0, maxRange=0;
	if(mobile_type!=UNIT_AIR && world.get_loc(next_x_loc(), next_y_loc())->region_id!=locPtr->region_id)
	{
		maxRange = max_attack_range();
		//Firm		*firmPtr = firm_array[locPtr->firm_recno()];
		FirmInfo *firmInfo = firm_res[firmPtr->firm_id];
		if(!possible_place_for_range_attack(firmXLoc, firmYLoc, firmInfo->loc_width, firmInfo->loc_height, maxRange))
		{
			if(action_mode2!=ACTION_AUTO_DEFENSE_ATTACK_TARGET && action_mode2!=ACTION_DEFEND_TOWN_ATTACK_TARGET &&
				action_mode2!=ACTION_MONSTER_DEFEND_ATTACK_TARGET)
				move_to(firmXLoc, firmYLoc);

			return;
		}
		else // can reach
			diffTerritoryAttack = 1;
	}

	//----------------------------------------------------------------//
	// action_mode2: checking for equal action or idle action
	//----------------------------------------------------------------//
	if((action_mode2==ACTION_ATTACK_FIRM || action_mode2==ACTION_AUTO_DEFENSE_ATTACK_TARGET ||
		 action_mode2==ACTION_DEFEND_TOWN_ATTACK_TARGET || action_mode2==ACTION_MONSTER_DEFEND_ATTACK_TARGET) &&
		 action_para2==firmPtr->firm_recno && action_x_loc2==firmXLoc && action_y_loc2==firmYLoc)
	{
		//-------------- old order -------------//
		err_when(action_mode==ACTION_STOP && cur_action==SPRITE_ATTACK);
		err_when(action_mode==ACTION_ATTACK_FIRM && !action_para);

		if(cur_action!=SPRITE_IDLE)
		{
			//------- old order is processing --------//
			#ifdef DEBUG
			if(action_mode==ACTION_ATTACK_FIRM)
			{
				err_when(action_mode2!=ACTION_ATTACK_FIRM && action_mode2!=ACTION_AUTO_DEFENSE_ATTACK_TARGET &&
							action_mode2!=ACTION_DEFEND_TOWN_ATTACK_TARGET && action_mode2!=ACTION_MONSTER_DEFEND_ATTACK_TARGET);
				err_when(action_para!=action_para2 || action_x_loc!=action_x_loc2 || action_y_loc!=action_y_loc2);
			}
			else
				err_when(action_mode!=ACTION_STOP || action_para || action_x_loc!=-1 || action_y_loc!=-1);
			#endif
			return;
		}
	}
	else
	{
		//-------------- new order -------------//
		if(action_mode2!=ACTION_AUTO_DEFENSE_ATTACK_TARGET && action_mode2!=ACTION_DEFEND_TOWN_ATTACK_TARGET &&
			action_mode2!=ACTION_MONSTER_DEFEND_ATTACK_TARGET)
			action_mode2  = ACTION_ATTACK_FIRM;

		action_para2  = firmPtr->firm_recno;
		action_x_loc2 = firmXLoc;
		action_y_loc2 = firmYLoc;
	}

   //-------------------------------------------------------------//
	// process new order
	//-------------------------------------------------------------//
	stop();
	cur_attack = 0;

	int defenseMode = in_auto_defense_mode();
	if(defenseMode)
		set_search_tries(AUTO_DEFENSE_SEARCH_TRIES);

	FirmInfo *firmInfo = firm_res[firmPtr->firm_id];
	int attackDistance = cal_distance(firmXLoc, firmYLoc, firmInfo->loc_width, firmInfo->loc_height);
	choose_best_attack_mode(attackDistance);

	AttackInfo* attackInfo = attack_info_array+cur_attack;
	if(attackInfo->attack_range<attackDistance) // need to move to target
	{
		err_when(attackDistance<2);
		int searchResult = 1;

		if(xOffset || yOffset)
		{
			int xLoc = firmXLoc+xOffset, yLoc = firmYLoc+yOffset;
			if(xLoc<0) xLoc = 0;
			else if(xLoc>=MAX_WORLD_X_LOC) xLoc = MAX_WORLD_X_LOC-1;
			if(yLoc<0) yLoc = 0;
			else if(yLoc>=MAX_WORLD_Y_LOC) yLoc = MAX_WORLD_Y_LOC-1;

			search(xLoc, yLoc, 1); // offset location is given, so move there directly
		}
		else // without offset given, so call set_move_to_surround()
		{
			if(diffTerritoryAttack)
			{
				//--------------------------------------------------------------------------------//
				// 1) different type from target, target located in different territory from this
				//		unit. But able to attack this target by range attacking
				//--------------------------------------------------------------------------------//
				move_to_range_attack(firmXLoc, firmYLoc, firmPtr->firm_id, SEARCH_MODE_ATTACK_FIRM_BY_RANGE, maxRange);
			}
			else
			{
				//--------------------------------------------------------------------------------//
				// 1) same type of target,
				// 2) this unit is air unit, or
				// 3) different type from target, but target located in the same territory of this
				//		unit.
				//--------------------------------------------------------------------------------//
				searchResult = set_move_to_surround(firmXLoc, firmYLoc, firmInfo->loc_width, firmInfo->loc_height, BUILDING_TYPE_FIRM_MOVE_TO, 0, 0);
			}
		}

		//---------------------------------------------------------------//
		// initialize parameters for blocked edge handling in attacking
		//---------------------------------------------------------------//
		if(searchResult)
		{
			waiting_term = 0;
			if(resetBlockedEdge)
				memset(blocked_edge, 0, sizeof(char)*4);
		}
		else
			memset(blocked_edge, 0xff, sizeof(char)*4); // for reactivating idle action
	}
	else if(cur_action==SPRITE_IDLE)
	{
		//---------------------------------------------------------------//
		// attack now
		//---------------------------------------------------------------//
		set_cur(next_x, next_y);

		if(firmPtr->firm_id!=FIRM_RESEARCH)
			set_attack_dir(next_x_loc(), next_y_loc(), firmPtr->center_x, firmPtr->center_y);
		else // FIRM_RESEARCH with size 2x3
		{
			int curXLoc = next_x_loc();
			int curYLoc = next_y_loc();

			int hitXLoc = (curXLoc > firmPtr->loc_x1) ? firmPtr->loc_x2 : firmPtr->loc_x1;

			int hitYLoc;
			if(curYLoc < firmPtr->center_y)
				hitYLoc = firmPtr->loc_y1;
			else if(curYLoc == firmPtr->center_y)
				hitYLoc = firmPtr->center_y;
			else
				hitYLoc = firmPtr->loc_y2;

			set_attack_dir(curXLoc, curYLoc, hitXLoc, hitYLoc);
		}
		
		if(is_dir_correct())
		{
			if(attackInfo->attack_range==1)
				set_attack();
			//else range_attack is processed in calling process_attack_firm()
		}
		else
			set_turn();
	}

	action_mode  = ACTION_ATTACK_FIRM;
	action_para  = firmPtr->firm_recno;
	action_x_loc = firmXLoc;
	action_y_loc = firmYLoc;

	if(defenseMode)
		reset_search_tries();

	err_when(action_mode==ACTION_ATTACK_FIRM && action_para==0);
	err_when(action_mode==ACTION_STOP && cur_action==SPRITE_ATTACK);
	err_when(action_mode!=action_mode2 && action_mode!=ACTION_ATTACK_FIRM);
	err_when(action_para!=action_para2 || !action_para);
	err_when(action_x_loc!=action_x_loc2 || action_y_loc!=action_y_loc2);
}
//----------- End of function Unit::attack_firm -----------//


//--------- Begin of function Unit::process_attack_firm ---------//
//
// Called by pre_process(), Unit's own function, not a derived
// function of Sprite.
//
// process attacking firm
//
void Unit::process_attack_firm()
{
	if(!can_attack())
	{
		stop2(KEEP_DEFENSE_MODE);
		return;
	}

	//------- if the targeted firm has been destroyed --------//
	err_when(!action_para || action_mode!=ACTION_ATTACK_FIRM || !can_attack());	// unable to attack
	// ###### begin Gilbert 17/3 #######//
	//err_when(attack_info_array[cur_attack].attack_range != attack_range);
	// ###### end Gilbert 17/3 #######//

	Firm	*targetFirm;
	int	clearOrder = 0;
	//------------------------------------------------------------//
	// check attack conditions
	//------------------------------------------------------------//
	if(firm_array.is_deleted(action_para))
		clearOrder++;
	else
	{
		targetFirm = firm_array[action_para];
		err_when(!targetFirm || action_para!=targetFirm->firm_recno);
		err_when(action_x_loc != targetFirm->loc_x1 || action_y_loc != targetFirm->loc_y1);

		if(!nation_can_attack(targetFirm->nation_recno)) // cannot attack this nation
			clearOrder++;
	}

	if(clearOrder)
	{
		//------------------------------------------------------------//
		// change to detect target if in defend mode
		//------------------------------------------------------------//
		/*if(action_mode2==ACTION_AUTO_DEFENSE_ATTACK_TARGET || action_mode2==ACTION_DEFEND_TOWN_ATTACK_TARGET ||
			action_mode2==ACTION_MONSTER_DEFEND_ATTACK_TARGET)
		{
			err_when((action_misc!=ACTION_MISC_DEFENSE_CAMP_RECNO && action_misc!=ACTION_MISC_DEFEND_TOWN_RECNO &&
						 action_misc!=ACTION_MISC_MONSTER_DEFEND_FIRM_RECNO) || !action_misc_para);
			stop2(KEEP_DEFENSE_MODE);
			return;
		}

		err_when(action_mode2==ACTION_AUTO_DEFENSE_DETECT_TARGET || action_mode2==ACTION_AUTO_DEFENSE_BACK_CAMP ||
					action_mode2==ACTION_DEFEND_TOWN_DETECT_TARGET || action_mode2==ACTION_DEFEND_TOWN_BACK_TOWN ||
					action_mode2==ACTION_MONSTER_DEFEND_DETECT_TARGET || action_mode2==ACTION_MONSTER_DEFEND_BACK_FIRM);

		stop2(); // clear order
		err_when(cur_action==SPRITE_ATTACK && (move_to_x_loc!=next_x_loc() || move_to_y_loc!=next_y_loc()));
		err_when(action_mode==ACTION_ATTACK_FIRM && !action_para);
		err_when(cur_action==SPRITE_ATTACK && action_mode==ACTION_STOP);
		return;*/

		#ifdef DEBUG
			int inDefMode = in_any_defense_mode();
		#endif

		stop2(KEEP_DEFENSE_MODE);

		#ifdef DEBUG
			if(inDefMode)
			{
				err_when((action_misc!=ACTION_MISC_DEFENSE_CAMP_RECNO && action_misc!=ACTION_MISC_DEFEND_TOWN_RECNO &&
							 action_misc!=ACTION_MISC_MONSTER_DEFEND_FIRM_RECNO) || !action_misc_para);
			}
		#endif

		err_when(cur_action==SPRITE_ATTACK && (move_to_x_loc!=next_x_loc() || move_to_y_loc!=next_y_loc()));
		err_when(action_mode==ACTION_ATTACK_FIRM && !action_para);
		err_when(cur_action==SPRITE_ATTACK && action_mode==ACTION_STOP);
		return;
	}

	err_when(ai_unit && nation_recno==targetFirm->nation_recno); // ai unit don't attack itself, but players can

	//-----------------------------------------------------//
	// If the unit is currently attacking somebody.
	//-----------------------------------------------------//
	if(cur_action==SPRITE_ATTACK)
   {
		err_when(cur_x!=next_x || cur_y!=next_y || !is_dir_correct());
		//###### begin alex 18/3 #######//
		err_when(attack_info_array[cur_attack].attack_range != attack_range);
		//###### end alex 18/3 #######//
		if(remain_attack_delay)
			return;

		AttackInfo* attackInfo = attack_info_array+cur_attack;
		if( attackInfo->attack_range > 1 )	// range attack
		{
			//--------- wait for bullet emit ----------//
			if(cur_frame!=attackInfo->bullet_out_frame)
				return;

			//------- seek location to attack by bullet ----------//
			int curXLoc = next_x_loc();
			int curYLoc = next_y_loc();
			if(!bullet_array.bullet_path_possible(curXLoc, curYLoc, mobile_type, range_attack_x_loc, range_attack_y_loc, UNIT_LAND,
															  attackInfo->bullet_speed, attackInfo->bullet_sprite_id))
			{
				FirmInfo *firmInfo = firm_res[targetFirm->firm_id];
				if(!bullet_array.add_bullet_possible(curXLoc, curYLoc, mobile_type, action_x_loc, action_y_loc, UNIT_LAND, firmInfo->loc_width,
					 firmInfo->loc_height, range_attack_x_loc, range_attack_y_loc, attackInfo->bullet_speed, attackInfo->bullet_sprite_id))
				{
					//------- no suitable location, so move to target again ---------//
					set_move_to_surround(action_x_loc, action_y_loc, firmInfo->loc_width, firmInfo->loc_height, BUILDING_TYPE_FIRM_MOVE_TO);
					err_when(action_mode!=ACTION_ATTACK_FIRM || action_para!=targetFirm->firm_recno);
					return;
				}
			}

			//--------- add bullet, bullet emits ----------//
			bullet_array.add_bullet(this, action_x_loc, action_y_loc);
			// ####### begin Gilbert 14/7 ########//
			add_close_attack_effect();
			// ####### end Gilbert 14/7 ########//

			// ------- reduce power --------//
			cur_power -= attackInfo->consume_power;
			err_when(cur_power < 0);
			//### begin alex 28/10 ###//
			if(cur_power<0) // ***** BUGHERE
				cur_power = 0;
			//#### end alex 28/10 ####//
			set_remain_attack_delay();
			return;
		}
		else // close attack
		{
			if(cur_frame!=cur_sprite_attack()->frame_count)
				return; // is attacking

			hit_firm(this, action_x_loc, action_y_loc, actual_damage(), nation_recno);
			// ####### begin Gilbert 14/7 ########//
			add_close_attack_effect();
			// ####### end Gilbert 14/7 ########//

			// ------- reduce power --------//
			cur_power -= attackInfo->consume_power;
			err_when(cur_power < 0);
			//### begin alex 28/10 ###//
			if(cur_power<0) // ***** BUGHERE
				cur_power = 0;
			//#### end alex 28/10 ####//
			set_remain_attack_delay();
		}
   }
	//--------------------------------------------------------------------------------------------------//
   // If the unit is on its way to attack somebody, if it has gotten close next to the target, attack now
   //--------------------------------------------------------------------------------------------------//
	else if(abs(cur_x-next_x)<=sprite_info->speed && abs(cur_y-next_y)<=sprite_info->speed ) // it has moved to the specified location. check cur_x & go_x to make sure the sprite has completely move to the location, not just crossing it.
	{
		err_when(cur_action==SPRITE_ATTACK);

		if(mobile_type==UNIT_LAND)
		{
			if(detect_surround_target())
				return;
		}
		
		if(attack_range==1)
		{
			//------------------------------------------------------------//
			// for close attack, the unit unable to attack the firm if
			// it is not in the firm surrounding
			//------------------------------------------------------------//
			if(result_path_dist > attack_range)
				return;
		}

		FirmInfo	*firmInfo = firm_res[targetFirm->firm_id];
		int targetXLoc	= targetFirm->loc_x1;
		int targetYLoc	= targetFirm->loc_y1;

		int attackDistance = cal_distance(targetXLoc , targetYLoc, firmInfo->loc_width, firmInfo->loc_height);
		int curXLoc = next_x_loc();
		int curYLoc = next_y_loc();

		if(attackDistance<=attack_range) // able to attack target
		{
			if((attackDistance==1) && attack_range>1) // often false condition is checked first
				choose_best_attack_mode(1); // may change to use close attack

			if(attack_range>1) // use range attack
			{
				set_cur(next_x, next_y);

				AttackInfo *attackInfo = attack_info_array+cur_attack;
				if(!bullet_array.add_bullet_possible(curXLoc, curYLoc, mobile_type, targetXLoc, targetYLoc, UNIT_LAND, firmInfo->loc_width, firmInfo->loc_height,
					range_attack_x_loc, range_attack_y_loc, attackInfo->bullet_speed, attackInfo->bullet_sprite_id))
				{
					//------- no suitable location, move to target ---------//
					if(!result_node_array || !result_node_count) // no step for continue moving
						set_move_to_surround(action_x_loc, action_y_loc, firmInfo->loc_width, firmInfo->loc_height, BUILDING_TYPE_FIRM_MOVE_TO);

					err_when(action_mode!=ACTION_ATTACK_FIRM || action_para!=targetFirm->firm_recno);
					return;	// unable to attack, continue to move
				}

				//---------- able to do range attack ----------//
				set_attack_dir(curXLoc, curYLoc, range_attack_x_loc, range_attack_y_loc);
				cur_frame  = 1;

				if(is_dir_correct())
					set_attack();
				else
					set_turn();
			}
			else // close attack
			{
				//---------- attack now ---------//
				set_cur(next_x, next_y);
				terminate_move();
				err_when(cur_frame!=1);

				if(targetFirm->firm_id!=FIRM_RESEARCH)
					set_attack_dir(curXLoc, curYLoc, targetFirm->center_x, targetFirm->center_y);
				else // FIRM_RESEARCH with size 2x3
				{
					int hitXLoc = (curXLoc > targetFirm->loc_x1) ? targetFirm->loc_x2 : targetFirm->loc_x1;

					int hitYLoc;
					if(curYLoc < targetFirm->center_y)
						hitYLoc = targetFirm->loc_y1;
					else if(curYLoc == targetFirm->center_y)
						hitYLoc = targetFirm->center_y;
					else
						hitYLoc = targetFirm->loc_y2;

					set_attack_dir(curXLoc, curYLoc, hitXLoc, hitYLoc);
				}

				if(is_dir_correct())
					set_attack();
				else
					set_turn();
			}

			err_when(cur_x!=next_x || cur_y!=next_y);
		}
   }

	err_when(action_mode==ACTION_STOP && cur_action==SPRITE_ATTACK);
}
//----------- End of function Unit::process_attack_firm -----------//


//--------- Begin of function Unit::attack_town ---------//
// the unit calling this function is to attack target by (1) default
// ordering or by (2) defense mode
//
// Note : (townXLoc, townYLoc) should be the upper left corner location
//			 of the town.
//
// <int>	townXLoc				-	x location of the town
// <int>	townYLoc				-	y location of the town
// <int>	xOffset				-	x offset to the town
// <int>	yOffset				-	y offset to the town
// <int>	resetBlockedEdge	-	flag to reset variable block_edge
//
void Unit::attack_town(int townXLoc, int townYLoc, int xOffset, int yOffset, int resetBlockedEdge)
{
	//------------------------------------------------------------//
	// return if this unit cannot do the attack action
	//------------------------------------------------------------//
	if(!can_attack())
	{
		stop2(KEEP_DEFENSE_MODE);
		return;
	}
	else if(is_unit_dead())
		return;
	err_when(!can_attack());
	
	Location *locPtr = world.get_loc(townXLoc, townYLoc);
	err_when(!locPtr);

	//------------------------------------------------------------//
	// no town there
	//------------------------------------------------------------//
	if(!locPtr->is_town())
	{
		if(action_mode2==ACTION_AUTO_DEFENSE_ATTACK_TARGET || action_mode2==ACTION_DEFEND_TOWN_ATTACK_TARGET ||
			action_mode2==ACTION_MONSTER_DEFEND_ATTACK_TARGET)
		{
			stop(KEEP_DEFENSE_MODE);
			err_when((action_mode2!=ACTION_AUTO_DEFENSE_DETECT_TARGET || action_para2!=AUTO_DEFENSE_DETECT_COUNT) &&
						(action_mode2!=ACTION_DEFEND_TOWN_DETECT_TARGET || action_para2!=UNIT_DEFEND_TOWN_DETECT_COUNT) &&
						(action_mode2!=ACTION_MONSTER_DEFEND_DETECT_TARGET || action_para2!=MONSTER_DEFEND_DETECT_COUNT));
		}

		err_when(action_mode==ACTION_STOP && cur_action==SPRITE_ATTACK);
		err_when(action_mode==ACTION_ATTACK_TOWN || action_para || action_x_loc!=-1 || action_y_loc!=-1);
		return;
	}

	//------------------------------------------------------------//
	// cannot attack this nation
	//------------------------------------------------------------//
	Town *townPtr = town_array[locPtr->town_recno()];
	err_when(townPtr->loc_x1!=townXLoc || townPtr->loc_y1!=townYLoc);
	if(!nation_can_attack(townPtr->nation_recno))
	{
		stop2(KEEP_DEFENSE_MODE);
		err_when(action_mode==ACTION_STOP && cur_action==SPRITE_ATTACK);
		err_when(action_mode==ACTION_ATTACK_FIRM && !action_para);
		return;
	}

	//------------------------------------------------------------------------------------//
	// move there if cannot reach the effective attacking region
	//------------------------------------------------------------------------------------//
	int diffTerritoryAttack = 0, maxRange=0;
	if(mobile_type!=UNIT_AIR && world.get_loc(next_x_loc(), next_y_loc())->region_id!=locPtr->region_id)
	{
		maxRange = max_attack_range();
		if(!possible_place_for_range_attack(townXLoc, townYLoc, STD_TOWN_LOC_WIDTH, STD_TOWN_LOC_HEIGHT, maxRange))
		{
			if(action_mode2!=ACTION_AUTO_DEFENSE_ATTACK_TARGET && action_mode2!=ACTION_DEFEND_TOWN_ATTACK_TARGET &&
				action_mode!=ACTION_MONSTER_DEFEND_ATTACK_TARGET)
				move_to(townXLoc, townYLoc);

			return;
		}
		else // can reach
			diffTerritoryAttack = 1;
	}

	//----------------------------------------------------------------//
	// action_mode2: checking for equal action or idle action
	//----------------------------------------------------------------//
	if((action_mode2==ACTION_ATTACK_TOWN || action_mode2==ACTION_AUTO_DEFENSE_ATTACK_TARGET ||
		 action_mode2==ACTION_DEFEND_TOWN_ATTACK_TARGET || action_mode2==ACTION_MONSTER_DEFEND_ATTACK_TARGET) &&
		action_para2==townPtr->town_recno && action_x_loc2==townXLoc && action_y_loc2==townYLoc)
	{
		//----------- old order ------------//
		err_when(action_mode==ACTION_STOP && cur_action==SPRITE_ATTACK);
		err_when(action_mode==ACTION_ATTACK_TOWN && !action_para);

		if(cur_action!=SPRITE_IDLE)
		{
			//-------- old order is processing -------//
			#ifdef DEBUG
			if(action_mode==ACTION_ATTACK_TOWN)
			{
				err_when(action_mode2!=ACTION_ATTACK_TOWN && action_mode2!=ACTION_AUTO_DEFENSE_ATTACK_TARGET &&
							action_mode2!=ACTION_DEFEND_TOWN_ATTACK_TARGET && action_mode2!=ACTION_MONSTER_DEFEND_ATTACK_TARGET);
				err_when(action_para!=action_para2 || action_x_loc!=action_x_loc2 || action_y_loc!=action_y_loc2);
			}
			else
				err_when(action_mode!=ACTION_STOP || action_para || action_x_loc!=-1 || action_y_loc!=-1);
			#endif
			return;
		}
	}
	else
	{
		//------------ new order -------------//
		if(action_mode2!=ACTION_AUTO_DEFENSE_ATTACK_TARGET && action_mode2!=ACTION_DEFEND_TOWN_ATTACK_TARGET &&
			action_mode2!=ACTION_MONSTER_DEFEND_ATTACK_TARGET)
			action_mode2  = ACTION_ATTACK_TOWN;

		action_para2  = townPtr->town_recno;
		action_x_loc2 = townXLoc;
		action_y_loc2 = townYLoc;
	}

   //-------------------------------------------------------------//
	// process new order
	//-------------------------------------------------------------//
	stop();
	cur_attack = 0;

	int attackDistance = cal_distance(townXLoc, townYLoc, STD_TOWN_LOC_WIDTH, STD_TOWN_LOC_HEIGHT);
	choose_best_attack_mode(attackDistance);

	AttackInfo* attackInfo = attack_info_array+cur_attack;
	if(attackInfo->attack_range<attackDistance)
	{
		int searchResult = 1;

		if(xOffset || yOffset)
		{
			int xLoc = townXLoc+xOffset, yLoc = townYLoc+yOffset;
			if(xLoc<0) xLoc = 0;
			else if(xLoc>=MAX_WORLD_X_LOC) xLoc = MAX_WORLD_X_LOC-1;
			if(yLoc<0) yLoc = 0;
			else if(yLoc>=MAX_WORLD_Y_LOC) yLoc = MAX_WORLD_Y_LOC-1;

			search(xLoc, yLoc, 1); // offset location is given, so move there directly
		}
		else // without offset given, so call set_move_to_surround()
		{
			if(diffTerritoryAttack)
			{
				//--------------------------------------------------------------------------------//
				// 1) different type from target, target located in different territory but able to
				//		attack this target by range attacking
				//--------------------------------------------------------------------------------//
				move_to_range_attack(townXLoc, townYLoc, 0, SEARCH_MODE_ATTACK_TOWN_BY_RANGE, maxRange);
			}
			else
			{
				//--------------------------------------------------------------------------------//
				// 1) same type of target,
				// 2) this unit is air unit, or
				// 3) different type from target, but target located in the same territory
				//--------------------------------------------------------------------------------//
				searchResult = set_move_to_surround(townXLoc, townYLoc, STD_TOWN_LOC_WIDTH, STD_TOWN_LOC_HEIGHT, BUILDING_TYPE_TOWN_MOVE_TO, 0, 0);
			}
		}

		//---------------------------------------------------------------//
		// initialize parameters for blocked edge handling in attacking
		//---------------------------------------------------------------//
		if(searchResult)
		{
			waiting_term = 0;
			if(resetBlockedEdge)
				memset(blocked_edge, 0, sizeof(char)*4);
		}
		else
			memset(blocked_edge, 0xff, sizeof(char)*4); // for reactivating idle action
	}
	else if(cur_action==SPRITE_IDLE)
	{
		//---------------------------------------------------------------//
		// attack now
		//---------------------------------------------------------------//
		set_cur(next_x, next_y);
		set_attack_dir(next_x_loc(), next_y_loc(), townPtr->center_x, townPtr->center_y);
		if(is_dir_correct())
		{
			if(attackInfo->attack_range==1)
				set_attack();
		}
		else
			set_turn();
	}

	action_mode  = ACTION_ATTACK_TOWN;
	action_para  = townPtr->town_recno;
	action_x_loc = townXLoc;
	action_y_loc = townYLoc;

	err_when(action_mode==ACTION_ATTACK_TOWN && action_para==0);
	err_when(action_mode==ACTION_STOP && cur_action==SPRITE_ATTACK);
	err_when(action_mode!=action_mode2 && action_mode!=ACTION_ATTACK_TOWN);
	err_when(action_para!=action_para2 || !action_para);
	err_when(action_x_loc!=action_x_loc2 || action_y_loc!=action_y_loc2);
}
//----------- End of function Unit::attack_town -----------//


//--------- Begin of function Unit::process_attack_town ---------//
// functions process attacking town
//
void Unit::process_attack_town()
{
	if(!can_attack())
	{
		stop2(KEEP_DEFENSE_MODE);
		return;
	}

	err_when(!action_para || action_mode!=ACTION_ATTACK_TOWN || !can_attack());	// unable to attack
	// ###### begin Gilbert 17/3 #######//
	//err_when(attack_info_array[cur_attack].attack_range != attack_range);
	// ###### end Gilbert 17/3 #######//

	Town* targetTown;
	int	clearOrder = 0;
	//------------------------------------------------------------//
	// check attack conditions
	//------------------------------------------------------------//
	if(town_array.is_deleted(action_para))
		clearOrder++;
	else
	{
		targetTown = town_array[action_para];
		err_when(!targetTown || action_para!=targetTown->town_recno);
		err_when(action_x_loc != targetTown->loc_x1 || action_y_loc != targetTown->loc_y1);

		if(!nation_can_attack(targetTown->nation_recno)) // cannot attack this nation
			clearOrder++;
	}

	if(clearOrder)
	{
		//------------------------------------------------------------//
		// change to detect target if in defend mode
		//------------------------------------------------------------//
		/*if(action_mode2==ACTION_AUTO_DEFENSE_ATTACK_TARGET || action_mode2==ACTION_DEFEND_TOWN_ATTACK_TARGET ||
			action_mode2==ACTION_MONSTER_DEFEND_ATTACK_TARGET)
		{
			err_when((action_misc!=ACTION_MISC_DEFENSE_CAMP_RECNO && action_misc!=ACTION_MISC_DEFEND_TOWN_RECNO &&
						 action_misc!=ACTION_MISC_MONSTER_DEFEND_FIRM_RECNO) || !action_misc_para);
			stop2(KEEP_DEFENSE_MODE);
			return;
		}

		err_when(action_mode2==ACTION_AUTO_DEFENSE_DETECT_TARGET || action_mode2==ACTION_AUTO_DEFENSE_BACK_CAMP ||
					action_mode2==ACTION_DEFEND_TOWN_DETECT_TARGET || action_mode2==ACTION_DEFEND_TOWN_BACK_TOWN ||
					action_mode2==ACTION_MONSTER_DEFEND_DETECT_TARGET || action_mode2==ACTION_MONSTER_DEFEND_BACK_FIRM);

		stop2(); // clear order
		err_when(cur_action==SPRITE_ATTACK && (move_to_x_loc!=next_x_loc() || move_to_y_loc!=next_y_loc()));
		err_when(action_mode==ACTION_ATTACK_TOWN && !action_para);
		err_when(cur_action==SPRITE_ATTACK && action_mode==ACTION_STOP);
		return;*/
		#ifdef DEBUG
			int inDefMode = in_any_defense_mode();
		#endif

		stop2(KEEP_DEFENSE_MODE);

		#ifdef DEBUG
			if(inDefMode)
			{
				err_when((action_misc!=ACTION_MISC_DEFENSE_CAMP_RECNO && action_misc!=ACTION_MISC_DEFEND_TOWN_RECNO &&
							 action_misc!=ACTION_MISC_MONSTER_DEFEND_FIRM_RECNO) || !action_misc_para);
			}
		#endif

		err_when(cur_action==SPRITE_ATTACK && (move_to_x_loc!=next_x_loc() || move_to_y_loc!=next_y_loc()));
		err_when(action_mode==ACTION_ATTACK_TOWN && !action_para);
		err_when(cur_action==SPRITE_ATTACK && action_mode==ACTION_STOP);
		return;
	}

	err_when(ai_unit && nation_recno==targetTown->nation_recno); // ai unit don't attack itself, but players can

	//-----------------------------------------------------//
	// If the unit is currently attacking somebody.
	//-----------------------------------------------------//
	if(cur_action==SPRITE_ATTACK)
   {
		err_when(cur_x!=next_x || cur_y!=next_y || !is_dir_correct());
		//###### begin alex 18/3 #######//
		err_when(attack_info_array[cur_attack].attack_range != attack_range);
		//###### end alex 18/3 #######//
		if(remain_attack_delay)
			return;
		
		AttackInfo* attackInfo  	  = attack_info_array+cur_attack;
		if( attackInfo->attack_range > 1 )	// range attack
		{
			//---------- wait for bullet emit ---------//
			if(cur_frame!=attackInfo->bullet_out_frame)
				return;

			//------- seek location to attack target by bullet --------//
			int curXLoc = next_x_loc();
			int curYLoc = next_y_loc();
			if(!bullet_array.bullet_path_possible(curXLoc, curYLoc, mobile_type, range_attack_x_loc, range_attack_y_loc, UNIT_LAND, attackInfo->bullet_speed, attackInfo->bullet_sprite_id))
			{
				if(!bullet_array.add_bullet_possible(curXLoc, curYLoc, mobile_type, action_x_loc, action_y_loc, UNIT_LAND, STD_TOWN_LOC_WIDTH, STD_TOWN_LOC_HEIGHT,
					range_attack_x_loc, range_attack_y_loc, attackInfo->bullet_speed, attackInfo->bullet_sprite_id))
				{
					//----- no suitable location, move to target --------//
					set_move_to_surround(action_x_loc, action_y_loc, STD_TOWN_LOC_WIDTH, STD_TOWN_LOC_HEIGHT, BUILDING_TYPE_TOWN_MOVE_TO);
					err_when(action_mode!=ACTION_ATTACK_TOWN || action_para!=targetTown->town_recno);
					return;
				}
			}

			//--------- add bullet, bullet emits --------//
			bullet_array.add_bullet(this, action_x_loc, action_y_loc);
			// ####### begin Gilbert 14/7 ########//
			add_close_attack_effect();
			// ####### end Gilbert 14/7 ########//

			// ------- reduce power --------//
			cur_power -= attackInfo->consume_power;
			err_when(cur_power < 0);
			//### begin alex 28/10 ###//
			if(cur_power<0) // ***** BUGHERE
				cur_power = 0;
			//#### end alex 28/10 ####//
			set_remain_attack_delay();
			return;
		}
		else // close attack
		{
			if(cur_frame!=cur_sprite_attack()->frame_count)
				return; // attacking

			err_when(final_dir != (check_unit_dir1 = get_dir(next_x_loc(), next_y_loc(), targetTown->center_x, targetTown->center_y)));
			hit_town(this, action_x_loc, action_y_loc, actual_damage(), nation_recno);
			// ####### begin Gilbert 14/7 ########//
			add_close_attack_effect();
			// ####### end Gilbert 14/7 ########//
			
			// ------- reduce power --------//
			cur_power -= attackInfo->consume_power;
			err_when(cur_power < 0);
			//### begin alex 28/10 ###//
			if(cur_power<0) // ***** BUGHERE
				cur_power = 0;
			//#### end alex 28/10 ####//
			set_remain_attack_delay();
		}
   }
	//--------------------------------------------------------------------------------------------------//
	// If the unit is on its way to attack the town, if it has gotten close next to it, attack now
   //--------------------------------------------------------------------------------------------------//
	else if(abs(cur_x-next_x)<=sprite_info->speed && abs(cur_y-next_y)<=sprite_info->speed)// it has moved to the specified location. check cur_x & go_x to make sure the sprite has completely move to the location, not just crossing it.
   {
		err_when(cur_action==SPRITE_ATTACK);

		if(mobile_type==UNIT_LAND)
		{
			if(detect_surround_target())
				return;
		}

		if(attack_range==1)
		{
			//------------------------------------------------------------//
			// for close attack, the unit unable to attack the firm if
			// it is not in the firm surrounding
			//------------------------------------------------------------//
			if(result_path_dist > attack_range)
				return;
		}

		int targetXLoc	= targetTown->loc_x1;
		int targetYLoc	= targetTown->loc_y1;

		int attackDistance = cal_distance(targetXLoc , targetYLoc, STD_TOWN_LOC_WIDTH, STD_TOWN_LOC_HEIGHT);

		if(attackDistance<=attack_range) // able to attack target
		{
			if((attackDistance==1) && attack_range>1) // often false condition is checked first
				choose_best_attack_mode(1); // may change to use close attack

			if(attack_range>1) // use range attack
			{
				set_cur(next_x, next_y);

				AttackInfo *attackInfo = attack_info_array+cur_attack;
				int curXLoc = next_x_loc();
				int curYLoc = next_y_loc();
				if(!bullet_array.add_bullet_possible(curXLoc, curYLoc, mobile_type, targetXLoc, targetYLoc, UNIT_LAND, STD_TOWN_LOC_WIDTH, STD_TOWN_LOC_HEIGHT,
					range_attack_x_loc, range_attack_y_loc, attackInfo->bullet_speed, attackInfo->bullet_sprite_id))
				{
					//------- no suitable location, move to target ---------//
					if(!result_node_array || !result_node_count) // no step for continuing moving
						set_move_to_surround(action_x_loc, action_y_loc, STD_TOWN_LOC_WIDTH, STD_TOWN_LOC_HEIGHT, BUILDING_TYPE_TOWN_MOVE_TO);

					err_when(action_mode!=ACTION_ATTACK_TOWN || action_para!=targetTown->town_recno);
					return;	// unable to attack, continue to move
				}

				//---------- able to do range attack ----------//
				set_attack_dir(next_x_loc(), next_y_loc(), range_attack_x_loc, range_attack_y_loc);
				cur_frame  = 1;

				if(is_dir_correct())
					set_attack();
				else
					set_turn();
			}
			else // close attack
			{
				//---------- attack now ---------//
				set_cur(next_x, next_y);
				terminate_move();
				err_when(cur_frame!=1);
				set_dir(next_x_loc(), next_y_loc(), targetTown->center_x, targetTown->center_y);

				if(is_dir_correct())
					set_attack();
				else
					set_turn();
			}

			err_when(cur_x!=next_x || cur_y!=next_y);
		}
   }

	err_when(action_mode==ACTION_STOP && cur_action==SPRITE_ATTACK);
}
//----------- End of function Unit::process_attack_town -----------//


//--------- Begin of function Unit::attack_wall ---------//
// <int> wallXLoc, wallYLoc - the wall location
// <int> xOffset, yOffset   - the offset from the wall location
//										for the unit to move to
// <int>	resetBlockedEdge	 - flag to reset variable blocked_edge
//
// the unit calling this function is to attack target by (1) default
// ordering or by (2) defense mode
//
// Note : (wallXLoc, wallYLoc) should be the upper left corner location
//			 of the wall
//
void Unit::attack_wall(int wallXLoc, int wallYLoc, int xOffset, int yOffset, int resetBlockedEdge)
{
	//------------------------------------------------------------//
	// return if this unit cannot do the attack action
	//------------------------------------------------------------//
	if(!can_attack())
	{
		stop2(KEEP_DEFENSE_MODE);
		return;
	}
	else if(is_unit_dead())
		return;
	err_when(!can_attack());

	Location *locPtr = world.get_loc(wallXLoc, wallYLoc);
	err_when(!locPtr);

	//------------------------------------------------------------//
	// no wall there
	//------------------------------------------------------------//
	if(!locPtr->is_wall())
	{
		if(action_mode2==ACTION_AUTO_DEFENSE_ATTACK_TARGET || action_mode2==ACTION_DEFEND_TOWN_ATTACK_TARGET ||
			action_mode2==ACTION_MONSTER_DEFEND_ATTACK_TARGET)
		{
			stop(KEEP_DEFENSE_MODE);
			err_when((action_mode2!=ACTION_AUTO_DEFENSE_DETECT_TARGET || action_para2!=AUTO_DEFENSE_DETECT_COUNT) &&
						(action_mode2!=ACTION_DEFEND_TOWN_DETECT_TARGET || action_para2!=UNIT_DEFEND_TOWN_DETECT_COUNT) &&
						(action_mode2!=ACTION_MONSTER_DEFEND_DETECT_TARGET || action_para2!=MONSTER_DEFEND_DETECT_COUNT));
		}

		err_when(action_mode==ACTION_ATTACK_WALL && action_para);
		err_when(action_mode==ACTION_STOP && cur_action==SPRITE_ATTACK);
		return;
	}

	//------------------------------------------------------------//
	// cannot attack this nation
	//------------------------------------------------------------//
	if(!nation_can_attack(locPtr->wall_nation_recno()))
	{
		stop2(KEEP_DEFENSE_MODE);
		err_when(action_mode==ACTION_STOP && cur_action==SPRITE_ATTACK);
		err_when(action_mode==ACTION_ATTACK_WALL || action_para || action_x_loc!=-1 || action_y_loc!=-1);
		return;
	}

	//------------------------------------------------------------------------------------//
	// move there if cannot reach the effective attacking region
	//------------------------------------------------------------------------------------//
	int diffTerritoryAttack = 0, maxRange=0;
	if(mobile_type!=UNIT_AIR && world.get_loc(next_x_loc(), next_y_loc())->region_id!=locPtr->region_id)
	{
		maxRange = max_attack_range();
		if(!possible_place_for_range_attack(wallXLoc, wallYLoc, 1, 1, maxRange))
		{
			if(action_mode2!=ACTION_AUTO_DEFENSE_ATTACK_TARGET && action_mode2!=ACTION_DEFEND_TOWN_ATTACK_TARGET &&
				action_mode!=ACTION_MONSTER_DEFEND_ATTACK_TARGET)
				move_to(wallXLoc, wallYLoc);

			return;
		}
		else // can reach
			diffTerritoryAttack = 1;
	}

	//----------------------------------------------------------------//
	// action_mode2: checking for equal action or idle action
	//----------------------------------------------------------------//
	if((action_mode2==ACTION_ATTACK_WALL || action_mode2==ACTION_AUTO_DEFENSE_ATTACK_TARGET ||
		 action_mode2==ACTION_DEFEND_TOWN_ATTACK_TARGET || action_mode2==ACTION_MONSTER_DEFEND_ATTACK_TARGET) &&
		!action_para2 && action_x_loc2==wallXLoc && action_y_loc2==wallYLoc)
	{
		//------------ old order ------------//
		err_when(action_mode==ACTION_ATTACK_WALL && action_para);
		err_when(action_mode==ACTION_STOP && cur_action==SPRITE_ATTACK);

		if(cur_action!=SPRITE_IDLE)
		{
			//------- old order is processing --------//
			#ifdef DEBUG
			if(action_mode==ACTION_ATTACK_WALL)
			{
				err_when(action_mode2!=ACTION_ATTACK_WALL && action_mode2!=ACTION_AUTO_DEFENSE_ATTACK_TARGET &&
							action_mode2!=ACTION_DEFEND_TOWN_ATTACK_TARGET && action_mode2!=ACTION_MONSTER_DEFEND_ATTACK_TARGET);
				err_when(action_para!=action_para2 || action_x_loc!=action_x_loc2 || action_y_loc!=action_y_loc2);
			}
			else
				err_when(action_mode!=ACTION_STOP || action_para || action_x_loc!=-1 || action_y_loc!=-1);
			#endif
			return;
		}
	}
	else
	{
		//-------------- new order -------------//
		if(action_mode2!=ACTION_AUTO_DEFENSE_ATTACK_TARGET && action_mode2!=ACTION_DEFEND_TOWN_ATTACK_TARGET &&
			action_mode2!=ACTION_MONSTER_DEFEND_ATTACK_TARGET)
			action_mode2  = ACTION_ATTACK_WALL;

		action_para2  = 0;
		action_x_loc2 = wallXLoc;
		action_y_loc2 = wallYLoc;
	}

   //-------------------------------------------------------------//
	// process new order
	//-------------------------------------------------------------//
	stop();
	cur_attack = 0;

	int attackDistance = cal_distance(wallXLoc, wallYLoc, 1, 1);
	choose_best_attack_mode(attackDistance);

	AttackInfo* attackInfo = attack_info_array+cur_attack;
	if(attackInfo->attack_range<attackDistance)
	{
		int searchResult = 1;

		if(xOffset || yOffset)
		{
			int xLoc = wallXLoc+xOffset, yLoc = wallYLoc+yOffset;
			if(xLoc<0) xLoc = 0;
			else if(xLoc>=MAX_WORLD_X_LOC) xLoc = MAX_WORLD_X_LOC-1;
			if(yLoc<0) yLoc = 0;
			else if(yLoc>=MAX_WORLD_Y_LOC) yLoc = MAX_WORLD_Y_LOC-1;

			search(xLoc, yLoc, 1); // offset location is given, so move there directly
		}
		else
		{
			if(diffTerritoryAttack)
			{
				//--------------------------------------------------------------------------------//
				// 1) different type from target, target located in different territory but able to
				//		attack this target by range attacking
				//--------------------------------------------------------------------------------//
				move_to_range_attack(wallXLoc, wallYLoc, 0, SEARCH_MODE_ATTACK_WALL_BY_RANGE, maxRange);
			}
			else
			{
				//--------------------------------------------------------------------------------//
				// 1) same type of target,
				// 2) this unit is air unit, or
				// 3) different type from target, but target located in the same territory
				//--------------------------------------------------------------------------------//
				searchResult = set_move_to_surround(wallXLoc, wallYLoc, 1, 1, BUILDING_TYPE_WALL, 0, 0);
			}
		}

		//---------------------------------------------------------------//
		// initialize parameters for blocked edge handling in attacking
		//---------------------------------------------------------------//
		if(searchResult)
		{
			waiting_term = 0;
			if(resetBlockedEdge)
				memset(blocked_edge, 0, sizeof(char)*4);
		}
		else
			memset(blocked_edge, 0xff, sizeof(char)*4);
	}
	else if(cur_action==SPRITE_IDLE)
	{
		//---------------------------------------------------------------//
		// attack now
		//---------------------------------------------------------------//
		set_cur(next_x, next_y);
		set_attack_dir(next_x_loc(), next_y_loc(), wallXLoc, wallYLoc);
		if(is_dir_correct())
		{
			if(attackInfo->attack_range==1)
				set_attack();
		}
		else
			set_turn();
	}

	action_mode  = ACTION_ATTACK_WALL;
	action_para  = 0;
	action_x_loc = wallXLoc;
	action_y_loc = wallYLoc;

	err_when(action_mode==ACTION_ATTACK_WALL && action_para);
	err_when(action_mode==ACTION_STOP && cur_action==SPRITE_ATTACK);
	err_when(action_mode!=action_mode2 && action_mode!=ACTION_ATTACK_WALL);
	err_when(action_para!=action_para2 || action_para);
	err_when(action_x_loc!=action_x_loc2 || action_y_loc!=action_y_loc2);
}
//----------- End of function Unit::attack_wall -----------//


//--------- Begin of function Unit::process_attack_wall ---------//
// process attack wall
//
void Unit::process_attack_wall()
{
	if(!can_attack())
	{
		stop2(KEEP_DEFENSE_MODE);
		return;
	}

	err_when(action_para || action_mode!=ACTION_ATTACK_WALL); // action_para should be 0
	err_when(!can_attack());	// unable to attack
	// ###### begin Gilbert 17/3 #######//
	//err_when(attack_info_array[cur_attack].attack_range != attack_range);
	// ###### end Gilbert 17/3 #######//

	//------------------------------------------------------------//
	// if the targeted wall has been destroyed
	//------------------------------------------------------------//
	Location *locPtr = world.get_loc(action_x_loc, action_y_loc);
	if(!locPtr->is_wall())
	{
		stop2(KEEP_DEFENSE_MODE);
		err_when(cur_action==SPRITE_ATTACK && action_mode==ACTION_STOP);
		return;
	}

	//-----------------------------------------------------//
	// If the unit is currently attacking.
	//-----------------------------------------------------//
	if(cur_action==SPRITE_ATTACK)
   {
		err_when(cur_x!=next_x || cur_y!=next_y || !is_dir_correct());
		//###### begin alex 18/3 #######//
		err_when(attack_info_array[cur_attack].attack_range != attack_range);
		//###### end alex 18/3 #######//
		if(remain_attack_delay)
			return;

		AttackInfo* attackInfo  	  = attack_info_array+cur_attack;
		if( attackInfo->attack_range > 1 )	// range attack
		{
			//--------- wait for bullet emit ----------//
			if(cur_frame!=attackInfo->bullet_out_frame)
				return;

			//---------- seek location to attack target by bullet --------//
			int curXLoc = next_x_loc();
			int curYLoc = next_y_loc();
			if(!bullet_array.bullet_path_possible(curXLoc, curYLoc, mobile_type, range_attack_x_loc, range_attack_y_loc, UNIT_LAND, attackInfo->bullet_speed, attackInfo->bullet_sprite_id))
			{
				if(!bullet_array.add_bullet_possible(curXLoc, curYLoc, mobile_type, action_x_loc, action_y_loc, UNIT_LAND, 1, 1,
					range_attack_x_loc, range_attack_y_loc, attackInfo->bullet_speed, attackInfo->bullet_sprite_id))
				{
					//--------- no suitable location, move to target ----------//
					set_move_to_surround(action_x_loc, action_y_loc, 1, 1, BUILDING_TYPE_WALL);
					err_when(action_mode!=ACTION_ATTACK_WALL || action_para);
					return;
				}
			}

			//---------- add bullet, bullet emits -----------//
			bullet_array.add_bullet(this, action_x_loc, action_y_loc);
			// ####### begin Gilbert 14/7 ########//
			add_close_attack_effect();
			// ####### end Gilbert 14/7 ########//

			// ------- reduce power --------//
			cur_power -= attackInfo->consume_power;
			err_when(cur_power < 0);
			//### begin alex 28/10 ###//
			if(cur_power<0) // ***** BUGHERE
				cur_power = 0;
			//#### end alex 28/10 ####//
			set_remain_attack_delay();
			return;
		}
		else
		{
			if(cur_frame!=cur_sprite_attack()->frame_count)
				return; // attacking

			err_when(final_dir!=(check_unit_dir1=get_dir(move_to_x_loc, move_to_y_loc, action_x_loc, action_y_loc)));
			hit_wall(this, action_x_loc, action_y_loc, actual_damage(), nation_recno);
			// ####### begin Gilbert 14/7 ########//
			add_close_attack_effect();
			// ####### end Gilbert 14/7 ########//

			//------- reduce power --------//
			cur_power -= attackInfo->consume_power;
			err_when(cur_power < 0);
			//### begin alex 28/10 ###//
			if(cur_power<0) // ***** BUGHERE
				cur_power = 0;
			//#### end alex 28/10 ####//
			set_remain_attack_delay();
		}
   }
   //--------------------------------------------------------------------------------------------------//
   // If the unit is on its way to attack somebody, if it has gotten close next to the target, attack now
   //--------------------------------------------------------------------------------------------------//
	else if(abs(cur_x-next_x)<=sprite_info->speed && abs(cur_y-next_y)<=sprite_info->speed )     // it has moved to the specified location. check cur_x & go_x to make sure the sprite has completely move to the location, not just crossing it.
   {
		err_when(cur_action==SPRITE_ATTACK);

		if(mobile_type==UNIT_LAND)
		{
			if(detect_surround_target())
				return;
		}

		if(attack_range==1)
		{
			//------------------------------------------------------------//
			// for close attack, the unit unable to attack the firm if
			// it is not in the firm surrounding
			//------------------------------------------------------------//
			if(result_path_dist > attack_range)
				return;
		}

		int attackDistance = cal_distance(action_x_loc, action_y_loc, 1, 1);

		if(attackDistance<=attack_range) // able to attack target
		{
			if((attackDistance==1) && attack_range>1) // often false condition is checked first
				choose_best_attack_mode(1); // may change to use close attack

			if(attack_range>1) // use range attack
			{
				set_cur(next_x, next_y);

				AttackInfo *attackInfo = attack_info_array+cur_attack;
				int curXLoc = next_x_loc();
				int curYLoc = next_y_loc();
				if(!bullet_array.add_bullet_possible(curXLoc, curYLoc, mobile_type, action_x_loc, action_y_loc, UNIT_LAND, 1, 1,
					range_attack_x_loc, range_attack_y_loc, attackInfo->bullet_speed, attackInfo->bullet_sprite_id))
				{
					//------- no suitable location, move to target ---------//
					if(!result_node_array || !result_node_count) // no step for continuing moving
						set_move_to_surround(action_x_loc, action_y_loc, 1, 1, BUILDING_TYPE_WALL);

					err_when(action_mode!=ACTION_ATTACK_WALL || action_para);
					return;	// unable to attack, continue to move
				}

				//---------- able to do range attack ----------//
				set_attack_dir(curXLoc, curYLoc, range_attack_x_loc, range_attack_y_loc);
				cur_frame  = 1;

				if(is_dir_correct())
					set_attack();
				else
					set_turn();
			}
			else // close attack
			{
				//---------- attack now ---------//
				set_cur(next_x, next_y);
				terminate_move();
				err_when(cur_frame!=1);
				set_attack_dir(next_x_loc(), next_y_loc(), action_x_loc, action_y_loc);

				if(is_dir_correct())
					set_attack();
				else
					set_turn();
			}

			err_when(cur_x!=next_x || cur_y!=next_y);
		}
	}

	err_when(action_mode==ACTION_STOP && cur_action==SPRITE_ATTACK);
}
//----------- End of function Unit::process_attack_wall -----------//
