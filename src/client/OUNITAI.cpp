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

//Filename    : OUNITAI.CPP
//Description : Object Unit AI

#include <OSYS.h>
#include <OSPY.h>
#include <OREBEL.h>
#include <OUNIT.h>
#include <OCONFIG.h>
#include <OREGIONS.h>
#include <OF_CAMP.h>
#include <ONATION.h>

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

//--------- Begin of function Unit::process_ai --------//
//
// [int] forceExecute - whether force execute all AI functions
//								without checking day interavals.
//							   (default: 0)
//
void Unit::process_ai()
{
	err_when( !nation_recno );

	//-*********** simulate aat ************-//
	#ifdef DEBUG
		if(debug_sim_game_type)
			return;
	#endif
	//-*********** simulate aat ************-//

	//------ the aggressive_mode of AI units is always 1 ------//

	aggressive_mode = 1;

	//------- handle Seek Path failures ------//

	if( ai_handle_seek_path_fail() )
		return;

	//--- if it's a spy from other nation, don't control it ---//

	if( spy_recno && true_nation_recno() != nation_recno )
	{
		//--- a random chance of the AI catching the spy and resign it ---//

		if( is_visible() && m.random(365 * FRAMES_PER_DAY)==0 )		// if the unit stay outside for one year, it will get caught
		{
			stop2();
			resign(COMMAND_AI);
			return;
		}

		if( !spy_array[spy_recno]->notify_cloaked_nation_flag )		// if notify_cloaked_nation_flag is 1, the nation will take it as its own spies
			return;
	}

	//----- think about rewarding this unit -----//

	if( race_id && rank_id != RANK_KING &&
		 info.game_date%5 == sprite_recno%5 )
	{
		think_reward();
	}

	//-----------------------------------------//

	if( !is_visible() )
		return;

	//--- if the unit has stopped, but ai_action_id hasn't been reset ---//

	if( cur_action==SPRITE_IDLE && action_mode==ACTION_STOP &&
		 action_mode2==ACTION_STOP && ai_action_id )
	{
		nation_array[nation_recno]->action_failure(ai_action_id, sprite_recno);

		err_when( ai_action_id );		// it should have been reset
	}

	//---- King flees under attack or surrounded by enemy ---//

	if( race_id && rank_id==RANK_KING )
	{
		if( think_king_flee() )
			return;
	}

	//---- General flees under attack or surrounded by enemy ---//

	if( race_id && rank_id==RANK_GENERAL &&
		 info.game_date%7 == sprite_recno%7 )
	{
		if( think_general_flee() )
			return;
	}

	//-- let Unit::next_day() process it process original_action_mode --//

	if( original_action_mode )
		return;

	//------ if the unit is not stop right now ------//

	if( !is_ai_all_stop() )
	{
		think_stop_chase();
		return;
	}

	//-----------------------------------------//

	if( mobile_type==UNIT_LAND )
	{
		if( ai_escape_fire() )
			return;
	}

	//---------- if this is your spy --------//

	if( spy_recno && true_nation_recno()==nation_recno )
		think_spy_action();

	//------ if this unit is from a camp --------//

	if( home_camp_firm_recno )
	{
		Firm* firmCamp = firm_array[home_camp_firm_recno];
		int   rc;

		if( rank_id == RANK_SOLDIER )
			rc = firmCamp->worker_count < MAX_WORKER;
		else
			rc = !firmCamp->overseer_recno;

		if( rc )
		{
			if( return_camp() )
				return;
		}

		home_camp_firm_recno = 0;		// the camp is already occupied by somebody
	}

	//----------------------------------------//

	if( race_id && rank_id==RANK_KING )
	{
		think_king_action();
	}
	else if( race_id && rank_id==RANK_GENERAL )
	{
		think_general_action();
	}
	else
	{
		if( unit_res[unit_id]->unit_class == UNIT_CLASS_WEAPON )
		{
			if( info.game_date%15 == sprite_recno%15 )	// don't call too often as the action may fail and it takes a while to call the function each time
			{
				think_weapon_action();	//-- ships AI are called in UnitMarine --//
			}
		}
		else if( race_id )
		{
			//--- if previous attempts for new action failed, don't call think_normal_human_action() so frequently then ---//

			if( ai_no_suitable_action )
			{
				if( info.game_date%15 != sprite_recno%15 )	// don't call too often as the action may fail and it takes a while to call the function each time
					return;
			}

			//---------------------------------//

			if( !think_normal_human_action() )
			{
				ai_no_suitable_action = 1; 	// set this flag so think_normal_human_action() won't be called continously

				if( !leader_unit_recno )		// only when the unit is not led by a commander
				{
					resign(COMMAND_AI);
				}
				else
				{
					ai_move_to_nearby_town();
				}
			}
		}
	}
}
//---------- End of function Unit::process_ai --------//


