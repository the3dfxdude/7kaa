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

//Filename   : OAI_UNIT.CPP
//Description: AI - unit related functions

#include <ALL.h>
#include <OUNIT.h>
#include <OF_INN.h>
#include <OTOWN.h>
#include <ONATION.h>


//-------- Begin of function Nation::get_skilled_unit -------//
//
// <int>		skillId			-	the skill the selected unit should have
// <int>		raceId			-	the race the selected unit should have
//										(0 for any races)
// <ActionNode*> actionNode - the ActionNode of the action that needs this skilled unit.
//
// return: <Unit*> skilledUnit - pointer to the skilled unit.
//
Unit* Nation::get_skilled_unit(int skillId, int raceId, ActionNode* actionNode)
{
	//--------- get a skilled unit --------//

	Unit* skilledUnit;

	if(actionNode->unit_recno) // a unit has started training previously
	{
		if( unit_array.is_deleted(actionNode->unit_recno) )
			return NULL; // but unit is lost already and action has failed
		skilledUnit = unit_array[actionNode->unit_recno];
	}
	else
	{
		char resultFlag;
		int  xLoc, yLoc;

		//---- for harbor, we have to get the land region id. instead of the sea region id. ----//

		if( actionNode->action_mode==ACTION_AI_BUILD_FIRM &&
			 actionNode->action_para==FIRM_HARBOR )
		{
			int rc=0;

			for( yLoc=actionNode->action_y_loc ; yLoc<actionNode->action_y_loc+3 ; yLoc++ )
			{
				for( xLoc=actionNode->action_x_loc ; xLoc<actionNode->action_x_loc+3 ; xLoc++ )
				{
					if( region_array[ world.get_region_id(xLoc,yLoc) ]->region_type == REGION_LAND )
					{
						rc=1;
						break;
					}
				}

				if( rc )
					break;
			}
		}
		else
		{
			xLoc = actionNode->action_x_loc;
			yLoc = actionNode->action_y_loc;
		}

		//-----------------------------------------//

		skilledUnit = find_skilled_unit(skillId, raceId, xLoc, yLoc, resultFlag, actionNode->action_id);

		if( !skilledUnit )		// skilled unit not found
			return NULL;
	}

	//------ if the unit is still in training -----//

	if( !skilledUnit->is_visible() )
	{
		actionNode->next_retry_date = info.game_date + TOTAL_TRAIN_DAYS + 1;
		return NULL;		// continue processing this action after this date, this is used when training a unit for construction
	}

	err_when( !skilledUnit->race_id );

	return skilledUnit;
}
//-------- End of function Nation::get_skilled_unit -------//


