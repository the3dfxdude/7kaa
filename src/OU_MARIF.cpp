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

//Filename   : OU_MARI2.CPP
//Description: UnitMarine - functions for displaying info.

#include <OVGA.h>
#include <vga_util.h>
#include <OINFO.h>
#include <OHELP.h>
#include <OMOUSE.h>
#include <OCONFIG.h>
#include <ORACERES.h>
#include <OIMGRES.h>
#include <ORAWRES.h>
#include <OPOWER.h>
#include <OFONT.h>
#include <OBUTTON.h>
#include <OBUTT3D.h>
#include <OREMOTE.h>
#include <OU_CARA.h>
#include <OU_MARI.h>
#include <ONATION.h>
#include <OF_MINE.h>
#include <OF_FACT.h>
#include <OBUTTCUS.h>
#include <OSE.h>
#include <OF_HARB.h>
#include "gettext.h"

#ifdef DEBUG
#include <stdio.h>
#endif

//--------- Define static vars ----------//

static short  	   	unit_disp_y1, unit_info_disp_y1, stop_disp_y1;
static Button3D		button_unload_all;
static ButtonGroup	button_mode(2);
static Button			button_auto_trade;
static Button 			button_set_stop[MAX_STOP_FOR_SHIP];
static Button 			button_go_stop[MAX_STOP_FOR_SHIP];
static Button 			button_cancel_stop[MAX_STOP_FOR_SHIP];
//static Button			button_select_attack;
static ButtonCustom	button_select_array[MAX_STOP_FOR_SHIP][MAX_GOODS_SELECT_BUTTON];

static void				i_disp_marine_select_button(ButtonCustom *button, int repaintBody);

//--------- Begin of function UnitMarine::disp_info ---------//
//
void UnitMarine::disp_info(int refreshFlag)
{
	disp_basic_info(INFO_Y1, refreshFlag);

	if( !should_show_info() )
		return;

	//---- display the switch between the units and goods menu ----//

	UnitInfo* unitInfo = unit_res[unit_id];

	int y=INFO_Y1+54;

	if( unitInfo->carry_unit_capacity && unitInfo->carry_goods_capacity )
	{
		if( refreshFlag == INFO_REPAINT )
		{
			vga_util.d3_panel_up( INFO_X1, y, INFO_X2, y+22 );

			button_mode[0].create_text( INFO_X1+5, y+3, INFO_X1+80, y+19, _("Units") );
			button_mode[1].create_text( INFO_X1+90, y+3, INFO_X1+155, y+19, _("Goods") );
			button_mode.paint(menu_mode);

			button_auto_trade.paint_text( INFO_X1+165, y+3, INFO_X2-10, y+19, auto_mode ? (char*)"T" : (char*)"C");
		}

		y += 25;
	}

	//-------------------------------------------------------------//

	switch( menu_mode )
	{
		case SHIP_MENU_GOODS:
			disp_goods_menu(y, refreshFlag);
			break;

		case SHIP_MENU_UNIT:
			disp_unit_menu(y, refreshFlag);
			break;
	}
}
//----------- End of function UnitMarine::disp_info -----------//


//--------- Begin of function UnitMarine::detect_info ---------//
//
void UnitMarine::detect_info()
{
	if(!is_visible())
		return;

	if( detect_basic_info() )
		return;

	if( detect_select_hotkey() )
		return;

	if( !is_own() )
		return;

	//----- detect switching the menu mode -----//

	UnitInfo* unitInfo = unit_res[unit_id];

	if( unitInfo->carry_unit_capacity && unitInfo->carry_goods_capacity )
	{
		int rc;

		if( (rc=button_mode.detect()) >= 0 )
		{
			menu_mode = rc;
			info.disp();
			return;
		}
	}

	if( !is_own() && !config.show_ai_info)
		return;

	//--------- detect menu mode --------//

	switch( menu_mode )
	{
		case SHIP_MENU_GOODS:
			detect_goods_menu();
			break;

		case SHIP_MENU_UNIT:
			detect_unit_menu();
			break;
	}

	//---- detect toggling auto trade mode ----//

	if( button_auto_trade.detect() )
	{
		if( !remote.is_enable() )
		{
			auto_mode = !auto_mode;
			button_auto_trade.paint_text( INFO_X1+165, INFO_Y1+57, INFO_X2-10, INFO_Y1+73, auto_mode ? (char*)"T" : (char*)"C");
		}
		else
		{
			// packet structure <unit recno> <new mode>
			short *shortPtr = (short *)remote.new_send_queue_msg(MSG_U_SHIP_CHANGE_MODE, 2*sizeof(short) );
			*shortPtr = sprite_recno;
			shortPtr[1] = !auto_mode;
		}
	}
}
//----------- End of function UnitMarine::detect_info -----------//


