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

//Filename    : OF_CAMP2.CPP
//Description : Firm Military Camp - AI functions

#include <ONATION.h>
#include <OINFO.h>
#include <OCONFIG.h>
#include <OTOWN.h>
#include <OSPY.h>
#include <OUNIT.h>
#include <OF_CAMP.h>

//--------- Begin of function FirmCamp::init_derived ---------//

void FirmCamp::init_derived()
{
	patrol_unit_count = 0;
	coming_unit_count = 0;

	ai_capture_town_recno = 0;
	ai_recruiting_soldier = 1;
}
//----------- End of function FirmCamp::init_derived -----------//


//--------- Begin of function FirmCamp::process_ai ---------//

void FirmCamp::process_ai()
{
	if( info.game_date%15==firm_recno%15 )		// do not call too often as the penalty of accumulation is 10 days
		think_use_cash_to_capture();

	//------- assign overseer and workers -------//

	if( info.game_date%15==firm_recno%15 )		// do not call too often because when an AI action is queued, it will take a while to carry it out
		think_recruit();

	//---- if this firm is currently trying to capture a town ----//

	if( ai_capture_town_recno )
	{
		should_close_flag = 0;		// disable should_close_flag when trying to capture a firm, should_close_flag is set in process_common_ai()

		if( nation_array[nation_recno]->attack_camp_count==0 &&		// only when attack_camp_count==0, the attack mission is complete
			 patrol_unit_count==0 )
		{
			ai_capture_town_recno = 0;
			defense_flag = 1;    			// turn it on again after the capturing plan is finished
			return;
		}

//		process_ai_capturing();
		return;
	}

	//--- if the firm is empty and should be closed, sell/destruct it now ---//

	if( should_close_flag && worker_count==0 && patrol_unit_count==0
		&& coming_unit_count==0 && ai_status != CAMP_IN_DEFENSE )
	{
		ai_del_firm();
		return;
	}

	//----- think about assigning a better commander -----//

	if( info.game_date%30==firm_recno%30 )
		think_assign_better_commander();

	//----- think about changing links to foreign town -----//

	if( info.game_date%30==firm_recno%30 )
		think_change_town_link();

	//------ think about attacking enemies nearby -------//

	int checkInterval = 13 - nation_array[nation_recno]->pref_military_development/10;

	if( info.game_date%checkInterval == firm_recno%checkInterval )
		think_attack_nearby_enemy();

	//------ think about capturing independent town -------//

	static short interval_days_array[] = { 60, 30, 20, 10 };

	int intervalDays = interval_days_array[config.ai_aggressiveness-1];

	if( info.game_date%intervalDays == firm_recno%intervalDays )		// do not call too often because when an AI action is queued, it will take a while to carry it out
		think_capture();

	//---------------------------------------//

	if( info.game_date%30 == firm_recno%30 )
	{
		ai_reset_defense_mode(); // reset defense mode if all soldiers are dead
	}
}
//----------- End of function FirmCamp::process_ai -----------//


//--------- Begin of function FirmCamp::ai_reset_defense_mode ---------//

void FirmCamp::ai_reset_defense_mode()
{
	if(ai_status!=CAMP_IN_DEFENSE)
		return;

	//------------------------------------------------------------//
	// reset defense mode if all soldiers are dead
	//------------------------------------------------------------//
	DefenseUnit *defPtr = defense_array;
	Unit	*unitPtr;
	int	found=0;

	for(int i=0; i<=MAX_WORKER; i++, defPtr++)
	{
		if(!defPtr->unit_recno)
			continue; // empty slot

		if(unit_array.is_deleted(defPtr->unit_recno))
			continue;

		unitPtr = unit_array[defPtr->unit_recno];
		if(unitPtr->nation_recno==nation_recno && unitPtr->action_misc==ACTION_MISC_DEFENSE_CAMP_RECNO &&
			unitPtr->action_misc_para==firm_recno) // is a soldier of this camp
			found++;
	}

	if(!found)
	{
		set_employ_worker(1); // all soldiers have died, reset defense mode to employ new workers
		memset(defense_array, 0, sizeof(DefenseUnit)*(MAX_WORKER+1));
	}
}
//----------- End of function FirmCamp::ai_reset_defense_mode -----------//


//--------- Begin of function FirmCamp::process_ai_capturing ---------//
//
// This function is called when the AI is in the process of
// attacking and capturing the independent town this camp is
// linked to.
//
void FirmCamp::process_ai_capturing()
{
	err_when( patrol_unit_count <= 0 );		// this function shouldn't be called at all if patrol_unit_count==0
	err_when( patrol_unit_count>9 );

	if( !ai_capture_town_recno )
		return;

	if( think_capture_return() )	// if the capturing units should now return to their base.
		return;

	//--- there are still town defender out there, order idle units to attack them ---//

	Unit* unitPtr;
	Town* townPtr = town_array[ai_capture_town_recno];

	if( townPtr->town_defender_count > 0 )
	{
		for( int i=patrol_unit_count ; i>0 ; i-- )
		{
			unitPtr = unit_array[ patrol_unit_array[i-1] ];

			if( unitPtr->cur_action == SPRITE_IDLE )
				ai_attack_town_defender(unitPtr);
		}
	}

	//--- if the town is still not captured but all mobile town defenders are destroyed, attack the town again ---//

	if( townPtr->town_defender_count == 0 )
	{
		//----- have one of the units attack the town ----//

		short unitArray[1];

		err_when( patrol_unit_count<=0 );

		unitArray[0] = patrol_unit_array[ misc.random(patrol_unit_count) ];

		err_when( unit_array.is_deleted(unitArray[0]) );

		if( townPtr->nation_recno )
			nation_array[nation_recno]->set_relation_should_attack( townPtr->nation_recno, 1, COMMAND_AI );

		// ##### patch begin Gilbert 5/8 ######//
		unit_array.attack(townPtr->loc_x1, townPtr->loc_y1, 0, unitArray, 1, COMMAND_AI, 0);
		// ##### patch end Gilbert 5/8 ######//
	}
}
//----------- End of function FirmCamp::process_ai_capturing -----------//


