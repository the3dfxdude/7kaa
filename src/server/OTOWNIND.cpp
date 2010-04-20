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

//Filename    : OTOWNINDP.CPP
//Description : Independent town thining

#include <OWORLD.h>
#include <OUNIT.h>
#include <ONEWS.h>
#include <OCONFIG.h>
#include <ORACERES.h>
#include <OGODRES.h>
#include <OTECHRES.h>
#include <OREGIONS.h>
#include <OFIRM.h>
#include <ONATION.h>
#include <OINFO.h>
#include <OTOWN.h>
#include <OLOG.h>

//------- Begin of function Town::think_independent_town --------//
//
// Independent town thinking.
//
void Town::think_independent_town()
{
	if( rebel_recno )		// if this is a rebel town, its AI will be executed in Rebel::think_town_action()
		return;

#if defined(DEBUG) && defined(ENABLE_LOG)
	String logStr;

	logStr = "begin Town::think_independent_town, town_recno=";
	logStr += town_recno;
	LOG_MSG(logStr);
#endif

	//---- think about toggling town links ----//

	if( info.game_date%15 == town_recno%15 )
	{
		LOG_MSG(" Town::think_independent_set_link");
		think_independent_set_link();
		LOG_MSG(m.get_random_seed() );
	}

	//---- think about independent units join existing nations ----//

	if( info.game_date%60 == town_recno%60 )
	{
		LOG_MSG(" Town::think_independent_unit_join_nation");
		think_independent_unit_join_nation();
		LOG_MSG(m.get_random_seed() );
	}

	//----- think about form a new nation -----//

	if( info.game_date%365 == town_recno%365 )
	{
		LOG_MSG(" Town::think_independent_form_new_nation");
		think_independent_form_new_nation();
		LOG_MSG(m.get_random_seed() );
	}

	LOG_MSG("end Town::think_independent_town");
	LOG_MSG(m.get_random_seed());
}
//-------- End of function Town::think_independent_town ---------//


//------- Begin of function Town::think_independent_set_link --------//
//
// Independent town thinking.
//
void Town::think_independent_set_link()
{
	//---- think about working for foreign firms ------//

	int	linkStatus;
	Firm* firmPtr;

	for(int i=0; i<linked_firm_count; i++)
	{
		firmPtr = firm_array[linked_firm_array[i]];

		if( firmPtr->firm_id == FIRM_CAMP )		// a town cannot change its status with a military camp
			continue;

		//---- think about the link status ----//

		linkStatus = 0;

		if( firmPtr->nation_recno == 0 )		// if the firm is also an independent firm
			linkStatus = 1;

		if( average_resistance(firmPtr->nation_recno) <= INDEPENDENT_LINK_RESISTANCE )
			linkStatus = 1;

		//---- set the link status -------//

		toggle_firm_link( i+1, linkStatus, COMMAND_AI );
	}

	ai_link_checked = 1;
}
//-------- End of function Town::think_independent_set_link ---------//


//------ Begin of function Town::think_independent_form_new_nation ------//
//
// Independent town thinking.
//
int Town::think_independent_form_new_nation()
{
	if( m.random(10) > 0 )		// 1/10 chance to set up a new nation. 
		return 0;

	//-------- check if the town is big enough -------//

	if( population < 30 )
		return 0;

	//---- don't form if the world is already densely populated ----//

	if( nation_array.all_nation_population > 60 * MAX_NATION )
		return 0;

	//----------------------------------------------//

	if( !nation_array.can_form_new_ai_nation() )
		return 0;

	//----------------------------------------------//

	return form_new_nation();
}
//------ End of function Town::think_independent_form_new_nation ------//