//--------- Begin of function UnitMarine::is_in_build_menu ---------//
// Returns true if a unit is currently in build mode.
// Only reliable if this unit is the selected unit.
// Used by Info to detect if the build mode is opened.
//
bool UnitMarine::is_in_build_menu()
{
	return false;
}
//----------- End of function UnitMarine::is_in_build_menu -----------//


//-------- Begin of function UnitMarine::should_show_info ------//
//
int UnitMarine::should_show_info()
{
	if( config.show_ai_info || nation_recno==nation_array.player_recno )
		return 1;

	//--- if any of the units on the ship are spies of the player ---//

	for( int i=0 ; i<unit_count ; i++ )
	{
		if( unit_array[ unit_recno_array[i] ]->is_own() )
			return 1;
	}

	return 0;
}
//---------- End of function UnitMarine::should_show_info --------//


//--------- Begin of function UnitMarine::disp_unit_menu ---------//
//
void UnitMarine::disp_unit_menu(int dispY1, int refreshFlag)
{
	disp_unit_list(dispY1   , refreshFlag);
	disp_unit_info(dispY1+90, refreshFlag);

	if( !is_own() )
		return;

	if( refreshFlag==INFO_REPAINT )
		button_unload_all.paint( INFO_X1, dispY1+165, 'A', "OUTSHIP" );

	if( can_unload_unit() )
		button_unload_all.enable();
	else
		button_unload_all.disable();

//	if( refreshFlag == INFO_REPAINT )
//		button_select_attack.paint_text( INFO_X2-50, INFO_Y1+180, INFO_X2-5, INFO_Y1+200, "0" );
}
//----------- End of function UnitMarine::disp_unit_menu -----------//


//--------- Begin of function UnitMarine::can_unload_unit ---------//
//
int UnitMarine::can_unload_unit()
{
	err_when(cur_action==SPRITE_ATTACK && (cur_x!=next_x || cur_y!=next_y));

	return unit_count>0 &&
			(cur_action==SPRITE_IDLE || cur_action==SPRITE_ATTACK) &&
			 is_on_coast();
}
//----------- End of function UnitMarine::can_unload_unit -----------//


//--------- Begin of function UnitMarine::detect_unit_menu ---------//
//
void UnitMarine::detect_unit_menu()
{
	//------- detect clicking on the units --------//

	if( detect_unit_list() )
		disp_unit_info(unit_info_disp_y1, INFO_UPDATE);

	if( !is_own() )
		return;

	//----------- detect the unload all button -----------//

	if( button_unload_all.detect('R') )
	{
		unload_all_units(COMMAND_PLAYER);
		info.disp();
		// ##### begin Gilbert 25/9 ######//
		se_ctrl.immediate_sound("TURN_ON");
		// ##### end Gilbert 25/9 ######//
	}
/*
	if(button_select_attack.detect())
	{
		se_ctrl.immediate_sound("TURN_ON", 100, 0);
		select_attack_weapon();
	}
*/
}
//----------- End of function UnitMarine::detect_unit_menu -----------//


