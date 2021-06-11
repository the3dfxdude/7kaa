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

//Filename    : OUNITAT.CPP
//Description : Object Unit attack supporting functions
//Owner		  : Alex

#include <ALL.h>
#include <ONEWS.h>
#include <OWORLD.h>
#include <ONATION.h>
#include <OU_MARI.h>
#include <OSPY.h>
#include <OGAME.h>
#include <OTOWN.h>
#include <OF_CAMP.h>
#include <OF_MONS.h>
#include <OEFFECT.h>
#include <OMONSRES.h>
#include <OREBEL.h>
#include <OSERES.h>
#include <OSYS.h>
#include <OWARPT.h>

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

//----------------- Begin of function Unit::update_attack_path_dist -----------------//
// return 1 if it is time to update result path
// return 0 otherwise
//
int Unit::update_attack_path_dist()
{
	if(result_path_dist<=6)	//1-6
	{
		return 1;
	}
	else if(result_path_dist<=10) // 8, 10
	{
		return !((result_path_dist-6)%2);
	}
	else if(result_path_dist<=20) // 15, 20
	{
		return !((result_path_dist-10)%5);
	}
	else if(result_path_dist<=60) // 28, 36, 44, 52, 60
	{
		return !((result_path_dist-20)%8);
	}
	else if(result_path_dist<=90) // 75, 90
	{
		return !((result_path_dist-60)%15);
	}
	else // 110, 130, 150, etc
	{
		return !((result_path_dist-90)%20);
	}

	err_here();
	return 0;
}
//---------- End of function Unit::update_attack_path_dist ----------//

//##### begin trevor 15/8 #######//

