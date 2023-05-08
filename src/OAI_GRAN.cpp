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

//Filename   : OAI_GRAN.CPP
//Description: AI grand plans

#include <OCONFIG.h>
#include <OTALKRES.h>
#include <ORACERES.h>
#include <OREGIONS.h>
#include <OF_CAMP.h>
#include <ONATION.h>
#include <ConfigAdv.h>


//----- Begin of function Nation::think_grand_plan -----//
//
void Nation::think_grand_plan()
{
	think_deal_with_all_enemy();

	think_against_mine_monopoly();

	think_ally_against_big_enemy();
}
//------ End of function Nation::think_grand_plan ------//


//----- Begin of function Nation::total_alliance_military -----//
//
// Return the total power of this nation and its friendly/allied
// nation.
//
int Nation::total_alliance_military()
{
	int totalPower = military_rank_rating();

	for( int i=nation_array.size() ; i>0 ; i-- )
	{
		if( nation_array.is_deleted(i) || i==nation_recno )
			continue;

		switch( get_relation_status(i) )
		{
			case NATION_ALLIANCE:
				totalPower += nation_array[i]->military_rank_rating() * 3 / 4;	// 75%
				break;
/*
			case NATION_FRIENDLY:
				totalPower += nation_array[i]->military_rank_rating() / 2;	//50%
				break;
*/
		}
	}

	return totalPower;
}
//------ End of function Nation::total_alliance_military ------//


//----- Begin of function Nation::total_enemy_military -----//
//
// Return the total power of this nation's enemies and potential
// enemies.
//
int Nation::total_enemy_military()
{
	Nation *nationPtr;
	int 	 totalPower=0, relationStatus;
//	int 	 relationStatus2, enemyRelationStatus;

	for( int i=nation_array.size() ; i>0 ; i-- )
	{
		if( nation_array.is_deleted(i) || i==nation_recno )
			continue;

		nationPtr = nation_array[i];
		relationStatus = get_relation_status(i);

		if( relationStatus == NATION_HOSTILE )
		{
			totalPower += nationPtr->military_rank_rating();
		}
/*
		else
		{
			//--- check this nation's status with our enemies ---//

			enemyRelationStatus = 0;

			for( int j=nation_array.size() ; j>0 ; j-- )
			{
				if( nation_array.is_deleted(j) || j==nation_recno )
					continue;

				if( get_relation_status(j) != NATION_HOSTILE )   // only if this is one of our enemies.
					continue;

				//--- check if it is allied or friendly to any of our enemies ---//

				relationStatus2 = nationPtr->get_relation_status(j);

				if( relationStatus2 > enemyRelationStatus )		// Friendly will replace none, Alliance will replace Friendly
					enemyRelationStatus = relationStatus2;
			}

			if( enemyRelationStatus == NATION_ALLIANCE )
				totalPower += nationPtr->military_rank_rating() * 3 / 4;	 // 75%

			else if( enemyRelationStatus == NATION_FRIENDLY )
				totalPower += nationPtr->military_rank_rating() / 2;		 // 50%
		}
*/
	}

	return totalPower;
}
//------ End of function Nation::total_enemy_military ------//


//----- Begin of function Nation::total_enemy_count -----//
//
int Nation::total_enemy_count()
{
	int totalEnemy=0;

	for( int i=nation_array.size() ; i>0 ; i-- )
	{
		if( nation_array.is_deleted(i) || i==nation_recno )
			continue;

		if( get_relation_status(i) == NATION_HOSTILE )
			totalEnemy++;
	}

	return totalEnemy;
}
//------ End of function Nation::total_enemy_count ------//


