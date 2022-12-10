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

//Filename    : OTOWNAI.CPP
//Description : Object Town AI

#include <OWORLD.h>
#include <OUNIT.h>
#include <OCONFIG.h>
#include <OSITE.h>
#include <ONEWS.h>
#include <OTECHRES.h>
#include <OWALLRES.h>
#include <ORACERES.h>
#include <OGODRES.h>
#include <OSPY.h>
#include <OTOWN.h>
#include <ONATION.h>
#include <OINFO.h>
#include <OFIRMALL.h>
#include <OLOG.h>


//--------- Begin of function Town::process_ai ----------//
//
void Town::process_ai()
{
	Nation* ownNation = nation_array[nation_recno];

#if defined(DEBUG) && defined(ENABLE_LOG)
	String logStr;

	logStr = "begin Town::process_ai, town_recno=";
	logStr += town_recno;
	logStr += " nation_recno=";
	logStr += nation_recno;
	LOG_MSG(logStr);
#endif

	//---- think about cancelling the base town status ----//

	if( info.game_date%30==town_recno%30 )
	{
		LOG_MSG(" update_base_town_status");
		update_base_town_status();
		LOG_MSG(misc.get_random_seed());
	}

	//------ think about granting villagers ---//

	if( info.game_date%30==town_recno%30 )
	{
		LOG_MSG(" think_reward");
		think_reward();
		LOG_MSG(misc.get_random_seed());
	}

	//----- if this town should migrate now -----//

	if( should_ai_migrate() )
	{
		if( info.game_date%30==town_recno%30 )
		{
			LOG_MSG(" think_ai_migrate");
			think_ai_migrate();
			LOG_MSG(misc.get_random_seed());
		}

		return;		// don't do anything else if the town is about to migrate
	}

	//------ think about building camps first -------//

	if( info.game_date%30==town_recno%30 && !no_neighbor_space &&		// if there is no space in the neighbor area for building a new firm.
		 population > 1 )
	{
		LOG_MSG(" think_build_camp");
		if( think_build_camp() )
		{
			LOG_MSG(misc.get_random_seed());
			return;
		}
		LOG_MSG(misc.get_random_seed());
	}

	//----- the following are base town only functions ----//

	if( !is_base_town )
		return;

	//------ think about collecting tax ---//

	if( info.game_date%30==(town_recno+15)%30 )
	{
		LOG_MSG("think_collect_tax");
		think_collect_tax();
		LOG_MSG(misc.get_random_seed());
	}

	//---- think about scouting in an unexplored map ----//

	if( info.game_date%30==(town_recno+20)%30  )
	{
		think_scout();
	}

	//---- think about splitting the town ---//

	if( info.game_date%30==town_recno%30 )
	{
		LOG_MSG("think_move_between_town");
		think_move_between_town();
		LOG_MSG(misc.get_random_seed() );
		LOG_MSG("think_split_town");
		think_split_town();
		LOG_MSG(misc.get_random_seed() );
	}

	//---- think about attacking firms/units nearby ---//

	if( info.game_date%30==town_recno%30 )
	{
		LOG_MSG(" think_attack_nearby_enemy");
		if( think_attack_nearby_enemy() )
		{
			LOG_MSG(misc.get_random_seed());
			return;
		}
		LOG_MSG(misc.get_random_seed());

		LOG_MSG(" think_attack_linked_enemy");
		if( think_attack_linked_enemy() )
		{
			LOG_MSG(misc.get_random_seed());
			return;
		}
		LOG_MSG(misc.get_random_seed());
	}

	//---- think about capturing linked enemy firms ----//

	if( info.game_date%60==town_recno%60 )
	{
		LOG_MSG(" think_capture_linked_firm");
		think_capture_linked_firm();
		LOG_MSG(misc.get_random_seed());
	}

	//---- think about capturing enemy towns ----//

	if( info.game_date%120==town_recno%120 )
	{
		LOG_MSG(" think_capture_enemy_town");
		think_capture_enemy_town();
		LOG_MSG(misc.get_random_seed());
	}

	//---- think about using spies on enemies ----//

	if( info.game_date%60==(town_recno+10)%60 )
	{
		LOG_MSG(" think_spying_town");
		if( think_spying_town() )
		{
			LOG_MSG(misc.get_random_seed());
			return;
		}
	}

	//---- think about anti-spies activities ----//

	if( info.game_date%60==(town_recno+20)%60 )
	{
		LOG_MSG(" think_counter_spy");
		if( think_counter_spy() )
		{
			LOG_MSG(misc.get_random_seed());
			return;
		}
	}

	//--- think about setting up firms next to this town ---//

	if( info.game_date%30==town_recno%30 && !no_neighbor_space &&		// if there is no space in the neighbor area for building a new firm.
		 population >= 5 )
	{
		LOG_MSG(" think_build_market");
		if( think_build_market() )
		{
			LOG_MSG(misc.get_random_seed());
			return;
		}
		LOG_MSG(misc.get_random_seed());

		//--- the following functions will only be called when the nation has at least a mine ---//

		if( site_array.untapped_raw_count > 0 &&
			 ownNation->ai_mine_count==0 &&		// don't build other structures if there are untapped raw sites and our nation still doesn't have any
			 ownNation->true_profit_365days() < 0 )
		{
			return;
		}

		//---- only build the following if we have enough food ----//

      if( ownNation->ai_has_enough_food() )
		{
			LOG_MSG(" think_build_research");
			if( think_build_research() )
			{
				LOG_MSG(misc.get_random_seed());
				return;
			}
			LOG_MSG(misc.get_random_seed());

			LOG_MSG(" think_build_war_factory");
			if( think_build_war_factory() )
			{
				LOG_MSG(misc.get_random_seed());
				return;
			}
			LOG_MSG(misc.get_random_seed());

			LOG_MSG(" think_build_base");
			if( think_build_base() )
			{
				LOG_MSG(misc.get_random_seed());
				return;
			}
			LOG_MSG(misc.get_random_seed());
		}

		//-------- think build inn ---------//

		LOG_MSG(" think_build_inn");
		think_build_inn();
		LOG_MSG(misc.get_random_seed());
	}
}
//--------- End of function Town::process_ai ----------//


