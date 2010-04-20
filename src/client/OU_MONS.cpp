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

//Filename   : OU_MONS.CPP
//Description: Unit Monster

#include <OWORLD.h>
#include <OSITE.h>
#include <OF_MONS.h>
#include <ONEWS.h>
#include <OTOWN.h>
#include <OMONSRES.h>
#include <OU_MONS.h>

//--------- Begin of function UnitMonster::UnitMonster --------//
UnitMonster::UnitMonster()
{
	monster_action_mode = MONSTER_ACTION_STOP;
}
//---------- End of function UnitMonster::UnitMonster --------//


//--------- Begin of function UnitMonster::set_monster_action_mode --------//
void UnitMonster::set_monster_action_mode(char monsterActionMode)
{
	monster_action_mode = monsterActionMode;
}
//---------- End of function UnitMonster::set_monster_action_mode --------//


//--------- Begin of function UnitMonster::unit_name ---------//
//
// [int] withTitle - whether return a string with the title of the unit
//                   or not. (default: 1)
//
char* UnitMonster::unit_name(int withTitle)
{
	static String str;

	char* monsterName = monster_res[get_monster_id()]->name;   // contribution is used for storing the monster id. temporary

	str = monsterName;

	switch( rank_id )
	{
		case RANK_KING:
			#if(defined(SPANISH))
				str  = "Gran Todo ";
				str += monsterName;
			#elif(defined(FRENCH))
				str  = "Tout Puissant ";
				str += monsterName;
			#elif(defined(GERMAN))
				str  = "Hochkönig ";
				str += monsterName;
			#else
				// GERMAN US
				str  = "All High ";
				str += monsterName;
			#endif
			break;

		case RANK_GENERAL:
			#if(defined(SPANISH) || defined(FRENCH))
				str  = "Ordo ";
				str += monsterName;
			#else
				// GERMAN US
				str += " Ordo";
			#endif
			break;
	}

	return str;
}
//--------- End of function UnitMonster::unit_name ---------//


//--------- Begin of function UnitMonster::process_ai --------//
//
void UnitMonster::process_ai()
{
	//----- when it is idle -------//

	if( !is_visible() || !is_ai_all_stop() )
		return;

	if( info.game_date%15 == sprite_recno%15 )
	{
		random_attack();		// randomly attacking targets
/*
		switch(m.random(2))
		{
			case 0:
				random_attack();		// randomly attacking targets
				break;

			case 1:
				assign_to_firm();			// assign the monsters to other monster structures
				break;
		}
*/
	}
}
//---------- End of function UnitMonster::process_ai --------//


//------- Begin of function UnitMonster::die -------//
//
void UnitMonster::die()
{
	if( !is_visible() )
		return;

	//--- check if the location where the unit dies already has an item ---//

	int xLoc = cur_x_loc();
	int yLoc = cur_y_loc();

	if( !world.get_loc(xLoc, yLoc)->can_build_site() )
	{
		int txLoc, tyLoc, foundFlag=0;

		for( tyLoc=MAX(yLoc-1,0) ; tyLoc<=MIN(yLoc+1,MAX_WORLD_Y_LOC-1) && !foundFlag ; tyLoc++ )
		{
			for( txLoc=MAX(xLoc-1,0) ; txLoc<=MIN(xLoc+1,MAX_WORLD_X_LOC-1) ; txLoc++ )
			{
				if( world.get_loc(txLoc,tyLoc)->can_build_site() )
				{
					xLoc = txLoc;
					yLoc = tyLoc;
					foundFlag = 1;
					break;
				}
			}
		}

		if( !foundFlag )
			return;
	}

	//--- when a general monster is killed, it leaves gold coins ---//

	if( !nation_recno && get_monster_id() != 0)	// to skip monster_res[ get_monster_id() ] error in test game 2
	{
		MonsterInfo* monsterInfo = monster_res[ get_monster_id() ];

		if( rank_id == RANK_GENERAL )
		{
			int goldAmount = 2 * max_hit_points * monsterInfo->level * (100+m.random(30)) / 100;

			site_array.add_site( xLoc, yLoc, SITE_GOLD_COIN, goldAmount );
			site_array.ai_get_site_object();		// ask AI units to get the gold coins
		}

		//--- when a king monster is killed, it leaves a scroll of power ---//

		else if( rank_id == RANK_KING )
		{
			king_leave_scroll();
		}
	}

	//---------- add news ----------//

	if( rank_id == RANK_KING )
		news_array.monster_king_killed( get_monster_id(), next_x_loc(), next_y_loc() );
}
//-------- End of function UnitMonster::die -------//