//----- Begin of function Nation::think_deal_with_all_enemy -----//
//
// Think about dealing with the enemy. The following are the
// actions a nation can take to deal with its enemies.
//
// >ask our allies to attack the enemy.
//
// >try to break the enemy's existing alliance/friendly treaty with other
//	 nations - to reduce its alliance strength.
//
// >convert enemy's allies to ours.
//
// >ask other nations to impose trade embargos on the enemy.
//
void Nation::think_deal_with_all_enemy()
{
	Nation* nationPtr;

	int ourMilitary = military_rank_rating();
	int enemyCount  = total_enemy_count();

	for( int i=1 ; i<=nation_array.size() ; i++ )
	{
		if( nation_array.is_deleted(i) || nation_recno == i )
			continue;

		if( get_relation_status(i) != NATION_HOSTILE )
			continue;

		nationPtr = nation_array[i];

		//------- think about eliminating the enemy ------//

		int rc = 0;

		if( nationPtr->total_population==0 ) 		// the enemy has no towns left
			rc = 1;

		if( enemyCount==1 &&
			 ourMilitary > 100-pref_military_courage/5 ) 		// 80 to 100
		{
			int enemyMilitary = nationPtr->military_rank_rating();

			if( enemyMilitary < 20 && ai_should_spend_war(enemyMilitary) )
				rc = 1;
		}

		if( rc )
		{
			if( think_eliminate_enemy_firm(i) )
				continue;

			if( think_eliminate_enemy_town(i) )
				continue;

			think_eliminate_enemy_unit(i);
			continue;
		}

		//----- think about dealing with the enemy with diplomacy -----//

		think_deal_with_one_enemy(i);
	}
}
//------ End of function Nation::think_deal_with_all_enemy ------//


//----- Begin of function Nation::think_deal_with_one_enemy -----//
//
void Nation::think_deal_with_one_enemy(int enemyNationRecno)
{
	Nation* nationPtr;
	NationRelation* nationRelation;

	for( int i=1 ; i<=nation_array.size() ; i++ )
	{
		if( nation_array.is_deleted(i) || nation_recno == i )
			continue;

		nationPtr = nation_array[i];

		nationRelation = nationPtr->get_relation(nation_recno);

		//--- if this nation is already allied to us, request it to declare war with the enemy ---//

		if( nationRelation->status == NATION_ALLIANCE &&
			 nationPtr->get_relation_status(enemyNationRecno) != NATION_HOSTILE )
		{
			if( should_diplomacy_retry(TALK_REQUEST_DECLARE_WAR, i) )
			{
				talk_res.ai_send_talk_msg(i, nation_recno, TALK_REQUEST_DECLARE_WAR, enemyNationRecno);
				continue;
			}
		}

		//---- if this nation is not friendly or alliance to our enemy ----//

		if( nationPtr->get_relation_status(enemyNationRecno) < NATION_FRIENDLY )
		{
			//--- and this nation is neutral or friendly with us ---//

			if( nationRelation->status >= NATION_NEUTRAL &&
				 nationPtr->get_relation(enemyNationRecno)->trade_treaty )
			{
				//--- ask it to join a trade embargo on the enemy ---//

				if( should_diplomacy_retry(TALK_REQUEST_TRADE_EMBARGO, i) )
					talk_res.ai_send_talk_msg(i, nation_recno, TALK_REQUEST_TRADE_EMBARGO, enemyNationRecno );
			}
		}
		else 	//---- if this nation is friendly or alliance to our enemy ----//
		{
			//----- and this nation is not at war with us -----//

			if( nationRelation->status != NATION_HOSTILE )
			{
				//--- if we do not have trade treaty with this nation, propose one ---//

				if( !nationRelation->trade_treaty )
				{
					if( should_diplomacy_retry(TALK_PROPOSE_TRADE_TREATY, i) )
						talk_res.ai_send_talk_msg(i, nation_recno, TALK_PROPOSE_TRADE_TREATY );
				}
				else //--- if we already have a trade treaty with this nation ---//
				{
					// if this nation is already friendly to us, propose an alliance treaty now --//

					if( nationRelation->status == NATION_FRIENDLY )
					{
						if( should_diplomacy_retry(TALK_PROPOSE_ALLIANCE_TREATY, i) )
							talk_res.ai_send_talk_msg(i, nation_recno, TALK_PROPOSE_ALLIANCE_TREATY );
					}

					//-- if the nation has significiant trade with us, propose a friendly treaty now --//

					else if( nationPtr->trade_rating(nation_recno) > 10 ||
								nationPtr->ai_trade_with_rating(nation_recno) >= 50 )		// or if the product complement each other very well
					{
						if( should_diplomacy_retry(TALK_PROPOSE_FRIENDLY_TREATY, i) )
							talk_res.ai_send_talk_msg(i, nation_recno, TALK_PROPOSE_FRIENDLY_TREATY );
					}
				}
			}
		}
	}
}
//------ End of function Nation::think_deal_with_one_enemy ------//


