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

//Filename	 : OAI_MONS.CPP
//Description: AI functions for dealing with the monsters.

#include <OF_MONS.h>
#include <ONATION.h>
#include <OCONFIG.h>
#include <OMONSRES.h>

//--------- Begin of function Nation::think_attack_monster --------//

int Nation::think_attack_monster()
{
	if( config.monster_type == OPTION_MONSTER_NONE )		// no monsters in the game
		return 0;

	//--- if the AI has run out of money and is currently cheating, it will have a urgent need to attack monsters to get money ---//

	int useAllCamp = income_365days(INCOME_CHEAT) > 0;

	if( !useAllCamp )		// use all camps to attack the monster
	{
		if( !is_at_war() )
		{
			if( cash < 500 && military_rank_rating() >= 75-pref_attack_monster/4 )		//  50 to 75
				useAllCamp = 1 ;
		}
	}

	if( !useAllCamp )
	{
		if( military_rank_rating() < 50-pref_attack_monster/4 )		// don't attack if the military strength is too low, 25 to 50
			return 0;
	}

	//------- select a monster target ---------//

	int targetCombatLevel;

	int targetFirmRecno = think_monster_target(targetCombatLevel);

	if( !targetFirmRecno )
		return 0;

	targetCombatLevel = targetCombatLevel * 150 / 100;		// X 150%

	//--- we need to use veteran soldiers to attack powerful monsters ---//

	FirmMonster* targetFirm = (FirmMonster*) firm_array[targetFirmRecno];

	int monsterLevel = monster_res[targetFirm->monster_id]->level;

	int attackerMinCombatLevel = 0;

	if( targetCombatLevel > 100 )			// if the nation's cash runs very low, it will attack anyway
		attackerMinCombatLevel = 20 + monsterLevel*3;

	//--- call ai_attack_target() to attack the target town ---//

	return ai_attack_target(targetFirm->loc_x1, targetFirm->loc_y1, targetCombatLevel, 0, 0, attackerMinCombatLevel, 0, useAllCamp );
}
//---------- End of function Nation::think_attack_monster --------//


//--------- Begin of function Nation::think_monster_target --------//
//
// Think about the best monster target in the given region.
//
// <int&> targetCombatLevel - var for returning the total combat level of the target firm.
//
int Nation::think_monster_target(int& targetCombatLevel)
{
	if( !largest_town_recno )
		return 0;

	Town* largestTown = town_array[largest_town_recno];
	Firm* firmPtr;
	int	combatLevel;
	int	curRating, bestRating= -10000, bestFirmRecno=0, hasWar;

	for( int firmRecno=firm_array.size() ; firmRecno>0 ; firmRecno-- )
	{
		if( firm_array.is_deleted(firmRecno) )
			continue;

		firmPtr = firm_array[firmRecno];

		if( firmPtr->firm_id != FIRM_MONSTER ||
			 firmPtr->region_id != largestTown->region_id )
		{
			continue;
		}

		//----- take into account of the mobile units around this town -----//

		int mobileCombatLevel = mobile_defense_combat_level(firmPtr->center_x, firmPtr->center_y, firmPtr->nation_recno, 1, hasWar);

		if( mobileCombatLevel == -1 )		// do not attack this town because a battle is already going on
			continue;

		curRating = 3 * m.points_distance( largestTown->center_x, largestTown->center_y,
													  firmPtr->center_x, firmPtr->center_y );

		combatLevel = mobileCombatLevel +
						  ((FirmMonster*)firmPtr)->total_combat_level();

		curRating -= combatLevel;

		//---------------------------------//

		if( curRating > bestRating )
		{
			targetCombatLevel = combatLevel;
			bestRating        = curRating;
			bestFirmRecno     = firmRecno;
		}
	}

	return bestFirmRecno;
}
//---------- End of function Nation::think_monster_target --------//