//--------- Begin of function Unit::think_stop_chase --------//

int Unit::think_stop_chase()
{
	//-----------------------------------------------------//
	//
	// Stop the chase if the target is being far away from
	// its original attacking location.
	//
	//-----------------------------------------------------//

	if( !(action_mode==ACTION_ATTACK_UNIT && ai_original_target_x_loc>=0) )
		return 0;

	if( unit_array.is_deleted(action_para) )
	{
		stop2();
		return 1;
	}

	Unit* targetUnit = unit_array[action_para];

	if( !targetUnit->is_visible() )
	{
		stop2();
		return 1;
	}

	//----------------------------------------//

	int aiChaseDistance = 10 + nation_array[nation_recno]->pref_military_courage/20;		// chase distance: 10 to 15

	int curDistance = m.points_distance( targetUnit->next_x_loc(), targetUnit->next_y_loc(),
							ai_original_target_x_loc, ai_original_target_y_loc );

	if( curDistance <= aiChaseDistance )
		return 0;

	//--------- stop the unit ----------------//

	stop2();

	//--- if this unit leads a troop, stop the action of all troop members as well ---//

	int leaderUnitRecno;

	if( leader_unit_recno )
		leaderUnitRecno = leader_unit_recno;
	else
		leaderUnitRecno = sprite_recno;

	TeamInfo* teamInfo = unit_array[leaderUnitRecno]->team_info;

	if( teamInfo )
	{
		for( int i=teamInfo->member_count-1 ; i>=0 ; i-- )
		{
			int unitRecno = teamInfo->member_unit_array[i];

			if( unit_array.is_deleted(unitRecno) )
				continue;

			unit_array[unitRecno]->stop2();
		}
	}

	return 1;
}
//---------- End of function Unit::think_stop_chase --------//


//--------- Begin of function Unit::is_ai_all_stop --------//

int Unit::is_ai_all_stop()
{
	return cur_action==SPRITE_IDLE && action_mode==ACTION_STOP &&
			 action_mode2==ACTION_STOP && !ai_action_id;
}
//---------- End of function Unit::is_ai_all_stop --------//


//--------- Begin of function Unit::ai_move_to_nearby_town --------//

void Unit::ai_move_to_nearby_town()
{
	//---- look for towns to assign to -----//

	Nation* ownNation = nation_array[nation_recno];
	Town *townPtr, *bestTown=NULL;
	int  regionId = world.get_region_id( next_x_loc(), next_y_loc() );
	int  curDistance, curRating, bestRating=0;
	int  curXLoc = next_x_loc(), curYLoc = next_y_loc();

	for( int i=town_array.size() ; i>0 ; i-- )		// can't use ai_town_array[] because this function will be called by Unit::betray() when a unit defected to the player's kingdom
	{
		if( town_array.is_deleted(i) )
			continue;

		townPtr = town_array[i];

		if( townPtr->nation_recno != nation_recno )
			continue;

		if( townPtr->region_id != regionId )
			continue;

		//-------------------------------------//

		curDistance = m.points_distance(curXLoc, curYLoc, townPtr->center_x, townPtr->center_y );

		if( curDistance < 10 )		// no need to move if the unit is already close enough to the town.
			return;

		curRating = 100 - 100 * curDistance / MAX_WORLD_X_LOC;

		curRating += townPtr->population;

		//-------------------------------------//

		if( curRating > bestRating )
		{
			bestRating = curRating;
			bestTown   = townPtr;
		}
	}

	if( bestTown )
		move_to_town_surround(bestTown->loc_x1, bestTown->loc_y1, sprite_info->loc_width, sprite_info->loc_height);
}
//---------- End of function Unit::ai_move_to_nearby_town --------//


