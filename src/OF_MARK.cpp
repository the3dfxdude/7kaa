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

//Filename    : OF_MARK.CPP
//Description : Firm Market Place

#include <OINFO.h>
#include <OVGA.h>
#include <vga_util.h>
#include <OSTR.h>
#include <OBUTTON.h>
#include <OBUTT3D.h>
#include <OFONT.h>
#include <ORAWRES.h>
#include <OIMGRES.h>
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
#include <OREMOTE.h>
#include <OSE.h>
#include <OMOUSE.h>
#include "gettext.h"

//------- define static vars -------//

struct Point
{
	short	x;
	short y;
};

static Point section_point_array[] =
{
	{ 40, 30 },
	{ 29, 42 },
	{ 24, 56 },
};

static Point slot_point_array[] =
{
	{  0,  0 },
	{  6,  1 },
	{ 12,  2 },
	{  8,  6 },
	{ 14,  7 },
	{ 20,  8 },
	{ 16, 12 },
	{ 22, 13 },
	{ 28, 14 },
};

static Button3D button_hire_caravan;
static Button 	 button_clear_stock[MAX_MARKET_GOODS];
static Button   button_switch_restock;

//--------- Begin of function FirmMarket::FirmMarket ---------//
//
FirmMarket::FirmMarket()
{
	max_stock_qty = (float) MAX_MARKET_STOCK;

	memset( market_goods_array	 , 0, sizeof(MarketGoods) * MAX_MARKET_GOODS );
	memset( market_raw_array	 , 0, sizeof(market_raw_array) );
	memset( market_product_array, 0, sizeof(market_product_array) );

	next_output_link_id	  = 0;
	next_output_firm_recno = 0;

	no_linked_town_since_date  = 0;
	last_import_new_goods_date = 0;
	// ####### patch begin Gilbert 23/1 #######//
	restock_type = RESTOCK_ANY;
	// ####### end begin Gilbert 23/1 #######//
}
//----------- End of function FirmMarket::FirmMarket -----------//


//--------- Begin of function FirmMarket::~FirmMarket ---------//
//
FirmMarket::~FirmMarket()
{
}
//----------- End of function FirmMarket::~FirmMarket -----------//


//--------- Begin of function FirmMarket::init_derived ---------//
//
void FirmMarket::init_derived()
{
	//------ redistribute town demand --------//

	town_array.distribute_demand();

	//-------- set restock_type for AI only --------//

	if( firm_ai )
	{
		Firm *firmPtr, *otherFirm;

		restock_type = RESTOCK_PRODUCT;		// default to product

		for( int i=0 ; i<linked_firm_count ; i++ )
		{
			firmPtr = firm_array[ linked_firm_array[i] ];

			//------ if this is our mine -------//

			if( firmPtr->firm_id != FIRM_MINE ||
				 firmPtr->nation_recno != nation_recno )
			{
				continue;
			}

			//--- if the mine doesn't have links to other market ---//

			int j;
			for( j=firmPtr->linked_firm_count-1 ; j>=0 ; j-- )
			{
				otherFirm = firm_array[ firmPtr->linked_firm_array[j] ];

				if( otherFirm->nation_recno == nation_recno &&
					 otherFirm->firm_recno 	 != firm_recno &&
					 otherFirm->firm_id		 == FIRM_MARKET &&
					 ((FirmMarket*)otherFirm)->is_raw_market() )
				{
					break;
				}
			}

			if( j<0 )	// if the mine doesn't have any links to other markets
			{
				restock_type = RESTOCK_RAW;
				break;
			}
		}
	}
}
//----------- End of function FirmMarket::init_derived -----------//


