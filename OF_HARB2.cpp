//Filename    : OF_HARB2.CPP
//Description : Firm Harbor - AI functions

#include <OINFO.h>
#include <OU_MARI.h>
#include <ONATION.h>
#include <OF_HARB.h>

//------- Begin of function FirmHarbor::process_ai -----------//
//
void FirmHarbor::process_ai()
{
	if( info.game_date%30 == firm_recno%30 )
		think_build_ship();
/*
	if( info.game_date%90 == firm_recno%90 )
		think_build_firm();

	if( info.game_date%60 == firm_recno%60 )
		think_trade();
*/
}
//--------- End of function FirmHarbor::process_ai -----------//


//------- Begin of function FirmHarbor::think_build_firm -----------//
//
void FirmHarbor::think_build_firm()
{
	Nation* ownNation = nation_array[nation_recno];

	if( ownNation->cash < 2000 )		// don't build if the cash is too low
		return;

	if( ownNation->true_profit_365days() < (50-ownNation->pref_use_marine)*20 )		//	-1000 to +1000
		return;

	//----- think about building markets ------//

	if( ownNation->pref_trading_tendency >= 60 )
	{
		if( ai_build_firm(FIRM_MARKET) )
			return;
	}

	//----- think about building camps ------//

	if( ownNation->pref_military_development/2 +
		 (linked_firm_count + ownNation->ai_ship_count + ship_count) * 10 +
		 ownNation->total_jobless_population*2 > 150 )
	{
		ai_build_firm(FIRM_CAMP);
	}
}
//--------- End of function FirmHarbor::think_build_firm -----------//


//------- Begin of function FirmHarbor::ai_build_firm -----------//
//
int FirmHarbor::ai_build_firm(int firmId)
{
	if( no_neighbor_space )		// if there is no space in the neighbor area for building a new firm.
		return 0;

	Nation* ownNation = nation_array[nation_recno];

	//--- check whether the AI can build a new firm next this firm ---//

	if( !ownNation->can_ai_build(firmId) )
		return 0;

	//-- only build one market place next to this mine, check if there is any existing one --//

		//### begin alex 24/9 ###//
	/*FirmMarket* firmPtr;

	for(int i=0; i<linked_firm_count; i++)
	{
		err_when(!linked_firm_array[i] || firm_array.is_deleted(linked_firm_array[i]));

		firmPtr = (FirmMarket*) firm_array[linked_firm_array[i]];

		if( firmPtr->firm_id!=firmId )
			continue;

		//------ if this market is our own one ------//

		if( firmPtr->nation_recno == nation_recno )
			return 0;
	}*/
	Firm* firmPtr;

	for(int i=0; i<linked_firm_count; i++)
	{
		err_when(!linked_firm_array[i] || firm_array.is_deleted(linked_firm_array[i]));

		firmPtr = firm_array[linked_firm_array[i]];

		if( firmPtr->firm_id!=firmId )
			continue;

		//------ if this market is our own one ------//

		if( firmPtr->nation_recno == nation_recno )
			return 0;
	}
	//#### end alex 24/9 ####//

	//------ queue building a new market -------//

	short buildXLoc, buildYLoc;

	if( !ownNation->find_best_firm_loc(firmId, loc_x1, loc_y1, buildXLoc, buildYLoc) )
	{
		no_neighbor_space = 1;
		return 0;
	}

	ownNation->add_action(buildXLoc, buildYLoc, loc_x1, loc_y1, ACTION_AI_BUILD_FIRM, firmId);

	return 1;
}
//--------- End of function FirmHarbor::ai_build_firm -----------//