//----------------- Begin of function Unit::hit_target -----------------//
//	The function can be called in by class Bullet since parent and target
//	are specified.
//
// <Unit*>  parentUnit is the unit attacking
// <Unit*>  targetUnit is the unit being attacked
// <int>    attackDamage is the damage done to the target Unit
// <short>  parentNationRecno - the nation that ordered the hit
//
// ****************************** Warning ***********************************
// don't use any member variables of this unit. This unit may not be involved
// in the attack event
// **************************************************************************
//
void Unit::hit_target(Unit* parentUnit, Unit* targetUnit, float attackDamage, short parentNationRecno)
{
	short	targetNationRecno = targetUnit->nation_recno;

	//------------------------------------------------------------//
	// if the attacked unit is in defense mode, order other available
	// unit in the same camp to help this unit
	// Note : checking for nation_recno since one unit can attack units
	//			 in same nation by bullet accidentally
	//------------------------------------------------------------//
	if(parentUnit && parentUnit->cur_action!=SPRITE_DIE && parentUnit->is_visible() &&
		parentNationRecno!=targetNationRecno && parentUnit->nation_can_attack(targetNationRecno) &&
		targetUnit->in_auto_defense_mode())
	{
		err_when(targetUnit->action_misc!=ACTION_MISC_DEFENSE_CAMP_RECNO || !targetUnit->action_misc_para);
		if(!firm_array.is_deleted(targetUnit->action_misc_para))
		{
			Firm *firmPtr = firm_array[targetUnit->action_misc_para];
			if(firmPtr->firm_id==FIRM_CAMP)
			{
				err_when(firmPtr->firm_id!=FIRM_CAMP);
				FirmCamp *campPtr = firmPtr->cast_to_FirmCamp();
				campPtr->defense(parentUnit->sprite_recno);
			}
		}
		else
			targetUnit->clear_unit_defense_mode();
	}

	// ---------- add indicator on the map ----------//
	if( nation_array.player_recno && targetUnit->is_own() )
		war_point_array.add_point(targetUnit->next_x_loc(),targetUnit->next_y_loc());

	//-----------------------------------------------------------------------//
	// decrease the hit points of the target Unit
	//-----------------------------------------------------------------------//
	#define DEFAULT_ARMOR		4
	#define DEFAULT_ARMOR_OVER_ATTACK_SLOW_DOWN	(float) DEFAULT_ARMOR / ATTACK_SLOW_DOWN
	#define ONE_OVER_ATTACK_SLOW_DOWN				(float) 1/ATTACK_SLOW_DOWN
	#define COMPARE_POINT								DEFAULT_ARMOR_OVER_ATTACK_SLOW_DOWN + ONE_OVER_ATTACK_SLOW_DOWN

	//-*********** simulate aat ************-//
#ifdef DEBUG
		if(!debug_sim_game_type)
		{
#endif
			if( attackDamage >= COMPARE_POINT )
				targetUnit->hit_points -= attackDamage - DEFAULT_ARMOR_OVER_ATTACK_SLOW_DOWN;
			else
				targetUnit->hit_points -= MIN(attackDamage,ONE_OVER_ATTACK_SLOW_DOWN);  // in case attackDamage = 0, no hit_point is reduced
#ifdef DEBUG
		}
#endif
	//-*********** simulate aat ************-//

	Nation *parentNationPtr = parentNationRecno ? nation_array[parentNationRecno] : NULL;
	Nation *targetNationPtr = targetNationRecno ? nation_array[targetNationRecno] : NULL;
	char targetUnitClass = unit_res[targetUnit->unit_id]->unit_class;

	if( targetUnit->hit_points <= 0 )
	{
		targetUnit->hit_points = (float) 0;

		//---- if the unit killed is a human unit -----//

		if( targetUnit->race_id )
		{
			//---- if the unit killed is a town defender unit -----//

			if( targetUnit->is_civilian() && targetUnit->in_defend_town_mode() )
			{
				if( targetNationRecno )
				{
					targetNationPtr->civilian_killed(targetUnit->race_id, 0);
					targetNationPtr->own_civilian_killed++;
				}

				if( parentNationPtr )
				{
					parentNationPtr->civilian_killed(targetUnit->race_id, 1);
					parentNationPtr->enemy_civilian_killed++;
				}
			}
			else if( targetUnit->is_civilian() && targetUnit->skill.combat_level<20 ) //--- mobile civilian ---//
			{
				if( targetNationRecno )
				{
					targetNationPtr->civilian_killed(targetUnit->race_id, 0);
					targetNationPtr->own_civilian_killed++;
				}

				if( parentNationPtr )
				{
					parentNationPtr->civilian_killed(targetUnit->race_id, 0);
					parentNationPtr->enemy_civilian_killed++;
				}
			}
			else	//---- if the unit killed is a soldier -----//
			{
				if( targetNationRecno )
					targetNationPtr->own_soldier_killed++;

				if( parentNationPtr )
					parentNationPtr->enemy_soldier_killed++;
			}
		}

		//--------- if it's a non-human unit ---------//

		else
		{
			switch( unit_res[targetUnit->unit_id]->unit_class )
			{
				case UNIT_CLASS_WEAPON:
					if( parentNationPtr )
						parentNationPtr->enemy_weapon_destroyed++;

					if( targetNationRecno )
						targetNationPtr->own_weapon_destroyed++;
					break;


				case UNIT_CLASS_SHIP:
					if( parentNationPtr )
						parentNationPtr->enemy_ship_destroyed++;

					if( targetNationRecno )
						targetNationPtr->own_ship_destroyed++;
					break;
			}

			//---- if the unit destroyed is a trader or caravan -----//

			if( targetUnit->unit_id == UNIT_CARAVAN ||	// killing a caravan is resented by all races
				 targetUnit->unit_id == UNIT_VESSEL )
			{
				// Race-Id of 0 means a loyalty penalty applied for all races
				if( targetNationRecno )
					targetNationPtr->civilian_killed(0, -1);

				if( parentNationPtr )
					parentNationPtr->civilian_killed(0, 3);
			}
		}

		return;
	}

	if(parentUnit!=NULL && parentNationRecno!=targetNationRecno)
		parentUnit->gain_experience(); // gain experience to increase combat level

	//-----------------------------------------------------------------------//
	// action of the target to take
	//-----------------------------------------------------------------------//
	if( !parentUnit )	// do nothing if parent is dead
		return;

	if( parentUnit->cur_action==SPRITE_DIE ) // skip for explosive cart
	{
		err_when(parentUnit->unit_id!=UNIT_EXPLOSIVE_CART);
		return;
	}

	if( targetNationRecno == parentNationRecno )	// the target and the attacker's nations are different (it's possible that when a unit who has just changed nation has its bullet hitting its own nation)
		return;

	//------- two nations at war ---------//

	if( parentNationPtr && targetNationRecno )
	{
		parentNationPtr->set_at_war_today();
		targetNationPtr->set_at_war_today(parentUnit->sprite_recno);
	}

	//-------- increase battling fryhtan score --------//

	if( parentNationPtr && targetUnitClass==UNIT_CLASS_MONSTER )
	{
		parentNationPtr->kill_monster_score += (float) 0.1;
	}

	//------ call target unit being attack functions -------//

	if( targetNationRecno )
	{
		targetNationPtr->being_attacked(parentNationRecno);

		if( targetUnit->ai_unit )
		{
			if( targetUnit->rank_id >= RANK_GENERAL )
				targetUnit->ai_leader_being_attacked(parentUnit->sprite_recno);

			if( unit_res[targetUnit->unit_id]->unit_class == UNIT_CLASS_SHIP )
				((UnitMarine*)targetUnit)->ai_ship_being_attacked(parentUnit->sprite_recno);
		}

		//--- if a member in a troop is under attack, ask for other troop members to help ---//

		if( info.game_date%2 == sprite_recno%2 )
		{
			if( targetUnit->leader_unit_recno || 
				 (targetUnit->team_info && targetUnit->team_info->member_count > 1) )
			{
				if( !unit_array.is_deleted(parentUnit->sprite_recno) )		// it is possible that parentUnit is dying right now 
				{
					targetUnit->ask_team_help_attack(parentUnit);
				}
			}
		}
	}

	//--- increase reputation of the nation that attacks monsters ---//

	else if( targetUnitClass == UNIT_CLASS_MONSTER )
	{
		if( parentNationPtr )
			parentNationPtr->change_reputation(REPUTATION_INCREASE_PER_ATTACK_MONSTER);
	}

	//------------------------------------------//

	if(!targetUnit->can_attack())	// no action if the target unit is unable to attack
		return;

	err_when(!targetUnit->can_attack());

	targetUnit->unit_auto_guarding(parentUnit);
}
//---------- End of function Unit::hit_target ----------//

//##### end trevor 15/8 #######//