//----- Begin of function Nation::think_surrender -----//
//
int Nation::think_surrender()
{
	//--- don't surrender if the nation still has a town ---//

	int rc=0;

	if( total_population == 0 )
		rc = 1;

	if( cash <= 0 && income_365days()==0 )
		rc = 1;

	if( !rc )
		return 0;

	//---- see if there is any nation worth getting our surrender ---//

	Nation* nationPtr;
	int	  curRating, bestRating=0, bestNationRecno=0;

	if( !king_unit_recno )		// if there is no successor to the king, the nation will tend more to surrender
		bestRating = -100;

	for( int i=nation_array.size() ; i>0 ; i-- )
	{
		if( nation_array.is_deleted(i) || i==nation_recno )
			continue;

		if( !get_relation(i)->has_contact )		// don't surrender to a nation without contact
			continue;

		nationPtr = nation_array[i];

		if( nationPtr->cash <= 300 )		// don't surrender to an economically handicapped nation
			continue;

		curRating = ai_surrender_to_rating(i);

		//--- the nation will tend to surrender if there is only a small number of units left ---//

		curRating += 50 - total_unit_count*5;

		if( curRating > bestRating )
		{
			bestRating 		 = curRating;
			bestNationRecno = i;
		}
	}

	//---------------------------------------//

	if( bestNationRecno )
	{
		surrender(bestNationRecno);
		return 1;
	}

	return 0;
}
//------ End of function Nation::think_surrender ------//


//----- Begin of function Nation::think_unite_against_big_enemy -----//
//
int Nation::think_unite_against_big_enemy()
{
	if( info.game_date - info.game_start_date <
		 365 * 3 * (100+pref_military_development) / 100 )		// only do this after 3 to 6 years into the game
	{
		return 0;
	}

	//-----------------------------------------------//

	if( config.ai_aggressiveness < OPTION_HIGH )
		return 0;

	if( config.ai_aggressiveness == OPTION_HIGH )
	{
		if( misc.random(10)!=0 )
			return 0;
	}
	else		// OPTION_VERY_HIGH
	{
		if( misc.random(5)!=0 )
			return 0;
	}

	//---------------------------------------//

	int enemyNationRecno = nation_array.max_overall_nation_recno;

	if( !enemyNationRecno )
		return 0;

	Nation* enemyNation = nation_array[enemyNationRecno];

	//----- only against human players -----//

	if( enemyNation->is_ai() )
		return 0;

	//---- find the overall rank rating of the second most powerful computer kingdom ---//

	Nation* nationPtr;
	int 	  secondBestOverall=0, secondBestNationRecno=0;

	for( int i=nation_array.size() ; i>0 ; i-- )
	{
		if( nation_array.is_deleted(i) || i==enemyNationRecno )
			continue;

		nationPtr = nation_array[i];

		if( !nationPtr->is_ai() )		// don't count human players
			continue;

		if( nationPtr->overall_rank_rating() > secondBestOverall )
		{
			secondBestOverall = nationPtr->overall_rank_rating();
			secondBestNationRecno = i;
		}
	}

	if( !secondBestNationRecno || secondBestNationRecno==nation_recno )
		return 0;

	//------- don't surrender to hostile nation -------//

	if( get_relation_status(secondBestNationRecno) < config_adv.nation_ai_unite_min_relation_level ) // defaults to NATION_NEUTRAL
		return 0;

	//--- if all AI kingdoms are way behind the human players, unite to against the human player ---//

	int compareRating;

	if( config.ai_aggressiveness == OPTION_HIGH )
		compareRating = 50;
	else               			// OPTION_VERY_AGGRESSIVE
		compareRating = 80;

	if( secondBestOverall < compareRating &&
		 secondBestNationRecno != nation_recno )
	{
		surrender(secondBestNationRecno);
		return 1;
	}

	return 0;
}
//------ End of function Nation::think_unite_against_big_enemy ------//