//--------- Begin of function UnitMarine::select_attack_weapon ---------//
void UnitMarine::select_attack_weapon()
{
	// ###### begin Gilbert 25/6 ########//
	if( attack_count == 0 )                 // TRANSPORT can't select attack weapon
		return;
	// ###### end Gilbert 25/6 ########//

	char oldAttackRange = attack_info_array[0].attack_range;
	if(attack_mode_selected>unit_count)
		attack_mode_selected = 0;
	else
	{
		Unit *unitPtr;
		int found = 0;
		for(int i=attack_mode_selected+1; i<=unit_count; i++)
		{
			unitPtr = unit_array[unit_recno_array[i-1]];
			if(unitPtr->attack_count && unit_res[unitPtr->unit_id]->unit_class==UNIT_CLASS_WEAPON)
			{
				//ship_attack_info = *unit_res.get_attack_info(unit_res[unitPtr->unit_id]->first_attack);
				ship_attack_info = *unitPtr->attack_info_array;
				ship_attack_info.eqv_attack_next = 0;
				ship_attack_info.bullet_out_frame = unit_res.get_attack_info(unit_res[unit_id]->first_attack)->bullet_out_frame;
				attack_count = 1;
				attack_info_array = &ship_attack_info;
				found++;
				attack_mode_selected = i;
				//cur_action = SPRITE_READY_TO_MOVE;
				break;
			}
		}

		if(!found)
			attack_mode_selected = 0;
	}

	if(attack_mode_selected==0)
	{
		ship_attack_info = *unit_res.get_attack_info(unit_res[unit_id]->first_attack);
		attack_count = 1;
		attack_info_array = &ship_attack_info;
	}

	//-------- update attacking if neccessary --------//
	if(attack_info_array[0].attack_range<oldAttackRange)
	{
		short attackXLoc, attackYLoc, attackPara;
		switch(action_mode)
		{
			case ACTION_ATTACK_UNIT:
					attackPara = action_para;
					stop2();
					attack_unit(attackPara);
					break;

			case ACTION_ATTACK_FIRM:
					attackXLoc = action_x_loc;
					attackYLoc = action_y_loc;
					stop2();
					attack_firm(attackXLoc, attackYLoc);
					break;

			case ACTION_ATTACK_TOWN:
					attackXLoc = action_x_loc;
					attackYLoc = action_y_loc;
					stop2();
					attack_town(attackXLoc, attackYLoc);
					break;

			case ACTION_ATTACK_WALL:
					attackXLoc = action_x_loc;
					attackYLoc = action_y_loc;
					stop2();
					attack_wall(attackXLoc, attackYLoc);
					break;
		}
	}

//	button_select_attack.paint_text( INFO_X2-50, INFO_Y1+180, INFO_X2-5, INFO_Y1+200, misc.format(attack_mode_selected));
}
//----------- End of function UnitMarine::select_attack_weapon -----------//


//--------- Begin of function UnitMarine::disp_unit_list ---------//
//
void UnitMarine::disp_unit_list(int dispY1, int refreshFlag)
{
	unit_disp_y1 = dispY1;

	//---------------- paint the panel --------------//

	if( refreshFlag == INFO_REPAINT )
		vga_util.d3_panel_up( INFO_X1, dispY1, INFO_X2, dispY1+88 );

	//------ display population composition -------//

	int	  x, y;
	Unit*   unitPtr;
	static  short last_unit_id_array[MAX_UNIT_IN_SHIP];

	if( selected_unit_id > unit_count )
		selected_unit_id = 0;

	dispY1+=4;

	for( int i=0 ; i<MAX_UNIT_IN_SHIP ; i++ )
	{
		x = INFO_X1+6+i%3*66;
		y = dispY1+i/3*28;

		if( i<unit_count )
		{
			unitPtr = unit_array[ unit_recno_array[i] ];

			if( refreshFlag==INFO_REPAINT || last_unit_id_array[i] != unitPtr->unit_id )
			{
				vga_front.d3_panel_up( x, y, x+27, y+23, 1 );
				// ###### begin Gilbert 17/10 ########//
				vga_front.put_bitmap(x+2, y+2, unit_res[unitPtr->unit_id]->get_small_icon_ptr(unitPtr->rank_id));
				// ###### end Gilbert 17/10 ########//
			}

			//----- highlight the selected unit -------//

			if( selected_unit_id == i+1 )
				vga_front.rect( x-2, y-2, x+29, y+25, 2, V_YELLOW );
			else
				vga_front.rect( x-2, y-2, x+29, y+25, 2, vga_front.color_up );

			//---------- display hit point ----------//

			font_san.disp(x+32, y+6, (int) unitPtr->hit_points, 1, x+61);

			last_unit_id_array[i] = unitPtr->unit_id;

			//------- set help parameters ---------//

			if( mouse.in_area(x, y, x+27, y+23) )
				help.set_unit_help( unitPtr->unit_id, unitPtr->rank_id, x, y, x+27, y+23 );
		}
		else
		{
			if( last_unit_id_array[i] != 0 )
			{
				vga_util.blt_buf( x-2, y-2, x+49, y+25, 0 );
				last_unit_id_array[i] = 0;
			}
		}
	}
}
//----------- End of function UnitMarine::disp_unit_list -----------//


