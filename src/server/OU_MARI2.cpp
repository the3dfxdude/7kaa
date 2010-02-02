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

//Filename    : OU_MARI2.CPP
//Description : Marine Units AI functions

#include <OINFO.h>
#include <ONATION.h>
#include <OF_HARB.h>
#include <OU_MARI.h>


//------- Begin of function UnitMarine::init_derived --------//

void UnitMarine::init_derived()
{
	last_load_goods_date = info.game_date;
}
//------- End of function UnitMarine::init_derived --------//


//------- Begin of function UnitMarine::process_ai --------//
//
void UnitMarine::process_ai()
{
	//-- Think about removing stops whose owner nation is at war with us. --//

	// AI does do any sea trade

//	if( info.game_date%30 == sprite_recno%30 )
//		think_del_stop();

	//---- Think about setting new trade route -------//

	if( info.game_date%15 == sprite_recno%15 )
	{
		if( stop_defined_num < 2 && is_visible() && is_ai_all_stop() )
			ai_sail_to_nearby_harbor();
	}

	//------ Think about resigning this caravan -------//

	if( info.game_date%60 == sprite_recno%60 )
		think_resign();
}
//------- End of function UnitMarine::process_ai --------//


//--------- Begin of function UnitMarine::is_ai_all_stop --------//

int UnitMarine::is_ai_all_stop()
{
	if( cur_action != SPRITE_IDLE || ai_action_id )
		return 0;

	//---- if the ship is on the beach, it's action mode is always ACTION_SHIP_TO_BEACH, so we can't check it against ACTION_STOP ---//

	if( action_mode2 == ACTION_SHIP_TO_BEACH )
	{
		if( in_beach && (extra_move_in_beach==NO_EXTRA_MOVE || extra_move_in_beach==EXTRA_MOVE_FINISH) ) 
			return 1;
	}
		
	return action_mode==ACTION_STOP && action_mode2==ACTION_STOP;
}
//---------- End of function UnitMarine::is_ai_all_stop --------//


//------- Begin of function UnitMarine::think_resign --------//

int UnitMarine::think_resign()
{
	//---- only resign when the ship has stopped ----//

	if( !is_ai_all_stop() )
		return 0;

	//--- retire this ship if we have better ship technology available ---//

	if( unit_id == UNIT_TRANSPORT )
	{
		if( unit_res[UNIT_CARAVEL]->get_nation_tech_level(nation_recno) > 0 ||
			 unit_res[UNIT_GALLEON]->get_nation_tech_level(nation_recno) > 0 )
		{
			if( !nation_array[nation_recno]->ai_is_sea_travel_safe() )
			{
				resign(COMMAND_AI);
				return 1;
			}
		}
	}

	return 0;
}
//------- End of function UnitMarine::think_resign --------//


//------- Begin of function UnitMarine::think_del_stop --------//
//
// Think about removing stops whose owner nation is at war with us.
//
void UnitMarine::think_del_stop()
{
	if( !is_visible() )		// cannot del stop if the caravan is inside a market place.
		return;

	Nation* nationPtr = nation_array[nation_recno];

	for( int i=stop_defined_num ; i>0 ; i-- )
	{
		if( firm_array.is_deleted(stop_array[i-1].firm_recno) )
		{
			del_stop(i, COMMAND_AI);
			continue;
		}

		//----------------------------------------------//

		int nationRecno = firm_array[ stop_array[i-1].firm_recno ]->nation_recno;

		if( nationPtr->get_relation_status(nationRecno) == NATION_HOSTILE )
		{
			del_stop(i, COMMAND_AI);
		}
	}
}
//------- End of function UnitMarine::think_del_stop --------//


//--------- Begin of function UnitMarine::ai_sail_to_nearby_harbor --------//

void UnitMarine::ai_sail_to_nearby_harbor()
{
	Nation		*ownNation = nation_array[nation_recno];
	FirmHarbor  *firmHarbor, *bestHarbor=NULL;
	int			curRating, bestRating=0; 
	int			curXLoc=cur_x_loc(), curYLoc=cur_y_loc();
	int			curRegionId=region_id();
	
	for( int i=0 ; i<ownNation->ai_harbor_count ; i++ )
	{
		firmHarbor = (FirmHarbor*) firm_array[ ownNation->ai_harbor_array[i] ];
		
		if( firmHarbor->sea_region_id != curRegionId )
			continue;

		curRating = world.distance_rating( curXLoc, curYLoc, firmHarbor->center_x, firmHarbor->center_y );

		curRating += (MAX_SHIP_IN_HARBOR - firmHarbor->ship_count) * 100;

		if( curRating > bestRating )
		{
			bestRating = curRating;
			bestHarbor = firmHarbor;
		}
	}

	if( bestHarbor )
		assign(bestHarbor->loc_x1, bestHarbor->loc_y1);
}
//---------- End of function UnitMarine::ai_sail_to_nearby_harbor --------//


//--------- Begin of function UnitMarine::ai_ship_being_attacked --------//
//
// This function is called by Unit::hit_target() when the king is
// under attack.
//
// <int> attackerUnitRecno - recno of the attacker unit.
//
void UnitMarine::ai_ship_being_attacked(int attackerUnitRecno)
{
	Unit* attackerUnit = unit_array[attackerUnitRecno];

	if( attackerUnit->nation_recno == nation_recno )		// this can happen when the unit has just changed nation
		return;

	if( info.game_date%5 == sprite_recno%5 )
	{
		nation_array[nation_recno]->ai_sea_attack_target( attackerUnit->next_x_loc(), attackerUnit->next_y_loc() );
	}
}
//---------- End of function UnitMarine::ai_ship_being_attacked --------//

