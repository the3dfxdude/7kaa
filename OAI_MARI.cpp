//Filename   : OAI_MARI.CPP
//Description: AI functions on sea exploration, trading

#include <OTOWN.H>
#include <OREGIONS.H>
#include <OU_MARI.H>
#include <OUNITRES.H>
#include <OSITE.H>
#include <OF_HARB.H>
#include <OF_CAMP.H>
#include <ONATION.H>

//--------- Begin of function Nation::think_marine --------//
//
void Nation::think_marine()
{
	if( pref_use_marine < 50 )		// don't use marine at all
		return;

	if( !ai_should_spend(20+pref_use_marine/2) )		// 20 to 70 importance rating
		return;

	//--- think over building harbor network ---//

	think_build_harbor_network();

	if( ai_harbor_count == 0 )
		return;

	//------ think about sea attack enemies -------//

	if( m.random(3)==0 )				// 33% chance
	{
		if( think_sea_attack_enemy() )
			return;
	}

	//---- check if it is safe for sea travel now ----//

	if( !ai_is_sea_travel_safe() )
		return;

	//----- think over moving between regions -----//

	think_move_between_region();

//	think_move_to_region_with_mine();
}
//---------- End of function Nation::think_marine --------//


//----- Begin of function Nation::think_build_harbor_network ----//
//
// Think about thinking a harbor network so that we have harbors
// from every region to any other regions.
//
int Nation::think_build_harbor_network()
{
	//--- only build one harbor at a time, to avoid double building ---//

	if( is_action_exist( ACTION_AI_BUILD_FIRM, FIRM_HARBOR ) )
		return 0;

	//--------------------------------------------//

	RegionStat* regionStat = region_array.region_stat_array;
	RegionPath* regionPath;

	for( int i=0 ; i<region_array.region_stat_count ; i++, regionStat++ )
	{
		//--- only build on those regions that this nation has base towns ---//

		if( !regionStat->base_town_nation_count_array[nation_recno-1] )
			continue;

		if( regionStat->harbor_nation_count_array[nation_recno-1] > 0 )		// if we already have a harbor in this region
			continue;

		err_when( regionStat->harbor_nation_count_array[nation_recno-1] > 1 );		// this shouldn't happen if the AI works properly

		//-----------------------------------------------------------------------//
		//
		// Scan thru all regions which this region can be connected to thru sea.
		// If one of them is worth our landing, then builld a harbor in this 
		// region so we can sail to that region. 
		//
		//-----------------------------------------------------------------------//

		regionPath = regionStat->reachable_region_array;

		for( int j=0 ; j<regionStat->reachable_region_count ; j++, regionPath++ )
		{
			err_when( regionPath->land_region_stat_id == i+1 );		// pointing back to its own

			//--------------------------------------//

			if( ai_harbor_count == 0 &&		// if we have already built one harbor, then we should continue to build others asa single harbor isn't useful
				 ai_should_sail_to_rating(regionPath->land_region_stat_id) <= 0 )
			{
				continue;
			}

			//--------- build a harbor now ---------//

			if( ai_build_harbor( regionStat->region_id, regionPath->sea_region_id ) )
				return 1;
		}
	}

	return 0;
}
//----- End of function Nation::think_build_harbor_network ----//


//----- Begin of function Nation::ai_should_sail_to_rating ----//
//
int Nation::ai_should_sail_to_rating(int regionStatId)
{
	RegionStat* regionStat = region_array.get_region_stat2(regionStatId);

	int curRating;

	curRating = regionStat->raw_count * 100
					+ regionStat->independent_town_count * 20
					+ regionStat->nation_presence_count * 30;
/*
					- (regionStat->total_town_count - regionStat->town_nation_count_array[nation_recno-1] ) * 10 	// towns of other nations
					- (regionStat->total_firm_count - regionStat->firm_nation_count_array[nation_recno-1] ) * 5		// firms of other nations
					- (regionStat->total_unit_count - regionStat->unit_nation_count_array[nation_recno-1] ) * 2		// units of other nations
					- regionStat->independent_unit_count * 2;				// monsters or rebel units
*/
	return curRating > 0;
}
//----- End of function Nation::ai_should_sail_to_rating ----//