//--------- Begin of function Unit::ai_escape_fire --------//
//
// Move away if the unit currently stands on a burning ground.
//
int Unit::ai_escape_fire()
{
	if(cur_action!=SPRITE_IDLE)
		return 0;

	if(mobile_type!=UNIT_LAND)
		return 0;

	Location *locPtr = world.get_loc(next_x_loc(), next_y_loc());

	if( !locPtr->fire_str() )
		return 0;

	//--------------------------------------------//

	int checkLimit = 400; // checking for 400 location
	int xShift, yShift, checkXLoc, checkYLoc;
	int curXLoc = next_x_loc();
	int curYLoc = next_y_loc();

	for(int i=2; i<checkLimit; i++)
	{
		m.cal_move_around_a_point(i, 20, 20, xShift, yShift);

		checkXLoc = curXLoc + xShift;
		checkYLoc = curYLoc + yShift;

		if(checkXLoc<0 || checkXLoc>=MAX_WORLD_X_LOC || checkYLoc<0 || checkYLoc>=MAX_WORLD_Y_LOC)
			continue;

		if(!locPtr->can_move(mobile_type))
			continue;

		locPtr = world.get_loc(checkXLoc, checkYLoc);

		if( locPtr->fire_str()==0 )		// move to a safe place now
		{
			move_to(checkXLoc, checkYLoc);
			return 1;
		}
	}

	return 0;
}
//---------- End of function Unit::ai_escape_fire --------//


//--------- Begin of function Unit::think_spy_action --------//

void Unit::think_spy_action()
{
	ai_move_to_nearby_town();		// just move it to one of our towns
}
//---------- End of function Unit::think_spy_action --------//


//--------- Begin of function Unit::think_king_action --------//

int Unit::think_king_action()
{
	return think_leader_action();
}
//---------- End of function Unit::think_king_action --------//


//--------- Begin of function Unit::think_general_action --------//

int Unit::think_general_action()
{
	if( think_leader_action() )
		return 1;

	//--- if the general is not assigned to a camp due to its low competency ----//

	Nation* ownNation = nation_array[nation_recno];
	int 	  rc = 0;

	if( team_info->member_count <= 1 )
	{
		rc = 1;
	}

	//--- if the skill of the general and the number of soldiers he commands is not large enough to justify building a new camp ---//

	else if( skill.skill_level + team_info->member_count*4
				< 40 + ownNation->pref_keep_general/5 )					// 40 to 60
	{
		rc = 1;
	}

	//-- think about splitting the team and assign them into other forts --//

	else if( ownNation->ai_has_too_many_camp() )
	{
		rc = 1;
	}

	//--------- demote the general to soldier and disband the troop -------//

	if( rc )
	{
		set_rank(RANK_SOLDIER);
		return think_normal_human_action();
	}

	return 0;
}
//---------- End of function Unit::think_general_action --------//


//--------- Begin of function Unit::think_leader_action --------//
//
// Think about the action of a leader (either a general or a king).
//
int Unit::think_leader_action()
{
	Nation*   nationPtr = nation_array[nation_recno];
	FirmCamp *firmCamp, *bestCamp=NULL;
	int		 curRating, bestRating=10, delActionRecno=0;
	int		 curXLoc = next_x_loc(), curYLoc = next_y_loc();
	int		 curRegionId = world.get_region_id( curXLoc, curYLoc );

	if( rank_id == RANK_KING )		// if this unit is the king, always assign it to a camp regardless of whether the king is a better commander than the existing one
		bestRating = -1000;

	err_when( skill.skill_id != SKILL_LEADING );

	//----- think about which camp to move to -----//

	for( int i=nationPtr->ai_camp_count-1 ; i>=0 ; i-- )
	{
		firmCamp = (FirmCamp*) firm_array[ nationPtr->ai_camp_array[i] ];

		if( firmCamp->region_id != curRegionId )
			continue;

		//--- if the commander of this camp is the king, never replace him ---//

		if( firmCamp->overseer_recno == nationPtr->king_unit_recno )
			continue;

		//-------------------------------------//

		int curLeadership = firmCamp->cur_commander_leadership();
		int newLeadership = firmCamp->new_commander_leadership(race_id, skill.skill_level);

		curRating = newLeadership - curLeadership;

		//-------------------------------------//

		if( curRating > bestRating )
		{
			//--- if there is already somebody being assigned to it ---//

			int actionRecno=0;

			if( rank_id != RANK_KING )			// don't check this if the current unit is the king
			{
				actionRecno = nationPtr->is_action_exist(firmCamp->loc_x1, firmCamp->loc_y1,
								  -1, -1, ACTION_AI_ASSIGN_OVERSEER, FIRM_CAMP);

				if( actionRecno && nationPtr->get_action(actionRecno)->processing_instance_count )
					continue;
			}

			bestRating = curRating;
			bestCamp   = firmCamp;
			delActionRecno = actionRecno;
		}
	}

	if( !bestCamp )
		return 0;

	//----- delete an unprocessed queued action if there is any ----//

	if( delActionRecno )
		nationPtr->del_action(delActionRecno);

	//--------- move to the camp now ---------//

	//-- if there is room in the camp to host all soldiers led by this general --//

	if( team_info->member_count-1 <= MAX_WORKER-bestCamp->worker_count )
	{
		validate_team();

		unit_array.assign( bestCamp->loc_x1, bestCamp->loc_y1, 0, COMMAND_AI,
								 team_info->member_unit_array, team_info->member_count );
		return 1;
	}
	else  //--- otherwise assign the general only ---//
	{
		return nationPtr->add_action(bestCamp->loc_x1, bestCamp->loc_y1, -1, -1, ACTION_AI_ASSIGN_OVERSEER, FIRM_CAMP, 1, sprite_recno);
	}

	return 0;
}
//---------- End of function Unit::think_leader_action --------//


