//Filename    : OFIRMAI.CPP
//Description : AI functions for the class Firm

#include <OF_INN.H>
#include <OF_MINE.H>
#include <OF_FACT.H>
#include <OF_CAMP.H>
#include <OF_MARK.H>
#include <OUNITRES.H>
#include <OTALKRES.H>
#include <ONATION.H>

//--------- Begin of function Firm::process_common_ai --------//
//
// AI processing functions common for all firm types.
//
void Firm::process_common_ai()
{
	if( info.game_date%30==firm_recno%30 )
		think_repair();

	//------ think about closing this firm ------//

	if( !should_close_flag )
	{
		if( ai_should_close() )
		{
			should_close_flag = 1;
			nation_array[nation_recno]->firm_should_close_array[firm_id-1]++;
		}
	}
}
//--------- End of function Firm::process_common_ai --------//


//------- Begin of function Firm::think_repair -----------//
//
void Firm::think_repair()
{
	Nation* ownNation = nation_array[nation_recno];

	//----- check if the damage is serious enough -----//

	if( hit_points >=
		 max_hit_points * (70+ownNation->pref_repair_concern/4) / 100 )	// 70% to 95%
	{
		return;
	}

	//--- if it's no too heavily damaged, it is just that the AI has a high concern on this ---//

	if( hit_points >= max_hit_points * 80 / 100 )
	{
		if( ownNation->total_jobless_population < 15 )
			return;
	}

	//------- queue assigning a construction worker now ------//

	ownNation->add_action(loc_x1, loc_y1, -1, -1, ACTION_AI_ASSIGN_CONSTRUCTION_WORKER, firm_id);
}
//--------- End of function Firm::think_repair -----------//


//------- Begin of function Firm::ai_del_firm -----------//
//
// Delete the firm no matter what status this firm is in.
//
void Firm::ai_del_firm()
{
	if( under_construction )
	{
		cancel_construction(COMMAND_PLAYER);
	}
	else
	{
		if( can_sell() )
			sell_firm(COMMAND_AI);
		else
			destruct_firm(COMMAND_AI);
	}
}
//--------- End of function Firm::ai_del_firm -----------//


//------- Begin of function Firm::ai_should_close -----------//
//
// This is function is for derived class to overload.
// 
int Firm::ai_should_close()
{
	return 0;
}
//--------- End of function Firm::ai_should_close -----------//


//------- Begin of function Firm::think_hire_inn_unit -------//
//
int Firm::think_hire_inn_unit()
{
	if( !nation_array[nation_recno]->ai_should_hire_unit(30) )		// 30 - importance rating
		return 0;

	//---- one firm only hire one foreign race worker ----//

	int i, foreignRaceCount=0;
	int majorityRace = majority_race();

	if( majorityRace )
	{
		for( i=0 ; i<worker_count ; i++ )
		{
			if( worker_array[i].race_id != majorityRace )
				foreignRaceCount++;
		}
	}

	//-------- try to get skilled workers from inns --------//

	Nation*  nationPtr = nation_array[nation_recno];
	FirmInn* firmInn, *bestInn=NULL;
	int		curRating, bestRating=0, bestInnUnitId=0;
	int		prefTownHarmony = nationPtr->pref_town_harmony;

	for( i=0 ; i<nationPtr->ai_inn_count ; i++ )
	{
		firmInn = (FirmInn*) firm_array[ nationPtr->ai_inn_array[i] ];

		if( firmInn->region_id != region_id )
			continue;

		InnUnit* innUnit = firmInn->inn_unit_array;

		for( int j=0 ; j<firmInn->inn_unit_count ; j++, innUnit++ )
		{
			if( innUnit->skill.skill_id != firm_skill_id )
				continue;

			//-------------------------------------------//
			// Rating of a unit to be hired is based on:
			//
			// -distance between the inn and this firm.
			// -whether the unit is racially homogenous to the majority of the firm workers
			//
			//-------------------------------------------//

			curRating = world.distance_rating( center_x, center_y,
							firmInn->center_x, firmInn->center_y );

			curRating += innUnit->skill.skill_level;

			if( majorityRace == unit_res[innUnit->unit_id]->race_id )
			{
				curRating += prefTownHarmony;
			}
			else
			{
				//----------------------------------------------------//
				// Don't pick this unit if it isn't racially homogenous
				// to the villagers, and its pref_town_harmony is higher
				// than its skill level. (This means if its skill level
				// is low, its chance of being selected is lower.
				//----------------------------------------------------//

				if( majorityRace )
				{
					if( foreignRaceCount>0 || prefTownHarmony > innUnit->skill.skill_level-50 )
						continue;
				}
			}

			if( curRating > bestRating )
			{
				bestRating    = curRating;
				bestInn       = firmInn;
				bestInnUnitId = j+1;
			}
		}
	}

	//-----------------------------------------//

	if( bestInn )
	{
		int unitRecno = bestInn->hire(bestInnUnitId);

		if( unitRecno )
		{
			unit_array[unitRecno]->assign(loc_x1, loc_y1);
			return 1;
		}
	}

	return 0;
}
//--------- End of function Firm::think_hire_inn_unit -------//