//----- Begin of function Nation::ai_surrender_to_rating -----//
//
// return a rating on how much the nation will tend to surrender
// to the specific nation.
//
int Nation::ai_surrender_to_rating(int nationRecno)
{
	Nation* 			 nationPtr = nation_array[nationRecno];
	NationRelation* nationRelation = get_relation(nationRecno);

	//--- higher tendency to surrender to a powerful nation ---//

	int curRating = nationPtr->overall_rank_rating() - overall_rank_rating();

	curRating += (nationRelation->ai_relation_level-40);

	curRating += (int) nationRelation->good_relation_duration_rating*3;

	curRating += (int) nationPtr->reputation/2;

	//------ shouldn't surrender to an enemy --------//

	if( nationRelation->status == NATION_HOSTILE )
		curRating -= 100;

	//--- if the race of the kings are the same, the chance is higher ---//

	if( race_res.is_same_race( nationPtr->race_id, race_id ) )
		curRating += 20;

	return curRating;
}
//------ End of function Nation::ai_surrender_to_rating ------//


//----- Begin of function Nation::think_eliminate_enemy_town -----//
//
// This function is called to eliminate remaining enemy firms
// when all enemy towns have been destroyed.
//
int Nation::think_eliminate_enemy_town(int enemyNationRecno)
{
	//---- look for enemy firms to attack ----//

	int  hasWar;
	Town *townPtr;

	for( int i=town_array.size() ; i>0 ; i-- )
	{
		if( town_array.is_deleted(i) )
			continue;

		townPtr = town_array[i];

		if( townPtr->nation_recno != enemyNationRecno )
			continue;

		//--- only attack if we have any base town in the enemy town's region ---//

		if( base_town_count_in_region(townPtr->region_id)==0 )
			continue;

		//----- take into account of the mobile units around this town -----//

		int mobileCombatLevel = mobile_defense_combat_level(townPtr->center_x, townPtr->center_y, townPtr->nation_recno, 1, hasWar);

		if( mobileCombatLevel == -1 )		// do not attack this town because a battle is already going on
			continue;

		//---- calculate the combat level of this target town ----//

		int townCombatLevel = townPtr->protection_available();

		return ai_attack_target(townPtr->loc_x1, townPtr->loc_y1, mobileCombatLevel + townCombatLevel);
	}

	return 0;
}
//------ End of function Nation::think_eliminate_enemy_town ------//


//----- Begin of function Nation::think_eliminate_enemy_firm -----//
//
// This function is called to eliminate remaining enemy firms
// when all enemy towns have been destroyed.
//
int Nation::think_eliminate_enemy_firm(int enemyNationRecno)
{
	//---- look for enemy firms to attack ----//

	int  hasWar;
	Firm *firmPtr;

	for( int i=firm_array.size() ; i>0 ; i-- )
	{
		if( firm_array.is_deleted(i) )
			continue;

		firmPtr = firm_array[i];

		if( firmPtr->nation_recno != enemyNationRecno )
			continue;

		//--- only attack if we have any base town in the enemy firm's region ---//

		if( base_town_count_in_region(firmPtr->region_id)==0 )
			continue;

		//----- take into account of the mobile units around this town -----//

		int mobileCombatLevel = mobile_defense_combat_level(firmPtr->center_x, firmPtr->center_y, firmPtr->nation_recno, 1, hasWar);

		if( mobileCombatLevel == -1 )		// do not attack this town because a battle is already going on
			continue;
	
		//---- calculate the combat level of this target firm ----//

		int firmCombatLevel;

		if( firmPtr->firm_id == FIRM_CAMP )                              		// other civilian firms
			firmCombatLevel = ((FirmCamp*)firmPtr)->total_combat_level();
		else
			firmCombatLevel = firmPtr->worker_count * 10;		// civilian firms have very low combat level

		return ai_attack_target(firmPtr->loc_x1, firmPtr->loc_y1, mobileCombatLevel + firmCombatLevel);
	}
	
	return 0;
}
//------ End of function Nation::think_eliminate_enemy_firm ------//