//--------- Begin of function FirmMarket::next_day ---------//
//
void FirmMarket::next_day()
{
	//----- call next_day() of the base class -----//

	Firm::next_day();

	//---- update trade link to harbors to towns -----//

	update_trade_link();

	//-------- input goods ----------//

	if( info.game_date%PROCESS_GOODS_INTERVAL == firm_recno%PROCESS_GOODS_INTERVAL )
	{
		input_goods(50);								// input maximum 50 qty of goods per day
		set_next_output_firm();						// set next output firm
	}

	//-------- sell goods --------//

	sell_goods();

	//------- free up unused slots -------//

	//### begin alex 24/10 ###//
	//if( info.game_date%30 == firm_recno%30 )
	//	free_unused_slot();
	//#### end alex 24/10 ####//
}
//----------- End of function FirmMarket::next_day -----------//


//--------- Begin of function FirmMarket::next_month ---------//
//
void FirmMarket::next_month()
{
	Firm::next_month();

	//------ post goods supply data ------//

	MarketGoods* marketGoods = market_goods_array;

	for( int i=0 ; i<MAX_MARKET_GOODS ; i++, marketGoods++ )
	{
		marketGoods->last_month_supply = marketGoods->cur_month_supply;
		marketGoods->cur_month_supply  = (float) 0;

		marketGoods->last_month_sale_qty = marketGoods->cur_month_sale_qty;
		marketGoods->cur_month_sale_qty  = (float) 0;
	}
}
//----------- End of function FirmMarket::next_month -----------//


//--------- Begin of function FirmMarket::next_year ---------//
//
void FirmMarket::next_year()
{
	Firm::next_year();

	//------ post goods supply data ------//

	MarketGoods* marketGoods = market_goods_array;

	for( int i=0 ; i<MAX_MARKET_GOODS ; i++, marketGoods++ )
	{
		marketGoods->last_year_sales = marketGoods->cur_year_sales;
		marketGoods->cur_year_sales  = (float) 0;
	}
}
//----------- End of function FirmMarket::next_year -----------//


//--------- Begin of function FirmMarket::put_info ---------//
//
void FirmMarket::put_info(int refreshFlag)
{
	disp_basic_info(INFO_Y1, refreshFlag);

	//--- only display market info if the player is allowed to trade with this market ---//

	put_market_info(INFO_Y1+50, refreshFlag);

	//------------------------------------------------//

	if( !config.show_ai_info && nation_recno!=nation_array.player_recno )
		return;

	disp_income(INFO_Y1+209, refreshFlag );	  // 1-display income figure

	if( refreshFlag == INFO_REPAINT )
		button_hire_caravan.paint( INFO_X1, INFO_Y1+251, 'A', "HIRECARA" );

	if( can_hire_caravan() )
		button_hire_caravan.enable();
	else
		button_hire_caravan.disable();
}
//----------- End of function FirmMarket::put_info -----------//


//--------- Begin of function FirmMarket::detect_info ---------//
//
int FirmMarket::detect_info()
{
	if( detect_basic_info() )
		return 1;

	if( !config.show_ai_info && nation_recno!=nation_array.player_recno )
		return 0;

	if( nation_recno != nation_array.player_recno )		// the following controls are only available for player's firms
		return 0;

	//----- detect clear stock buttons -------//

	for( int i=0 ; i<MAX_MARKET_GOODS ; i++ )
	{
		if( button_clear_stock[i].detect() )
		{
			if( !remote.is_enable() )
			{
				MarketGoods* marketGoods = market_goods_array+i;
				
				clear_market_goods(i+1);
				info.disp();
			}
			else
			{
				// message structure : <firm recno> <cell no 0-3>
				short *shortPtr = (short *)remote.new_send_queue_msg(MSG_F_MARKET_SCRAP, sizeof(short)+sizeof(short) );
				shortPtr[0] = firm_recno;
				shortPtr[1] = i;
			}
			se_ctrl.immediate_sound("TURN_OFF");
			return 1;
		}
	}

	if( button_switch_restock.detect() )
	{
		if( !remote.is_enable() )
		{
			switch_restock();
		}
		else
		{
			// message structure : <firm recno>
			short *shortPtr = (short *)remote.new_send_queue_msg(MSG_F_MARKET_RESTOCK, sizeof(short) );
			shortPtr[0] = firm_recno;
		}
		se_ctrl.immediate_sound("TURN_OFF");
		return 1;
	}

	//----- detect hire caravan button -------//

	if( button_hire_caravan.detect(GETKEY(KEYEVENT_FIRM_PATROL)) )
	{
		hire_caravan(COMMAND_PLAYER);
		return 1;
	}

	return 0;
}
//----------- End of function FirmMarket::detect_info -----------//


