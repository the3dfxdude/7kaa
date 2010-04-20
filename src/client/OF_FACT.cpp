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

//Filename    : OF_FACT.CPP
//Description : Firm Factory

#include <OINFO.h>
#include <OVGA.h>
#include <OSTR.h>
#include <OUNIT.h>
#include <OGAME.h>
#include <OFONT.h>
#include <OMOUSE.h>
#include <OBUTT3D.h>
#include <ONATION.h>
#include <ORAWRES.h>
#include <ORACERES.h>
#include <OTOWNRES.h>
#include <OWORLD.h>
#include <OF_MINE.h>
#include <OF_MARK.h>
#include <OF_FACT.h>
#ifdef USE_DPLAY
#include <OREMOTE.h>
#endif
#include <OSE.h>


//-------- define constant ---------//

#define DEFAULT_FACTORY_MAX_STOCK_QTY 		500
#define DEFAULT_FACTORY_MAX_RAW_STOCK_QTY 500

//------- define static vars -------//

static Button3D	button_change_production;

//--------- Begin of function FirmFactory::FirmFactory ---------//
//
FirmFactory::FirmFactory()
{
	firm_skill_id  = SKILL_MFT;
	product_raw_id = 1;

	cur_month_production = (float) 0;
	last_month_production = (float) 0;

	stock_qty	   = (float) 0;
	max_stock_qty  = (float) DEFAULT_FACTORY_MAX_STOCK_QTY;

	raw_stock_qty		= (float) 0;
	max_raw_stock_qty = (float) DEFAULT_FACTORY_MAX_RAW_STOCK_QTY;

   next_output_link_id	  = 0;
	next_output_firm_recno = 0;
}
//----------- End of function FirmFactory::FirmFactory -----------//


//--------- Begin of function FirmFactory::~FirmFactory ---------//
//
FirmFactory::~FirmFactory()
{
}
//----------- End of function FirmFactory::~FirmFactory -----------//


//--------- Begin of function FirmFactory::init_derived ---------//
//
void FirmFactory::init_derived()
{
	auto_set_product();
}
//----------- End of function FirmFactory::init_derived -----------//


//------- Begin of function FirmFactory::auto_set_product --------//
//
void FirmFactory::auto_set_product()
{
	//---- automatically set the factory product type -----//

	int 			i, j, k, rawId, firmDistance;
	Firm* 		firmPtr, *otherFirm;
	FirmMarket* firmMarket;
	int 			minDistance=0x7FFF;

	for( i=0 ; i<linked_firm_count ; i++ )
	{
		firmPtr = firm_array[linked_firm_array[i]];

		firmDistance = m.points_distance( firmPtr->center_x, firmPtr->center_y,
													 center_x, center_y );

		//----------- if the firm is a mine ----------//

		if( firmPtr->firm_id == FIRM_MINE )
		{
			rawId = ((FirmMine*)firmPtr)->raw_id;

			if( !rawId )
				continue;

			//--- if this mine hasn't been used by any factories yet, then select it ---//

			for( j=firmPtr->linked_firm_count-1 ; j>=0 ; j-- )
			{
				if( !firmPtr->linked_firm_enable_array[j] )
					continue;

				otherFirm = firm_array[ firmPtr->linked_firm_array[j] ];

				if( otherFirm->firm_id == FIRM_FACTORY &&
					 ((FirmFactory*)otherFirm)->product_raw_id == rawId )
				{
					break;
				}

			}

			if( j<0 )
			{
				product_raw_id = rawId;
				return;
			}

			//--------------------------------//

			if( firmDistance < minDistance )
			{
				product_raw_id = ((FirmMine*)firmPtr)->raw_id;
				minDistance    = firmDistance;
			}
		}

		//----------- if the firm is a market place ----------//

		else if( firmPtr->firm_id == FIRM_MARKET )
		{
			firmMarket = (FirmMarket*) firmPtr;

			for( j=0 ; j<MAX_MARKET_GOODS ; j++ )
			{
				rawId = firmMarket->market_goods_array[j].raw_id;

				if( !rawId )
					continue;

				//--- if this raw material in this market hasn't been used by any factories yet, then select it ---//

				for( k=firmPtr->linked_firm_count-1 ; k>=0 ; k-- )
				{
					if( firmPtr->linked_firm_enable_array[k] != LINK_EE )
						continue;

					otherFirm = firm_array[ firmPtr->linked_firm_array[k] ];

					if( otherFirm->firm_id == FIRM_FACTORY &&
						 ((FirmFactory*)otherFirm)->product_raw_id == rawId )
					{
						break;
					}
				}

				if( k<0 )
				{
					product_raw_id = rawId;
					return;
				}

				//-----------------------------------//

				if( firmDistance < minDistance )
				{
					product_raw_id = rawId;
					minDistance    = firmDistance;
				}
			}
		}
	}
}
//------- End of function FirmFactory::auto_set_product -------//


