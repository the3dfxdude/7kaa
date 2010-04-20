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

//Filename   : OAI_CAP2.CPP
//Description: AI - capturing AI towns

#include <stdlib.h>
#include <ALL.h>
#include <OGAME.h>
#include <OCONFIG.h>
#include <OUNIT.h>
#include <OFIRMALL.h>
#include <OTALKRES.h>
#include <ONATION.h>

//--------- Begin of function Nation::think_capture_new_enemy_town --------//
//
// <Town*> capturerTown - our town to capture enemy towns.
// [int]   useAllCamp   - whether use troops in all camps to attack the enemy town
//								  (default: 0)
//
int Nation::think_capture_new_enemy_town(Town* capturerTown, int useAllCamp)
{
	if( ai_camp_count==0 )		// this can happen when a new nation has just emerged
		return 0;

	if( ai_capture_enemy_town_recno )		// no new action if we are still trying to capture a town
		return 0;

	//---- only attack when we have enough money to support the war ----//

	if( cash < 1000 + 2000 * pref_cash_reserve / 100 )		// if the cash is really too low now
		return 0;

	//--------------------------------------//

	Town* targetTown = think_capture_enemy_town_target(capturerTown);

	if( !targetTown )
		return 0;

	//---- attack enemy's defending forces on the target town ----//

	int rc = attack_enemy_town_defense(targetTown, useAllCamp);

	if( rc==0 ) 		// 0 means we don't have enough troop to attack the enemy
		return 0;

	else if( rc == 1 )		// 1 means a troop has been sent to attack the town
	{
		ai_capture_enemy_town_recno = targetTown->town_recno;		// this nation is currently trying to capture this town
		ai_capture_enemy_town_plan_date = info.game_date;
		ai_capture_enemy_town_start_attack_date = 0;
		ai_capture_enemy_town_use_all_camp = useAllCamp;

		return 1;
	}

	else if( rc == -1 )		// -1 means no defense on the target town, no attacking is needed.
	{
		return start_capture( targetTown->town_recno );		// call AI functions in OAI_CAPT.CPP to capture the town
	}

	return 0;
}
//---------- End of function Nation::think_capture_new_enemy_town --------//


//--------- Begin of function Nation::think_capturing_enemy_town --------//

void Nation::think_capturing_enemy_town()
{
	if( !ai_capture_enemy_town_recno )
		return;

	if( town_array.is_deleted(ai_capture_enemy_town_recno) ||
		 town_array[ai_capture_enemy_town_recno]->nation_recno == nation_recno )		// this town has been captured already
	{
		ai_capture_enemy_town_recno = 0;
		return;
	}

	//--- check the enemy's mobile defense combat level around the town ---//

	Town* targetTown = town_array[ai_capture_enemy_town_recno];
	int	hasWar;

	int mobileCombatLevel = mobile_defense_combat_level(targetTown->center_x, targetTown->center_y, targetTown->nation_recno, 0, hasWar);		// 0-don't return immediately even if there is war around this town

	//---- if we haven't started attacking the town yet -----//

	if( !ai_capture_enemy_town_start_attack_date )
	{
		if( hasWar==2 )		// we are at war with the nation now
			ai_capture_enemy_town_start_attack_date = info.game_date;

		if( info.game_date > ai_capture_enemy_town_plan_date + 90 )		// when 3 months have gone and there still hasn't been any attack on the town, there must be something bad happened to our troop, cancel the entire action
			ai_capture_enemy_town_recno = 0;

		return;		// do nothing if the attack hasn't started yet
	}

	//--------- check if we need reinforcement --------//

	//-----------------------------------------------------------//
	// Check how long we have started attacking because only
	// when the it has been started for a while, our force
	// will reach the target and the offensive and defensive force
	// total can be calculated accurately.
	//-----------------------------------------------------------//

	if( info.game_date - ai_capture_enemy_town_start_attack_date >= 15 )
	{
		//-------- check if we need any reinforcement --------//

		if( mobileCombatLevel > 0 && hasWar==2 )		// we are still in war with the enemy
		{
			ai_attack_target(targetTown->center_x, targetTown->center_y, mobileCombatLevel, 0, 1 );      // 1-just all move there and wait for the units to attack the enemies automatically
			return;
		}
	}

	//----- there is currently no war at the town  -----//
	//
	// - either we are defeated or we have destroyed their command base.
	//
	//--------------------------------------------------//

	if( hasWar != 2 )
	{
		//---- attack enemy's defending forces on the target town ----//

		int rc = attack_enemy_town_defense(targetTown, ai_capture_enemy_town_use_all_camp);

		if( rc == 1 )		// 1 means a troop has been sent to attack the town
		{
			ai_capture_enemy_town_start_attack_date = 0;
			return;
		}

		//---------- reset the vars --------//

		ai_capture_enemy_town_recno = 0;
		ai_capture_enemy_town_start_attack_date = 0;

		//--------- other situations --------//

		if( rc == -1 )		// -1 means no defense on the target town, no attacking is needed.
		{
			start_capture( targetTown->town_recno );		// call AI functions in OAI_CAPT.CPP to capture the town
		}
		
		// 0 means we don't have enough troop to attack the enemy
	}
}
//---------- End of function Nation::think_capturing_enemy_town --------//