//----------- Begin of function Unit::unit_auto_guarding ---------------//
// the unit attacks the unit attacking it
//
// <Unit*> attackUnit	- the unit attacking this unit
//
void Unit::unit_auto_guarding(Unit *attackUnit)
{
	if( force_move_flag )
		return;

	//##### begin trevor 9/10 ########//

	//---------------------------------------//
	//
	// If the aggressive_mode is off, then don't
	// fight back when the unit is moving, only
	// fight back when the unit is already fighting
	// or is idle.
	//
	//---------------------------------------//

	if( !aggressive_mode && cur_action != SPRITE_ATTACK &&
		 cur_action != SPRITE_IDLE )
	{
		return;
	}

	//##### begin trevor 9/10 ########//

	//--------------------------------------------------------------------//
	// decide attack or not
	//--------------------------------------------------------------------//

	int changeToAttack=0;
	if(cur_action==SPRITE_ATTACK || (sprite_info->need_turning && cur_action==SPRITE_TURN &&
		(abs(next_x_loc()-action_x_loc)<attack_range || abs(next_y_loc()-action_y_loc)<attack_range)))
	{
		err_when(cur_x!=next_x || cur_y!=next_y);
		if(action_mode!=ACTION_ATTACK_UNIT)
			changeToAttack++;  //else continue to attack the target unit
		else
		{
			err_when(!action_para);
			if(unit_array.is_deleted(action_para))
				changeToAttack++; // attack new target
		}
	}
	else if(cur_action!=SPRITE_DIE)// && abs(cur_x-next_x)<spriteInfo->speed && abs(cur_y-next_y)<spriteInfo->speed)
	{
		changeToAttack++;
		/*if(!ai_unit) // player unit
		{
			if(action_mode!=ACTION_ATTACK_UNIT)
				changeToAttack++;  //else continue to attack the target unit
			else
			{
				err_when(!action_para);
				if(unit_array.is_deleted(action_para))
					changeToAttack++; // attack new target
			}
		}
		else
			changeToAttack++;*/
	}

	if(!changeToAttack)
	{
		if(ai_unit) // allow ai unit to select target to attack
		{
			//------------------------------------------------------------//
			// conditions to let the unit escape
			//------------------------------------------------------------//
			//-************* codes here ************-//

			//------------------------------------------------------------//
			// select the weaker target to attack first, if more than one
			// unit attack this unit
			//------------------------------------------------------------//
			int attackXLoc = attackUnit->next_x_loc();
			int attackYLoc = attackUnit->next_y_loc();

			int attackDistance = cal_distance(attackXLoc, attackYLoc, attackUnit->sprite_info->loc_width,
														 attackUnit->sprite_info->loc_height);
			if(attackDistance==1) // only consider close attack
			{
				err_when(!action_para || unit_array.is_deleted(action_para));
				Unit *targetUnit = unit_array[action_para];
				if(targetUnit->hit_points > attackUnit->hit_points) // the new attacker is weaker
					attack_unit(attackUnit->sprite_recno);
			}
		}

		return;
	}

	//--------------------------------------------------------------------//
	// cancel AI actions
	//--------------------------------------------------------------------//
	if( ai_action_id )
		nation_array[nation_recno]->action_failure(ai_action_id, sprite_recno);

	if(in_auto_defense_mode())
		set_search_tries(AUTO_DEFENSE_SEARCH_TRIES);

	if(!attackUnit->is_visible())
		return;

	//--------------------------------------------------------------------------------//
	// checking for ship processing trading
	//--------------------------------------------------------------------------------//
	if(sprite_info->sprite_sub_type=='M') //**** BUGHERE, is sprite_sub_type really representing UNIT_MARINE???
	{
		UnitInfo* unitInfo = unit_res[unit_id];
		if(unitInfo->carry_goods_capacity)
		{
			UnitMarine *shipPtr = (UnitMarine*) this;
			if(shipPtr->auto_mode && shipPtr->stop_defined_num>1)
			{
				int targetXLoc = attackUnit->next_x_loc();
				int targetYLoc = attackUnit->next_y_loc();
				SpriteInfo *targetSpriteInfo = attackUnit->sprite_info;
				int attackDistance = cal_distance(targetXLoc, targetYLoc, targetSpriteInfo->loc_width, targetSpriteInfo->loc_height);
				int maxAttackRange = max_attack_range();
				if(maxAttackRange<attackDistance)
					return; // can't attack the target
			}
		}
	}

	switch(action_mode2)
	{
		case ACTION_AUTO_DEFENSE_DETECT_TARGET: case ACTION_AUTO_DEFENSE_BACK_CAMP:
				action_mode2 = ACTION_AUTO_DEFENSE_ATTACK_TARGET;
				break;

		case ACTION_DEFEND_TOWN_DETECT_TARGET:	case ACTION_DEFEND_TOWN_BACK_TOWN:
				action_mode2 = ACTION_DEFEND_TOWN_ATTACK_TARGET;
				break;

		case ACTION_MONSTER_DEFEND_DETECT_TARGET:	case ACTION_MONSTER_DEFEND_BACK_FIRM:
				action_mode2 = ACTION_MONSTER_DEFEND_ATTACK_TARGET;
				break;
	}

	//##### trevor 9/10 #######//

	save_original_action();

	//----------------------------------------------------------//
	// set the original location of the attacking target when
	// the attack() function is called, action_x_loc2 & action_y_loc2
	// will change when the unit move, but these two will not.
	//----------------------------------------------------------//

	original_target_x_loc = attackUnit->next_x_loc();
	original_target_y_loc = attackUnit->next_y_loc();

	//##### trevor 9/10 #######//

	if(!unit_array.is_deleted(attackUnit->sprite_recno))
		attack_unit(attackUnit->sprite_recno);
}
//---------- End of function Unit::unit_auto_guarding ----------//


//----------- Begin of function Unit::hit_building ---------------//
//
// note:	If parentUnit==NULL, the attacking unit is already dead.
//			In range attack, the unit calling this function may not be the
//			attacking unit.
//
// <Unit*> attackUnit   - the attacking unit
// <int>   target?Loc   - the target building location
// <int>   attackDamage - the actual damage made
// <short> attackNationRecno - the nation that ordered the hit
//
// ****************************** Warning ***********************************
// don't use any member variables of this unit. This unit may not be involved
// in the attack event
// **************************************************************************
//
void Unit::hit_building(Unit* attackUnit, int targetXLoc, int targetYLoc, float attackDamage, short attackNationRecno)
{
	Location* locPtr = world.get_loc(targetXLoc, targetYLoc);

	if(locPtr->is_firm())
		hit_firm(attackUnit, targetXLoc, targetYLoc, attackDamage, attackNationRecno);
	else if( locPtr->is_town() )
		hit_town(attackUnit, targetXLoc, targetYLoc, attackDamage, attackNationRecno);
}
//---------- End of function Unit::hit_building ----------//

//##### begin trevor 15/8 #######//


