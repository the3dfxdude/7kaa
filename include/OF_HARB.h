//Filename    : OF_HARB.H
//Description : Header of Firm Harbor

#ifndef __OF_HARB_H
#define __OF_HARB_H

#ifndef __OFIRM_H
#include <OFIRM.h>
#endif

//----------- Define constant --------------//

#define MAX_SHIP_IN_HARBOR  4  		// maximum no. of ships in a harbor
#define MAX_BUILD_SHIP_QUEUE	10

//------- Define class FirmHarbor --------//

class UnitMarine;

class FirmHarbor : public Firm
{
public:
	short			 ship_recno_array[MAX_SHIP_IN_HARBOR];
	short			 ship_count;

	short			 build_unit_id;			// race id. of the unit the town is currently building, 0-if currently not building any
	DWORD			 start_build_frame_no;

	char			 build_queue_array[MAX_BUILD_SHIP_QUEUE];	// it stores the unit id.
	char			 build_queue_count;

	UCHAR			 land_region_id;
	UCHAR			 sea_region_id;

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

	void  init(int xLoc, int yLoc, int nationRecno, int firmId, char* buildCode=NULL, short builderRecno=0);
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
	void	add_queue(int unitId);
	void	remove_queue(int unitId);
	void	cancel_build_unit();

	//-------------- multiplayer checking codes ---------------//
	virtual	UCHAR crc8();
	virtual	void	clear_ptr();

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

//--------------------------------------//

#endif