//--------- Begin of function Nation::find_skilled_unit --------//
//
// <int>		skillId			-	the skill the selected unit should have
// <int>		raceId			-	the race the selected unit should have
//										(0 for any races)
// <short>	destX, destY	-	location the unit move to
// <char&>	resultFlag		-	describle how to find the skilled unit
//										0 - for unable to train unit,
//										1 - for existing skilled unit
//										2 - for unit hired from inn
//										3 - for training unit in town (training is required)
//
// [int]	   actionId - the action id. of the action which
//							  the unit should do after it has finished training.
//
// return the unit pointer pointed to the skilled unit
//
Unit* Nation::find_skilled_unit(int skillId, int raceId, short destX, short destY, char& resultFlag, int actionId)
{
	//----- try to find an existing unit with the required skill -----//

	Unit	*skilledUnit = NULL;
	Unit	*unitPtr;
	Firm	*firmPtr;
	short	curDist, minDist=0x1000;
	int   destRegionId = world.get_region_id(destX, destY);

	for(int i=unit_array.size(); i>0; i--)
	{
		if(unit_array.is_deleted(i))
			continue;

		unitPtr = unit_array[i];

		if( unitPtr->nation_recno!=nation_recno || !unitPtr->race_id )
			continue;

		if( raceId && unitPtr->race_id != raceId )
			continue;

		//---- if this unit is on a mission ----//

		if( unitPtr->home_camp_firm_recno )
			continue;

		if( unitPtr->region_id() != destRegionId )
			continue;

		//----- if this is a mobile unit ------//

		if( unitPtr->is_visible() )
		{
			if( !unitPtr->is_ai_all_stop() )
				continue;

			if( unitPtr->skill.skill_id==skillId &&
				 unitPtr->cur_action!=SPRITE_ATTACK && !unitPtr->ai_action_id )
			{
				curDist = misc.points_distance(unitPtr->next_x_loc(), unitPtr->next_y_loc(), destX, destY);

				if(curDist < minDist)
				{
					skilledUnit = unitPtr;
					minDist = curDist;
				}
			}
		}

		//------- if this is an overseer ------//

		else if( skillId==SKILL_LEADING && unitPtr->unit_mode==UNIT_MODE_OVERSEE )
		{
			firmPtr = firm_array[unitPtr->unit_mode_para];

			if( firmPtr->region_id != destRegionId )
				continue;

			if( firmPtr->firm_id == FIRM_CAMP )
			{
				//--- if this military camp is going to be closed, use this overseer ---//

				if( firmPtr->should_close_flag )
				{
					firmPtr->mobilize_overseer();
					skilledUnit = unitPtr;       	// pick this overseer
					break;
				}
			}
		}
		else if( skillId==SKILL_CONSTRUCTION && unitPtr->unit_mode==UNIT_MODE_CONSTRUCT )		// the unit is a residental builder for repairing the firm
		{
			firmPtr = firm_array[unitPtr->unit_mode_para];

			if( !firmPtr->under_construction )		// only if the unit is repairing instead of constructing the firm
			{
				if( firmPtr->set_builder(0) )			// return 1 if the builder is mobilized successfully, 0 if the builder was killed because of out of space on the map
				{
					skilledUnit = unitPtr;
					break;
				}
			}
		}
	}

	//---------------------------------------------------//

	if(skilledUnit)
	{
		resultFlag = 1;
	}
	else
	{
		//--- if no existing skilled unit found, try to hire one from inns ---//

		int unitRecno = hire_unit(skillId, raceId, destX, destY);	// this function will try going with hiring units that are better than training your own ones

		if( unitRecno )
		{
			skilledUnit = unit_array[unitRecno];
			resultFlag = 2;
		}
		else	//--- if still cannot get a skilled unit, train one ---//
		{
			int trainTownRecno;

			if( train_unit(skillId, raceId, destX, destY, trainTownRecno, actionId) )
				resultFlag = 3;
			else
				resultFlag = 0;
		}
	}

	err_when(skilledUnit && !skilledUnit->is_visible());
	err_when(skilledUnit && skilledUnit->rank_id==RANK_KING && (skillId!=SKILL_CONSTRUCTION && skillId!=SKILL_LEADING));
	err_when(skilledUnit && (skilledUnit->cur_action==SPRITE_DIE || skilledUnit->action_mode==ACTION_DIE));

	return skilledUnit;
}
//---------- End of function Nation::find_skilled_unit --------//


//--------- Begin of function Nation::hire_unit --------//
//
// <int> importantRating - importance of hiring the unit.
//
int Nation::ai_should_hire_unit(int importanceRating)
{
	if( !ai_inn_count )		// don't hire any body in the cash is too low
		return 0;

	return ai_should_spend(importanceRating + pref_hire_unit/5 - 10 );		// -10 to +10 depending on pref_hire_unit
}
//---------- End of function Nation::hire_unit --------//