//------- Begin of function UnitMonster::king_leave_scroll -------//
//
void UnitMonster::king_leave_scroll()
{
	#define SCROLL_SCAN_RANGE	10

	int		 xOffset, yOffset;
	int		 xLoc, yLoc;
	Location* locPtr;
	int		 curXLoc = next_x_loc(), curYLoc = next_y_loc();
	BYTE	 	 regionId = world.get_region_id(curXLoc, curYLoc);
	short		 raceCountArray[MAX_RACE];

	memset( raceCountArray, 0, sizeof(raceCountArray) );

	int i;
	for( i=2 ; i<SCROLL_SCAN_RANGE*SCROLL_SCAN_RANGE ; i++ )
	{
		m.cal_move_around_a_point(i, SCROLL_SCAN_RANGE, SCROLL_SCAN_RANGE, xOffset, yOffset);

		xLoc = curXLoc + xOffset;
		yLoc = curYLoc + yOffset;

		xLoc = MAX(0, xLoc);
		xLoc = MIN(MAX_WORLD_X_LOC-1, xLoc);

		yLoc = MAX(0, yLoc);
		yLoc = MIN(MAX_WORLD_Y_LOC-1, yLoc);

		locPtr = world.get_loc(xLoc, yLoc);

		//----- if there is a unit on the location ------//

		if( locPtr->has_unit(UNIT_LAND) )
		{
			Unit* unitPtr = unit_array[locPtr->unit_recno(UNIT_LAND)];
			raceCountArray[ unitPtr->race_id-1 ]++;
		}
	}

	//------ find out which race is most populated in the area -----//

	int maxRaceCount=0, bestRaceId=0;

	for( i=0 ; i<MAX_RACE ; i++ )
	{
		if( raceCountArray[i] > maxRaceCount )
		{
			maxRaceCount = raceCountArray[i];
			bestRaceId   = i+1;
		}
	}

	if( !bestRaceId )
		bestRaceId = m.random(MAX_RACE)+1;		// if there is no human units nearby (perhaps just using weapons)

	//------ locate for space to add the scroll -------//

	#define ADD_SITE_RANGE	5

	for( i=1 ; i<ADD_SITE_RANGE*ADD_SITE_RANGE ; i++ )
	{
		m.cal_move_around_a_point(i, ADD_SITE_RANGE, ADD_SITE_RANGE, xOffset, yOffset);

		xLoc = curXLoc + xOffset;
		yLoc = curYLoc + yOffset;

		xLoc = MAX(0, xLoc);
		xLoc = MIN(MAX_WORLD_X_LOC-1, xLoc);

		yLoc = MAX(0, yLoc);
		yLoc = MIN(MAX_WORLD_Y_LOC-1, yLoc);

		locPtr = world.get_loc(xLoc, yLoc);

		if( locPtr->can_build_site() && locPtr->region_id==regionId )
		{
			int scrollGodId = bestRaceId;

			site_array.add_site( xLoc, yLoc, SITE_SCROLL, scrollGodId );
			site_array.ai_get_site_object();		// ask AI units to get the scroll
			break;
		}
	}
}
//-------- End of function UnitMonster::king_leave_scroll -------//


//--------- Begin of function UnitMonster::random_attack --------//
//
// Randomly pick an object to attack.
//
int UnitMonster::random_attack()
{
	#define ATTACK_SCAN_RANGE	100

	int		 xOffset, yOffset;
	int		 xLoc, yLoc;
	Location* locPtr;
	int		 curXLoc = next_x_loc(), curYLoc = next_y_loc();
	BYTE	 	 regionId = world.get_region_id(curXLoc, curYLoc);
	int		 rc;

	for( int i=2 ; i<ATTACK_SCAN_RANGE*ATTACK_SCAN_RANGE ; i++ )
	{
		m.cal_move_around_a_point(i, ATTACK_SCAN_RANGE, ATTACK_SCAN_RANGE, xOffset, yOffset);

		xLoc = curXLoc + xOffset;
		yLoc = curYLoc + yOffset;

		xLoc = MAX(0, xLoc);
		xLoc = MIN(MAX_WORLD_X_LOC-1, xLoc);

		yLoc = MAX(0, yLoc);
		yLoc = MIN(MAX_WORLD_Y_LOC-1, yLoc);

		locPtr = world.get_loc(xLoc, yLoc);

		if( locPtr->region_id != regionId )
			continue;

		rc = 0;

		//----- if there is a unit on the location ------//

		if( locPtr->has_unit(UNIT_LAND) )
		{
			int unitRecno = locPtr->unit_recno(UNIT_LAND);

			if( unit_array.is_deleted(unitRecno) )
				continue;

			rc = 1;
		}

		//----- if there is a firm on the location ------//

		if( !rc && locPtr->is_firm() )
		{
			int firmRecno = locPtr->firm_recno();

			if( firm_array.is_deleted(firmRecno) )
				continue;

			rc = 1;
		}

		//----- if there is a town on the location ------//

		if( !rc && locPtr->is_town() )
		{
			int townRecno = locPtr->town_recno();

			if( town_array.is_deleted(townRecno) )
				continue;

			rc = 1;
		}

		//-------------------------------------//

		if( rc )
		{
			group_order_monster(xLoc, yLoc, 1);		// 1-the action is attack
			return 1;
		}
	}

	return 0;
}
//---------- End of function UnitMonster::random_attack --------//


