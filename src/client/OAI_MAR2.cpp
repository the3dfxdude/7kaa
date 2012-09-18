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

//Filename   : OAI_MAR2.CPP
//Description: AI functions for processing AI marine actions

#include <OTOWN.h>
#include <OREGIONS.h>
#include <OU_MARI.h>
#include <OF_HARB.h>
#include <ONATION.h>

//-------- Begin of function Nation::ai_sea_travel -------//
//
// action_x_loc, action_y_loc - location of the destination
//
// group_unit_array - array of units that will do the sea travel
// instance_count - no. of units in the array
//
// action_para  - the action of units after they have arrived the region,
//					   it is one of the following:
//
// SEA_ACTION_SETTLE		 	  - settle into a town
//	SEA_ACTION_BUILD_CAMP 	  - build and assign to a firm
//	SEA_ACTION_ASSIGN_TO_FIRM - assign to a firm
//	SEA_ACTION_MOVE			  - just move to the destination
//								 	    (this include moving to the location of a battle field.)
//
//---------------------------------------------//
//
// Procedures:
// * 1. Locate a ship, build one if cannot locate any.
// * 2. Assign the units to the ship.
//   3. Move the ship to the destination region.
//   4. Units disembark on the coast.
//   5. Units move to the destination.
//
// This function deal with the 1st and 2nd procedures,
// when they are finished, action ACTION_AI_SEA_TRAVEL2
// will be added.
//
//---------------------------------------------//
//
int Nation::ai_sea_travel(ActionNode* actionNode)
{
	err_when( actionNode->instance_count < 1 ||
				 actionNode->instance_count > ActionNode::MAX_ACTION_GROUP_UNIT );

	Unit* unitPtr = unit_array[actionNode->group_unit_array[0]];

	err_when( unitPtr->nation_recno != nation_recno );
	err_when( !unitPtr->is_visible() );

	//---- figure out the sea region id which the ship should appear ----//

	int unitRegionId = world.get_region_id(unitPtr->next_x_loc(), unitPtr->next_y_loc());
	int destRegionId = world.get_region_id(actionNode->action_x_loc, actionNode->action_y_loc);

	int seaRegionId = region_array.get_sea_path_region_id(unitRegionId, destRegionId);

	//------- 1. try to locate a ship --------//

	int shipUnitRecno = ai_find_transport_ship(seaRegionId, unitPtr->next_x_loc(), unitPtr->next_y_loc());

	if( !shipUnitRecno )
		return -1;					// must return -1 instead of 0 as the action must be executed immediately otherwise the units will be assigned with other action and the unit list may no longer be valid

	//---- if this ship is in the harbor, sail it out ----//

	UnitMarine* unitMarine = (UnitMarine*) unit_array[shipUnitRecno];

	if( unitMarine->unit_mode == UNIT_MODE_IN_HARBOR )
	{
		FirmHarbor*	firmHarbor = (FirmHarbor*) firm_array[unitMarine->unit_mode_para];

		firmHarbor->sail_ship(unitMarine->sprite_recno, COMMAND_AI);
	}

	if( !unitMarine->is_visible() )		// no space in the sea for placing the ship 
		return -1;

	//------ 2. Assign the units to the ship -------//

	unitMarine->ai_action_id = actionNode->action_id;

	err_when( unit_res[unitMarine->unit_id]->unit_class != UNIT_CLASS_SHIP );

	// ##### patch begin Gilbert 5/8 #######//
	unit_array.assign_to_ship(unitMarine->next_x_loc(), unitMarine->next_y_loc(), 0, actionNode->group_unit_array, actionNode->instance_count, COMMAND_AI, unitMarine->sprite_recno );
	// ##### patch end Gilbert 5/8 #######//

	for( int i=0 ; i<actionNode->instance_count ; i++ )
		unit_array[ actionNode->group_unit_array[i] ]->ai_action_id = actionNode->action_id;

	actionNode->instance_count++;		// +1 for the ship

	actionNode->processing_instance_count = actionNode->instance_count-1;		// -1 because when we return 1, it will be increased by 1 automatically
	actionNode->action_para2 = 0;		// reset it, it is set in Nation::action_finished()

	return 1;
}
//-------- End of function Nation::ai_sea_travel -------//


//-------- Begin of function Nation::ai_sea_travel2 -------//
//
// action_x_loc, action_y_loc - location of the destination
// action_para2 					- the recno of the ship
//										  (this is set when the ship has moved to the beach,
//											the function responsible for setting this is Nation::action_finished() )
//
//---------------------------------------------//
//
// Procedures:
//   1. Locate a ship, build one if cannot locate any.
//   2. Assign the units to the ship.
// * 3. Move the ship to the destination region.
//   4. Units disembark on the coast.
//   5. Units move to the destination.
//
//---------------------------------------------//
//
int Nation::ai_sea_travel2(ActionNode* actionNode)
{
	if( unit_array.is_deleted(actionNode->action_para2) )
		return -1;

	UnitMarine* unitMarine = (UnitMarine*) unit_array[actionNode->action_para2];

	if( unit_res[unitMarine->unit_id]->unit_class != UNIT_CLASS_SHIP )
		return -1;

	if( unitMarine->nation_recno != nation_recno )
		return -1;

	//--------------------------------------------------------//

	int realDestXLoc, realDestYLoc;		// reference vars for returning vars. 

	unitMarine->ship_to_beach( actionNode->action_x_loc, actionNode->action_y_loc, realDestXLoc, realDestYLoc );		// the real destination the ship is moving towards.
	unitMarine->ai_action_id = actionNode->action_id;

	return 1;
}
//-------- End of function Nation::ai_sea_travel2 -------//