//------------ Begin of function Unit::hit_firm --------------//
//
// note:	If parentUnit==NULL, the attacking unit is already dead.
//			In range attack, the unit calling this function may not be the
//			attacking unit.
//
// <Unit*> attackUnit   - the attacking unit
// <int>   target?Loc   - the target building location
// <int>   attackDamage - the actual damage made
// <short> attackNationRecno - the nation that ordered the hit
//
// ****************************** Warning ***********************************
// don't use any member variables of this unit. This unit may not be involved
// in the attack event
// **************************************************************************
//
void Unit::hit_firm(Unit* attackUnit, int targetXLoc, int targetYLoc, float attackDamage, short attackNationRecno)
{
	Location* locPtr = world.get_loc(targetXLoc, targetYLoc);
	if(!locPtr->is_firm())
		return;	// do nothing if no firm there

	//----------- attack firm ------------//
	err_when(!locPtr->firm_recno());
	Firm *targetFirm = firm_array[locPtr->firm_recno()];
	err_when(!targetFirm);

	//------------------------------------------------------------------------------//
	// change relation to hostile
	// check for NULL to skip unhandled case by bullets
	// check for SPRITE_DIE to skip the case by EXPLOSIVE_CART
	//------------------------------------------------------------------------------//
	if( attackUnit!=NULL && attackUnit->cur_action!=SPRITE_DIE &&
		 targetFirm->nation_recno != attackNationRecno )		// the target and the attacker's nations are different (it's possible that when a unit who has just changed nation has its bullet hitting its own nation)
	{
		if( attackNationRecno && targetFirm->nation_recno )
		{
			//### trevor 29/9 ###//
			nation_array[attackNationRecno]->set_at_war_today();
			nation_array[targetFirm->nation_recno]->set_at_war_today(attackUnit->sprite_recno);
			//### trevor 29/9 ###//
		}

		if( targetFirm->nation_recno )
			nation_array[targetFirm->nation_recno]->being_attacked(attackNationRecno);

		//------------ auto defense -----------------//
		if(attackUnit->is_visible())
			targetFirm->auto_defense(attackUnit->sprite_recno);

		if( attackNationRecno != targetFirm->nation_recno )
			attackUnit->gain_experience(); // gain experience to increase combat level

		targetFirm->being_attacked(attackUnit->sprite_recno);

		//------ increase battling fryhtan score -------//

		if( attackNationRecno && targetFirm->firm_id == FIRM_MONSTER )
			nation_array[attackNationRecno]->kill_monster_score += (float) 0.01;
	}

	//---------- add indicator on the map ----------//

	// ###### begin Gilbert 6/10 #######//
	if( nation_array.player_recno && targetFirm->own_firm() )
		war_point_array.add_point(targetFirm->center_x, targetFirm->center_y);
	// ###### end Gilbert 6/10 #######//

	//---------- damage to the firm ------------//

	targetFirm->hit_points -= attackDamage/3;		// /3 so that it takes longer to destroy a firm

	//######## begin trevor 25/8 ##########//

	if(targetFirm->hit_points <= 0)
	{
		targetFirm->hit_points = (float) 0;

		se_res.sound(targetFirm->center_x, targetFirm->center_y, 1,
			'F', targetFirm->firm_id, "DIE" );

		if( targetFirm->nation_recno == nation_array.player_recno )
			news_array.firm_destroyed(targetFirm->firm_recno, attackUnit, attackNationRecno);

		if( targetFirm->nation_recno )
		{
			if( attackNationRecno )
				nation_array[attackNationRecno]->enemy_firm_destroyed++;

			nation_array[targetFirm->nation_recno]->own_firm_destroyed++;
		}

		else if( targetFirm->firm_id == FIRM_MONSTER )
		{
			news_array.monster_firm_destroyed( ((FirmMonster*)targetFirm)->monster_id, targetFirm->center_x, targetFirm->center_y );
		}

		firm_array.del_firm(targetFirm->firm_recno);
	}

	//######## end trevor 25/8 ##########//
}
//---------- End of function Unit::hit_firm ----------//


//--------- Begin of function Unit::hit_town ---------//
//
// note:	If attackUnit==NULL, the attacking unit is already dead.
//			In range attack, the unit calling this function may not be the
//			attacking unit.
//
// <Unit*> attackUnit   - the attacking unit
// <int>   target?Loc   - the target building location
// <int>   attackDamage - the actual damage made
// <short> attackNationRecno - the nation that ordered the hit
//
// ****************************** Warning ***********************************
// don't use any member variables of this unit. This unit may not be involved
// in the attack event
// **************************************************************************
//
void Unit::hit_town(Unit* attackUnit, int targetXLoc, int targetYLoc, float attackDamage, short attackNationRecno)
{
	Location *locPtr = world.get_loc(targetXLoc, targetYLoc);

	if(!locPtr->is_town())
		return;	// do nothing if no town there

	//----------- attack town ----------//

	err_when(!locPtr->town_recno());

	Town *targetTown = town_array[locPtr->town_recno()];
	int   targetTownRecno = targetTown->town_recno;
	int 	targetTownNameId = targetTown->town_name_id;
	int 	targetTownXLoc = targetTown->center_x;
	int 	targetTownYLoc = targetTown->center_y;

	// ---------- add indicator on the map ----------//
	// ###### begin Gilbert 6/10 #######//
	if( nation_array.player_recno && targetTown->nation_recno == nation_array.player_recno )
		war_point_array.add_point(targetTown->center_x, targetTown->center_y);
	// ###### end Gilbert 6/10 #######//

	//------------------------------------------------------------------------------//
	// change relation to hostile
	// check for NULL to skip unhandled case by bullets
	// check for SPRITE_DIE to skip the case by EXPLOSIVE_CART
	//------------------------------------------------------------------------------//
	if( attackUnit!=NULL && attackUnit->cur_action!=SPRITE_DIE &&
		 targetTown->nation_recno != attackNationRecno )		// the target and the attacker's nations are different (it's possible that when a unit who has just changed nation has its bullet hitting its own nation)
	{
		int townNationRecno = targetTown->nation_recno;

		//------- change to hostile relation -------//

		if( attackNationRecno && targetTown->nation_recno )
		{
			//### trevor 29/9 ###//
			nation_array[attackNationRecno]->set_at_war_today();
			nation_array[targetTown->nation_recno]->set_at_war_today(attackUnit->sprite_recno);
			//### trevor 29/9 ###//
		}

		if( targetTown->nation_recno)
		{
			nation_array[targetTown->nation_recno]->being_attacked(attackNationRecno);
		}

		news_array.disable();		// don't add the town abandon news that might be called by Town::dec_pop() as the town is actually destroyed not abandoned

		targetTown->being_attacked(attackUnit->sprite_recno, attackDamage);

		news_array.enable();

		//------ if the town is destroyed, add a news --------//

		if( town_array.is_deleted(targetTownRecno) &&
			 townNationRecno == nation_array.player_recno )
		{
			news_array.town_destroyed(targetTownNameId, targetTownXLoc, targetTownYLoc, attackUnit, attackNationRecno);
		}

		//---------- gain experience --------//

		if( attackNationRecno != targetTown->nation_recno )
			attackUnit->gain_experience(); // gain experience to increase combat level

		//------------ auto defense -----------------//

		if( !firm_array.is_deleted(targetTownRecno) )
			targetTown->auto_defense(attackUnit->sprite_recno);
	}
}
//---------- End of function Unit::hit_town ----------//