//----- Begin of function Nation::think_eliminate_enemy_unit -----//
//
// This function is called to eliminate remaining enemy firms
// when all enemy towns have been destroyed.
//
int Nation::think_eliminate_enemy_unit(int enemyNationRecno)
{
	Unit *unitPtr;
	int  hasWar;

	for( int i=unit_array.size() ; i>0 ; i-- )
	{
		if( unit_array.is_deleted(i) )
			continue;

		unitPtr = unit_array[i];

		if( unitPtr->nation_recno != enemyNationRecno )
			continue;

		if( !unitPtr->is_visible() || unitPtr->mobile_type != UNIT_LAND )		// only deal with land units now 
			continue;

		//--- only attack if we have any base town in the enemy unit's region ---//

		if( base_town_count_in_region(unitPtr->region_id()) == 0 )
			continue;

		//----- take into account of the mobile units around this town -----//

		int mobileCombatLevel = mobile_defense_combat_level(unitPtr->next_x_loc(), unitPtr->next_y_loc(), unitPtr->nation_recno, 1, hasWar);

		if( mobileCombatLevel == -1 )		// do not attack this town because a battle is already going on
			continue;

		return ai_attack_target(unitPtr->next_x_loc(), unitPtr->next_y_loc(), mobileCombatLevel + (int) unitPtr->unit_power());
	}

	return 0;
}
//------ End of function Nation::think_eliminate_enemy_unit ------//


//----- Begin of function Nation::think_ally_against_big_enemy -----//
//
// Think about allying against a big enemy
//
int Nation::think_ally_against_big_enemy()
{
	if( info.game_date < info.game_start_date + 365 + nation_recno*70 )		// don't ask for tribute too soon, as in the beginning, the ranking are all the same for all nations
		return 0;

	//---------------------------------------//

	int enemyNationRecno = nation_array.max_overall_nation_recno;

	if( enemyNationRecno == nation_recno )
		return 0;

	//-- if AI aggressiveness > high, only deal against the player, but not other kingdoms ---//

	if( config.ai_aggressiveness >= OPTION_HIGH )
	{
		if( nation_array[enemyNationRecno]->is_ai() )
			return 0;
	}

	//-- if AI aggressiveness is low, don't do this against the human player --//

	else if( config.ai_aggressiveness == OPTION_LOW )
	{
		if( !nation_array[enemyNationRecno]->is_ai() )
			return 0;
	}

	//--- increase the ai_relation_level towards other nations except the enemy so we can ally against the enemy ---//

	Nation* enemyNation = nation_array[enemyNationRecno];
	int     incRelationLevel = (100-overall_rank_rating())/10;

	int i;
	for( i=nation_array.size() ; i>0 ; i-- )
	{
		if( nation_array.is_deleted(i) )
			continue;

		if( i==nation_recno || i==enemyNationRecno )
			continue;

		int thisIncLevel = incRelationLevel * (100-get_relation(i)->ai_relation_level) / 100;

		change_ai_relation_level( i, thisIncLevel );
	}

	//---- don't have all nations doing it the same time ----//

	if( misc.random(nation_array.ai_nation_count)==0 )
		return 0;

	//---- if the trade rating is high, stay war-less with it ----//

	if( trade_rating(enemyNationRecno) +
		 ai_trade_with_rating(enemyNationRecno) > 100 - pref_trading_tendency/3 )
	{
		return 0;
	}

	//---- if the nation relation level is still high, then request aid/tribute ----//

	NationRelation* nationRelation = get_relation(enemyNationRecno);

	if( nationRelation->ai_relation_level > 30 )
	{
		int talkId;

		if( nationRelation->status >= NATION_FRIENDLY )
			talkId = TALK_DEMAND_AID;
		else
			talkId = TALK_DEMAND_TRIBUTE;

		if( should_diplomacy_retry(talkId, enemyNationRecno) )
		{
			static short aidAmountArray[] = { 500, 1000, 2000 };

			int aidAmount = aidAmountArray[misc.random(3)];

			talk_res.ai_send_talk_msg(enemyNationRecno, nation_recno, talkId, aidAmount);
		}

		return 0;
	}

	//-------------------------------------//

	Nation* nationPtr;

	NationRelation *ourNationRelation, *enemyNationRelation;

	for( i=nation_array.size() ; i>0 ; i-- )
	{
		if( nation_array.is_deleted(i) )
			continue;

		if( i==nation_recno || i==enemyNationRecno )
			continue;

		nationPtr = nation_array[i];

		ourNationRelation   = get_relation(i);
		enemyNationRelation = enemyNation->get_relation(i);

	}

	return 0;
}
//------ End of function Nation::think_ally_against_big_enemy ------//