//------- Begin of function FirmHarbor::think_trade -----------//
//
int FirmHarbor::think_trade()
{
	//---- see if we have any free ship available ----//

	Nation*     ownNation  = nation_array[nation_recno];
	UnitMarine* unitMarine = ai_get_free_trade_ship();

	if( !unitMarine )
		return 0;

	//--- if this harbor is not linked to any trade firms, return now ---//

	if( total_linked_trade_firm() == 0 )
		return 0;

	//----- scan for another harbor to trade with this harbor ----//

	Firm* 		firmPtr;
	FirmHarbor* firmHarbor;

	int i;
	for( i=firm_array.size() ; i>0 ; i-- )
	{
		if( firm_array.is_deleted(i) )
			continue;

		firmPtr = firm_array[i];

		//### begin alex 24/9 ###//
		if(firmPtr->firm_id!=FIRM_HARBOR)
			continue;
		//#### end alex 24/9 ####//

		firmHarbor = (FirmHarbor*) firmPtr;

		if( firmHarbor->total_linked_trade_firm() == 0 )
			continue;

		if( !ownNation->has_trade_ship(firm_recno, i) )		// if there has been any ships trading between these two harbors yet
			break;
	}

	if( i==0 )
		return 0;

	//------ try to set up a sea trade now ------//

	unitMarine->set_stop( 1, loc_x1, loc_y1, COMMAND_AI);
	unitMarine->set_stop( 2, firmHarbor->loc_x1, firmHarbor->loc_y1, COMMAND_AI);

	unitMarine->set_stop_pick_up(1, AUTO_PICK_UP, COMMAND_AI);
	unitMarine->set_stop_pick_up(2, AUTO_PICK_UP, COMMAND_AI);

	return 1;
}
//--------- End of function FirmHarbor::think_trade -----------//


//------ Begin of function FirmHarbor::ai_get_free_trade_ship ------//
//
UnitMarine* FirmHarbor::ai_get_free_trade_ship()
{
	Nation* ownNation = nation_array[nation_recno];
	UnitMarine* unitMarine;

	for( int i=ownNation->ai_ship_count-1 ; i>=0 ; i-- )
	{
		unitMarine = (UnitMarine*) unit_array[ ownNation->ai_ship_array[i] ];

		//--- if this is a goods carrying ship and it doesn't have a defined trade route ---//

		if( unitMarine->stop_defined_num < 2 &&
			 unit_res[unitMarine->unit_id]->carry_goods_capacity > 0 )
		{
			return unitMarine;
		}
	}

	return NULL;
}
//-------- End of function FirmHarbor::ai_get_free_trade_ship ------//


//------- Begin of function FirmHarbor::think_build_ship -----------//
//
void FirmHarbor::think_build_ship()
{
	if( build_unit_id )		// if it's currently building a ship
		return;

	if( !can_build_ship() )		// full, cannot build anymore
		return;

	Nation* ownNation = nation_array[nation_recno];

	if( !ownNation->ai_should_spend( 50+ownNation->pref_use_marine/4 ) )
		return;

	//---------------------------------------------//
	//
	// For Transport, in most cases, an AI will just
	// need one to two.
	//
	// For Caravel and Galleon, the AI will build as many
	// as the harbor can hold if any of the human players
	// has more ships than the AI has.
	//
	//---------------------------------------------//

	int buildId=0, rc=0;
	int enemyShipCount = ownNation->max_human_battle_ship_count();		// return the no. of ships of the enemy that is strongest on the sea

	if( unit_res[UNIT_GALLEON]->get_nation_tech_level(nation_recno) > 0 )
	{
		buildId = UNIT_GALLEON;

		if( ownNation->ai_ship_count < 2 + (ownNation->pref_use_marine>50) )
			rc = 1;
		else
			rc = enemyShipCount > ownNation->ai_ship_count;
	}
	else if( unit_res[UNIT_CARAVEL]->get_nation_tech_level(nation_recno) > 0 )
	{
		buildId = UNIT_CARAVEL;

		if( ownNation->ai_ship_count < 2 + (ownNation->pref_use_marine>50) )
			rc = 1;
		else
			rc = enemyShipCount > ownNation->ai_ship_count;
	}
	else
	{
		buildId = UNIT_TRANSPORT;

		if( ownNation->ai_ship_count < 2 )
			rc = 1;
	}

	//---------------------------------------------//

	if( buildId && rc )
		build_ship( buildId, COMMAND_AI );
}
//--------- End of function FirmHarbor::think_build_ship -----------//
