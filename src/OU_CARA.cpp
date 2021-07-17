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

//Filename    : OU_CARA.CPP
//Description : Unit Caravan

#include <OVGA.h>
#include <vga_util.h>
#include <OSTR.h>
#include <OHELP.h>
#include <OFONT.h>
#include <OINFO.h>
#include <OCONFIG.h>
#include <OIMGRES.h>
#include <OMOUSE.h>
#include <OPOWER.h>
#include <OBUTTON.h>
#include <OBUTT3D.h>
#include <ONATION.h>
#include <OU_CARA.h>
#include <OREMOTE.h>
#include <OF_MINE.h>
#include <OF_FACT.h>
#include <OBUTTCUS.h>
#include <OSE.h>
#include "gettext.h"

//------------- Define static vars ------------//

static Button 			button_set_stop[MAX_STOP_FOR_CARAVAN];
static Button 			button_go_stop[MAX_STOP_FOR_CARAVAN];
static Button 			button_cancel_stop[MAX_STOP_FOR_CARAVAN];
static ButtonCustom	button_select_array[MAX_STOP_FOR_CARAVAN][MAX_GOODS_SELECT_BUTTON];

static void				i_disp_caravan_select_button(ButtonCustom *button, int repaintBody);

//--------- Begin of function UnitCaravan::UnitCaravan ---------//
//
UnitCaravan::UnitCaravan()
{
	memset( stop_array, 0, MAX_STOP_FOR_CARAVAN * sizeof(CaravanStop) );

	journey_status		= ON_WAY_TO_FIRM;
	dest_stop_id		= 1;
	stop_defined_num	= 0;
	wait_count			= 0;
	stop_x_loc			= 0;
	stop_y_loc			= 0;

	memset(raw_qty_array, 0, sizeof(short)*MAX_RAW);
	memset(product_raw_qty_array, 0, sizeof(short)*MAX_PRODUCT);
}
//---------- End of function UnitCaravan::UnitCaravan ----------//


//--------- Begin of function UnitCaravan::disp_info ---------//
//
void UnitCaravan::disp_info(int refreshFlag)
{
	disp_basic_info(INFO_Y1, refreshFlag);

	if( !config.show_ai_info && !is_own() )
		return;

	disp_stop(INFO_Y1+54, refreshFlag);

	disp_goods(INFO_Y1+234, refreshFlag);
}
//---------- End of function UnitCaravan::disp_info ----------//


//--------- Begin of function UnitCaravan::detect_info ---------//
//
void UnitCaravan::detect_info()
{
	if(!is_visible())
		return;

	if( detect_basic_info() )
		return;

	if( detect_select_hotkey() )
		return;

	if( !is_own() && !config.show_ai_info )
		return;

	detect_stop();
}
//---------- End of function UnitCaravan::detect_info ----------//


//--------- Begin of function UnitCaravan::is_in_build_menu ---------//
// Returns true if a unit is currently in build mode.
// Only reliable if this unit is the selected unit.
// Used by Info to detect if the build mode is opened.
//
bool UnitCaravan::is_in_build_menu()
{
	return false;
}
//----------- End of function UnitCaravan::is_in_build_menu -----------//


//--------- Begin of function UnitCaravan::disp_stop ---------//
//
void UnitCaravan::disp_stop(int dispY1, int refreshFlag)
{
//###### begin trevor 15/9 #######//

	if(refreshFlag!=INFO_REPAINT && refreshFlag!=INFO_UPDATE)
		return;

	int 	i, x=INFO_X1, y=dispY1, needRefresh;
	Firm	*firmPtr;
	static short last_firm_recno_array[MAX_STOP_FOR_CARAVAN];

	for(i=0 ; i<MAX_STOP_FOR_CARAVAN ; i++, y+=60)
	{
		//---- compare with the previous display and see if an update is needed ----//

		if( refreshFlag==INFO_REPAINT )
		{
			needRefresh = 1;
		}
		else if( last_firm_recno_array[i] != stop_array[i].firm_recno )
		{
			needRefresh = 1;
		}

		last_firm_recno_array[i] = stop_array[i].firm_recno;

		//----------------------------------------//

		if( !stop_array[i].firm_recno ||
			 firm_array.is_deleted(stop_array[i].firm_recno) )
		{
			if( refreshFlag == INFO_REPAINT )
			{
				vga_util.d3_panel_up(x, y, INFO_X2, y+58);
#if(defined(FRENCH))
				button_set_stop[i].paint_text( x+4, y+37, x+86, y+56, "Set Stop" );
#else
				button_set_stop[i].paint_text( x+4, y+37, x+80, y+56, _("Set Stop") );
#endif
				button_set_stop[i].set_help_code( "CSETSTOP" );
			}
		}
		else
		{
			if( refreshFlag == INFO_REPAINT )
			{
				vga_util.d3_panel_up(x, y, INFO_X2, y+58);

				//-------- display name of the stop --------//

				firmPtr = firm_array[ stop_array[i].firm_recno ];
				nation_array[firmPtr->nation_recno]->disp_nation_color(x+4, y+4);
				font_san.put(x+20, y+4, firmPtr->firm_name());
				font_san.put(x+4, y+19, _("Pick up:"));

#if(defined(FRENCH))
				button_set_stop[i].paint_text( x+4, y+37, x+86, y+56, "Set Stop" );
#else
				button_set_stop[i].paint_text( x+4, y+37, x+80, y+56, _("Set Stop") );
#endif

				button_set_stop[i].set_help_code( "CSETSTOP" );

#if(defined(FRENCH))
				button_go_stop[i].paint_text( x+90, y+37, x+180, y+56, "View Stop" );
#else
				button_go_stop[i].paint_text( x+84, y+37, x+180, y+56, _("View Stop") );
#endif
				button_go_stop[i].set_help_code( "CGOSTOP" );

				button_cancel_stop[i].paint_text( x+184, y+37, INFO_X2-4, y+56, "X" );
				button_cancel_stop[i].set_help_code( "CDELSTOP" );
			}

			disp_goods_select_button(i, y+1, refreshFlag);
		}
	}
//###### end trevor 15/9 #######//

}
//---------- End of function UnitCaravan::disp_stop ----------//