//--------- Begin of function Nation::ai_build_harbor --------//
//
// Build a harbor across the given land and sea region id.
//
// <int> landRegionId - the land region id.
// <int> seaRegionId - the sea region id.
//
// return: <int> 1 - a suitable location is found and the
//						   building action has been queued.
//					  0 - not suitable location is found.
//
int Nation::ai_build_harbor(int landRegionId, int seaRegionId)
{
	#define ADEQUATE_ENEMY_HARBOR_DISTANCE   10

	//---- randomly pick a base town of this nation ----//

	Town* townPtr;
	int 	townSeq = m.random(ai_town_count);

	for( int i=0 ; i<ai_town_count ; i++ )
	{
		if( ++townSeq >= ai_town_count )
			townSeq=0;

		townPtr = town_array[ ai_town_array[townSeq] ];

		if( townPtr->is_base_town && landRegionId==townPtr->region_id )
			break;
	}

	if( i==ai_town_count )		// not found
		return 0;

	int homeXLoc = townPtr->center_x;
	int homeYLoc = townPtr->center_y;

	//---- scan out from the town and find the nearest suitable location to build the harbor ----//

	int		 xOffset, yOffset;
	int		 xLoc, yLoc, bestXLoc= -1, bestYLoc= -1, maxEnemyDistance=0;
	Location* locPtr;

	for( i=2 ; i<MAX_WORLD_X_LOC*MAX_WORLD_Y_LOC ; i++ )
	{
		m.cal_move_around_a_point(i, MAX_WORLD_X_LOC, MAX_WORLD_Y_LOC, xOffset, yOffset);

		xLoc = homeXLoc + xOffset;
		yLoc = homeYLoc + yOffset;

		xLoc = max(0, xLoc);
		xLoc = min(MAX_WORLD_X_LOC-1, xLoc);

		yLoc = max(0, yLoc);
		yLoc = min(MAX_WORLD_Y_LOC-1, yLoc);

		locPtr = world.get_loc(xLoc, yLoc);

		if( !locPtr->can_build_whole_harbor() )
			continue;

		if( !world.is_adjacent_region(xLoc, yLoc, seaRegionId) )
			continue;

		if( !world.can_build_firm(xLoc, yLoc, FIRM_HARBOR) )
			continue;

		//--------------------------------------//

		int enemyDistance = closest_enemy_firm_distance(FIRM_HARBOR, xLoc, yLoc);

		if( enemyDistance > maxEnemyDistance )
		{
			maxEnemyDistance = enemyDistance;
			bestXLoc = xLoc;
			bestYLoc = yLoc;

			if( enemyDistance >= ADEQUATE_ENEMY_HARBOR_DISTANCE )
				break;
		}
	}

	//--------------------------------//

	if( bestXLoc >= 0 )
	{
		add_action(xLoc, yLoc, homeXLoc, homeYLoc, ACTION_AI_BUILD_FIRM, FIRM_HARBOR);
		return 1;
	}

	return 0;
}
//---------- End of function Nation::ai_build_harbor --------//


//--------- Begin of function Nation::closest_enemy_firm_distance --------//
//
// Return how close is the cloeset enemy harbor to the given location.
//
// <int> firmId      - firm id.
// <int> xLoc, yLoc  - the given location
//
int Nation::closest_enemy_firm_distance(int firmId, int xLoc, int yLoc)
{
	int curDistance, minDistance=0x7FFF;

	for( int i=firm_array.size() ; i>0 ; i-- )
	{
		if( firm_array.is_deleted(i) )
			continue;

		Firm* firmPtr = firm_array[i];

		if( firmPtr->firm_id != firmId ||
			 firmPtr->nation_recno == nation_recno )		// belonging to own nation, not enemy nation
		{
			continue;
		}

		curDistance = m.points_distance(firmPtr->center_x, firmPtr->center_y, xLoc, yLoc);

		if( curDistance < minDistance )
			minDistance = curDistance;
	}

	return minDistance;
}
//---------- End of function Nation::closest_enemy_firm_distance --------//