//##### end trevor 15/8 #######//


//--------- Begin of function Unit::hit_wall -----------//
//
// <Unit*> attackUnit   - the attacking unit
// <int>   target?Loc   - the targeted wall location
// <int>   attackDamage - the actual damage made
// <short> attackNationRecno - the nation that ordered the hit
//
// ****************************** Warning ***********************************
// don't use any member variables of this unit. This unit may not be involved
// in the attack event
// **************************************************************************
//
void Unit::hit_wall(Unit* attackUnit, int targetXLoc, int targetYLoc, float attackDamage, short attackNationRecno)
{
	Location *locPtr = world.get_loc(targetXLoc, targetYLoc);
	err_when(!locPtr->is_wall());

	//######## begin trevor 25/6 #########//
/*
	if(attackUnit!=NULL)
		attackUnit->change_relation(attackNationRecno, locPtr->wall_nation_recno(), NATION_HOSTILE);
*/
	//######## end trevor 25/6 #########//

	if( !locPtr->attack_wall((int)attackDamage) )
		world.correct_wall(targetXLoc, targetYLoc);
}
//---------- End of function Unit::hit_wall ----------//


//--------- Begin of function Unit::cal_distance ---------//
// calculate the distance from this unit to the target
// (assume the size of this unit is a square)
//
// <int>	targetXLoc		- x location of the target
// <int>	targetYLoc		- y location of the target
// <int> targetWidth		- target width
// <int> targetHeight	- target height
//
int Unit::cal_distance(int targetXLoc, int targetYLoc, int targetWidth, int targetHeight)
{
	int curXLoc = next_x_loc();
	int curYLoc = next_y_loc();
	int dispX=0, dispY=0;

	if(curXLoc<targetXLoc)
		dispX = (targetXLoc - curXLoc - sprite_info->loc_width) + 1;
	else if((dispX=curXLoc-targetXLoc-targetWidth+1)<0)
		dispX  = 0;
	err_when(dispX<0 || dispX>MAX_WORLD_X_LOC);

	if(curYLoc<targetYLoc)
		dispY = (targetYLoc - curYLoc - sprite_info->loc_height) + 1;
	else if((dispY=curYLoc-targetYLoc-targetHeight+1)<0)
	{
		err_when(mobile_type!=UNIT_AIR && !dispX); // inside the target
		return dispX;
	}
	err_when(dispY<0 || dispY>MAX_WORLD_Y_LOC);

	return (dispX>=dispY)? dispX : dispY;
}
//----------- End of function Unit::cal_distance -----------//


//------------ Begin of function Unit::actual_damage --------------//
//
// return: return the actual hit damage this unit can do to a target.
//
float Unit::actual_damage()
{
	AttackInfo *attackInfo = attack_info_array+cur_attack;

	int attackDamage = attackInfo->attack_damage;

	//-------- pierce damage --------//

	attackDamage += misc.random(3) + attackInfo->pierce_damage
						 * misc.random(skill.combat_level-attackInfo->combat_level)
						 / (100-attackInfo->combat_level);

	//--- if this unit is led by a general, its attacking ability is influenced by the general ---//
	//
	// The unit's attacking ability is increased by a percentage equivalent to
	// the leader unit's leadership.
	//
	//------------------------------------------------------------------------//

	if( is_leader_in_range() )
	{
		Unit *leaderUnit = unit_array[leader_unit_recno];
		attackDamage += attackDamage * leaderUnit->skill.skill_level / 100;
	}

	return (float) attackDamage / ATTACK_SLOW_DOWN;		// lessen all attacking damages, thus slowing down all battles.
}
//------------ End of function Unit::actual_damage --------------//


//--------- Begin of function Unit::gain_experience ---------//
void Unit::gain_experience()
{
	if(unit_res[unit_id]->unit_class != UNIT_CLASS_HUMAN)
		return; // no experience gain if unit is not human

	//---- increase the unit's contribution to the nation ----//

	if( nation_contribution < MAX_NATION_CONTRIBUTION )
	{
		nation_contribution++;

		err_when( nation_contribution < 0 );		// overflow
	}

	//------ increase combat skill -------//

	err_when(skill.combat_level<0 || skill.combat_level>100);

	inc_minor_combat_level(6);

	//--- if this is a soldier led by a commander, increase the leadership of its commander -----//

	if( is_leader_in_range() )
	{
		Unit* leaderUnit = unit_array[leader_unit_recno];

		leaderUnit->inc_minor_skill_level(1);

		//-- give additional increase if the leader has skill potential on leadership --//

		if( leaderUnit->skill.skill_potential > 0 )
		{
			if( misc.random(10-leaderUnit->skill.skill_potential/10)==0 )
				leaderUnit->inc_minor_skill_level(5);
		}

		//--- if this soldier has leadership potential and is led by a commander ---//
		//--- he learns leadership by watching how the commander commands the troop --//

		if( skill.skill_potential > 0 )
		{
			if( misc.random(10-skill.skill_potential/10)==0 )
				inc_minor_skill_level(5);
		}
	}
}
//------------ End of function Unit::gain_experience --------------//