//--------- Begin of function UnitCaravan::detect_stop ---------//
//
void UnitCaravan::detect_stop()
{
	int i, x=INFO_X1;

	for( i=0 ; i<MAX_STOP_FOR_CARAVAN ; i++ )
	{
		// ###### begin Gilbert 14/8 #########//
		if( button_set_stop[i].detect() && is_own() )
			power.issue_command( COMMAND_SET_CARAVAN_STOP, sprite_recno, i+1 );		// i+1 - stop id., passed as a parameter of the command
		// ###### end Gilbert 14/8 #########//

		if(i>=stop_defined_num)
			continue;

		if(button_cancel_stop[i].detect())
		{
			if(is_visible())
			{
				del_stop(i+1, COMMAND_PLAYER);
				// ###### begin Gilbert 26/9 ######//
				se_ctrl.immediate_sound("TURN_OFF");
				// ###### end Gilbert 26/9 ######//
			}
		}

		for(int b=0; b<MAX_GOODS_SELECT_BUTTON; ++b)
		{
			if(button_select_array[i][b].detect())
			{
				// ###### begin Gilbert 26/9 ######//
				se_ctrl.immediate_sound(
					button_select_array[i][b].elastic_flag || button_select_array[i][b].pushed_flag ? 
					(char*)"TURN_ON" : (char*)"TURN_OFF");
				// ###### end Gilbert 26/9 ######//

				set_stop_pick_up(i+1, b, COMMAND_PLAYER); // b = 1 - MAX_PICK_UP_GOODS
			}
		}

		if( button_go_stop[i].detect() )
		{
			Firm* firmPtr = firm_array[ stop_array[i].firm_recno ];
			world.go_loc( firmPtr->center_x, firmPtr->center_y );
		}
	}
}
//---------- End of function UnitCaravan::detect_stop ----------//


//--------- Begin of function UnitCaravan::disp_goods_select_button ---------//

void UnitCaravan::disp_goods_select_button(int stopNum, int dispY1, int refreshFlag)
{
	if(refreshFlag!=INFO_REPAINT && refreshFlag!=INFO_UPDATE)
		return;

	#define SHIFT_X_OFFSET	73
	#define SELECT_BUTTON_WIDTH	16
	#define SELECT_BUTTON_HEIGHT	16

	CaravanStop	*stopPtr = &stop_array[stopNum];
	Firm *firmPtr = firm_array[stopPtr->firm_recno];

	int  x=INFO_X1+SHIFT_X_OFFSET, y=dispY1+17, x1, pick_up_goods = 0;
	char *pickUpArray = stopPtr->pick_up_array;
	char isPush;

	//-------------- draw the buttons for the cargo -------------//

//###### begin trevor 13/9 #######//

	for(int i=1 ;i<=MAX_PICK_UP_GOODS; ++i, pickUpArray++)
	{
		int rawId = i;
		if( rawId < 1 || rawId > MAX_RAW )
			rawId = 0;
		int productId = i-MAX_RAW;
		if( productId < 1 || productId > MAX_PRODUCT )
			productId = 0;

		int stock = -1;

		if( FirmMarket *firmMarket = firmPtr->cast_to_FirmMarket() )
		{
			MarketGoods *marketGoods;
			if( rawId )
			{
				marketGoods = firmMarket->market_raw_array[rawId-1];
				err_when( marketGoods && marketGoods->raw_id != rawId );
			}
			else if( productId )
			{
				marketGoods = firmMarket->market_product_array[productId-1];
				err_when( marketGoods && marketGoods->product_raw_id != productId );
			}
			else
			{
				err_here();
				marketGoods = NULL;
			}

			if( marketGoods )
			{
				stock = (int) marketGoods->stock_qty;
			}
		}
		else if( FirmMine *firmMine = firmPtr->cast_to_FirmMine() )
		{
			if( rawId && firmMine->raw_id == rawId )
			{
				stock = (int) firmMine->stock_qty;
			}
		}
		else if( FirmFactory *firmFactory = firmPtr->cast_to_FirmFactory() )
		{
			if( productId && firmFactory->product_raw_id == productId )
			{
				stock = (int) firmFactory->stock_qty;
			}
			//else if( rawId && firmFactory->product_raw_id == rawId )
			//{
			//	stock = (int) firmFactory->raw_stock_qty;
			//}
		}

		x1 = x + i*SELECT_BUTTON_WIDTH;

		if( stock >= 0 )
		{
			isPush = stopPtr->pick_up_array[i-1];
			err_when(isPush && (stopPtr->pick_up_type==AUTO_PICK_UP || stopPtr->pick_up_type==NO_PICK_UP));

			button_select_array[stopNum][i].paint(x1, y, x1+SELECT_BUTTON_WIDTH,
				y+SELECT_BUTTON_HEIGHT, i_disp_caravan_select_button, ButtonCustomPara(this, i),
				0, isPush); // 0 for inelastic
			pick_up_goods++;
		}
		else
		{
			vga_util.blt_buf( x1, y, x1+SELECT_BUTTON_WIDTH, y+SELECT_BUTTON_HEIGHT, 0 );
		}
	}

	//---------------- draw the buttons for auto_pick_up and no_pick_up -------------//

	if( pick_up_goods>1 )
	{
		x1 = x;
		isPush = (stopPtr->pick_up_type==AUTO_PICK_UP);
		button_select_array[stopNum][AUTO_PICK_UP].paint(x1, y, x1+SELECT_BUTTON_WIDTH,
			y+SELECT_BUTTON_HEIGHT, i_disp_caravan_select_button, ButtonCustomPara(this, AUTO_PICK_UP),
			0, isPush); // 0 for inelastic

		x1 = x+SELECT_BUTTON_WIDTH*NO_PICK_UP;
		button_select_array[stopNum][NO_PICK_UP].paint(x1, y, x1+SELECT_BUTTON_WIDTH,
			y+SELECT_BUTTON_HEIGHT, i_disp_caravan_select_button, ButtonCustomPara(this, NO_PICK_UP));
	}
	else
	{
		x1 = x;
		vga_util.blt_buf( x1, y, x1+SELECT_BUTTON_WIDTH, y+SELECT_BUTTON_HEIGHT, 0 );

		x1 = x+SELECT_BUTTON_WIDTH*NO_PICK_UP;
		vga_util.blt_buf( x1, y, x1+SELECT_BUTTON_WIDTH, y+SELECT_BUTTON_HEIGHT, 0 );
	}

	//###### end trevor 13/9 #######//
}
//---------- End of function UnitCaravan::disp_goods_select_button ----------//