//------- Begin of function FirmMarket::can_hire_caravan -------//
//
// return: <int> 0 - if there is no more caravan we can hire
//					 >0 - the number of new caravans we can hire.
//
int FirmMarket::can_hire_caravan()
{
	Nation* nationPtr = nation_array[nation_recno];

	if( nationPtr->cash < 0 )
		return 0;

	int supportedCaravan = nationPtr->total_population / POPULATION_PER_CARAVAN;
	int caravanCount 		= unit_res[UNIT_CARAVAN]->nation_unit_count_array[nation_recno-1];

	if( supportedCaravan > caravanCount )
		return supportedCaravan - caravanCount;
	else
		return 0;
}
//-------- End of function FirmMarket::can_hire_caravan --------//


//--------- Begin of function FirmMarket::hire_caravan ---------//
//
short FirmMarket::hire_caravan(char remoteAction)
{
	if( !can_hire_caravan() )
		return 0;

	//---------------------------------------//

	Nation *nationPtr = nation_array[nation_recno];

	if(!remoteAction && remote.is_enable())
	{
		// packet structure : <town recno>
		short *shortPtr = (short *) remote.new_send_queue_msg(MSG_F_MARKET_HIRE_CARA, sizeof(short));
		*shortPtr = firm_recno;
		return 0;
	}

	//---------- add the unit now -----------//

	int unitRecno = create_unit( UNIT_CARAVAN );

	UnitCaravan* unitCaravan = (UnitCaravan*)unit_array[unitRecno];

	unitCaravan->loyalty = 100;
	unitCaravan->set_stop( 1, loc_x1, loc_y1, COMMAND_AUTO );

	//---------- deduct cash for the caravan's cost ----------//

	if(unitCaravan)
		return unitCaravan->sprite_recno;
	else
		return 0;
}
//----------- End of function FirmMarket::hire_caravan -----------//


