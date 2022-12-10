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

//Filename    : OF_MARK2.CPP
//Description : Firm Market Place - AI functions

#include <OINFO.h>
#include <ORAWRES.h>
#include <ORACERES.h>
#include <OTOWN.h>
#include <OGAME.h>
#include <ONATION.h>
#include <OU_CARA.h>
#include <OWORLD.h>
#include <OSYS.h>
#include <OF_FACT.h>
#include <OF_MINE.h>
#include <OF_MARK.h>


//------- Begin of function FirmMarket::process_ai -----------//
//
void FirmMarket::process_ai()
{
	//---- think about deleting this firm ----//

	if( info.game_date%30==firm_recno%30 )
	{
		if( think_del() )
			return;
	}

	//----- think about demand trade treaty -----//

	if( info.game_date%30==firm_recno%30 )
		think_demand_trade_treaty();

	//----- think about building a factory next to this market ----//

	if( info.game_date%60==firm_recno%60 )
		think_market_build_factory();

	//-------- think about new trading routes --------//

	if( info.game_date < last_import_new_goods_date+60 )		// don't new imports until it's 60 days after the last one was imported
		return;

	if( can_hire_caravan() )
	{
		Nation* ownNation = nation_array[nation_recno];

		int thinkInterval = 10 + (100-ownNation->pref_trading_tendency)/5;		// think once every 10 to 30 days

		if( is_market_linked_to_town() )
		{
			if( info.game_date%thinkInterval==firm_recno%thinkInterval )
				think_import_new_product();

			if( info.game_date%60 == (firm_recno+20)%60 )
			{
				//------------------------------------------------------//
				// Don't think about increaseing existing product supply
				// if we have just import a new goods, it takes time
				// to transport and pile up goods.
				//------------------------------------------------------//

				if( !last_import_new_goods_date ||
					 info.game_date > last_import_new_goods_date+180 )	// only think increase existing supply 180 days after importing a new one
				{
					think_increase_existing_product_supply();
				}
			}
		}

		if( info.game_date%thinkInterval == firm_recno%thinkInterval )
			think_export_product();
	}
}
//--------- End of function FirmMarket::process_ai -----------//


//------- Begin of function FirmMarket::think_market_build_factory ------//
//
void FirmMarket::think_market_build_factory()
{
	if( no_neighbor_space )		// if there is no space in the neighbor area for building a new firm.
		return;

	//---- think about building factories to manufacture goods using raw materials in the market place ---//

	MarketGoods* marketGoods = market_goods_array;
	Firm* firmPtr;

	ai_should_build_factory_count = 2;		// always set it to 2, so think_build_factory() will start to build a market as soon as there is a need

	for( int i=0 ; i<MAX_MARKET_GOODS ; i++, marketGoods++ )
	{
		if( !marketGoods->raw_id )
			continue;

		if( marketGoods->stock_qty < 250 )		// only when the stock is >= 250
			continue;

		//----- check if the raw materials are from a local mine, if so don't build a factory, we only build a factory to manufacture goods using raw materials from a remote town.

		int j;
		for( j=0 ; j<linked_firm_count ; j++ )
		{
			firmPtr = firm_array[ linked_firm_array[j] ];

			if( firmPtr->firm_id == FIRM_MINE &&
				 firmPtr->nation_recno == nation_recno &&
				 ((FirmMine*)firmPtr)->raw_id == marketGoods->raw_id )
			{
				break;
			}
		}

		if( j<linked_firm_count )		// if this raw material is from a local mine
			continue;

		//-------------------------------------------//

		if( think_build_factory(marketGoods->raw_id) )
			return;
	}
}
//-------- End of function FirmMarket::think_market_build_factory ------//