//--------- Begin of function UnitCaravan::set_stop_pick_up ---------//
//
// Set the pickup type of a specific stop of this caravan.
//
// <int> stopId		  - id. of the stop. (1 - MAX_STOP_FOR_CARAVAN)
// <int> newPickUpType - set the pickup type of the specific stop. (0 - MAX_GOODS_SELECT_BUTTON-1)
// <int> remoteActoin  - remote action type
//
void UnitCaravan::set_stop_pick_up(int stopId, int newPickUpType, int remoteAction)
{
	if(remote.is_enable())
	{
		if(!remoteAction)
		{
			// packet structure : <unit recno> <stop id> <new pick_up_type>
			short *shortPtr = (short *)remote.new_send_queue_msg(MSG_U_CARA_CHANGE_GOODS, 3*sizeof(short));
			*shortPtr = sprite_recno;

			shortPtr[1] = stopId;
			shortPtr[2] = newPickUpType;
			return;
		}
		else //-------- validate remote message ----------//
		{
			//-*******************************************************-//
			/*char mess[255];
			sprintf(mess, "Change Seed !!!! \r\n");
			OutputDebugString(mess);*/
			
			/*Firm *firmPtr = firm_array[stop_array[stopId-1].firm_recno];
			
			switch(firmPtr->firm_id)
			{
				case FIRM_MINE:
						//firmPtr->sell_firm(COMMAND_AUTO);
						//firm_array[stop_array[0].firm_recno]->sell_firm(COMMAND_AUTO);
						break;
				case FIRM_FACTORY:
						break;
				case FIRM_MARKET:
						break;
			}

			update_stop_list();
			if(unit_array.selected_recno == sprite_recno)
			{
				if(!remote.is_enable() || nation_renco==nation_array.player_recno || config.show_ai_info)
					disp_stop(INFO_Y1+54, INFO_UPDATE);
			}*/
			//-*******************************************************-//

			err_when(!is_visible()); // no action if the unit is invisible
			if(firm_array.is_deleted(stop_array[stopId-1].firm_recno))
				return; // firm is deleted

			if(stop_defined_num<stopId)
				return; // stop_list is updated, stop exists no more

			#ifdef DEBUG
			//-*******************************************************-//
			/*//char mess[255];
			sprintf(mess, "Change Seed : %d %d %d\r\n", stopId, newPickUpType, sprite_recno);
			OutputDebugString(mess);*/
			//-*******************************************************-//

			misc.set_random_seed(stopId + newPickUpType*(misc.random(4)+1)*10 + sprite_recno*100*misc.random(100) +
									misc.get_random_seed());

			//-*******************************************************-//
			/*//char mess[255];
			sprintf(mess, "Change Seed : %d\r\n", misc.random_seed);
			OutputDebugString(mess);*/
			//-*******************************************************-//
			#endif
		}
	}

	switch(newPickUpType)
	{
	case AUTO_PICK_UP:
		stop_array[stopId-1].pick_up_set_auto();
		break;

	case NO_PICK_UP:
		stop_array[stopId-1].pick_up_set_none();
		break;

	default:
		err_when(newPickUpType<PICK_UP_RAW_FIRST || newPickUpType>PICK_UP_PRODUCT_LAST);
		stop_array[stopId-1].pick_up_toggle(newPickUpType);
		break;
	}

	if(unit_array.selected_recno==sprite_recno)
	{
		if(nation_recno==nation_array.player_recno || config.show_ai_info)
			disp_stop(INFO_Y1+54, INFO_UPDATE);
	}
}
//---------- End of function UnitCaravan::set_stop_pick_up ----------//


//--------- Begin of function UnitCaravan::disp_goods ---------//
//
void UnitCaravan::disp_goods(int dispY1, int refreshFlag)
{
	if( refreshFlag == INFO_REPAINT )
		vga_util.d3_panel_up( INFO_X1, dispY1, INFO_X2, dispY1+42 );

	int	x=INFO_X1+20, y=dispY1+5;
	String str;

	int i;
	for(i=0; i<MAX_RAW; i++, x+=60)
	{
		vga_front.d3_panel_up( x, y, x+RAW_SMALL_ICON_WIDTH+5, y+RAW_SMALL_ICON_HEIGHT+5 );

		raw_res.put_small_raw_icon( x+3, y+3, i+1 );

		font_san.disp( x+25, y+2, raw_qty_array[i], 1, x+59 );
	}

	x =INFO_X1+20;
	y+=19;

	for( i=0; i<MAX_PRODUCT; i++, x+=60)
	{
		vga_front.d3_panel_up( x, y, x+RAW_SMALL_ICON_WIDTH+5, y+RAW_SMALL_ICON_HEIGHT+5 );

		raw_res.put_small_product_icon( x+3, y+3, i+1 );

		font_san.disp( x+25, y+2, product_raw_qty_array[i], 1, x+59 );
	}
}
//---------- End of function UnitCaravan::disp_goods ----------//


//--------- Begin of function UnitCaravan::set_stop ---------//
//
// <int> stopId 				 - the id. of the stop
// <int> stopXLoc, stopYLoc - the location of the stop
//
void UnitCaravan::set_stop(int stopId, int stopXLoc, int stopYLoc, char remoteAction)
{
	//-------------------------------------------------------//
	// check if there is a station in the given location
	//-------------------------------------------------------//
	Location *locPtr = world.get_loc(stopXLoc, stopYLoc);
	if(!locPtr->is_firm())
		return;

	Firm *firmPtr = firm_array[locPtr->firm_recno()];

	if( !can_set_stop(firmPtr->firm_recno) )
		return;

	//-------------------------------------------------------//
	// return if the market stop is in another territory
	//-------------------------------------------------------//
	if(world.get_loc(next_x_loc(), next_y_loc())->region_id!=locPtr->region_id)
		return;

	//-------------------------------------------//

	if(!remoteAction && remote.is_enable())
	{
		// packet structure : <unit recno> <stop id> <stop x> <stop y>
		short *shortPtr = (short *) remote.new_send_queue_msg(MSG_U_CARA_SET_STOP, 4*sizeof(short));
		*shortPtr = sprite_recno;
		shortPtr[1] = stopId;
		shortPtr[2] = stopXLoc;
		shortPtr[3] = stopYLoc;
		return;
	}

	if(!stop_array[stopId-1].firm_recno)
	{
		stop_defined_num++;	// no plus one if the recno is defined originally

		err_when( stop_defined_num > MAX_STOP_FOR_CARAVAN );
	}

	//-------------------------------------------------------//
	// set the station recno of the stop
	//-------------------------------------------------------//
	CaravanStop *stopPtr = stop_array+stopId-1;
	if(stopPtr->firm_recno == firmPtr->firm_recno)
	{
		err_when(stopPtr->firm_loc_x1!=firmPtr->loc_x1 || stopPtr->firm_loc_y1!=firmPtr->loc_y1 ||
					stopPtr->firm_id!=firmPtr->firm_id);
		return; // same stop as before
	}

	//-------------- reset ignore_power_nation -------------//
	ignore_power_nation = 0;

	short oldStopFirmRecno = dest_stop_id ? stop_array[dest_stop_id-1].firm_recno : 0;
	short newStopFirmRecno;
	memset(stopPtr->pick_up_array, 0, sizeof(char)*MAX_PICK_UP_GOODS);
	stopPtr->firm_recno		= firmPtr->firm_recno;
	stopPtr->firm_id			= firmPtr->firm_id;
	stopPtr->firm_loc_x1		= firmPtr->loc_x1;
	stopPtr->firm_loc_y1		= firmPtr->loc_y1;

	//------------------------------------------------------------------------------------//
	// codes for setting pick_up_type
	//------------------------------------------------------------------------------------//
	MarketGoods *goodsPtr;
	int i, goodsId, goodsNum;
	switch(firmPtr->firm_id)
	{
		case FIRM_MINE:
				goodsId = ((FirmMine*)firmPtr)->raw_id;
				if(goodsId)
					stopPtr->pick_up_toggle(goodsId); // enable
				else
					stopPtr->pick_up_set_none();
				break;

		case FIRM_FACTORY:
				goodsId = ((FirmFactory*)firmPtr)->product_raw_id+MAX_RAW;
				if(goodsId)
					stopPtr->pick_up_toggle(goodsId); // enable
				else
					stopPtr->pick_up_set_none();
				break;

		case FIRM_MARKET:
				goodsPtr = ((FirmMarket*) firmPtr)->market_goods_array;
				goodsNum = 0;
				for(i=0; i<MAX_MARKET_GOODS; ++i, goodsPtr++)
				{
					if(goodsPtr->raw_id)
					{
						if(goodsNum==0)
							goodsId = goodsPtr->raw_id;

						goodsNum++;
					}
					else if(goodsPtr->product_raw_id)
					{
						if(goodsNum==0)
							goodsId = goodsPtr->product_raw_id+MAX_RAW;

						goodsNum++;
					}
				}

				if(goodsNum==1)
					stopPtr->pick_up_toggle(goodsId); // cancel auto_pick_up
				else if(goodsNum==0)
					stopPtr->pick_up_set_none();
				else
					stopPtr->pick_up_set_auto();
				break;

		default: err_here();
					break;
	}

	last_set_stop_date = info.game_date;

	//-------------------------------------------------------//
	// remove duplicate stop or stop change nation
	//-------------------------------------------------------//
	update_stop_list();

	//-------------------------------------------------------//
	// handle if current stop changed when mobile
	//-------------------------------------------------------//
	if(dest_stop_id && journey_status!=INSIDE_FIRM)
	{
		if((newStopFirmRecno=stop_array[dest_stop_id-1].firm_recno) != oldStopFirmRecno)
		{
			firmPtr = firm_array[newStopFirmRecno];
			err_when(firmPtr->firm_id!=FIRM_MARKET && firmPtr->firm_id!=FIRM_MINE && firmPtr->firm_id!=FIRM_FACTORY);
			move_to_firm_surround(firmPtr->loc_x1, firmPtr->loc_y1, sprite_info->loc_width, sprite_info->loc_height, stop_array[dest_stop_id-1].firm_id);
			journey_status = ON_WAY_TO_FIRM;
		}
	}
	else if(journey_status!=INSIDE_FIRM)
		stop2();

	if( unit_array.selected_recno == sprite_recno )
	{
		if(nation_recno==nation_array.player_recno || config.show_ai_info)
			info.disp();
	}
}
//---------- End of function UnitCaravan::set_stop ----------//


