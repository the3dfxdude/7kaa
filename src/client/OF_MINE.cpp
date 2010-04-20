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

//Filename    : OF_MINE.CPP
//Description : Firm Mine

#include <OINFO.h>
#include <OVGA.h>
#include <OSTR.h>
#include <OFONT.h>
#include <ONEWS.h>
#include <OUNIT.h>
#include <ORACERES.h>
#include <OGAME.h>
#include <OWORLD.h>
#include <ONATION.h>
#include <OSITE.h>
#include <OF_MINE.h>
#include <OF_FACT.h>

//--------- Begin of function FirmMine::FirmMine ---------//
//
FirmMine::FirmMine()
{
	firm_skill_id = SKILL_MINING;

	cur_month_production = (float) 0;
	last_month_production = (float) 0;

	next_output_link_id	  = 0;
	next_output_firm_recno = 0;

	ai_should_build_factory_count = 0;
}
//----------- End of function FirmMine::FirmMine -----------//


//--------- Begin of function FirmMine::~FirmMine ---------//
//
FirmMine::~FirmMine()
{
	//------- update the site deposit reserve ------//

	if( site_recno )
	{
		site_array.untapped_raw_count++;

		if( reserve_qty==0 )		// if the reserve has been used up
		{
			site_array.del_site(site_recno);
		}
		else		// restore the site
		{
			Site* sitePtr = site_array[site_recno];

			sitePtr->reserve_qty = (int) reserve_qty;
			sitePtr->has_mine 	= 0;
		}
	}

	//-------- decrease AI raw count --------//

	if( raw_id )
	{
		nation_array[nation_recno]->raw_count_array[raw_id-1]--;

		err_when( nation_array[nation_recno]->raw_count_array[raw_id-1] < 0 );
	}
}
//----------- End of function FirmMine::~FirmMine -----------//


//--------- Begin of function FirmMine::init_derived ---------//
//
void FirmMine::init_derived()
{
	//---- scan for raw site in this firm's building location ----//

	Location* locPtr = scan_raw_site();

	if( locPtr )
	{
		site_recno  = locPtr->site_recno();
		raw_id		= site_array[site_recno]->object_id;
		reserve_qty = (float) site_array[site_recno]->reserve_qty;

		site_array[site_recno]->has_mine = 1;
		site_array.untapped_raw_count--;

		err_when( site_array.untapped_raw_count < 0 );
	}
	else
	{
		site_recno 	= 0;
		raw_id 		= 0;
		reserve_qty = (float) 0;
	}

	stock_qty 	 	= (float) 0;
	max_stock_qty  = (float) DEFAULT_MINE_MAX_STOCK_QTY;

	//-------- increase AI raw count --------//

	if( raw_id )
		nation_array[nation_recno]->raw_count_array[raw_id-1]++;
}
//----------- End of function FirmMine::init_derived -----------//


//------- Begin of function FirmMine::change_nation ---------//
//
void FirmMine::change_nation(int newNationRecno)
{
	if( raw_id )
	{
		nation_array[nation_recno]->raw_count_array[raw_id-1]--;

		err_when( nation_array[nation_recno]->raw_count_array[raw_id-1] < 0 );

		nation_array[newNationRecno]->raw_count_array[raw_id-1]++;
	}

	//-------- change the nation of this firm now ----------//

	Firm::change_nation(newNationRecno);
}
//-------- End of function FirmMine::change_nation ---------//


//--------- Begin of function FirmMine::scan_raw_site ---------//
//
Location* FirmMine::scan_raw_site()
{
	//---- scan for raw site in this firm's building location ----//

	int xLoc, yLoc;
	Location* locPtr;

	for( yLoc=loc_y1 ; yLoc<=loc_y2 ; yLoc++ )
	{
		for( xLoc=loc_x1 ; xLoc<=loc_x2 ; xLoc++ )
		{
			locPtr = world.get_loc(xLoc,yLoc);

			if( locPtr->has_site() &&
				 site_array[locPtr->site_recno()]->site_type == SITE_RAW )
			{
				return locPtr;
			}
		}
	}

	return NULL;
}
//--------- End of function FirmMine::scan_raw_site ---------//


//--------- Begin of function FirmMine::put_info ---------//
//
void FirmMine::put_info(int refreshFlag)
{
	disp_basic_info(INFO_Y1, refreshFlag);

	if( !should_show_info() )
		return;

	disp_mine_info(INFO_Y1+52, refreshFlag);
	disp_worker_list(INFO_Y1+127, refreshFlag);
	disp_worker_info(INFO_Y1+191, refreshFlag);

	//---------- display spy button ----------//

	disp_spy_button(INFO_X1, INFO_Y1+249, refreshFlag);
}
//----------- End of function FirmMine::put_info -----------//


//--------- Begin of function FirmMine::detect_info ---------//
//
void FirmMine::detect_info()
{
	//-------- detect basic info -----------//

	if( detect_basic_info() )
		return;

	//----------- detect worker -----------//

	if( detect_worker_list() )
	{
		disp_mine_info(INFO_Y1+52, INFO_UPDATE);
		disp_worker_info(INFO_Y1+191, INFO_UPDATE);
	}

	//-------- detect spy button ----------//

	detect_spy_button();

	if( !own_firm() )
		return;
}
//----------- End of function FirmMine::detect_info -----------//