//--------- Begin of function FirmFactory::put_info ---------//
//
void FirmFactory::put_info(int refreshFlag)
{
	//---------- display info ------------//

	disp_basic_info(INFO_Y1, refreshFlag);

	if( !should_show_info() )
		return;

	disp_factory_info(INFO_Y1+54, refreshFlag);
	disp_worker_list(INFO_Y1+126, refreshFlag);
	disp_worker_info(INFO_Y1+190, refreshFlag);

	//------ display button -------//

	int x;

	if( own_firm() && refreshFlag==INFO_REPAINT )
	{
		button_change_production.paint( INFO_X1, INFO_Y1+248, 'A', "CHGPROD" );
		x = INFO_X1+BUTTON_ACTION_WIDTH;
	}
	else
		x = INFO_X1;

	//---------- display spy button ----------//

	disp_spy_button(x, INFO_Y1+248, refreshFlag);
}
//----------- End of function FirmFactory::put_info -----------//


//--------- Begin of function FirmFactory::detect_info ---------//
//
void FirmFactory::detect_info()
{
	//-------- detect basic info -----------//

	if( detect_basic_info() )
		return;

	//-------- detect workers ----------//

	if( detect_worker_list() )		// detect this when: it's the player's firm or the player has spies in this firm
	{
		disp_worker_list(INFO_Y1+126, INFO_UPDATE);
		disp_worker_info(INFO_Y1+190, INFO_UPDATE);
	}

	//-------- detect spy button ----------//

	detect_spy_button();

	if( !own_firm() )
		return;

	//---- detect change production button -----//

	if( button_change_production.detect() )
	{	
		change_production();
		disp_factory_info(INFO_Y1+54, INFO_UPDATE);
		// ##### begin Gilbert 25/9 ######//
		se_ctrl.immediate_sound("TURN_ON");
		// ##### end Gilbert 25/9 ######//
	}
}
//----------- End of function FirmFactory::detect_info -----------//


//--------- Begin of function FirmFactory::next_day ---------//
//
void FirmFactory::next_day()
{
	//----- call next_day() of the base class -----//

	Firm::next_day();

	//----------- update population -------------//

	recruit_worker();

	//-------- train up the skill ------------//

	update_worker();

	//--------- daily manufacturing activities ---------//

	if( info.game_date%PROCESS_GOODS_INTERVAL == firm_recno%PROCESS_GOODS_INTERVAL )
	{
		input_raw();
		production();
		set_next_output_firm();						// set next output firm
	}
}
//----------- End of function FirmFactory::next_day -----------//


//--------- Begin of function FirmFactory::next_month ---------//
//
void FirmFactory::next_month()
{
	last_month_production = cur_month_production;
	cur_month_production  = (float) 0;
}
//----------- End of function FirmFactory::next_month -----------//


//------- Begin of function FirmFactory::draw -----------//
//
// Draw product stocks.
//
void FirmFactory::draw(int displayLayer)
{
	Firm::draw(displayLayer);

	if( !should_show_info() )
		return;

	if( under_construction )
		return;

	if( product_raw_id && displayLayer == 1 )
	{
		int cargoCount = MAX_CARGO * (int)stock_qty / (int)max_stock_qty;

		draw_cargo( MAX(1,cargoCount), raw_res.small_product_icon(product_raw_id) );
	}
}
//--------- End of function FirmFactory::draw -----------//


