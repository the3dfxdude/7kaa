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

 //Filename    : OU_CARAS.CPP
 //Description : Unit Caravan - Stops


#include <OU_CARA.h>
#include <OF_MINE.h>
#include <OF_FACT.h>
#include <OF_MARK.h>

//--------- Begin of function TradeStop::pick_up_set_auto ---------//
void TradeStop::pick_up_set_auto()
{
	memset(pick_up_array, 0, sizeof(char)*MAX_PICK_UP_GOODS);
	pick_up_type = AUTO_PICK_UP;
}
//----------- End of function TradeStop::pick_up_set_auto -----------//


//--------- Begin of function TradeStop::pick_up_set_none ---------//
void TradeStop::pick_up_set_none()
{
	memset(pick_up_array, 0, sizeof(char)*MAX_PICK_UP_GOODS);
	pick_up_type = NO_PICK_UP;
}
//----------- End of function TradeStop::pick_up_set_none -----------//


//--------- Begin of function TradeStop::pick_up_toggle ---------//
void TradeStop::pick_up_toggle(int pos)
{
	char *posPtr = &pick_up_array[pos-1];
	if(*posPtr)
	{
		*posPtr = 0;

		char firmId = firm_array[firm_recno]->firm_id;
		if(firmId==FIRM_MARKET || firmId==FIRM_HARBOR)
		{
			int i, allZero;
			for(i=0, allZero=1; i<MAX_PICK_UP_GOODS; ++i)
			{
				if(pick_up_array[i])
				{
					allZero = 0;
					pick_up_type = i+1;
					break;
				}
			}

			if(allZero)
				pick_up_type = NO_PICK_UP;
		}
		else
			pick_up_type = NO_PICK_UP;
	}
	else
	{
		*posPtr = 1;
		pick_up_type = pos; // that means selective
	}
}
//----------- End of function TradeStop::pick_up_toggle -----------//


//--------- Begin of function TradeStop::num_of_pick_up_goods ---------//
int TradeStop::num_of_pick_up_goods(char *enableTable)
{
	int num=0;

	//for(int i=0; i<MAX_PICK_UP_GOODS; i++)
	for(int i=1; i<=MAX_PICK_UP_GOODS; i++)
	{
		if(enableTable[i])
			num++;
	}

	return num;
}
//----------- End of function TradeStop::num_of_pick_up_goods -----------//


//--------- Begin of function TradeStop::mp_pick_up_set_auto ---------//
void TradeStop::mp_pick_up_set_auto(char *enableTable)
{
	if(pick_up_type==AUTO_PICK_UP)
		return;

	//---------------------------------------------------------------------------//
	// 1) none -> auto, only if more than one kind of cargo can be selected
	// 2) selective -> auto, only if num of goods > 1
	//---------------------------------------------------------------------------//
	if(pick_up_type==NO_PICK_UP || num_of_pick_up_goods(enableTable)>1)
	{
		memset(pick_up_array, 0, sizeof(char)*MAX_PICK_UP_GOODS);
		pick_up_type = AUTO_PICK_UP;
	}
}
//----------- End of function TradeStop::mp_pick_up_set_auto -----------//


//--------- Begin of function TradeStop::mp_pick_up_set_none ---------//
void TradeStop::mp_pick_up_set_none(char *enableTable)
{
	if(pick_up_type==NO_PICK_UP)
		return;

	//---------------------------------------------------------------------------//
	// 1) none -> none, only if more than one kind of cargo can be selected
	// 2) selective -> none, only if num of goods > 1
	//---------------------------------------------------------------------------//
	if(pick_up_type==AUTO_PICK_UP || num_of_pick_up_goods(enableTable)>1)
	{
		memset(pick_up_array, 0, sizeof(char)*MAX_PICK_UP_GOODS);
		pick_up_type = NO_PICK_UP;
	}
}
//----------- End of function TradeStop::mp_pick_up_set_none -----------//


