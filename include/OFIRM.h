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

//Filename    : OFIRM.H
//Description : Header file of Object Firm

#ifndef __OFIRM_H
#define __OFIRM_H

#ifndef __GAMEDEF_H
#include <GAMEDEF.h>
#endif

#ifndef __OFIRMA_H
#include <OFIRMA.h>
#endif

#ifndef __OINFO_H
#include <OINFO.h>
#endif

#ifndef __OFIRMID_H
#include <OFIRMID.h>
#endif

#ifndef __OFIRMRES_H
#include <OFIRMRES.h>
#endif

#ifndef __OSKILL_H
#include <OSKILL.h>
#endif

//----------- Define constant ------------//

#define MAX_WORKER         	 8       // maximum no. of workers in a firm
#define MAX_CARGO					 9

#define WORKER_YEAR_SALARY    10

#define CAN_SELL_HIT_POINTS_PERCENT	 80		// only when the firm's hit points is higher than this can the firm be sold

//------- define firm menu modes --------//

enum { FIRM_MENU_MAIN,
		 FIRM_MENU_SPY,
		 FIRM_MENU_SELECT_BRIBER,
		 FIRM_MENU_SET_BRIBE_AMOUNT,
		 FIRM_MENU_VIEW_SECRET,
		 FIRM_MENU_ASSASSINATE_RESULT };

//------- define parameters for ai_status, used for AI ------//

enum	{	FIRM_WITHOUT_ACTION=0,
			FACTORY_RELOCATE,
			MARKET_FOR_SELL,
			CAMP_IN_DEFENSE,
		};

//-------- define values for Firm::bribe_result -------//

enum { BRIBE_NONE,
		 BRIBE_SUCCEED,
		 BRIBE_FAIL,
	  };

//-------- define values for Firm::assassinate_result -------//

enum { ASSASSINATE_FAIL,
		 ASSASSINATE_SUCCEED_AT_LARGE,		// assassination succeed and the assassinator is at large
		 ASSASSINATE_SUCCEED_KILLED,			// assassination succeed and the assassinator is caught and executed
	  };

//------------- Define struct Worker ------------//

#pragma pack(1)
struct Worker
{
public:
	Worker();

	char  race_id;
	char  unit_id;
	short town_recno;
	short	name_id;

	char	skill_id;
	char  skill_level;
	char  skill_level_minor;
	char  skill_potential;

	char  combat_level;
	char  combat_level_minor;

	short spy_recno;

	char  rank_id;
	char  worker_loyalty;		// only for firms with live_in_town being 0
	short hit_points;
	short extra_para;			// weapon version for weapons and power attack points for human units

	short	max_hit_points();
	int   loyalty();
	int	target_loyalty(int firmRecno);
	int	is_nation(int firmRecno, int nationRecno);

public:
	void	init_potential();
	char *small_icon_ptr();
	void  change_loyalty(int loyaltyChange);
	void	change_hit_points(int changePoints);
	int	max_attack_range();
};
#pragma pack()

class FirmBase;
class FirmMine;
class FirmFactory;
class FirmMarket;
class FirmCamp;
class FirmFarm;
class FirmInn;
class FirmResearch;
class FirmWar;
class FirmHarbor;

//----------- Define class Firm ------------//

#pragma pack(1)
class Firm
{
public:
	char   firm_id;            // Firm ID, meanings are defined in OFIRMID.H
	short  firm_build_id;
	short  firm_recno;         // record no. of this firm in the firm_array
	char   firm_ai;            // whether Computer AI control this firm or not
	char	 ai_processed;			// some ai actions are processed once only in the processing day. To prevent multiple checking in the processing day
	char	 ai_status;
	char	 ai_link_checked;		// AI checks firms and towns location by links, disable checking by setting this parameter to 1
	char	 ai_sell_flag;			// this is true if the AI has queued the command to sell this firm

	char   race_id;
	short  nation_recno;       // this firm's parent company nation
	char   majority_race();		// the race that has the majority of the population
	int    own_firm();         // whether the firm is controlled by the current player
	int	 can_sell() 		{ return hit_points >= (int) max_hit_points * CAN_SELL_HIT_POINTS_PERCENT / 100; }

	//-------- firm name vars ---------//

	short	 closest_town_name_id;			// name id. of the town that is closest to this firm when it is built
	short	 firm_name_instance_id;
	virtual char*	 firm_name();

	//--------- display info ----------//

	short  loc_x1, loc_y1, loc_x2, loc_y2;
	short  abs_x1, abs_y1, abs_x2, abs_y2;
	short  center_x, center_y;
	BYTE	 region_id;

