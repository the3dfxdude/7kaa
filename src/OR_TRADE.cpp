/*
 * Seven Kingdoms: Ancient Adversaries
 *
 * Copyright 1997,1998 Enlight Software Ltd.
 * Copyright 2020 Jesse Allen
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

//Filename    : OR_TRADE.CPP
//Description : Trade Report

#include <OVGA.h>
#include <OSTR.h>
#include <OFONT.h>
#include <OIMGRES.h>
#include <OVBROWIF.h>
#include <OBUTTON.h>
#include <ORACERES.h>
#include <OFIRM.h>
#include <OWORLD.h>
#include <ONATION.h>
#include <OU_CARA.h>
#include <OU_MARI.h>
#include <OINFO.h>
#include "gettext.h"
#include <vga_util.h>
#include <OMOUSE.h>
#include <OF_FACT.h>
#include <OF_MINE.h>
#include <OF_HARB.h>
#include <OPOWER.h>
#include <OSERES.h>

//------------- Define coordinations -----------//

enum { CARAVAN_BROWSE_X1 = ZOOM_X1+6,
		 CARAVAN_BROWSE_Y1 = ZOOM_Y1+6,
		 CARAVAN_BROWSE_X2 = ZOOM_X2-6,
		 CARAVAN_BROWSE_Y2 = CARAVAN_BROWSE_Y1+220
	  };

enum { REPORT_BUTTON_X1 = ZOOM_X1+8,
		 REPORT_BUTTON_Y1 = CARAVAN_BROWSE_Y2+4,
		 REPORT_BUTTON_WIDTH = 90,
		 REPORT_BUTTON_Y2 = REPORT_BUTTON_Y1+20,
		 REPORT_BUTTON_X_SPACE = 130,
	};

enum { FIRM_BROWSE_X1 = ZOOM_X1+6,
		 FIRM_BROWSE_Y1 = REPORT_BUTTON_Y2+4,
		 FIRM_BROWSE_X2 = ZOOM_X2-6,
		 FIRM_BROWSE_Y2 = ZOOM_Y2-6,
	  };

//----------- Define static variables ----------//

static VBrowseIF browse_caravan, browse_ship, browse_firm;
static char mode_unit, mode_firm;
static short selected_unit_recno, selected_harbor_recno, unit_x, unit_y, browse_firm_recno, idle_caravans, idle_firms;

#define MAX_FIRM_REPORT_MODE 4
static const char* firm_mode_str_array[MAX_FIRM_REPORT_MODE] =
{
	N_("Market"),
	N_("Factory"),
	N_("Mine"),
	N_("Any"),
};

#define MAX_UNIT_REPORT_MODE 2
static const char* unit_mode_str_array[MAX_UNIT_REPORT_MODE] =
{
	N_("Caravan"),
	N_("Ship"),
};

enum { BROWSE_CARAVAN = 0,
		BROWSE_SHIP = 1,
	};

enum { BROWSE_MARKET = 0,
		BROWSE_FACTORY = 1,
		BROWSE_MINE = 2,
		BROWSE_ANY = 3,
	};

//----------- Define static functions ----------//

static int  is_caravan_route_idle(UnitCaravan* unitPtr);
static int  is_firm_idle(Firm* firmPtr);
static void create_caravan_list();
static void create_firm_list();
static void create_ship_list();
static void put_caravan_rec(int recNo, int x, int y, int refreshFlag);
static void put_firm_rec(int recNo, int x, int y, int refreshFlag);
static void put_ship_rec(int recNo, int x, int y, int refreshFlag);
static int  detect_button();
static void disp_button();
static void	disp_total();
static void put_stop_info(int x, int y, TradeStop* tradeStop);

static int  sort_firm( const void *a, const void *b );
static int  sort_unit( const void *a, const void *b );

//--------- Begin of function Info::disp_trade ---------//
//
void Info::disp_trade(int refreshFlag)
{
	if( mode_unit == BROWSE_CARAVAN )
	{
		create_caravan_list();
		browse_caravan_recno = browse_caravan.recno();
	}
	else
	{
		create_ship_list();
		browse_ship_recno = browse_ship.recno();
	}

	//-------- display the caravan browser --------//

	int x=CARAVAN_BROWSE_X1+9;
	int y=CARAVAN_BROWSE_Y1+4;

	vga_back.d3_panel_up(CARAVAN_BROWSE_X1, CARAVAN_BROWSE_Y1, CARAVAN_BROWSE_X2, CARAVAN_BROWSE_Y1+20 );

	if( mode_unit == BROWSE_CARAVAN )
		font_san.put( x	 , y, _("Caravan") );
	else
		font_san.put( x  , y, _("Ship") );
#if(defined(FRENCH))
	font_san.put( x+75 , y, "Hit Points" );
#else
	font_san.put( x+90 , y, _("Hit Points") );
#endif
	font_san.put( x+160, y, _("Stop 1") );
	font_san.put( x+220, y, _("Stop 2") );
	font_san.put( x+280, y, _("Stop 3") );
	font_san.put( x+340, y, _("Goods Carried") );

	if( refreshFlag == INFO_REPAINT )
	{
		if( mode_unit == BROWSE_CARAVAN )
		{
			browse_caravan.init( CARAVAN_BROWSE_X1, CARAVAN_BROWSE_Y1+22, CARAVAN_BROWSE_X2, CARAVAN_BROWSE_Y2-20,
									0, 16, report_array.size(), put_caravan_rec, 1 );
			browse_caravan.open(browse_caravan_recno);
		}
		else
		{
			browse_ship.init( CARAVAN_BROWSE_X1, CARAVAN_BROWSE_Y1+22, CARAVAN_BROWSE_X2, CARAVAN_BROWSE_Y2-20,
									0, 16, report_array.size(), put_ship_rec, 1 );
			browse_ship.open(browse_ship_recno);
		}

	}
	else
	{
		if( mode_unit == BROWSE_CARAVAN )
		{
			if( !browse_caravan.init_flag )
				browse_caravan.init( CARAVAN_BROWSE_X1, CARAVAN_BROWSE_Y1+22, CARAVAN_BROWSE_X2, CARAVAN_BROWSE_Y2-20,
									0, 16, report_array.size(), put_ship_rec, 1 );
			else
				browse_caravan.paint();
			browse_caravan.open(browse_caravan_recno, report_array.size());
		}
		else
		{
			if( !browse_ship.init_flag )
				browse_ship.init( CARAVAN_BROWSE_X1, CARAVAN_BROWSE_Y1+22, CARAVAN_BROWSE_X2, CARAVAN_BROWSE_Y2-20,
									0, 16, report_array.size(), put_ship_rec, 1 );
			else
				browse_ship.paint();
			browse_ship.open(browse_ship_recno, report_array.size());
		}
	}

	//-------- display the firm browser ---------//

	create_firm_list();

	x=FIRM_BROWSE_X1+9;
	y=FIRM_BROWSE_Y1+4;

	vga_back.d3_panel_up(FIRM_BROWSE_X1, FIRM_BROWSE_Y1, FIRM_BROWSE_X2, FIRM_BROWSE_Y1+20 );

	font_san.put( x	 , y, _("Location") );
	font_san.put( x+185, y, _("Stock/Sales/Demand") );
	if( selected_unit_recno )
		font_san.put( x+350, y, _("(In Range of Unit)") );

	if( refreshFlag == INFO_REPAINT )
	{
		browse_firm.init( FIRM_BROWSE_X1, FIRM_BROWSE_Y1+22, FIRM_BROWSE_X2, FIRM_BROWSE_Y2-20,
								0, 16, report_array2.size(), put_firm_rec, 1 );

		browse_firm.open(browse_firm_recno);
	}
	else
	{
		browse_firm.paint();
		browse_firm.open(browse_firm_recno, report_array2.size());
	}

	//------------ display total -------------//

	disp_total();

	//------------ display buttons -------------//

	disp_button();
}
//----------- End of function Info::disp_trade -----------//


//--------- Begin of function Info::detect_trade ---------//
//
void Info::detect_trade()
{
	//-------- detect the caravan browser ---------//

	if( mode_unit == BROWSE_CARAVAN && browse_caravan.detect() )
	{
		browse_caravan_recno = browse_caravan.recno();

		if( browse_caravan.double_click )
		{
			Unit* unitPtr = unit_array[ get_report_data(browse_caravan_recno) ];

			world.go_loc(unitPtr->next_x_loc(), unitPtr->next_y_loc(), 1);
		}
	}

	else if( mode_unit == BROWSE_CARAVAN && browse_caravan.detect_right() )
	{
		browse_caravan_recno = browse_caravan.recno();

		if( unit_array.selected_recno && unit_array[unit_array.selected_recno]->unit_id == UNIT_CARAVAN )
		{
			UnitCaravan* unitPtr = (UnitCaravan*) unit_array[ get_report_data(browse_caravan_recno) ];
			if( unitPtr->nation_recno == nation_array.player_recno )
				unitPtr->copy_route(unit_array.selected_recno, COMMAND_PLAYER);
		}
	}

	//-------- detect the ship browser ---------//

	if( mode_unit == BROWSE_SHIP && browse_ship.detect() )
	{
		browse_ship_recno = browse_ship.recno();

		if( browse_ship.double_click )
		{
			Unit* unitPtr = unit_array[ get_report_data(browse_ship_recno) ];

			world.go_loc(unitPtr->next_x_loc(), unitPtr->next_y_loc(), 1);
		}
	}

	//-------- detect the firm browser ---------//

	if( browse_firm.detect() )
	{
		browse_firm_recno = browse_firm.recno();

		if( power.command_id == COMMAND_SET_CARAVAN_STOP &&
			unit_array[power.command_unit_recno]->is_visible() )
		{
			Firm* firmPtr = firm_array[ get_report_data2(browse_firm_recno) ];
			UnitCaravan* unitPtr = (UnitCaravan*) unit_array[power.command_unit_recno];
			if( unitPtr->can_set_stop(firmPtr->firm_recno) )
			{
				if( se_res.mark_command_time() )
				{
					se_res.far_sound( unitPtr->cur_x_loc(), unitPtr->cur_y_loc(), 1,
						'S', unitPtr->sprite_id, "ACK");
				}
				unitPtr->set_stop(power.command_para, firmPtr->center_x, firmPtr->center_y, COMMAND_PLAYER);              // command_para is the id. of the stop
			}
			power.command_id = 0;
		}

		else if( power.command_id == COMMAND_SET_SHIP_STOP &&
			unit_array[power.command_unit_recno]->is_visible() )
		{
			Firm* firmPtr = firm_array[ get_report_data2(browse_firm_recno) ];
			UnitMarine* unitPtr = (UnitMarine*) unit_array[power.command_unit_recno];
			if( unitPtr->can_set_stop(firmPtr->firm_recno) )
			{
				if( se_res.mark_command_time() )
				{
					se_res.far_sound( unitPtr->cur_x_loc(), unitPtr->cur_y_loc(), 1,
						'S', unitPtr->sprite_id, "ACK");
				}
				unitPtr->set_stop(power.command_para, firmPtr->center_x, firmPtr->center_y, COMMAND_PLAYER);              // command_para is the id. of the stop
			}
			power.command_id = 0;
		}

		else if( browse_firm.double_click )
		{
			Firm* firmPtr = firm_array[ get_report_data2(browse_firm_recno) ];

			world.go_loc(firmPtr->loc_x1, firmPtr->loc_y1, 1);
		}
	}

	//------- detect report buttons --------//

	if( detect_button() )
		return;
}
//----------- End of function Info::detect_trade -----------//


//--------- Begin of static function detect_button ---------//
//
static int detect_button()
{
	int x=REPORT_BUTTON_X1;

	for( int i=0 ; i<MAX_FIRM_REPORT_MODE ; i++ )
	{
		//-----------------------------------------//

		if( mouse.single_click( x, REPORT_BUTTON_Y1, x+REPORT_BUTTON_WIDTH-1, REPORT_BUTTON_Y2 ) )
		{
			mode_firm = i;
			return 1;
		}

		x+=REPORT_BUTTON_WIDTH;
	}

	x+=10;

	for( int i=0 ; i<MAX_UNIT_REPORT_MODE ; i++ )
	{
		//-----------------------------------------//

		if( mouse.single_click( x, REPORT_BUTTON_Y1, x+REPORT_BUTTON_WIDTH-1, REPORT_BUTTON_Y2 ) )
		{
			mode_unit = i;
			return 1;
		}

		x+=REPORT_BUTTON_WIDTH;
	}

	return 0;
}
//----------- End of static function detect_button -----------//


//--------- Begin of static function disp_button ---------//
//
static void disp_button()
{
	int x=REPORT_BUTTON_X1;

	for( int i=0 ; i<MAX_FIRM_REPORT_MODE ; i++ )
	{
		//-----------------------------------------//

		if( mode_firm == i )
			vga_util.d3_panel_down( x, REPORT_BUTTON_Y1, x+REPORT_BUTTON_WIDTH-1, REPORT_BUTTON_Y2 );
		else
			vga_util.d3_panel_up( x, REPORT_BUTTON_Y1, x+REPORT_BUTTON_WIDTH-1, REPORT_BUTTON_Y2 );

		font_san.center_put( x, REPORT_BUTTON_Y1, x+REPORT_BUTTON_WIDTH-1, REPORT_BUTTON_Y2, _(firm_mode_str_array[i]) );

		x+=REPORT_BUTTON_WIDTH;
	}

	x+=10;

	for( int i=0 ; i<MAX_UNIT_REPORT_MODE ; i++ )
	{
		//-----------------------------------------//

		if( mode_unit == i )
			vga_util.d3_panel_down( x, REPORT_BUTTON_Y1, x+REPORT_BUTTON_WIDTH-1, REPORT_BUTTON_Y2 );
		else
			vga_util.d3_panel_up( x, REPORT_BUTTON_Y1, x+REPORT_BUTTON_WIDTH-1, REPORT_BUTTON_Y2 );

		font_san.center_put( x, REPORT_BUTTON_Y1, x+REPORT_BUTTON_WIDTH-1, REPORT_BUTTON_Y2, _(unit_mode_str_array[i]) );

		x+=REPORT_BUTTON_WIDTH;
	}
}
//----------- End of static function disp_button -----------//


//-------- Begin of static function disp_total --------//

static void disp_total()
{
	//------- display caravan total --------//

	int x=CARAVAN_BROWSE_X1+9;
	int y=CARAVAN_BROWSE_Y2-16;

	vga_back.d3_panel_up(CARAVAN_BROWSE_X1, CARAVAN_BROWSE_Y2-18, CARAVAN_BROWSE_X2, CARAVAN_BROWSE_Y2 );

	String str;

	if( mode_unit == BROWSE_CARAVAN )
		snprintf( str, MAX_STR_LEN+1, _("Total Caravans: %s"), misc.format(info.report_array.size()) );
	else
		snprintf( str, MAX_STR_LEN+1, _("Total Ships: %s"), misc.format(info.report_array.size()) );

	font_san.put( x, y, str );

	if( mode_unit == BROWSE_CARAVAN )
	{
		snprintf( str, MAX_STR_LEN+1, _("*Idle Caravans: %s"), misc.format(idle_caravans) );
		font_san.put( x+200, y, str );
	}

	//-------- display firm total --------//

	x=FIRM_BROWSE_X1+9;
	y=FIRM_BROWSE_Y2-16;

	vga_back.d3_panel_up(FIRM_BROWSE_X1, FIRM_BROWSE_Y2-18, FIRM_BROWSE_X2, FIRM_BROWSE_Y2 );

	snprintf( str, MAX_STR_LEN+1, _("Total Locations: %s"), misc.format(info.report_array2.size()) );

	font_san.put( x, y, str );

	snprintf( str, MAX_STR_LEN+1, _("*Idle Locations: %s"), misc.format(idle_firms) );
	font_san.put( x+200, y, str );
}
//----------- End of static function disp_total -----------//


//-------- Begin of static function create_caravan_list --------//
//
static void create_caravan_list()
{
	int   totalUnit = unit_array.size();
	Unit* unitPtr;

	info.report_array.zap();
	idle_caravans = 0;

	for( short unitRecno=1 ; unitRecno<=totalUnit ; unitRecno++ )
	{
		if( unit_array.is_deleted(unitRecno) )
			continue;

		unitPtr = unit_array[unitRecno];

		if( unitPtr->nation_recno == info.viewing_nation_recno &&
			 unitPtr->unit_id == UNIT_CARAVAN )
		{
			info.report_array.linkin(&unitRecno);
			if( is_caravan_route_idle((UnitCaravan*)unitPtr) )
				idle_caravans++;
		}
	}

	info.report_array.quick_sort(sort_unit);
}
//----------- End of static function create_caravan_list -----------//


//-------- Begin of static function create_firm_list --------//
//
static void create_firm_list()
{
	int   totalFirm = firm_array.size();
	Firm* firmPtr;

	info.report_array2.zap();
	idle_firms = 0;

        if( !nation_array.player_recno )
		return;

	int regionId, viewHarbor = 0;
	short unitRecno = 0;

        if( unit_array.selected_recno )
        {
                Unit* unitPtr = unit_array[unit_array.selected_recno];

		if( unitPtr->nation_recno == nation_array.player_recno && unitPtr->unit_id == UNIT_CARAVAN )
		{
			unitRecno = unit_array.selected_recno;
		}

		else if( unitPtr->nation_recno == nation_array.player_recno && unit_res[unitPtr->unit_id]->carry_goods_capacity > 0 )
		{
			unitRecno = unit_array.selected_recno;
			viewHarbor = 1;
		}
        }

	else if( firm_array.selected_recno )
	{
		Firm* firmPtr = firm_array[firm_array.selected_recno];

		if( firmPtr->nation_recno == nation_array.player_recno && firmPtr->firm_id == FIRM_HARBOR )
		{
			viewHarbor = 1;
		}
	}

	if( unitRecno )
	{
		Unit* unitPtr = unit_array[unitRecno];
		unit_x = unitPtr->next_x_loc();
		unit_y = unitPtr->next_y_loc();
		regionId = world.get_region_id(unit_x, unit_y);
		selected_unit_recno = unitRecno;
	}
	else
	{
		unit_x = -1;
		unit_y = -1;
		selected_unit_recno = 0;
	}

	for( short firmRecno=1 ; firmRecno<=totalFirm ; firmRecno++ )
	{
		if( firm_array.is_deleted(firmRecno) )
			continue;

		firmPtr = firm_array[firmRecno];

		if( !viewHarbor && firmPtr->firm_id == FIRM_MARKET )
		{
			if( unitRecno && firmPtr->region_id != regionId )
				continue;

			if( mode_firm != BROWSE_MARKET && mode_firm != BROWSE_ANY )
				continue;

			if( !nation_array[firmPtr->nation_recno]->get_relation(nation_array.player_recno)->trade_treaty )
				continue;
		}

		else if( !viewHarbor && firmPtr->firm_id == FIRM_FACTORY )
		{
			if( unitRecno && firmPtr->region_id != regionId )
				continue;

			if( firmPtr->nation_recno != nation_array.player_recno )
				continue;

			if( mode_firm != BROWSE_FACTORY && mode_firm != BROWSE_ANY )
				continue;
		}

		else if( !viewHarbor && firmPtr->firm_id == FIRM_MINE )
		{
			if( unitRecno && firmPtr->region_id != regionId )
				continue;

			if( firmPtr->nation_recno != nation_array.player_recno )
				continue;

			if( mode_firm != BROWSE_MINE && mode_firm != BROWSE_ANY )
				continue;
		}

		else if( viewHarbor && firmPtr->firm_id == FIRM_HARBOR )
		{
			if( firmPtr->nation_recno != nation_array.player_recno )
				continue;

			FirmHarbor* firmHarbor = firmPtr->cast_to_FirmHarbor();

			if( unitRecno && firmHarbor->sea_region_id != regionId )
				continue;

			if( mode_firm == BROWSE_MARKET && !firmHarbor->linked_market_num )
				continue;
			else if( mode_firm == BROWSE_FACTORY && !firmHarbor->linked_factory_num )
				continue;
			else if( mode_firm == BROWSE_MINE && !firmHarbor->linked_mine_num )
				continue;
		}

		else
		{
			continue;
		}

		info.report_array2.linkin(&firmRecno);
		if( is_firm_idle((Firm*)firmPtr) )
			idle_firms++;
	}

	info.report_array2.quick_sort(sort_firm);
}
//----------- End of static function create_firm_list -----------//


//-------- Begin of static function create_ship_list --------//
//
static void create_ship_list()
{
	int   totalUnit = unit_array.size();
	Unit* unitPtr;

	info.report_array.zap();

	for( short unitRecno=1 ; unitRecno<=totalUnit ; unitRecno++ )
	{
		if( unit_array.is_deleted(unitRecno) )
			continue;

		unitPtr = unit_array[unitRecno];

		if( unitPtr->nation_recno == info.viewing_nation_recno &&
			 unit_res[unitPtr->unit_id]->carry_goods_capacity > 0 )
		{
			info.report_array.linkin(&unitRecno);
		}
	}

	info.report_array.quick_sort(sort_unit);
}
//----------- End of static function create_ship_list -----------//


//-------- Begin of static function is_caravan_route_idle --------//
//
static int is_caravan_route_idle(UnitCaravan* unitPtr)
{
	int stops = 0;
	int can_pick_up = 0;
	for( int i = 0; i<MAX_STOP_FOR_CARAVAN; i++ )
	{
		CaravanStop* tradeStop = &unitPtr->stop_array[i];

		if( !tradeStop->firm_recno || firm_array.is_deleted(tradeStop->firm_recno) )
			continue;

		stops++;

		if( tradeStop->pick_up_type == NO_PICK_UP )
			continue;

		Firm* firmPtr = firm_array[tradeStop->firm_recno];

		if( firmPtr->firm_id == FIRM_MARKET )
		{
			FirmMarket* firmMarket = firmPtr->cast_to_FirmMarket();
			MarketGoods* marketGoods = firmMarket->market_goods_array;
			for( int j=0; j<MAX_MARKET_GOODS; j++, marketGoods++ )
			{
				if( !marketGoods->stock_qty && !marketGoods->sales_365days() )
					continue;

				if( marketGoods->raw_id && (tradeStop->pick_up_array[marketGoods->raw_id-1] || tradeStop->pick_up_type==AUTO_PICK_UP) )
				{
					can_pick_up++;
					break;
				}
				else if( marketGoods->product_raw_id && (tradeStop->pick_up_array[marketGoods->product_raw_id-1+MAX_RAW] || tradeStop->pick_up_type==AUTO_PICK_UP) )
				{
					can_pick_up++;
					break;
				}
			}
		}

		else if( firmPtr->firm_id == FIRM_FACTORY )
		{
			FirmFactory* firmFactory = firmPtr->cast_to_FirmFactory();

			if( !tradeStop->pick_up_array[firmFactory->product_raw_id-1+MAX_RAW] ||
					(!firmFactory->stock_qty && !firmFactory->production_30days()) )
				continue;

			can_pick_up++;
		}

		else if( firmPtr->firm_id == FIRM_MINE )
		{
			FirmMine* firmMine = firmPtr->cast_to_FirmMine();

			if( !tradeStop->pick_up_array[firmMine->raw_id-1] ||
					(!firmMine->stock_qty && !firmMine->production_30days()) )
				continue;

			can_pick_up++;
		}
	}

	if( stops <= 1 )
		return 1;
	return !can_pick_up;
}
//----------- End of static function is_caravan_route_idle -----------//


//-------- Begin of static function is_firm_idle --------//
//
static int is_firm_idle(Firm* firmPtr)
{
	if( firmPtr->under_construction )
		return 0;

	if( firmPtr->firm_id == FIRM_MARKET )
	{
		FirmMarket* firmMarket = firmPtr->cast_to_FirmMarket();
		MarketGoods* marketGoods = firmMarket->market_goods_array;
		for( int j=0; j<MAX_MARKET_GOODS; j++, marketGoods++ )
		{
			if( marketGoods->stock_qty || marketGoods->sales_365days() )
				return 0;
		}
		return 1;
	}

	if( firmPtr->firm_id == FIRM_FACTORY )
	{
		FirmFactory* firmFactory = firmPtr->cast_to_FirmFactory();

		return !firmFactory->stock_qty && !firmFactory->production_30days();
	}

	if( firmPtr->firm_id == FIRM_MINE )
	{
		FirmMine* firmMine = firmPtr->cast_to_FirmMine();

		return !firmMine->stock_qty && !firmMine->production_30days();
	}

	return 0;
}
//----------- End of static function is_firm_idle -----------//


//-------- Begin of static function put_caravan_rec --------//
//
static void put_caravan_rec(int recNo, int x, int y, int refreshFlag)
{
	int   		 unitRecno = info.get_report_data(recNo);
	UnitCaravan* unitPtr   = (UnitCaravan*) unit_array[unitRecno];

	int x2 = x+browse_caravan.rec_width-3;

	//---------- display info ----------//

	x+=3;
	y+=3;

	String str;

	str  = "";
	if( is_caravan_route_idle(unitPtr) )
		str += "*";
	str += unitPtr->unit_name();

	font_san.put( x    , y, str );

	str  = (int) unitPtr->hit_points;
	str += "/";
	str += unitPtr->max_hit_points;

	font_san.put( x+90 , y, str );

	//------- display pick up type of each stop -------//

	if( unitPtr->stop_defined_num >= 1 )
		put_stop_info( x+160, y, unitPtr->stop_array );

	if( unitPtr->stop_defined_num >= 2 )
		put_stop_info( x+220, y, unitPtr->stop_array+1 );

	if( unitPtr->stop_defined_num >= 3 )
		put_stop_info( x+280, y, unitPtr->stop_array+2 );

	//------- display goods carried -------//

	x += 340;

	char *bitmapPtr;

	int i;
	for(i=0; i<MAX_RAW; i++)
	{
		if( unitPtr->raw_qty_array[i]==0 )
			continue;

		bitmapPtr = raw_res.small_raw_icon(i+1);
		vga_back.put_bitmap( x, y, bitmapPtr );

		font_san.disp( x+14, y-1, unitPtr->raw_qty_array[i], 1, x+45 );
		x+=36;

		if( x+36 > x2 )
			return;
	}

	for( i=0; i<MAX_PRODUCT; i++)
	{
		if( unitPtr->product_raw_qty_array[i]==0 )
			continue;

		bitmapPtr = raw_res.small_product_icon(i+1);
		vga_back.put_bitmap( x, y, bitmapPtr );

		font_san.disp( x+14, y-1, unitPtr->product_raw_qty_array[i], 1, x+45 );
		x+=36;

		if( x+36 > x2 )
			return;
	}
}
//----------- End of static function put_caravan_rec -----------//


//-------- Begin of static function put_firm_rec --------//
//
static void put_firm_rec(int recNo, int x, int y, int refreshFlag)
{
	int firmRecno = info.get_report_data2(recNo);
	Firm* firmPtr = firm_array[firmRecno];

	int x2 = x+browse_firm.rec_width-3;

	//---------- display info ----------//

	x+=3;
	y+=3;

        nation_array[firmPtr->nation_recno]->disp_nation_color( x, y );

        x+=15;

	String str;

	str  = "";
	if( is_firm_idle(firmPtr) )
		str += "*";
	str += firmPtr->firm_name();

	font_san.put( x    , y, str );

	x+=170;

	//---------- content info ----------//

	if( firmPtr->firm_id == FIRM_MARKET )
	{
		FirmMarket* firmPtr2 = firmPtr->cast_to_FirmMarket();
		MarketGoods* marketGoods = firmPtr2->market_goods_array;

		for( int i=0 ; i<MAX_MARKET_GOODS ; i++, marketGoods++ )
		{
			if( marketGoods->raw_id )
			{
				vga_back.put_bitmap(x, y, raw_res.small_raw_icon(marketGoods->raw_id));
			}
			else if( marketGoods->product_raw_id )
			{
				vga_back.put_bitmap(x, y, raw_res.small_product_icon(marketGoods->product_raw_id));
			}
			else
			{
				continue;
			}

			x+=15;

			str  = marketGoods->stock_qty;
			str += "/";
			str += misc.format((int)marketGoods->sales_365days(),2);
			str += "/";
			str += misc.format(MAX((int)marketGoods->month_demand,0),1);

			font_san.put(x, y, str);

			x+=100;
		}
	}

	else if( firmPtr->firm_id == FIRM_FACTORY )
	{
		FirmFactory* firmPtr2 = firmPtr->cast_to_FirmFactory();

		vga_back.put_bitmap( x, y, raw_res.small_raw_icon(firmPtr2->product_raw_id) );

		x+=15;

		str  = _("Raw Stock");
		str += ": ";
		str += firmPtr2->raw_stock_qty;
		font_san.put(x, y, str);

		x+=110;

		vga_back.put_bitmap( x, y, raw_res.small_product_icon(firmPtr2->product_raw_id) );

		x+=15;

		str  = _("Product Stock");
		str += ": ";
		str += firmPtr2->stock_qty;
		font_san.put(x, y, str);
	}

	else if( firmPtr->firm_id == FIRM_MINE )
	{
		FirmMine* firmPtr2 = firmPtr->cast_to_FirmMine();

		vga_back.put_bitmap( x, y, raw_res.small_raw_icon(firmPtr2->raw_id) );

		x+=15;

		str  = _("Reserve");
		str += ": ";
		str += firmPtr2->reserve_qty;
		font_san.put(x, y, str);

		x+=110;

		vga_back.put_bitmap( x, y, raw_res.small_raw_icon(firmPtr2->raw_id) );

		x+=15;

		str  = _("Raw Stock");
		str += ": ";
		str += firmPtr2->stock_qty;
		font_san.put(x, y, str);
	}

	else if( firmPtr->firm_id == FIRM_HARBOR )
	{
		FirmHarbor* firmPtr2 = firmPtr->cast_to_FirmHarbor();

		str  = _("Markets");
		str += ": ";
		str += firmPtr2->linked_market_num;
		font_san.put(x, y, str);

		x+=110;

		str  = _("Factories");
		str += ": ";
		str += firmPtr2->linked_factory_num;
		font_san.put(x, y, str);

		x+=110;

		str  = _("Mines");
		str += ": ";
		str += firmPtr2->linked_mine_num;
		font_san.put(x, y, str);
	}
}
//----------- End of static function put_firm_rec -----------//


//-------- Begin of static function put_ship_rec --------//
//
static void put_ship_rec(int recNo, int x, int y, int refreshFlag)
{
	int   		unitRecno = info.get_report_data(recNo);
	UnitMarine* unitPtr   = (UnitMarine*) unit_array[unitRecno];

	int x2 = x+browse_ship.rec_width-3;

	//---------- display info ----------//

	x+=3;
	y+=3;

	String str;
	str  = (int) unitPtr->hit_points;
	str += "/";
	str += unitPtr->max_hit_points;

	font_san.put( x    , y, unitPtr->unit_name() );
	font_san.put( x+90 , y, str );

	//------- display pick up type of each stop -------//

	if( unitPtr->stop_defined_num >= 1 )
		put_stop_info( x+160, y, unitPtr->stop_array );

	if( unitPtr->stop_defined_num >= 2 )
		put_stop_info( x+220, y, unitPtr->stop_array+1 );

	if( unitPtr->stop_defined_num >= 3 )
		put_stop_info( x+280, y, unitPtr->stop_array+2 );

	//------- display goods carried -------//

	x += 340;

	char *bitmapPtr;

	int i;
	for(i=0; i<MAX_RAW; i++)
	{
		if( unitPtr->raw_qty_array[i]==0 )
			continue;

		bitmapPtr = raw_res.small_raw_icon(i+1);
		vga_back.put_bitmap( x, y, bitmapPtr );

		font_san.disp( x+14, y-1, unitPtr->raw_qty_array[i], 1, x+45 );
		x+=36;

		if( x+36 > x2 )
			return;
	}

	for( i=0; i<MAX_PRODUCT; i++)
	{
		if( unitPtr->product_raw_qty_array[i]==0 )
			continue;

		bitmapPtr = raw_res.small_product_icon(i+1);
		vga_back.put_bitmap( x, y, bitmapPtr );

		font_san.disp( x+14, y-1, unitPtr->product_raw_qty_array[i], 1, x+45 );
		x+=36;

		if( x+36 > x2 )
			return;
	}
}
//----------- End of static function put_ship_rec -----------//


//-------- Begin of static function put_stop_info --------//
//
static void put_stop_info(int x, int y, TradeStop* tradeStop)
{
	int x2=x+58;

	//----- display the color of the stop ----//

	if( firm_array.is_deleted(tradeStop->firm_recno) )
		return;

	Firm* firmPtr = firm_array[tradeStop->firm_recno];

	nation_array[firmPtr->nation_recno]->disp_nation_color( x, y );

	x+=15;

	//------ display pick up type icon ------//

	int i, pickUpType = tradeStop->pick_up_type;

	switch( pickUpType )
	{
		case AUTO_PICK_UP:
			vga_back.put_bitmap( x, y, image_icon.get_ptr("AUTOPICK") );
			break;

		case NO_PICK_UP:
			vga_back.put_bitmap( x, y, image_icon.get_ptr("NOPICK") );
			break;

		default:
			for( i=PICK_UP_RAW_FIRST ; i<=PICK_UP_RAW_LAST ; i++ )
			{
				if( tradeStop->pick_up_array[i-1] )
				{
					vga_back.put_bitmap( x, y, raw_res.small_raw_icon(i-PICK_UP_RAW_FIRST+1) );
					x+=10;

					if( x+10 > x2 )
						return;
				}
			}

			for( i=PICK_UP_PRODUCT_FIRST ; i<=PICK_UP_PRODUCT_LAST ; i++ )
			{
				if( tradeStop->pick_up_array[i-1] )
				{
					vga_back.put_bitmap( x, y, raw_res.small_product_icon(i-PICK_UP_PRODUCT_FIRST+1) );
					x+=12;

					if( x+10 > x2 )
						return;
				}
			}
	}
}
//----------- End of static function put_stop_info -----------//


//------ Begin of function sort_firm ------//
//
static int sort_firm( const void *a, const void *b )
{
	Firm* firmPtr1 = (Firm*) firm_array[*((short*)a)];
	Firm* firmPtr2 = (Firm*) firm_array[*((short*)b)];

	if( selected_unit_recno )
	{

		return misc.points_distance(unit_x, unit_y, firmPtr1->center_x, firmPtr1->center_y) -
			misc.points_distance(unit_x, unit_y, firmPtr2->center_x, firmPtr2->center_y);
	}

	return strcmp(firmPtr1->firm_name(), firmPtr2->firm_name());
}
//------- End of function sort_firm ------//


//------ Begin of function sort_unit ------//
//
static int sort_unit( const void *a, const void *b )
{
	Unit* unitPtr1 = (Unit*) unit_array[*((short*)a)];
	Unit* unitPtr2 = (Unit*) unit_array[*((short*)b)];

	return unitPtr1->name_id - unitPtr2->name_id;
}
//------- End of function sort_unit ------//