//--------- Begin of function Town::think_defense ----------//
//
void Town::think_defense()
{
	int enemyUnitRecno = detect_enemy(3);		// only when 3 units are detected, we consider them as enemy

	if( !enemyUnitRecno )
		return;

	Unit* enemyUnit = unit_array[enemyUnitRecno];

	int enemyXLoc = enemyUnit->cur_x_loc();
	int enemyYLoc = enemyUnit->cur_y_loc();

	//----- get our unit that is closet to it to attack it -------//

	int   i, closestUnitRecno=0, curDis, minDis=0x7FFF;
	Unit* unitPtr;

	for( i=unit_array.size() ; i>0 ; i-- )
	{
		if( unit_array.is_deleted(i) )
			continue;

		unitPtr = unit_array[i];

		if( unitPtr->nation_recno == nation_recno )
		{
			curDis = MAX( abs(unitPtr->cur_x_loc()-enemyXLoc), abs(unitPtr->cur_y_loc()-enemyYLoc) );

			if( curDis < minDis )
			{
				minDis = curDis;
				closestUnitRecno = i;
			}
		}
	}

	//-------- attack the enemy now ----------//

	if( closestUnitRecno )
		unit_array[closestUnitRecno]->attack_unit(enemyUnit->sprite_recno);
}
//--------- End of function Town::think_defense ----------//


//--------- Begin of function Town::detect_enemy ----------//
//
// Detect if there is any enemy in the town. (within the city
// wall area.)
//
// <int> alertNum - only alert when this number of units are detected.
//
int Town::detect_enemy(int alertNum)
{
	//------ check if any enemies have entered in the city -----//

	int xLoc1 = MAX(0, loc_x1-WALL_SPACE_LOC);
	int yLoc1 = MAX(0, loc_y1-WALL_SPACE_LOC);
	int xLoc2 = MIN(MAX_WORLD_X_LOC-1, loc_x2+WALL_SPACE_LOC);
	int yLoc2 = MIN(MAX_WORLD_Y_LOC-1, loc_y2+WALL_SPACE_LOC);
	int xLoc, yLoc, unitRecno;
	int enemyCount=0;
	Location* locPtr;
	Unit* unitPtr;

	for( yLoc=yLoc1 ; yLoc<=yLoc2 ; yLoc++ )
	{
		locPtr = world.get_loc(xLoc1, yLoc);

		for( xLoc=xLoc1 ; xLoc<=xLoc2 ; xLoc++, locPtr++ )
		{
			if( locPtr->has_unit(UNIT_LAND) )
			{
				unitRecno = locPtr->unit_recno(UNIT_LAND);

				//------- if any enemy detected -------//

				unitPtr = unit_array[unitRecno];

				if( unitPtr->nation_recno != nation_recno && unitPtr->nation_recno > 0 )
				{
					if( ++enemyCount >= alertNum )
						return unitRecno;
				}
			}
		}
	}

	return 0;
}
//--------- End of function Town::detect_enemy ----------//


//------- Begin of function Town::think_build_firm --------//
//
// Think about building a specific type of firm next to this town.
//
// <int> firmId  - id. of the firm to be built.
// <int> maxFirm - MAX. no. of firm of this type to be built next to this town.
//
int Town::think_build_firm(int firmId, int maxFirm)
{
	Nation* nationPtr = nation_array[nation_recno];

	//--- check whether the AI can build a new firm next this firm ---//

	if( !nationPtr->can_ai_build(firmId) )
		return 0;

	//-- only build one market place next to this town, check if there is any existing one --//

	Firm* firmPtr;
	int 	firmCount=0;

	for(int i=0; i<linked_firm_count; i++)
	{
		err_when(!linked_firm_array[i] || firm_array.is_deleted(linked_firm_array[i]));

		firmPtr = firm_array[linked_firm_array[i]];

		//---- if there is one firm of this type near the town already ----//

		if( firmPtr->firm_id == firmId &&
			 firmPtr->nation_recno == nation_recno )
		{
			if( ++firmCount >= maxFirm )
				return 0;
		}
	}

	//------ queue building a new firm -------//

	return ai_build_neighbor_firm(firmId);
}
//-------- End of function Town::think_build_firm ---------//


//------- Begin of function Town::think_collect_tax --------//
//
// Think about collecting tax.
//
void Town::think_collect_tax()
{
	if( !has_linked_own_camp )
		return;

	if( should_ai_migrate() )		// if the town should migrate, do collect tax, otherwise the loyalty will be too low for mobilizing the peasants.
		return;

	if( accumulated_collect_tax_penalty > 0 )
		return;

	//--- collect tax if the loyalty of all the races >= minLoyalty (55-85) ---//

	int yearProfit = (int) nation_array[nation_recno]->profit_365days();

	int minLoyalty = 55 + 30 * nation_array[nation_recno]->pref_loyalty_concern / 100;

	if( yearProfit < 0 )								// we are losing money now
		minLoyalty -= (-yearProfit) / 100;		// more aggressive in collecting tax if we are losing a lot of money

	minLoyalty = MAX( 55, minLoyalty );

	//---------------------------------------------//

	int achievableLoyalty = average_target_loyalty()-10;		// -10 because it's an average, -10 will be safer

	if( achievableLoyalty > minLoyalty )		// if the achievable loyalty is higher, then use it 
		minLoyalty = achievableLoyalty;

	if( average_loyalty() < minLoyalty )
		return;

	//---------- collect tax now ----------//

	collect_tax( COMMAND_AI );
}
//-------- End of function Town::think_collect_tax ---------//


//------- Begin of function Town::think_reward --------//
//
// Think about granting the villagers.
//
void Town::think_reward()
{
	if( !has_linked_own_camp )
		return;

	if( accumulated_reward_penalty > 0 )
		return;

	//---- if accumulated_reward_penalty>0, don't grant unless the villagers are near the rebel level ----//

	Nation* ownNation = nation_array[nation_recno];
	int averageLoyalty = average_loyalty();

	if( averageLoyalty < REBEL_LOYALTY + 5 + ownNation->pref_loyalty_concern/10 )		// 35 to 45
	{
		int importanceRating;

		if( averageLoyalty < REBEL_LOYALTY+5 )
			importanceRating = 40+population;
		else
			importanceRating = population;

		if( ownNation->ai_should_spend(importanceRating) )
			reward( COMMAND_AI );
	}
}
//-------- End of function Town::think_reward ---------//


