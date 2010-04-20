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

//Filename   : OAI_ACT.CPP
//Description: AI - building firms

#include <ALL.h>
#include <OUNIT.h>
#include <OF_CAMP.h>
#include <OREGIONS.h>
#include <OCONFIG.h>
#include <OSITE.h>
#include <ONATION.h>


//--------- Begin of function Nation::think_build_firm --------//

void Nation::think_build_firm()
{
	if( !ai_should_build_mine() )
		return;

	//----- think building mine --------//

	if( think_build_mine() )
		return;

	//---- if there is a mine action currently -----//

	think_destroy_raw_site_guard();
}
//---------- End of function Nation::think_build_firm --------//


//--------- Begin of function Nation::think_build_mine --------//
//
int Nation::think_build_mine()
{
	//------- queue to build the new mine -------//

	short xLoc, yLoc, refXLoc, refYLoc;

	int rawId = seek_mine(xLoc, yLoc, refXLoc, refYLoc);	//reference location is the raw material location

	if( !rawId )
		return 0;

	//--- if we have a mine producing that raw type already ---//

	if( raw_count_array[rawId-1] > 0 )
	{
		if( !ai_should_spend(20) )		// then it's not important to build it
			return 0;
	}

	//-------------------------------------------//
	// If the map is set to unexplored, wait for a
	// reasonable amount of time before moving out
	// to build the mine.
	//-------------------------------------------//

	if( !config.explore_whole_map )
	{
		int i;
		for( i=0 ; i<ai_town_count ; i++ )
		{
			Town* townPtr = town_array[ ai_town_array[i] ];

			int rawDistance = m.points_distance(xLoc, yLoc, townPtr->center_x, townPtr->center_y);

			if( info.game_date-info.game_start_date >
				 rawDistance * (5-config.ai_aggressiveness) / 5 )		// 3 to 5 / 5
			{
				break;
			}
		}

		if( i==ai_town_count )
			return 0;
	}

	return add_action(xLoc, yLoc, refXLoc, refYLoc, ACTION_AI_BUILD_FIRM, FIRM_MINE);
}
//---------- End of function Nation::think_build_mine --------//


//--------- Begin of function Nation::ai_should_build_mine --------//
//
int Nation::ai_should_build_mine()
{
	//---- only build mines when it has enough population ----//

	if( total_jobless_population < (100-pref_economic_development)/2 )
		return 0;

	if( total_jobless_population < 16 )		// only build mine when you have enough population to support the economic chain: mine + factory + camp
		return 0;

	if( site_array.untapped_raw_count==0 )
		return 0;

	if( !can_ai_build(FIRM_MINE) )
		return 0;

	//--- don't build additional mines unless we have enough population and demand to support it ----//

	if( ai_mine_count == 1 )
	{
		if( true_profit_365days() < 0 && total_population < 40+pref_economic_development/5 )
			return 0;
	}

	//-- if the nation is already in the process of building a new one --//

	if( is_action_exist( ACTION_AI_BUILD_FIRM, FIRM_MINE ) )
		return 0;

	//--- if the population is low, make sure existing mines are in full production before building a new one ---//

	if( total_jobless_population < 30 )
	{
		Firm* firmPtr;

		for( int i=0 ; i<ai_mine_count ; i++ )
		{
			firmPtr = firm_array[ ai_mine_array[i] ];

			if( firmPtr->worker_count < MAX_WORKER )
				return 0;

			if( firmPtr->linked_firm_count==0 && !firmPtr->no_neighbor_space ) 	// if the firm does not have any linked firms, that means the firm is still not in full operation
				return 0;
		}
	}

	return 1;
}
//---------- End of function Nation::ai_should_build_mine --------//