//--------- Begin of function FirmFactory::disp_factory_info ---------//
//
void FirmFactory::disp_factory_info(int dispY1, int refreshFlag)
{
	//---------------- paint the panel --------------//

	if( refreshFlag == INFO_REPAINT )
		vga.d3_panel_up( INFO_X1, dispY1, INFO_X2, dispY1+70);

	//---------- display production info -------------//

	int x=INFO_X1+4, y=dispY1+4;

	vga_front.put_bitmap_trans( x+1, y+1, raw_res.small_product_icon(product_raw_id) );

	String str;

	str  = translate.process("Producing ");
#if(defined(FRENCH))
	char productName[20];
	strcpy(productName, raw_res[product_raw_id]->name);
	strcat(productName, " Products");
	str += translate.process(productName);
#else
	str += raw_res[product_raw_id]->name;
	str += translate.process(" Products");
#endif

	font_san.use_max_height();							// make sure the old text is replaced completely
#if(defined(FRENCH) || defined(SPANISH))
	font_san.disp( x+15, y, str, INFO_X2-1);
#else
	font_san.disp( x+20, y, str, INFO_X2-2);
#endif
	font_san.use_std_height();

	y+=16;

	font_san.field( x, y, "Monthly Production", x+133, (int) production_30days(), 1, INFO_X2-2, refreshFlag, "FC_PROD" );
	y+=16;

	str  = (int) raw_stock_qty;
	str += " / ";
	str += (int) max_raw_stock_qty;
	font_san.field( x, y, "Raw Material Stock", x+133, str, INFO_X2-2, refreshFlag, "FC_RAW" );
	y+=16;

	str  = (int) stock_qty;
	str += " / ";
	str += (int) max_stock_qty;

	font_san.field( x, y, "Product Stock", x+133, str, INFO_X2-2, refreshFlag, "FC_PROD");
}
//----------- End of function FirmFactory::disp_factory_info -----------//


//------ Begin of function FirmFactory::change_production -------//
//
void FirmFactory::change_production()
{
#ifdef USE_DPLAY
	if( remote.is_enable() )
	{
		// packet structure : <firm recno> <product id>
		short *shortPtr = (short *)remote.new_send_queue_msg(MSG_F_FACTORY_CHG_PROD, 2*sizeof(short) );
		shortPtr[0] = firm_recno;
		shortPtr[1] = product_raw_id >= MAX_PRODUCT ? 1 : product_raw_id + 1;
	}
	else
#endif
	{
		// update RemoteMsg::factory_change_product
		if( ++product_raw_id > MAX_PRODUCT )
			product_raw_id = 1;

		set_production( product_raw_id==MAX_PRODUCT ? 1 : product_raw_id+1 );
	}
}
//--------- End of function FirmFactory::change_production --------//


//------ Begin of function FirmFactory::set_production -------//
//
void FirmFactory::set_production(int newProductId)
{
	product_raw_id = newProductId;

	stock_qty 	   = (float) 0;
	max_stock_qty  = (float) DEFAULT_FACTORY_MAX_STOCK_QTY;
	raw_stock_qty		= (float) 0;
	max_raw_stock_qty = (float) DEFAULT_FACTORY_MAX_RAW_STOCK_QTY;
}
//--------- End of function FirmFactory::set_production --------//


//--------- Begin of function FirmFactory::production ---------//
//
void FirmFactory::production()
{
	//----- if stock capacity reached or reserve exhausted -----//

	if( stock_qty == max_stock_qty )
		return;

	err_when( stock_qty > max_stock_qty );

	//------- calculate the productivity of the workers -----------//

	calc_productivity();

	//------- generate revenue for the nation --------//

	float produceQty = (float) 20 * productivity / 100;

	produceQty = MIN( produceQty, max_stock_qty-stock_qty );

	manufacture(produceQty);
}
//----------- End of function FirmFactory::production -----------//