//--------- Begin of function Unit::think_normal_human_action --------//

int Unit::think_normal_human_action()
{
	if( home_camp_firm_recno )
		return 0;

	if( leader_unit_recno &&
		 unit_array[leader_unit_recno]->is_visible() )		// if the unit is led by a commander, let the commander makes the decision. If the leader has been assigned to a firm, don't consider it as a leader anymore
	{
		return 0;
	}

	err_when( !race_id );
	err_when( !nation_recno );

	//---- think about assign the unit to a firm that needs workers ----//

	Nation* ownNation = nation_array[nation_recno];
	Firm *firmPtr, *bestFirm=NULL;
	int  regionId = world.get_region_id( next_x_loc(), next_y_loc() );
	int  skillId  = skill.skill_id;
	int  skillLevel = skill.skill_level;
	int  i, curRating, bestRating=0;
	int  curXLoc = next_x_loc(), curYLoc = next_y_loc();

	if( skill.skill_id )
	{
		for( i=firm_array.size() ; i>0 ; i-- )
		{
			if( firm_array.is_deleted(i) )
				continue;

			firmPtr = firm_array[i];

			if( firmPtr->nation_recno != nation_recno )
				continue;

			if( firmPtr->region_id != regionId )
				continue;

			curRating = 0;

			if( skill.skill_id == SKILL_CONSTRUCTION )		// if this is a construction worker
			{
				if( firmPtr->builder_recno )			// assign the construction worker to this firm as an residental builder
					continue;
			}
			else
			{
				if( !firmPtr->worker_array ||
					 firmPtr->firm_skill_id != skillId )
				{
					continue;
				}

				//----- if the firm is full of worker ------//

				if( firmPtr->is_worker_full() )
				{
					//---- get the lowest skill worker of the firm -----//

					Worker* workerPtr = firmPtr->worker_array;
					int	  minSkill=100;

					for( int j=0 ; j<firmPtr->worker_count ; j++, workerPtr++ )
					{
						if( workerPtr->skill_level < minSkill )
							minSkill = workerPtr->skill_level;
					}

					//------------------------------//

					if( firmPtr->majority_race() == race_id )
					{
						if( skill.skill_level < minSkill+10 )
							continue;
					}
					else //-- for different race, only assign if the skill is significantly higher than the existing ones --//
					{
						if( skill.skill_level < minSkill+30 )
							continue;
					}
				}
				else
				{
					curRating += 300;		// if the firm is not full, rating + 300
				}
			}

			//-------- calculate the rating ---------//

			curRating += world.distance_rating( curXLoc, curYLoc, firmPtr->center_x, firmPtr->center_y );

			if( firmPtr->majority_race() == race_id )
				curRating += 70;

			curRating += (MAX_WORKER - firmPtr->worker_count) * 10;

			//-------------------------------------//

			if( curRating > bestRating )
			{
				bestRating = curRating;
				bestFirm   = firmPtr;
			}
		}

		if( bestFirm )
		{
			assign(bestFirm->loc_x1, bestFirm->loc_y1);
			return 1;
		}
	}

	//---- look for towns to assign to -----//

	bestRating = 0;
	int  hasTownInRegion=0;
	Town *townPtr, *bestTown=NULL;

	for( i=town_array.size() ; i>0 ; i-- )		// can't use ai_town_array[] because this function will be called by Unit::betray() when a unit defected to the player's kingdom
	{
		if( town_array.is_deleted(i) )
			continue;

		townPtr = town_array[i];

		if( townPtr->nation_recno != nation_recno )
			continue;

		if( townPtr->region_id != regionId )
			continue;

		hasTownInRegion = 1;

		if( townPtr->population >= MAX_TOWN_POPULATION || !townPtr->is_base_town )
			continue;

		//--- only assign to towns of the same race ---//

		if( ownNation->pref_town_harmony > 50 )
		{
			if( townPtr->majority_race() != race_id )
				continue;
		}

		//-------- calculate the rating ---------//

		curRating = world.distance_rating(curXLoc, curYLoc, townPtr->center_x, townPtr->center_y );

		curRating += 300 * townPtr->race_pop_array[race_id-1] / townPtr->population;		// racial homogenous bonus

		curRating += MAX_TOWN_POPULATION - townPtr->population;

		//-------------------------------------//

		if( curRating > bestRating )
		{
			bestRating = curRating;
			bestTown   = townPtr;
		}
	}

	if( bestTown )
	{
		assign(bestTown->loc_x1, bestTown->loc_y1);
		return 1;
	}

	//----- if we don't have any existing towns in this region ----//

	if( !hasTownInRegion )
	{

//#ifdef AMPLUS
		// --- if region is too small don't consider this area, stay in the island forever --//
		if( region_array[regionId]->region_stat_id == 0 )
			return 0;
//#endif

		//-- if we also don't have any existing camps in this region --//

		if( region_array.get_region_stat(regionId)->camp_nation_count_array[nation_recno-1]==0 )
		{
			//---- try to build one if this unit can ----//

			if( ownNation->cash > firm_res[FIRM_CAMP]->setup_cost &&
				 firm_res[FIRM_CAMP]->can_build(sprite_recno) &&
				 !leader_unit_recno )		// if this unit is commanded by a leader, let the leader build the camp
			{
				ai_build_camp();
			}
		}
		else		// if there is already a camp in this region, try to settle a new town next to the camp
		{
			ai_settle_new_town();
		}

		return 1;	// if we don't have any town in this region, return 1, so this unit won't be resigned and so it can wait for other units to set up camps and villages later ---//
	}

	return 0;
}
//---------- End of function Unit::think_normal_human_action --------//