//--------- Begin of function UnitMonster::assign_to_firm --------//
//
int UnitMonster::assign_to_firm()
{
	int 	i, firmCount=firm_array.size();
	Firm* firmPtr;
	int	curXLoc = next_x_loc(), curYLoc = next_y_loc();
	BYTE	regionId = world.get_region_id(curXLoc, curYLoc);

	int firmRecno = m.random(firm_array.size())+1;

	for( i=0 ; i<firmCount ; i++ )
	{
		if( ++firmRecno > firmCount )
			firmRecno = 1;

		if( firm_array.is_deleted(firmRecno) )
			continue;

		firmPtr = firm_array[firmRecno];

		if( firmPtr->region_id != regionId )
			continue;

		if( firmPtr->firm_id == FIRM_MONSTER )
		{
			if( ((FirmMonster*)firmPtr)->can_assign_monster(sprite_recno) )
			{
				group_order_monster(firmPtr->loc_x1, firmPtr->loc_y1, 2);	// 2-the action is assign
				return 1;
			}
		}
	}

	return 0;
}
//---------- End of function UnitMonster::assign_to_firm --------//


//--------- Begin of function UnitMonster::group_order_monster --------//
//
// <int> destXLoc, destYLoc - location of the destination.
// <int> actionType			 - 1-attack, 2-assign.
//
void UnitMonster::group_order_monster(int destXLoc, int destYLoc, int actionType)
{
	#define   GROUP_ACTION_RANGE 		30		// only notify units within this range
	#define 	 MAX_MONSTER_GROUP_COUNT  15

	int		 curXLoc = next_x_loc(), curYLoc = next_y_loc();
	BYTE		 regionId = world.get_region_id(curXLoc, curYLoc);

	short 	 unitOrderedArray[MAX_MONSTER_GROUP_COUNT];
	int		 unitOrderedCount=0;

	int		 xOffset, yOffset;
	int		 xLoc, yLoc;
	Location* locPtr;
	Unit*	    unitPtr;

	//----------------------------------------------//

	for( int i=1 ; i<GROUP_ACTION_RANGE*GROUP_ACTION_RANGE ; i++ )
	{
		m.cal_move_around_a_point(i, GROUP_ACTION_RANGE, GROUP_ACTION_RANGE, xOffset, yOffset);

		xLoc = curXLoc + xOffset;
		yLoc = curYLoc + yOffset;

		xLoc = MAX(0, xLoc);
		xLoc = MIN(MAX_WORLD_X_LOC-1, xLoc);

		yLoc = MAX(0, yLoc);
		yLoc = MIN(MAX_WORLD_Y_LOC-1, yLoc);

		locPtr = world.get_loc(xLoc, yLoc);

		if( !locPtr->has_unit(UNIT_LAND) )
			continue;

		//------------------------------//

		int unitRecno = locPtr->unit_recno(UNIT_LAND);

		if( unit_array.is_deleted(unitRecno) )
			continue;

		unitPtr = unit_array[unitRecno];

		if( unit_res[unitPtr->unit_id]->unit_class != UNIT_CLASS_MONSTER )
			continue;

		unitOrderedArray[unitOrderedCount] = unitRecno;

		if( ++unitOrderedCount >= MAX_MONSTER_GROUP_COUNT )
			break;
	}

	if( unitOrderedCount == 0 )
		return;

	//---------------------------------------//

	if( actionType==1 )		// attack
	{
		// ##### patch begin Gilbert 5/8 ######//
		unit_array.attack( destXLoc, destYLoc, 0, unitOrderedArray, unitOrderedCount, COMMAND_AI, 0 );
		// ##### patch end Gilbert 5/8 ######//
	}
	else
	{
		unit_array.assign( destXLoc, destYLoc, 0, COMMAND_AI, unitOrderedArray, unitOrderedCount);
	}
}
//---------- End of function UnitMonster::group_order_monster --------//