//------- Begin of function Town::think_ai_migrate --------//
//
// Think about migrating to another town.
//
int Town::think_ai_migrate()
{
	if( info.game_date < setup_date+90 )		// don't move if this town has just been set up for less than 90 days. It may be a town set up by think_split_town()
		return 0;

	Nation* nationPtr = nation_array[nation_recno];

	//-- the higher the loyalty, the higher the chance all the unit can be migrated --//

	int averageLoyalty = average_loyalty();
	int minMigrateLoyalty = 35 + nationPtr->pref_loyalty_concern/10;		// 35 to 45

	if( averageLoyalty < minMigrateLoyalty )
	{
		//-- if the total population is low (we need people) and the cash is high (we have money), then grant to increase loyalty for migration --//

		if( accumulated_reward_penalty==0 &&
			 average_target_loyalty() < minMigrateLoyalty+5 )		// if the average target loyalty is also lower than
		{
			if( nationPtr->ai_should_spend( 20+nationPtr->pref_territorial_cohesiveness/2 ) )		// 20 to 70 
				reward( COMMAND_AI );
		}

		if( average_loyalty() < minMigrateLoyalty )
			return 0;
	}

	if( !should_ai_migrate() )
		return 0;

	//------ think about which town to migrate to ------//

	int bestTownRecno = think_ai_migrate_to_town();

	if( !bestTownRecno )
		return 0;

	//--- check if there are already units currently migrating to the destination town ---//

	Town* destTownPtr = town_array[bestTownRecno];

	if( nationPtr->is_action_exist( destTownPtr->loc_x1, destTownPtr->loc_y1,
		 loc_x1, loc_y1, ACTION_AI_SETTLE_TO_OTHER_TOWN, 0, 0, 1 ) )		// last 1-check duplication on the destination town only
	{
		return 0;
	}

	//--------- queue for migration now ---------//

	int migrateCount = (average_loyalty() - MIN_RECRUIT_LOYALTY) / 5;

	migrateCount = MIN( migrateCount, jobless_population );

	if( migrateCount <= 0 )
		return 0;

	nationPtr->add_action( destTownPtr->loc_x1, destTownPtr->loc_y1,
		 loc_x1, loc_y1, ACTION_AI_SETTLE_TO_OTHER_TOWN, 0, migrateCount);

	return 1;
}
//-------- End of function Town::think_ai_migrate ---------//


//------- Begin of function Town::think_ai_migrate_to_town --------//
//
// Think about the town to migrate to.
//
int Town::think_ai_migrate_to_town()
{
	//------ think about which town to migrate to ------//

	Nation* nationPtr = nation_array[nation_recno];
	int 	 curRating, bestRating=0, bestTownRecno=0;
	short  *aiTownPtr = nationPtr->ai_town_array;
	int	 majorityRace = majority_race();
	Town	 *townPtr;

	for(int i=0; i<nationPtr->ai_town_count; i++, aiTownPtr++)
	{
		if( town_recno == *aiTownPtr )
			continue;

		townPtr = town_array[*aiTownPtr];

		err_when( townPtr->nation_recno != nation_recno );

		if( !townPtr->is_base_town )		// only migrate to base towns
			continue;

		if( townPtr->region_id != region_id )
			continue;

		if( population > MAX_TOWN_POPULATION-townPtr->population )		// if the town does not have enough space for the migration
			continue;

		//--------- compare the ratings ---------//

		curRating = 1000 * townPtr->race_pop_array[majorityRace-1] / townPtr->population;	// *1000 so that this will have a much bigger weight than the distance rating

		curRating += world.distance_rating( center_x, center_y, townPtr->center_x, townPtr->center_y );

		if( curRating > bestRating )
		{
			//--- if there is a considerable population of this race, then must migrate to a town with the same race ---//

			if( race_pop_array[majorityRace-1] >= 6 )
			{
				if( townPtr->majority_race() != majorityRace )				// must be commented out otherwise low population town will never be optimized
					continue;
			}

			bestRating   = curRating;
			bestTownRecno = townPtr->town_recno;
		}
	}

	return bestTownRecno;
}
//-------- End of function Town::think_ai_migrate_to_town ---------//


//------- Begin of function Town::should_ai_migrate --------//
//
// Whether this town should think about migration or not.
//
int Town::should_ai_migrate()
{
	//--- if this town is the base town of the nation's territory, don't migrate ---//

	Nation* nationPtr = nation_array[nation_recno];

	if( is_base_town || nationPtr->ai_base_town_count==0 )		// don't migrate if this is a base town or there is no base town in this nation
		return 0;

	if( population-jobless_population>0 )		// if there are workers in this town, don't migrate the town people
		return 0;

	return 1;
}
//-------- End of function Town::should_ai_migrate ---------//


//-------- Begin of function Town::protection_needed ------//
//
// Return an index from 0 to 100 indicating the military
// protection needed for this town.
//
int Town::protection_needed()
{
	int   protectionNeeded = population * 10;
	Firm* firmPtr;

	for( int i=linked_firm_count-1 ; i>=0 ; i-- )
	{
		firmPtr = firm_array[ linked_firm_array[i] ];

		if( firmPtr->nation_recno != nation_recno )
			continue;

		//----- if this is a camp, add combat level points -----//

		if( firmPtr->firm_id == FIRM_MARKET )
		{
			protectionNeeded += ((FirmMarket*)firmPtr)->stock_value_index()*2;
		}
		else
		{
			protectionNeeded += (int) firmPtr->productivity*2;

			if( firmPtr->firm_id == FIRM_MINE )		// more protection for mines
				protectionNeeded += 200;
		}
	}

	return protectionNeeded;
}
//---------- End of function Town::protection_needed ------//


//-------- Begin of function Town::protection_available ------//
//
// Return an index from 0 to 100 indicating the military
// protection currently available for this town.
//
int Town::protection_available()
{
	int 	protectionLevel=0;
	Firm* firmPtr;

	for( int i=linked_firm_count-1 ; i>=0 ; i-- )
	{
		firmPtr = firm_array[ linked_firm_array[i] ];

		if( firmPtr->nation_recno != nation_recno )
			continue;

		//----- if this is a camp, add combat level points -----//

		if( firmPtr->firm_id == FIRM_CAMP )
			protectionLevel += 10 + ((FirmCamp*)firmPtr)->total_combat_level();		// +10 for the existence of the camp structure
	}

	return protectionLevel;
}
//---------- End of function Town::protection_available ------//


