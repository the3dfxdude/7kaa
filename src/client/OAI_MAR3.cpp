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

//Filename   : OAI_MAR3.CPP
//Description: AI functions on sea exploration, trading

#include <OTOWN.h>
#include <OREGIONS.h>
#include <OU_MARI.h>
#include <OUNITRES.h>
#include <OSITE.h>
#include <OF_HARB.h>
#include <OF_CAMP.h>
#include <ONATION.h>


//------ Begin of function Nation::ai_settle_to_region ------//
//
// <int> destXLoc, destYLoc - the location of the destination to settle.
// <int> seaActionId			 - SEA_ACTION_???
//
int Nation::ai_settle_to_region(int destXLoc, int destYLoc, int seaActionId)
{
	#define SETTLE_REGION_UNIT_COUNT 	9		// no. of units to move to settle on a new region each time

	//---- think about which town to recruit the people -----//

	int  destRegionId = world.get_region_id(destXLoc, destYLoc);
	int  curRating, bestRating=0, seaRegionId;
	Town *townPtr, *bestTown=NULL;

	for( int i=0 ; i<ai_town_count ; i++ )
	{
		townPtr = town_array[ ai_town_array[i] ];

		if( townPtr->has_linked_own_camp==0 )		// if there is no command base linked to this town, we cannot recruit any peasants from it
			continue;

		if( townPtr->jobless_population < SETTLE_REGION_UNIT_COUNT )		// don't get peasant from this town if the jobless population is less than 20
			continue;

		//--- only send units from this region if we have a harbor in that region ---//

		// region_stat_id of a region may be zero
		if( 
			region_array[townPtr->region_id]->region_stat_id == 0 || 
			region_array.get_region_stat(townPtr->region_id)->harbor_nation_count_array[nation_recno-1] == 0 )
			continue;

		curRating  = world.distance_rating(destXLoc, destYLoc, townPtr->center_x, townPtr->center_y);

		curRating += townPtr->jobless_population;

		curRating += townPtr->average_loyalty();		// select a town with high loyalty

		if( curRating <= bestRating )
			continue;

		//------- see if we have ships ready currently -----//

		seaRegionId = region_array.get_sea_path_region_id(townPtr->region_id, destRegionId);

		if( !ai_find_transport_ship(seaRegionId, townPtr->center_x, townPtr->center_y, 0) )		// 0-don't have to find the best, return immediately whenever a suitable one is found 
			continue;

		bestRating = curRating;
		bestTown   = townPtr;
	}

	if( !bestTown )
		return 0;

	//------- try to recruit 9 units from one of our towns --------//

	short recruitedUnitArray[SETTLE_REGION_UNIT_COUNT];
	int recruitedCount=0;
	int raceId = bestTown->majority_race();
	int unitRecno;
	int loopCount=0;

	while( recruitedCount < SETTLE_REGION_UNIT_COUNT )
	{
		err_when( ++loopCount > 100 );

		if( bestTown->recruitable_race_pop( raceId, 1 ) )
		{
			unitRecno = bestTown->recruit(-1, raceId, COMMAND_AI);

			if( !unitRecno )		// no space for new unit 
				break;

			err_when( unit_array.is_deleted(unitRecno) );
			err_when( !unit_array[unitRecno]->is_visible() );

			recruitedUnitArray[recruitedCount++] = unitRecno;
		}
		else
		{
			raceId = bestTown->pick_random_race(0, 1);		// 0-recruitable only, 1-will also pick spies

			if( !raceId )
				break;
		}
	}

	//--- if due to some reasons that the no. of units recruited is less than half of what we need, do not continue to sea travel.

	if( recruitedCount < SETTLE_REGION_UNIT_COUNT/2 )
		return 0;

	int actionRecno = add_action( destXLoc, destYLoc, 0, 0, ACTION_AI_SEA_TRAVEL, seaActionId,
											recruitedCount, 0, 0, recruitedUnitArray );

	if( actionRecno )						// must process it immediately otherwise the recruited units will do something else
		return process_action(actionRecno);
	else
		return 0;
}
//------ End of function Nation::ai_settle_to_region -------//