//--------- Begin of function FirmFactory::input_raw ---------//
//
// Input raw materials from mines and market places.
//
void FirmFactory::input_raw()
{
	//------ scan for a firm to input raw materials --------//

	int   	 i, j;
	float		 inputQty;
	Firm* 	 firmPtr;
	FirmMine* firmMine;
	FirmMarket* firmMarket;
	Nation* 	 nationPtr = nation_array[nation_recno];
	MarketGoods* marketGoods;

	for( i=0 ; i<linked_firm_count ; i++ )
	{
		if( linked_firm_enable_array[i] != LINK_EE )
			continue;

		firmPtr = firm_array[linked_firm_array[i]];

		//----------- check if the firm is a mine ----------//

		if( firmPtr->firm_id != FIRM_MINE && firmPtr->firm_id != FIRM_MARKET )
			continue;

		//--------- if the firm is a mine ------------//

		if( firmPtr->firm_id == FIRM_MINE )
		{
			firmMine = (FirmMine*) firmPtr;

			if( firmMine->next_output_firm_recno == firm_recno &&
				 firmMine->raw_id==product_raw_id && firmMine->stock_qty > 0 )
			{
				inputQty = MIN( firmMine->stock_qty, max_raw_stock_qty - raw_stock_qty );

				if( firmMine->nation_recno != nation_recno )			// make sure it has the cash to pay for the raw materials
					inputQty = MIN( inputQty, nationPtr->cash/RAW_PRICE );

				if( inputQty > 0 )
				{
					firmMine->stock_qty  -= inputQty;
					raw_stock_qty			+= inputQty;

					err_when( raw_stock_qty > max_raw_stock_qty );

					//---- import from other nation -----//

					if( firmMine->nation_recno != nation_recno )
						nationPtr->import_goods(IMPORT_RAW, firmMine->nation_recno, inputQty*RAW_PRICE );
				}
			}
		}

		//------- if the firm is a market place --------//

		if( firmPtr->firm_id == FIRM_MARKET )
		{
			firmMarket = (FirmMarket*) firmPtr;

			if( firmMarket->next_output_firm_recno == firm_recno )
			{
				marketGoods = firmMarket->market_goods_array;

				for( j=0 ; j<MAX_MARKET_GOODS ; j++, marketGoods++ )
				{
					if( marketGoods->raw_id == product_raw_id &&
						 marketGoods->stock_qty > 0 )
					{
						inputQty = MIN( marketGoods->stock_qty, max_raw_stock_qty - raw_stock_qty );

						if( firmMarket->nation_recno != nation_recno )			// make sure it has the cash to pay for the raw materials
							inputQty = MIN( inputQty, nationPtr->cash/RAW_PRICE );

						if( inputQty > 0 )
						{
							marketGoods->stock_qty -= inputQty;
							raw_stock_qty			  += inputQty;

							err_when( raw_stock_qty > max_raw_stock_qty );

							//---- import from other nation -----//

							if( firmMarket->nation_recno != nation_recno )
								nationPtr->import_goods(IMPORT_RAW, firmMarket->nation_recno, inputQty*RAW_PRICE );
						}
					}
				}
			}
		}
	}
}
//----------- End of function FirmFactory::input_raw -----------//


//--------- Begin of function FirmFactory::manufacture ---------//
//
// Input raw materials into the factory
//
// <float> maxMftQty - maximum qty this firm can manufacture in this call.
//
void FirmFactory::manufacture(float maxMftQty)
{
	if( raw_stock_qty==0 )
		return;

	float	inputQty;

	inputQty = MIN(raw_stock_qty, maxMftQty);
	inputQty = MIN(inputQty, max_stock_qty-stock_qty);

	if( inputQty <= 0 )
		return;

	raw_stock_qty			-= inputQty;
	stock_qty			   += inputQty;
	cur_month_production += inputQty;
}
//----------- End of function FirmFactory::manufacture -----------//


//------- Begin of function FirmFactory::set_next_output_firm ------//
//
// Set next_output_firm_recno, the recno of the linked firm
// to which this factory is going to output products.
//
void FirmFactory::set_next_output_firm()
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

			if( firmId==FIRM_MARKET )
			{
				next_output_firm_recno = firmRecno;
				return;
			}
		}
	}

	next_output_firm_recno = 0;		// this mine has no linked output firms
}
//-------- End of function FirmFactory::set_next_output_firm ---------//


