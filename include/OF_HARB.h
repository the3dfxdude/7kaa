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

//Filename    : OF_HARB.H
//Description : Header of Firm Harbor

#ifndef __OF_HARB_H
#define __OF_HARB_H

#ifndef __OFIRM_H
#include <OFIRM.h>
#endif

//----------- Define constant --------------//

#define MAX_SHIP_IN_HARBOR		4  		// maximum no. of ships in a harbor
#define MAX_BUILD_SHIP_QUEUE	10

//------- Define class FirmHarbor --------//

struct FirmHarborCrc;
class UnitMarine;

#pragma pack(1)
class FirmHarbor : public Firm
{
public:
	short			 ship_recno_array[MAX_SHIP_IN_HARBOR];
	short			 ship_count;

	short			 build_unit_id;			// race id. of the unit the town is currently building, 0-if currently not building any
	uint32_t			 start_build_frame_no;

	char			 build_queue_array[MAX_BUILD_SHIP_QUEUE];	// it stores the unit id.
	char			 build_queue_count;

	uint8_t			 land_region_id;
	uint8_t			 sea_region_id;

	//----------- for harbor trading ------------//

	char			 link_checked; // similar to ai_link_checked, but this parameter can be used for players' nation
	char			 linked_mine_num;
	char			 linked_factory_num;
	char			 linked_market_num;
	short			 linked_mine_array[MAX_LINKED_FIRM_FIRM];
	short			 linked_factory_array[MAX_LINKED_FIRM_FIRM];
	short			 linked_market_array[MAX_LINKED_FIRM_FIRM];

	int			 total_linked_trade_firm()
					 { return linked_mine_num + linked_factory_num + linked_market_num; }

public:
	FirmHarbor();

	void  init(int xLoc, int yLoc, int nationRecno, int firmId, const char* buildCode=NULL, short builderRecno=0);
	void  deinit_derived();
	void 	put_info(int refreshFlag);
	void 	detect_info();

	void 	assign_unit(int unitRecno);
	void	next_day();

	int	is_operating()		{ return 1; }

	int	can_build_ship()	{ return !build_unit_id; }
	void	build_ship(int unitId, char remoteAction);
	void	sail_ship(int unitRecno, char remoteAction);
	void 	del_hosted_ship(int delUnitRecno);

	//-------- for harbor trading ----------//

	char	get_linked_mine_num()		{ return linked_mine_num; }
	char	get_linked_factory_num()	{ return linked_factory_num; }
	char	get_linked_market_num()		{ return linked_market_num; }
	void	update_linked_firm_info();

	//----------- AI functions -------------//

	void 	process_ai();
	void 	think_build_ship();
	void 	think_build_firm();
	int 	ai_build_firm(int firmId);

	int 	think_trade();
	UnitMarine* ai_get_free_trade_ship();

	//--------------------------------------//

	virtual FirmHarbor*     cast_to_FirmHarbor() { return this; };
	void	add_queue(int unitId, int amount = 1);
	void	remove_queue(int unitId, int amount = 1);
	void	cancel_build_unit();

	enum {HARBOR_BUILD_BATCH_COUNT = 5}; // Number of units enqueued when holding shift - ensure this is less than MAX_BUILD_SHIP_QUEUE

	//-------------- multiplayer checking codes ---------------//
	virtual	uint8_t crc8();
	virtual	void	clear_ptr();
	virtual	void	init_crc(FirmHarborCrc *c);

private:
	int 	should_show_harbor_info();

	void 	disp_main_menu(int refreshFlag);
	void 	detect_main_menu();

	void 	disp_build_menu(int refreshFlag);
	void  disp_build_button(int y, int unitId, int buttonUp);
	void  disp_queue_button(int y, int unitId, int buttonUp);
	void 	detect_build_menu();

	void 	put_det(int refreshFlag);
	int	detect_det();

	void 	disp_ship_goods(UnitMarine* shipUnit, int dispY1, int refreshFlag);
	void 	disp_ship_units(UnitMarine* shipUnit, int dispY1, int refreshFlag);

	void 	disp_build_info(int refreshFlag);

	void 	add_hosted_ship(int shipRecno);

	void	process_build();
	void	process_queue();
	void 	repair_ship();
};
#pragma pack()

//--------------------------------------//

#endif