//-------- Begin of function Town::think_build_market ------//
//
int Town::think_build_market()
{
	if( info.game_date < setup_date + 90 )		// don't build the market too soon, as it may need to migrate to other town
		return 0;

	Nation* ownNation = nation_array[nation_recno];

	if( population < 10 + (100-ownNation->pref_trading_tendency)/20 )
		return 0;

	if( no_neighbor_space )		// if there is no space in the neighbor area for building a new firm.
		return 0;

	//--- check whether the AI can build a new firm next this firm ---//

	if( !ownNation->can_ai_build(FIRM_MARKET) )
		return 0;

	//----------------------------------------------------//
	// If there is already a firm queued for building with
	// a building location that is within the effective range
	// of the this firm.
	//----------------------------------------------------//

	if( ownNation->is_build_action_exist(FIRM_MARKET, center_x, center_y) )
		return 0;

	//-- only build one market place next to this mine, check if there is any existing one --//

	FirmMarket* firmPtr;

	for(int i=0; i<linked_firm_count; i++)
	{
		firmPtr = (FirmMarket*) firm_array[linked_firm_array[i]];

		if(firmPtr->firm_id!=FIRM_MARKET)
			continue;

		//------ if this market is our own one ------//

		if( firmPtr->nation_recno == nation_recno &&
			 ((FirmMarket*)firmPtr)->is_retail_market() )
		{
			return 0;
		}
	}

	//------ queue building a new market -------//

	short buildXLoc, buildYLoc;

	if( !ownNation->find_best_firm_loc(FIRM_MARKET, loc_x1, loc_y1, buildXLoc, buildYLoc) )
	{
		no_neighbor_space = 1;
		return 0;
	}

	ownNation->add_action(buildXLoc, buildYLoc, loc_x1, loc_y1, ACTION_AI_BUILD_FIRM, FIRM_MARKET);

	return 1;
}
//-------- End of function Town::think_build_market ------//


//-------- Begin of function Town::think_build_camp ------//
//
// Think about building military camps for protecting this town.
//
int Town::think_build_camp()
{
	//----- check if any of the other camps protecting this town is still recruiting soldiers, if so, wait until their recruitment is finished. So we can measure the protection available accurately.

	Nation* 	 ownNation = nation_array[nation_recno];
	FirmCamp* firmCamp;
	Firm*		 firmPtr;
	int		 campCount=0;

	for(int i=linked_firm_count-1; i>=0; --i)
	{
		err_when(firm_array.is_deleted(linked_firm_array[i]));

		firmPtr = firm_array[linked_firm_array[i]];

		if(firmPtr->firm_id!=FIRM_CAMP)
			continue;

		firmCamp = (FirmCamp*) firmPtr;

		if( firmCamp->nation_recno != nation_recno )
			continue;

		if( firmCamp->under_construction || firmCamp->ai_recruiting_soldier )  			// if this camp is still trying to recruit soldiers
			return 0;

		campCount++;
	}

	//--- this is one of the few base towns the nation has, then a camp must be built ---//

	if( campCount==0 && is_base_town && ownNation->ai_base_town_count<=2 )
	{
		return ai_build_neighbor_firm(FIRM_CAMP);
	}

	//---- only build camp if we have enough cash and profit ----//

	if( !ownNation->ai_should_spend(70+ownNation->pref_military_development/4) ) 		// 70 to 95
		return 0;

	//---- only build camp if we need more protection than it is currently available ----//

	int protectionNeeded 	= protection_needed();
	int protectionAvailable = protection_available();

	if( protectionAvailable >= protectionNeeded )
		return 0;

	Nation* nationPtr = nation_array[nation_recno];

	if( !(protectionNeeded>0 && protectionAvailable==0) )		// if protection needed > 0, and protection available is 0, we must build a camp now
	{
		int needUrgency = 100 * (protectionNeeded-protectionAvailable) / protectionNeeded;

		if( nationPtr->total_jobless_population-MAX_WORKER <
			 (100-needUrgency) * (200 - nationPtr->pref_military_development) / 200 )
		{
			return 0;
		}
	}

	//--- check if we have enough people to recruit ---//

	int buildFlag = 0;

	if( nationPtr->total_jobless_population >= 16 )
	{
		buildFlag = 1;
	}
	if( nationPtr->total_jobless_population >= 8 && is_base_town )
	{
		buildFlag = 1;
	}
	else if( nationPtr->ai_has_should_close_camp(region_id) )		// there is camp that can be closed
	{
		buildFlag = 1;
	}

	if( !buildFlag )
		return 0;

	return ai_build_neighbor_firm(FIRM_CAMP);
}
//---------- End of function Town::think_build_camp ------//


//-------- Begin of function Town::update_product_supply ------//
//
void Town::update_product_supply()
{
	FirmMarket* firmPtr;
	int  productId;

	memset( has_product_supply, 0, sizeof(has_product_supply) );

	//----- scan for linked market place -----//

	for( int i=linked_firm_count-1 ; i>=0 ; i-- )
	{
		firmPtr = (FirmMarket*) firm_array[ linked_firm_array[i] ];

		if( firmPtr->nation_recno != nation_recno ||
			 firmPtr->firm_id != FIRM_MARKET )
		{
			continue;
		}

		//---- check what type of products they are selling ----//

		for( int j=0 ; j<MAX_MARKET_GOODS ; j++ )
		{
			productId = firmPtr->market_goods_array[j].product_raw_id;

			if (productId > 1)
				has_product_supply[productId-1]++;
		}
	}
}
//---------- End of function Town::update_product_supply ------//


//------- Begin of function Town::think_build_research -------//