//--------- Begin of function FirmMarket::put_market_info ---------//
//
void FirmMarket::put_market_info(int dispY1, int refreshFlag)
{
	static char lastNoTrade;

	//--- only display market info if the player is allowed to trade with this market ---//

	char noTrade;

	if( nation_array.player_recno )
		noTrade = nation_array[nation_recno]->get_relation(nation_array.player_recno)->trade_treaty==0;
	else
		noTrade = 0;		// the player has been destroyed

	if( config.show_ai_info )
		noTrade = 0;

	if( lastNoTrade != noTrade )
	{
		lastNoTrade = noTrade;

		if( refreshFlag == INFO_UPDATE )
		{
			info.disp();
			return;
		}
	}

	if( noTrade )
	{
		if( refreshFlag == INFO_REPAINT )
		{
			vga_util.d3_panel_up( INFO_X1, dispY1, INFO_X2, dispY1+51 );
			font_san.put_paragraph( INFO_X1, dispY1+8, INFO_X2, dispY1+51, _("You're not permitted to trade with this market."), 4, 1, 1, Font::CENTER_JUSTIFY );
		}

		return;
	}

	//-----------------------------------------------------//

	int 				i, x, y=dispY1;
	static char* 	last_bitmap_array[MAX_MARKET_GOODS];
	MarketGoods*	marketGoods;
	String			str;
	char*				bitmapPtr;

	for( i=0, marketGoods=market_goods_array ; i<MAX_MARKET_GOODS ; i++, marketGoods++, y+=53 )
	{
		if( refreshFlag == INFO_REPAINT )
			vga_util.d3_panel_up( INFO_X1, y, INFO_X2, y+51 );

		if( marketGoods->raw_id )
		{
			str = _(raw_res[marketGoods->raw_id]->name);
			bitmapPtr = raw_res.small_raw_icon(marketGoods->raw_id);
		}
		else if( marketGoods->product_raw_id )
		{
			str = raw_res.product_name(marketGoods->product_raw_id);
			bitmapPtr = raw_res.small_product_icon(marketGoods->product_raw_id);
		}
		else
		{
			button_clear_stock[i].reset();
			continue;
		}

		//----- if product type changed, refresh info ----//

		if( bitmapPtr != last_bitmap_array[i] )
		{
			refreshFlag = INFO_REPAINT;
			last_bitmap_array[i] = bitmapPtr;
		}

		//------------ display info --------------//

		x=INFO_X1+2;

		if( refreshFlag == INFO_REPAINT )
		{
			vga_front.put_bitmap_trans( x+3, y+4, bitmapPtr );
			font_san.put( x+19, y+4, str );

			if( nation_recno == nation_array.player_recno )
			{
				button_clear_stock[i].paint_text( INFO_X2-46, y+2, INFO_X2-3, y+19, _("Clear") );	// Clear Stock
				button_clear_stock[i].set_help_code( "MK_CLEAR" );
			}
		}

		x+=3;
		int ty=y+18;

		str  = (int) marketGoods->stock_qty;
		str += "/";
		str += (int) max_stock_qty;

		font_san.field( x, ty, _("Stock"), x+60, str, x+119, refreshFlag, "MK_STOCK" );

		font_san.field( x, ty+16, _("Sales"), x+60, (int) marketGoods->sales_365days(), 2,
							 x+104, refreshFlag, "MK_SALES" );

		x+=105;

		// ####### patch begin Gilbert 16/3 #########//
		//font_san.field( x, ty+16, "Demand", x+70, (int) marketGoods->month_demand, 1,
		//					 INFO_X2-2, refreshFlag, "MK_DEMAN" );
		font_san.field( x, ty+16, _("Demand"), x+67, (int) marketGoods->month_demand, 1,
							 INFO_X2-1, refreshFlag, "MK_DEMAN" );
		// ####### patch end Gilbert 16/3 #########//
	}
}
//----------- End of function FirmMarket::put_market_info -----------//


const char *restocking_msg[] =
{
	N_("Any"),
	N_("Factory"),
	N_("Mine"),
	N_("None"),
};
//--------- Begin of function FirmMarket::disp_income ---------//
//
// Display monthly expense information.
//
void FirmMarket::disp_income(int dispY1, int refreshFlag)
{
	if( refreshFlag == INFO_REPAINT )
		vga_util.d3_panel_up( INFO_X1, dispY1, INFO_X2, dispY1+39 );

	int x=INFO_X1+4, y=dispY1+4;

	font_san.field( x, y, _("Yearly Income"), x+110, (int) income_365days(), 2, x+200, refreshFlag, "MK_INCOM" );
	font_san.field( x, y+16, _("Restock From"), x+110, restocking_msg[restock_type], x+200, refreshFlag, "MK_RSTKF" );

	if( nation_recno == nation_array.player_recno )
	{
		button_switch_restock.paint_text( INFO_X2-20, y+18, INFO_X2-3, y+36, ">" );	// change restocking
		button_switch_restock.set_help_code( "MK_RSTKS" );
	}
}
//----------- End of function FirmMarket::disp_income -----------//