//--------- Begin of function FirmMine::disp_mine_info ---------//
//
void FirmMine::disp_mine_info(int dispY1, int refreshFlag)
{
	//---------------- paint the panel --------------//

	if( refreshFlag == INFO_REPAINT )
		vga.d3_panel_up( INFO_X1, dispY1, INFO_X2, dispY1+70);

	//------ if there is no natural resource on this location ------//

	if( !raw_id )
	{
		font_san.center_put( INFO_X1, dispY1, INFO_X2, dispY1+70, "No Natural Resources" );
		return;
	}

	//-------------- display mining info -----------//

	int x=INFO_X1+4, y=dispY1+4;

	raw_res.put_small_raw_icon( x+1, y+1, raw_id );

	String str;

	str  = translate.process("Mining ");
	str += raw_res[raw_id]->name;

	font_san.disp( x+20, y, str, INFO_X2-2);
	y+=16;

	font_san.field( x, y, "Monthly Production", x+126, (int) production_30days(), 1, INFO_X2-2, refreshFlag, "MN_PROD");
	y+=16;

	str  = (int) stock_qty;
	str += " / ";
	str += (int) max_stock_qty;

	font_san.field( x, y, "Mined Stock", x+126, str, INFO_X2-2, refreshFlag, "MN_STOCK");
	y+=16;

	font_san.field( x, y, "Untapped Reserve", x+126, (int) reserve_qty, 1, INFO_X2-2, refreshFlag, "MN_UNTAP");
}
//----------- End of function FirmMine::disp_mine_info -----------//


//--------- Begin of function FirmMine::next_day ---------//
//
void FirmMine::next_day()
{
	//----- call next_day() of the base class -----//

	Firm::next_day();

	//----------- update population -------------//

	recruit_worker();

	//-------- train up the skill ------------//

	update_worker();

	//---------------------------------------//

	if( info.game_date%PROCESS_GOODS_INTERVAL == firm_recno%PROCESS_GOODS_INTERVAL )		// produce raw materials once every 3 days
	{
		produce_raw();
		set_next_output_firm();						// set next output firm
	}
}
//----------- End of function FirmMine::next_day -----------//


//--------- Begin of function FirmMine::next_month ---------//
//
void FirmMine::next_month()
{
	last_month_production = cur_month_production;
	cur_month_production  = (float) 0;
}
//----------- End of function FirmMine::next_month -----------//


//------- Begin of function FirmMine::set_next_output_firm ------//
//
// Set next_output_firm_recno, the recno of the linked firm
// to which this mine is going to output raw materials.
//
void FirmMine::set_next_output_firm()
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

			if( firmId==FIRM_FACTORY || firmId==FIRM_MARKET )
			{
				next_output_firm_recno = firmRecno;
				return;
			}
		}
	}

	next_output_firm_recno = 0;		// this mine has no linked output firms
}
//-------- End of function FirmMine::set_next_output_firm ---------//


//--------- Begin of function FirmMine::produce_raw ---------//
//
// Produce raw materials.
//
void FirmMine::produce_raw()
{
	//----- if stock capacity reached or reserve exhausted -----//

	if( stock_qty == max_stock_qty || reserve_qty==0 )
		return;

	err_when( reserve_qty < 0 );
	err_when( stock_qty > max_stock_qty );

	//------- calculate the productivity of the workers -----------//

	calc_productivity();

	//-------- mine raw materials -------//

	float produceQty = (float) 100 * productivity / 100;

	produceQty = MIN( produceQty, reserve_qty );
	produceQty = MIN( produceQty, max_stock_qty-stock_qty );

	reserve_qty -= produceQty;
	stock_qty	+= produceQty;

	cur_month_production += produceQty;

	site_array[site_recno]->reserve_qty = (int) reserve_qty;		// update the reserve_qty in site_array

	err_when( reserve_qty < 0 );
	err_when( stock_qty > max_stock_qty );

	//---- add news if run out of raw deposit ----//

	if( reserve_qty == 0 )
	{
		site_array.untapped_raw_count++;		// have to restore its first as del_site() will decrease uptapped_raw_count

		site_array.del_site(site_recno);
		site_recno = 0;

		if( nation_recno == nation_array.player_recno )
			news_array.raw_exhaust(raw_id, center_x, center_y);
	}
}
//----------- End of function FirmMine::produce_raw -----------//


//------- Begin of function FirmMine::draw -----------//
//
// Draw raw materials stocks.
//
void FirmMine::draw(int displayLayer)
{
	Firm::draw(displayLayer);

	if( !should_show_info() )
		return;

	if( under_construction )
		return;

	if( raw_id && displayLayer == 1)
	{
		int cargoCount = MAX_CARGO	* (int)stock_qty / (int)max_stock_qty;

		draw_cargo( MAX(1,cargoCount), raw_res.small_raw_icon(raw_id) );
	}
}
//--------- End of function FirmMine::draw -----------//