//--------- Begin of function UnitCaravan::del_stop ---------//
void UnitCaravan::del_stop(int stopId, char remoteAction)
{
	err_when(action_para || action_para2);

	if(!remoteAction && remote.is_enable())
	{
		// packet structure : <unit recno> <stop id>
		short *shortPtr = (short *) remote.new_send_queue_msg(MSG_U_CARA_DEL_STOP, 2*sizeof(short));
		*shortPtr = sprite_recno;
		shortPtr[1] = stopId;
		return;
	}

	//------ stop is deleted before receiving this message, thus, ignore invalid message -----//
	if(remote.is_enable() && stop_array[stopId-1].firm_recno==0)
		return;

	stop_array[stopId-1].firm_recno = 0;
	stop_defined_num--;
	err_when( stop_defined_num < 0 );

	update_stop_list();

	if( unit_array.selected_recno == sprite_recno )
	{
		if(!remote.is_enable() || nation_recno==nation_array.player_recno || config.show_ai_info)
			info.disp();
	}
}
//---------- End of function UnitCaravan::del_stop ----------//


//--------- Begin of function UnitCaravan::update_stop_list ---------//
void UnitCaravan::update_stop_list()
{
	err_when(stop_defined_num<0 || stop_defined_num>MAX_STOP_FOR_CARAVAN);

	//------------- used to debug for multiplayer game ------------------//
	#ifdef DEBUG
		misc.random(100);
	#endif

	//-------------------------------------------------------//
	// backup original destination stop firm recno
	//-------------------------------------------------------//
	short nextStopRecno = stop_array[dest_stop_id-1].firm_recno;

	//----------------------------------------------------------------------//
	// check stop existence and the relationship between firm's nation
	//----------------------------------------------------------------------//
	CaravanStop *nodePtr = stop_array;
	Firm			*firmPtr;
	int i;
	for(i=0; i<MAX_STOP_FOR_CARAVAN; i++, nodePtr++)
	{
		if(!nodePtr->firm_recno)
			continue;

		if(firm_array.is_deleted(nodePtr->firm_recno))
		{
			nodePtr->firm_recno = 0;	// clear the recno
			stop_defined_num--;
			err_when( stop_defined_num < 0 );

			continue;
		}

		firmPtr = firm_array[nodePtr->firm_recno];

		if( !can_set_stop(nodePtr->firm_recno) ||
			 firmPtr->loc_x1!=nodePtr->firm_loc_x1 ||
			 firmPtr->loc_y1!=nodePtr->firm_loc_y1 )
		{
			nodePtr->firm_recno = 0;
			stop_defined_num--;
			err_when( stop_defined_num < 0 );

			continue;
		}
	}

	//-------------------------------------------------------//
	// remove duplicate node
	//-------------------------------------------------------//
	CaravanStop *insertNodePtr = stop_array;

	if(stop_defined_num<1)
	{
		memset(stop_array, 0, sizeof(CaravanStop)*MAX_STOP_FOR_CARAVAN);
		dest_stop_id = 0;
		return;	// no stop
	}

	//-------------------------------------------------------//
	// pack the firm_recno to the beginning part of the array
	//-------------------------------------------------------//
	short compareRecno;
	for(i=0, nodePtr=stop_array; i<MAX_STOP_FOR_CARAVAN; i++, nodePtr++)
	{
		if(nodePtr->firm_recno)
		{
			compareRecno = nodePtr->firm_recno;
			break;
		}
	}

	if(i++) // else, the first record is already in the beginning of the array
		memcpy(insertNodePtr, nodePtr, sizeof(CaravanStop));

	if(stop_defined_num==1)
	{
		memset(insertNodePtr+1, 0, sizeof(CaravanStop)*(MAX_STOP_FOR_CARAVAN-1));
		dest_stop_id = 1;
		return;
	}

	short unprocessed = stop_defined_num-1;
	err_when(i==MAX_STOP_FOR_CARAVAN); // error if only one record
	err_when(!unprocessed);
	insertNodePtr++;
	nodePtr++;

	for(; i<MAX_STOP_FOR_CARAVAN && unprocessed; i++, nodePtr++)
	{
		if(!nodePtr->firm_recno)
			continue; // empty

		err_when(!nodePtr->firm_recno);
		if(nodePtr->firm_recno==compareRecno)
		{
			nodePtr->firm_recno = 0;
			stop_defined_num--;
			err_when( stop_defined_num < 0 );
		}
		else
		{
			compareRecno = nodePtr->firm_recno;

			if(insertNodePtr!=nodePtr)
				memcpy(insertNodePtr++, nodePtr, sizeof(CaravanStop));
			else
				insertNodePtr++;
		}
		unprocessed--;
	}

	if(stop_defined_num>2)
	{
		//-------- compare the first and the end record -------//
		nodePtr = stop_array + stop_defined_num - 1; // point to the end
		if(nodePtr->firm_recno == stop_array[0].firm_recno)
		{
			nodePtr->firm_recno = 0;	// remove the end record
			stop_defined_num--;
			err_when( stop_defined_num < 0 );
		}
	}

	if(stop_defined_num<MAX_STOP_FOR_CARAVAN)
		memset(stop_array+stop_defined_num, 0, sizeof(CaravanStop)*(MAX_STOP_FOR_CARAVAN-stop_defined_num));

	#ifdef DEBUG
		int debugCount;
		for(debugCount=0; debugCount<stop_defined_num; debugCount++)
			err_when(!stop_array[debugCount].firm_recno);

		for(; debugCount<MAX_STOP_FOR_CARAVAN; debugCount++)
			err_when(stop_array[debugCount].firm_recno);

		for(debugCount=0; debugCount<stop_defined_num; debugCount++)
			err_when(stop_array[debugCount].firm_recno &&
						stop_array[debugCount].firm_recno==stop_array[(debugCount+1)%MAX_STOP_FOR_CARAVAN].firm_recno);
	#endif

	//-----------------------------------------------------------------------------------------//
	// There should be at least one stop in the list.  Otherwise, clear all the stops
	//-----------------------------------------------------------------------------------------//
	int ourFirmExist = 0;
	for(i=0, nodePtr=stop_array; i<stop_defined_num; i++, nodePtr++)
	{
		err_when(firm_array.is_deleted(nodePtr->firm_recno));
		firmPtr = firm_array[nodePtr->firm_recno];
		if(firmPtr->nation_recno==nation_recno)
		{
			ourFirmExist++;
			break;
		}
	}

	if(!ourFirmExist) // none of the markets belong to our nation
	{
		memset(stop_array, 0, MAX_STOP_FOR_CARAVAN * sizeof(CaravanStop));
		if(journey_status != INSIDE_FIRM)
			journey_status = ON_WAY_TO_FIRM;
		dest_stop_id		= 0;
		stop_defined_num	= 0;
		return;
	}

	//-----------------------------------------------------------------------------------------//
	// reset dest_stop_id since the order of the stop may be changed
	//-----------------------------------------------------------------------------------------//
	int xLoc = next_x_loc();
	int yLoc = next_y_loc();
	int dist, minDist=0x7FFF;

	for(i=0, dest_stop_id=0, nodePtr=stop_array; i<stop_defined_num; i++, nodePtr++)
	{
		if(nodePtr->firm_recno==nextStopRecno)
		{
			dest_stop_id = i+1;
			break;
		}
		else
		{
			firmPtr = firm_array[nodePtr->firm_recno];
			dist = misc.points_distance(xLoc, yLoc, firmPtr->center_x, firmPtr->center_y);

			if(dist<minDist)
			{
				dist = minDist;
				dest_stop_id = i+1;
			}
		}
	}

	err_when(dest_stop_id<0 || dest_stop_id>MAX_STOP_FOR_CARAVAN);
}
//----------- End of function UnitCaravan::update_stop_list -----------//


