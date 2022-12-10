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

//Filename    : OU_CARA2.CPP
//Description : Unit Caravan - AI functions

#include <OINFO.h>
#include <ONATION.h>
#include <OF_FACT.h>
#include <OU_CARA.h>


//------- Begin of function UnitCaravan::init_derived --------//

void UnitCaravan::init_derived()
{
	last_load_goods_date = info.game_date;
}
//------- End of function UnitCaravan::init_derived --------//


//------- Begin of function UnitCaravan::process_ai --------//
//
void UnitCaravan::process_ai()
{
	//-- Think about removing stops whose owner nation is at war with us. --//

	if( info.game_date%30 == sprite_recno%30 )
	{
		if( think_del_stop() )
			return;

		//------ think about setting pickup goods type -----//

		think_set_pick_up_type();
	}

	//------ Think about resigning this caravan -------//

	think_resign();
}
//------- End of function UnitCaravan::process_ai --------//


//------- Begin of function UnitCaravan::think_resign --------//

int UnitCaravan::think_resign()
{
	if( !is_visible() )		// can only resign when the caravan is not in a stop
		return 0;

	//---- resign this caravan if it has only one stop ----//

	if( stop_defined_num < 2 )
	{
		resign(COMMAND_AI);
		return 1;
	}

	//---- if the caravan hasn't loaded any goods for a year ----//

	if( info.game_date > last_load_goods_date + 365 &&
		 info.game_date%30 == sprite_recno%30 )			// don't call too often as the action may fail and it takes a while to call the function each time
	{
		//--- don't resign if this caravan carries any goods ---//

		for( int i=0 ; i<MAX_RAW ; i++ )
		{
			if( raw_qty_array[i] > 0 || product_raw_qty_array[i] > 0 )
				return 0;
		}

		//------ resign now --------//

		resign(COMMAND_AI);
		return 1;
	}

	//--- if this caravan is travelling between two retail markets ---//
	//--- (neither of them has any direct supplies) ------//

	if( info.game_date%30 == sprite_recno%30 )			// don't call too often as the action may fail and it takes a while to call the function each time
	{
		for( int i=stop_defined_num ; i>0 ; i-- )
		{
			int firmRecno = stop_array[i-1].firm_recno;

			if( firm_array.is_deleted(firmRecno) ||
				 firm_array[firmRecno]->firm_id != FIRM_MARKET )
			{
				del_stop(i, COMMAND_AI);
				return 1;
			}

			//--- see if this market has any direct supply ---//

			FirmMarket* firmMarket = (FirmMarket*) firm_array[firmRecno];

			MarketGoods* marketGoods = firmMarket->market_goods_array;

			for( int j=0 ; j<MAX_MARKET_GOODS ; j++, marketGoods++ )
			{
				if( marketGoods->supply_30days() > 0 )
					return 0;
			}
		}

		//--- resign now if none of the linked markets have any direct supplies ---//

		resign(COMMAND_AI);
		return 1;
	}

	return 0;
}
//------- End of function UnitCaravan::think_resign --------//


//------- Begin of function UnitCaravan::think_del_stop --------//
//
// Think about removing stops whose owner nation is at war with us.
//
int UnitCaravan::think_del_stop()
{
	if( !is_visible() )		// cannot del stop if the caravan is inside a market place.
		return 0;

	Firm*	  firmPtr;
	Nation* nationPtr = nation_array[nation_recno];

	int i;
	for( i=stop_defined_num ; i>0 ; i-- )
	{
		int firmRecno = stop_array[i-1].firm_recno;

		if( firm_array.is_deleted(firmRecno) )
		{
			del_stop(i, COMMAND_AI);
			return 1;
		}

		//---- AI only knows how to trade from a market to another ------//

		firmPtr = firm_array[firmRecno];

		if( firmPtr->firm_id != FIRM_MARKET ||
			 nationPtr->get_relation(firmPtr->nation_recno)->trade_treaty==0 )		// if the treaty trade has been terminated, delete the stop
		{
			del_stop(i, COMMAND_AI);
			return 1;
		}

		//--- If this market is not linked to any towns ---//

		FirmMarket* firmMarket = (FirmMarket*) firm_array[stop_array[i-1].firm_recno];

		if( !firmMarket->is_market_linked_to_town() )
		{
			//--- and the caravan is not currently picking up goods from the market ---//

			int hasPickUp=0;
			TradeStop* tradeStop = stop_array + i - 1; 

			int j;
			for( j=PICK_UP_RAW_FIRST ; j<=PICK_UP_RAW_LAST ; j++ )
			{
				if( tradeStop->pick_up_array[j] )
					hasPickUp = 1;
			}

			for( j=PICK_UP_PRODUCT_FIRST ; j<=PICK_UP_PRODUCT_LAST ; j++ )
			{
				if( tradeStop->pick_up_array[j] )
					hasPickUp = 1;
			}

			//---- then delete the stop -----//

			if( !hasPickUp )
			{
				del_stop(i, COMMAND_AI);
				return 1;
			}
		}

		//----------------------------------------------//

		int nationRecno = firmMarket->nation_recno;

		if( nationPtr->get_relation_status(nationRecno) == NATION_HOSTILE )
		{
			del_stop(i, COMMAND_AI);
			return 1;
		}
	}

	//----------- debug code ----------//

#ifdef DEBUG
	for( i=stop_defined_num ; i>0 ; i-- )
	{
		err_when( firm_array.is_deleted(stop_array[i-1].firm_recno) );
	}
#endif

	return 0;
}
//------- End of function UnitCaravan::think_del_stop --------//