//--------- Begin of function Unit::can_attack ---------//
// return 1 if can attack
// return 0 otherwise
//
/*int Unit::can_attack()
{
	return (can_attack_flag && attack_count);
}*/
//------------ End of function Unit::can_attack --------------//


//--------- Begin of function Unit::nation_can_attack ---------//
// return 1 for able to attack this nation i.e. relation is not
//				friendly or alliance
// return 0 otherwise
//
// Whether this unit can attack others with the specified nation recno
//
// <short> nationRecno	-	nation recno to be checked
//
int Unit::nation_can_attack(short nationRecno)
{
	if(!ai_unit)
	{
		//return 1;
		if( game.game_mode == GAME_TEST )	// in testing games, player units can attack their own units
			return 1;
		else
			return nationRecno!=nation_recno; // able to attack all nation except our own nation
	}
	else if(nation_recno == nationRecno)
		return 0; // ai unit don't attack its own nation, except special order

	if(!nation_recno || !nationRecno)
		return 1; // true if either nation is independent

	Nation *nationPtr = nation_array[nation_recno];

	char relatinStatus = nationPtr->get_relation_status(nationRecno);
	if(relatinStatus==NATION_FRIENDLY || relatinStatus==NATION_ALLIANCE)
		return 0;

	return 1;
}
//------------ End of function Unit::nation_can_attack --------------//


//--------- Begin of function Unit::independent_nation_can_attack ---------//
// Note: this unit should be an independent unit
//
// Should this independent unit attack other units with specified nation recno
//
// return 1 if decision is attacking
// return 0 otherwise
//
// <short> nationRecno	-	nation recno to be checked
//
int Unit::independent_nation_can_attack(short nationRecno)
{
	err_when(nation_recno);

	Town *townPtr;
	FirmMonster *firmMonster;
	Rebel *rebelPtr;

	switch(unit_mode)
	{
		case UNIT_MODE_DEFEND_TOWN:
				err_when(unit_mode_para<=0);
				if(town_array.is_deleted(unit_mode_para))
					return 0; // don't attack independent unit with no town

				townPtr = town_array[unit_mode_para];

				if( !townPtr->is_hostile_nation(nationRecno) )
					return 0; // false if the indepentent unit don't want to attack us

				break;

		case UNIT_MODE_REBEL:
				if(rebel_array.is_deleted(unit_mode_para))
					return 0;

				rebelPtr = rebel_array[unit_mode_para];

				if( !rebelPtr->is_hostile_nation(nationRecno) )
					return 0;

				break;

//######## begin trevor 22/8 ##########//
		case UNIT_MODE_MONSTER:
				if( unit_mode_para==0 )
					return nationRecno; // attack anything that is not independent

				firmMonster = (FirmMonster*) firm_array[unit_mode_para];

				err_when( firmMonster->firm_id != FIRM_MONSTER );

				if( !firmMonster->is_hostile_nation(nationRecno) )
					return 0; 			// false if the indepentent unit don't want to attack us

				break;
//######## end trevor 22/8 ##########//

		default:
				return 0;
	}

	return 1;
}
//------------ End of function Unit::independent_nation_can_attack --------------//