//----- Begin of function Nation::ai_should_attack_friendly -----//
//
// This function returns whether this nation should attack
// the given friendly nation.
//
// <int> friendlyNationRecno - the nation recno of the friendly nation.
// <int> attackTemptation    - a rating from 0 to 100 indicating
//										 temptation of attacking the target.
//
int Nation::ai_should_attack_friendly(int friendlyNationRecno, int attackTemptation)
{
	Nation  			*friendlyNation = nation_array[friendlyNationRecno];
	NationRelation *nationRelation = get_relation(friendlyNationRecno);

	//--- don't change terminate treaty too soon ---//

	if( info.game_date < nationRelation->last_change_status_date+60+pref_honesty/2 )		// only after 60 to 110 days
		return 0;

	//------------------------------------------------//

	int resistanceRating = friendlyNation->military_rank_rating()
								  - military_rank_rating();

	resistanceRating += nationRelation->ai_relation_level - 50;

	resistanceRating += trade_rating(friendlyNationRecno);

	return attackTemptation > resistanceRating;
}
//------ End of function Nation::ai_should_attack_friendly ------//


//----- Begin of function Nation::ai_end_treaty -----//
//
// Terminate the treaty with the given nation.
//
// <int> nationRecno - the nation to terminate treaty with.
//
void Nation::ai_end_treaty(int nationRecno)
{
	NationRelation *nationRelation = get_relation(nationRecno);

	err_when( nationRelation->status < NATION_FRIENDLY );

	if( nationRelation->status == NATION_FRIENDLY )
	{
		talk_res.ai_send_talk_msg(nationRecno, nation_recno, TALK_END_FRIENDLY_TREATY, 0, 0, 1);		// 1-force send
	}
	else if( nationRelation->status == NATION_ALLIANCE )
	{
		talk_res.ai_send_talk_msg(nationRecno, nation_recno, TALK_END_ALLIANCE_TREATY, 0, 0, 1);
	}

	err_when( nationRelation->status >= NATION_FRIENDLY );		// when the status is still friendly or alliance
}
//------ End of function Nation::ai_end_treaty ------//


//----- Begin of function Nation::ai_has_enough_food -----//
//
// return whether this nation has enough food or not.
//
int Nation::ai_has_enough_food()
{
	return food > 2000 + 2000 * pref_food_reserve / 100
			 || yearly_food_change() > 0;
}
//------ End of function Nation::ai_has_enough_food ------//


//----- Begin of function Nation::think_attack_enemy_firm -----//
//
// Think about attacking a specific type of firm of a specific enemy.
//
int Nation::think_attack_enemy_firm(int enemyNationRecno, int firmId)
{
	if( !largest_town_recno )
		return 0;

	Town*   ourLargestTown = town_array[largest_town_recno];
	Nation  *nationPtr = nation_array[enemyNationRecno];
	Firm    *firmPtr, *targetFirm=NULL;
	int     curRating, bestRating=0, targetCombatLevel, hasWar;

	for( int i=firm_array.size() ; i>0 ; i-- )
	{
		if( firm_array.is_deleted(i) )
			continue;

		firmPtr = firm_array[i];

		if( firmPtr->firm_id != firmId ||
			 firmPtr->nation_recno != enemyNationRecno )
		{
			continue;
		}

		int combatLevel = enemy_firm_combat_level(firmPtr, 1, hasWar);

		if( combatLevel==0 )		// no protection with this mine
		{
			targetFirm = firmPtr;
			targetCombatLevel = combatLevel;
			break;
		}

		curRating = world.distance_rating( firmPtr->center_x, firmPtr->center_y,
						ourLargestTown->center_x, ourLargestTown->center_y );

		curRating += 1000 - combatLevel/5;

		if( curRating > bestRating )
		{
			bestRating = curRating;
			targetFirm = firmPtr;
			targetCombatLevel = combatLevel;
		}
	}

	if( !targetFirm )
		return 0;

	//---------------------------------------------//

	int useAllCamp=1;

	return ai_attack_target( targetFirm->loc_x1, targetFirm->loc_y1,
									 targetCombatLevel, 0, 0, 0, 0, useAllCamp );
}
//------ End of function Nation::think_attack_enemy_firm ------//


