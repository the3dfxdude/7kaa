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

//Filename   : OAI_CAPT.CPP
//Description: AI - capturing independent towns

#include <ALL.h>
#ifdef USE_DPLAY
#include <OREMOTE.h>
#endif
#include <OCONFIG.h>
#include <OUNIT.h>
#include <OF_CAMP.h>
#include <OF_INN.h>
#include <ONATION.h>

//------- define struct CaptureTown -------//

struct CaptureTown
{
	short town_recno;
	short min_resistance;
};

//------ Declare static functions --------//

static int sort_capture_town_function( const void *a, const void *b );

//--------- Begin of function Nation::think_capture --------//
//
int Nation::think_capture()
{
	if( ai_camp_count==0 )		// this can happen when a new nation has just emerged
		return 0;

	//--- don't capture if the AI is using growth and capture strategy (as opposite to build mine strategy) ---//

	if( ai_mine_count==0 && total_population < 25 )
		return 0;

	//-----------------------------------------//

	if( think_capture_independent() )
		return 1;

	return 0;
}
//---------- End of function Nation::think_capture ---------//


//--------- Begin of function Nation::think_capture_independent --------//
//
// Think about capturing independent towns.
//
int Nation::think_capture_independent()
{
	//------- Capture target choices -------//

	#define MAX_CAPTURE_TOWN	30

	CaptureTown capture_town_array[MAX_CAPTURE_TOWN];
	short 		capture_town_count=0;

	//--- find the town that makes most sense to capture ---//

	int 	townRecno;
	Town* townPtr;

	for(townRecno=town_array.size(); townRecno>0; townRecno--)
	{
		if(town_array.is_deleted(townRecno))
			continue;

		townPtr = town_array[townRecno];

		if( townPtr->nation_recno )		// only capture independent towns
			continue;

		if( townPtr->no_neighbor_space )		// if there is no space in the neighbor area for building a new firm.
			continue;

		if( townPtr->rebel_recno )			// towns controlled by rebels will not drop in resistance even if a command base is present
			continue;

		//------ only if we have a presence/a base town in this region -----//

		if( !has_base_town_in_region(townPtr->region_id) )
			continue;

		//---- check if there are already camps linked to this town ----//

		int i;
		for( i=townPtr->linked_firm_count-1 ; i>=0 ; i-- )
		{
			Firm* firmPtr = firm_array[ townPtr->linked_firm_array[i] ];

			if( firmPtr->firm_id != FIRM_CAMP )
				continue;

			//------ if we already have a camp linked to this town -----//

			if( firmPtr->nation_recno == nation_recno )
				break;

			//--- if there is an overseer with high leadership and right race in the opponent's camp, do bother to compete with him ---//

			if( firmPtr->overseer_recno )
			{
				Unit* unitPtr = unit_array[firmPtr->overseer_recno];

				if( unitPtr->skill.skill_level >= 70 &&
					 unitPtr->race_id == townPtr->majority_race() )
				{
					break;
				}
			}
		}

		if( i>=0 )			// there is already a camp linked to this town and we don't want to get involved with its capturing plan
			continue;

		//-- if the town has linked military camps of the same nation --//

		int targetResistance  = capture_expected_resistance(townRecno);
		int averageResistance = townPtr->average_resistance(nation_recno);
		int minResistance 	 = MIN( averageResistance, targetResistance );

		if( minResistance < 50 - pref_peacefulness/5 )		// 30 to 50 depending on
		{
			capture_town_array[capture_town_count].town_recno 	   = townRecno;
			capture_town_array[capture_town_count].min_resistance = minResistance;

			capture_town_count++;
		}
	}

	//------ sort the capture target choices by min_resistance ----//

	qsort( &capture_town_array, capture_town_count, sizeof(capture_town_array[0]), sort_capture_town_function );

	//------- try to capture the town in their resistance order ----//

	for( int i=0 ; i<capture_town_count ; i++ )
	{
		err_when( town_array.is_deleted(capture_town_array[i].town_recno) );

		//-------------------------------------------//
		// If the map is set to unexplored, wait for a
		// reasonable amount of time before moving out
		// to build the mine.
		//-------------------------------------------//

		if( !config.explore_whole_map )
		{
			Town* targetTown = town_array[ capture_town_array[i].town_recno ];

			int j;
			for( j=0 ; j<ai_town_count ; j++ )
			{
				Town* ownTown = town_array[ ai_town_array[j] ];

				int townDistance = m.points_distance(targetTown->center_x, targetTown->center_y, 
										 ownTown->center_x, ownTown->center_y);

				if( info.game_date-info.game_start_date >
					 townDistance * (5-config.ai_aggressiveness) / 5 )		// 3 to 5 / 5
				{
					break;
				}
			}

			if( j==ai_town_count )
				continue;
		}

		if( start_capture( capture_town_array[i].town_recno ) )
			return 1;
	}

	return 0;
}
//---------- End of function Nation::think_capture_independent ---------//