//------ Begin of function FirmCamp::ai_attack_town_defender ------//
//
void FirmCamp::ai_attack_town_defender(Unit* attackerUnit)
{
	int shouldAttackUnit=0;

	if( attackerUnit->cur_action == SPRITE_IDLE )
		shouldAttackUnit = 1;

	else if( attackerUnit->cur_action == SPRITE_ATTACK )
	{
		//--- if this unit is currently attacking the town, ask it to attack a defender unit ---//

		Town* townPtr = town_array[ai_capture_town_recno];

		if( attackerUnit->action_x_loc==townPtr->loc_x1 && attackerUnit->action_y_loc==townPtr->loc_y1 )
			shouldAttackUnit = 1;
	}

	if( !shouldAttackUnit )
		return;

	//---- if there are still town defenders out there ---//

	int unitCount = unit_array.size();
	int unitRecno = misc.random(unitCount)+1;		// scan randomly
	short unitArray[1];
	Unit* targetUnit;

	for( int i=0 ; i<unitCount ; i++ )
	{
		if( ++unitRecno > unitCount )
			unitRecno = 1;

		if( unit_array.is_deleted(unitRecno) )
			continue;

		targetUnit = unit_array[unitRecno];

		if( targetUnit->unit_mode == UNIT_MODE_DEFEND_TOWN &&
			 targetUnit->unit_mode_para == ai_capture_town_recno )
		{
			unitArray[0] = attackerUnit->sprite_recno;

			if( targetUnit->nation_recno )
				nation_array[nation_recno]->set_relation_should_attack( targetUnit->nation_recno, 1, COMMAND_AI );

			// ##### patch begin Gilbert 5/8 ######//
			unit_array.attack( targetUnit->next_x_loc(), targetUnit->next_y_loc(),
									 0, unitArray, 1, COMMAND_AI, targetUnit->sprite_recno );
			// ##### patch end Gilbert 5/8 ######//
			break;
		}
	}
}
//-------- End of function FirmCamp::ai_attack_town_defender ------//


//--------- Begin of function FirmCamp::think_recruit ---------//
//
// Think about recruiting an overseer and soliders to this base.
//
void FirmCamp::think_recruit()
{
	if( patrol_unit_count )		// if there are units of this camp patrolling outside
		return;

	Nation* nationPtr = nation_array[nation_recno];

	ai_recruiting_soldier = 1; 		// the AI is currently trying to recruit soldiers

	//---- if there are currently units coming to this firm ----//

	if( coming_unit_count > 0 )
	{
		Unit* unitPtr;

		for( int i=0 ; i<coming_unit_count ; i++ )
		{
			if( unit_array.is_deleted( coming_unit_array[i] ) )
				continue;

			unitPtr = unit_array[ coming_unit_array[i] ];

			//--- check if any of them are still on their way to this firm ---//

			if( unitPtr->nation_recno == nation_recno &&
				 unitPtr->action_mode == ACTION_ASSIGN_TO_FIRM )
			{
				//--- if so, do not do anything unit they are all done ---//

				return;
			}
		}

		coming_unit_count = 0;
	}

	//-- if this camp is empty, think about move a whole troop from a useless camp (should_ai_close()==1)

	if( !overseer_recno && worker_count==0 &&
		 nationPtr->firm_should_close_array[FIRM_CAMP-1] > 0 )
	{
		FirmCamp *firmCamp, *bestCampPtr=NULL;
		int		bestRating=0, curRating;

		//--- see if there are any useless camps around and pick the most suitable one ---//

		for( int i=nationPtr->ai_camp_count-1 ; i>=0 ; i-- )
		{
			firmCamp = (FirmCamp*) firm_array[ nationPtr->ai_camp_array[i] ];

			err_when( firmCamp->firm_id != FIRM_CAMP );

			if( firmCamp->region_id != region_id )
				continue;

			if( firmCamp->should_close_flag && 
				 (firmCamp->overseer_recno || firmCamp->worker_count>0) )
			{
				curRating = 100 - misc.points_distance( center_x, center_y,
								firmCamp->center_x, firmCamp->center_y );

				if( curRating > bestRating )
				{
					bestRating = curRating;
					bestCampPtr = firmCamp;
				}
			}
		}

		//--------- start the move now -------//

		if( bestCampPtr )
		{
			bestCampPtr->patrol();

			if( bestCampPtr->patrol_unit_count==0 )		// there could be chances that there are no some for mobilizing the units
				return;

			//--- set coming_unit_count for later checking ---//

			err_when( sizeof(coming_unit_array) != sizeof(patrol_unit_array) );

			memcpy( coming_unit_array, bestCampPtr->patrol_unit_array, sizeof(coming_unit_array) );
			coming_unit_count = bestCampPtr->patrol_unit_count;

			//---- order the unit to move to this camp now ---//

			unit_array.assign(loc_x1, loc_y1, 0, COMMAND_AI, bestCampPtr->patrol_unit_array, bestCampPtr->patrol_unit_count);

			//------ delete the camp as it no longer has any use ----//
			
			bestCampPtr->ai_del_firm();
			return;
		}
	}

	//------- get an overseer if there isn't any right now -----//

	if( !overseer_recno )
		nationPtr->add_action(loc_x1, loc_y1, -1, -1, ACTION_AI_ASSIGN_OVERSEER, FIRM_CAMP);

	//---- think about the no. of workers needed for this base ----//

	int combatDiff;

	if( overseer_recno == nationPtr->king_unit_recno )		// recruit as many soldiers as possible if the commander is the king
	{
		combatDiff = 1000;
	}
	else if( nationPtr->total_jobless_population > 20 + (100-nationPtr->pref_military_development) / 3 )		// 20 to 53
	{
		combatDiff = 1000;		// recruit as many as possible
	}
	else
	{
		int combatLevelNeeded = ai_combat_level_needed();

		combatDiff = combatLevelNeeded - total_combat_level();
	}

	if( combatDiff > 0 )
	{
		ai_recruit(combatDiff);
	}
	else
	{
		if( overseer_recno )
			ai_recruiting_soldier = 0;		// this firm has enough soldiers already
	}
}
//----------- End of function FirmCamp::think_recruit ----------//


