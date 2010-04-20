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

//Filename   : OAI_SPY.CPP
//Description: AI - Spy activities

#include <ALL.h>
#include <OFIRM.h>
#include <OTOWN.h>
#include <OUNIT.h>
#include <ONATION.h>
#include <OSPY.h>

//--------- Begin of function Nation::think_spy --------//

void Nation::think_spy()
{


}
//---------- End of function Nation::think_spy --------//


//--------- Begin of function Nation::ai_assign_spy_to_town --------//
//
// Think about sending spies to the specific town.
//
// <int> townRecno - recno of the town
// [int] raceId 	 - race id. of the spy
//							(default: majority_race() of the tonw)
//
// return: <int> 1 - a spy is assigned successfully.
// 				  0 - failure.
//
int Nation::ai_assign_spy_to_town(int townRecno, int raceId)
{
	Town* townPtr = town_array[townRecno];

	if( townPtr->population >= MAX_TOWN_POPULATION )
		return 0;

	if( !raceId )
		raceId = townPtr->majority_race();

	int mobileOnly = townPtr->nation_recno == nation_recno;   // if assign to own towns/firms, only get mobile spies, don't get spies from existing towns/firms as that will result in a loop effect

	return ai_assign_spy( townPtr->loc_x1, townPtr->loc_y1, raceId, mobileOnly );
}
//---------- End of function Nation::ai_assign_spy_to_town --------//


//--------- Begin of function Nation::ai_assign_spy_to_firm --------//
//
// Think about sending spies to the specific firm.
//
// return: <int> 1 - a spy is assigned successfully.
// 				  0 - failure.
//
int Nation::ai_assign_spy_to_firm(int firmRecno)
{
	Firm* firmPtr = firm_array[firmRecno];

	err_when( !firmPtr->worker_array );

	//---- check if the firm is full or not -----//

	if( firmPtr->nation_recno == nation_recno )	// if it's our own firm
	{
		if( firmPtr->is_worker_full() )	// use is_worker_full() for own firms as it take into account of units patrolling outside
			return 0;
	}
	else
	{
		if( firmPtr->worker_count == MAX_WORKER )
			return 0;
	}

	//------ look for an existing spy -------//

	int raceId	   = firmPtr->majority_race();
	int mobileOnly = firmPtr->nation_recno == nation_recno;   // if assign to own firms/firms, only get mobile spies, don't get spies from existing firms/firms as that will result in a loop effect

	return ai_assign_spy( firmPtr->loc_x1, firmPtr->loc_y1, raceId, mobileOnly );
}
//---------- End of function Nation::ai_assign_spy_to_firm --------//


//--------- Begin of function Nation::ai_assign_spy --------//
//
// Try to locate an existing spy for use.
//
// <int> targetXLoc, targetYLoc - the target location
// [int] spyRaceId			  - if specified, only spies of this race
//											 will be located. (default:0)
// [int] mobileOnly 				  - get mobile spies only. (default:0)
//
int Nation::ai_assign_spy(int targetXLoc, int targetYLoc, int spyRaceId, int mobileOnly)
{
	int unitRecno=0;

	//---- try to find an existing spy ----//

	Spy* spyPtr = ai_find_spy( targetXLoc, targetYLoc, spyRaceId, mobileOnly );

	if( spyPtr )
		unitRecno = spyPtr->mobilize_spy();

	//--- if not successful, then try to hire one ---//

	if( !unitRecno )
		unitRecno = hire_unit(SKILL_SPYING, spyRaceId, targetXLoc, targetYLoc);

	//--- if cannot hire one, try to train one ----//

	int trainTownRecno=0;

	if( !unitRecno )
		unitRecno = train_unit(SKILL_SPYING, spyRaceId, targetXLoc, targetYLoc, trainTownRecno);

	if( !unitRecno )
		return 0;

	//------ get the spy object of the unit ------//

	Unit* unitPtr = unit_array[unitRecno];

	err_when( !unitPtr->spy_recno );

	spyPtr = spy_array[unitPtr->spy_recno];

	//------- get the nation of the assign destination -----//

	Location* locPtr = world.get_loc(targetXLoc, targetYLoc);
	int 		 cloakedNationRecno;

	if( locPtr->is_firm() )
	{
		Firm* firmPtr = firm_array[locPtr->firm_recno()];

		err_when( firmPtr->nation_recno==0 );		// cannot assign to a monster firm

		cloakedNationRecno = firmPtr->nation_recno;
	}
	else if( locPtr->is_town() )
	{
		Town* townPtr = town_array[locPtr->town_recno()];

		cloakedNationRecno = townPtr->nation_recno;
	}
	else
	{
		return 0;		// the original firm or town has been destroyed or sold
	}

	//------- Add the assign spy action --------//

	int actionRecno = add_action( targetXLoc, targetYLoc,
							-1, -1, ACTION_AI_ASSIGN_SPY, cloakedNationRecno, 1, unitRecno );

	if( !actionRecno )
		return 0;

	//------ if the unit is under training ------//

	if( trainTownRecno )
		town_array[trainTownRecno]->train_unit_action_id = get_action(actionRecno)->action_id;

	return 1;
}
//---------- End of function Nation::ai_assign_spy --------//