//--------- Begin of function Nation::hire_unit --------//
//
// <int> 	skillId - the skill the unit should have
// <int>		raceId  - the race the selected unit should have
//							 (0 for any races)
//	<short>	destX	  - the x location the unit will move to
//	<short>	destY	  - the y location the unit will move to
//
// hire unit with specified skill from an inn
// return the unit pointer pointed to the skilled unit
//
// return: <int> recno of the unit recruited.
//
int Nation::hire_unit(int skillId, int raceId, short destX, short destY)
{
	if( !ai_should_hire_unit(20) )			// 20 - importance rating
		return 0;

	//-------------------------------------------//

	FirmInn	*firmInnPtr;
	Firm		*firmPtr;
	InnUnit *innUnit;
	Skill		*innUnitSkill;
	int		i, j, innUnitCount, curRating, curFirmDist;
	int		bestRating=0, bestInnRecno=0, bestInnUnitId=0;
	int		destRegionId = world.get_region_id(destX, destY);

	for(i=0; i<ai_inn_count; i++)
	{
		firmPtr = firm_array[ai_inn_array[i]];

		if( firmPtr->region_id != destRegionId )
			continue;

		firmInnPtr = firmPtr->cast_to_FirmInn();

		innUnitCount=firmInnPtr->inn_unit_count;

		if( !innUnitCount )
			continue;

		innUnit = firmInnPtr->inn_unit_array + innUnitCount - 1;

		curFirmDist = misc.points_distance(firmPtr->center_x, firmPtr->center_y, destX, destY);

		//------- check units in the inn ---------//

		for(j=innUnitCount; j>0; j--, innUnit--)
		{
			innUnitSkill = &(innUnit->skill);

			if( innUnitSkill->skill_id==skillId &&
				 (!raceId || unit_res[innUnit->unit_id]->race_id == raceId) &&
				 cash >= innUnit->hire_cost )
			{
				//----------------------------------------------//
				// evalute a unit on:
				// -its race, whether it's the same as the nation's race
				// -the inn's distance from the destination
				// -the skill level of the unit.
				//----------------------------------------------//

				curRating = innUnitSkill->skill_level
								- (100-100*curFirmDist/MAX_WORLD_X_LOC);

				if( unit_res[innUnit->unit_id]->race_id == race_id )
					curRating += 50;

				if( curRating > bestRating )
				{
					bestRating = curRating;

					bestInnRecno  = firmInnPtr->firm_recno;
					bestInnUnitId = j;
				}
			}
		}
	}

	//----------------------------------------------------//

	if( bestInnUnitId )
	{
		firmPtr = firm_array[bestInnRecno];
		firmInnPtr = firmPtr->cast_to_FirmInn();

		return firmInnPtr->hire(bestInnUnitId);
	}

	return 0;
}
//---------- End of function Nation::hire_unit --------//


//--------- Begin of function Nation::train_unit --------//
//
// <int> 	skillId - the skill the unit should have
// <int>		raceId  - the race the selected unit should have
//							 (0 for any races)
//	<short>	destX	  - the x location the unit will move to
//	<short>	destY	  - the y location the unit will move to
//
// <int&>   trainTownRecno - the recno of the town where this unit is trained.
//
// [int]	   actionId - the action id. of the action which
//							  the unit should do after it has finished training.
//
// return: <int> recno of the unit trained.
//
int Nation::train_unit(int skillId, int raceId, short destX, short destY, int& trainTownRecno, int actionId)
{
	//----- locate the best town for training the unit -----//

	int 	 i;
	Town	 *townPtr;
	int	 curDist, curRating, bestRating=0;
	int    destRegionId = world.get_loc(destX, destY)->region_id;

	trainTownRecno = 0;

	for(i=0; i<ai_town_count; i++)
	{
		townPtr = town_array[ai_town_array[i]];

		if( !townPtr->jobless_population || townPtr->train_unit_recno ||	// no jobless population or currently a unit is being trained
			 !townPtr->has_linked_own_camp )
		{
			continue;
		}

		if( townPtr->region_id != destRegionId )
			continue;

		if( raceId && townPtr->jobless_race_pop_array[raceId-1] <= 0 )
			continue;

		//--------------------------------------//

		curDist = misc.points_distance(townPtr->center_x, townPtr->center_y, destX, destY);

		curRating = 100-100*curDist/MAX_WORLD_X_LOC;

		if( curRating > bestRating )
		{
			bestRating 	  = curRating;
			trainTownRecno = townPtr->town_recno;
		}
	}

	if( !trainTownRecno )
		return 0;

	//---------- train the unit ------------//

	townPtr = town_array[trainTownRecno];

	if( !raceId )
		raceId = townPtr->pick_random_race(0, 1);		// 0-pick jobless units, 1-pick spy units

	if( !raceId )
		return 0;

	int unitRecno = townPtr->recruit(skillId, raceId, COMMAND_AI);

	if( !unitRecno )
		// can happen when training a spy and the selected recruit is an enemy spy
		return 0;

	townPtr->train_unit_action_id = actionId;		// set train_unit_action_id so the unit can immediately execute the action when he has finished training.
	return unitRecno;
}
//---------- End of function Nation::train_unit --------//