//--------- Begin of function Nation::attack_enemy_town_defense --------//
//
// Attack enemy's defending forces on the target town.
//
// <Town*> targetTown - the pointer to the target town.
// [int]   useAllCamp   - whether use troops in all camps to attack the enemy town
//								  (default: 0)
//
// return: <int> 1 - a troop has been sent to attack the target.
//					  0 - we don't have sufficient troops for attacking the target.
//					 -1 - no defense on the target town, no attacking is needed.
//
int Nation::attack_enemy_town_defense(Town* targetTown, int useAllCamp)
{
	err_when( targetTown->nation_recno == nation_recno );		// cannot attack itself

	if( targetTown->nation_recno == 0 )
		return -1;

	//--- if there are any command bases linked to the town, attack them first ---//

	int  campCombatLevel, maxCampCombatLevel= -1;
	Firm *firmPtr, *bestTargetFirm=NULL;

	for( int i=targetTown->linked_firm_count-1 ; i>=0 ; i-- )
	{
		firmPtr = firm_array[ targetTown->linked_firm_array[i] ];

		if( firmPtr->nation_recno == targetTown->nation_recno &&
			 firmPtr->firm_id == FIRM_CAMP )
		{
			campCombatLevel = ((FirmCamp*)firmPtr)->total_combat_level();

			if( campCombatLevel > maxCampCombatLevel )
			{
				maxCampCombatLevel = campCombatLevel;
				bestTargetFirm = firmPtr;
			}
		}
	}

	//----- get the defense combat level of the mobile units around the town ----//

	int hasWar;
	int townMobileCombatLevel = mobile_defense_combat_level(targetTown->center_x, targetTown->center_y, targetTown->nation_recno, 0, hasWar);
	int totalDefenseCombatLevel = maxCampCombatLevel + townMobileCombatLevel;

	//----------------------------------------//

	if( bestTargetFirm )
	{
		Nation* targetNation = nation_array[bestTargetFirm->nation_recno];

		if( targetNation->is_at_war() )		// use all camps force if the nation is at war
			useAllCamp = 1;

		return ai_attack_target(bestTargetFirm->loc_x1, bestTargetFirm->loc_y1, totalDefenseCombatLevel,
										0, 0, 0, 0, useAllCamp );
	}
	else
	{
		//--- if there are any mobile defense force around the town ----//

		if( townMobileCombatLevel > 0 )
			return ai_attack_target(targetTown->center_x, targetTown->center_y, totalDefenseCombatLevel, 0, 1 );      // 1-just all move there and wait for the units to attack the enemies automatically
	}

	return -1;
}
//---------- End of function Nation::attack_enemy_town_defense --------//