//--------- Begin of function Unit::think_weapon_action --------//

int Unit::think_weapon_action()
{
	//---- first try to assign the weapon to an existing camp ----//

	if( think_assign_weapon_to_camp() )
		return 1;

	//----- if no camp to assign, build a new one ------//

	if( think_build_camp() )
		return 1;

	return 0;
}
//---------- End of function Unit::think_weapon_action --------//


//--------- Begin of function Unit::think_assign_weapon_to_camp --------//

int Unit::think_assign_weapon_to_camp()
{
	Nation 	*nationPtr = nation_array[nation_recno];
	FirmCamp *firmCamp, *bestCamp=NULL;
	int      curRating=0, bestRating=0;
	int		regionId = world.get_region_id( next_x_loc(), next_y_loc() );
	int		curXLoc = next_x_loc(), curYLoc = next_y_loc();

	for( int i=0 ; i<nationPtr->ai_camp_count ; i++ )
	{
		firmCamp = (FirmCamp*) firm_array[ nationPtr->ai_camp_array[i] ];

		if( firmCamp->region_id	!= regionId || firmCamp->is_worker_full() )
			continue;

		//-------- calculate the rating ---------//

		curRating = world.distance_rating(curXLoc, curYLoc, firmCamp->center_x, firmCamp->center_y );

		curRating += (MAX_WORKER - firmCamp->worker_count) * 10;

		//-------------------------------------//

		if( curRating > bestRating )
		{
			bestRating = curRating;
			bestCamp   = firmCamp;
		}
	}

	//-----------------------------------//

	if( bestCamp )
	{
		assign(bestCamp->loc_x1, bestCamp->loc_y1);
		return 1;
	}

	return 0;
}
//---------- End of function Unit::think_assign_weapon_to_camp --------//