//--------- Begin of function Nation::recruit_jobless_worker --------//
//
// <Firm*> destFirmPtr  - the firm which the workers are recruited for.
// [int] preferedRaceId - the prefered race id.
//
// return: <int> recno of the unit recruited.
//
int Nation::recruit_jobless_worker(Firm* destFirmPtr, int preferedRaceId)
{
	#define MIN_AI_TOWN_POP		8

	int needSpecificRace, raceId;		// the race of the needed unit

	if( preferedRaceId )
	{
		raceId = preferedRaceId;
		needSpecificRace = 1;
	}
	else
	{
		if( destFirmPtr->firm_id == FIRM_BASE )          // for seat of power, the race must be specific
		{
			raceId = firm_res.get_build(destFirmPtr->firm_build_id)->race_id;
			needSpecificRace = 1;
		}
		else
		{
			raceId = destFirmPtr->majority_race();
			needSpecificRace = 0;
		}
	}

	if( !raceId )
		return 0;

	//----- locate the best town for recruiting the unit -----//

	Town	 *townPtr;
	int	 curDist, curRating, bestRating=0, bestTownRecno=0;

	for( int i=0; i<ai_town_count; i++ )
	{
		townPtr = town_array[ai_town_array[i]];

		err_when( townPtr->nation_recno != nation_recno );

		if( !townPtr->jobless_population )	// no jobless population or currently a unit is being recruited
			continue;

		if( !townPtr->should_ai_migrate() )		// if the town is going to migrate, disregard the minimum population consideration
		{
			if( townPtr->population < MIN_AI_TOWN_POP )		// don't recruit workers if the population is low
				continue;
		}

		if( !townPtr->has_linked_own_camp && townPtr->has_linked_enemy_camp )		// cannot recruit from this town if there are enemy camps but no own camps
			continue;

		if( townPtr->region_id != destFirmPtr->region_id )
			continue;

		//--- get the distance beteween town & the destination firm ---//

		curDist = misc.points_distance(townPtr->center_x, townPtr->center_y, destFirmPtr->center_x, destFirmPtr->center_y);

		curRating = 100-100*curDist/MAX_WORLD_X_LOC;

		//--- recruit units from non-base town first ------//

		if( !townPtr->is_base_town )
			curRating += 100;

		//---- if the town has the race that the firm needs most ----//

		if( townPtr->can_recruit(raceId) )
		{
			curRating += 50 + (int) townPtr->race_loyalty_array[raceId-1];
		}
		else
		{
			if( needSpecificRace )			// if the firm must need this race, don't consider the town if it doesn't have the race. 
				continue;
		}

		if( curRating > bestRating )
		{
			bestRating 	  = curRating;
			bestTownRecno = townPtr->town_recno;
		}
	}

	if( !bestTownRecno )
		return 0;

	//---------- recruit the unit ------------//

	townPtr = town_array[bestTownRecno];

	if( townPtr->recruitable_race_pop(raceId, 1) == 0 )
	{
		err_when( needSpecificRace );
		raceId = townPtr->pick_random_race(0, 1);				// 0-pick jobless only, 1-pick spy units

		if( !raceId )
			return 0;
	}

	//--- if the chosen race is not recruitable, pick any recruitable race ---//

	if( !townPtr->can_recruit(raceId) )
	{
		//---- if the loyalty is too low to recruit, grant the town people first ---//

		if( cash > 0 && townPtr->accumulated_reward_penalty < 10 )
			townPtr->reward(COMMAND_AI);

		//---- if the loyalty is still too low, return now ----//

		if( !townPtr->can_recruit(raceId) )
			return 0;
	}

	return townPtr->recruit(-1, raceId, COMMAND_AI);
}
//---------- End of function Nation::recruit_jobless_worker --------//