//--------- Begin of function Nation::think_capture_enemy_town_target --------//
//
// <Town*> capturerTown - our town to capture enemy towns.
//
// Motives for attacking another nation:
//
// 1. Capture towns
// 2. Conquer land
// 3. Defeat enemies
//
Town* Nation::think_capture_enemy_town_target(Town* capturerTown)
{
	int   townRecno, curRating;
	Town* targetTown, *bestTown=NULL;
	Firm* firmPtr;
	int   ourMilitary = military_rank_rating();
	Nation* ownNation = nation_array[nation_recno];
	int   bestRating = -1000;
	int  	hasWar;
	int   neededCombatLevel=0;

	for( townRecno=town_array.size() ; townRecno>0 ; townRecno-- )
	{
		if( town_array.is_deleted(townRecno) )
			continue;

		targetTown = town_array[townRecno];

		if( targetTown->nation_recno == 0 ||
			 targetTown->nation_recno == nation_recno )
		{
			continue;
		}

		if( targetTown->region_id != capturerTown->region_id )
			continue;

		//----- if we have already built a camp next to this town -----//

		if( targetTown->has_linked_camp(nation_recno, 0) )		//0-count both camps with or without overseers
			continue;

		//--------- only attack enemies -----------//

		NationRelation* nationRelation = get_relation(targetTown->nation_recno);

		int rc=0;

		if( nationRelation->status == NATION_HOSTILE )
			rc = 1;

		else if( nationRelation->ai_relation_level < 10 )			// even if the relation is not hostile, if the ai_relation_level is < 10, attack anyway
			rc = 1;

		else if( nationRelation->status <= NATION_NEUTRAL &&
			 targetTown->nation_recno == nation_array.max_overall_nation_recno &&		// if this is our biggest enemy
			 nationRelation->ai_relation_level < 30 )
		{
			rc = 1;
		}

		if( !rc )
			continue;

		//----- if this town does not have any linked camps, capture this town immediately -----//

		if( targetTown->has_linked_camp(targetTown->nation_recno, 0) )		//0-count both camps with or without overseers
			return targetTown;

		//--- if the enemy is very powerful overall, don't attack it yet ---//

		if( nation_array[targetTown->nation_recno]->military_rank_rating() >
			 ourMilitary * (80+pref_military_courage/2) / 100 )
		{
			continue;
		}

		//------ only attack if we have enough money to support the war ----//

		if( !ai_should_spend_war( nation_array[targetTown->nation_recno]->military_rank_rating() ) )
			continue;

		//-------------------------------------------------------//

		int townCombatLevel = enemy_town_combat_level(targetTown, 1, hasWar);		// 1-return a rating if there is war with the town

		if( townCombatLevel == -1 )      // do not attack this town because a battle is already going on
			continue;

		//------- calculate the rating --------------//

		curRating = world.distance_rating(capturerTown->center_x, capturerTown->center_y,
						targetTown->center_x, targetTown->center_y);

		curRating -= townCombatLevel/10;

		curRating -= targetTown->average_loyalty();

		curRating += targetTown->population;		// put a preference on capturing villages with large population

		//----- the power of between the nation also affect the rating ----//

		curRating += 2 * (ourMilitary - nation_array[targetTown->nation_recno]->military_rank_rating());

		//-- AI Aggressive is set above Low, than the AI will try to capture the player's town first ---//

		if( !targetTown->ai_town )
		{
			if( game.game_mode == GAME_TUTORIAL )		// next attack the player in a tutorial game
			{
				continue;
			}
			else
			{
				switch( config.ai_aggressiveness )
				{
					case OPTION_MODERATE:
						curRating += 100;
						break;

					case OPTION_HIGH:
						curRating += 300;
						break;

					case OPTION_VERY_HIGH:
						curRating += 500;
						break;
				}
			}
		}

		//--- if there are mines linked to this town, increase its rating ---//

		for( int i=targetTown->linked_firm_count-1 ; i>=0 ; i-- )
		{
			firmPtr = firm_array[ targetTown->linked_firm_array[i] ];

			if( firmPtr->nation_recno != targetTown->nation_recno )
				continue;

			if( firmPtr->firm_id == FIRM_MINE )
			{
				//--- if this mine's raw materials is one that we don't have --//

				if( raw_count_array[ ((FirmMine*)firmPtr)->raw_id-1 ]==0 )
					curRating += 150 * (int) ((FirmMine*)firmPtr)->reserve_qty / MAX_RAW_RESERVE_QTY;
			}
		}

		//--- more linked towns increase the attractiveness rating ---//

		curRating += targetTown->linked_firm_count*5;

		//-------- compare with the current best rating ---------//

		if( curRating > bestRating )
		{
			bestRating    = curRating;
			bestTown      = targetTown;
			neededCombatLevel = townCombatLevel;
		}
	}

	return bestTown;
}
//-------- End of function Nation::think_capture_enemy_town_target ------//