//------ Begin of function Nation::think_move_between_region ------//
//
int Nation::think_move_between_region()
{
	if( think_move_people_between_region() )
		return 1;

	if( think_move_troop_between_region() )
		return 1;

	return 0;
}
//------ End of function Nation::think_move_between_region -------//


//------ Begin of function Nation::think_move_troop_between_region ------//
//
// Thing about moving units between regions
//
int Nation::think_move_troop_between_region()
{
	//----- find the region with the least population -----//

	int campCount, maxCampCount=0, minCampCount=0x1000;
	int maxRegionId=0, minRegionId=0;
	RegionStat* regionStat = region_array.region_stat_array;
	int curRating, minRegionRating=0;

	for( int i=0 ; i<region_array.region_stat_count ; i++, regionStat++ )
	{
		if( regionStat->nation_presence_count==0 &&
			 regionStat->independent_town_count==0 &&
			 regionStat->raw_count==0 )
		{
			continue;
		}

		campCount = regionStat->camp_nation_count_array[nation_recno-1];

		if( campCount > maxCampCount )
		{
			maxCampCount = campCount;
			maxRegionId   = regionStat->region_id;
		}

		if( campCount <= minCampCount )
		{
			curRating = ai_should_sail_to_rating(i+1);

			if( campCount < minCampCount || curRating >= minRegionRating )
			{
				minCampCount 	 = campCount;
				minRegionId  	 = regionStat->region_id;
				minRegionRating = curRating;
			}
		}
	}

	if( !maxRegionId || !minRegionId || maxRegionId==minRegionId )
		return 0;

	//----- only move if the difference is big enough ------//

	int minJoblessPop = region_array.get_region_stat(minRegionId)->nation_jobless_population_array[nation_recno-1];
	int maxJoblessPop = region_array.get_region_stat(maxRegionId)->nation_jobless_population_array[nation_recno-1];

	if( pref_use_marine < 90 )		// if > 90, it will ignore all these and move anyway  
	{
		if( minCampCount==0 )
		{
			if( maxJoblessPop - minJoblessPop < 200 - pref_use_marine )  // 150 to 200 (pref_use_marine is always >= 50, if it is < 50, marine functions are not called at all
				return 0;
		}
		else
		{
			if( maxJoblessPop - minJoblessPop < 150 - pref_use_marine )  // 100 to 150 (pref_use_marine is always >= 50, if it is < 50, marine functions are not called at all
				return 0;
		}
	}
	else
	{
		if( maxJoblessPop < 20 )		// don't move if we only have a few jobless people
			return 0;
	}

	//------------ see if we have any camps in the region -----------//

	int   destRegionId = minRegionId;
	Firm* firmPtr;

	for( i=ai_camp_count-1 ; i>=0 ; i-- )
	{
		firmPtr = firm_array[ai_camp_array[i]];

		if( firmPtr->region_id == destRegionId &&
			 !firmPtr->under_construction )				// if it's under construction there may be unit waiting outside of the camp
		{
			//--- if there is one, must move the troop close to it ---//

			return ai_patrol_to_region(firmPtr->center_x, firmPtr->center_y, SEA_ACTION_NONE);
		}
	}
	//----- if we don't have any camps in the region, build one ----//

	int 		 xLoc=0, yLoc=0;
	FirmInfo* firmInfo = firm_res[FIRM_CAMP];

	if(world.locate_space_random(xLoc, yLoc, MAX_WORLD_X_LOC-1,
		MAX_WORLD_Y_LOC-1, firmInfo->loc_width, firmInfo->loc_height,
		MAX_WORLD_X_LOC*MAX_WORLD_Y_LOC, destRegionId, 1))
	{
		return ai_patrol_to_region(xLoc, yLoc, SEA_ACTION_BUILD_CAMP);
	}

	return 0;
}
//------ End of function Nation::think_move_troop_between_region -------//


