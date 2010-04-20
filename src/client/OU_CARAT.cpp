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

//Filename    : OU_CARA3.CPP
//Description : Unit Caravan unload/load functions

#include <ONATION.h>
#include <OF_MINE.h>
#include <OF_FACT.h>
#include <OF_MARK.h>
#include <OU_CARA.h>
#ifdef USE_DPLAY
#include <OREMOTE.h>
#endif

static char		 processed_raw_qty_array[MAX_RAW];					// 1 for not unload but can up load, 2 for unload but not up load
static char		 processed_product_raw_qty_array[MAX_PRODUCT];	// ditto

//--------- Begin of function UnitCaravan::market_unload_goods ---------//
//
void UnitCaravan::market_unload_goods()
{
	FirmMarket *curMarket = (FirmMarket*) firm_array[stop_array[dest_stop_id-1].firm_recno];
	err_when(curMarket->firm_id != FIRM_MARKET);

	memset(processed_raw_qty_array, 0, sizeof(char)*MAX_RAW);
	memset(processed_product_raw_qty_array, 0, sizeof(char)*MAX_PRODUCT);

	//--------------------------------------------------------------------//
	// only unload goods to our market
	//--------------------------------------------------------------------//
	if(curMarket->nation_recno!=nation_recno)
		return;

	//--------------------------------------------------------------------//
	// unload goods
	//-------------------------------------------------//
	MarketGoods *marketGoods = curMarket->market_goods_array;
	short	unloadQty;
	int	goodsId, withEmptySlot=0;

	int i;
	for(i=0; i<MAX_MARKET_GOODS; i++, marketGoods++)
	{
		if(marketGoods->raw_id)
		{
			err_when(marketGoods->product_raw_id);
			//-------------- is raw material ----------------//
			goodsId = marketGoods->raw_id-1;

			//if( (marketGoods->supply_30days()==0 && marketGoods->stock_qty<curMarket->max_stock_qty) || // no supply and stock isn't full
			//	 (marketGoods->stock_qty<CARAVAN_UNLOAD_TO_MARKET_QTY &&
			//	 //##### begin trevor 16/7 #######//
			//	  marketGoods->month_demand > marketGoods->supply_30days()) ) // demand > supply
			//	 //##### end trevor 16/7 #######//
			if(marketGoods->stock_qty<curMarket->max_stock_qty)
			{
				//-------- demand > supply and stock is not full ----------//
				if(raw_qty_array[goodsId]) // have this goods
				{
					//---------- process unload -------------//
					unloadQty = (short) MIN(raw_qty_array[goodsId], curMarket->max_stock_qty-marketGoods->stock_qty);
					raw_qty_array[goodsId]		  -= unloadQty;
					err_when(raw_qty_array[goodsId]<0);
					marketGoods->stock_qty += unloadQty;
					processed_raw_qty_array[goodsId] += 2;
				}
				else if(!marketGoods->stock_qty && !marketGoods->supply_30days())
				{
					//---------- no supply, no stock, without this goods ------------//
					withEmptySlot++;
					//processed_raw_qty_array[goodsId] = 0; // reset to zero for handling empty slot
				}
			}
			else if(raw_qty_array[goodsId]) // have this goods
			{
				processed_raw_qty_array[goodsId]++;
			}
		}
		else if(marketGoods->product_raw_id)
		{
			err_when(marketGoods->raw_id);
			//---------------- is product -------------------//
			goodsId = marketGoods->product_raw_id-1;

			//if( (marketGoods->supply_30days()==0 && marketGoods->stock_qty<curMarket->max_stock_qty) || // no supply and stock isn't full
			//	 //##### begin trevor 16/7 #######//
			//	 (marketGoods->stock_qty<50 && marketGoods->month_demand > marketGoods->supply_30days()) ) // demand > supply
			//	 //##### end trevor 16/7 #######//
			if(marketGoods->stock_qty<curMarket->max_stock_qty)
			{
				if(product_raw_qty_array[goodsId]) // have this goods
				{
					unloadQty = (short) MIN(product_raw_qty_array[goodsId], curMarket->max_stock_qty-marketGoods->stock_qty);
					product_raw_qty_array[goodsId]	-= unloadQty;
					err_when(product_raw_qty_array[goodsId]<0);
					marketGoods->stock_qty	+= unloadQty;
					processed_product_raw_qty_array[goodsId] += 2;
				}
				else if(!marketGoods->stock_qty && !marketGoods->supply_30days()) // no supply, no stock, without this goods
				{
					withEmptySlot++;
					//processed_product_raw_qty_array[goodsId] = 0; // reset to zero for handling empty slot
				}
			}
			else if(product_raw_qty_array[goodsId]) // have this goods
			{
				processed_product_raw_qty_array[goodsId]++;
			}
		}
		else	// is empty
		{
			if(!market_unload_goods_in_empty_slot(curMarket, i))
				break; // no goods for further checking
		}
	}

	//-------------------------------------------------//
	// unload new goods in the empty slots
	//-------------------------------------------------//
	if(withEmptySlot)
	{
		marketGoods = curMarket->market_goods_array;
		for(i=0; i<MAX_MARKET_GOODS && withEmptySlot; i++, marketGoods++)
		{
			if(marketGoods->stock_qty || marketGoods->supply_30days())
				continue;

			market_unload_goods_in_empty_slot(curMarket, i);
			withEmptySlot--;
		}
	}

	err_when(withEmptySlot);
}
//----------- End of function UnitCaravan::market_unload_goods -----------//