//--------- Begin of function Nation::should_use_cash_to_capture --------//
//
int Nation::should_use_cash_to_capture()
{
	//--- if we have plenty of cash, use cash to decrease the resistance of the villagers ---//

	return military_rank_rating() < 50+pref_peacefulness/5 &&		// 50 to 70
			 ai_should_spend(pref_loyalty_concern/4);
}
//---------- End of function Nation::should_use_cash_to_capture ---------//


//--------- Begin of function Nation::capture_expected_resistance --------//
//
// The lowest resistance can be expected if we are going to capture the
// town.
//
int Nation::capture_expected_resistance(int townRecno)
{
	//--- we have plenty of cash, use cash to decrease the resistance of the villagers ---//

	if( should_use_cash_to_capture() )
		return 0;			// return zero resistance

	//----- the average resistance determines the captureRating ------//

	int	captureRating = 0;
	Town* townPtr = town_array[townRecno];

	int averageResistance;

	if( townPtr->nation_recno )
		averageResistance = townPtr->average_loyalty();
	else
		averageResistance = townPtr->average_resistance(nation_recno);

	//----- get the id. of the most populated races in the town -----//

	int majorityRace = townPtr->majority_race();

	err_when( !majorityRace );		// this should not happen

	//---- see if there are general available for capturing this town ---//

	int targetResistance=0;

	if( !find_best_capturer(townRecno, majorityRace, targetResistance) )
		return 100;

	int resultResistance =
		( targetResistance * townPtr->race_pop_array[majorityRace-1] +
		  averageResistance * (townPtr->population - townPtr->race_pop_array[majorityRace-1]) )
		/ townPtr->population;

	return resultResistance;
}
//---------- End of function Nation::capture_expected_resistance ---------//


//--------- Begin of function Nation::start_capture --------//
//
int Nation::start_capture(int townRecno)
{
	//--- find the two races with most population in the town ---//

	Town* townPtr = town_array[townRecno];

	int  majorityRace=0;

	//--- if it's an independent town, the race of the commander must match with the race of the town ---//

	if( townPtr->nation_recno == 0 )
	{
		majorityRace = townPtr->majority_race();
		err_when( !majorityRace );		// this shouldn't happen
	}

	//---- see if we have generals in the most populated race, if so build a camp next to the town ----//

	return capture_build_camp(townRecno, majorityRace);
}
//---------- End of function Nation::start_capture ---------//


//--------- Begin of function Nation::capture_build_camp --------//
//
int Nation::capture_build_camp(int townRecno, int raceId)
{
	Town* captureTown = town_array[townRecno];

	//---- find the best available general for the capturing action ---//

	int targetResistance;

	int unitRecno = find_best_capturer(townRecno, raceId, targetResistance);

	if( !unitRecno )
		unitRecno = hire_best_capturer(townRecno, raceId);

	if( !unitRecno )
	{
		//--- if we have plenty of cash and can use cash to decrease the resistance of the independent villagers ---//

		if( should_use_cash_to_capture() )
		{
			char resultFlag;

			Unit* skilledUnit = find_skilled_unit(SKILL_LEADING, raceId,
									  captureTown->center_x, captureTown->center_y, resultFlag);

			if( skilledUnit )
				unitRecno = skilledUnit->sprite_recno;
		}

		if( !unitRecno )
			return 0;
	}

	//------- locate a place to build the camp -------//

	short buildXLoc, buildYLoc;

	if( !find_best_firm_loc(FIRM_CAMP, captureTown->loc_x1, captureTown->loc_y1, buildXLoc, buildYLoc) )
	{
		captureTown->no_neighbor_space = 1;
		return 0;
	}

	//--- if the picked unit is an overseer of an existng camp ---//

	if( !mobilize_capturer(unitRecno) )
		return 0;

	//---------- add the action to the queue now ----------//

	err_when( captureTown->nation_recno==0 &&
				 unit_array[unitRecno]->race_id != captureTown->majority_race() );

	int actionRecno = add_action( buildXLoc, buildYLoc, captureTown->loc_x1, captureTown->loc_y1,
							ACTION_AI_BUILD_FIRM, FIRM_CAMP, 1, unitRecno );

	if( actionRecno )
		process_action(actionRecno);

	return 1;
}
//---------- End of function Nation::capture_build_camp ---------//