//--------- Begin of function FirmCamp::ai_recruit ---------//
//
// <int> recruitCombatLevel - the combat level needed to be added.
//
// return: <int> 1-succeeded, 0-failed.
//
int FirmCamp::ai_recruit(int recruitCombatLevel)
{
	if( worker_count == MAX_WORKER || !overseer_recno )
		return 0;

	int recruitCount = MAX( 1, recruitCombatLevel / 20 );

	recruitCount = MIN( recruitCount, MAX_WORKER-worker_count );

	//--- first try to recruit soldiers directly from a linked village ---//

	int 	majorityRace = majority_race();
	int	raceId;
	int	loopCount=0;
	Town* townPtr;

	for( int i=0 ; i<linked_town_count ; i++ )
	{
		townPtr = town_array[ linked_town_array[i] ];

		if( townPtr->nation_recno != nation_recno || !townPtr->jobless_population )
			continue;

		//-- recruit majority race first, but will also consider other races --//

		raceId = MAX( 1, majorityRace );

		for( int j=0 ; j<MAX_RACE ; j++ )
		{
			if( ++raceId > MAX_RACE )
				raceId = 1;

			//--- if the loyalty is too low, reward before recruiting ---//

			if( townPtr->jobless_race_pop_array[raceId-1] > 0 &&
				 townPtr->race_loyalty_array[raceId-1] < 40 )
			{
				if( townPtr->accumulated_reward_penalty > 30 )		// if the reward penalty is too high, do reward
					break;

				townPtr->reward(COMMAND_AI);
			}

			//---- recruit the soldiers we needed ----//

			while( townPtr->can_recruit(raceId) )
			{
				err_when( ++loopCount > 1000 );

				pull_town_people(townPtr->town_recno, COMMAND_AI, raceId, 1);			// last 1-force pulling people from the town to the firm

				if( --recruitCount == 0 )
					return 1;
			}
		}
	}

	//------ next, try to recruit from remote villages -----//

	if( recruitCount > 0 )
		nation_array[nation_recno]->add_action(loc_x1, loc_y1, -1, -1, ACTION_AI_ASSIGN_WORKER, FIRM_CAMP, recruitCount);

	return 1;
}
//----------- End of function FirmCamp::ai_recruit ----------//


//--------- Begin of function FirmCamp::ai_combat_level_needed ---------//
//
// Think about the no. of soldiers needed by this base.
//
int FirmCamp::ai_combat_level_needed()
{
	Town*	  townPtr;
	int  	  combatNeeded=0;
	Nation* nationPtr = nation_array[nation_recno];

	//------- scan for linked towns ---------//

	for( int i=0 ; i<linked_town_count ; i++ )
	{
		townPtr = town_array[ linked_town_array[i] ];

		//------- this is its own town -------//

		if( townPtr->nation_recno == nation_recno )
		{
			if( townPtr->should_ai_migrate() ) 	// no need for this if this town is going to migrate
				continue;

			combatNeeded += townPtr->population * 10;	// 30 people need 300 combat levels

			if( townPtr->is_base_town )
				combatNeeded += townPtr->population * 10;		// double the combat level need for base town
		}
	}

	//--- if the overseer is the king, increase its combat level needed ---//

	if( overseer_recno && unit_array[overseer_recno]->rank_id == RANK_KING )
		combatNeeded = MAX(400, combatNeeded);

	//---------------------------------------//

	return combatNeeded;
}
//----------- End of function FirmCamp::ai_combat_level_needed ----------//


//--------- Begin of function FirmCamp::total_combat_level ---------//
//
// Total combat level of all soldiers and commander in the base.
// The leadership of the general also applies to the final combat level.
//
// return: <int> the total combat level - it is the sum of hit points
//					  of all the units in the camp.
//
int FirmCamp::total_combat_level()
{
	int 	  totalCombatLevel=0;
	Worker* workerPtr = worker_array;

	for( int i=0 ; i<worker_count ; i++, workerPtr++ )
	{
		totalCombatLevel += workerPtr->hit_points;			// use it points instead of combat level because hit_points represent both combat level and hit points left

		err_when( totalCombatLevel < 0 );

		//---- the combat level of weapons are higher ------//

		UnitInfo* unitInfo = unit_res[workerPtr->unit_id];

		if( unitInfo->unit_class == UNIT_CLASS_WEAPON )
			totalCombatLevel += (unitInfo->weapon_power + workerPtr->extra_para - 1) * 30;		// extra_para keeps the weapon version

		err_when( totalCombatLevel < 0 );
	}

	if( overseer_recno )
	{
		Unit* unitPtr = unit_array[overseer_recno];

		//--- the commander's leadership effects over the soldiers ---//

		totalCombatLevel += totalCombatLevel * unitPtr->skill.skill_level / 150;		// divided by 150 instead of 100 because while the attacking ability of the unit is affected by the general, the hit points isn't, so we shouldn't do a direct multiplication.

		//------ the leader's own hit points ------//

		totalCombatLevel += (int) unitPtr->hit_points;

		err_when( totalCombatLevel < 0 );
	}

	return totalCombatLevel;
}
//----------- End of function FirmCamp::total_combat_level ----------//


//--------- Begin of function FirmCamp::average_combat_level ---------//
//
int FirmCamp::average_combat_level()
{
	int personCount = worker_count + (overseer_recno>0);

	if( personCount > 0 )
		return total_combat_level() / personCount;
	else
		return 0;
}
//----------- End of function FirmCamp::average_combat_level ----------//