	char   cur_frame;          // current animation frame id.
	char   remain_frame_delay;

	//---------- game vars ------------//

	float  hit_points;
	float  max_hit_points;
	char	 under_construction;		// whether the firm is under construction

	char   firm_skill_id;
	short  overseer_recno;
	short  overseer_town_recno;
	short	 builder_recno;		// the recno of the builder
	BYTE 	 builder_region_id;	// the original region no. of builder
	float  productivity;

	Worker* worker_array;
	char   worker_count;
	char   selected_worker_id;

	char	 player_spy_count;
	BYTE	 sabotage_level;			// 0-100 for counter productivity

	int	 average_worker_skill();

	virtual int is_operating()		{ return productivity > 0; }

	//------ inter-relationship -------//

	char 	 linked_firm_count;
	char 	 linked_town_count;

	short	 linked_firm_array[MAX_LINKED_FIRM_FIRM];
	short	 linked_town_array[MAX_LINKED_FIRM_TOWN];

	char 	 linked_firm_enable_array[MAX_LINKED_FIRM_FIRM];
	char 	 linked_town_enable_array[MAX_LINKED_FIRM_TOWN];

	//--------- financial vars ---------//

	float  last_year_income;
	float  cur_year_income;

	float  income_365days()      { return last_year_income*(365-info.year_day)/365 +
											cur_year_income; }
	int	 year_expense();

	//---------------------------------//

	int    setup_date;             // the date which this firm is setup
	char*  setup_years_str(int=0); // return the no. of years this firm has been setup in string

	char	 should_set_power;
	int 	 last_attacked_date;		 // the date when the firm was last being attacked.

	//----------- AI vars ------------//

	char	 should_close_flag;
	char	 no_neighbor_space;			// no space to build firms/towns next to this town
	char	 ai_should_build_factory_count;

	//--------- static vars ----------//

	static char  firm_menu_mode;
	static short action_spy_recno;	// recno of the spy that is doing the bribing or viewing secret reports of other nations
	static char  bribe_result;
	static char  assassinate_result;

public:
	Firm();
	virtual ~Firm();

	virtual void init(int xLoc, int yLoc, int nationRecno, int firmId, const char* buildCode=NULL, short builderRecno=0);
	virtual void deinit();

	void		init_name();
	int 		get_closest_town_name_id();

   virtual void assign_unit(int unitRecno);
   virtual void assign_overseer(int overseerUnitRecno);
   virtual void assign_worker(int workerUnitRecno);

	void		kill_overseer();
	void		kill_all_worker();
	void		kill_worker(int workerId);
	void		kill_builder(short builderRecno);

	virtual void being_attacked(int attackerUnitRecno);

	virtual int	 pull_town_people(int townRecno, char remoteAction, int raceId=0, int forcePull=0);
	void 		resign_overseer()		{ assign_overseer(0); }

	void     set_world_matrix();
	void     restore_world_matrix();
	int		get_should_set_power();

	void 		establish_contact_with_player();
	void		complete_construction();		// complete construction instantly

	void		capture_firm(int newNationRecno);
	virtual void change_nation(int newNationRecno);

	void		setup_link();
	void		release_link();
	void		release_firm_link(int);
	void		release_town_link(int);

	int 		can_toggle_town_link();
	int 		can_toggle_firm_link(int firmRecno);

	int      is_in_zoom_win();
	int		find_settle_town();
	int 		should_show_info();

	int		set_builder(short newBuilderRecno);

	virtual void sell_firm(char remoteAction);
	void		destruct_firm(char remoteAction);
	void		cancel_construction(char remoteAction);
	int		can_assign_capture();
	int		can_worker_capture(int captureNationRecno);
	virtual int	 is_worker_full();

	void 		set_worker_home_town(int townRecno, char remoteAction, int workerId=0);
	int 		can_spy_bribe(int bribeWorkerId, int briberNationRecno);
	int 		spy_bribe(int bribeAmount, short briber, short workerId);
	int 		spy_bribe_succeed_chance(int bribeAmount, short briberSpyRecno, short workerId);
	int 		validate_cur_bribe();

	//------------------- defense --------------------//
	virtual void auto_defense(short targetRecno);

	int		locate_space(int removeFirm, int &xLoc, int &yLoc, int xLoc2, int yLoc2, int width, int height, int mobileType=UNIT_LAND, int regionId=0);
	//---------------------------------------//

	virtual void draw(int displayLayer=1);
	virtual void draw_full_size(int displayLayer=1);
	virtual void draw_frame(int frameId, int displayLayer=1);
			  void draw_selected();
			  int  draw_detect_link_line(int actionDetect);