//-------- Begin of function Nation::find_best_capturer ------//
//
// Find an existing unit as the capturer of the town.
//
// <int>  townRecno - recno of the town to capture
// <int>  raceId    - race id. of the capturer. 0 if any races.
// <int&> bestTargetResistance - a reference var for returning the target resistance if the returned unit is assigned as the overseer
//
// return: <int> the recno of the unit found.
//
int Nation::find_best_capturer(int townRecno, int raceId, int& bestTargetResistance)
{
	#define MIN_CAPTURE_RESISTANCE_DEC 	20		// if we assign a unit as the commander, the minimum expected resistance decrease should be 20, otherwise we don't do it.

	Unit* unitPtr;
	Town* targetTown = town_array[townRecno];
	Firm* firmPtr;
	int   targetResistance;
	int   bestUnitRecno=0;

	bestTargetResistance = 100;

	for( int i=ai_general_count-1 ; i>=0 ; i-- )
	{
		unitPtr = unit_array[ ai_general_array[i] ];

		if( raceId && unitPtr->race_id != raceId )
			continue;

		err_when( unitPtr->nation_recno != nation_recno );
		err_when( unitPtr->rank_id != RANK_KING && unitPtr->rank_id != RANK_GENERAL );

		if( unitPtr->nation_recno != nation_recno )
			continue;

      //---- if this unit is on a mission ----//

		if( unitPtr->home_camp_firm_recno )
			continue;

		//---- don't use the king to build camps next to capture enemy towns, only next to independent towns ----//

		if( unitPtr->rank_id == RANK_KING && targetTown->nation_recno )
			continue;

		//----- if this unit is in a camp -------//

		if( unitPtr->unit_mode == UNIT_MODE_OVERSEE )
		{
			firmPtr = firm_array[unitPtr->unit_mode_para];

			//--- check if the unit currently in a command base trying to take over an independent town ---//

			int j;
			for( j=firmPtr->linked_town_count-1 ; j>=0 ; j-- )
			{
				Town* townPtr = town_array[ firmPtr->linked_town_array[j] ];

				//--- if the unit is trying to capture an independent town and he is still influencing the town to decrease resistance ---//

				if( townPtr->nation_recno==0 &&
					 townPtr->average_target_resistance(nation_recno) <
					 townPtr->average_resistance(nation_recno) )
				{
					break;	// then don't use this unit
				}
			}

			if( j>=0 )		// if so, don't use this unit
				continue;
		}

		//--- if this unit is idle and the region ids are matched ---//

		if( unitPtr->action_mode != ACTION_STOP ||
			 unitPtr->region_id() != targetTown->region_id )
		{
			continue;
		}

		//------- get the unit's influence index --------//

		err_when( unitPtr->skill.skill_id != SKILL_LEADING );

		targetResistance = 100-targetTown->camp_influence(unitPtr->sprite_recno); 	// influence of this unit if he is assigned as a commander of a military camp

		//-- see if this unit's rating is higher than the current best --//

		if( targetResistance < bestTargetResistance )
		{
			bestTargetResistance = targetResistance;
			bestUnitRecno = unitPtr->sprite_recno;
		}
	}

	return bestUnitRecno;
}
//-------- End of function Nation::find_best_capturer -------//