//--------- Begin of function Nation::enemy_town_combat_level --------//
//
// <Town*> targetTown  - the target town
// <int>   returnIfWar - return -1 if there is any war around the town
// <int&>  hasWar      - a reference var for returning whether there is any war
//
// return: <int> the enemy's total defense combat level minus the player's
//					  combat level.
///		        return -1 if there is war and returnIfWar is 1
//
int Nation::enemy_town_combat_level(Town* targetTown, int returnIfWar, int hasWar)
{
	int enemyCombatLevel = mobile_defense_combat_level(targetTown->center_x, targetTown->center_y, targetTown->nation_recno, returnIfWar, hasWar);		//0-don't return even there are wars around the town

	if( enemyCombatLevel < 0 )		// there is a war going on
		return -1;

	//---- calculate the attack rating of this target town ----//

//	enemyCombatLevel += targetTown->jobless_population * 5;	//**BUGHERE

	//--- calculate the combat level of enemy camps linked to this town ---//

	Firm* firmPtr;

	for( int i=targetTown->linked_firm_count-1 ; i>=0 ; i-- )
	{
		firmPtr = firm_array[ targetTown->linked_firm_array[i] ];

		if( firmPtr->nation_recno == targetTown->nation_recno &&
			 firmPtr->firm_id == FIRM_CAMP )
		{
			enemyCombatLevel += ((FirmCamp*)firmPtr)->total_combat_level();
		}
	}
/*
	//----- add this and neighbor town's needed combat level ----//

	Town* townPtr;

	for( i=targetTown->linked_town_count-1 ; i>=0 ; i-- )
	{
		townPtr = town_array[ targetTown->linked_town_array[i] ];

		if( townPtr->nation_recno == targetTown->nation_recno )	//**BUGHERE
			enemyCombatLevel += townPtr->jobless_population * 5;
	}
*/
	return enemyCombatLevel;
}
//-------- End of function Nation::enemy_town_combat_level ------//


//--------- Begin of function Nation::enemy_firm_combat_level --------//
//
// <Firm*> targetFirm  - the target firm
// <int>   returnIfWar - return -1 if there is any war around the town
// <int&>  hasWar      - a reference var for returning whether there is any war
//
// return: <int> the enemy's total defense combat level minus the player's
//					  combat level.
///		        return -1 if there is war and returnIfWar is 1
//
int Nation::enemy_firm_combat_level(Firm* targetFirm, int returnIfWar, int hasWar)
{
	int enemyCombatLevel = mobile_defense_combat_level(targetFirm->center_x, targetFirm->center_y, targetFirm->nation_recno, returnIfWar, hasWar);		//0-don't return even there are wars around the town

	if( enemyCombatLevel < 0 )		// there is a war going on
		return -1;

	//--- calculate the combat level of enemy camps linked to this towns that are linked to this mine ---//

	Town* linkedTown;
	Firm* firmPtr;
	int   targetNationRecno = targetFirm->nation_recno;

	//---- scan towns linked to this mine -----//

	for( int i=targetFirm->linked_town_count-1 ; i>=0 ; i-- )
	{
		linkedTown = town_array[ targetFirm->linked_town_array[i] ];

		if( linkedTown->nation_recno != targetNationRecno )
			continue;

		//------ scan firms linked to this town -------//

		for( int j=linkedTown->linked_firm_count-1 ; j>=0 ; j-- )
		{
			firmPtr = firm_array[ linkedTown->linked_firm_array[j] ];

			if( firmPtr->nation_recno == targetNationRecno &&
				 firmPtr->firm_id == FIRM_CAMP )
			{
				enemyCombatLevel += ((FirmCamp*)firmPtr)->total_combat_level();
			}
		}
	}

	return enemyCombatLevel;
}
//-------- End of function Nation::enemy_firm_combat_level ------//