//--------- Begin of function FirmCamp::ai_should_close ---------//
//
// Whether this AI firm should be closed or not.
//
int FirmCamp::ai_should_close()
{
	return linked_town_count==0 && linked_firm_count==0;
}
//----------- End of function FirmCamp::ai_should_close ----------//


//--------- Begin of function FirmCamp::think_capture ---------//
//
// Think about capturing towns.
//
void FirmCamp::think_capture()
{
	if( is_attack_camp )		// if this camp has been assigned to an attack mission already
		return;

	int targetTownRecno = think_capture_target_town();

	if( !targetTownRecno )
		return;

	//----- if the target town is a nation town -----//

	Town* targetTown = town_array[targetTownRecno];
	Nation* ownNation = nation_array[nation_recno];

	if( targetTown->nation_recno )
	{
		//--- if there are any defenses (camps and mobile units) on the target town, destroy them all first -----//

		if( ownNation->attack_enemy_town_defense(targetTown) != -1 )		// only proceed further when the result is -1, which means no defense on the target town, no attacking is needed.
			return;
	}

	//------ check if the town people will go out to defend -----//

	float thisResistance, resistanceDiff;
	int   defenseCombatLevel=0;

	for( int i=0 ; i<MAX_RACE ; i++ )
	{
		if( targetTown->race_pop_array[i] < 5 )		// if the pop count is lower than 5, ingore it
			continue;

		if( targetTown->nation_recno )
		{
			thisResistance = targetTown->race_loyalty_array[i];
			resistanceDiff = thisResistance - MIN_NATION_DEFEND_LOYALTY;
		}
		else
		{
			thisResistance = targetTown->race_resistance_array[i][nation_recno-1];
			resistanceDiff = thisResistance - MIN_INDEPENDENT_DEFEND_LOYALTY;
		}

		if( resistanceDiff >= 0 )
		{
			float resistanceDecPerDefender = thisResistance/targetTown->race_pop_array[i];  // resistance decrease per new defender

			int defenderCount = int(resistanceDiff / resistanceDecPerDefender)+1;		// no. of defenders will go out if you attack this town

			defenseCombatLevel += targetTown->town_combat_level * 2 * defenderCount;	// *2 is defenseCombatLevel is actually the sum of hit points, not combat level 
		}
	}

	//--- try using spies if there are defense forces and the nation likes to use spies ---//

	if( defenseCombatLevel > 0 ) 		// && ownNation->pref_spy >= 50 && misc.random(3)==0 )		// 1/3 chance of using spies here, otherwise only use spies when we are not strong enough to take over the village by force
	{
		if( targetTown->nation_recno==0 )		// if the camp is trying to capture an independent town, the leadership and race id. of the overseer matters.
		{
			if( think_assign_better_overseer(targetTown) )		// a better general is being assigned to this firm, wait for it
				return;

			if( think_capture_use_spy(targetTown) )
				return;

			if( defenseCombatLevel > 100+ownNation->pref_military_courage &&
				 resistanceDiff > (100-ownNation->pref_peacefulness)/5  )		// depending on the peacefulness, the nation won't attack if resistance > (0-20)
			{
				return;
			}
		}
		else
		{
			//--- don't attack if the target nation's military rating is higher than ours ---//

			if( nation_array[targetTown->nation_recno]->military_rank_rating()
				 > ownNation->military_rank_rating() )
			{
				return;
			}
		}
	}

	//------ send out troops to capture the target town now ----//

	int rc;

	if( targetTown->nation_recno )
		rc = ai_capture_enemy_town(targetTown, defenseCombatLevel);
	else
		rc = ai_capture_independent_town(targetTown, defenseCombatLevel);

	//-- use the same approach to capture both enemy and independent towns --//

	if( rc )
	{
		ai_capture_town_recno = targetTownRecno;
		defense_flag 			 = 0;			// turn off the defense flag during capturing so the general is staying in the base to influence the town

		//--- as the current commander has been out to attack the town by ai_attack_target(), we need to assign him back to the camp for influencing the town and eventually capture it ---//

		if( !overseer_recno && targetTown->nation_recno && patrol_unit_count>0 )
			unit_array[ patrol_unit_array[0] ]->assign(loc_x1, loc_y1);
	}
}
//----------- End of function FirmCamp::think_capture -----------//


//--------- Begin of function FirmCamp::think_capture_target_town ---------//
//
// Think about which town to capture.
//
// Return: <int> recno of the target town.
//
int FirmCamp::think_capture_target_town()
{
	if( !linked_town_count || !overseer_recno )
		return 0;

	//--- if there are any units currently being assigned to this camp ---//

	if( nation_array[nation_recno]->is_action_exist( loc_x1, loc_y1, -1, -1, ACTION_AI_ASSIGN_WORKER, FIRM_CAMP ) )
		return 0;

	//-- decide which town to attack (only when the camp is linked to more than one town ---//

	int	curResistance, curTargetResistance, resistanceDec;
	int 	i, minResistance=100, bestTownRecno=0;
	Town* townPtr;
	int   prefPeacefulness = nation_array[nation_recno]->pref_peacefulness;
	Nation* ownNation = nation_array[nation_recno];
	int   overseerRaceId = unit_array[overseer_recno]->race_id;

	for( i=0 ; i<linked_town_count ; i++ )
	{
		townPtr = town_array[ linked_town_array[i] ];

		if( townPtr->nation_recno == nation_recno )
			continue;

		//------- if it's an independent town -------//

		if( !townPtr->nation_recno )		// only capture independent town
		{
			curResistance = townPtr->average_resistance(nation_recno);
			curTargetResistance = townPtr->average_target_resistance(nation_recno);

			resistanceDec = curResistance - curTargetResistance;

			//------- if the resistance is decreasing ---------//

			if( resistanceDec > 0 &&
				 curResistance > 25-25*ownNation->pref_peacefulness/100 &&		// for nation that has a high peacefulness preference they will wait for the loyalty to fall and try not to attack the town unless necessary
				 townPtr->race_pop_array[overseerRaceId-1] >= 5 )		// if it's less than 5, don't count it, as that it will be easy to attack
			{
				continue;		// let it decrease until it can no longer decrease
			}
		}
		else	//-------- if it's a nation town ---------//
		{
			NationRelation* nationRelation = ownNation->get_relation(townPtr->nation_recno);

			if( nationRelation->status != NATION_HOSTILE )
				continue;

			curResistance = townPtr->average_loyalty();
		}

		//--------------------------------------//

		if( curResistance < minResistance )
		{
			minResistance = curResistance;
			bestTownRecno = townPtr->town_recno;
		}
	}

	return bestTownRecno;
}
//--------- End of function FirmCamp::think_capture_target_town --------//