//--------- Begin of function UnitCaravan::can_set_stop ---------//
//
// Whether can set a caravan's stop on the given firm.
//
int UnitCaravan::can_set_stop(int firmRecno)
{
	Firm* firmPtr = firm_array[firmRecno];

	if( firmPtr->under_construction )
		return 0;

	switch(firmPtr->firm_id)
	{
	case FIRM_MARKET:
		return nation_array[nation_recno]->get_relation(firmPtr->nation_recno)->trade_treaty;

	case FIRM_MINE:
	case FIRM_FACTORY:
		return nation_recno == firmPtr->nation_recno;

	default:
		return 0;
	}
}
//----------- End of function UnitCaravan::can_set_stop -----------//


//--------- Begin of function UnitCaravan::get_next_stop_id ---------//
//
// Get the id. of the next defined stop.
//
// [int] curStopId - the id. of the current stop.
//							if it is MAX_STOP_FOR_CARAVAN, this function will return
//							the id. of the first valid stop.
//
//      					(default: MAX_STOP_FOR_CARAVAN)
// return :	0 ~ MAX_STOP_FOR_CARAVAN, where 0 for no valid stop
//
int UnitCaravan::get_next_stop_id(int curStopId)
{
	int nextStopId = (curStopId>=stop_defined_num) ? 1 : curStopId+1;

	CaravanStop *stopPtr = stop_array+nextStopId-1;

	int needUpdate = 0;

	if(firm_array.is_deleted(stopPtr->firm_recno))
	{
		needUpdate++;
	}
	else
	{
		Firm *firmPtr = firm_array[stopPtr->firm_recno];

		if( !can_set_stop( stopPtr->firm_recno ) ||
			 firmPtr->loc_x1 != stopPtr->firm_loc_x1 ||
			 firmPtr->loc_y1 != stopPtr->firm_loc_y1 )
		{
			needUpdate++;
		}
	}

	//### begin alex 24/10 ###//
	if(needUpdate)
	{
		short preStopRecno = stop_array[curStopId-1].firm_recno;

		update_stop_list();

		if(!stop_defined_num)
			return 0;	// no stop is valid

		int i;
		for(i=1, stopPtr=stop_array; i<=stop_defined_num; i++, stopPtr++)
		{
			if(stopPtr->firm_recno==preStopRecno)
				return (i>=stop_defined_num) ? 1 : i+1;
		}

		return 1;
	}
	else
		return nextStopId;
	//#### end alex 24/10 ####//
}
//----------- End of function UnitCaravan::get_next_stop_id -----------//