//------- Begin of function FirmMarket::draw -----------//
//
// Draw product stocks.
//
void FirmMarket::draw(int displayLayer)
{
	Firm::draw(displayLayer);

	if( under_construction )
		return;

	if( displayLayer == 1)
	{
		//------- draw market goods cargoes ---------//

		int   	 		i, j, x, y, cargoCount, sectionId=0;
		MarketGoods*	marketGoods;
		char* 			iconPtr;

		for( i=0, marketGoods=market_goods_array ; i<MAX_MARKET_GOODS ; i++, marketGoods++ )
		{
			if( marketGoods->raw_id )
				iconPtr = raw_res.small_raw_icon(marketGoods->raw_id);

			else if( marketGoods->product_raw_id )
				iconPtr = raw_res.small_product_icon(marketGoods->product_raw_id);

			else
				continue;

			//------- draw cargo on the firm bitmap buffer --------//

			cargoCount = MAX_CARGO * (int)marketGoods->stock_qty/(int)max_stock_qty;
			cargoCount = MAX(1, cargoCount);

			x = ZOOM_X1 + (loc_x1-world.zoom_matrix->top_x_loc) * ZOOM_LOC_WIDTH + section_point_array[sectionId].x;
			y = ZOOM_Y1 + (loc_y1-world.zoom_matrix->top_y_loc) * ZOOM_LOC_HEIGHT + section_point_array[sectionId].y;

			sectionId++;

			for( j=0 ; j<cargoCount ; j++ )
			{
				world.zoom_matrix->put_bitmap_clip(x+slot_point_array[j].x, y+slot_point_array[j].y, iconPtr );
			}
		}
	}
}
//--------- End of function FirmMarket::draw -----------//


//--------- Begin of function FirmMarket::input_goods ---------//
//
// Input goods from factories and mines.
//
// <int> maxInputQty - maximum goods can be inputed in this call.
//
void FirmMarket::input_goods(int maxInputQty)
{
	//------ scan for a firm to input raw materials --------//

	int   	 		i, t;
	float				inputQty;
	Firm* 	 		firmPtr;
	FirmMine* 		firmMine;
	FirmFactory* 	firmFactory;
	MarketGoods*	marketGoods;
	Nation*		   nationPtr = nation_array[nation_recno];
	char				is_inputing_array[MAX_MARKET_GOODS];
	short				queued_firm_recno=0;

	memset( is_inputing_array, 0, sizeof(is_inputing_array) );

	for( t=0 ; t<linked_firm_count ; t++ )
	{
		if( linked_firm_enable_array[t] != LINK_EE )
			continue;

		firmPtr = firm_array[linked_firm_array[t]];

		//----------- check if the firm is a mine ----------//

		if( firmPtr->firm_id != FIRM_MINE && firmPtr->firm_id != FIRM_FACTORY )
			continue;

		//--------- if it's a mine ------------//

		if( firmPtr->firm_id == FIRM_MINE && is_raw_market() )
		{
			firmMine = (FirmMine*) firmPtr;

			if( firmMine->raw_id )
			{
				for( i=0, marketGoods=market_goods_array ; i<MAX_MARKET_GOODS ; i++, marketGoods++ )
				{
					//--- only assign a slot to the product if it comes from a firm of our own ---//

					if( marketGoods->raw_id == firmMine->raw_id )
					{
						is_inputing_array[i] = 1;

						if( firmMine->next_output_firm_recno == firm_recno &&
							 firmMine->stock_qty > 0 && marketGoods->stock_qty < max_stock_qty )
						{
							inputQty = MIN( firmMine->stock_qty, maxInputQty );
							inputQty = MIN( inputQty, max_stock_qty - marketGoods->stock_qty );

							firmMine->stock_qty	  -= inputQty;
							marketGoods->stock_qty += inputQty;
							marketGoods->cur_month_supply += inputQty;

							if( firmPtr->nation_recno != nation_recno )
								nationPtr->import_goods(IMPORT_RAW, firmPtr->nation_recno, inputQty*RAW_PRICE );
						}
						else if( marketGoods->stock_qty == max_stock_qty )
						{
							marketGoods->cur_month_supply++;		// add it so the other functions can know that this market has direct supply links
						}

						break;
					}
				}

				//----- no matched slot for this goods -----//

				if( i==MAX_MARKET_GOODS && firmMine->stock_qty>0 && !queued_firm_recno )
					queued_firm_recno = firmPtr->firm_recno;
			}
		}

		//--------- if it's a factory ------------//

		else if( firmPtr->firm_id == FIRM_FACTORY && is_retail_market() )
		{
			firmFactory = (FirmFactory*) firmPtr;

			if( firmFactory->product_raw_id )
			{
				for( i=0, marketGoods=market_goods_array ; i<MAX_MARKET_GOODS ; i++, marketGoods++ )
				{
					if( marketGoods->product_raw_id == firmFactory->product_raw_id )
					{
						is_inputing_array[i] = 1;

						if( firmFactory->next_output_firm_recno == firm_recno &&
							 firmFactory->stock_qty > 0 && marketGoods->stock_qty < max_stock_qty )
						{
							inputQty = MIN( firmFactory->stock_qty, maxInputQty );
							inputQty = MIN( inputQty, max_stock_qty - marketGoods->stock_qty );

							firmFactory->stock_qty -= inputQty;
							marketGoods->stock_qty += inputQty;
							marketGoods->cur_month_supply += inputQty;

							if( firmPtr->nation_recno != nation_recno )
								nationPtr->import_goods(IMPORT_PRODUCT, firmPtr->nation_recno, inputQty*PRODUCT_PRICE );
						}
						else if( marketGoods->stock_qty == max_stock_qty )
						{
							marketGoods->cur_month_supply++;		// add it so the other functions can know that this market has direct supply links
						}

						break;
					}
				}

				//----- no matched slot for this goods -----//

				if( i==MAX_MARKET_GOODS && firmFactory->stock_qty>0 && !queued_firm_recno )
					queued_firm_recno = firmPtr->firm_recno;
			}
		}
	}

	//---- if there are any empty slots for new goods -----//

	if( queued_firm_recno > 0 )
	{
		firmPtr = firm_array[queued_firm_recno];

		for( i=0, marketGoods=market_goods_array ; i<MAX_MARKET_GOODS ; i++, marketGoods++ )
		{
			if( !is_inputing_array[i] && marketGoods->stock_qty==0 )
			{
				if( firmPtr->firm_id == FIRM_MINE && is_raw_market() )
				{
					set_goods(1, ((FirmMine*)firmPtr)->raw_id, i);
					break;
				}
				else if( firmPtr->firm_id == FIRM_FACTORY && is_retail_market() )
				{
					set_goods(0, ((FirmFactory*)firmPtr)->product_raw_id, i);
					break;
				}
			}
		}
	}
}
//----------- End of function FirmMarket::input_goods -----------//