			  void disp_info_both(int refreshFlag);
			  void detect_info_both();

	virtual void put_info(int refreshFlag)		{;}
	virtual void detect_info()						{;}

	 		  void process_animation();
			  void process_construction();
           void process_repair();

			  void process_common_ai();
	virtual void process_ai()      {;}
	virtual void process_tell()    {;}

	virtual void next_day();
	virtual void next_month();
	virtual void next_year();

			  void mobilize_all_worker(int leaderUnitRecno);
	virtual int	 mobilize_worker(int workerId, char remoteAction);
			  int	 create_worker_unit(Worker& thisWorker);
	virtual int  mobilize_overseer();
			  int	 mobilize_builder(short recno);

			  void resign_all_worker(int disappearFlag=0);
	virtual int  resign_worker(int workerId);

			  void reward(int workerId, int remoteAction);

			  void toggle_firm_link(int linkId, int toggleFlag, char remoteAction, int setBoth=0);
			  void toggle_town_link(int linkId, int toggleFlag, char remoteAction, int setBoth=0);

	virtual int  write_derived_file(File*); 
	virtual int  read_derived_file(File*);

	virtual FirmBase*		cast_to_FirmBase() { return 0; };
	virtual FirmMine*		cast_to_FirmMine() { return 0; };
	virtual FirmFactory*	cast_to_FirmFactory() { return 0; };
	virtual FirmMarket*	cast_to_FirmMarket() { return 0; };
	virtual FirmCamp*		cast_to_FirmCamp() { return 0; };
	virtual FirmFarm*		cast_to_FirmFarm() { return 0; };
	virtual FirmInn*		cast_to_FirmInn() { return 0; };
	virtual FirmResearch* cast_to_FirmResearch() { return 0; };
	virtual FirmWar*		cast_to_FirmWar() { return 0; };
	virtual FirmHarbor*	cast_to_FirmHarbor() { return 0; };

	//---------- AI functions ----------//

			  void 			think_repair();
			  void			ai_del_firm();
			  int 			ai_recruit_worker();
	virtual int	   		ai_has_excess_worker()	 { return 0; }		// whether the AI has excess workers on this firm or not
			  int				think_build_factory(int rawId);
	virtual int				ai_should_close();
			  int 			ai_build_neighbor_firm(int firmId);
	virtual void			ai_update_link_status();
			  int 			think_hire_inn_unit();
			  int				think_capture();
	virtual void 			think_linked_town_change_nation(int linkedTownRecno, int oldNationRecno, int newNationRecno);
			  void 			ai_firm_captured(int capturerNationRecno);

	//-------------- multiplayer checking codes ---------------//
	virtual	UCHAR crc8();
	virtual	void	clear_ptr();

   //---------------------------------------//

protected:
	virtual void init_derived()   {;}
	virtual void deinit_derived() {;}

			  void recruit_worker();
			  void free_worker_room();
			  int  assign_settle(int raceId, int unitLoyalty, int isOverseer);
			  int	 best_worker_id();

			  void calc_productivity();
			  void update_worker();
			  void add_income(int incomeType, float incomeAmt);
			  void pay_expense();
			  void consume_food();
			  void update_loyalty();

           void think_worker_migrate();
			  void worker_migrate(int workerId, int destTownZoneRecno, int newLoyalty);
			  void process_independent_town_worker();

			  void disp_spy_button(int x, int y, int refreshFlag);
			  void detect_spy_button();
			  void disp_spy_menu(int refreshFlag);
			  void detect_spy_menu();
			  int  can_player_spy_capture();

			  void disp_bribe_menu(int refreshFlag);
			  void detect_bribe_menu();
			  void spy_bribe(int bribeAmount);
			  void disp_bribe_unit(int dispY1);

			  void disp_assassinate_result(int refreshFlag);
			  void detect_assassinate_result();

			  void disp_worker_list(int dispY1, int refreshFlag);
			  int  detect_worker_list();
           void disp_basic_info(int dispY1, int refreshFlag);
			  int  detect_basic_info();
			  void disp_worker_info(int dispY1, int refreshFlag);
			  void disp_overseer_info(int dispY1, int refreshFlag);

			  void draw_cargo(int cargoCount, char* cargoBitmapPtr);

				int create_unit(int unitId, int townZoneRecno=0, int unitHasJob=0);
			  void disp_hit_point(int dispY1);
			  // ##### begin Gilbert 18/10 #######//
			  int	construction_frame();			// for under construction only
			  // ##### end Gilbert 18/10 #######//
};
#pragma pack()

//------------------------------------------//

#endif