//--------- Begin of function Nation::ai_find_spy --------//
//
// Try to locate an existing spy for use.
//
// <int> targetXLoc, targetYLoc - the target location
// [int] spyRaceId			  - if specified, only spies of this race
//											 will be located. (default:0)
// [int] mobileOnly 				  - get mobile spies only. (default:0)
//
Spy* Nation::ai_find_spy(int targetXLoc, int targetYLoc, int spyRaceId, int mobileOnly)
{
	//--- first check if we have an existing spy ready for the mission ---//

	Spy  *spyPtr, *bestSpy=NULL;
	int  curRating, bestRating=0;
	int  spyXLoc, spyYLoc;
	int  targetRegionId = world.get_region_id(targetXLoc, targetYLoc);

	for(int i=spy_array.size(); i>0; i--)
	{
		if(spy_array.is_deleted(i))
			continue;

		spyPtr = spy_array[i];

		if( !spyPtr->true_nation_recno != nation_recno )
			continue;

		if( spyRaceId && spyRaceId != race_id )
			continue;

		if( spyPtr->spy_place == SPY_MOBILE )
		{
			Unit* unitPtr = unit_array[spyPtr->spy_place_para];

			if( !unitPtr->is_ai_all_stop() )
				continue;

			spyXLoc = unitPtr->next_x_loc();
			spyYLoc = unitPtr->next_y_loc();
		}
		else
		{
			if( mobileOnly )
				continue;

			if( spyPtr->spy_place == SPY_FIRM )
			{
				Firm* firmPtr = firm_array[spyPtr->spy_place_para];

				if( firmPtr->nation_recno != nation_recno )		// only get spies from our own firms
					continue;

				spyXLoc = firmPtr->center_x;
				spyYLoc = firmPtr->center_y;
			}
			else if( spyPtr->spy_place == SPY_TOWN )
			{
				Town* townPtr = town_array[spyPtr->spy_place_para];

				if( townPtr->nation_recno != nation_recno )		// only get spies from our own towns
					continue;

				spyXLoc = townPtr->center_x;
				spyYLoc = townPtr->center_y;
			}
			else
				continue;		// in ships or undefined
		}

		//--- check if the region ids are matched ---//

		if( world.get_region_id(spyXLoc, spyYLoc) != targetRegionId )
			continue;

		//----------------------------------------//

		curRating  = world.distance_rating(targetXLoc, targetYLoc, spyXLoc, spyYLoc);
		curRating += spyPtr->spy_skill + spyPtr->spy_loyalty/2;

		if( curRating > bestRating )
		{
			bestRating = curRating;
			bestSpy    = spyPtr;
		}
	}

	return bestSpy;
}
//---------- End of function Nation::ai_find_spy --------//


//----- Begin of function Nation::ai_assign_spy -----//
//
// action_x_loc, action_y_loc - location of the target firm or town
// ref_x_loc,    ref_y_loc    - not used
// unit_recno 					   - unit recno of the spy
// action_para						- the cloak nation recno the spy should set to.
//
int Nation::ai_assign_spy(ActionNode* actionNode)
{
	if(!seek_path.total_node_avail)
		return 0;

	if( unit_array.is_deleted(actionNode->unit_recno) )
		return -1;

	Unit* spyUnit = unit_array[actionNode->unit_recno];

	if( !spyUnit->is_visible() )		// it's still under training, not available yet
		return -1;

	if( !spyUnit->spy_recno || spyUnit->true_nation_recno() != nation_recno )
		return -1;

	//------ change the cloak of the spy ------//

	int  newFlag;
	Spy* spyPtr = spy_array[spyUnit->spy_recno];

	if( reputation < 0 )		// if the nation's reputation is negative, use sneak mode to avoid chance of being uncovered and further damage the reputation
		newFlag = m.random( 2+(-(int)reputation)/5 )==0;	// 2 to 22
	else
		newFlag = m.random(2)==0;		// 50% chance of being 1

	spyPtr->notify_cloaked_nation_flag = newFlag;

	if( !spyUnit->can_spy_change_nation() )		// if the spy can't change nation recno now
	{
   	int destXLoc = spyUnit->next_x_loc() + m.random(20) - 10;
		int destYLoc = spyUnit->next_y_loc() + m.random(20) - 10;

		destXLoc = MAX(0, destXLoc);
		destXLoc = MIN(MAX_WORLD_X_LOC-1, destXLoc);

		destYLoc = MAX(0, destYLoc);
		destYLoc = MIN(MAX_WORLD_Y_LOC-1, destXLoc);

		spyUnit->move_to( destXLoc, destYLoc );

		actionNode->retry_count++;			// never give up
		return 0;								// return now and try again later
	}

	spyUnit->spy_change_nation(actionNode->action_para,COMMAND_AI);

	//------- assign the spy to the target -------//

	spyUnit->assign(actionNode->action_x_loc, actionNode->action_y_loc);

	//----------------------------------------------------------------//
	// Since the spy has already changed its cloaked nation recno
	// we cannot set the ai_action_id of the unit as when it needs
	// to call action_finished() or action_failure() it will
	// use the cloaked nation recno, which is incorrect.
	// So we just return -1, noting that the action has been completed.
	//----------------------------------------------------------------//

	return -1;
}
//----- End of function Nation::ai_assign_spy -----//