//------- Begin of function FirmMarket::is_market_linked_to_town --------//
//
// Return whether this market is linked (enabled link) to any town.
//
// [int] ownBaseTownOnly - whether only count own base town or not.
//									(default:0)
//
int FirmMarket::is_market_linked_to_town(int ownBaseTownOnly)
{
	Town* townPtr;

	for(int i=0 ; i<linked_town_count ; i++ )
	{
		if( linked_town_enable_array[i] != LINK_EE )
			continue;

		if( ownBaseTownOnly )
		{
			townPtr = town_array[ linked_town_array[i] ];

			if( townPtr->nation_recno == nation_recno &&
				 townPtr->is_base_town )
			{
				return 1;
			}
		}
		else
		{
			return 1;
		}
	}

	return 0;
}
//--------- End of function FirmMarket::is_market_linked_to_town --------//


//------ Begin of function FirmMarket::ai_update_link_status ------//
//
void FirmMarket::ai_update_link_status()
{
	//---- make sure the restocking type is defined ----//

	if( restock_type == RESTOCK_ANY )
	{
		MarketGoods* marketGoods = market_goods_array;

		int i;
		for( i=MAX_MARKET_GOODS ; i>0 ; i--, marketGoods++ )
		{
			if( marketGoods->raw_id )
				break;
		}
		if( i>0 )
			restock_type = RESTOCK_RAW;
		else
			restock_type = RESTOCK_PRODUCT;
	}

	//---- consider enabling/disabling links to firms ----//

	Nation* nationPtr = nation_array[nation_recno];
	Firm*   firmPtr;
	int 	  rc;

	int i;
	for(i=0; i<linked_firm_count; i++)
	{
		err_when(!linked_firm_array[i] || firm_array.is_deleted(linked_firm_array[i]));

		firmPtr = firm_array[linked_firm_array[i]];

		//-------- check product type ----------//

		if( is_retail_market() )
			rc = firmPtr->firm_id == FIRM_FACTORY;
		else
			rc = firmPtr->firm_id == FIRM_MINE || firmPtr->firm_id == FIRM_FACTORY;		// for output raw materials to the factory to manufacture

		toggle_firm_link( i+1, rc, COMMAND_AI );		// enable the link
	}

	//----- always enable links to towns as there is no downsides for selling goods to the villagers ----//

	for(i=0; i<linked_town_count; i++)
	{
		err_when(!linked_town_array[i] || town_array.is_deleted(linked_town_array[i]));

		toggle_town_link( i+1, 1, COMMAND_AI );		// enable the link
	}
}
//------- End of function FirmMarket::ai_update_link_status --------//


//------- Begin of function FirmMarket::think_del -----------//
//
// Think about deleting this firm.
//
int FirmMarket::think_del()
{
	if( linked_town_count > 0 )
	{
		no_linked_town_since_date = 0;		// reset it
		return 0;
	}
	else
	{
		no_linked_town_since_date = info.game_date;
	}

	//---- don't delete it if there are still signiciant stockhere ---//

	if( stock_value_index() >= 10 )
	{
		Nation* ownNation = nation_array[nation_recno];

		//--- if the market has been sitting idle for too long, delete it ---//

		if( info.game_date < no_linked_town_since_date
			 + 180 + 180 * ownNation->pref_trading_tendency / 100 )
		{
			return 0;
		}
	}

	//------------------------------------------------//

	ai_del_firm();

	return 1;
}
//--------- End of function FirmMarket::think_del -----------//