//------- Begin of function FirmMarket::set_goods -----------//
void FirmMarket::set_goods(int isRaw, int goodsId, int position)
{
	MarketGoods *marketGoods = market_goods_array+position;
	if(isRaw)
	{
		if(marketGoods->raw_id)
			market_raw_array[marketGoods->raw_id-1] = NULL;
		else if(marketGoods->product_raw_id)
			market_product_array[marketGoods->product_raw_id-1] = NULL;

		marketGoods->raw_id = goodsId;
		marketGoods->product_raw_id = 0;
		market_raw_array[goodsId-1] = marketGoods;
	}
	else
	{
		if(marketGoods->product_raw_id)
			market_product_array[marketGoods->product_raw_id-1] = NULL;
		else if(marketGoods->raw_id)
			market_raw_array[marketGoods->raw_id-1] = NULL;

		marketGoods->raw_id = 0;
		marketGoods->product_raw_id = goodsId;
		market_product_array[goodsId-1] = marketGoods;
	}

	if( firm_array.selected_recno == firm_recno )
		info.disp();
}
//----------- End of function FirmMarket::set_goods -----------//


//------- Begin of function FirmMarket::sell_goods -----------//
//
// Sell products to consumers. Called by Town::sell_goods()
//
void FirmMarket::sell_goods()
{
	//----------- sell products now ------------//

	int   	 		i;
	float			   saleQty;
	MarketGoods*	marketGoods;

	for( i=0, marketGoods=market_goods_array ; i<MAX_MARKET_GOODS ; i++, marketGoods++ )
	{
		if( marketGoods->product_raw_id && marketGoods->stock_qty > 0 )
		{
			saleQty = MIN(marketGoods->month_demand/30, marketGoods->stock_qty);

			marketGoods->stock_qty -= saleQty;

			marketGoods->cur_month_sale_qty += saleQty;
			marketGoods->cur_year_sales  	  += saleQty * CONSUMER_PRICE;

			add_income(INCOME_SELL_GOODS, saleQty * CONSUMER_PRICE);
		}
	}
}
//--------- End of function FirmMarket::sell_goods -----------//