//--------- Begin of function UnitCaravan::market_unload_goods_in_empty_slot ---------//
// return 0 if no goods for further checking
// return 1 if unload goods successfully
//
int UnitCaravan::market_unload_goods_in_empty_slot(FirmMarket *curMarket, int position)
{
	MarketGoods* marketGoods = curMarket->market_goods_array + position;
	MarketGoods *checkGoods;
	int	productExistInOtherSlot, rawExistInOtherSlot;

	//-------------------------------------------------//
	// unload product and then raw
	//-------------------------------------------------//
	int processed, j;
	for(processed=0, j=0; j<MAX_PRODUCT; j++)
	{
		if(processed_product_raw_qty_array[j] || !product_raw_qty_array[j])
			continue; // this product is processed or no stock in the caravan

		checkGoods = curMarket->market_goods_array;
		productExistInOtherSlot = 0;
		for(int k=0; k<MAX_MARKET_GOODS; k++, checkGoods++)
		{
			if(checkGoods->product_raw_id==j+1)
			{
				productExistInOtherSlot++;
				break;
			}
		}

		if(productExistInOtherSlot)
			continue;

		#ifdef DEBUG
			MarketGoods *debugGoods = curMarket->market_goods_array;
			for(int debugCount=0; debugCount<MAX_MARKET_GOODS; ++debugCount, debugGoods++)
				err_when(debugGoods->product_raw_id==j+1);
		#endif

		//-**************************************************-//
		//err_when(marketGoods->stock_qty);
		marketGoods->stock_qty = (float) 0; // BUGHERE, there is a case that marketGoods->stock_qty > 0
		//-**************************************************-//
		processed_product_raw_qty_array[j] += 2;
		curMarket->set_goods(0, j+1, position);

		short unloadQty = (short) MIN(product_raw_qty_array[j], curMarket->max_stock_qty-marketGoods->stock_qty);
		product_raw_qty_array[j] -= unloadQty;
		marketGoods->stock_qty	 += unloadQty;
		processed++;
		break;
	}

	if(!processed)
	{
		for(j=0; j<MAX_PRODUCT; j++)
		{
			if(processed_raw_qty_array[j] || !raw_qty_array[j])
				continue; // this product is processed or no stock in the caravan

			checkGoods = curMarket->market_goods_array;
			rawExistInOtherSlot = 0;
			for(int k=0; k<MAX_MARKET_GOODS; k++, checkGoods++)
			{
				if(checkGoods->raw_id==j+1)
				{
					rawExistInOtherSlot++;
					break;
				}
			}

			if(rawExistInOtherSlot)
				continue;

			#ifdef DEBUG
				MarketGoods *debugGoods = curMarket->market_goods_array;
				for(int debugCount=0; debugCount<MAX_MARKET_GOODS; ++debugCount, debugGoods++)
					err_when(debugGoods->raw_id==j+1);
			#endif

			//-**************************************************-//
			//err_when(marketGoods->stock_qty);
			marketGoods->stock_qty = (float) 0; // BUGHERE, there is a case that marketGoods->stock_qty > 0
			//-**************************************************-//
			processed_raw_qty_array[j] += 2;
			curMarket->set_goods(1, j+1, position);

			short unloadQty = (short) MIN(raw_qty_array[j], curMarket->max_stock_qty-marketGoods->stock_qty);
			raw_qty_array[j]			-= unloadQty;
			marketGoods->stock_qty	+= unloadQty;
			processed++;
			break;
		}

		if(!processed && !productExistInOtherSlot && !rawExistInOtherSlot)
			return 0;	// no goods for further processsing
	}

	if( unit_array.selected_recno == sprite_recno )
		info.disp();

	return 1;
}
//----------- End of function UnitCaravan::market_unload_goods_in_empty_slot -----------//