//--------- Begin of function Town::form_new_nation ---------//
//
// This independent town forms a new nation.
//
// Return: <int> 
//
int Town::form_new_nation()
{
	err_when( nation_recno );

	if( !nation_array.can_form_new_ai_nation() )
		return 0;

	//----- determine the race with most population -----//

	int maxPop=0, raceId=0;

	int i;
	for( i=0 ; i<MAX_RACE ; i++ )
	{
		if( race_pop_array[i] > maxPop )
		{
			maxPop = race_pop_array[i];
			raceId = i+1;
		}
	}

	err_when( !raceId );

	//---- create the king of the new nation ----//

	int unitId = race_res[raceId]->basic_unit_id;
	int xLoc=loc_x1, yLoc=loc_y1;     // xLoc & yLoc are used for returning results
	SpriteInfo* spriteInfo = sprite_res[unit_res[unitId]->sprite_id];

	if( !world.locate_space( xLoc, yLoc, loc_x2, loc_y2, spriteInfo->loc_width, spriteInfo->loc_height ) )
		return 0;

	//--------- create a new nation ---------//

	int nationRecno = nation_array.new_nation( NATION_AI, raceId, nation_array.random_unused_color() );

	err_when( !nationRecno );

	//-------- create the king --------//

	int kingUnitRecno = unit_array.add_unit( unitId, nationRecno, RANK_KING, 100, xLoc, yLoc );

	Unit* kingUnit = unit_array[kingUnitRecno];

	kingUnit->skill.skill_id = SKILL_LEADING;
	kingUnit->skill.skill_level = 50+m.random(51);

	kingUnit->set_combat_level(70+m.random(31));

	nation_array[nationRecno]->set_king(kingUnitRecno, 1);		// 1-this is the first king of the nation

	dec_pop(raceId, 0);		// 0-the unit doesn't have a job

	//------ set the nation of the rebel town -----//

   err_when( rebel_recno );		// rebel has its own function in Rebel, this shouldn't be called

	set_nation(nationRecno);		// set the town at last because set_nation() will delete the Town object

	//------ increase the loyalty of the town -----//

	for( i=0 ; i<MAX_RACE ; i++ )
		race_loyalty_array[i] = (float) 70 + m.random(20);			// 70 to 90 initial loyalty 

	//--------- add news ----------//

	news_array.new_nation(nationRecno);

	//--- random extra beginning advantages -----//

	int mobileCount;
	Nation* nationPtr = nation_array[nationRecno];

	switch( m.random(10) )
	{
		case 1:		// knowledge of weapon in the beginning.
			tech_res[ m.random(tech_res.tech_count)+1 ]->set_nation_tech_level(nationRecno, 1);
			break;

		case 2:		// random additional cash
			nationPtr->cash += m.random(5000);
			break;

		case 3:		// random additional food
			nationPtr->food += m.random(5000);
			break;

		case 4:		// random additional skilled units
			mobileCount = m.random(5)+1;

			for( i=0 ; i<mobileCount && recruitable_race_pop(raceId,0)>0 ; i++ )		// 0-don't recruit spies
			{
				int unitRecno = mobilize_town_people(raceId, 1, 0);		// 1-dec pop, 0-don't mobilize spies

				if( unitRecno )
				{
					Unit* unitPtr = unit_array[unitRecno];

					//------- randomly set a skill -------//

					int skillId = m.random(MAX_TRAINABLE_SKILL)+1;
					int loopCount=0;		// no spying skill

					while( skillId==SKILL_SPYING )		// no spy skill as skill_id can't be set as SKILL_SPY, for spies, spy_recno must be set instead
					{
						if( ++skillId > MAX_TRAINABLE_SKILL )
							skillId = 1;

						err_when( ++loopCount > 100 );
					}

					unitPtr->skill.skill_id    = skillId;
					unitPtr->skill.skill_level = 50 + m.random(50);
					unitPtr->set_combat_level( 50 + m.random(50) );
				}
				else
					break;
			}
			break;
	}

	return nationRecno;
}
//----------- End of function Town::form_new_nation ---------//