//--------- Begin of function UnitMarine::detect_unit_list ---------//
//
int UnitMarine::detect_unit_list()
{
	//------- detect buttons on hiring firm units -------//

	int i, x, y;

	for( i=0 ; i<unit_count ; i++ )
	{
		x = INFO_X1+6+i%3*66;
		y = unit_disp_y1+4+i/3*28;

		//---------------------------------//

		if( mouse.press_area(x, y, x+27, y+23) )	// left click to select unit
		{
			selected_unit_id = i+1;
			return 1;
		}
		else if( mouse.any_click(x, y, x+27, y+23, 1) )			// 1-right button. right click to call out unit
		{
			mouse.reset_click();		// reset queued mouse click for fast single clicking

			unload_unit( i+1, COMMAND_PLAYER );
			info.disp();
			return 1;
		}
	}

	return 0;
}
//----------- End of function UnitMarine::detect_unit_list -----------//


//--------- Begin of function UnitMarine::disp_unit_info ---------//
//
void UnitMarine::disp_unit_info(int dispY1, int refreshFlag)
{
	static int lastSelected;

	unit_info_disp_y1 = dispY1;

	if( selected_unit_id > unit_count )
		selected_unit_id = unit_count;

	//---------------- paint the panel --------------//

	if( refreshFlag == INFO_REPAINT )
	{
		vga_util.d3_panel_up( INFO_X1, dispY1, INFO_X2, dispY1+71 );
	}
	else
	{
		if( lastSelected != selected_unit_id > 0 )
		{
			lastSelected = selected_unit_id > 0;
			vga_util.blt_buf( INFO_X1, dispY1, INFO_X2, dispY1+71, 0 );
		}
	}

	//-----------------------------------------------//

	if( selected_unit_id > 0 )
	{
		int x=INFO_X1+4, y=dispY1+4, x1=x+100;

		Unit* unitPtr = unit_array[ unit_recno_array[selected_unit_id-1] ];

		if( unitPtr->race_id && unitPtr->rank_id != RANK_KING )
			info.disp_loyalty( x, y, x1, unitPtr->loyalty, unitPtr->target_loyalty, nation_recno, refreshFlag);
		else
			font_san.field( x, y, _("Loyalty"), x1, _("N/A"), INFO_X2-2, refreshFlag );	// no loyalty because it does not belong to your empire

		y+=16;

		font_san.field( x, y, _("Combat"), x1, unitPtr->skill.combat_level, 1, INFO_X2-2, refreshFlag);
		y+=16;

		//----------------------------------------------//

		String str;
		str  = (int) unitPtr->hit_points;
		str += "/";
		str += unitPtr->max_hit_points;

		font_san.field( x, y, _("Hit Points"), x1, str, INFO_X2-2, refreshFlag);
		y += 16;

		//----------------------------------------------//

		if( unitPtr->skill.skill_id )
		{
			if( refreshFlag == INFO_REPAINT )
				font_san.field( x, y, unitPtr->skill.skill_des(), x1, unitPtr->skill.skill_level , 1, INFO_X2-2, refreshFlag );
			else
			{
				font_san.put( x+2, y+2, unitPtr->skill.skill_des(), 1, x1-2 );
				font_san.update_field( x1, y, unitPtr->skill.skill_level, 1, INFO_X2-10);
			}
		}
		else
		{
			if( refreshFlag == INFO_REPAINT )
				font_san.field( x, y, "", x1, "", INFO_X2-2, refreshFlag );
			else
			{
				font_san.put( x+2, y+2, "", 1, x1-2 );
				font_san.update_field( x1, y, "", INFO_X2-10);
			}
		}
	}
}
//----------- End of function UnitMarine::disp_unit_info -----------//