//-------- Begin of function Nation::ai_sea_travel3 -------//
//
// action_x_loc, action_y_loc - location of the destination
// action_para2 					- the recno of the ship
//
//---------------------------------------------//
//
// Procedures:
//   1. Locate a ship, build one if cannot locate any.
//   2. Assign the units to the ship.
//   3. Move the ship to the destination region.
// * 4. Units disembark on the coast.
// * 5. Units move to the destination.
//
//---------------------------------------------//
//
int Nation::ai_sea_travel3(ActionNode* actionNode)
{
	if( unit_array.is_deleted(actionNode->action_para2) )
		return -1;

	UnitMarine* unitMarine = (UnitMarine*) unit_array[actionNode->action_para2];

	if( unit_res[unitMarine->unit_id]->unit_class != UNIT_CLASS_SHIP )
		return -1;

	if( unitMarine->nation_recno != nation_recno )
		return -1;

	//-------- 4. Units disembark on the coast. -------//

	if( !unitMarine->can_unload_unit() )
		return 0;

	//--- make a copy of the recnos of the unit on the ship ---//

	short unitRecnoArray[MAX_UNIT_IN_SHIP];
	short unitCount;

	memcpy( unitRecnoArray, unitMarine->unit_recno_array, sizeof(unitRecnoArray) );
	unitCount = unitMarine->unit_count;

	unitMarine->unload_all_units(COMMAND_AI);		// unload all units now

	return 1;		// finish the action.

/*
	//---------- 5. Validate all units ----------//

	for( int i=unitCount-1 ; i>=0 ; i-- )
	{
		if( unit_array.is_deleted( unitRecnoArray[i] ) ||
			 unit_array[ unitRecnoArray[i] ]->nation_recno != nation_recno )
		{
			err_when( unitCount > MAX_UNIT_IN_SHIP );

			misc.del_array_rec( unitRecnoArray, unitCount, sizeof(unitRecnoArray[0]), i+1 );
			unitCount--;
		}
	}

	if( unitCount==0 )
		return -1;

	err_when( unitCount < 0 );

	//--- 6. Unit actions after they have arrived the destination region ----//

	int	destXLoc = actionNode->action_x_loc;
	int	destYLoc = actionNode->action_y_loc;
	Location* locPtr = world.get_loc(destXLoc, destYLoc);

	switch(actionNode->action_para)
	{
		case SEA_ACTION_SETTLE:
			if( locPtr->is_town() && town_array[locPtr->town_recno()]->nation_recno == nation_recno )
			{
				Town *townPtr = town_array[locPtr->town_recno()];
				unit_array.assign(townPtr->loc_x1, townPtr->loc_y1, 0, COMMAND_AI, unitRecnoArray, unitCount);		// assign to an existing town
			}
			else  //-- if there is no town there, the unit will try to settle, if there is no space for settle, settle() will just have the units move to the destination
			{
				unit_array.settle(destXLoc, destYLoc, 0, COMMAND_AI, unitRecnoArray, unitCount);		// settle as a new town
			}
			break;

		case SEA_ACTION_BUILD_CAMP:
		{
			Unit* unitPtr = unit_array[ unitRecnoArray[0] ];

			unitPtr->build_firm(destXLoc, destYLoc, FIRM_CAMP, COMMAND_AI );

			unitPtr->ai_action_id = actionNode->action_id;
			actionNode->processing_instance_count++;
			break;
		}

		case SEA_ACTION_ASSIGN_TO_FIRM:
			if( check_firm_ready(destXLoc, destYLoc) )
				unit_array.assign(destXLoc, destYLoc, 0, COMMAND_AI, unitRecnoArray, unitCount);
			break;

		case SEA_ACTION_MOVE:
			unit_array.move_to(destXLoc, destYLoc, 0, unitRecnoArray, unitCount, COMMAND_AI);
			break;

		case SEA_ACTION_NONE:		// just transport them to the specific region and disemark and wait for their own actions
			break;
	}

	//---------- set the action id. of the units ---------//

	if( actionNode->action_para != SEA_ACTION_BUILD_CAMP )		// with the exception of SEA_ACTION_BUILD_CAMP, units in all other actions are immediately executed
	{
		for( int i=unitCount-1 ; i>=0 ; i-- )
		{
			unit_array[ unitRecnoArray[i] ]->ai_action_id = actionNode->action_id;
			actionNode->processing_instance_count++;
		}
	}

	//---------------------------------------------//

	actionNode->processing_instance_count--;		// decrease it by one as it will be increased in process_action()

	actionNode->instance_count = actionNode->processing_instance_count+1;		// set the instance count so process_action() won't cause error.

	return 1;
*/
}
//-------- End of function Nation::ai_sea_travel3 -------//