//------- Begin of function Firm::being_attacked -----------//
//
void Firm::being_attacked(int attackerUnitRecno)
{
	last_attacked_date = info.game_date;

	if( nation_recno && firm_ai )
	{
		if( unit_array[attackerUnitRecno]->nation_recno == nation_recno )		// this can happen when the unit has just changed nation
			return;

		nation_array[nation_recno]->ai_defend(attackerUnitRecno);
	}
}
//--------- End of function Firm::being_attacked -----------//


//------- Begin of function Firm::ai_recruit_worker -----------//
//
int Firm::ai_recruit_worker()
{
	if( worker_count == MAX_WORKER )
		return 0;

	Nation* nationPtr = nation_array[nation_recno];
	Town*   townPtr;

	for( int i=0; i<linked_town_count ; i++ )
	{
		if( linked_town_enable_array[i] != LINK_EE )
			continue;

		townPtr = town_array[ linked_town_array[i] ];

		//-- only recruit workers from towns of other nations if we don't have labor ourselves

		if( townPtr->nation_recno != nation_recno &&
			 nationPtr->total_jobless_population > MAX_WORKER )
		{
			continue;
		}

		if( townPtr->jobless_population > 0 )
			return 0;		// don't order units to move into it as they will be recruited from the town automatically
	}

	//---- order workers to move into the firm ----//

	nationPtr->add_action(loc_x1, loc_y1, -1, -1, ACTION_AI_ASSIGN_WORKER, firm_id, MAX_WORKER-worker_count);

	return 1;
}
//--------- End of function Firm::ai_recruit_worker -----------//


//------- Begin of function Firm::ai_build_neighbor_firm -----------//
//
int Firm::ai_build_neighbor_firm(int firmId)
{
	short   buildXLoc, buildYLoc;
	Nation* nationPtr = nation_array[nation_recno];

	if( !nationPtr->find_best_firm_loc(firmId, loc_x1, loc_y1, buildXLoc, buildYLoc) )
	{
		no_neighbor_space = 1;
		return 0;
	}

	nationPtr->add_action(buildXLoc, buildYLoc, loc_x1, loc_y1, ACTION_AI_BUILD_FIRM, firmId);
	return 1;
}
//--------- End of function Firm::ai_build_neighbor_firm -----------//


//--------- Begin of function Firm::ai_update_link_status ---------//
//
// Updating link status of this firm with towns. 
//
void Firm::ai_update_link_status()
{
	err_when( firm_id == FIRM_CAMP );		// FirmCamp has its own ai_update_link_status(), this version shouldn't be called.

	if( !worker_array )		// if this firm does not need any workers. 
		return;

	if( is_worker_full() )	// if this firm already has all the workers it needs. 
		return;

	//------------------------------------------------//

	Nation* ownNation = nation_array[nation_recno];
	int	  i, rc;

	for( i=0 ; i<linked_town_count ; i++ )
	{
		Town* townPtr = town_array[linked_town_array[i]];

		//--- enable link to hire people from the town ---//

		rc = townPtr->nation_recno==0 ||		// either it's an independent town or it's friendly or allied to our nation
			  ownNation->get_relation_status(townPtr->nation_recno) >= NATION_FRIENDLY;

		toggle_town_link( i+1, rc, COMMAND_AI );
	}
}
//----------- End of function Firm::ai_update_link_status ----------//