//------- Begin of function FirmMarket::think_import_new_product ------//
//
// Think about importing goods to sell in this market place.
//
int FirmMarket::think_import_new_product()
{
	//---- check if the market place has free space for new supply ----//

	int			i, j, emptySlot=0;
	MarketGoods *marketGoods = market_goods_array;

	for( i=0 ; i<MAX_MARKET_GOODS ; i++, marketGoods++ )
	{
		if( !marketGoods->product_raw_id && !marketGoods->raw_id )
			emptySlot++;
	}

	if( emptySlot==0 )
		return 0;

	//--- update what products are needed for this market place ---//

	Town* townPtr;
	short needProductSupplyPop[MAX_PRODUCT];			// the total population in the towns linked to the market that needs the supply of the product
	Nation* nationPtr = nation_array[nation_recno];

	memset( needProductSupplyPop, 0, sizeof(needProductSupplyPop) );

	for( i=0; i<linked_town_count; i++ )
	{
		err_when(!linked_town_array[i] || town_array.is_deleted(linked_town_array[i]));

		if( linked_town_enable_array[i] != LINK_EE )
			continue;

		townPtr = town_array[linked_town_array[i]];

		if( townPtr->region_id != region_id )
			continue;

		if( !townPtr->is_base_town )		// don't import if it isn't a base town
			continue;

		//------------------------------------------------//
		//
		// Only if the population of the town is equal or
		// larger than minTradePop, the AI will try to do trade.
		// The minTradePop is between 10 to 20 depending on the
		// pref_trading_tendency.
		//
		//------------------------------------------------//

		townPtr->update_product_supply();

		for( j=0 ; j<MAX_PRODUCT ; j++ )
		{
			if( !townPtr->has_product_supply[j] )
				needProductSupplyPop[j] += townPtr->population;
		}
	}

	//---- think about importing the products that need supply ----//

	int minTradePop = 10;

	if( is_retail_market() )
	{
		for( int productId=1 ; productId<=MAX_PRODUCT ; productId++ )
		{
			if( needProductSupplyPop[productId-1] >= minTradePop || emptySlot==MAX_MARKET_GOODS )		// if  market is empty, try to import some goods
			{
				if( think_import_specific_product(productId) )
				{
					last_import_new_goods_date = info.game_date;
					return 1;
				}
			}
		}
	}

	//----------------------------------------------------------//
	// Think about importing the raw materials of the needed
	// products and build factories to manufacture them ourselves
	//----------------------------------------------------------//

	//--- first check if we can build a new factory to manufacture the products ---//

	if( is_raw_market() && is_market_linked_to_town(1) )		// 1-only count towns that are our own and are base towns
	{
		if( !no_neighbor_space &&
			 nationPtr->total_jobless_population >= MAX_WORKER*2 &&
			 can_hire_caravan() >= 2 )		// if there is a shortage of caravan supplies, use it for transporting finished products instead of raw materials
		{
			if( nationPtr->can_ai_build(FIRM_FACTORY) )
			{
				for( int productId=1 ; productId<=MAX_PRODUCT ; productId++ )
				{
					if( needProductSupplyPop[productId-1] >= minTradePop )
					{
						if( think_mft_specific_product(productId) )
						{
							last_import_new_goods_date = info.game_date;
							return 1;
						}
					}
				}
			}
		}
	}

	return 0;
}
//--------- End of function FirmMarket::think_import_new_product -------//


//--- Begin of function FirmMarket::think_increase_existing_product_supply ---//
//
// Think about increasing the supply of existing products.
//
int FirmMarket::think_increase_existing_product_supply()
{
	MarketGoods *marketGoods = market_goods_array;

	for( int i=0 ; i<MAX_MARKET_GOODS ; i++, marketGoods++ )
	{
		if( marketGoods->product_raw_id )
		{
			if( marketGoods->stock_qty < MAX_MARKET_STOCK/10 &&
				 marketGoods->month_demand * 0.8 > marketGoods->supply_30days() )		// the supply falls behind the demand by at least 20%
			{
				if( think_import_specific_product(marketGoods->product_raw_id) )
					return 1;
			}
		}
	}

	return 0;
}
//---- End of function FirmMarket::think_increase_existing_product_supply ---//