//--------- Begin of function Unit::choose_best_attack_mode ---------//
// if the unit has more than one attack mode, select the suitable mode
// to attack the target
//
// <int>  attackDistance	- the distance from the target
// [char] targetMobileType	- the target's mobile_type (default: UNIT_LAND)
//
void Unit::choose_best_attack_mode(int attackDistance, char targetMobileType)
{
	//------------ enable/disable range attack -----------//
	//cur_attack = 0;
	//return;

	//-------------------- define parameters -----------------------//
	uint8_t attackModeBeingUsed = cur_attack;
	err_when(attackModeBeingUsed<0 || attackModeBeingUsed>MAX_UNIT_ATTACK_TYPE);
	//UCHAR maxAttackRangeMode = 0;
	uint8_t maxAttackRangeMode = cur_attack;
	AttackInfo* attackInfoMaxRange = attack_info_array;
	AttackInfo* attackInfoChecking;
	AttackInfo* attackInfoSelected = attack_info_array+cur_attack;

	//--------------------------------------------------------------//
	// If targetMobileType==UNIT_AIR or mobile_type==UNIT_AIR,
	//	force to use range_attack.
	// If there is no range_attack, return 0, i.e. cur_attack=0
	//--------------------------------------------------------------//
	if(attack_count>1)
	{
		int canAttack = 0;
		int checkingDamageWeight, selectedDamageWeight;

		for(uint8_t i=0; i<attack_count; i++)
		{
			if(attackModeBeingUsed==i)
				continue; // it is the mode already used

			attackInfoChecking = attack_info_array+i;
			if(can_attack_with(attackInfoChecking) && attackInfoChecking->attack_range>=attackDistance)
			{
				//-------------------- able to attack ----------------------//
				canAttack = 1;

				if(attackInfoSelected->attack_range<attackDistance)
				{
					attackModeBeingUsed = i;
					attackInfoSelected = attackInfoChecking;
					continue;
				}
				
				checkingDamageWeight = attackInfoChecking->attack_damage;
				selectedDamageWeight = attackInfoSelected->attack_damage;

				if(attackDistance==1 && (targetMobileType!=UNIT_AIR && mobile_type!=UNIT_AIR))
				{
					//------------ force to use close attack if possible -----------//
					if(attackInfoSelected->attack_range==attackDistance)
					{
						if(attackInfoChecking->attack_range==attackDistance && checkingDamageWeight>selectedDamageWeight)
						{
							attackModeBeingUsed = i; // choose the one with strongest damage
							attackInfoSelected = attackInfoChecking;
						}
						continue;
					}
					else if(attackInfoChecking->attack_range==1)
					{
						attackModeBeingUsed = i;
						attackInfoSelected = attackInfoChecking;
						continue;
					}
				}
				
				//----------------------------------------------------------------------//
				// further selection
				//----------------------------------------------------------------------//
				if(checkingDamageWeight == selectedDamageWeight)
				{
					if(attackInfoChecking->attack_range<attackInfoSelected->attack_range)
					{
						if(attackInfoChecking->attack_range>1 || (targetMobileType!=UNIT_AIR && mobile_type!=UNIT_AIR))
						{
							//--------------------------------------------------------------------------//
							// select one with shortest attack_range
							//--------------------------------------------------------------------------//
							attackModeBeingUsed = i;
							attackInfoSelected = attackInfoChecking;
						}
					}
				}
				else
				{
					//--------------------------------------------------------------------------//
					// select one that can do the attacking immediately with the strongest damage point
					//--------------------------------------------------------------------------//
					attackModeBeingUsed = i;
					attackInfoSelected = attackInfoChecking;
				}
			}
			
			if(!canAttack)
			{
				//------------------------------------------------------------------------------//
				// if unable to attack the target, choose the mode with longer attack_range and
				// heavier damage
				//------------------------------------------------------------------------------//
				if(can_attack_with(attackInfoChecking) &&
					(attackInfoChecking->attack_range>attackInfoMaxRange->attack_range ||
					(attackInfoChecking->attack_range==attackInfoMaxRange->attack_range &&
					 attackInfoChecking->attack_damage>attackInfoMaxRange->attack_damage)))
				{
					maxAttackRangeMode = i;
					attackInfoMaxRange = attackInfoChecking;
				}
			}
		}

		if(canAttack)
			cur_attack = attackModeBeingUsed;	// choose the strongest damage mode if able to attack
		else
			cur_attack = maxAttackRangeMode;		//	choose the longest attack range if unable to attack

		attack_range = attack_info_array[cur_attack].attack_range;
		err_when(final_dir<0 || final_dir>=MAX_SPRITE_DIR_TYPE);
		err_when(cur_attack>=attack_count || cur_attack<0);
	}
	else
	{
		cur_attack = 0;	// only one mode is supported
		attack_range = attack_info_array[0].attack_range;
		return;
	}
}
//---------- End of function Unit::choose_best_attack_mode ----------//


//------ Begin of function Unit::set_attack_dir ---------//
// set direction for attacking
//
//	<short>	curX		-	x location of the unit
// <short>	curY		-	y location of the unit
// <short>	targetX	-	x location of the target
// <short>	targetY	-	y location of the target
//
void Unit::set_attack_dir(short curX, short curY, short targetX, short targetY)
{
	int targetDir = get_dir(curX, curY, targetX, targetY);
	if(unit_res[unit_id]->unit_class==UNIT_CLASS_SHIP)
	{
		int attackDir1, attackDir2;

		attackDir1 = (targetDir+2)%MAX_SPRITE_DIR_TYPE;
		attackDir2 = (targetDir+6)%MAX_SPRITE_DIR_TYPE;

		if((attackDir1+8-final_dir)%MAX_SPRITE_DIR_TYPE <= (attackDir2+8-final_dir)%MAX_SPRITE_DIR_TYPE)
			final_dir = attackDir1;
		else
			final_dir = attackDir2;

		attack_dir = targetDir;
		err_when(attack_dir<0 || attack_dir>=MAX_SPRITE_DIR_TYPE);
	}
	else
	{
		attack_dir = targetDir;
		set_dir(targetDir);
	}
}
//---------- End of function Unit::set_attack_dir ----------//


//--------- Begin of function Unit::set_unreachable_location ---------//
// used to set the bit in the unreachable_flag (16 bits)
//
//	The 16 bits of the flag are used to represent the 16 location of a unit
// as follows:
//
//	1	2	3	4			where x is the upper left corner of the unit.
//	5	x	6	7			For 1x1 unit, 1,2,3,5,6,8,9,10 are meaningful
//	8	9	10	11			For 2x2 unit, 1,2,3,4,5,7,8,11,12,13,14,15 are meaningful
//	12	13	14	15
//
void Unit::set_unreachable_location(int xLoc, int yLoc)
{
	static unsigned short bitFlag[16] = {0x0001, 0x0002, 0x0004, 0x0008, 0x0010, 0x0020, 0x0040, 0x0080,
	 												 0x0100, 0x0200, 0x0400, 0x0800, 0x1000, 0x2000, 0x4000, 0x8000};
	/*int curXLoc = next_x_loc();
	int curYLoc = next_y_loc();
	int xDist = xLoc-curXLoc+1;
	int yDist = yLoc-curYLoc+1;

	err_when(xDist<0 || xDist>3 || yDist<0 || yDist>3);
	char bitNo = yDist*4 + xDist;

	err_when(bitNo==5);
	if(bitNo>5)
		bitNo--;

	unreachable_flag |= bitFlag[bitNo];*/
}
//----------- End of function Unit::set_unreachable_location -----------//

//--------- Begin of function Unit::check_self_surround ---------//
// Note : mobile_type used is this unit's mobile_type
//
void Unit::check_self_surround()
{
	/*err_when(sprite_info->loc_height!=sprite_info->loc_width);
	err_when(sprite_info->loc_width<1 || sprite_info->loc_width>2);
	
	Location *locPtr;
	int width = sprite_info->loc_width;
	int curXLoc = next_x_loc();
	int curYLoc = next_y_loc();
	int startCount = (width==1) ? 2 : 5;
	int endCount = (width==1) ? 9 : 16;
	int xShift, yShift;

	self_surround_flag = 0;

	for(int i=startCount; i<=endCount; i++)
	{
		misc.cal_move_around_a_point(i, width, width, xShift, yShift);
		locPtr = world.get_loc(curXLoc+xShift, curYLoc+yShift);
		
		if(!locPtr->can_move(mobile_type))
			misc.set_surround_bit(self_surround_flag, i-startCount);
	}*/
}
//----------- End of function Unit::check_self_surround -----------//