//--------- Begin of function UnitMarine::disp_goods_menu ---------//
//
void UnitMarine::disp_goods_menu(int dispY1, int refreshFlag)
{
	disp_stop(dispY1, refreshFlag);

	disp_goods(dispY1+180, refreshFlag);
}
//----------- End of function UnitMarine::disp_goods_menu -----------//


//--------- Begin of function UnitMarine::detect_goods_menu ---------//
//
void UnitMarine::detect_goods_menu()
{
	detect_stop();
}
//---------- End of function UnitMarine::detect_goods_menu ----------//


//--------- Begin of function UnitMarine::disp_stop ---------//
//
void UnitMarine::disp_stop(int dispY1, int refreshFlag)
{
//###### begin trevor 3/10 #######//

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
				button_set_stop[i].paint_text( x+4, y+37, x+90, y+56, "Faire Escale" );
#else
				button_set_stop[i].paint_text( x+4, y+37, x+80, y+56, _("Set Stop") );
#endif
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
				button_set_stop[i].paint_text( x+4, y+37, x+90, y+56, "Faire Escale" );
#else
				button_set_stop[i].paint_text( x+4, y+37, x+80, y+56, _("Set Stop") );
#endif

				button_set_stop[i].set_help_code( "SSETSTOP" );

#if(defined(FRENCH))
				button_go_stop[i].paint_text( x+94, y+37, x+180, y+56, "Voir Escale" );
#else
				button_go_stop[i].paint_text( x+84, y+37, x+180, y+56, _("View Stop") );
#endif
				button_go_stop[i].set_help_code( "SGOSTOP" );

				button_cancel_stop[i].paint_text( x+184, y+37, INFO_X2-4, y+56, "X" );
				button_cancel_stop[i].set_help_code( "SDELSTOP" );
			}

			disp_goods_select_button(i, y+1, refreshFlag);
		}
	}
//###### end trevor 3/10 #######//
}
//---------- End of function UnitMarine::disp_stop ----------//