//--------- Begin of function Unit::think_build_camp --------//
//
// Think about building a camp next to the town which is
// closest to the unit.
//
int Unit::think_build_camp()
{
	//---- select a town to build the camp ---//

	Nation* ownNation = nation_array[nation_recno];
	Town *townPtr, *bestTown=NULL;
	int   curRating=0, bestRating=0;
	int	regionId = world.get_region_id( next_x_loc(), next_y_loc() );
	int  	curXLoc = next_x_loc(), curYLoc = next_y_loc();

	for( int i=ownNation->ai_town_count-1 ; i>=0 ; i-- )
	{
		townPtr = town_array[ ownNation->ai_town_array[i] ];

		if( townPtr->region_id != regionId )
			continue;

		if( !townPtr->is_base_town || townPtr->no_neighbor_space )
			continue;

		curRating = world.distance_rating(curXLoc, curYLoc, townPtr->center_x, townPtr->center_y );

		if( curRating > bestRating )
		{
			bestRating = curRating;
			bestTown   = townPtr;
		}
	}

	if( bestTown )
		return bestTown->ai_build_neighbor_firm(FIRM_CAMP);

	return 0;
}
//---------- End of function Unit::think_build_camp --------//


//--------- Begin of function Unit::think_reward --------//

int Unit::think_reward()
{
	Nation* ownNation = nation_array[nation_recno];

	//----------------------------------------------------------//
	// The need to secure high loyalty on this unit is based on:
	// -its skill
	// -its combat level 
	// -soldiers commanded by this unit
	//----------------------------------------------------------//

	if( spy_recno && true_nation_recno() == nation_recno )		// if this is a spy of ours
	{
		return 0;		// Spy::think_reward() will handle this.
	}

	int curLoyalty = loyalty;
	int neededLoyalty;

	//----- if this unit is on a mission ------/

	if( ai_action_id )
	{
		neededLoyalty = UNIT_BETRAY_LOYALTY+10;
	}

	//----- otherwise only reward soldiers and generals ------//

	else if( skill.skill_id == SKILL_LEADING )
	{
		//----- calculate the needed loyalty --------//

		neededLoyalty = commanded_soldier_count()*5 + skill.skill_level;

		if( unit_mode == UNIT_MODE_OVERSEE )		// if this unit is an overseer
		{
			if( loyalty < UNIT_BETRAY_LOYALTY )		// if this unit's loyalty is < betrayel level, reward immediately
			{
				reward(nation_recno);		// reward it immediatley if it's an overseer, don't check ai_should_spend()
				return 1;
			}

			neededLoyalty += 30;
		}

		neededLoyalty = MAX( UNIT_BETRAY_LOYALTY+10, neededLoyalty );		// 10 points above the betray loyalty level to prevent betrayal
		neededLoyalty = MIN( 100, neededLoyalty );
	}
	else
	{
		return 0;
	}

	//------- if the loyalty is already high enough ------//

	if( curLoyalty >= neededLoyalty )
		return 0;

	//---------- see how many cash & profit we have now ---------//

	int rewardNeedRating = neededLoyalty - curLoyalty;

	if( curLoyalty < UNIT_BETRAY_LOYALTY+5 )
		rewardNeedRating += 50;

	if( ownNation->ai_should_spend(rewardNeedRating) )
	{
		reward(nation_recno);
		return 1;
	}

	return 0;
}
//---------- End of function Unit::think_reward --------//



//--------- Begin of function Unit::ai_leader_being_attacked --------//
//
// This function is called when the king is under attack.
//
// <int> attackerUnitRecno - recno of the attacker unit.
//
void Unit::ai_leader_being_attacked(int attackerUnitRecno)
{
	err_when( !team_info );

	if( unit_array[attackerUnitRecno]->nation_recno == nation_recno )		// this can happen when the unit has just changed nation
		return;

	//------------------------------------//

	int rc=0, callIntervalDays;

	if( rank_id == RANK_KING )
	{
		rc = 1;
		callIntervalDays = 7;
	}
	else if( rank_id == RANK_GENERAL )
	{
		rc = skill.skill_level >= 30 + (100-nation_array[nation_recno]->pref_keep_general)/2;		// 30 to 80
		callIntervalDays = 15;		// don't call too freqently
	}

	if( rc )
	{
		if( info.game_date > team_info->ai_last_request_defense_date + callIntervalDays )
		{
			team_info->ai_last_request_defense_date = info.game_date;
			nation_array[nation_recno]->ai_defend(attackerUnitRecno);
		}
	}
}
//---------- End of function Unit::ai_leader_being_attacked --------//