//-------- Begin of function Nation::think_assign_spy_target_camp --------//
//
// Think about planting spies into independent towns and enemy towns.
//
int Nation::think_assign_spy_target_camp(int raceId, int regionId)
{
	Firm  *firmPtr;
	int	curRating, bestRating=0, bestFirmRecno=0;

	for( int firmRecno=firm_array.size() ; firmRecno>0 ; firmRecno-- )
	{
		if( firm_array.is_deleted(firmRecno) )
			continue;

		firmPtr = firm_array[firmRecno];

		if( firmPtr->nation_recno == nation_recno )		// don't assign to own firm
			continue;

		if( firmPtr->region_id != regionId )
			continue;

		if( firmPtr->overseer_recno == 0 ||
			 firmPtr->worker_count == MAX_WORKER )
		{
			continue;
		}

		if( firmPtr->majority_race() != raceId )
			continue;

		//---------------------------------//

		Unit* overseerUnit = unit_array[firmPtr->overseer_recno];

		if( overseerUnit->spy_recno )		// if the overseer is already a spy
			continue;

		curRating = 100 - overseerUnit->loyalty;

		if( curRating > bestRating )
		{
			bestRating 	  = curRating;
			bestFirmRecno = firmRecno;
		}
	}

	return bestFirmRecno;
}
//-------- End of function Nation::think_assign_spy_target_camp --------//


//-------- Begin of function Nation::think_assign_spy_target_town --------//
//
// Think about planting spies into independent towns and enemy towns.
//
int Nation::think_assign_spy_target_town(int raceId, int regionId)
{
	Town  *townPtr;
	int   townCount = town_array.size();
	int   townRecno = m.random(townCount)+1;

	for( int i=town_array.size() ; i>0 ; i-- )
	{
		if( ++townRecno > townCount )
			townRecno = 1;

		if( town_array.is_deleted(townRecno) )
			continue;

		townPtr = town_array[townRecno];

		if( townPtr->nation_recno == nation_recno )		// don't assign to own firm
			continue;

		if( townPtr->region_id != regionId )
			continue;

		if( townPtr->population > MAX_TOWN_POPULATION-5 )		// -5 so that even if we assign too many spies to a town at the same time, there will still room for them
			continue;

		//---- for player towns, don't assign too frequently ----//

		if( !townPtr->ai_town )
		{
			if( m.random(3) != 0 )
				continue;
		}

		//----------------------------------------//

		if( townPtr->nation_recno )
		{
			if( townPtr->race_loyalty_array[raceId-1] < MIN_NATION_DEFEND_LOYALTY )		// no need to assign spies to these towns as they are already very low
				continue;
		}
		else
		{
			if( townPtr->race_resistance_array[raceId-1][nation_recno-1] < MIN_INDEPENDENT_DEFEND_LOYALTY )		// no need to assign spies to these towns as they are already very low
				continue;
		}

		if( townPtr->majority_race() != raceId )
			continue;

		return townRecno;
	}

	return 0;
}
//-------- End of function Nation::think_assign_spy_target_town --------//


//-------- Begin of function Nation::think_assign_spy_own_town --------//
//
// Think about planting spies into independent towns and enemy towns.
//
int Nation::think_assign_spy_own_town(int raceId, int regionId)
{
	Town  *townPtr;
	int   townCount = town_array.size();
	int   townRecno = m.random(townCount)+1;
	int   spyCount;

	for( int i=town_array.size() ; i>0 ; i-- )
	{
		if( ++townRecno > townCount )
			townRecno = 1;

		if( town_array.is_deleted(townRecno) )
			continue;

		townPtr = town_array[townRecno];

		if( townPtr->nation_recno != nation_recno )		// only assign to own firm
			continue;

		if( townPtr->region_id != regionId )
			continue;

		if( townPtr->population > MAX_TOWN_POPULATION-5 )
			continue;

		if( townPtr->majority_race() != raceId )
			continue;

		int curSpyLevel    = spy_array.total_spy_skill_level( SPY_TOWN, townRecno, nation_recno, spyCount );
		int neededSpyLevel = townPtr->needed_anti_spy_level();

		if( neededSpyLevel > curSpyLevel + 30 )
			return townRecno;
	}

	return 0;
}
//-------- End of function Nation::think_assign_spy_own_town --------//