//--- Begin of function FirmMarket::think_import_specific_product ---//
//
// Think about importing a specific product.
//
int FirmMarket::think_import_specific_product(int productId)
{
	int		i, firmRecno;
	Firm*	   firmPtr, *bestFirmPtr=NULL;
	Nation* 	nationPtr = nation_array[nation_recno];
	int		stockLevel, curRating, bestRating=0;
	int		canHireCaravan = can_hire_caravan();

	RawInfo* rawInfo = raw_res[productId];

	for( i=rawInfo->product_supply_firm_array.size() ; i>0 ; i-- )
	{
		firmRecno = rawInfo->get_product_supply_firm(i);

		if( firm_array.is_deleted(firmRecno) || firmRecno==firm_recno )
			continue;

		//-- if there is already a caravan travelling between two points --//

		firmPtr = firm_array[firmRecno];

		if( firmPtr->region_id != region_id )
			continue;

		//-----------------------------------------//
		// The rating of a supply is determined by:
		//	- distance
		// - supply
		// - nation relationship
		//-----------------------------------------//

		//------ determine the stock level of this supply ------//

		stockLevel = 0;

		//---- think about inputing goods from this factory ----//

		if( firmPtr->firm_id == FIRM_FACTORY )
		{
			if( firmPtr->nation_recno != nation_recno )	// can import goods from own factories only
				continue;

			FirmFactory* firmFactory = (FirmFactory*) firmPtr;

			if( firmFactory->product_raw_id == productId )
				stockLevel = 100 * (int) firmFactory->stock_qty / (int) firmFactory->max_stock_qty;
		}

		//---- think about inputing goods from this market ----//

		else if( firmPtr->firm_id == FIRM_MARKET )
		{
			//--- if this is a foreign sale market, don't import from it (e.g. Nation A's market built near Nation B's village -----//

			if( firmPtr->nation_recno != nation_recno )		// if this is not our market
			{
				int j;
				for( j=firmPtr->linked_town_count-1 ; j>=0 ; j-- )
				{
					Town* townPtr = town_array[ firmPtr->linked_town_array[j] ];

					if( townPtr->nation_recno == firmPtr->nation_recno )
						break;
				}

				if( j<0 )		// if this market is not linked to its own town (then it must be a foreign market)
					continue;
			}

			//-- only either from own market place or from nations that trade with you --//

			if( nation_array[firmPtr->nation_recno]->get_relation(nation_recno)->trade_treaty == 0 )
				continue;

			MarketGoods* marketGoods = ((FirmMarket*)firmPtr)->market_product_array[productId-1];

			//--- if this market has the supply of this goods ----//

			if( marketGoods && marketGoods->supply_30days() > 0 )
			{
				stockLevel = 100 * (int) marketGoods->stock_qty / MAX_MARKET_STOCK;
			}
		}

		//----------------------------------------------//

		if( firmPtr->nation_recno == nation_recno )
		{
			if( stockLevel < 10 )		// for our own market, the stock level requirement is lower
				continue;

			curRating = 50;
		}
		else
		{
			if( stockLevel < 20 )		// for other player's market, only import when the stock level is high enough
				continue;

			curRating = nationPtr->get_relation_status(firmPtr->nation_recno) * 5;
		}

		//---- calculate the current overall rating ----//

		curRating += stockLevel/2 + world.distance_rating(center_x, center_y, firmPtr->center_x, firmPtr->center_y);

		//----------- compare ratings -------------//

		if( curRating > bestRating )
		{
			bestRating 	= curRating;
			bestFirmPtr = firmPtr;
		}
	}

	//---- if a suitable supplier is found -----//

	if( bestFirmPtr )
		return ai_create_new_trade(bestFirmPtr, 0, PICK_UP_PRODUCT_FIRST+productId-1);

	return 0;
}
//---- End of function FirmMarket::think_import_specific_product ----//