//------- Begin of function FirmMarket::free_unused_slot -----------//
//
// Free up unused slots (those with sales==0 and stock_qty==0)
//
void FirmMarket::free_unused_slot()
{
	int   	 		i;
	MarketGoods*	marketGoods;

	for( i=0, marketGoods=market_goods_array ; i<MAX_MARKET_GOODS ; i++, marketGoods++ )
	{
		if( marketGoods->product_raw_id || marketGoods->raw_id )
		{
			if( marketGoods->sales_365days()==0 &&
				 marketGoods->supply_30days()==0 &&
				 marketGoods->stock_qty==0 )
			{
				clear_market_goods(i+1);
			}
		}
	}
}
//--------- End of function FirmMarket::free_unused_slot -----------//


//------- Begin of function FirmMarket::clear_market_goods ------//
void FirmMarket::clear_market_goods(int position)
{
	MarketGoods	*marketGoods = market_goods_array + position - 1;

	err_when((marketGoods->raw_id && marketGoods->product_raw_id) ||
				(!marketGoods->raw_id && !marketGoods->product_raw_id));
	marketGoods->stock_qty = (float) 0;

	if(marketGoods->raw_id)
	{
		market_raw_array[marketGoods->raw_id-1] = NULL;
		marketGoods->raw_id = 0;
	}
	else
	{
		market_product_array[marketGoods->product_raw_id-1] = NULL;
		marketGoods->product_raw_id = 0;
	}
}
//--------- End of function FirmMarket::clear_market_goods -----------//


//------- Begin of function FirmMarket::set_next_output_firm ------//
//
// Set next_output_firm_recno, the recno of the linked firm
// to which this firm is going to output goods.
//
void FirmMarket::set_next_output_firm()
{
	int i, firmRecno, firmId;

	for( i=0 ; i<linked_firm_count ; i++ )		// MAX tries
	{
		if( ++next_output_link_id > linked_firm_count )    // next firm in the link
			next_output_link_id = 1;

		if( linked_firm_enable_array[next_output_link_id-1] == LINK_EE )
		{
			firmRecno = linked_firm_array[next_output_link_id-1];
			firmId 	 = firm_array[firmRecno]->firm_id;

			if( firmId==FIRM_FACTORY )
			{
				next_output_firm_recno = firmRecno;
				return;
			}
		}
	}

	next_output_firm_recno = 0;		// this mine has no linked output firms
}
//-------- End of function FirmMarket::set_next_output_firm ---------//


//------- Begin of function FirmMarket::stock_value_index ------//
//
// For AI, return a 0-100 index number telling the total value
// of the market's stock.
//
int FirmMarket::stock_value_index()
{
	int   	 		i;
	float				totalValue = (float) 0;
	MarketGoods*	marketGoods;

	for( i=0, marketGoods=market_goods_array ; i<MAX_MARKET_GOODS ; i++, marketGoods++ )
	{
		if( marketGoods->raw_id )
		{
			totalValue += marketGoods->stock_qty * RAW_PRICE;
		}
		else if( marketGoods->product_raw_id )
		{
			totalValue += marketGoods->stock_qty * PRODUCT_PRICE;
		}
	}

	return 100 * (int)totalValue / (MAX_MARKET_GOODS * PRODUCT_PRICE * MAX_MARKET_STOCK);
}
//-------- End of function FirmMarket::stock_value_index ---------//