//--------- Begin of function UnitCaravan::market_load_goods ---------//
//
void UnitCaravan::market_load_goods()
{
	CaravanStop *stopPtr = stop_array+dest_stop_id-1;
	err_when(stopPtr->pick_up_type == NO_PICK_UP);

	FirmMarket	*curMarket = (FirmMarket*) firm_array[ stopPtr->firm_recno ];
	err_when(curMarket->firm_id != FIRM_MARKET);
	MarketGoods	*marketGoods=curMarket->market_goods_array;

	//------------------------------------------------------------//
	// scan the market, see if it has the specified pickup goods
	//------------------------------------------------------------//
	for(int i=0; i<MAX_MARKET_GOODS; i++, marketGoods++)
	{
		if(marketGoods->raw_id)
		{
			if(stopPtr->pick_up_array[marketGoods->raw_id-1])
				market_load_goods_now(marketGoods, marketGoods->stock_qty);
		}
		else if(marketGoods->product_raw_id)
		{
			if(stopPtr->pick_up_array[marketGoods->product_raw_id-1+MAX_RAW])
				market_load_goods_now(marketGoods, marketGoods->stock_qty);
		}
	}
}
//----------- End of function UnitCaravan::market_load_goods -----------//


//--------- Begin of function UnitCaravan::market_auto_load_goods ---------//
//
void UnitCaravan::market_auto_load_goods()
{
	FirmMarket	*curMarket = (FirmMarket*) firm_array[ stop_array[dest_stop_id-1].firm_recno ];
	err_when(curMarket->firm_id != FIRM_MARKET);

	MarketGoods *marketGoods = curMarket->market_goods_array;
	//int	isOurMarket = (curMarket->nation_recno==nation_recno); // is 1 or 0
	int	goodsId;
	short	loadQty;

	//----------------------------------------------------------------------//
	// keep empty stock if the market(AI) is for sale, otherwise use the
	// default value
	//----------------------------------------------------------------------//
	//short minFirmStockQty = (int)curMarket->max_stock_qty/5; // keep at least 20% capacity in the firm if the market is not for sale

	for(int i=0; i<MAX_MARKET_GOODS; i++, marketGoods++)
	{
		if(!marketGoods->stock_qty)
			continue;

		if(marketGoods->raw_id)
		{
			err_when(marketGoods->product_raw_id);
			(goodsId = marketGoods->raw_id)--;
			if(processed_raw_qty_array[goodsId]==2)
				continue;	// continue if it is the goods unloaded

			if(marketGoods->stock_qty > MIN_FIRM_STOCK_QTY)
			{
				loadQty = (short) (marketGoods->stock_qty - MIN_FIRM_STOCK_QTY);
				err_when(loadQty<0);
				market_load_goods_now(marketGoods, (float) loadQty);
			}
		}
		//else if(marketGoods->product_raw_id && isOurMarket) // only load product in our market
		else if(marketGoods->product_raw_id)
		{
			err_when(marketGoods->raw_id);
			(goodsId = marketGoods->product_raw_id)--;
			if(processed_product_raw_qty_array[goodsId]==2)
				continue;	// continue if it is the goods unloaded

			if(marketGoods->stock_qty > MIN_FIRM_STOCK_QTY)
			{
				loadQty = (short) (marketGoods->stock_qty - MIN_FIRM_STOCK_QTY);
				err_when(loadQty<0);
				market_load_goods_now(marketGoods, (float) loadQty);
			}
		}
	}
}
//----------- End of function UnitCaravan::market_auto_load_goods -----------//