//------- Begin of function FirmMarket::think_export_product -----------//
//
// Think about exporting products from this market to another market.
//
int FirmMarket::think_export_product()
{
	//--- first check if there is any excessive supply for export ---//

	int			exportProductId = 0;
	MarketGoods *marketGoods = market_goods_array;

	for( int i=0 ; i<MAX_MARKET_GOODS ; i++, marketGoods++ )
	{
		if( marketGoods->product_raw_id )
		{
			if( marketGoods->stock_qty > MAX_MARKET_STOCK * 3 / 4 &&
				 marketGoods->month_demand < marketGoods->supply_30days() / 2 )		// the supply is at least double of the demand
			{
				exportProductId = marketGoods->product_raw_id;
				break;
			}
		}
	}

	if( !exportProductId )
		return 0;

	//----- locate for towns that do not have the supply of the product ----//

	Town*   townPtr;
	Nation* nationPtr = nation_array[nation_recno];

	for( int townRecno=town_array.size() ; townRecno>0 ; townRecno-- )
	{
		if( town_array.is_deleted(townRecno) )
			continue;

		townPtr = town_array[townRecno];

		if( townPtr->population < 20 - (10*nationPtr->pref_trading_tendency/100) )	// 10 to 20 as the minimum population for considering trade
			continue;

		if( townPtr->has_product_supply[exportProductId-1] )		// if the town already has the supply of product, return now
			continue;

		if( townPtr->region_id != region_id )
			continue;

		if( townPtr->no_neighbor_space )		// if there is no space in the neighbor area for building a new firm.
			continue;

		if( misc.points_distance( center_x, center_y, townPtr->center_x, center_y ) > MAX_WORLD_X_LOC/4 )		// don't consider if it is too far away
			continue;

      //-----------------------------------------//

		if( townPtr->nation_recno )
		{

			if( nationPtr->get_relation_status(townPtr->nation_recno) < NATION_FRIENDLY )		// only build markets to friendly nation's town
			{
				continue;
			}
			//--- if it's a nation town, only export if we have trade treaty with it ---//

			if( !nationPtr->get_relation(townPtr->nation_recno)->trade_treaty )
				continue;
		}
		else
		{
			//--- if it's an independent town, only export if the resistance is low ---//

			if( townPtr->average_resistance(nation_recno) > INDEPENDENT_LINK_RESISTANCE )
				continue;
		}

		//----- think about building a new market to the town for exporting our goods -----//

		if( think_build_export_market(townRecno) )
			return 1;
	}

	return 0;
}
//--------- End of function FirmMarket::think_export_product -----------//


//------- Begin of function FirmMarket::think_build_export_market -----------//
//
// Think about export goods of this market to other markets.
//
int FirmMarket::think_build_export_market(int townRecno)
{
	Town* 		townPtr = town_array[townRecno];
	Firm* 		firmPtr;
	Nation* 		nationPtr = nation_array[nation_recno];

	//---- see if we already have a market linked to this town ----//

	for( int i=0 ; i<townPtr->linked_firm_count ; i++ )
	{
		firmPtr = firm_array[ townPtr->linked_firm_array[i] ];

		if( firmPtr->firm_id != FIRM_MARKET || firmPtr->firm_recno==firm_recno )
			continue;

		//--- if we already have a market there, no need to build a new market ----//

		if( firmPtr->nation_recno == nation_recno )
			return 0;
	}

	//--- if there is no market place linked to this town, we can set up one ourselves ---//

	short buildXLoc, buildYLoc;

	if( !nationPtr->find_best_firm_loc(FIRM_MARKET, townPtr->loc_x1, townPtr->loc_y1, buildXLoc, buildYLoc) )
	{
		townPtr->no_neighbor_space = 1;
		return 0;
	}

	nationPtr->add_action(buildXLoc, buildYLoc, townPtr->loc_x1, townPtr->loc_y1, ACTION_AI_BUILD_FIRM, FIRM_MARKET);
	return 1;
}
//--------- End of function FirmMarket::think_build_export_market -----------//