int Town::think_build_research()
{
	Nation* nationPtr = nation_array[nation_recno];

	if( !is_base_town )
		return 0;

	if( jobless_population < MAX_WORKER ||
		 nationPtr->total_jobless_population < MAX_WORKER*2 )
	{
		return 0;
	}

	if( nationPtr->true_profit_365days() < 0 )
		return 0;

	if( !nationPtr->ai_should_spend( 25 + nationPtr->pref_use_weapon/2 - nationPtr->ai_research_count*10 ) )
		return 0;

	int totalTechLevel = nationPtr->total_tech_level();

	if( totalTechLevel == tech_res.total_tech_level )		// all technology have been researched
		return 0;

	//--------------------------------------------//

	int maxResearch = 2 * (50+nationPtr->pref_use_weapon) / 50;

	maxResearch = MIN(nationPtr->ai_town_count, maxResearch);

	if( nationPtr->ai_research_count >= maxResearch )
		return 0;

	//---- if any of the existing ones are not full employed ----//

	FirmResearch* firmResearch;

	for( int i=0 ; i<nationPtr->ai_research_count ; i++ )
	{
		firmResearch = (FirmResearch*) firm_array[ nationPtr->ai_research_array[i] ];

		if( firmResearch->region_id != region_id )
			continue;

		if( firmResearch->worker_count < MAX_WORKER )
			return 0;
	}
	//------- queue building a war factory -------//

	return ai_build_neighbor_firm(FIRM_RESEARCH);
}
//-------- End of function Town::think_build_research -------//


//------- Begin of function Town::think_build_war_factory -------//

int Town::think_build_war_factory()
{
	Nation* nationPtr = nation_array[nation_recno];

	if( !is_base_town )
		return 0;

	if( jobless_population < MAX_WORKER ||
		 nationPtr->total_jobless_population < MAX_WORKER*2 )
	{
		return 0;
	}

	int totalWeaponTechLevel = nationPtr->total_tech_level(UNIT_CLASS_WEAPON);

	if( totalWeaponTechLevel==0 )
		return 0;

	//----- see if we have enough money to build & support the weapon ----//

	if( nationPtr->true_profit_365days() < 0 && nationPtr->ai_war_count > 0 )		// if we don't have any war factory, we may want to build one despite that we are losing money
		return 0;

	if( !nationPtr->ai_should_spend( nationPtr->pref_use_weapon/2 ) )
		return 0;

	//--------------------------------------------//

	int maxWarFactory = (1+totalWeaponTechLevel) * nationPtr->pref_use_weapon / 100;

	maxWarFactory = MIN(nationPtr->ai_town_count, maxWarFactory);

	if( nationPtr->ai_war_count >= maxWarFactory )
		return 0;

	//---- if any of the existing ones are not full employed or under capacity ----//

	FirmWar* firmWar;

	for( int i=0 ; i<nationPtr->ai_war_count ; i++ )
	{
		firmWar = (FirmWar*) firm_array[ nationPtr->ai_war_array[i] ];

		if( firmWar->region_id != region_id )
			continue;

		if( firmWar->worker_count < MAX_WORKER || !firmWar->build_unit_id )
			return 0;
	}

	//------- queue building a war factory -------//

	return ai_build_neighbor_firm(FIRM_WAR_FACTORY);
}
//-------- End of function Town::think_build_war_factory -------//


//------- Begin of function Town::think_build_base -------//
//
// Think about building seats of power.
//
int Town::think_build_base()
{
	Nation* nationPtr = nation_array[nation_recno];

	if( !is_base_town )
		return 0;

	//----- see if we have enough money to build & support the weapon ----//

	if( !nationPtr->ai_should_spend(50) )
		return 0;

	if( jobless_population < MAX_BASE_PRAYER/2 ||
		 nationPtr->total_jobless_population < MAX_BASE_PRAYER )
	{
		return 0;
	}

	//------ do a scan on the existing bases first ------//

	static short buildRatingArray[MAX_RACE];

	memset( buildRatingArray, 0, sizeof(buildRatingArray) );

	//--- increase build rating for the seats that this nation knows how to build ---//

	GodInfo* godInfo;

	int i;
	for( i=1 ; i<=god_res.god_count ; i++ )
	{
		godInfo = god_res[i];

		if( godInfo->is_nation_know(nation_recno) )
			buildRatingArray[godInfo->race_id-1] += 100;
	}

	//--- decrease build rating for the seats that the nation currently has ---//

	FirmBase* firmBase;

	for( i=0 ; i<nationPtr->ai_base_count ; i++ )
	{
		firmBase = (FirmBase*) firm_array[ nationPtr->ai_base_array[i] ];

		buildRatingArray[ god_res[firmBase->god_id]->race_id-1 ] = 0;		// only build one 

		/*
		if( firmBase->prayer_count < MAX_BASE_PRAYER )
			buildRatingArray[ god_res[firmBase->god_id]->race_id-1 ] = 0;
		else
			buildRatingArray[ god_res[firmBase->god_id]->race_id-1 ] -= 10;		// -10 for one existing instance
		*/
	}

	//------ decide which is the best to build -------//

	int bestRating=0, bestRaceId=0;

	for( i=0 ; i<MAX_RACE ; i++ )
	{
		if( buildRatingArray[i] > bestRating )
		{
			bestRating = buildRatingArray[i];
			bestRaceId = i+1;
		}
	}

	//------- queue building a seat of power ------//

	if( bestRaceId )
		return ai_build_neighbor_firm(FIRM_BASE, bestRaceId);

	return 0;
}
//-------- End of function Town::think_build_base -------//


//------- Begin of function Town::think_build_inn -------//
//
int Town::think_build_inn()
{
	Nation* ownNation = nation_array[nation_recno];

	if( ownNation->ai_inn_count < ownNation->ai_supported_inn_count() )
	{
		return think_build_firm( FIRM_INN, 1 );
	}

	return 0;
}
//-------- End of function Town::think_build_inn -------//


//------- Begin of function Town::ai_build_neighbor_firm -------//
//
// Build a specific firm next to this town.
//
int Town::ai_build_neighbor_firm(int firmId, int firmRaceId)
{
	short buildXLoc, buildYLoc;
	Nation* nationPtr = nation_array[nation_recno];

	if( !nationPtr->find_best_firm_loc(firmId, loc_x1, loc_y1, buildXLoc, buildYLoc) )
	{
		no_neighbor_space = 1;
		return 0;
	}

	nationPtr->add_action(buildXLoc, buildYLoc, loc_x1, loc_y1, ACTION_AI_BUILD_FIRM, firmId, 1, 0, firmRaceId);

	return 1;
}
//-------- End of function Town::ai_build_neighbor_firm -------//