//-------- Begin of function Nation::mobilize_capturer ------//
//
// Mobilize the capturer unit if he isn't mobilized yet.
//
int Nation::mobilize_capturer(int unitRecno)
{
	//--- if the picked unit is an overseer of an existng camp ---//

	Unit* unitPtr = unit_array[unitRecno];

	if( unitPtr->unit_mode == UNIT_MODE_OVERSEE )
	{
		Firm* firmPtr = firm_array[unitPtr->unit_mode_para];
		Town* townPtr;

		//-- can recruit from either a command base or seat of power --//

		//-- train a villager with leadership to replace current overseer --//

		int i;
		for( i=0 ; i<firmPtr->linked_town_count ; i++ )
		{
			townPtr = town_array[ firmPtr->linked_town_array[i] ];

			if( townPtr->nation_recno != nation_recno )
				continue;

			//--- first try to train a unit who is racially homogenous to the commander ---//

			int unitRecno = townPtr->recruit( SKILL_LEADING, firmPtr->majority_race(), COMMAND_AI );

			//--- if unsucessful, then try to train a unit whose race is the same as the majority of the town ---//

			if( !unitRecno )
				unitRecno = townPtr->recruit( SKILL_LEADING, townPtr->majority_race(), COMMAND_AI );

			if( unitRecno )
			{
				add_action(townPtr->loc_x1, townPtr->loc_y1, -1, -1, ACTION_AI_ASSIGN_OVERSEER, FIRM_CAMP);
				break;
			}
		}

		if( i==firmPtr->linked_town_count )			// unsuccessful
			return 0;

		//------- mobilize the current overseer --------//

		firmPtr->mobilize_overseer();
	}

	return 1;
}
//-------- End of function Nation::mobilize_capturer -------//


//-------- Begin of function Nation::hire_best_capturer ------//
//
// Hire the best unit you can find in one of the existing inns.
//
// <int>  townRecno 			    - recno of the town to capture
// <int>  raceId				    - race id. of the unit to hire
//
// return: <int> the recno of the unit hired.
//
int Nation::hire_best_capturer(int townRecno, int raceId)
{
	if( !ai_should_hire_unit(30) )		// 30 - importance rating
		return 0;

	FirmInn	*firmInn;
	Firm		*firmPtr;
	InnUnit *innUnit;
	Skill		*innUnitSkill;
	int		i, j, innUnitCount, curRating;
	int		bestRating=0, bestInnRecno=0, bestInnUnitId=0;
	Town* 	townPtr = town_array[townRecno];
	int		destRegionId = world.get_region_id(townPtr->loc_x1, townPtr->loc_y1);

	for(i=0; i<ai_inn_count; i++)
	{
		firmPtr = (FirmInn*) firm_array[ai_inn_array[i]];

		err_when( firmPtr->firm_id != FIRM_INN );

		if( firmPtr->region_id != destRegionId )
			continue;

		firmInn = firmPtr->cast_to_FirmInn();

		innUnitCount=firmInn->inn_unit_count;

		if( !innUnitCount )
			continue;

		innUnit = firmInn->inn_unit_array + innUnitCount - 1;

		//------- check units in the inn ---------//

		for(j=innUnitCount; j>0; j--, innUnit--)
		{
			innUnitSkill = &(innUnit->skill);

			if( innUnitSkill->skill_id==SKILL_LEADING &&
				 unit_res[innUnit->unit_id]->race_id == raceId &&
				 cash >= innUnit->hire_cost )
			{
				//----------------------------------------------//
				// evalute a unit on:
				// -its race, whether it's the same as the nation's race
				// -the inn's distance from the destination
				// -the skill level of the unit.
				//----------------------------------------------//

				curRating = innUnitSkill->skill_level;

				if( curRating > bestRating )
				{
					bestRating = curRating;

					bestInnRecno  = firmInn->firm_recno;
					bestInnUnitId = j;
				}
			}
		}
	}

	if( !bestInnUnitId )
		return 0;

	//----------------------------------------------------//

	firmInn = (FirmInn*) firm_array[bestInnRecno];

	int unitRecno = firmInn->hire(bestInnUnitId);

	if( !unitRecno )
		return 0;

	unit_array[unitRecno]->set_rank(RANK_GENERAL);

	return unitRecno;
}
//-------- End of function Nation::hire_best_capturer -------//


//------ Begin of function sort_capture_town_function ------//
//
static int sort_capture_town_function( const void *a, const void *b )
{
	return ((CaptureTown*)a)->min_resistance - ((CaptureTown*)b)->min_resistance;
}
//------- End of function sort_capture_town_function ------//