//--------- Begin of function FirmCamp::ai_capture_independent_town -------//
//
// Try to capture the given independent town.
//
int FirmCamp::ai_capture_independent_town(Town* targetTown, int defenseCombatLevel)
{
	//---- see if the force is strong enough to attack the town ----//

	Nation* nationPtr = nation_array[nation_recno];

	int curCombatLevel = total_combat_level();		// total combat level

	int combatDiff = defenseCombatLevel * (150+nationPtr->pref_force_projection/2) / 100
						  - curCombatLevel;					// try to recruit soldiers based on the force projection perference

	if( combatDiff > 0 )
	{
		if( ai_recruit(combatDiff) )    	// try to recruit new soldiers to increase the combat ability of the troop
			return 0;
	}

	combatDiff = defenseCombatLevel * (200-nationPtr->pref_military_courage/2) / 100
					 - curCombatLevel;

	//---------- attack the target town now ----------//

	if( nation_array[nation_recno]->ai_attack_target(targetTown->loc_x1, targetTown->loc_y1, defenseCombatLevel, 0, 0, 0, firm_recno) )
		return 1;

	return 0;
}
//--------- End of function FirmCamp::ai_capture_independent_town --------//


//--------- Begin of function FirmCamp::think_capture_return ---------//
//
// Think about if the capturing units should now return to their base.
//
int FirmCamp::think_capture_return()
{
	//----- if the target town is destroyed ------//

	int returnCamp=0;

	if( town_array.is_deleted(ai_capture_town_recno) )
	{
		returnCamp = 1;
	}
	else //---- check whether the town has been captured ----//
	{
		Town* townPtr = town_array[ai_capture_town_recno];

		if( townPtr->nation_recno == nation_recno )		// the town has been captured
			returnCamp = 1;
	}

	//-------- if should return to the camp now ---------//

	if( returnCamp )
	{
		Unit* unitPtr;

		for( int i=0; i<patrol_unit_count ; i++ )
		{
			unitPtr = unit_array[ patrol_unit_array[i] ];

			if( unitPtr->is_visible() )
				unitPtr->assign(loc_x1, loc_y1);
		}

		ai_capture_town_recno = 0;		// turn it on again after the capturing plan is finished
		defense_flag = 1;
		return 1;
	}

	return 0;
}
//----------- End of function FirmCamp::think_capture_return -----------//


//--------- Begin of function FirmCamp::think_assign_better_overseer -------//
//
int FirmCamp::think_assign_better_overseer(Town* targetTown)
{
	//----- check if there is already a queued action -----//

	Nation* ownNation = nation_array[nation_recno];

	if( ownNation->is_action_exist( loc_x1, loc_y1, -1, -1,
		 ACTION_AI_ASSIGN_OVERSEER, FIRM_CAMP ) )
	{
		return 1;		// there is a queued action being processed already
	}

	//------ get the two most populated races of the town ----//

	int mostRaceId1, mostRaceId2;

	targetTown->get_most_populated_race(mostRaceId1, mostRaceId2);

	//-- if the resistance of the majority race has already dropped to its lowest possible --//

	if( targetTown->race_resistance_array[mostRaceId1-1][nation_recno-1] <=
		 (float) (targetTown->race_target_resistance_array[mostRaceId1-1][nation_recno-1]+1) )
	{
		if( targetTown->race_resistance_array[mostRaceId1-1][nation_recno-1] > 30 )
		{
			if( think_assign_better_overseer2(targetTown->town_recno, mostRaceId1) )
				return 1;
		}
	}

	//-- if the resistance of the 2nd majority race has already dropped to its lowest possible --//

	if( targetTown->race_resistance_array[mostRaceId2-1][nation_recno-1] <=
		 (float) (targetTown->race_target_resistance_array[mostRaceId2-1][nation_recno-1]+1) )
	{
		if( targetTown->race_resistance_array[mostRaceId2-1][nation_recno-1] > 30 )
		{
			if( think_assign_better_overseer2(targetTown->town_recno, mostRaceId2) )
				return 1;
		}
	}

	return 0;
}
//--------- End of function FirmCamp::think_assign_better_overseer --------//


//--------- Begin of function FirmCamp::think_assign_better_overseer2 -------//
//
int FirmCamp::think_assign_better_overseer2(int targetTownRecno, int raceId)
{
	int reduceResistance;

	Nation* ownNation = nation_array[nation_recno];

	int bestUnitRecno = ownNation->find_best_capturer(targetTownRecno, raceId, reduceResistance);

	if( !bestUnitRecno || bestUnitRecno==overseer_recno )		// if we already got the best one here
		return 0;

	//---- only assign new overseer if the new one's leadership is significantly higher than the current one ----//

	if( overseer_recno && 
		 unit_array[bestUnitRecno]->skill.skill_level < unit_array[overseer_recno]->skill.skill_level + 15 )
	{
		return 0;
	}

	//------ check what the best unit is -------//

	if( !ownNation->mobilize_capturer(bestUnitRecno) )
		return 0;

	//---------- add the action to the queue now ----------//

	int actionRecno = ownNation->add_action( loc_x1, loc_y1, -1, -1,
							ACTION_AI_ASSIGN_OVERSEER, FIRM_CAMP, 1, bestUnitRecno );

	if( actionRecno )
		ownNation->process_action(actionRecno);

	return 1;
}
//--------- End of function FirmCamp::think_assign_better_overseer2 --------//