//------ Begin of function UnitMarine::disp_goods_select_button -------//
//
void UnitMarine::disp_goods_select_button(int stopNum, int dispY1, int refreshFlag)
{
	if(refreshFlag!=INFO_REPAINT && refreshFlag!=INFO_UPDATE)
		return;

	#define SHIFT_X_OFFSET	73
	#define SELECT_BUTTON_WIDTH	16
	#define SELECT_BUTTON_HEIGHT	16

	ShipStop	*stopPtr = &stop_array[stopNum];
	Firm *harborPtr = firm_array[stopPtr->firm_recno];

	int  x=INFO_X1+SHIFT_X_OFFSET, y=dispY1+17, x1, pick_up_goods = 0;
	char *pickUpArray = stopPtr->pick_up_array;
	char isPush;

	//###### begin trevor 3/10 #######//

	for(int i=1 ;i<=MAX_PICK_UP_GOODS; ++i, pickUpArray++)
	{
		int rawId = i;
		if( rawId < 1 || rawId > MAX_RAW )
			rawId = 0;
		int productId = i-MAX_RAW;
		if( productId < 1 || productId > MAX_PRODUCT )
			productId = 0;

		int stock = -1;

		for(int j=harborPtr->linked_firm_count-1; j>=0 && stock<0; --j)
		{
			err_when(firm_array.is_deleted(harborPtr->linked_firm_array[j]));
			Firm *firmPtr = firm_array[harborPtr->linked_firm_array[j]];
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
		}

		x1 = x + i*SELECT_BUTTON_WIDTH;

		if( stock >= 0 )
		{
			isPush = stopPtr->pick_up_array[i-1];
			err_when(isPush && (stopPtr->pick_up_type==AUTO_PICK_UP || stopPtr->pick_up_type==NO_PICK_UP));

			button_select_array[stopNum][i].paint(x1, y, x1+SELECT_BUTTON_WIDTH,
				y+SELECT_BUTTON_HEIGHT, i_disp_marine_select_button, ButtonCustomPara(this, i),
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
			y+SELECT_BUTTON_HEIGHT, i_disp_marine_select_button, ButtonCustomPara(this, AUTO_PICK_UP),
			0, isPush); // 0 for inelastic

		x1 = x+SELECT_BUTTON_WIDTH*NO_PICK_UP;
		button_select_array[stopNum][NO_PICK_UP].paint(x1, y, x1+SELECT_BUTTON_WIDTH,
			y+SELECT_BUTTON_HEIGHT, i_disp_marine_select_button, ButtonCustomPara(this, NO_PICK_UP));
	}
	else
	{
		x1 = x;
		vga_util.blt_buf( x1, y, x1+SELECT_BUTTON_WIDTH, y+SELECT_BUTTON_HEIGHT, 0 );

		x1 = x+SELECT_BUTTON_WIDTH*NO_PICK_UP;
		vga_util.blt_buf( x1, y, x1+SELECT_BUTTON_WIDTH, y+SELECT_BUTTON_HEIGHT, 0 );
	}

	//###### end trevor 3/10 #######//
}
//---------- End of function UnitMarine::disp_goods_select_button ----------//


//--------- Begin of function UnitMarine::detect_stop ---------//
//
void UnitMarine::detect_stop()
{
	int i, x=INFO_X1, y=INFO_Y1+54+25;

	for( i=0 ; i<MAX_STOP_FOR_SHIP ; i++, y+=38 )
	{
		if( button_set_stop[i].detect() && is_own() )
			power.issue_command( COMMAND_SET_SHIP_STOP, sprite_recno, i+1 );		// i+1 - stop id., passed as a parameter of the command

		if(i>=stop_defined_num)
			continue;

		if(button_cancel_stop[i].detect())
		{
			if(is_visible())
			{
				del_stop(i+1, COMMAND_PLAYER);
				// ##### begin Gilbert 25/9 ######//
				se_ctrl.immediate_sound("TURN_ON");
				// ##### end Gilbert 25/9 ######//
			}
		}

		for(int b=0; b<MAX_GOODS_SELECT_BUTTON; ++b)
		{
			if(button_select_array[i][b].detect())
			{
				// ###### begin Gilbert 25/9 #######//
				se_ctrl.immediate_sound( 
					button_select_array[i][b].elastic_flag || button_select_array[i][b].pushed_flag ?
					(char*)"TURN_ON" : (char*)"TURN_OFF");
				// ###### end Gilbert 25/9 #######//
				set_stop_pick_up(i+1, b, COMMAND_PLAYER); // b = 1 - MAX_PICK_UP_GOODS
			}
		}

		if( button_go_stop[i].detect() )
		{
			Firm* firmPtr = firm_array[stop_array[i].firm_recno];
			world.go_loc(firmPtr->center_x, firmPtr->center_y);
		}
	}
}
//---------- End of function UnitMarine::detect_stop ----------//


//--------- Begin of function UnitMarine::set_stop_pick_up ---------//
//
// Set the pickup type of a specific stop of this marine.
//
// <int> stopId		  - id. of the stop.  (1 - MAX_STOP_FOR_SHIP)
// <int> newPickUpType - set the pickup type of the specific stop. (0 - MAX_GOODS_SELECT_BUTTON-1)
// <int> remoteActoin  - remote action type
//
void UnitMarine::set_stop_pick_up(int stopId, int newPickUpType, int remoteAction)
{
	if(remote.is_enable())
	{
		if(!remoteAction)
		{
			// packet structure : <unit recno> <stop id> <new pick_up_type>
			short *shortPtr = (short *)remote.new_send_queue_msg(MSG_U_SHIP_CHANGE_GOODS, 3*sizeof(short));
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
			OutputDebugString(mess);

			Firm *firmPtr = firm_array[stop_array[stopId-1].firm_recno];
			
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
				if(!remote.is_enable() || nation_recno==nation_array.player_recno || config.show_ai_info)
				{
					int y=INFO_Y1+54;
					UnitInfo* unitInfo = unit_res[unit_id];
					if( unitInfo->carry_unit_capacity && unitInfo->carry_goods_capacity )
						y+=25;

					disp_stop(y, INFO_UPDATE);
				}
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

			misc.set_random_seed(stopId + newPickUpType*(misc.random(4)+1)*10 + sprite_recno*100*misc.random(100));

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

	if( unit_array.selected_recno == sprite_recno )
	{
		if(nation_recno==nation_array.player_recno || config.show_ai_info)
		{
			int y=INFO_Y1+54;
			UnitInfo* unitInfo = unit_res[unit_id];
			if( unitInfo->carry_unit_capacity && unitInfo->carry_goods_capacity )
				y+=25;

			disp_stop(y, INFO_UPDATE);
		}
	}
}
//---------- End of function UnitMarine::set_stop_pick_up ----------//


//--------- Begin of function UnitMarine::disp_goods ---------//
//
void UnitMarine::disp_goods(int dispY1, int refreshFlag)
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
//---------- End of function UnitMarine::disp_goods ----------//


//---------- begin static function i_disp_marine_select_button ---------------//

static void i_disp_marine_select_button(ButtonCustom *button, int repaintBody)
{
	int x1 = button->x1;
	int y1 = button->y1;
	int x2 = button->x2;
	int y2 = button->y2;
	int shift;

	//------------- modify x1,y1, x2,y2 to the button body ---------------//
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
//---------- end static function i_disp_marine_select_button ---------------//


//---------- Begin of function UnitMarine::fix_attack_info ----------//
void UnitMarine::fix_attack_info()
{
	Unit::fix_attack_info();

	err_when( attack_mode_selected < 0 || attack_mode_selected > unit_count );
	if( attack_count > 0 )
	{
		err_when(attack_count > 1);
		if( attack_mode_selected == 0 )
		{
			ship_attack_info = *unit_res.get_attack_info(unit_res[unit_id]->first_attack);
		}
		attack_info_array = &ship_attack_info;
	}
}
//---------- End of function UnitMarine::fix_attack_info ----------//


//--------- Begin of function UnitMarine::set_stop ---------//
// <int> stopId 				 - the id. of the stop
// <int> stopXLoc, stopYLoc - the location of the stop
//
void UnitMarine::set_stop(int stopId, int stopXLoc, int stopYLoc, char remoteAction)
{
	//-------------------------------------------------------//
	// check if there is a station in the given location
	//-------------------------------------------------------//
	Location *locPtr = world.get_loc(stopXLoc, stopYLoc);
	if(!locPtr->is_firm())
		return;

	Firm *firmPtr = firm_array[locPtr->firm_recno()];

	if( !can_set_stop( firmPtr->firm_recno ) )
		return;

	//-------------------------------------------------------//
	// return if the harbor stop is in another territory
	//-------------------------------------------------------//
	FirmHarbor	*harborPtr = (FirmHarbor*) firmPtr;

	if(world.get_loc(next_x_loc(), next_y_loc())->region_id!=harborPtr->sea_region_id)
		return;

	//-----------------------------------------//

	if(!remoteAction && remote.is_enable())
	{
		// packet structure : <unit recno> <stop id> <stop x> <stop y>
		short *shortPtr = (short *) remote.new_send_queue_msg(MSG_U_SHIP_SET_STOP, 4*sizeof(short));
		*shortPtr = sprite_recno;
		shortPtr[1] = stopId;
		shortPtr[2] = stopXLoc;
		shortPtr[3] = stopYLoc;
		return;
	}

	if(!stop_array[stopId-1].firm_recno)
		stop_defined_num++;	// no plus one if the recno is defined originally

	//-------------------------------------------------------//
	// set the station recno of the stop
	//-------------------------------------------------------//
	ShipStop *stopPtr = stop_array + stopId - 1;
	if(stopPtr->firm_recno==firmPtr->firm_recno)
	{
		err_when(stopPtr->firm_loc_x1!=firmPtr->loc_x1 || stopPtr->firm_loc_y1!=firmPtr->loc_y1);
		return; // same stop as before
	}

	short oldStopFirmRecno = dest_stop_id ? stop_array[dest_stop_id-1].firm_recno : 0; 
	stopPtr->firm_recno		= firmPtr->firm_recno;
	stopPtr->firm_loc_x1		= firmPtr->loc_x1;
	stopPtr->firm_loc_y1		= firmPtr->loc_y1;

	//-------------------------------------------------------//
	// set pick up selection based on availability
	//-------------------------------------------------------//
	stopPtr->pick_up_set_auto();

	int goodsId, goodsNum = 0;
	for(int i=harborPtr->linked_firm_count-1; i>=0 && goodsNum<2; --i)
	{
		MarketGoods *goodsPtr;
		int id = 0;
		err_when(firm_array.is_deleted(harborPtr->linked_firm_array[i]));
		firmPtr = firm_array[harborPtr->linked_firm_array[i]];

		switch(firmPtr->firm_id)
		{
		case FIRM_MINE:
			id = ((FirmMine*)firmPtr)->raw_id;
			if(id)
			{
				if(!goodsNum)
					goodsId = id;
				goodsNum++;
			}
			break;
		case FIRM_FACTORY:
			id = ((FirmFactory*)firmPtr)->product_raw_id+MAX_RAW;
			if(id)
			{
				if(!goodsNum)
					goodsId = id;
				goodsNum++;
			}
			break;
		case FIRM_MARKET:
			goodsPtr = ((FirmMarket*) firmPtr)->market_goods_array;

			for(int j=0; j<MAX_MARKET_GOODS; ++j && goodsNum<2, goodsPtr++)
			{
				if(goodsPtr->raw_id)
				{
					id = goodsPtr->raw_id;

					if(!goodsNum)
						goodsId = id;
					goodsNum++;
				}
				else if(goodsPtr->product_raw_id)
				{
					id = goodsPtr->product_raw_id+MAX_RAW;

					if(!goodsNum)
						goodsId = id;
					goodsNum++;
				}
			}
			break;
		default:
			err_here();
			break;
		}
	}

	if(goodsNum==1)
		stopPtr->pick_up_toggle(goodsId); // cancel auto_pick_up
	else if(!goodsNum)
		stopPtr->pick_up_set_none();

	//-------------------------------------------------------//
	// remove duplicate stop or stop change nation
	//-------------------------------------------------------//
	update_stop_list();

	//-------------------------------------------------------//
	// handle if current stop changed when mobile
	//-------------------------------------------------------//
	if(dest_stop_id && journey_status!=INSIDE_FIRM)
	{
		short newStopFirmRecno;
		err_when(firm_array.is_deleted(stop_array[dest_stop_id-1].firm_recno));
		if((newStopFirmRecno=stop_array[dest_stop_id-1].firm_recno) != oldStopFirmRecno)
		{
			firmPtr = firm_array[newStopFirmRecno];
			err_when(firmPtr->firm_id!=FIRM_HARBOR);
			move_to_firm_surround(firmPtr->loc_x1, firmPtr->loc_y1, sprite_info->loc_width, sprite_info->loc_height, FIRM_HARBOR);
			journey_status = ON_WAY_TO_FIRM;
		}
	}
	else if(journey_status!=INSIDE_FIRM)
		stop2();

	//-------------------------------------------------------//
	// refresh stop info area
	//-------------------------------------------------------//
	if(unit_array.selected_recno==sprite_recno)
	{
		if(nation_recno==nation_array.player_recno || config.show_ai_info)
			info.disp();
	}
}
//---------- End of function UnitMarine::set_stop ----------//