//------- Begin of function UnitCaravan::think_set_pick_up_type --------//
//
// Think about setting the pick up types of this caravan's stops.
//
void UnitCaravan::think_set_pick_up_type()
{
	if( !is_visible() )		// cannot change pickup type if the caravan is inside a market place.
		return;

	if( stop_defined_num < 2 )
		return;

	//------------------------------------------//

	err_when( firm_array.is_deleted(stop_array[0].firm_recno) );
	err_when( firm_array.is_deleted(stop_array[1].firm_recno) );

	Firm* firmPtr1 = firm_array[stop_array[0].firm_recno];
	Firm* firmPtr2 = firm_array[stop_array[1].firm_recno];

	if( firmPtr1->firm_id != FIRM_MARKET ||		// only when both firms are markets
		 firmPtr2->firm_id != FIRM_MARKET )
	{
		return;
	}

	if( firmPtr2->nation_recno == nation_recno &&		// only when the market is our own, we can use it as a TO market
		 ((FirmMarket*)firmPtr2)->is_retail_market() )
	{
		think_set_pick_up_type2( 1, 2 );
	}

	if( firmPtr1->nation_recno == nation_recno &&
		 ((FirmMarket*)firmPtr1)->is_retail_market() )
	{
		think_set_pick_up_type2( 2, 1 );
	}
}
//------- End of function UnitCaravan::think_set_pick_up_type --------//


//------- Begin of function UnitCaravan::think_set_pick_up_type2 --------//
//
// Think about importing products from one firm to another
//
void UnitCaravan::think_set_pick_up_type2(int fromStopId, int toStopId)
{
	FirmMarket* fromMarket = (FirmMarket*) firm_array[stop_array[fromStopId-1].firm_recno];
	FirmMarket* toMarket   = (FirmMarket*) firm_array[stop_array[toStopId-1].firm_recno];

	//----- AI only knows about market to market trade -----//

	if( fromMarket->firm_id != FIRM_MARKET || toMarket->firm_id != FIRM_MARKET )
		return;

	//---- think about adding new pick up types -----//

	MarketGoods* marketGoods = fromMarket->market_goods_array;
	TradeStop* tradeStop = stop_array+fromStopId-1;

	int i;
	for( i=0 ; i<MAX_MARKET_GOODS ; i++, marketGoods++ )
	{
		if( !marketGoods->product_raw_id )
			continue;

		//----- only if this market has direct supplies -----//

		if( marketGoods->supply_30days()==0 )
			continue;

		//-- when the from market has the product and the to market does not have the product, then trade this good --//

		int pickUpType = PICK_UP_PRODUCT_FIRST+marketGoods->product_raw_id-1;

		//------ toggle it if the current flag and the flag we need are different ----//

		if( !tradeStop->pick_up_array[pickUpType-1] )
			set_stop_pick_up(fromStopId, pickUpType, COMMAND_AI);
	}

	//---- think about droping existing pick up types -----//

	for( i=PICK_UP_RAW_FIRST ; i<=PICK_UP_RAW_LAST ; i++ )
	{
		if( !tradeStop->pick_up_array[i-1] )
			continue;

		marketGoods = fromMarket->market_raw_array[i-PICK_UP_RAW_FIRST];

		//----- if there is no supply, drop the pick up type -----//

		if( !marketGoods || marketGoods->supply_30days() == 0 )
			set_stop_pick_up(fromStopId, i, COMMAND_AI);
	}

	for( i=PICK_UP_PRODUCT_FIRST ; i<=PICK_UP_PRODUCT_LAST ; i++ )
	{
		if( !tradeStop->pick_up_array[i-1] )
			continue;

		marketGoods = fromMarket->market_product_array[i-PICK_UP_PRODUCT_FIRST];

		//--- if the supply is not enough, drop the pick up type ---//

		if( !marketGoods || marketGoods->supply_30days()==0 )
			set_stop_pick_up(fromStopId, i, COMMAND_AI);
	}
}
//------- End of function UnitCaravan::think_set_pick_up_type2 --------//