//--------- Begin of function FirmCamp::ai_capture_enemy_town -------//
//
// When capturing an enemy town, the commander should stay in the
// command base to influence the village and reinforcement should be
// sent instead of using the troop in the base for attacking the enemies.
//
int FirmCamp::ai_capture_enemy_town(Town* targetTown, int defenseCombatLevel)
{
	int useAllCamp = 0;

	Nation* ownNation = nation_array[nation_recno];
	Nation* targetNation = nation_array[targetTown->nation_recno];

	int ourMilitary   = ownNation->military_rank_rating();
	int enemyMilitary = targetNation->military_rank_rating();

	//--- use all camps to attack if we have money and we are stronger than the enemy ---//

	if( ourMilitary - enemyMilitary > 30 && ownNation->ai_should_spend(ownNation->pref_military_courage/2) )
		useAllCamp = 1;

	//---- use all camps to attack the enemy if the enemy is a human player

	else if( config.ai_aggressiveness >= OPTION_MODERATE &&
				!targetNation->is_ai() && ourMilitary > enemyMilitary )
	{
		if( config.ai_aggressiveness >= OPTION_HIGH ||
			 ownNation->pref_peacefulness < 50 )
		{
			useAllCamp = 1;
		}
	}

	return nation_array[nation_recno]->ai_attack_target(targetTown->loc_x1, targetTown->loc_y1,
			 defenseCombatLevel, 0, 0, 0, firm_recno, useAllCamp );
}
//--------- End of function FirmCamp::ai_capture_enemy_town --------//


//--------- Begin of function FirmCamp::think_capture_use_spy ---------//
//
// Think about using spies for capturing the target town.
//
int FirmCamp::think_capture_use_spy(Town* targetTown)
{
	Nation* ownNation = nation_array[nation_recno];

	//------ get the two most populated races of the town ----//

	int mostRaceId1, mostRaceId2;

	targetTown->get_most_populated_race(mostRaceId1, mostRaceId2);

	//-- get the current number of our spies in this town and see if we need more --//

	int spyCount;
	int curSpyLevel = spy_array.total_spy_skill_level( SPY_TOWN, targetTown->town_recno, nation_recno, spyCount );

	//--------------------------------------------//

	int rc=0;

	if( think_capture_use_spy2(targetTown, mostRaceId1, curSpyLevel ) )
		rc = 1;

	if( mostRaceId2 )
	{
		if( think_capture_use_spy2(targetTown, mostRaceId2, curSpyLevel ) )
			rc = 1;
	}

	return rc;
}
//----------- End of function FirmCamp::think_capture_use_spy -----------//


//--------- Begin of function FirmCamp::think_capture_use_spy2 ---------//
//
// Think about using spies for capturing the target town.
//
int FirmCamp::think_capture_use_spy2(Town* targetTown, int raceId, int curSpyLevel)
{
	Nation* ownNation = nation_array[nation_recno];

	int curResistance, targetResistance;

	if( targetTown->nation_recno )
	{
		curResistance 	  = (int) targetTown->race_loyalty_array[raceId-1];
		targetResistance = targetTown->race_target_loyalty_array[raceId-1];
	}
	else
	{
		curResistance    = (int) targetTown->race_resistance_array[raceId-1][nation_recno-1];
		targetResistance = targetTown->race_target_resistance_array[raceId-1][nation_recno-1];
	}

	int minResistance = MIN(curResistance, targetResistance);

	//----- if the resistance is low enough, don't have to use spies -----//

	if( targetTown->nation_recno )
	{
		if( minResistance < MIN_NATION_DEFEND_LOYALTY )
			return 0;
	}
	else
	{
		if( minResistance < MIN_INDEPENDENT_DEFEND_LOYALTY )
			return 0;
	}

	//----- if the needed spy level > current spy level, assign more spies ----//

	int neededSpyLevel = minResistance * (50+ownNation->pref_spy) / 50;

	if( neededSpyLevel > curSpyLevel )
		return ownNation->ai_assign_spy_to_town(targetTown->town_recno, raceId);

	return 0;
}
//----------- End of function FirmCamp::think_capture_use_spy2 -----------//


//--------- Begin of function FirmCamp::ai_update_link_status ---------//
//
// Updating link status of this firm with towns.
//
// It's a overloaded function of Firm::ai_update_link_status().
//
void FirmCamp::ai_update_link_status()
{
	Nation* ownNation = nation_array[nation_recno];

	//------ always enable links to all towns -----//

	for( int i=0 ; i<linked_town_count ; i++ )
	{
		Town* townPtr = town_array[linked_town_array[i]];

		//---- don't try to capture other nation's towns unless the AI is at war or tense with the nation ----//

		if( townPtr->nation_recno &&
			 ownNation->get_relation_status(townPtr->nation_recno) <= NATION_TENSE )		// hostile or tense 
		{	
			continue;
		}

		toggle_town_link( i+1, 1, COMMAND_AI );

		//------------------------------------------------------------------//
		// Here we only update this camp's link to the town. 
		// The town's link to this firm is updated in Town::update_target_loyalty().
		//------------------------------------------------------------------//
	}
}
//----------- End of function FirmCamp::ai_update_link_status ----------//