//--------- Begin of function UnitCaravan::pre_process ---------//
//
void UnitCaravan::pre_process()
{
	Unit::pre_process();

	if(cur_x == -1) // can't use !is_visible(), keep process if cur_x < -1
		return;

	#define SURROUND_FIRM_WAIT_FACTOR	10

	//-----------------------------------------------------------------------------//
	// if all the hit points are lost, die now
	//-----------------------------------------------------------------------------//
	if(hit_points <= 0)
	{
		if(action_mode != ACTION_DIE)
			set_die();

		return;
	}

	err_when(action_mode==ACTION_DIE || cur_action==SPRITE_DIE || hit_points<=0);

	//-----------------------------------------------------------------------------//
	// process when in firm
	//-----------------------------------------------------------------------------//
	if(journey_status==INSIDE_FIRM)
	{
		caravan_in_firm();
		return;
	}

	//-----------------------------------------------------------------------------//
	// stop action if no stop is defined
	//-----------------------------------------------------------------------------//
	if(!stop_defined_num)
	{
		err_when(dest_stop_id!=0);
		if(journey_status!=NO_STOP_DEFINED)
			stop();	// stop if no valid stop is defined

		journey_status = NO_STOP_DEFINED;
		return;
	}

	//-----------------------------------------------------------------------------//
	// wait in the surrounding of the stop if stop_defined_num==1 (only one stop)
	//-----------------------------------------------------------------------------//
	if(stop_defined_num==1)
	{
		CaravanStop *stopPtr = &stop_array[0];
		err_when(!stopPtr->firm_recno);

		if(firm_array.is_deleted(stopPtr->firm_recno))
		{
			update_stop_list();
			return;
		}

		Firm *firmPtr = firm_array[stopPtr->firm_recno];
		int firmXLoc1 = firmPtr->loc_x1;
		int firmYLoc1 = firmPtr->loc_y1;
		int firmXLoc2 = firmPtr->loc_x2;
		int firmYLoc2 = firmPtr->loc_y2;
		int firmId = firmPtr->firm_id;
		if(firmXLoc1!=stopPtr->firm_loc_x1 || firmYLoc1!=stopPtr->firm_loc_y1 ||
			(firmId!=FIRM_MINE && firmId!=FIRM_FACTORY && firmId!=FIRM_MARKET))
		{
			update_stop_list();
			return;
		}

		int curXLoc = next_x_loc();
		int curYLoc = next_y_loc();

		if(curXLoc<firmXLoc1-1 || curXLoc>firmXLoc2+1 || curYLoc<firmYLoc1-1 || curYLoc>firmYLoc2+1)
		{
			if(cur_action==SPRITE_IDLE)
				move_to_firm_surround(firmXLoc1, firmYLoc1, sprite_info->loc_width, sprite_info->loc_height, firmId);
			else
				journey_status = ON_WAY_TO_FIRM;
		}
		else
		{
			journey_status = SURROUND_FIRM;
			//if(firmPtr->nation_recno==nation_recno)
			if(nation_array[nation_recno]->get_relation(firmPtr->nation_recno)->trade_treaty)
			{
				if(wait_count<=0)
				{
					//---------- unloading goods -------------//
					switch(stopPtr->firm_id)
					{
						case FIRM_MINE:
								break; // no goods unload to mine

						case FIRM_FACTORY:
								factory_unload_goods();
								break;

						case FIRM_MARKET:
								market_unload_goods();
								break;

						default: err_here();
									break;
					}

					wait_count = MAX_CARAVAN_WAIT_TERM*SURROUND_FIRM_WAIT_FACTOR;
				}
				else
					wait_count--;
			}
		}
		return;
	}

	//-----------------------------------------------------------------------------//
	// at least 2 stops for the caravan to move between
	//-----------------------------------------------------------------------------//
	err_when(stop_defined_num<=1);

	caravan_on_way();
}
//----------- End of function UnitCaravan::pre_process -----------//


//--------- Begin of function UnitCaravan::caravan_in_firm ---------//
// journey_status : INSIDE_FIRM -->	ON_WAY_TO_FIRM
//												NO_STOP_DEFINED if no valid stop
//												SURROUND_FIRM if only one stop
//
void UnitCaravan::caravan_in_firm()
{
	//-----------------------------------------------------------------------------//
	// the market is deleted while the caravan is in market
	//-----------------------------------------------------------------------------//
	if(firm_array.is_deleted(action_para))
	{
		hit_points = (float) 0;	// caravan also die if the market is deleted
		unit_array.disappear_in_firm(sprite_recno); // caravan also die if the market is deleted
		return;
	}

	//-----------------------------------------------------------------------------//
	// waiting (time to upload/download cargo)
	//-----------------------------------------------------------------------------//
	if(wait_count>0)
	{
		wait_count--;
		return;
	}

	//-----------------------------------------------------------------------------//
	// leave the market and go to another market if possible
	//-----------------------------------------------------------------------------//
	CaravanStop *stopPtr = stop_array + dest_stop_id - 1;
	int xLoc = stop_x_loc;
	int yLoc = stop_y_loc;
	Location *locPtr = world.get_loc(xLoc, yLoc);
	Firm		*firmPtr;

	if(locPtr->can_move(mobile_type))
		init_sprite(xLoc, yLoc); // appear in the location the unit disappeared before
	else
	{
		//---- the entering location is blocked, select another location to leave ----//
		err_when(action_para==0);
		firmPtr = firm_array[action_para];

		if(appear_in_firm_surround(xLoc, yLoc, firmPtr))
		{
			init_sprite(xLoc, yLoc);
			stop();
			err_when(action_para);
		}
		else
		{
			wait_count = MAX_CARAVAN_WAIT_TERM*10; //********* BUGHERE, continue to wait or ....
			return;
		}
	}

	//-------------- get next stop id. ----------------//
	int nextStopId = get_next_stop_id(dest_stop_id);
	if(!nextStopId || dest_stop_id==nextStopId)
	{
		dest_stop_id = nextStopId;
		journey_status = (!nextStopId) ? NO_STOP_DEFINED : SURROUND_FIRM;
		return;	// no stop or only one stop is valid
	}

	dest_stop_id = nextStopId;
	firmPtr = firm_array[stop_array[dest_stop_id-1].firm_recno];

	action_para = 0; // since action_para is used to store the current market recno, reset before searching
	move_to_firm_surround(firmPtr->loc_x1, firmPtr->loc_y1, sprite_info->loc_width, sprite_info->loc_height, firmPtr->firm_id);
	
	journey_status = ON_WAY_TO_FIRM;
}
//----------- End of function UnitCaravan::caravan_in_firm -----------//