//--------- Begin of function CaravanStop::update_pick_up ---------//
// enableFlag	- represent which button will be displayed
//
int CaravanStop::update_pick_up(char *enableFlag)
{
	#ifdef DBEUG
		if(pick_up_type == AUTO_PICK_UP || pick_up_type == NO_PICK_UP)
		{
			for(int di=0; di<MAX_PICK_UP_GOODS; ++di)
				err_when(pick_up_array[di]);
		}
	#endif

	err_when(firm_array.is_deleted(firm_recno));

	static char dummyBuffer[MAX_GOODS_SELECT_BUTTON];
	if(enableFlag==NULL)
		enableFlag = dummyBuffer;

	memset(enableFlag, 0, sizeof(char)*MAX_GOODS_SELECT_BUTTON);

	Firm *firmPtr = firm_array[firm_recno];
	MarketGoods *marketGoodsPtr;
	int	goodsNum=0; // represent the number of cargo displayed in the menu for this stop
	int	firstGoodsId = 0;
	int 	id, i, selectiveMode;

	switch(firmPtr->firm_id)
	{
		case FIRM_MINE:
				err_when(pick_up_type==AUTO_PICK_UP);
				id = ((FirmMine*)firmPtr)->raw_id+PICK_UP_RAW_FIRST-1;
				if(id)
				{
					goodsNum = enableFlag[id] = 1;
					
					if(!pick_up_array[id-1])
						pick_up_set_none(); // nothing can be taken if no cargo is matched
				}
				break;

		case FIRM_FACTORY:
				err_when(pick_up_type==AUTO_PICK_UP);
				id = ((FirmFactory*)firmPtr)->product_raw_id+PICK_UP_PRODUCT_FIRST-1;
				if(id)
				{
					goodsNum = enableFlag[id] = 1;

					if(!pick_up_array[id-1])
						pick_up_set_none(); // nothing can be taken if no cargo is matched
				}
				break;

		case FIRM_MARKET:
				marketGoodsPtr = ((FirmMarket*) firmPtr)->market_goods_array;
				selectiveMode = (pick_up_type!=AUTO_PICK_UP && pick_up_type!=NO_PICK_UP);
				for(i=1; i<=MAX_MARKET_GOODS; i++, marketGoodsPtr++)
				{
					if((id = marketGoodsPtr->raw_id)) // 1-3
					{
						goodsNum++;
						enableFlag[id] = 1;
						if(firstGoodsId==0)
							firstGoodsId = id;
					}
					else if((id = marketGoodsPtr->product_raw_id)) // 1-3
					{
						id += MAX_RAW;	// 4-6
						goodsNum++;
						enableFlag[id] = 1;
						if(firstGoodsId==0)
							firstGoodsId = id;
					}
				}

				for(i=0; i<MAX_PICK_UP_GOODS; ++i)
				{
					if(pick_up_array[i])
					{
						if(enableFlag[i+1]==0)
							pick_up_array[i] = 0;
					}
				}
				break;
	
		default:	err_here();
					break;
	}

	if(goodsNum==0 && pick_up_type!=NO_PICK_UP)
		pick_up_set_none();

	if(goodsNum==1 && pick_up_type==AUTO_PICK_UP)
	{
		pick_up_type = firstGoodsId; // change to selective
		pick_up_array[pick_up_type-1] = 1;
	}

	err_when(pick_up_type==AUTO_PICK_UP && goodsNum==0);
	#ifdef DEBUG
		int debugCount = 0;
		if(pick_up_type!=AUTO_PICK_UP && pick_up_type!=NO_PICK_UP)
		{
			for(int di=1; di<=MAX_PICK_UP_GOODS; ++di)
			{
				if(enableFlag[di])
					debugCount++;
			}
			err_when(debugCount==0);
		}
	#endif

	return goodsNum;
}
//----------- End of function CaravanStop::update_pick_up -----------//


//--------- Begin of function CaravanStop::mp_pick_up_toggle ---------//
void CaravanStop::mp_pick_up_toggle(int pos)
{
	err_when(firm_array.is_deleted(firm_recno));
	//------------------------------------------------------------//
	// verify for cargo button
	// if not exist then return else call pick_up_toggle(int pos)
	//------------------------------------------------------------//

	Firm *firmPtr = firm_array[firm_recno];
	MarketGoods *marketGoodsPtr;
	int	cargoExist = 0;
	int	id, i;

	switch(firmPtr->firm_id)
	{
		case FIRM_MINE:
				id = ((FirmMine*)firmPtr)->raw_id+PICK_UP_RAW_FIRST-1;
				if(id==pos)
					cargoExist = 1;
				break;

		case FIRM_FACTORY:
				id = ((FirmFactory*)firmPtr)->product_raw_id+PICK_UP_PRODUCT_FIRST-1;
				if(id==pos)
					cargoExist = 1;
				break;

		case FIRM_MARKET:
				marketGoodsPtr = ((FirmMarket*) firmPtr)->market_goods_array;
				for(i=1; i<=MAX_MARKET_GOODS; i++, marketGoodsPtr++)
				{
					if((id = marketGoodsPtr->raw_id)) // 1-3
					{
						if(id==pos)
						{
							cargoExist = 1;
							break;
						}
					}
					else if((id = marketGoodsPtr->product_raw_id)) // 1-3
					{
						id += MAX_PRODUCT;	// 4-6
						if(id==pos)
						{
							cargoExist = 1;
							break;
						}
					}
				}
				break;

		default:	err_here();
					break;
	}

	if(cargoExist)
		pick_up_toggle(pos);
}
//----------- End of function CaravanStop::mp_pick_up_toggle -----------//