//------- Begin of function FirmCamp::ai_has_excess_worker -------//
//
// Return whether this firm has any excessive soldiers or not.
//
int FirmCamp::ai_has_excess_worker()
{
	if( linked_town_count==0 )
		return 1;

	if( ai_capture_town_recno )		// no if the camp is trying to capture an independent town
		return 0;

	if( is_attack_camp )		// no if the camp is trying to capture an independent town
		return 0;

	if( should_close_flag )
		return 1;

	return 0;

//	return total_combat_level() > ai_combat_level_needed()+100;
}
//--------- End of function FirmCamp::ai_has_excess_worker -------//


//------- Begin of function FirmCamp::think_use_cash_to_capture -------//
//
// Think about using money to decrease the resistance of the
// independent villagers.
//
int FirmCamp::think_use_cash_to_capture()
{
	if( !nation_array[nation_recno]->should_use_cash_to_capture() )
		return 0;

	//-------------------------------------//

	Town* townPtr;

	for( int i=0 ; i<linked_town_count ; i++ )
	{
		townPtr = town_array[ linked_town_array[i] ];

		if( townPtr->nation_recno == nation_recno )
			continue;

		if( townPtr->accumulated_enemy_grant_penalty > 0 )
			continue;

		if( townPtr->can_grant_to_non_own_town(nation_recno) )
			townPtr->grant_to_non_own_town(nation_recno, COMMAND_AI);
	}

	return 1;
}
//--------- End of function FirmCamp::think_use_cash_to_capture -------//


//------- Begin of function FirmCamp::think_linked_town_change_nation ------//
//
// This function is called by Town::set_nation() when a town linked
// to this firm has changed nation.
//
// <int> linkedTownRecno - the recno of the town that has changed nation.
// <int> oldNationRecno  - the old nation recno of the town
// <int> newNationRecno  - the new nation recno of the town
//
void FirmCamp::think_linked_town_change_nation(int linkedTownRecno, int oldNationRecno, int newNationRecno)
{
	//-----------------------------------------------//
	//
	// If we are trying to capture an independent town and our
	// enemies have managed to capture it first.
	//
	//-----------------------------------------------//

	Nation* ownNation = nation_array[nation_recno];

	if( oldNationRecno==0 && newNationRecno>0 && newNationRecno != nation_recno  )
	{
		Town* townPtr = town_array[linkedTownRecno];

		//--- if the town does not have any protection, then don't remove this camp ---//

		if( townPtr->protection_available()==0 )
			return;

		should_close_flag = 1;
		ownNation->firm_should_close_array[firm_id-1]++;
	}
}
//-------- End of function FirmCamp::think_linked_town_change_nation ------//


//------- Begin of function FirmCamp::think_attack_nearby_enemy -------//
//
// Think about attacking enemies near this cmap.
//
int FirmCamp::think_attack_nearby_enemy()
{
	//------------------------------------------//

	Nation* 	nationPtr = nation_array[nation_recno];

	int scanRange = 6 + nationPtr->pref_military_courage/20;		// 6 to 11

	int xLoc1 = loc_x1 - scanRange;
	int yLoc1 = loc_y1 - scanRange;
	int xLoc2 = loc_x2 + scanRange;
	int yLoc2 = loc_y2 + scanRange;

	xLoc1 = MAX( xLoc1, 0 );
	yLoc1 = MAX( yLoc1, 0 );
	xLoc2 = MIN( xLoc2, MAX_WORLD_X_LOC-1 );
	yLoc2 = MIN( yLoc2, MAX_WORLD_Y_LOC-1 );

	//------------------------------------------//

	int		enemyCombatLevel=0;		// the higher the rating, the easier we can attack the target town.
	int 		xLoc, yLoc;
	int		enemyXLoc= -1, enemyYLoc;
	Unit* 	unitPtr;
	Location* locPtr;

	for( yLoc=yLoc1 ; yLoc<=yLoc2 ; yLoc++ )
	{
		locPtr = world.get_loc(xLoc1, yLoc);

		for( xLoc=xLoc1 ; xLoc<=xLoc2 ; xLoc++, locPtr++ )
		{
			//--- if there is an enemy unit here ---//

			if( locPtr->has_unit(UNIT_LAND) )
			{
				unitPtr = unit_array[ locPtr->unit_recno(UNIT_LAND) ];

				if( !unitPtr->nation_recno )
					continue;

				//--- if the unit is idle and he is our enemy ---//

				if( unitPtr->cur_action == SPRITE_ATTACK &&
					 nationPtr->get_relation_status(unitPtr->nation_recno) == NATION_HOSTILE )
				{
					err_when( unitPtr->nation_recno == nation_recno );

					enemyCombatLevel += (int) unitPtr->hit_points;

					if( enemyXLoc == -1 || misc.random(5)==0 )
					{
						enemyXLoc = xLoc;
						enemyYLoc = yLoc;
					}
				}
			}
		}
	}

	if( enemyCombatLevel==0 )
		return 0;

	err_when( enemyXLoc < 0 );

	//--------- attack the target now -----------//

	if( worker_count > 0 )
	{
		patrol_all_soldier();

		if( patrol_unit_count > 0 )
		{
			// ###### patch begin Gilbert 5/8 ########//
			unit_array.attack(enemyXLoc, enemyYLoc, 0, patrol_unit_array, patrol_unit_count, COMMAND_AI,
				world.get_loc(enemyXLoc, enemyYLoc)->unit_recno(UNIT_LAND));
			// ###### patch end Gilbert 5/8 ########//
			return 1;
		}
	}

	return 0;
}
//-------- End of function FirmCamp::think_attack_nearby_enemy -------//


//------- Begin of function FirmCamp::think_change_town_link -------//
//
// Think about changing links to foreign towns.
//
void FirmCamp::think_change_town_link()
{
	Nation* ownNation = nation_array[nation_recno];

	for( int i=linked_town_count-1 ; i>=0 ; i-- )
	{
		Town* townPtr = town_array[ linked_town_array[i] ];

		//--- only change links to foreign towns, links to own towns are always on ---//

		if( townPtr->nation_recno == nation_recno )
			continue;

		//---- only enable links to non-friendly towns ----//

		int enableFlag = townPtr->nation_recno==0 || 
							  ownNation->get_relation(townPtr->nation_recno)->status == NATION_HOSTILE;

		toggle_town_link( i+1, enableFlag, COMMAND_AI );
	}
}
//--------- End of function FirmCamp::think_change_town_link -------//