//--------- Begin of function Unit::think_king_flee --------//
//
// Note: we still need to keep think_king_action() because
// 		between these two functions, a number of things
//			may be done, like returning home camp. We only
//			carry out actions in this function if the king
//		   is in danger and urgently need to flee.
//
int Unit::think_king_flee()
{
	if( force_move_flag && cur_action != SPRITE_IDLE )		// the king is already fleeing now
		return 1;

	//------- if the king is alone --------//

	Nation* ownNation = nation_array[nation_recno];

	//------------------------------------------------//
	// When the king is alone and there is no assigned action OR
	// when the king is injured, the king will flee
	// back to its camp.
	//------------------------------------------------//

	if( ( team_info->member_count==0 && !ai_action_id ) ||
		 hit_points < 125-ownNation->pref_military_courage/4 )
	{
		//------------------------------------------//
		//
		// If the king is currently under attack, flee
		// to the nearest camp with the maximum protection.
		//
		//------------------------------------------//

		Firm *firmCamp, *bestCamp=NULL;
		int	curRating, bestRating=0;
		int	curXLoc = next_x_loc(), curYLoc = next_y_loc();
		int	curRegionId = world.get_region_id( curXLoc, curYLoc );

		if( cur_action == SPRITE_ATTACK )
		{
			for( int i=ownNation->ai_camp_count-1 ; i>=0 ; i-- )
			{
				firmCamp = (FirmCamp*) firm_array[ ownNation->ai_camp_array[i] ];

				if( firmCamp->region_id != curRegionId )
					continue;

				if( firmCamp->overseer_recno && rank_id!=RANK_KING )		// if there is already a commander in this camp. However if this is the king, than ingore this
					continue;

				curRating = world.distance_rating( curXLoc, curYLoc,
								 firmCamp->center_x, firmCamp->center_y );

				if( curRating > bestRating )
				{
					bestRating = curRating;
					bestCamp   = firmCamp;
				}
			}

		}
		else if( home_camp_firm_recno )	// if there is a home for the king
		{
			bestCamp = firm_array[home_camp_firm_recno];
		}

		//------------------------------------//

		if( bestCamp )
		{
			if( config.ai_aggressiveness > OPTION_LOW )
				force_move_flag = 1;

			assign( bestCamp->loc_x1, bestCamp->loc_y1 );
		}
		else	// if the king is neither under attack or has a home camp, then call the standard think_leader_action()
		{
			think_leader_action();
		}

		return cur_action != SPRITE_IDLE;
	}

	return 0;
}
//---------- End of function Unit::think_king_flee --------//


//--------- Begin of function Unit::think_general_flee --------//

int Unit::think_general_flee()
{
	if( force_move_flag && cur_action != SPRITE_IDLE )		// the general is already fleeing now
		return 1;

	//------- if the general is alone --------//

	Nation* ownNation = nation_array[nation_recno];

	//------------------------------------------------//
	// When the general is alone and there is no assigned action OR
	// when the general is injured, the general will flee
	// back to its camp.
	//------------------------------------------------//

	if( ( team_info->member_count==0 && !ai_action_id ) ||
		 hit_points < max_hit_points * (75+ownNation->pref_military_courage/2) / 200 )		// 75 to 125 / 200
	{
		//------------------------------------------//
		//
		// If the general is currently under attack, flee
		// to the nearest camp with the maximum protection.
		//
		//------------------------------------------//

		Firm *firmCamp, *bestCamp=NULL;
		int	curRating, bestRating=0;
		int	curXLoc = next_x_loc(), curYLoc = next_y_loc();
		int	curRegionId = world.get_region_id( curXLoc, curYLoc );

		if( cur_action == SPRITE_ATTACK )
		{
			for( int i=ownNation->ai_camp_count-1 ; i>=0 ; i-- )
			{
				firmCamp = (FirmCamp*) firm_array[ ownNation->ai_camp_array[i] ];

				if( firmCamp->region_id != curRegionId )
					continue;

				curRating = world.distance_rating( curXLoc, curYLoc,
								 firmCamp->center_x, firmCamp->center_y );

				if( curRating > bestRating )
				{
					bestRating = curRating;
					bestCamp   = firmCamp;
				}
			}

		}
		else if( home_camp_firm_recno )	// if there is a home for the general
		{
			bestCamp = firm_array[home_camp_firm_recno];
		}

		//------------------------------------//

		if( bestCamp )
		{
			if( bestCamp->overseer_recno )		// if there is already an overseer there, just move close to the camp for protection
			{
				if( config.ai_aggressiveness > OPTION_LOW )
					force_move_flag = 1;

				move_to( bestCamp->loc_x1, bestCamp->loc_y1 );
			}
			else
				assign( bestCamp->loc_x1, bestCamp->loc_y1 );
		}
		else	// if the general is neither under attack or has a home camp, then call the standard think_leader_action()
		{
			think_leader_action();
		}

		return cur_action != SPRITE_IDLE;
	}

	return 0;
}
//---------- End of function Unit::think_general_flee --------//