//------ Begin of function Nation::think_move_people_between_region ------//
//
// Thing about moving units between regions
//
int Nation::think_move_people_between_region()
{
	//----- find the region with the least population -----//

	int joblessPop, maxJoblessPop=0, minJoblessPop=0x1000;
	int maxRegionId=0, minRegionId=0;
	RegionStat* regionStat = region_array.region_stat_array;

	for( int i=0 ; i<region_array.region_stat_count ; i++, regionStat++ )
	{
		//--- only move to regions in which we have camps ---//

		if( regionStat->camp_nation_count_array[nation_recno-1] == 0 )
			continue;

		joblessPop = regionStat->nation_jobless_population_array[nation_recno-1];

		if( joblessPop > maxJoblessPop )
		{
			maxJoblessPop = joblessPop;
			maxRegionId   = regionStat->region_id;
		}

		if( joblessPop < minJoblessPop )
		{
			minJoblessPop = joblessPop;
			minRegionId   = regionStat->region_id;
		}
	}

	if( !maxRegionId || !minRegionId || maxRegionId==minRegionId )
		return 0;

	//----- only move if the difference is big enough ------//

	if( pref_use_marine < 90 )		// if > 90, it will ignore all these and move anyway
	{
		if( maxJoblessPop - minJoblessPop < 150 - pref_use_marine )	 // 100 to 150 (pref_use_marine is always >= 50, if it is < 50, marine functions are not called at all
			return 0;
	}
	else
	{
		if( maxJoblessPop < 20 )	// don't move if we only have a few jobless people
			return 0;
	}

	//------------ see if we have any towns in the region -----------//

	int   destRegionId = minRegionId;
	Town* townPtr;

	for( i=ai_town_count-1 ; i>=0 ; i-- )
	{
		townPtr = town_array[ai_town_array[i]];

		if( townPtr->region_id == destRegionId )
		{
			//--- if there is one, must move the people to it ---//

			return ai_settle_to_region(townPtr->center_x, townPtr->center_y, SEA_ACTION_NONE);
		}
	}

	//----- if we don't have any towns in the region, settle one ----//

	int xLoc=0, yLoc=0;

	if(world.locate_space_random(xLoc, yLoc, MAX_WORLD_X_LOC-1,
		MAX_WORLD_Y_LOC-1, STD_TOWN_LOC_WIDTH, STD_TOWN_LOC_HEIGHT,
		MAX_WORLD_X_LOC*MAX_WORLD_Y_LOC, destRegionId, 1))
	{
		return ai_settle_to_region(xLoc, yLoc, SEA_ACTION_SETTLE);
	}

	return 0;
}
//------ End of function Nation::think_move_people_between_region -------//