//------- Begin of function Firm::think_build_factory -----------//
//
int Firm::think_build_factory(int rawId)
{
	if( no_neighbor_space )		// if there is no space in the neighbor area for building a new firm.
		return 0;

	Nation* nationPtr = nation_array[nation_recno];

	//--- check whether the AI can build a new firm next this firm ---//

	if( !nationPtr->can_ai_build(FIRM_FACTORY) )
		return 0;

	//---------------------------------------------------//

	int 			 factoryCount=0;
	FirmFactory* firmPtr;
	Firm*			 firmMarket;

	for(int i=0; i<linked_firm_count; i++)
	{
		err_when(!linked_firm_array[i] || firm_array.is_deleted(linked_firm_array[i]));

		firmPtr = (FirmFactory*) firm_array[linked_firm_array[i]];

		if(firmPtr->firm_id!=FIRM_FACTORY)
			continue;

		if( firmPtr->nation_recno != nation_recno )
			continue;

		if( firmPtr->product_raw_id != rawId )
			continue;

		//--- if one of own factories still has not recruited enough workers ---//

		if( firmPtr->worker_count < MAX_WORKER )
			return 0;

		//---------------------------------------------------//
		//
		// If this factory has a medium to high level of stock,
		// this means the bottleneck is not at the factories,
		// building more factories won't solve the problem.
		//
		//---------------------------------------------------//

		if( firmPtr->stock_qty > firmPtr->max_stock_qty * 0.1 )
			return 0;

		//---------------------------------------------------//
		//
		// Check if this factory is just outputing goods to
		// a market and it is actually not overcapacity.
		//
		//---------------------------------------------------//

		for( int j=firmPtr->linked_firm_count-1 ; j>=0 ; j-- )
		{
			if( firmPtr->linked_firm_enable_array[j] != LINK_EE )
				continue;

			firmMarket = firm_array[ firmPtr->linked_firm_array[j] ];

			if( firmMarket->firm_id != FIRM_MARKET )
				continue;

			//--- if this factory is producing enough goods to the market place, then it means it is still quite efficient

			MarketGoods *marketGoods = ((FirmMarket*)firmMarket)->market_product_array[rawId-1];

			if( marketGoods && marketGoods->stock_qty > 100 )
				return 0;
		}

		//----------------------------------------------//

		factoryCount++;
	}

	//---- don't build additional factory if we don't have enough peasants ---//

	if( factoryCount>=1 && !nationPtr->ai_has_enough_food() )
		return 0;

	//-- if there isn't much raw reserve left, don't build new factories --//

	if( firm_id == FIRM_MINE )
	{
		if( ((FirmMine*)this)->reserve_qty < 1000 && factoryCount>=1 )
			return 0;
	}

	//--- only build additional factories if we have a surplus of labor ---//

	if( nationPtr->total_jobless_population < factoryCount * MAX_WORKER )
		return 0;

	//--- only when we have checked it three times and all say it needs a factory, we then build a factory ---//

	if( ++ai_should_build_factory_count >= 3 )
	{
		short buildXLoc, buildYLoc;

		if( !nationPtr->find_best_firm_loc(FIRM_FACTORY, loc_x1, loc_y1, buildXLoc, buildYLoc) )
		{
			no_neighbor_space = 1;
			return 0;
		}

		nationPtr->add_action(buildXLoc, buildYLoc, loc_x1, loc_y1, ACTION_AI_BUILD_FIRM, FIRM_FACTORY);

		ai_should_build_factory_count = 0;
	}

	return 1;
}
//--------- End of function Firm::think_build_factory -----------//


//------- Begin of function Firm::think_capture -----------//
//
int Firm::think_capture()
{
	Nation* nationPtr;

	for( int i=nation_array.size() ; i>0 ; i-- )
	{
		if( nation_array.is_deleted(i) )
			continue;

		nationPtr = nation_array[i];

		if( nationPtr->is_ai() && can_worker_capture(i) )
			break;
	}

	if( i==0 )
		return 0;

	//------- capture the firm --------//

	capture_firm(i);

	//------ order troops to attack nearby enemy camps -----//

	Firm *firmPtr, *bestTarget=NULL;
	int  curDistance, minDistance=0x1000;

	for( i=firm_array.size() ; i>0 ; i-- )
	{
		if( firm_array.is_deleted(i) )
			continue;

		firmPtr = firm_array[i];

		//----- only attack enemy camps -----//

		if( firmPtr->nation_recno != nation_recno ||
			 firmPtr->firm_id != FIRM_CAMP )
		{
			continue;
		}

		curDistance = m.points_distance(center_x, center_y,
						  firmPtr->center_x, firmPtr->center_y );

		//--- only attack camps within 15 location distance to this firm ---//

		if( curDistance < 15 && curDistance < minDistance )
		{
			minDistance = curDistance;
			bestTarget  = firmPtr;
		}
	}

	if( bestTarget )
	{
		int useAllCamp = nationPtr->pref_military_courage > 60 || m.random(3)==0;

		nationPtr->ai_attack_target( bestTarget->loc_x1, bestTarget->loc_y1,
			((FirmCamp*)bestTarget)->total_combat_level(), 0, 0, 0, 0, useAllCamp );
	}

	return 1;
}
//--------- End of function Firm::think_capture -----------//


//------- Begin of function Firm::think_linked_town_change_nation ------//
//
// This function is called by Town::set_nation() when a town linked
// to this firm has changed nation.
//
// <int> linkedTownRecno - the recno of the town that has changed nation.
// <int> oldNationRecno  - the old nation recno of the town
// <int> newNationRecno  - the new nation recno of the town
//
void Firm::think_linked_town_change_nation(int linkedTownRecno, int oldNationRecno, int newNationRecno)
{

}
//-------- End of function Firm::think_linked_town_change_nation ------//


//--------- Begin of function Firm::ai_firm_captured --------//
//
// This is function is called when the AI's firm is just
// about to be captured.
//
void Firm::ai_firm_captured(int capturerNationRecno)
{
	Nation* ownNation = nation_array[nation_recno];

	if( !ownNation->is_ai() )			//**BUGHERE
		return;

	if( ownNation->get_relation(capturerNationRecno)->status >= NATION_FRIENDLY )
		ownNation->ai_end_treaty(capturerNationRecno);

	talk_res.ai_send_talk_msg(capturerNationRecno, nation_recno, TALK_DECLARE_WAR);
}
//--------- End of function Firm::ai_firm_captured --------//