//---- Begin of function Town::think_independent_unit_join_nation ----//
//
// Think having independent units joining existing nations.
//
int Town::think_independent_unit_join_nation()
{
	if( jobless_population==0 )
		return 0;

	independent_unit_join_nation_min_rating -= 2;		// make it easier to join nation everytime it's called
																		// -2 each time, adding of 30 after a unit has been recruited and calling it once every 2 months make it a normal rate of joining once a year per town

	//------ think about which nation to turn towards -----//

	int    i, bestNationRecno=0, curRating, raceId, bestRaceId=0;
	int	 bestRating=independent_unit_join_nation_min_rating;
	Nation *nationPtr;

	// ###### patch begin Gilbert 16/3 ########//
// #ifdef AMPLUS
	if( region_array[region_id]->region_stat_id == 0)
		return 0;
// #endif
	// ###### patch end Gilbert 16/3 ########//
	RegionStat* regionStat = region_array.get_region_stat(region_id);

	for( i=nation_array.size() ; i>0 ; i-- )
	{
		if( nation_array.is_deleted(i) )
			continue;

		nationPtr = nation_array[i];

		if( !nationPtr->race_id )
			continue;

		if( nationPtr->cash <= 0 )
			continue;

		if( info.game_date < nationPtr->last_independent_unit_join_date + 90 )		// don't join too frequently, at most 3 months a unit
			continue;

		//--- only join the nation if the nation has town in the town's region ---//

		if( regionStat->town_nation_count_array[i-1] == 0 )
			continue;

		//----- calculate the rating of the nation -----//

		curRating = (int) nationPtr->reputation + nationPtr->overall_rating;

		if( recruitable_race_pop(nationPtr->race_id, 0) > 0 ) 	// 0-don't count spies
		{
			curRating += 30;
			raceId     = nationPtr->race_id;
		}
		else
			raceId = 0;

		if( curRating > bestRating )
		{
			bestRating 	 	 = curRating;
			bestNationRecno = i;
			bestRaceId  	 = raceId;
		}
	}

	//--------------------------------------------//

	if( !bestNationRecno )
		return 0;

	if( !bestRaceId )
		bestRaceId = pick_random_race(0, 0);		// 0-only pick jobless unit, 0-don't pick spy units

	if( !bestRaceId )
   	return 0;

	if( !independent_unit_join_nation(bestRaceId, bestNationRecno) )
		return 0;

	//--- set a new value to independent_unit_join_nation_min_rating ---//

	independent_unit_join_nation_min_rating = bestRating + 100 + m.random(30);		// reset it to a higher rating

	if( independent_unit_join_nation_min_rating < 100 )
		independent_unit_join_nation_min_rating = 100;

	return 1;
}
//----- End of function Town::think_independent_unit_join_nation -----//


//---- Begin of function Town::independent_unit_join_nation ----//
//
// <int> raceId 		  - race id. of the unit
// <int> toNationRecno - recno of the nation the unit should turn toward
//
int Town::independent_unit_join_nation(int raceId, int toNationRecno)
{
	//----- mobilize a villager ----//

	int unitRecno = mobilize_town_people(raceId, 1, 0);		// 1-dec population after mobilizing the unit, 0-don't mobilize spies

	if( !unitRecno )
		return 0;

	Unit* unitPtr = unit_array[unitRecno];

	//----- set the skills of the unit -----//

	int skillId, skillLevel, combatLevel;

	switch( m.random(3) )
	{
		case 0:		// leaders
			skillId = SKILL_LEADING;

			if( m.random(3)==0 )
				skillLevel = m.random(100);
			else
				skillLevel = m.random(50);

			combatLevel = skillLevel + m.random(40) - 20;
         combatLevel = MIN(combatLevel, 100);
			combatLevel = MAX(combatLevel, 10);
			break;

		case 1:		// peasants
			skillId = 0;
			skillLevel = 0;
			combatLevel = 10 + m.random(10);
			break;

		case 2:		// skilled units
			skillId = m.random(MAX_TRAINABLE_SKILL)+1;
			{
				int loopCount=0;		// no spying skill

				while( skillId==SKILL_SPYING )
				{
					if( ++skillId > MAX_TRAINABLE_SKILL )
						skillId = 1;

					err_when( ++loopCount > 100 );
				}
			}
			skillLevel = 10+m.random(80);
			combatLevel = 10+m.random(30);
			break;
	}

	//--------------------------------------//

	unitPtr->skill.skill_id = skillId;
	unitPtr->skill.skill_level = skillLevel;
	unitPtr->set_combat_level( combatLevel );

	//------ change nation now --------//

	if( !unitPtr->betray(toNationRecno) )
		return 0;

	//---- the unit moves close to the newly joined nation ----//

	unitPtr->ai_move_to_nearby_town();

	//-------- set last_independent_unit_join_date --------//

	nation_array[toNationRecno]->last_independent_unit_join_date = info.game_date;

	return 1;
}
//----- End of function Town::independent_unit_join_nation -----//