//--- Begin of function FirmMarket::think_mft_specific_product ---//
//
// Think about importing a specific type of raw material and build a
// factory to manufacture ourselves.
//
// <int> rawId	    - id. of the raw material.
//
int FirmMarket::think_mft_specific_product(int rawId)
{
	int		i, j, firmRecno;
	Firm*	   firmPtr, *bestFirmPtr=NULL;
	Nation* 	nationPtr = nation_array[nation_recno];
	int		stockLevel, curRating, bestRating=0;

	RawInfo* rawInfo = raw_res[rawId];

	for( i=rawInfo->raw_supply_firm_array.size() ; i>0 ; i-- )
	{
		firmRecno = rawInfo->get_raw_supply_firm(i);

		if( firm_array.is_deleted(firmRecno) || firmRecno == firm_recno )
			continue;

		//-- if there is already a caravan travelling between two points --//

		firmPtr = firm_array[firmRecno];

		if( firmPtr->region_id != region_id )
			continue;

		//-- if this is our own supply, don't import the raw material, but import the finished goods instead. --//

		if( firmPtr->nation_recno == nation_recno )
			continue;

		//-----------------------------------------//
		// The rating of a supply is determined by:
		//	- distance
		// - supply
		// - nation relationship
		//-----------------------------------------//

		//------ determine the stock level of this supply ------//

		stockLevel = 0;

		if( firmPtr->firm_id == FIRM_MARKET )
		{
			//-- only either from own market place or from nations that trade with you --//

			if( nation_array[firmPtr->nation_recno]->get_relation(nation_recno)->trade_treaty == 0 )
				continue;

			//----- check if this market is linked to any mines directly ----//

			for( j=firmPtr->linked_firm_count-1 ; j>=0 ; j-- )
			{
				Firm* linkedFirm = firm_array[ firmPtr->linked_firm_array[j] ];

				if( linkedFirm->firm_id == FIRM_MINE && linkedFirm->nation_recno == firmPtr->nation_recno )
				{
					if( ((FirmMine*)linkedFirm)->raw_id == rawId )
						break;
				}
			}
				
			if( j<0 )			// this market does not have any direct supplies, so don't pick up goods from it 
				continue;

			//---------------------------------------------------------------//

			MarketGoods* marketGoods = ((FirmMarket*)firmPtr)->market_goods_array;

			for( j=0 ; j<MAX_MARKET_GOODS ; j++, marketGoods++ )
			{
				if( marketGoods->stock_qty > MAX_MARKET_STOCK / 5 )
				{
					if( marketGoods->raw_id == rawId )
						stockLevel = 100 * (int) marketGoods->stock_qty / MAX_MARKET_STOCK;
				}
			}
		}

		if( stockLevel < 50 )		// if the stock is too low, don't consider it
			continue;

		//---- calculate the current overall rating ----//

		NationRelation* nationRelation = nationPtr->get_relation(firmPtr->nation_recno);

		curRating  = stockLevel
						 - 100 * misc.points_distance( center_x, center_y,
							firmPtr->center_x, firmPtr->center_y ) / MAX_WORLD_X_LOC;

		if( firmPtr->nation_recno == nation_recno )
			curRating += 100;
		else
			curRating += nationRelation->status * 20;

		//----------- compare ratings -------------//

		if( curRating > bestRating )
		{
			bestRating 	= curRating;
			bestFirmPtr = firmPtr;
		}
	}

	if( !bestFirmPtr )
		return 0;

	if( !ai_create_new_trade(bestFirmPtr, 0, PICK_UP_RAW_FIRST+rawId-1) )
		return 0;

	return 1;
}
//---- End of function FirmMarket::think_mft_specific_product ----//