//------- Begin of function FirmCamp::think_assign_better_commander -------//
//
// Assign a better commander to this camp.
//
int FirmCamp::think_assign_better_commander()
{
	//----- if there is already an overseer being assigned to the camp ---//

	Nation* ownNation = nation_array[nation_recno];

	int actionRecno = ownNation->is_action_exist(loc_x1, loc_y1, -1, -1,
							 ACTION_AI_ASSIGN_OVERSEER, FIRM_CAMP);

	//--- if there is already one existing ---//

	if( actionRecno )
	{
		//--- if the action still is already being processed, don't bother with it ---//

		if( ownNation->get_action(actionRecno)->processing_instance_count > 0 )
			return 0;

		//--- however, if the action hasn't been processed, we still try to use the approach here ---//
	}

	//-------------------------------------------------//

	Firm*   firmPtr;
	Worker* workerPtr;
	int     bestRaceId = best_commander_race();
	int	  bestFirmRecno=0, bestWorkerId;
	int  	  bestLeadership=0;

	if( overseer_recno )
	{
		bestLeadership	= cur_commander_leadership(bestRaceId)
							  + 10 + ownNation->pref_loyalty_concern/10;			// nations that have higher loyalty concern will not switch commander too frequently
	}

	//--- locate for a soldier who has a higher leadership ---//

	for( int i=ownNation->ai_camp_count-1 ; i>=0 ; i-- )
	{
		firmPtr = firm_array[ ownNation->ai_camp_array[i] ];

		if( firmPtr->region_id != region_id )
			continue;

		workerPtr = firmPtr->worker_array;

		for( int j=1 ; j<=firmPtr->worker_count ; j++, workerPtr++ )
		{
			if( !workerPtr->race_id )
				continue;

			int workerLeadership = workerPtr->skill_level;

			if( workerPtr->race_id != bestRaceId )
				workerLeadership /= 2;

			if( workerLeadership > bestLeadership )
			{
				bestLeadership = workerLeadership;
				bestFirmRecno  = firmPtr->firm_recno;
				bestWorkerId   = j;
			}
		}
	}

	if( bestFirmRecno == 0 )
		return 0;

	//-------- assign the overseer now -------//

	int unitRecno = firm_array[bestFirmRecno]->mobilize_worker(bestWorkerId, COMMAND_AI);

	if( !unitRecno )
		return 0;

	Unit* unitPtr = unit_array[unitRecno];

	unitPtr->set_rank(RANK_GENERAL);

	//---- if there is already an existing but unprocessed one, delete it first ---//

	if( actionRecno )
		ownNation->del_action(actionRecno);

	return ownNation->add_action(loc_x1, loc_y1, -1, -1, ACTION_AI_ASSIGN_OVERSEER, FIRM_CAMP, 1, unitRecno);
}
//-------- End of function FirmCamp::think_assign_better_commander ---------//


//------- Begin of function FirmCamp::cur_commander_leadership -------//
//
// [int] bestRaceId - if this is given, it will be used, otherwise
//							 it will use best_commander_race().
//
// Return the commander leadership
//
int FirmCamp::cur_commander_leadership(int bestRaceId)
{
	int commanderLeadership=0;

	//--- get the current leadership of the commander ----//

	if( overseer_recno )
	{
		if( !bestRaceId )
			bestRaceId = best_commander_race();

		Unit* unitCommander = unit_array[overseer_recno];

		if( unitCommander->race_id == bestRaceId )
			commanderLeadership = unitCommander->skill.skill_level;
		else
			commanderLeadership = unitCommander->skill.skill_level / 2;		// divided by 2 if the race doesn't match
	}

	return commanderLeadership;
}
//-------- End of function FirmCamp::cur_commander_leadership ---------//


//------- Begin of function FirmCamp::new_commander_leadership -------//
//
// <int> newRaceId 	  - the race id. of the would-be commander.
// <int> newSkillLevel - the skill level of the would-be commander.
//
// Return the commander leadership if the unit is assigned to this camp.
//
int FirmCamp::new_commander_leadership(int newRaceId, int newSkillLevel)
{
	int commanderLeadership=0;
	int bestRaceId = best_commander_race();

	//--- get the current leadership of the commander ----//

	if( overseer_recno )
	{
		if( newRaceId == bestRaceId )
			commanderLeadership = newSkillLevel;
		else
			commanderLeadership = newSkillLevel / 2;		// divided by 2 if the race doesn't match
	}

	return commanderLeadership;
}
//-------- End of function FirmCamp::new_commander_leadership ---------//


//------- Begin of function FirmCamp::best_commander_race -------//
//
// Return what race the commander should be for this camp.
//
int FirmCamp::best_commander_race()
{
	//---------------------------------------------//
	//
	// If this camp is the commanding camp of a town,
	// then return the majority race of the town.
	//
	// A camp is the commanding camp of a town when
	// it is the closest camp to the town.
	//
	//---------------------------------------------//

	for( int i=linked_town_count-1 ; i>=0 ; i-- )
	{
		Town* townPtr = town_array[ linked_town_array[i] ];

		if( townPtr->closest_own_camp() == firm_recno )
			return townPtr->majority_race();
	}

	//----- check if this camp is trying to capture an independent town ---//

	int targetTownRecno = think_capture_target_town();

	if( targetTownRecno && town_array[targetTownRecno]->nation_recno==0 )
		return town_array[targetTownRecno]->majority_race();

	//----- Otherwise return the majority race of this camp -----//

	return majority_race();
}
//-------- End of function FirmCamp::best_commander_race ---------//