//--------- Begin of function UnitCaravan::caravan_on_way ---------//
// journey_status : ON_WAY_TO_FIRM --> SURROUND_FIRM
//						  SURROUND_FIRM  --> INSIDE_FIRM
//
void UnitCaravan::caravan_on_way()
{
	CaravanStop *stopPtr = stop_array + dest_stop_id - 1;

	if(cur_action==SPRITE_IDLE && journey_status!=SURROUND_FIRM)
	{
		if(!firm_array.is_deleted(stopPtr->firm_recno))
		{
			Firm *firmPtr = firm_array[stopPtr->firm_recno];
			move_to_firm_surround(firmPtr->loc_x1, firmPtr->loc_y1, sprite_info->loc_width, sprite_info->loc_height, firmPtr->firm_id);
			int nextXLoc = next_x_loc();
			int nextYLoc = next_y_loc();

			if(nextXLoc>=firmPtr->loc_x1-1 && nextXLoc<=firmPtr->loc_x2+1 &&
				nextYLoc>=firmPtr->loc_y1-1 && nextYLoc<=firmPtr->loc_y2+1) // hard code 1 for carvan size 1x1
				journey_status = SURROUND_FIRM;

			if(nextXLoc==move_to_x_loc && nextYLoc==move_to_y_loc && !ignore_power_nation)
				ignore_power_nation = 1;

			return;
		}
	}

	short unitRecno = sprite_recno;

	err_when(cur_action==SPRITE_ATTACK || action_mode==ACTION_ATTACK_UNIT || action_mode==ACTION_ATTACK_FIRM ||
				action_mode==ACTION_ATTACK_TOWN || action_mode==ACTION_ATTACK_WALL);

	if(unit_array.is_deleted(unitRecno))
		return; //-***************** BUGHERE ***************//

	if(firm_array.is_deleted(stopPtr->firm_recno))
	{
		update_stop_list();

		if(stop_defined_num) // move to next stop
		{
			Firm *firmPtr = firm_array[stop_array[stop_defined_num-1].firm_recno];
			move_to_firm_surround(firmPtr->loc_x1, firmPtr->loc_y1, sprite_info->loc_width, sprite_info->loc_height, firmPtr->firm_id);
		}
		return;
	}

	//CaravanStop *stopPtr = stop_array + dest_stop_id - 1;
	Firm	*firmPtr = firm_array[stopPtr->firm_recno];

	int nextXLoc = next_x_loc();
	int nextYLoc = next_y_loc();

	if(journey_status==SURROUND_FIRM ||
		( nextXLoc==move_to_x_loc && nextYLoc==move_to_y_loc && cur_x==next_x && cur_y==next_y && // move in a tile exactly
		  (nextXLoc>=firmPtr->loc_x1-1 && nextXLoc<=firmPtr->loc_x2+1 &&
			nextYLoc>=firmPtr->loc_y1-1 && nextYLoc<=firmPtr->loc_y2+1) )) // in the surrounding of the firm
	{
		//-------------------- update pick_up_array --------------------//
		stopPtr->update_pick_up();

		//-------------------------------------------------------//
		// load/unload goods
		//-------------------------------------------------------//
		if(nation_array[nation_recno]->get_relation(firmPtr->nation_recno)->trade_treaty)
		{
			switch(firmPtr->firm_id)
			{
				case FIRM_MINE:
						mine_load_goods(stopPtr->pick_up_type);
						break;

				case FIRM_FACTORY:
						factory_unload_goods();
						factory_load_goods(stopPtr->pick_up_type);
						break;

				case FIRM_MARKET:
						market_unload_goods();

						if(stopPtr->pick_up_type == AUTO_PICK_UP)
							market_auto_load_goods();
						else if(stopPtr->pick_up_type!=NO_PICK_UP)
							market_load_goods();
						break;

				default: err_here();
							break;
			}
		}

		//-------------------------------------------------------//
		// action_para is used to store the firm_recno of the market
		// where the caravan move in.
		//-------------------------------------------------------//
		action_para = stopPtr->firm_recno;

		stop_x_loc = move_to_x_loc; // store entering location
		stop_y_loc = move_to_y_loc;
		wait_count = MAX_CARAVAN_WAIT_TERM;		// set waiting term

		reset_path();
		deinit_sprite(1);	// the caravan enters the market now. 1-keep it selected if it is currently selected

		err_when(cur_x!=-1);
		cur_x--;	// set cur_x to -2, such that invisible but still process pre_process()

		journey_status = INSIDE_FIRM;
	}
	else
	{
		if(cur_action!=SPRITE_MOVE)
		{
			//----------------------------------------------------//
			// blocked by something, go to the destination again
			// note: if return value is 0, cannot reach the firm.		//*********BUGHERE
			//----------------------------------------------------//
			move_to_firm_surround(firmPtr->loc_x1, firmPtr->loc_y1, sprite_info->loc_width, sprite_info->loc_height, firmPtr->firm_id);
			journey_status = ON_WAY_TO_FIRM;
		}
	}
}
//----------- End of function UnitCaravan::caravan_on_way -----------//


//--------- Begin of function UnitCaravan::appear_in_firm_surround ---------//
//
// This function return 1 if a suitable location is found, that means the
// caravan will leave the firm there. Otherwise, return 0.
//
// xLoc, yLoc are reference variables for returning the location found.
//
int UnitCaravan::appear_in_firm_surround(int& xLoc, int& yLoc, Firm* firmPtr)
{
	int upperLeftBoundX	= firmPtr->loc_x1 - 1;	// the surrounding coordinates of the firm
	int upperLeftBoundY	= firmPtr->loc_y1 - 1;
	int lowerRightBoundX = firmPtr->loc_x2 + 1;
	int lowerRightBoundY = firmPtr->loc_y2 + 1;

	int count = 1, inside = 1, found = 0, i;
	int testXLoc = xLoc;
	int testYLoc = yLoc;
	int limit;
	Location *locPtr;

	//---------------------------------------------------------//
	//		9  10  11  12		the location is tested in the order
	//		8   1   2  13		shown, if the location is the surrounding
	//		7   x   3  14		of the firm and non-blocked, break
	//		6   5   4  ...		the test
	//---------------------------------------------------------//

	while(inside)
	{
		inside = 0;
		limit = count<<1;
		err_when(limit!=count*2);

		//------------ upper --------------//
		testXLoc = xLoc - count + 1;
		testYLoc = yLoc - count;
		for(i=0; i<limit; i++)
		{
			if(testXLoc<0 || testYLoc>=MAX_WORLD_X_LOC || testYLoc<0 || testYLoc>=MAX_WORLD_Y_LOC)
				continue;

			if(testXLoc<upperLeftBoundX || testXLoc>lowerRightBoundX || testYLoc<upperLeftBoundY || testYLoc>lowerRightBoundY)
				continue;

			locPtr = world.get_loc(testXLoc, testYLoc);
			if(locPtr->can_move(mobile_type))
			{
				found++;
				break;
			}
			else
				xLoc++;

			inside++;
		}

		if(found)
			break;

		//------------ right --------------//
		testXLoc = xLoc + count;
		testYLoc = yLoc - count + 1;
		for(i=0; i<limit; i++)
		{
			if(testXLoc<0 || testYLoc>=MAX_WORLD_X_LOC || testYLoc<0 || testYLoc>=MAX_WORLD_Y_LOC)
				continue;

			if(testXLoc<upperLeftBoundX || testXLoc>lowerRightBoundX || testYLoc<upperLeftBoundY || testYLoc>lowerRightBoundY)
				continue;

			locPtr = world.get_loc(testXLoc, testYLoc);
			if(locPtr->can_move(mobile_type))
			{
				found++;
				break;
			}
			else
				yLoc++;

			inside++;
		}

		if(found)
			break;

		//------------- down --------------//
		testXLoc = xLoc + count - 1;
		testYLoc = yLoc + count;
		for(i=0; i<limit; i++)
		{
			if(testXLoc<0 || testYLoc>=MAX_WORLD_X_LOC || testYLoc<0 || testYLoc>=MAX_WORLD_Y_LOC)
				continue;

			if(testXLoc<upperLeftBoundX || testXLoc>lowerRightBoundX || testYLoc<upperLeftBoundY || testYLoc>lowerRightBoundY)
				continue;

			locPtr = world.get_loc(testXLoc, testYLoc);
			if(locPtr->can_move(mobile_type))
			{
				found++;
				break;
			}
			else
				xLoc--;

			inside++;
		}

		if(found)
			break;

		//------------- left --------------//
		testXLoc = xLoc - count;
		testYLoc = yLoc + count - 1;
		for(i=0; i<limit; i++)
		{
			if(testXLoc<0 || testYLoc>=MAX_WORLD_X_LOC || testYLoc<0 || testYLoc>=MAX_WORLD_Y_LOC)
				continue;

			if(testXLoc<upperLeftBoundX || testXLoc>lowerRightBoundX || testYLoc<upperLeftBoundY || testYLoc>lowerRightBoundY)
				continue;

			locPtr = world.get_loc(testXLoc, testYLoc);
			if(locPtr->can_move(mobile_type))
			{
				found++;
				break;
			}
			else
				yLoc--;

			inside++;
		}

		if(found)
			break;

		//---------------------------------------------//
		count++;
	}

	if(found)
	{
		xLoc = testXLoc;
		yLoc = testYLoc;
		return 1;
	}

	return 0;
}
//----------- End of function UnitCaravan::appear_in_firm_surround -----------//