//------ Begin of function Nation::ai_patrol_to_region ------//
//
// Look for a military camp for moving all of its soldiers to
// a new region and set up a new military camp to host the soldiers.
//
// <int> destXLoc, destYLoc - the location of the destination to set up
//										a military camp.
// <int> seaActionId			 - SEA_ACTION_???
//
int Nation::ai_patrol_to_region(int destXLoc, int destYLoc, int seaActionId)
{
	#define SETTLE_REGION_UNIT_COUNT 	9		// no. of units to move to settle on a new region each time

	//---- think about which town to recruit the people -----//

	int  		destRegionId = world.get_region_id(destXLoc, destYLoc);
	int  		curRating, bestRating=0, seaRegionId;
	int		kingRecno = nation_array[nation_recno]->king_unit_recno;
	FirmCamp *firmCamp, *bestCamp=NULL;

	for( int i=0 ; i<ai_camp_count ; i++ )
	{
		firmCamp = (FirmCamp*) firm_array[ ai_camp_array[i] ];

		if( !(firmCamp->overseer_recno && firmCamp->worker_count==MAX_WORKER) )		// only when the camp is filled with workers
			continue;

		if( firmCamp->ai_capture_town_recno )     // the base is trying to capture an independent town
			continue;

		if( firmCamp->is_attack_camp )
			continue;

		if( firmCamp->overseer_recno == kingRecno )		// if the king oversees this firm
			continue;

		//--- only send units from this region if we have a harbor in that region ---//

		if( 
			region_array[firmCamp->region_id]->region_stat_id == 0 ||
			region_array.get_region_stat(firmCamp->region_id)->harbor_nation_count_array[nation_recno-1] == 0 )
			continue;

		curRating  = world.distance_rating(destXLoc, destYLoc, firmCamp->center_x, firmCamp->center_y);

		if( curRating <= bestRating )
			continue;

		//------- see if we have ships ready currently -----//

		seaRegionId = region_array.get_sea_path_region_id(firmCamp->region_id, destRegionId);

		if( !ai_find_transport_ship(seaRegionId, firmCamp->center_x, firmCamp->center_y, 0) )		// 0-don't have to find the best, return immediately whenever a suitable one is found
			continue;

		bestRating = curRating;
		bestCamp   = firmCamp;
	}

	if( !bestCamp )
		return 0;

	//----- patrol the camp troop ajnd assign it to a ship -----//

	bestCamp->patrol();

	if( bestCamp->patrol_unit_count > 0 )     // there could be chances that there are no some for mobilizing the units
	{
		int actionRecno = add_action( destXLoc, destYLoc, 0, 0, ACTION_AI_SEA_TRAVEL, seaActionId,
												bestCamp->patrol_unit_count, 0, 0, bestCamp->patrol_unit_array );

		if( actionRecno )						// must process it immediately otherwise the recruited units will do something else
			return process_action(actionRecno);
	}

	return 0;
}
//------ End of function Nation::ai_patrol_to_region -------//



//------- Begin of function Nation::has_trade_ship -----------//
//
// Whether this nation already has a trade ship travelling between
// the two given harbors.
//
// return: <int> recno of the ship.
//					  0 - if there isn't any yet.
//
int Nation::has_trade_ship(int firmRecno1, int firmRecno2)
{
	UnitMarine* unitMarine;

	for( int i=ai_ship_count-1 ; i>=0 ; i-- )
	{
		unitMarine = (UnitMarine*) unit_array[ ai_ship_array[i] ];

		err_when( unit_res[ unitMarine->unit_id ]->unit_class != UNIT_CLASS_SHIP );

		if( unitMarine->stop_defined_num < 2 )
			continue;

		if( ( unitMarine->stop_array[0].firm_recno == firmRecno1 &&
				unitMarine->stop_array[1].firm_recno == firmRecno2 ) ||
			 ( unitMarine->stop_array[1].firm_recno == firmRecno1 &&
				unitMarine->stop_array[0].firm_recno == firmRecno2 ) )
		{
			return unitMarine->sprite_recno;
		}
	}

	return 0;
}
//--------- End of function Nation::has_trade_ship -----------//


//------ Begin of function Nation::think_move_to_region_with_mine ------//
//
// Think about moving to a region with mines and settle a town next to
// the mine.
//
int Nation::think_move_to_region_with_mine()
{
	if( total_jobless_population < 30 )
		return 0;

	//---------------------------------------------------//

	int curRating, bestRating=0, bestRegionId=0;
	RegionStat* regionStat = region_array.region_stat_array;

	int i;
	for( i=0 ; i<region_array.region_stat_count ; i++, regionStat++ )
	{
		if( regionStat->town_nation_count_array[nation_recno-1] > 0 )		// if we already have towns there
			continue;

		if( regionStat->raw_count==0 )
			continue;

		//-- if we have already build one camp there, just waiting for sending a few peasants there, then process it first --//

		if( regionStat->camp_nation_count_array[nation_recno-1] > 0 )
		{
			bestRegionId = regionStat->region_id;
			break;
		}

		//-----------------------------------------------//

		curRating = regionStat->raw_count*3 - regionStat->nation_presence_count;

		if( curRating > bestRating )
		{
			bestRating   = curRating;
			bestRegionId = regionStat->region_id;
		}
	}

	if( !bestRegionId )
		return 0;

	//----- select the raw site to acquire -----//

	Site* sitePtr;

	for( i=site_array.size() ; i>0 ; i-- )
	{
		if( site_array.is_deleted(i) )
			continue;

		sitePtr = site_array[i];

		if( sitePtr->region_id == bestRegionId )
			break;
	}

	if( i==0 )
		return 0;

	//----- decide the location of the settlement -----//

	return ai_build_camp_town_next_to( sitePtr->map_x_loc-1, sitePtr->map_y_loc-1,
												  sitePtr->map_x_loc+1, sitePtr->map_y_loc+1  );
}
//------ End of function Nation::think_move_to_region_with_mine -------//