//----- Begin of function Nation::think_against_mine_monopoly -----//
//
int Nation::think_against_mine_monopoly()
{
	//-- only think this after the game has been running for at least one year --//

	if( config.ai_aggressiveness < OPTION_HIGH )		// only attack if aggressiveness >= high
		return 0;

	if( info.game_date - info.game_start_date > 365 )
		return 0;

	if( profit_365days() > 0 )		// if we are making a profit, don't attack
		return 0;

	//-- for high aggressiveness, it will check cash before attack, for very high aggressiveness, it won't check cash before attack ---//

	if( config.ai_aggressiveness < OPTION_VERY_HIGH )		// only attack if aggressiveness >= high
	{
		if( cash > 2000 + 1000 * pref_cash_reserve / 100 )		// only attack if we run short of cash
			return 0;
	}

	//--------------------------------------------------------//

	if( !largest_town_recno )
		return 0;

	//--------------------------------------------------//

	int baseRegionId = town_array[largest_town_recno]->region_id;

	// no region stat (region is too small), don't care
	if( !region_array[baseRegionId]->region_stat_id )
		return 0;

	RegionStat* regionStat = region_array.get_region_stat(baseRegionId);

	//---- if we already have a mine in this region ----//

	if( regionStat->mine_nation_count_array[nation_recno-1] > 0 )
		return 0;

	//----- if there is no mine in this region -----//

	if( regionStat->raw_count == 0 )
		return 0;

	//----- if enemies have occupied all mines -----//

	int mineCount, totalMineCount=0;
	int curRating, bestRating=0, targetNationRecno=0;

	int i;
	for( i=nation_array.size() ; i>0 ; i-- )
	{
		if( nation_array.is_deleted(i) )
			continue;

		//------ only deal with human players ------//

		if( nation_array[i]->is_ai() || i==nation_recno )
			continue;

		//------------------------------------------//

		mineCount = regionStat->mine_nation_count_array[i-1];
		totalMineCount += mineCount;

		curRating = mineCount * 100
						- get_relation(i)->ai_relation_level
						- trade_rating(i);

		if( curRating > bestRating )
		{
			bestRating 		   = curRating;
			targetNationRecno = i;
		}
	}

	if( !targetNationRecno )
		return 0;

	//--- if the relationship with this nation is still good, don't attack yet, ask for aid first ---//

	NationRelation* nationRelation = get_relation(targetNationRecno);

	if( nationRelation->ai_relation_level > 30 )
	{
		int talkId;

		if( nationRelation->status >= NATION_FRIENDLY )
			talkId = TALK_DEMAND_AID;
		else
			talkId = TALK_DEMAND_TRIBUTE;

		if( should_diplomacy_retry(talkId, targetNationRecno) )
		{
			static short aidAmountArray[] = { 500, 1000, 2000 };

			int aidAmount = aidAmountArray[misc.random(3)];

			talk_res.ai_send_talk_msg(targetNationRecno, nation_recno, talkId, aidAmount);
		}

		return 0;
	}

	//------- attack one of the target enemy's mines -------//

	Firm* firmPtr;

	for( i=firm_array.size() ; i>0 ; i-- )
	{
		if( firm_array.is_deleted(i) )
			continue;

		firmPtr = firm_array[i];

		if( firmPtr->firm_id != FIRM_MINE ||
			 firmPtr->nation_recno != targetNationRecno ||
			 firmPtr->region_id != baseRegionId )
		{
			continue;
		}

		//--------------------------------------------//

		int hasWar;
		int targetCombatLevel = enemy_firm_combat_level(firmPtr, 1, hasWar);

		return ai_attack_target( firmPtr->loc_x1, firmPtr->loc_y1,
										 targetCombatLevel, 0, 0, 0, 0, 1 );		// 1-use all camps
	}

	return 0;
}
//------ End of function Nation::think_against_mine_monopoly ------//