//----------- Begin of function Unit::cycle_eqv_attack -----------//
void Unit::cycle_eqv_attack()
{
	int trial = MAX_UNIT_ATTACK_TYPE+2;
	if( attack_info_array[cur_attack].eqv_attack_next > 0)
	{
		do
		{
			cur_attack = attack_info_array[cur_attack].eqv_attack_next-1;
			err_when(--trial == 0);
		} while( !can_attack_with(cur_attack) );
	}
	else
	{
		if( !can_attack_with(cur_attack) )
		{
			err_here();
			// force to search again
			char attackRange = char(attack_info_array[cur_attack].attack_range);
			err_when(attackRange != attack_range);			// redundant check
			AttackInfo *attackInfo = attack_info_array;
			for(int i = 0; i < attack_count; ++i, ++attackInfo)
			{
				if( attackInfo->attack_range >= attackRange && 
					can_attack_with(attackInfo))
				{
					cur_attack = i;
					break;
				}
				err_when(i >= attack_count);		// not found
			}
		}
	}
	err_when(cur_attack < 0 || cur_attack >= attack_count);
}
//----------- End of function Unit::cycle_eqv_attack -----------//


//----------- Begin of function Unit::max_attack_range -----------//
// return the maximum attack range the unit can make
// 
//
//
int Unit::max_attack_range()
{
	int maxRange=0;

	AttackInfo *attackInfo = attack_info_array;
	for(int i=0; i<attack_count; i++, attackInfo++)
	{
		if(can_attack_with(attackInfo) &&
			attackInfo->attack_range>maxRange)
			maxRange = attackInfo->attack_range;
	}
	
	return maxRange;
}
//----------- End of function Unit::max_attack_range -----------//


//----------- Begin of function Unit::can_attack_with -------//
int Unit::can_attack_with(int i)
{
	err_when( i< 0 || i >= attack_count);
	AttackInfo *attackInfo = attack_info_array+i;
	return( skill.combat_level >= attackInfo->combat_level &&
		cur_power >= attackInfo->min_power);
	
}


int Unit::can_attack_with(AttackInfo *attackInfo)
{
	return( skill.combat_level >= attackInfo->combat_level &&
		cur_power >= attackInfo->min_power);
}
//----------- End of function Unit::can_attack_with -------//


//----------- Begin of function Unit::get_hit_x_y -------//
void Unit::get_hit_x_y(short *xPtr, short *yPtr)
{
	switch(cur_dir)
	{
	case 0:	// north
		*xPtr = cur_x;
		*yPtr = cur_y - ZOOM_LOC_HEIGHT;
		break;
	case 1:	// north east
		*xPtr = cur_x + ZOOM_LOC_WIDTH;
		*yPtr = cur_y - ZOOM_LOC_HEIGHT;
		break;
	case 2:	// east
		*xPtr = cur_x + ZOOM_LOC_WIDTH;
		*yPtr = cur_y;
		break;
	case 3:	// south east
		*xPtr = cur_x + ZOOM_LOC_WIDTH;
		*yPtr = cur_y + ZOOM_LOC_HEIGHT;
		break;
	case 4:	// south
		*xPtr = cur_x;
		*yPtr = cur_y + ZOOM_LOC_HEIGHT;
		break;
	case 5:	// south west
		*xPtr = cur_x - ZOOM_LOC_WIDTH;
		*yPtr = cur_y + ZOOM_LOC_HEIGHT;
		break;
	case 6:	// west
		*xPtr = cur_x - ZOOM_LOC_WIDTH;
		*yPtr = cur_y;
		break;
	case 7:	// north west
		*xPtr = cur_x - ZOOM_LOC_WIDTH;
		*yPtr = cur_y - ZOOM_LOC_HEIGHT;
		break;
	default:
		err_here();
		*xPtr = cur_x;
		*yPtr = cur_y;
	}
}
//----------- End of function Unit::get_hit_x_y -------//


//----------- Begin of function Unit::add_close_attack_effect -------//
void Unit::add_close_attack_effect()
{
	short effectId = (attack_info_array+cur_attack)->effect_id;
	if( effectId )
	{
		short x,y;
		get_hit_x_y(&x, &y);
		Effect::create(effectId, x, y, SPRITE_IDLE, cur_dir, mobile_type == UNIT_AIR ? 8 : 2, 0);
	}
}
//----------- End of function Unit::add_close_attack_effect -------//


//----------- Begin of function Unit::is_action_attack -------//
// check whether the unit carrys on attacking action
//
int Unit::is_action_attack()
{
	switch(action_mode2)
	{
		case ACTION_ATTACK_UNIT:
		case ACTION_ATTACK_FIRM:
		case ACTION_ATTACK_TOWN:
		case ACTION_ATTACK_WALL:
		case ACTION_AUTO_DEFENSE_ATTACK_TARGET:
		case ACTION_AUTO_DEFENSE_DETECT_TARGET:
		case ACTION_AUTO_DEFENSE_BACK_CAMP:
		case ACTION_DEFEND_TOWN_ATTACK_TARGET:
		case ACTION_DEFEND_TOWN_DETECT_TARGET:
		case ACTION_DEFEND_TOWN_BACK_TOWN:
		case ACTION_MONSTER_DEFEND_ATTACK_TARGET:
		case ACTION_MONSTER_DEFEND_DETECT_TARGET:
		case ACTION_MONSTER_DEFEND_BACK_FIRM:
				return 1;
	
		default: return 0;
	}

	return 0;
}
//----------- End of function Unit::is_action_attack -------//