//--------- Begin of function Unit::ai_build_camp --------//
//
// Order this unit to build a camp in its region.
//
int Unit::ai_build_camp()
{
	//--- to prevent building more than one camp at the same time ---//

	int     curRegionId = region_id();
	Nation* ownNation = nation_array[nation_recno];

	if( ownNation->is_action_exist( ACTION_AI_BUILD_FIRM, FIRM_CAMP, curRegionId ) )
		return 0;

	//------- locate a place for the camp --------//

	FirmInfo* firmInfo = firm_res[FIRM_CAMP];
	int 		 xLoc=0, yLoc=0;
	char 	 	 teraMask = UnitRes::mobile_type_to_mask(UNIT_LAND);

	if( world.locate_space_random(xLoc, yLoc, MAX_WORLD_X_LOC-1,
		 MAX_WORLD_Y_LOC-1, firmInfo->loc_width+2, firmInfo->loc_height+2,   // leave at least one location space around the building
		 MAX_WORLD_X_LOC*MAX_WORLD_Y_LOC, curRegionId, 1, teraMask) )
	{
		return ownNation->add_action( xLoc, yLoc, -1, -1,
					ACTION_AI_BUILD_FIRM, FIRM_CAMP, 1, sprite_recno );
	}

	return 0;
}
//---------- End of function Unit::ai_build_camp --------//


//--------- Begin of function Unit::ai_settle_new_town --------//
//
// Settle a new village next to one of the camps in this region.
//
int Unit::ai_settle_new_town()
{
	//----- locate a suitable camp for the new town to settle next to ----//

	Nation* 	 ownNation = nation_array[nation_recno];
	FirmCamp* firmCamp, *bestCamp=NULL;
	int	    curRegionId = region_id();
	int		 curRating, bestRating=0;

	for( int i=ownNation->ai_camp_count-1 ; i>=0 ; i-- )
	{
		firmCamp = (FirmCamp*) firm_array[ ownNation->ai_camp_array[i] ];

		if( firmCamp->region_id != curRegionId )
			continue;

		curRating = firmCamp->total_combat_level();

		if( curRating > bestRating )
		{
			bestRating = curRating;
			bestCamp   = firmCamp;
		}
	}

	if( !bestCamp )
		return 0;

	//--------- settle a new town now ---------//

	int xLoc=bestCamp->loc_x1;
	int yLoc=bestCamp->loc_y1;

	if( world.locate_space(xLoc, yLoc, bestCamp->loc_x2, bestCamp->loc_y2,
								  STD_TOWN_LOC_WIDTH, STD_TOWN_LOC_HEIGHT,
								  UNIT_LAND, curRegionId, 1 ) )					// 1-build flag
	{
		settle( xLoc, yLoc );
		return 1;
	}

	return 0;
}
//---------- End of function Unit::ai_settle_new_town --------//


//--------- Begin of function Unit::ai_handle_seek_path_fail --------//
//
// This function is used for handling cases when AI units are not
// able to seek a path successfully.
//
int Unit::ai_handle_seek_path_fail()
{
	if( seek_path_fail_count < 5 )		// wait unit it has failed many times
		return 0;

	//----- try to move to a new location -----//

	if( seek_path_fail_count==5 )
	{
		stop2();		// stop the unit and think for new action
		return 0;
	}

	//--- if the seek path has failed too many times, resign the unit ---//

	int resignFlag = 0;

	if( rank_id == RANK_SOLDIER && !leader_unit_recno )
	{
		if( seek_path_fail_count>=7 )
			resignFlag = 1;
	}
	else if( rank_id == RANK_GENERAL )
	{
		if( seek_path_fail_count >= 7+skill.skill_level/10 )
			resignFlag = 1;
	}

	if( resignFlag && is_visible() )
	{
		resign(COMMAND_AI);
		return 1;
	}
	else
		return 0;
}
//---------- End of function Unit::ai_handle_seek_path_fail --------//

