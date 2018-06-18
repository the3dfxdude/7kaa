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

#include <OU_MARI.h>
#include <OF_MINE.h>
#include <OF_FACT.h>
#include <OF_MARK.h>

//--------- Begin of function ShipStop::update_pick_up ---------//
// enableFlag	- represent which button will be displayed
//
int ShipStop::update_pick_up(char *enableFlag)
{
	#ifdef DBEUG
		if(pick_up_type == AUTO_PICK_UP || pick_up_type == NO_PICK_UP)
		{
			for(int di=0; di<MAX_PICK_UP_GOODS; ++di)
				err_when(pick_up_array[di]);
		}
	#endif

	static char dummyBuffer[MAX_GOODS_SELECT_BUTTON];
	if(enableFlag==NULL)
		enableFlag = dummyBuffer;

	memset(enableFlag, 0, sizeof(char)*MAX_GOODS_SELECT_BUTTON);

	Firm	*harborPtr = firm_array[firm_recno];
	Firm	*firmPtr;
	MarketGoods *marketGoodsPtr;
	int	selectiveMode = (pick_up_type!=AUTO_PICK_UP && pick_up_type!=NO_PICK_UP);
	int	goodsNum=0; // represent the number of cargo displayed in the menu for this stop
	int	firstGoodsId = 0;
	int 	id, i, j;

	for(i=harborPtr->linked_firm_count-1; i>=0; --i)
	{
		err_when(firm_array.is_deleted(harborPtr->linked_firm_array[i]));
		firmPtr = firm_array[harborPtr->linked_firm_array[i]];

		switch(firmPtr->firm_id)
		{
			case FIRM_MINE:
				if((id = ((FirmMine*)firmPtr)->raw_id)) // 1-3
				{
					goodsNum++;
					enableFlag[id] = 1;
					if(firstGoodsId==0)
						firstGoodsId = id;
				}
				break;

			case FIRM_FACTORY:
				if((id = ((FirmFactory*)firmPtr)->product_raw_id)) // 1-3
				{
					id += MAX_RAW;	// 4-6
					goodsNum++;
					enableFlag[id] = 1;
					if(firstGoodsId==0)
						firstGoodsId = id;
				}
				break;
			
			case FIRM_MARKET:
				marketGoodsPtr = ((FirmMarket*) firmPtr)->market_goods_array;
				for(j=1; j<=MAX_MARKET_GOODS; j++, marketGoodsPtr++)
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
				break;

			default:	err_here();
						break;
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
//----------- End of function ShipStop::update_pick_up -----------//