//------- Begin of function Town::think_split_town -------//
//
// Split the town into two, migrating some population to the new
// town.
//
int Town::think_split_town()
{
	if( jobless_population==0 )			// cannot move if we don't have any mobilizable unit
		return 0;

	//--- split when the population is close to its limit ----//

	Nation* nationPtr = nation_array[nation_recno];

	if( population < 45 + nationPtr->pref_territorial_cohesiveness / 10 )
		return 0;

	//-------- think about which race to move --------//

	int mostRaceId1, mostRaceId2, raceId=0;

	get_most_populated_race(mostRaceId1, mostRaceId2);

	if( mostRaceId2 && jobless_race_pop_array[mostRaceId2-1] > 0 && can_recruit(mostRaceId2) )
		raceId = mostRaceId2;

	else if( mostRaceId1 && jobless_race_pop_array[mostRaceId1-1] > 0 && can_recruit(mostRaceId1) )
		raceId = mostRaceId1;

	else
		raceId = pick_random_race(0, 1);			// 0-recruitable only, 1-also pick spy.

	if( !raceId )
	{
		//---- if the racial mix favors a split of town -----//
		//
		// This is when there are two major races, both of significant
		// population in the town. This is a good time for us to move
		// the second major race to a new town.
		//
		//---------------------------------------------------//

		if( mostRaceId2 && jobless_race_pop_array[mostRaceId2] > 0 &&
			 race_pop_array[mostRaceId2] >= population/3 &&		// the race's has at least 1/3 of the town's total population
			 can_recruit(mostRaceId2) )
		{
			raceId = mostRaceId2;
		}
	}

	if( !raceId )
		return 0;

	//---- check if there is already a town of this race with low population linked to this town, then don't split a new one ---//

	Town* townPtr;

	for( int i=0 ; i<linked_town_count ; i++ )
	{
		townPtr = town_array[ linked_town_array[i] ];

		if( townPtr->nation_recno == nation_recno &&
			 townPtr->population < 20 &&
			 townPtr->majority_race() == raceId )
		{
			return 0;
		}
	}

	//-------- settle to a new town ---------//

	return ai_settle_new(raceId);
}
//-------- End of function Town::think_split_town -------//


//------- Begin of function Town::think_move_between_town -------//
//
void Town::think_move_between_town()
{
	//------ move people between linked towns ------//

	int	ourMajorityRace = majority_race();
	int	raceId, rc, loopCount;
	Town* townPtr;

	for( int i=0 ; i<linked_town_count ; i++ )
	{
		townPtr = town_array[ linked_town_array[i] ];

		if( townPtr->nation_recno != nation_recno )
			continue;

		loopCount=0;

		//--- migrate people to the new town ---//

		while(1)
		{
			err_when( ++loopCount > 100 );

			rc = 0;
			raceId = townPtr->majority_race();		// get the linked town's major race

			if( ourMajorityRace != raceId )		// if our major race is not this race, then move the person to the target town
			{
				rc = 1;
			}
			else //-- if this town's major race is the same as the target town --//
			{
				if( population - townPtr->population > 10 )	// if this town's population is larger than the target town by 10 people, then move
					rc = 1;
			}

			if( rc )
			{
				if( !migrate_to(townPtr->town_recno, COMMAND_AI, raceId) )
					break;
			}
			else
				break;
		}
	}
}
//-------- End of function Town::think_move_between_town -------//


//------- Begin of function Town::ai_settle_new -------//
//
int Town::ai_settle_new(int raceId)
{
	//-------- locate space for the new town --------//

	int xLoc=loc_x1, yLoc=loc_y1;    // xLoc & yLoc are used for returning results

	if( !world.locate_space( &xLoc, &yLoc, loc_x2, loc_y2, STD_TOWN_LOC_WIDTH+2,			// STD_TOWN_LOC_WIDTH+2 for space around the town
									 STD_TOWN_LOC_HEIGHT+2, UNIT_LAND, region_id, 1 ) )
	{
		return 0;
	}

	//------- it must be within the effective town-to-town distance ---//

	if( misc.points_distance( center_x, center_y, xLoc+1+(STD_TOWN_LOC_WIDTH-1)/2,
		yLoc+1+(STD_TOWN_LOC_HEIGHT-1)/2 ) > EFFECTIVE_TOWN_TOWN_DISTANCE )
	{
		return 0;
	}

	// TODO: Should preferably check for a space that has an attached camp, and if can't find then immediately issue order to build a new camp.

	//--- recruit a unit from the town and order it to settle a new town ---//

	int unitRecno = recruit(-1, raceId, COMMAND_AI);

	if( !unitRecno || !unit_array[unitRecno]->is_visible() )
		return 0;

	unit_array[unitRecno]->settle( xLoc+1, yLoc+1 );

	return 1;
}
//-------- End of function Town::ai_settle_new -------//


//------- Begin of function Town::think_attack_nearby_enemy -------//
//
// Think about attacking enemies near the town.
//
int Town::think_attack_nearby_enemy()
{
	enum { SCAN_X_RANGE = 6,
			 SCAN_Y_RANGE = 6  };

	int xLoc1 = loc_x1 - SCAN_X_RANGE;
	int yLoc1 = loc_y1 - SCAN_Y_RANGE;
	int xLoc2 = loc_x2 + SCAN_X_RANGE;
	int yLoc2 = loc_y2 + SCAN_Y_RANGE;

	xLoc1 = MAX( xLoc1, 0 );
	yLoc1 = MAX( yLoc1, 0 );
	xLoc2 = MIN( xLoc2, MAX_WORLD_X_LOC-1 );
	yLoc2 = MIN( yLoc2, MAX_WORLD_Y_LOC-1 );

	//------------------------------------------//

	int		enemyCombatLevel=0;		// the higher the rating, the easier we can attack the target town.
	int 		xLoc, yLoc;
	int		enemyXLoc= -1, enemyYLoc;
	Unit* 	unitPtr;
	Nation* 	nationPtr = nation_array[nation_recno];
	Firm*		firmPtr;
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

				if( unitPtr->cur_action == SPRITE_IDLE &&
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

			//--- if there is an enemy firm here ---//

			else if( locPtr->is_firm() )
			{
				firmPtr = firm_array[locPtr->firm_recno()];

				//------- if this is a monster firm ------//

				if( firmPtr->firm_id == FIRM_MONSTER )		// don't attack monster here, OAI_MONS.CPP will handle that
					continue;

				//------- if this is a firm of our enemy -------//

				if( nationPtr->get_relation_status(firmPtr->nation_recno) == NATION_HOSTILE )
				{
					err_when( firmPtr->nation_recno == nation_recno );

					if( firmPtr->worker_count == 0 )
						enemyCombatLevel += 50;			// empty firm
					else
					{
						Worker* workerPtr = firmPtr->worker_array;

						for( int i=firmPtr->worker_count ; i>0 ; i--, workerPtr++ )
							enemyCombatLevel += workerPtr->hit_points;
					}

					if( enemyXLoc == -1 || misc.random(5)==0 )
					{
						enemyXLoc = firmPtr->loc_x1;
						enemyYLoc = firmPtr->loc_y1;
					}
				}
			}
		}
	}

	//--------- attack the target now -----------//

	if( enemyCombatLevel > 0 )
	{
		err_when( enemyXLoc < 0 );

		nationPtr->ai_attack_target( enemyXLoc, enemyYLoc, enemyCombatLevel );
		return 1;
	}

	return 0;
}
//-------- End of function Town::think_attack_nearby_enemy -------//


