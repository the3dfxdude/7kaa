//Filename   : OAI_MILI.CPP
//Description: AI - think about expanding the military force

#include <ALL.h>
#include <OTOWN.h>
#include <OF_CAMP.h>
#include <ONATION.h>

//--------- Begin of function Nation::think_military --------//

void Nation::think_military()
{
	//---- don't build new camp if we our food consumption > production ----//

	if( yearly_food_change() < 0 )
	{
		think_close_camp();		// think about closing down an existing one
		return;
	}

	//--- think about whether it should expand now ---//

	if( !ai_should_expand_military() && !ai_is_troop_need_new_camp() )
		return;

	//----- think about where to expand -----//

	int  bestRating=0, curRating;
	Town *townPtr, *bestTownPtr=NULL;

	for( int i=ai_town_count-1 ; i>=0 ; i-- )
	{
		townPtr = town_array[ ai_town_array[i] ];

		if( !townPtr->is_base_town )		// only expand on base towns
			continue;

		if( townPtr->no_neighbor_space )		// if there is no space in the neighbor area for building a new firm.
			continue;

		curRating = townPtr->population; 	//**BUGHERE, to be modified.

		if( curRating > bestRating )
		{
			bestRating  = curRating;
			bestTownPtr = townPtr;
		}
	}

	if( !bestTownPtr )
		return;

	//--------- queue building the camp now ---------//

	short buildXLoc, buildYLoc;

	if( !find_best_firm_loc(FIRM_CAMP, bestTownPtr->loc_x1, bestTownPtr->loc_y1, buildXLoc, buildYLoc) )
	{
		bestTownPtr->no_neighbor_space = 1;
		return;
	}

	add_action(buildXLoc, buildYLoc, bestTownPtr->loc_x1, bestTownPtr->loc_y1, ACTION_AI_BUILD_FIRM, FIRM_CAMP);
}

//--------- Begin of function Nation::think_military --------//


//--------- Begin of function Nation::ai_should_expand_military --------//

int Nation::ai_should_expand_military()
{
	//----- don't expand if it is losing money -----//

	if( true_profit_365days() < 0 )
		return 0;

	//----- expand if it has enough cash -----//

	if( !ai_should_spend(pref_military_development/2) )
		return 0;

	//--- check whether the current military force is already big enough ---//

	if( ai_camp_count * 9 > total_jobless_population * (50+pref_military_development) / 150 )		// (50 to 150) / 150
		return 0;

	//---- see if any of the camps are still needed soldiers -----//

	int		 i, soldierCount, freeSpaceCount=0;
	FirmCamp* firmCamp;

	for( i=ai_camp_count-1 ; i>=0 ; i-- )
	{
		firmCamp = (FirmCamp*) firm_array[ ai_camp_array[i] ];

		if( firmCamp->should_close_flag )		// exclude those going to be closed down
			continue;

		//---- only build a new one when existing ones are all full ----//

		soldierCount = (firmCamp->overseer_recno>0) + firmCamp->worker_count +
							firmCamp->patrol_unit_count;

		freeSpaceCount += 9-soldierCount;

		if( firmCamp->ai_recruiting_soldier )
			return 0;
	}

	return freeSpaceCount < 9;		// build build new ones when the existing ones are almost full
}
//--------- Begin of function Nation::ai_should_expand_military --------//


//--------- Begin of function Nation::ai_is_troop_need_new_camp --------//

int Nation::ai_is_troop_need_new_camp()
{
	//--- check whether the current military force is already big enough ---//

	if( ai_has_too_many_camp() )
		return 0;

	//----- expand if it has enough cash -----//

	if( !ai_should_spend(50+pref_military_development/2) )
		return 0;

	//----- if existing camps can already host all units ----//

	int neededSoldierSpace = total_human_count+total_weapon_count - ai_camp_count*MAX_WORKER;
	int neededGeneralSpace = total_general_count - ai_camp_count;

	return neededSoldierSpace >= 9 + 20 * (100-pref_military_development) / 200 &&
			 neededGeneralSpace >= 2 + 4  * (100-pref_military_development) / 200;
}
//--------- Begin of function Nation::ai_is_troop_need_new_camp --------//


//--------- Begin of function Nation::ai_has_too_many_camp --------//

int Nation::ai_has_too_many_camp()
{
	return ai_camp_count * 9 > total_jobless_population * (150+pref_military_development) / 150;		// (150 to 250) / 150
}
//--------- Begin of function Nation::ai_has_too_many_camp --------//


//--------- Begin of function Nation::think_close_camp --------//

int Nation::think_close_camp()
{
	return 0;

/*
	FirmCamp *firmCamp, *closeCamp=NULL;
	int 		curRating, bestRating=0;

	for( int i=ai_camp_count-1 ; i>=0 ; i-- )
	{
		firmCamp = (FirmCamp*) firm_array[ ai_camp_array[i] ];

		curRating = world.distance_rating(firmCamp->center_x, firmCamp->center_y
*/
}
//--------- Begin of function Nation::think_close_camp --------//