//--------- Begin of function Nation::recruit_on_job_worker --------//
//
// Get skilled workers from existing firms who have labor more than
// it really needs.
//
// <Firm*> destFirmPtr - the firm which the workers are recruited for.
// [int] preferedRaceId - the prefered race id.
//
// return: <int> recno of the unit recruited.
//
int Nation::recruit_on_job_worker(Firm* destFirmPtr, int preferedRaceId)
{
	err_when( !firm_res[destFirmPtr->firm_id]->need_worker );

	err_when( destFirmPtr->firm_id == FIRM_BASE );	// seat of power shouldn't call this function at all, as it doesn't handle the racial issue.

	if( !preferedRaceId )
	{
		preferedRaceId = destFirmPtr->majority_race();

		if( !preferedRaceId )
			return 0;
	}

	//--- scan existing firms to see if any of them have excess workers ---//

	int    aiFirmCount;		// reference vars for returning result
	short* aiFirmArray = update_ai_firm_array(destFirmPtr->firm_id, 0, 0, aiFirmCount);
	Firm*   firmPtr, *bestFirmPtr=NULL;
	int	  bestRating=0, curRating, curDistance;
	int	  hasHuman;
	Worker* workerPtr;

	int i;
	for( i=0 ; i<aiFirmCount ; i++ )
	{
		firmPtr = firm_array[aiFirmArray[i]];

		err_when( firmPtr->firm_id != destFirmPtr->firm_id );

		if( firmPtr->firm_recno == destFirmPtr->firm_recno )
			continue;

		if( firmPtr->region_id != destFirmPtr->region_id )
			continue;

		if( !firmPtr->ai_has_excess_worker() )
			continue;

		//-----------------------------------//

		curDistance = misc.points_distance( firmPtr->center_x, firmPtr->center_y,
						  destFirmPtr->center_x, destFirmPtr->center_y );

		curRating = 100 - 100 * curDistance / MAX_WORLD_X_LOC;

		workerPtr = firmPtr->worker_array;
		hasHuman  = 0;

		for( int j=0 ; j<firmPtr->worker_count ; j++, workerPtr++ )
		{
			if( workerPtr->race_id )
				hasHuman = 1;

			if( workerPtr->race_id == preferedRaceId )
			{
				//---- can't recruit this unit if he lives in a foreign town ----//

				if( workerPtr->town_recno &&
					 town_array[workerPtr->town_recno]->nation_recno != nation_recno )
				{
					continue;
				}

				//--------------------------//

				curRating += 100;
				break;
			}
		}

		if( hasHuman && curRating > bestRating )
		{
			bestRating = curRating;
			bestFirmPtr = firmPtr;
		}
	}

	if( !bestFirmPtr )
		return 0;

	//------ mobilize a worker form the selected firm ----//

	int workerId=0;

	workerPtr = bestFirmPtr->worker_array;

	for( i=1 ; i<=bestFirmPtr->worker_count ; i++, workerPtr++ )
	{
		//---- can't recruit this unit if he lives in a foreign town ----//

		if( workerPtr->town_recno &&
			 town_array[workerPtr->town_recno]->nation_recno != nation_recno )
		{
			continue;
		}

		//--------------------------------//

		if( workerPtr->race_id )		// if this is a human unit, take it first
			workerId = i;

		if( workerPtr->race_id == preferedRaceId )  // if we have a better one, take the better one
		{
			workerId = i;
			break;
		}
	}

	if( !workerId )		// this can happen if all the workers are foreign workers. 
		return 0;

	return bestFirmPtr->mobilize_worker( workerId, COMMAND_AI );
}
//---------- End of function Nation::recruit_on_job_worker --------//