//------- Begin of function FirmMarket::think_demand_trade_treaty ------//
//
void FirmMarket::think_demand_trade_treaty()
{
	Nation* nationPtr = nation_array[nation_recno];
	int 	  nationRecno;

	//----- demand towns to open up market ----//

	for( int i=0 ; i<linked_town_count ; i++ )
	{
		//----- if the link is not enabled -----//

		if( linked_town_enable_array[i] != LINK_EE )
		{
			nationRecno = town_array[ linked_town_array[i] ]->nation_recno;

			if( nationRecno )
				nationPtr->get_relation(nationRecno)->ai_demand_trade_treaty++;
		}
	}
}
//-------- End of function FirmMarket::think_demand_trade_treaty -------//


//------- Begin of function FirmMarket::ai_create_new_trade ------//
//
int FirmMarket::ai_create_new_trade(Firm* firmPtr, int stop1PickUpType, int stop2PickUpType)
{
	//---- see if there is already a caravan moving along the route -----//

	Nation* 		 ownNation = nation_array[nation_recno];
	UnitCaravan* unitCaravan;
	int			 rc, stop1Id, stop2Id;
	int			 caravanInRouteCount=0;

	for( int i=ownNation->ai_caravan_count-1 ; i>=0 ; i-- )
	{
		unitCaravan = (UnitCaravan*) unit_array[ ownNation->ai_caravan_array[i] ];

		err_when( unitCaravan->nation_recno != nation_recno );
		err_when( unitCaravan->unit_id != UNIT_CARAVAN );

		if( unitCaravan->stop_defined_num < 2 )
			continue;

		if( unitCaravan->stop_array[0].firm_recno == firm_recno &&
			 unitCaravan->stop_array[1].firm_recno == firmPtr->firm_recno )
		{
			stop1Id = 1;
			stop2Id = 2;
		}
		else if( unitCaravan->stop_array[1].firm_recno == firm_recno &&
					unitCaravan->stop_array[0].firm_recno == firmPtr->firm_recno )
		{
			stop1Id = 2;
			stop2Id = 1;
		}
		else
		{
			continue;
		}

		//------- add the goods to the pick up list ----//

		rc = 0;

		if( stop1PickUpType && !unitCaravan->has_pick_up_type(stop1Id, stop1PickUpType) )
		{
			if( unitCaravan->is_visible() )		// can't set stop when the caravan is in a firm
				unitCaravan->set_stop_pick_up(stop1Id, stop1PickUpType, COMMAND_AI);
			rc = 1;
		}

		if( stop2PickUpType && !unitCaravan->has_pick_up_type(stop2Id, stop2PickUpType) )
		{
			if( unitCaravan->is_visible() )		// can't set stop when the caravan is in a firm
				unitCaravan->set_stop_pick_up(stop2Id, stop2PickUpType, COMMAND_AI);
			rc = 1;
		}

		if( rc )			// don't add one if we can utilize an existing one.
			return 1;

		caravanInRouteCount++;
	}

	if( caravanInRouteCount >= 2 )		// don't have more than 2 caravans on a single route
		return 0;

	//----------- hire a new caravan -----------//

	int unitRecno = hire_caravan(COMMAND_AI);

	if( !unitRecno )
		return 0;

	//----------- set up the trade route ----------//

	unitCaravan = (UnitCaravan*) unit_array[unitRecno];

	unitCaravan->set_stop(2, firmPtr->loc_x1, firmPtr->loc_y1, COMMAND_AI);

	err_when( unitCaravan->stop_array[0].firm_recno == firmPtr->firm_recno );		// cannot set both stops to the same firm

	unitCaravan->set_stop_pick_up(1, NO_PICK_UP, COMMAND_AI);
	unitCaravan->set_stop_pick_up(2, NO_PICK_UP, COMMAND_AI);

	if( stop1PickUpType )
		unitCaravan->set_stop_pick_up(1, stop1PickUpType, COMMAND_AI);

	if( stop2PickUpType )
		unitCaravan->set_stop_pick_up(2, stop2PickUpType, COMMAND_AI);

	return 1;
}
//-------- End of function FirmMarket::ai_create_new_trade -------//