//------ Begin of function Nation::ai_is_sea_travel_safe ------//
//
// return: <int> 1 - it's safe for sea travel
//					  0 - it's not safe for sea travel
//
int Nation::ai_is_sea_travel_safe()
{
	//--- count the no. of battle ships owned by each nation ---//

	Unit* unitPtr;
	short nationShipCountArray[MAX_NATION];

	memset( nationShipCountArray, 0, sizeof(nationShipCountArray) );

	for( int i=unit_array.size() ; i>0 ; i-- )
	{
		if( unit_array.is_deleted(i) )
			continue;

		unitPtr = unit_array[i];

		if( unitPtr->unit_id != UNIT_CARAVEL &&
			 unitPtr->unit_id != UNIT_GALLEON )
		{
			continue;
		}

		err_when( unitPtr->nation_recno < 1 || unitPtr->nation_recno > MAX_NATION );

		nationShipCountArray[unitPtr->nation_recno-1]++;
	}

	//--- compare the no. of ships of ours and those of the human players ---//

	int ourBattleShipCount = nationShipCountArray[nation_recno-1];
	int nationRecno = m.random(nation_array.size())+1;

	for( i=nation_array.size() ; i>0 ; i-- )
	{
		if( ++nationRecno > nation_array.size() )
			nationRecno = 1;

		if( nation_array.is_deleted(nationRecno) )
			continue;

		if( get_relation(nationRecno)->status != NATION_HOSTILE )		// only check enemies
			continue;

		if( nation_array[nationRecno]->is_ai() )		// only check human players
			continue;

		//-- if enemy has battle ships, it is not safe for sea travel, destroy them first ---//

		if( nationShipCountArray[nationRecno-1] > 0 )
		{
			//--- if enemy ships significantly outnumber ours, don't do any sea travel ---//

			if( nationShipCountArray[nationRecno-1] - ourBattleShipCount >
				 pref_military_courage/3  )		// 0 to 3
			{
				return 0;
			}
		}
	}

	return 1;
}
//----- End of function Nation::ai_is_sea_travel_safe -----//


//------ Begin of function Nation::max_human_battle_ship_count ------//
//
// return: <int> the number of ships owned by the human player who
//					  is strongest on sea power.
//
int Nation::max_human_battle_ship_count()
{
	//--- count the no. of battle ships owned by each nation ---//

	Unit* unitPtr;
	short nationShipCountArray[MAX_NATION];

	memset( nationShipCountArray, 0, sizeof(nationShipCountArray) );

	for( int i=unit_array.size() ; i>0 ; i-- )
	{
		if( unit_array.is_deleted(i) )
			continue;

		unitPtr = unit_array[i];

		if( unitPtr->unit_id != UNIT_CARAVEL &&
			 unitPtr->unit_id != UNIT_GALLEON )
		{
			continue;
		}

		err_when( unitPtr->nation_recno < 1 || unitPtr->nation_recno > MAX_NATION );

		nationShipCountArray[unitPtr->nation_recno-1]++;
	}

	//--- compare the no. of ships of ours and those of the human players ---//

	int maxShipCount=0;

	for( i=nation_array.size() ; i>0 ; i-- )
	{
		if( nation_array.is_deleted(i) )
			continue;

		if( nation_array[i]->is_ai() )		// only check human players
			continue;

		//-- if enemy has battle ships, it is not safe for sea travel, destroy them first ---//

		if( nationShipCountArray[i-1] > maxShipCount )
		{
			maxShipCount = nationShipCountArray[i-1];
		}
	}

	return maxShipCount;
}
//----- End of function Nation::max_human_battle_ship_count -----//


//------ Begin of function Nation::think_sea_attack_enemy ------//
//
// Think about attacking enemy harbors and ships.
//
int Nation::think_sea_attack_enemy()
{
	if( total_ship_combat_level < 700 - (pref_military_courage + pref_use_marine)*2 )		// 300 to 700
		return 0;

	//-----------------------------------------//

	int 	totalFirm = firm_array.size();
	int 	firmRecno = m.random(totalFirm)+1;
	Firm* firmPtr;

	for( int i=0 ; i<totalFirm ; i++ )
	{
		if( ++firmRecno > totalFirm )
			firmRecno = 1;

		if( firm_array.is_deleted(firmRecno) )
			continue;

		firmPtr = firm_array[firmRecno];

		if( firmPtr->firm_id != FIRM_HARBOR )
			continue;

		if( get_relation_status(firmPtr->nation_recno) != NATION_HOSTILE )
			continue;

		//--- if the AI has more powerful fleets than the enemy ---//

		if( total_ship_combat_level >
			 nation_array[firmPtr->nation_recno]->total_ship_combat_level )
		{
			ai_sea_attack_target(firmPtr->center_x, firmPtr->center_y);
			return 1;
		}
	}

	return 0;
}
//----- End of function Nation::think_sea_attack_enemy -----//