//--------- Begin of function FirmMarket::free_slot_count ---------//
//
// Count the number of free slots available in the market.
//
int FirmMarket::free_slot_count()
{
	MarketGoods* marketGoods = market_goods_array;
	int			 freeSlotCount = 0;

	for( int i=0 ; i<MAX_MARKET_GOODS ; i++, marketGoods++ )
	{
		if( !marketGoods->raw_id && !marketGoods->product_raw_id )
			freeSlotCount++;
	}

	return freeSlotCount;
}
//----------- End of function FirmMarket::free_slot_count -----------//


//--------- Begin of function FirmMarket::read_derived_file ---------//
//
int FirmMarket::read_derived_file(File* filePtr)
{
	if( !Firm::read_derived_file(filePtr) )
		return 0;

	//----- reset market_raw_array[] & market_product_array[] ----//

	int i;
	for( i=0 ; i<MAX_RAW ; i++ )
	{
		market_raw_array[i]	   = NULL;
		market_product_array[i] = NULL;
	}

	//------- rebuild market_product_array --------//

	int rawId, productId;

	for( i=0 ; i<MAX_MARKET_GOODS ; i++ )
	{
		rawId 	 = market_goods_array[i].raw_id;
		productId = market_goods_array[i].product_raw_id;

		if( rawId )
			market_raw_array[rawId-1] = market_goods_array + i;

		if( productId )
			market_product_array[productId-1] = market_goods_array + i;
	}

        //---- force ai to update restocking type and links after load ----//

	if( firm_ai )
		ai_link_checked = 0;

	return 1;
}
//----------- End of function FirmMarket::read_derived_file -----------//


//----- Begin of function FirmMarket::update_trade_link -----//
//
// Update the status of links to harbors and towns based
// on the current trade treaty status. 
//
void FirmMarket::update_trade_link()
{
	Nation* ownNation = nation_array[nation_recno];
	int tradeTreaty;

	//------ update links to harbors -----//

	Firm* firmPtr;

	int i;
	for( i=0 ; i<linked_firm_count ; i++ )
	{
		 firmPtr = firm_array[linked_firm_array[i]];

		 if( firmPtr->firm_id != FIRM_HARBOR )
			 continue;

		 tradeTreaty = ownNation->get_relation(firmPtr->nation_recno)->trade_treaty || firmPtr->nation_recno==nation_recno;

		 if( linked_firm_enable_array[i] != (tradeTreaty ? LINK_EE : LINK_DD) )
			 toggle_firm_link( i+1, tradeTreaty, COMMAND_AUTO, 1 );					// 1-toggle both side
	}

	//------ update links to towns -----//

	Town* townPtr;

	for( i=0 ; i<linked_town_count ; i++ )
	{
		 townPtr = town_array[linked_town_array[i]];

		 if( !townPtr->nation_recno )
			 continue;

		 tradeTreaty = ownNation->get_relation(townPtr->nation_recno)->trade_treaty || townPtr->nation_recno==nation_recno;

		 if( linked_town_enable_array[i] != (tradeTreaty ? LINK_EE : LINK_DD) )
			 toggle_town_link( i+1, tradeTreaty, COMMAND_AUTO, 1 );					// 1-toggle both side
	}
}
//------ End of function FirmMarket::update_trade_link -----//


//----- Begin of function FirmMarket::is_raw_market -----//
//
int FirmMarket::is_raw_market()
{
	if( restock_type == RESTOCK_RAW )
		return 1;
	if( restock_type == RESTOCK_ANY )
		return 1;
	return 0;
}
//------ End of function FirmMarket::is_raw_market -----//


//----- Begin of function FirmMarket::is_retail_market -----//
//
int FirmMarket::is_retail_market()
{
	if( restock_type == RESTOCK_PRODUCT )
		return 1;
	if( restock_type == RESTOCK_ANY )
		return 1;
	return 0;
}
//------ End of function FirmMarket::is_raw_market -----//


//----- Begin of function FirmMarket::switch_restock -----//
//
void FirmMarket::switch_restock()
{
	if( ++restock_type > RESTOCK_NONE )
		restock_type = RESTOCK_ANY;
	if( firm_array.selected_recno == firm_recno )
		info.disp();
}
//------ End of function FirmMarket::switch_restock -----//