//---------- begin static function i_disp_caravan_select_button ---------------//
static void i_disp_caravan_select_button(ButtonCustom *button, int repaintBody)
{
	int x1 = button->x1;
	int y1 = button->y1;
	int x2 = button->x2;
	int y2 = button->y2;
	int shift;

	//------------- modify x1,y1, x2,y2 to the button body --------------//
	if(button->pushed_flag)
	{
		int colorDown = Vga::active_buf->color_down;		// change the color of the body area to yellow to highlight the change
		Vga::active_buf->color_down = (char) V_YELLOW;

		Vga::active_buf->d3_panel_down(x1, y1, x2, y2);

		Vga::active_buf->color_down = (char) colorDown;

		x1++;
		y1++;
		shift = 2;
	}
	else
	{
		Vga::active_buf->d3_panel_up(x1, y1, x2, y2);
		x2--;
		y2--;
		shift = 3;
	}

	//-------------- put goods icon ---------------//

	int id = button->custom_para.value;
	const char *iconName=NULL;

	int x = x1+shift;
	int y = y1+shift;

	if(id==AUTO_PICK_UP)
	{
		iconName = "AUTOPICK";
	}
	else if(id==NO_PICK_UP)
	{
		iconName = "NOPICK";
	}
	else if(id>=PICK_UP_RAW_FIRST && id<=PICK_UP_RAW_LAST)
	{
		raw_res.put_small_raw_icon( x, y, id-PICK_UP_RAW_FIRST+1 );
	}
	else if(id>=PICK_UP_PRODUCT_FIRST && id<=PICK_UP_PRODUCT_LAST)
	{
		raw_res.put_small_product_icon( x, y, id-PICK_UP_PRODUCT_FIRST+1 );
	}
	else
		err_here();

	if( iconName )
	{
		help.set_help( x, y, x+9, y+9, iconName );
		Vga::active_buf->put_bitmap_trans( x, y, image_icon.get_ptr(iconName) );
	}
}
//---------- end static function i_disp_caravan_select_button ---------------//


//--------- Begin of function UnitCaravan::has_pick_up_type ---------//
//
// Return whether the specific stop has the specific pick up types.
// 
int UnitCaravan::has_pick_up_type(int stopId, int pickUpType)
{
	return stop_array[stopId-1].pick_up_array[pickUpType-1];
}
//---------- End of function UnitCaravan::has_pick_up_type ----------//


//--------- Begin of function UnitCaravan::carrying_qty ---------//
//
// Return whether the qty of the specific product/raw type that
// this caravan is currently carrying.
//
int UnitCaravan::carrying_qty(int pickUpType)
{
	if( pickUpType >= PICK_UP_RAW_FIRST &&
		 pickUpType <= PICK_UP_RAW_LAST )
	{
		return raw_qty_array[pickUpType-PICK_UP_RAW_FIRST];
	}
	else if( pickUpType >= PICK_UP_PRODUCT_FIRST &&
				pickUpType <= PICK_UP_PRODUCT_LAST )
	{
		return product_raw_qty_array[pickUpType-PICK_UP_PRODUCT_FIRST];
	}
	else
	{
		err_here();
		return 0;
	}
}
//---------- End of function UnitCaravan::carrying_qty ----------//


//--------- Begin of function UnitCaravan::copy_route ---------//
//
// Copies trade route from copyUnitRecno to this caravan.
//
void UnitCaravan::copy_route(short copyUnitRecno, int remoteAction)
{
	if( sprite_recno == copyUnitRecno )
		return;

	UnitCaravan* copyUnit = (UnitCaravan*)unit_array[copyUnitRecno];

	if( copyUnit->nation_recno != nation_recno )
		return;

	if( remote.is_enable() && !remoteAction )
	{
		// packet structure : <unit recno> <copy recno>
		short *shortPtr = (short *)remote.new_send_queue_msg(MSG_U_CARA_COPY_ROUTE, 2*sizeof(short));
		*shortPtr = sprite_recno;

		shortPtr[1] = copyUnitRecno;
		return;
	}

	// clear existing stops
	int num_stops = stop_defined_num;
	for( int i=0; i<num_stops; i++ )
		del_stop(1, COMMAND_AUTO); // stop ids shift up

	CaravanStop* caravanStopA = copyUnit->stop_array;
	CaravanStop* caravanStopB = stop_array;
	for( int i=0; i<MAX_STOP_FOR_CARAVAN; i++, caravanStopA++, caravanStopB++ )
	{
		if( !caravanStopA->firm_recno )
			break;

		if( firm_array.is_deleted(caravanStopA->firm_recno) )
			continue;

		Firm* firmPtr = firm_array[caravanStopA->firm_recno];
		set_stop(i+1, caravanStopA->firm_loc_x1, caravanStopA->firm_loc_y1, COMMAND_AUTO);

		if( caravanStopA->pick_up_type == AUTO_PICK_UP )
		{
			set_stop_pick_up(i+1, AUTO_PICK_UP, COMMAND_AUTO );
		}

		else if( caravanStopA->pick_up_type == NO_PICK_UP )
		{
			set_stop_pick_up(i+1, NO_PICK_UP, COMMAND_AUTO );
		}

		else
		{
			for( int b=0; b<MAX_PICK_UP_GOODS; ++b )
			{
				if( caravanStopA->pick_up_array[b] != caravanStopB->pick_up_array[b] )
					set_stop_pick_up(i+1, b+1, COMMAND_PLAYER);
			}
		}
	}
}
//---------- End of function UnitCaravan::copy_route ----------//