//------- Begin of function Town::think_attack_linked_enemy -------//
//
// Think about attacking enemy camps that are linked to this town.
//
int Town::think_attack_linked_enemy()
{
	Firm*   firmPtr;
	Nation* ownNation = nation_array[nation_recno];
	int	  targetCombatLevel;

	for(int i=0; i<linked_firm_count; i++)
	{
		if( linked_firm_enable_array[i] != LINK_EE )		// don't attack if the link is not enabled
			continue;

		firmPtr = firm_array[ linked_firm_array[i] ];

		if( firmPtr->nation_recno == nation_recno )
			continue;

		if( firmPtr->should_close_flag )		// if this camp is about to close
			continue;

		//--- only attack AI firms when they belong to a hostile nation ---//

		if( firmPtr->firm_ai &&
			 ownNation->get_relation_status(firmPtr->nation_recno) != NATION_HOSTILE )
		{
			continue;
		}

		//---- if this is a camp -----//

		if( firmPtr->firm_id == FIRM_CAMP )
		{
			//----- if we are friendly with the target nation ------//

			int nationStatus = ownNation->get_relation_status(firmPtr->nation_recno);

			if( nationStatus >= NATION_FRIENDLY )
			{
				if( !ownNation->ai_should_attack_friendly(firmPtr->nation_recno, 100) )
					continue;

				ownNation->ai_end_treaty(firmPtr->nation_recno);
			}
			else if( nationStatus == NATION_NEUTRAL )
			{
				//-- if the link is off and the nation's miliary strength is bigger than us, don't attack --//

				if( nation_array[firmPtr->nation_recno]->military_rank_rating() >
					 ownNation->military_rank_rating() )
				{
					continue;
				}
			}

			//--- don't attack when the trade rating is high ----//

			int tradeRating = ownNation->trade_rating(firmPtr->nation_recno);

			if( tradeRating > 50 ||
				 tradeRating + ownNation->ai_trade_with_rating(firmPtr->nation_recno) > 100 )
			{
				continue;
			}

			targetCombatLevel = ((FirmCamp*)firmPtr)->total_combat_level();
		}
		else //--- if this is another type of firm ----//
		{
			//--- only attack other types of firm when the status is hostile ---//

			if( ownNation->get_relation_status(firmPtr->nation_recno) != NATION_HOSTILE )
				continue;

			targetCombatLevel = 50;
		}

		//--------- attack now ----------//

		ownNation->ai_attack_target( firmPtr->loc_x1, firmPtr->loc_y1, targetCombatLevel );

		return 1;
	}

	return 0;
}
//-------- End of function Town::think_attack_linked_enemy -------//


//-------- Begin of function Town::think_capture_linked_firm --------//
//
// Try to capture firms linked to this town if all the workers in firms
// resident in this town. 
//
void Town::think_capture_linked_firm()
{
	Firm* firmPtr;

	//----- scan for linked firms -----//

	for( int i=linked_firm_count-1 ; i>=0 ; i-- )
	{
		firmPtr = firm_array[ linked_firm_array[i] ];

		if( firmPtr->nation_recno == nation_recno )		// this is our own firm
			continue;

		if( firmPtr->can_worker_capture(nation_recno) )	// if we can capture this firm, capture it now
		{
			firmPtr->capture_firm(nation_recno);
		}
	}
}
//-------- End of function Town::think_capture_linked_firm --------//


//-------- Begin of function Town::think_capture_enemy_town --------//
//
// Think about sending troops from this town to capture enemy towns.
//
int Town::think_capture_enemy_town()
{
	if( !is_base_town )
		return 0;

	Nation* nationPtr = nation_array[nation_recno];

	if( nationPtr->ai_capture_enemy_town_recno )		// a capturing action is already in process
		return 0;

	int surplusProtection = protection_available() - protection_needed();

	//-- only attack enemy town if we have enough military protection surplus ---//

	if( surplusProtection < 300 - nationPtr->pref_military_courage*2 )
		return 0;

	return nationPtr->think_capture_new_enemy_town(this);
}
//-------- End of function Town::think_capture_enemy_town --------//


//-------- Begin of function Town::think_counter_spy --------//
//
// Think about planting anti-spy units in this town.
//
int Town::think_counter_spy()
{
	if( population >= MAX_TOWN_POPULATION )
		return 0;

	Nation* ownNation = nation_array[nation_recno];

	if( ownNation->total_spy_count > ownNation->total_population * (10+ownNation->pref_spy/5) / 100 )		// 10% to 30%
		return 0;

	if( !ownNation->ai_should_spend(ownNation->pref_counter_spy/2) )		// 0 to 50
		return 0;

	//--- the expense of spies should not be too large ---//

	if( ownNation->expense_365days(EXPENSE_SPY) >
		 ownNation->expense_365days() * (50+ownNation->pref_counter_spy) / 400 )
	{
		return 0;
	}

	//------- check if we need additional spies ------//

	int spyCount;
	int curSpyLevel = spy_array.total_spy_skill_level( SPY_TOWN, town_recno, nation_recno, spyCount );

	if( spyCount >= 1+ownNation->pref_counter_spy/40 )		// 1 to 3 spies at MAX in each town
		return 0;

	//---- if the spy skill needed is more than the current skill level ----//

	int neededSpyLevel = needed_anti_spy_level();

	if( neededSpyLevel > curSpyLevel+30 )
	{
		return ownNation->ai_assign_spy_to_town(town_recno);
	}
	else
		return 0;
}
//-------- End of function Town::think_counter_spy --------//