//------ Begin of function Nation::ai_build_camp_town_next_to ------//
//
// Build a new camp and settle a new town next to the given location.
//
// <int> xLoc1, yLoc1, xLoc2, yLoc2 - the location that the new camp
//												  and town should be built next to.
//
int Nation::ai_build_camp_town_next_to(int xLoc1, int yLoc1, int xLoc2, int yLoc2)
{
	//---- first see if we already have a camp in the region ---//

	int regionId = world.get_region_id(xLoc1, yLoc1);

	if( region_array[regionId]->region_stat_id == 0)
		return 0;

	if( region_array.get_region_stat(regionId)->camp_nation_count_array[nation_recno-1] == 0 )
	{
		//--- if we don't have one yet, build one next to the destination ---//

		if( !world.locate_space( xLoc1, yLoc1, xLoc2, yLoc2,
									 3, 3, UNIT_LAND, regionId, 1 ) )		// 1-locating the space for building
		{
			return 0;
		}

		if( !world.can_build_firm( xLoc1, yLoc1, FIRM_CAMP ) )
			return 0;

		return ai_patrol_to_region(xLoc1, yLoc1, SEA_ACTION_BUILD_CAMP);
	}
	else //-- if there's already a camp there, then set people there to settle --//
	{
		FirmCamp* firmCamp;

		for( int i=0 ; i<ai_camp_count ; i++ )
		{
			firmCamp = (FirmCamp*) firm_array[ ai_camp_array[i] ];

			if( firmCamp->region_id != regionId )
				continue;

			xLoc1 = firmCamp->loc_x1;
			yLoc1 = firmCamp->loc_y1;
			xLoc2 = firmCamp->loc_x2;
			yLoc2 = firmCamp->loc_y2;

			if( world.locate_space( xLoc1, yLoc1, xLoc2, yLoc2,
				 STD_TOWN_LOC_WIDTH, STD_TOWN_LOC_HEIGHT, UNIT_LAND, regionId, 1 ) )		// 1-locating the space for building
			{
				if( world.can_build_town( xLoc1, yLoc1 ) )
					return ai_settle_to_region(xLoc1, yLoc1, SEA_ACTION_SETTLE);
			}
		}
	}

	return 0;
}
//------ End of function Nation::ai_build_camp_town_next_to -------//


//------ Begin of function Nation::ai_sea_attack_target ------//
//
// <int> targetXLoc, targetYLoc - the location of the target.
//
int Nation::ai_sea_attack_target(int targetXLoc, int targetYLoc)
{
	UnitMarine* unitMarine;
	int 		   targetRegionId = world.get_region_id(targetXLoc, targetYLoc);
	int			rc = 0;

	for( int i=0 ; i<ai_ship_count ; i++ )
	{
		unitMarine = (UnitMarine*) unit_array[ ai_ship_array[i] ];

		if( unitMarine->attack_count==0 )
			continue;

		if( !unitMarine->is_ai_all_stop() )
			continue;

		//----- if the ship is in the harbor now -----//

		if( unitMarine->unit_mode == UNIT_MODE_IN_HARBOR )
		{
			FirmHarbor*	firmHarbor = (FirmHarbor*) firm_array[unitMarine->unit_mode_para];

			if( firmHarbor->sea_region_id != targetRegionId )
				continue;

			firmHarbor->sail_ship(unitMarine->sprite_recno, COMMAND_AI);
		}

		if( !unitMarine->is_visible() )		// no space in the sea for placing the ship
			continue;

		if( unitMarine->region_id() != targetRegionId )
			continue;

		//------ order the ship to attack the target ------//

		unitMarine->attack_unit( targetXLoc, targetYLoc );
		rc = 1;
	}

	return rc;
}
//-------- End of function Nation::ai_sea_attack_target -------//