//-------- Begin of function Nation::ai_find_transport_ship -------//
//
// Locate a ship for transporting units.
//
// <int> seaRegionId			 - region id. of the sea which the ship should appear in.
// <int> unitXLoc, unitYLoc - the location of the units to be picked up,
//									   try to select a harbor close to this location.
// [int] findBest           - whether need to find the best ship or just return
//									   one if there is one found. (default: 1)
//
// return: <int> the recno of the ship located.
//
int Nation::ai_find_transport_ship(int seaRegionId, int unitXLoc, int unitYLoc, int findBest)
{
	//------- locate a suitable ship --------//

	UnitMarine* unitMarine;
	int			curRating, bestRating=0, bestUnitRecno=0;

	for( int i=0 ; i<ai_ship_count ; i++ )
	{
		unitMarine = (UnitMarine*) unit_array[ ai_ship_array[i] ];

		err_when( unit_res[unitMarine->unit_id]->unit_class != UNIT_CLASS_SHIP );

		if( unitMarine->unit_count > 0 ||										// if there are already units in the ship
			 unit_res[unitMarine->unit_id]->carry_unit_capacity==0 )		// if the ship does not carry units
		{
			continue;
		}

		//------- if this ship is in the harbor ---------//

		if( unitMarine->unit_mode == UNIT_MODE_IN_HARBOR )
		{
			FirmHarbor* firmHarbor = (FirmHarbor*) firm_array[unitMarine->unit_mode_para];

			err_when( firmHarbor->firm_id != FIRM_HARBOR );

			if( firmHarbor->sea_region_id != seaRegionId )
				continue;
		}

		//--------- if this ship is on the sea ----------//

		else
		{
			if( !unitMarine->is_ai_all_stop() )
				continue;

			if( unitMarine->region_id() != seaRegionId )
				continue;

			err_when( !unitMarine->is_visible() );

			if( !unitMarine->is_visible() )
				continue;
		}

		//--------- check if the sea region is matched ---------//

		if( !findBest )		// return immediately when a suitable one is found
			return unitMarine->sprite_recno;

		curRating = world.distance_rating( unitXLoc, unitYLoc,
						unitMarine->next_x_loc(), unitMarine->next_y_loc() );

		curRating += (int)unitMarine->hit_points/10				// damage
						 + (int)unitMarine->max_hit_points/10;	// ship class

		if( curRating > bestRating )
		{
			bestRating 	  = curRating;
			bestUnitRecno = unitMarine->sprite_recno;
		}
	}

	return bestUnitRecno;
}
//-------- End of function Nation::ai_find_transport_ship -------//


//-------- Begin of function Nation::ai_build_ship -------//
//
// <int> seaRegionId	- region id. of the sea the ship will sail on.
//
// <int> preferXLoc, preferYLoc - prefer selecting a harbor that
//											 is close to this location.
//
// <int> needTransportUnit - 1 if need to transport units
//								  	  0 if the ship is for trading
//
// return: <int> the recno of the ship that the harbor has building.
//
int Nation::ai_build_ship(int seaRegionId, int preferXLoc, int preferYLoc, int needTransportUnit)
{
	//------ select the harbor for building ship -----//

	FirmHarbor *firmHarbor, *bestHarbor=NULL;
	int	curRating, bestRating=0;

	for( int i=0 ; i<ai_harbor_count ; i++ )
	{
		firmHarbor = (FirmHarbor*) firm_array[ ai_harbor_array[i] ];

		err_when( firmHarbor->nation_recno != nation_recno );

		if( !firmHarbor->can_build_ship() )
			continue;

		if( firmHarbor->sea_region_id != seaRegionId )
			continue;

		curRating = world.distance_rating( preferXLoc, preferYLoc,
						firmHarbor->center_x, firmHarbor->center_y );

		if( curRating > bestRating )
		{
			bestRating = curRating;
			bestHarbor = firmHarbor;
		}
	}

	if( !bestHarbor )
		return 0;

	//------ think about the type of ship to build -----//

	int unitId;

	if( needTransportUnit )
	{
		if( unit_res[UNIT_GALLEON]->get_nation_tech_level(nation_recno) > 0 )
			unitId = UNIT_GALLEON;

		else if( unit_res[UNIT_CARAVEL]->get_nation_tech_level(nation_recno) > 0 )
			unitId = UNIT_GALLEON;

		else
			unitId = UNIT_VESSEL;
	}
	else
	{
		if( unit_res[UNIT_GALLEON]->get_nation_tech_level(nation_recno) > 0 )
			unitId = UNIT_GALLEON;
		else                       	// don't use Caravel as it can only transport 5 units at a time
			unitId = UNIT_TRANSPORT;
	}

	bestHarbor->build_ship( unitId, COMMAND_AI );

	return 1;
}
//-------- End of function Nation::ai_build_ship -------//

