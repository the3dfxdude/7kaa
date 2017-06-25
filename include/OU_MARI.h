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

// Filename    : OU_MARI.H
// Description : header file of sea unit

#ifndef __OU_MARI_H
#define __OU_MARI_H

#ifndef __OUNIT_H
#include <OUNIT.h>
#endif

#ifndef __F_MARK_H
#include <OF_MARK.h>
#endif

#ifndef __OU_CARA_H
#include <OU_CARA.h>
#endif

//------- Define constant ---------//

#define MAX_UNIT_IN_SHIP	   9	
#define MAX_STOP_FOR_SHIP		3		// Maximum no. of destination stations per train
#define MAX_SHIP_WAIT_TERM		8		// no. of term the ship is in the market to upload/download cargo

//---------- Define constant ------------//

enum { SHIP_MENU_UNIT,
		 SHIP_MENU_GOODS,
	  };

//-------------- define extra_move_in_beach ------------//

enum	{	NO_EXTRA_MOVE = 0,
			EXTRA_MOVING_IN,
			EXTRA_MOVING_OUT,
			EXTRA_MOVE_FINISH,
		};

//-------- Define struct ShipStop ----------//
#pragma pack(1)
struct ShipStop : public TradeStop
{
public:
	int	update_pick_up(char *enableFlag=NULL);

	//----------- multiplayer version --------------//
	void			mp_pick_up_toggle(int pos);
};
#pragma pack()

//------- Define class UnitMarine -------//

#pragma pack(1)
class UnitMarine : public Unit
{
public:
	Sprite	splash;

	char		menu_mode;				// goods menu or units menu
	char		extra_move_in_beach;
	char		in_beach;

	//------ vars for carrying units ------//

	char		selected_unit_id;
	short		unit_recno_array[MAX_UNIT_IN_SHIP];
	char 		unit_count;

	//------ vars for carrying goods ------//

	char			journey_status;				// 1 for not unload but can up load, 2 for unload but not up load
	char		 	dest_stop_id;					// destination stop id. the stop which the train currently is moving towards
	char			stop_defined_num;				// num of stop defined
	char			wait_count;						// set to -1 to indicate only one stop is specified

	short			stop_x_loc;						// the x location the unit entering the stop
	short			stop_y_loc;						// the y location the unit entering the stop

	char			auto_mode;						// indicate whether auto mode is on/off, 1 - on, 0 - off
	short			cur_firm_recno;				// the recno of current firm the ship entered
	short			carry_goods_capacity;

	ShipStop		stop_array[MAX_STOP_FOR_SHIP];	// an array of firm_recno telling train stop stations
	void			update_stop_list();
	void			update_stop_and_goods_info();
	int			get_next_stop_id(int curStopId);

	void			pre_process();
	void			set_stop_pick_up(int stopId, int newPickUpType, int remoteAction);
	void			ship_in_firm(int autoMode=1);
	void			ship_on_way();
	int			appear_in_firm_surround(int& xLoc, int& yLoc, Firm* firmPtr);
	
	void			get_harbor_linked_firm_info();
	void			harbor_unload_goods();
	void			harbor_unload_product();
	void			harbor_unload_raw();

	void			harbor_load_goods();
	void			harbor_auto_load_goods();
	void			harbor_load_product(int goodsId, int autoPickUp, int considerMode);
	void			harbor_load_raw(int goodsId, int autoPickUp, int considerMode);

	short			raw_qty_array[MAX_RAW];
	short			product_raw_qty_array[MAX_PRODUCT];
	int			total_carried_goods();

	//----------- vars for attacking ----------//

	AttackInfo	ship_attack_info;
	uint8_t			attack_mode_selected;

	//-------------- vars for AI --------------//

	int			last_load_goods_date;

public:
	UnitMarine();
	~UnitMarine();

	//------ overloaded function -------//

	void  init(int unitId, int nationRecno, int rankId, int unitLoyalty, int startX= -1, int startY= -1);
	void	init_derived();

	void  disp_info(int refreshFlag);
	void  detect_info();
	bool  is_in_build_menu();

	int 	should_show_info();

	void  draw();
	void	draw_outlined();
	void  update_abs_pos(SpriteFrame* =0);

	float	actual_damage();
	short wave_height(int =0);

	int	can_unload_unit();

	void  load_unit(int unitRecno);
	void 	unload_unit(int unitSeqId, char remoteAction);
	void	unload_all_units(char remoteAction);
	int	unloading_unit(int isAll, int unitSeqId=0);
	void	del_unit(int unitRecno);
	int 	can_set_stop(int firmRecno);

	void	extra_move();
	void	process_extra_move();

	void 	set_stop(int stopId, int stopXLoc, int stopYLoc, char remoteAction);
	void	del_stop(int stopId, char remoteAction);
	void	select_attack_weapon();

   int   is_ai_all_stop();

	int	can_resign();

	int   read_derived_file(File *);
	int   write_derived_file(File *);
	virtual void fix_attack_info();         // set attack_info_array appropriately

	//------- ai functions --------//

	void 	process_ai();
	int	think_resign();
	void 	think_del_stop();
	void 	ai_sail_to_nearby_harbor();

	void  ai_ship_being_attacked(int attackerUnitRecno);

	//-------------- multiplayer checking codes ---------------//

	virtual	uint8_t crc8();
	virtual	void	clear_ptr();

private:
	void 	disp_unit_menu(int dispY1, int refreshFlag);
	void 	detect_unit_menu();

	void 	disp_goods_menu(int dispY1, int refreshFlag);
	void 	detect_goods_menu();

	void 	disp_stop(int dispY1, int refreshFlag);
	void 	detect_stop();
	void 	disp_goods(int dispY1, int refreshFlag);
	void	disp_goods_select_button(int stopNum, int dispY1, int refreshFlag);

	void 	disp_unit_list(int dispY1, int refreshFlag);
	int 	detect_unit_list();
	void 	disp_unit_info(int dispY1, int refreshFlag);

	int	is_on_coast();
};
#pragma pack()

//---------------------------------------//

#endif