//--------- Begin of function Nation::ai_attack_unit_in_area --------//
//
// AI attacks all units that are not friendly to us within the given area.
//
// <int> xLoc1, yLoc1, xLoc2, yLoc2 - coordination of the area.
//
void Nation::ai_attack_unit_in_area(int xLoc1, int yLoc1, int xLoc2, int yLoc2)
{
	int		 enemyXLoc, enemyYLoc, enemyCombatLevel=0;
	int		 enemyStatus = NATION_FRIENDLY;
	Location* locPtr;
	Unit* 	 unitPtr;

	//--------------------------------------------------//

	for( int yLoc=yLoc1 ; yLoc<=yLoc2 ; yLoc++ )
	{
		for( int xLoc=xLoc1 ; xLoc<=xLoc2 ; xLoc++ )
		{
			locPtr = world.get_loc(xLoc, yLoc);

			if( !locPtr->has_unit(UNIT_LAND) )
				continue;

			unitPtr = unit_array[ locPtr->unit_recno(UNIT_LAND) ];

			//--- if there is an idle unit on the mine building site ---//

			if( unitPtr->cur_action != SPRITE_IDLE || unitPtr->nation_recno==0 )
				continue;

			//----- if this is our spy cloaked in another nation, reveal its true identity -----//

			if( unitPtr->nation_recno != nation_recno &&
				 unitPtr->true_nation_recno() == nation_recno )
			{
				unitPtr->spy_change_nation(nation_recno, COMMAND_AI);
			}

			//--- if this is our own unit, order him to stay out of the building site ---//

			if( unitPtr->nation_recno == nation_recno )
			{
				unitPtr->think_normal_human_action();		// send the unit to a firm or a town
			}
			else //--- if it is an enemy unit, attack it ------//
			{
				int nationStatus = get_relation_status(unitPtr->nation_recno);

				if( nationStatus < enemyStatus )		// if the status is worse than the current target
				{
					enemyXLoc = xLoc;
					enemyYLoc = yLoc;
					enemyStatus = nationStatus;
					enemyCombatLevel += (int) unitPtr->unit_power();
				}
			}
		}
	}

	//--- if there are enemies on our firm building site, attack them ---//

	if( enemyCombatLevel )
	{
		ai_attack_target( enemyXLoc, enemyYLoc, enemyCombatLevel );
	}
}
//---------- End of function Nation::ai_attack_unit_in_area --------//


//--------- Begin of function Nation::think_destroy_raw_site_guard --------//
//
int Nation::think_destroy_raw_site_guard()
{
	Site* 	 sitePtr;
	Location* locPtr;
	Unit* 	 unitPtr;

	for( int i=site_array.size() ; i>0 ; i-- )
	{
		if( site_array.is_deleted(i) )
			continue;

		sitePtr = site_array[i];

		//--- if there is already a mine built on this raw site ---//

		if( sitePtr->has_mine )
			continue;

		//----- if there is a unit standing on this site -----//

		locPtr = world.get_loc( sitePtr->map_x_loc, sitePtr->map_y_loc );

		if( !locPtr->has_unit(UNIT_LAND) )
			continue;

		unitPtr = unit_array[ locPtr->unit_recno(UNIT_LAND) ];

		if( unitPtr->cur_action != SPRITE_IDLE )		// only attack if this unit is idle
			continue;

		if( unitPtr->nation_recno == nation_recno )	// don't attack our own units
			continue;

		//------ check if we have a presence in this region ----//

		// ####### patch begin Gilbert 16/3 ########//
		//if( region_array.get_region_stat(sitePtr->region_id)->base_town_nation_count_array[nation_recno-1] == 0 )
		//	continue;
		if( base_town_count_in_region(sitePtr->region_id) == 0 )
			continue;
		// ####### patch end Gilbert 16/3 ########//

		//------ check the relationship with this unit ------//
		//
		// If we are friendly with this nation, don't attack it.
		//
		//---------------------------------------------------//

		if( get_relation_status(unitPtr->nation_recno) >= NATION_FRIENDLY )
			continue;

		//--------- attack the enemy unit ---------//

		int hasWar;
		int enemyCombatLevel = mobile_defense_combat_level( sitePtr->map_x_loc,
									  sitePtr->map_y_loc, unitPtr->nation_recno, 1, hasWar );

		if( enemyCombatLevel == - 1 )		// a war is going on here, don't attack this target
			continue;

		if( ai_attack_target(sitePtr->map_x_loc, sitePtr->map_y_loc, enemyCombatLevel, 0, 0, 0, 0, 1) )		// 1-use all camps
			return 1;
	}

	return 0;
}
//---------- End of function Nation::think_destroy_raw_site_guard --------//


//--------- Begin of function Nation::ai_supported_inn_count --------//
//
// Return the number of inns this nation can support.
//
int Nation::ai_supported_inn_count()
{
	float fixedExpense = fixed_expense_365days();

	int innCount = int( cash / 5000 * (100+pref_hire_unit) / 100 );

	innCount = MIN(3, innCount);		// maximum 3 inns, minimum 1 inn.

	return MAX(1, innCount);
}
//---------- End of function Nation::ai_supported_inn_count --------//


//--------- Begin of function Nation::ai_has_should_close_camp --------//
//
// Return whether the this nation has any camps that should be closed.
//
// <int> regionId - only camps in this region are counted.
//
int Nation::ai_has_should_close_camp(int regionId)
{
	//--- if this nation has some firms going to be closed ---//

	if( firm_should_close_array[FIRM_CAMP-1] > 0 )
	{
		//--- check if any of them are in the same region as the current town ---//

		for( int i=ai_camp_count-1 ; i>=0 ; i-- )
		{
			FirmCamp* firmCamp = (FirmCamp*) firm_array[ ai_camp_array[i] ];

			if( firmCamp->should_close_flag && firmCamp->region_id == regionId )
			{
				return 1;
			}
		}
	}

	return 0;
}
//---------- End of function Nation::ai_has_should_close_camp --------//