//--------- Begin of function UnitCaravan::market_load_goods_now ---------//
//
void UnitCaravan::market_load_goods_now(MarketGoods* marketGoods, float loadQty)
{
	Nation	*nationPtr = nation_array[nation_recno];
	int		marketNationRecno = firm_array[stop_array[dest_stop_id-1].firm_recno]->nation_recno;
	short		qty;
	int		goodsId;

	if(marketGoods->product_raw_id)
	{
		//---------------- is product ------------------//
		err_when(marketGoods->raw_id);
		(goodsId = marketGoods->product_raw_id)--;

		qty = MIN(MAX_CARAVAN_CARRY_QTY-product_raw_qty_array[goodsId], (int)loadQty);
		if(marketNationRecno!=nation_recno) // calculate the qty again if this is not our own market
		{
			qty = (nationPtr->cash>0) ? (short) MIN(nationPtr->cash/PRODUCT_PRICE, qty) : 0;

			if(qty)
				nationPtr->import_goods(IMPORT_PRODUCT, marketNationRecno, (float)qty * PRODUCT_PRICE);
		}

		product_raw_qty_array[goodsId] += qty;
		err_when(product_raw_qty_array[goodsId]<0 || product_raw_qty_array[goodsId]>MAX_CARAVAN_CARRY_QTY);
		marketGoods->stock_qty	-= qty;
	}
	else if(marketGoods->raw_id)
	{
		//---------------- is raw ---------------------//
		err_when(marketGoods->product_raw_id);
		(goodsId = marketGoods->raw_id)--;

		qty = MIN(MAX_CARAVAN_CARRY_QTY-raw_qty_array[goodsId], (int)loadQty);
		if(marketNationRecno!=nation_recno) // calculate the qty again if this is not our own market
		{
			qty = (nationPtr->cash>0) ? (short) MIN(nationPtr->cash/RAW_PRICE, qty) : 0;

			if(qty)
				nationPtr->import_goods(IMPORT_RAW, marketNationRecno, (float)qty * RAW_PRICE);
		}

		raw_qty_array[goodsId]			+= qty;
		err_when(raw_qty_array[goodsId]<0 || raw_qty_array[goodsId]>MAX_CARAVAN_CARRY_QTY);
		marketGoods->stock_qty	-= qty;
	}

	//### begin trevor 7/8 ###//

	if( qty > 0 )
		last_load_goods_date = info.game_date;

	//#### end trevor 7/8 ####//
}
//----------- End of function UnitCaravan::market_load_goods_now -----------//


//--------- Begin of function UnitCaravan::mine_load_goods ---------//
void UnitCaravan::mine_load_goods(char pickUpType)
{
	if(pickUpType == NO_PICK_UP)
		return; // return if not allowed to load any goods

	err_when(pickUpType>=PICK_UP_PRODUCT_FIRST && pickUpType<=PICK_UP_PRODUCT_LAST);
	CaravanStop *stopPtr = stop_array+dest_stop_id-1;
	FirmMine		*curMine = (FirmMine*) firm_array[stopPtr->firm_recno];
	err_when(curMine->firm_id != FIRM_MINE);

	if(curMine->nation_recno!=nation_recno)
		return; // no action if this is not our own mine

	//------------- load goods -----------//
	int searchRawId = pickUpType-PICK_UP_RAW_FIRST+1;
	if(pickUpType==AUTO_PICK_UP || curMine->raw_id==searchRawId) // auto_pick_up or is the raw to pick up
	{
		int		goodsId = curMine->raw_id-1;
		short		maxLoadQty = (pickUpType!=AUTO_PICK_UP) ? (short)curMine->stock_qty :
									 MAX(0, (int)(curMine->stock_qty-MIN_FIRM_STOCK_QTY)); // MAX Qty mine can supply
		short		qty = MIN(MAX_CARAVAN_CARRY_QTY-raw_qty_array[goodsId], maxLoadQty); // MAX Qty caravan can carry

		raw_qty_array[goodsId]		+= qty;
		err_when(raw_qty_array[goodsId]<0 || raw_qty_array[goodsId]>MAX_CARAVAN_CARRY_QTY);
		curMine->stock_qty	-= qty;

		//### begin trevor 7/8 ####//

		if( maxLoadQty > 0 )
			last_load_goods_date = info.game_date;

		//#### end trevor 7/8 ####//
	}
}
//----------- End of function UnitCaravan::mine_load_goods -----------//