//------- Begin of function Nation::mobile_defense_combat_level ------//
//
// Take into account of the mobile units around this target location
// when considering attacking it.
//
// <int> targetXLoc, targetYLoc - the target location
// <int> targetNationRecno      - nation recno of the target
// <int> returnIfWar	 		  	  - whether return -1 if there is war
//											 around the given area.
// <int&> hasWar					  - a var for returning whether there is war
//											 around the given area.
//											 1 - if there is war
//											 2 - if our nation is involved in the war
//
// return : >= 0 the defense rating of this location, the rating can be < 0,
//               if we have our own units there.
//
//          -1  don't attack this town because a battle is already
//              going on.
//
int Nation::mobile_defense_combat_level(int targetXLoc, int targetYLoc, int targetNationRecno, int returnIfWar, int& hasWar)
{
	//--- the scanning distance is determined by the AI aggressiveness setting ---//

	int scanRangeX = 5 + config.ai_aggressiveness * 2;
	int scanRangeY = scanRangeX;

	int xLoc1 = targetXLoc - scanRangeX;
	int yLoc1 = targetYLoc - scanRangeY;
	int xLoc2 = targetXLoc + scanRangeX;
	int yLoc2 = targetYLoc + scanRangeY;

	xLoc1 = MAX( xLoc1, 0 );
	yLoc1 = MAX( yLoc1, 0 );
	xLoc2 = MIN( xLoc2, MAX_WORLD_X_LOC-1 );
	yLoc2 = MIN( yLoc2, MAX_WORLD_Y_LOC-1 );

	//------------------------------------------//

	float totalCombatLevel=(float)0;    // the higher the rating, the easier we can attack the target town.
	int   xLoc, yLoc;
	Unit* unitPtr;
	Location* locPtr;

	hasWar = 0;

	for( yLoc=yLoc1 ; yLoc<=yLoc2 ; yLoc++ )
	{
		locPtr = world.get_loc(xLoc1, yLoc);

		for( xLoc=xLoc1 ; xLoc<=xLoc2 ; xLoc++, locPtr++ )
		{
			if( !locPtr->has_unit(UNIT_LAND) )
				continue;

			unitPtr = unit_array[ locPtr->unit_recno(UNIT_LAND) ];

			//--------------------------------------------------//
			// If there is already a battle going on in this town,
			// do not attack this town.
			//--------------------------------------------------//

			if( unitPtr->cur_action == SPRITE_ATTACK )
			{
				if( returnIfWar )
					return -1;
				else
				{
					if( unitPtr->nation_recno == nation_recno )
						hasWar = 2;
					else
						hasWar = 1;
				}
			}

			//---- if this unit is guarding the town -----//

			if( unitPtr->nation_recno == targetNationRecno )
			{
				totalCombatLevel += unitPtr->unit_power();
			}

			//------- if this is our own unit ------//

			else if( unitPtr->nation_recno == nation_recno )
			{
				if( unitPtr->cur_action == SPRITE_ATTACK ||		// only units that are currently attacking or idle are counted, moving units may just be passing by
					 unitPtr->cur_action == SPRITE_IDLE )
				{
					totalCombatLevel -= unitPtr->unit_power();
				}
			}
		}
	}

	if( totalCombatLevel == -1 )		// -1 is reserved for returning don't attack 
		return 0;
	else
		return (int) totalCombatLevel;
}
//-------- End of function Nation::mobile_defense_combat_level ------//