//-------- Begin of function Town::needed_anti_spy_level --------//
//
// Return the total needed spy level for anti-spying in this town.
//
int Town::needed_anti_spy_level()
{
	return ( linked_firm_count*10 +
				100 * population / MAX_TOWN_POPULATION )
			 * nation_array[nation_recno]->pref_counter_spy / 100;
}
//-------- End of function Town::needed_anti_spy_level --------//


//-------- Begin of function Town::think_spying_town --------//
//
// Think about planting spies into independent towns and enemy towns.
//
int Town::think_spying_town()
{
	int majorityRace = majority_race();

	//---- don't recruit spy if we are low in cash or losing money ----//

	Nation* ownNation = nation_array[nation_recno];

	if( ownNation->total_population < 30-ownNation->pref_spy/10 )		// don't use spies if the population is too low, we need to use have people to grow population
		return 0;

	if( ownNation->total_spy_count > ownNation->total_population * (10+ownNation->pref_spy/5) / 100 )		// 10% to 30%
		return 0;

	if( !ownNation->ai_should_spend(ownNation->pref_spy/2) )		// 0 to 50
		return 0;

	//--- pick minority units first (also for increasing town harmony) ---//

	for( int raceId=1 ; raceId<=MAX_RACE ; raceId++ )
	{
		if( race_pop_array[raceId-1]==0 || raceId==majorityRace )
			continue;

		if( !can_recruit(raceId) )
			continue;

		if( think_spying_town_assign_to(raceId) )
			return 1;
	}

	//---- then think about assign spies of majority race ----//

	if( ownNation->pref_spy > 50 )		// only if pref_spy is > 50, it will consider using spies of majority race
	{
		if( can_recruit(majorityRace) )
		{
			if( think_spying_town_assign_to(majorityRace) )
				return 1;
		}
	}

	return 0;
}
//-------- End of function Town::think_spying_town --------//


//-------- Begin of function Town::think_spying_town_assign_to --------//
//
// Think about planting spies into independent towns and enemy towns.
//
int Town::think_spying_town_assign_to(int raceId)
{
	Nation* ownNation = nation_array[nation_recno];

	int targetTownRecno = ownNation->think_assign_spy_target_town(race_id, region_id);

	if( !targetTownRecno )
		return 0;

	//------- assign the spy now -------//

	int unitRecno = recruit(SKILL_SPYING, raceId, COMMAND_AI);

	if( !unitRecno )
		return 0;

	Town* targetTown = town_array[targetTownRecno];

	int actionRecno = ownNation->add_action( targetTown->loc_x1, targetTown->loc_y1,
							-1, -1, ACTION_AI_ASSIGN_SPY, targetTown->nation_recno, 1, unitRecno );

	if( !actionRecno )
		return 0;

	train_unit_action_id = ownNation->get_action(actionRecno)->action_id;

	return 1;
}
//-------- End of function Town::think_spying_town_assign_to --------//


//-------- Begin of function Town::update_base_town_status --------//
//
void Town::update_base_town_status()
{
	int newBaseTownStatus = new_base_town_status();

	if( newBaseTownStatus == is_base_town )
		return;

	Nation* ownNation = nation_array[nation_recno];

	if( is_base_town )
		ownNation->ai_base_town_count--;

	if( newBaseTownStatus )
		ownNation->ai_base_town_count++;

	is_base_town = newBaseTownStatus;
}
//-------- End of function Town::update_base_town_status --------//


//-------- Begin of function Town::new_base_town_status --------//
//
int Town::new_base_town_status()
{
	Nation* ownNation = nation_array[nation_recno];

	if( population > 20 + ownNation->pref_territorial_cohesiveness/10 )
		return 1;

	//---- if there is only 1 town, then it must be the base town ---//

	if( ownNation->ai_town_count==1 )
		return 1;

	//---- if there is only 1 town in this region, then it must be the base town ---//

	AIRegion* aiRegion = ownNation->get_ai_region(region_id);

	if( aiRegion && aiRegion->town_count==1 )
		return 1;

	//--- don't drop if there are employed villagers ---//

	if( jobless_population != population )
		return 1;

	//---- if this town is linked to a base town, also don't drop it ----//

	Town* townPtr;

	for( int i=linked_town_count-1 ; i>=0 ; i-- )
	{
		townPtr = town_array[ linked_town_array[i] ];

		if( townPtr->nation_recno == nation_recno &&
			 townPtr->is_base_town )
		{
			return 1;
		}
	}
/*
	//---- if there is not a single town meeting the above criteria, then set the town with the largest population to be the base town. ----//

	if( ownNation->ai_base_town_count <= 1 )
	{
		for( i=ownNation->ai_town_count-1 ; i>=0 ; i-- )
		{
			townPtr = town_array[ ownNation->ai_town_array[i] ];

			if( townPtr->population > population )		// if this town is not the largest town return 0.
				return 0;
		}

		return 1;		// return 1 if this town is the largest town.
	}
*/
	return 0;
}
//-------- End of function Town::new_base_town_status --------//


//-------- Begin of function Town::think_scout --------//
//
int Town::think_scout()
{
	Nation* ownNation = nation_array[nation_recno];

	if( config.explore_whole_map )
		return 0;

	if( info.game_date > info.game_start_date + 50 + ownNation->pref_scout )	// only in the first half year of the game
		return 0;

	if( ownNation->ai_town_count > 1 ) 													// only when there is only one town
		return 0;

	//-------------------------------------------//

	int destX, destY;

	if( misc.random(2)==0 )
		destX = center_x + 50 + misc.random(50);
	else
		destX = center_x - 50 - misc.random(50);

	if( misc.random(2)==0 )
		destY = center_y + 50 + misc.random(50);
	else
		destY = center_y - 50 - misc.random(50);

	destX = MAX(0, destX);
	destX = MIN(MAX_WORLD_X_LOC-1, destX);

	destY = MAX(0, destY);
	destY = MIN(MAX_WORLD_Y_LOC-1, destY);

	ownNation->add_action( destX, destY, loc_x1, loc_y1, ACTION_AI_SCOUT, 0);

	return 1;
}
//-------- End of function Town::think_scout --------//