//--------- Begin of function UnitCaravan::factory_unload_goods ---------//
// unload raw material to factory
//
void UnitCaravan::factory_unload_goods()
{
	CaravanStop	*stopPtr = stop_array+dest_stop_id-1;
	FirmFactory	*curFactory = (FirmFactory*) firm_array[stopPtr->firm_recno];
	err_when(curFactory->firm_id != FIRM_FACTORY);

	if(curFactory->nation_recno!=nation_recno)
		return; // don't unload goods if this isn't our own factory

	//--- if the factory does not have any stock and there is no production, set it to type of raw materials the caravan is carring ---//

	if( curFactory->stock_qty == 0 &&
		 curFactory->raw_stock_qty == 0 &&
		 curFactory->production_30days() == 0 )
	{
		int rawCount=0;
		int rawId=0;

		for( int i=0 ; i<MAX_RAW ; i++ )
		{
			if( raw_qty_array[i] > 0 )
			{
				rawCount++;
				rawId=i+1;
			}
		}

		//-- only if the caravan only carries one type of raw material --//

		if( rawCount==1 && rawId )
			curFactory->product_raw_id = rawId;
	}

	//---------- unload materials automatically --------//
	int goodsId = curFactory->product_raw_id-1;

	if(raw_qty_array[goodsId]) // caravan has this raw materials
	{
		short qty = MIN(raw_qty_array[goodsId], (short)(curFactory->max_raw_stock_qty-curFactory->raw_stock_qty));
		raw_qty_array[goodsId] -= qty;
		err_when(raw_qty_array[goodsId]<0);
		curFactory->raw_stock_qty += qty;
		err_when(curFactory->raw_stock_qty>curFactory->max_raw_stock_qty);
	}
}
//----------- End of function UnitCaravan::factory_unload_goods -----------//


//--------- Begin of function UnitCaravan::factory_load_goods ---------//
void UnitCaravan::factory_load_goods(char pickUpType)
{
	if(pickUpType==NO_PICK_UP)
		return; // return not allowed to load any goods

	err_when(pickUpType>=PICK_UP_RAW_FIRST && pickUpType<=PICK_UP_RAW_LAST);
	CaravanStop *stopPtr = stop_array+dest_stop_id-1;
	FirmFactory	*curFactory = (FirmFactory*) firm_array[stopPtr->firm_recno];
	err_when(curFactory->firm_id != FIRM_FACTORY);

	if(curFactory->nation_recno!=nation_recno)
		return; // don't load goods if this isn't our own factory

	//------------- load goods -----------//
	int searchProductRawId = pickUpType-PICK_UP_PRODUCT_FIRST+1;
	if(pickUpType==AUTO_PICK_UP || curFactory->product_raw_id==searchProductRawId) // auto_pick_up or is the product to pick up
	{
		int		goodsId = curFactory->product_raw_id-1;
		short		maxLoadQty = (pickUpType!=AUTO_PICK_UP) ? (short)curFactory->stock_qty :
									 MAX(0, (int)(curFactory->stock_qty-MIN_FIRM_STOCK_QTY)); // MAX Qty factory can supply
		short		qty = MIN(MAX_CARAVAN_CARRY_QTY-product_raw_qty_array[goodsId], maxLoadQty); // MAX Qty caravan can carry

		product_raw_qty_array[goodsId]	+= qty;
		err_when(product_raw_qty_array[goodsId]<0 || product_raw_qty_array[goodsId]>MAX_CARAVAN_CARRY_QTY);
		curFactory->stock_qty	-= qty;

		//### begin trevor 7/8 ####//

		if( maxLoadQty > 0 )
			last_load_goods_date = info.game_date;

		//#### end trevor 7/8 ####//
	}
}
//----------- End of function UnitCaravan::factory_load_goods -----------//
