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

//Filename    : OUNIT.H
//Description : Header file of Object Unit

#ifndef __OUNIT_H
#define __OUNIT_H

#ifndef __OSPRITE_H
#include <OSPRITE.h>
#endif

#ifndef __OSPATH_H
#include <OSPATH.h>
#endif

#ifndef __OUNITRES_H
#include <OUNITRES.h>
#endif

#ifndef __OBUTTON_H
#include <OBUTTON.h>
#endif

#ifndef __OSKILL_H
#include <OSKILL.h>
#endif

#ifndef __OSPREUSE_H
#include <OSPREUSE.h>
#endif

#ifdef NO_DEBUG_UNIT
#undef DEBUG
#endif

#define GAME_FRAMES_PER_DAY 10

//-------- action code for action_mode ---------//

enum { ACTION_STOP,
       ACTION_ATTACK_UNIT,
       ACTION_ATTACK_FIRM,
       ACTION_ATTACK_TOWN,
       ACTION_ATTACK_WALL,
       ACTION_ASSIGN_TO_FIRM,
       ACTION_ASSIGN_TO_TOWN,
       ACTION_ASSIGN_TO_VEHICLE,
       ACTION_ASSIGN_TO_SHIP,
       ACTION_SHIP_TO_BEACH,  // used for UNIT_SEA only
       ACTION_BUILD_FIRM,
       ACTION_SETTLE,
       ACTION_BURN,
       ACTION_DIE,
       ACTION_MOVE,
       ACTION_GO_CAST_POWER,  // for god only

       //------------ used only for action_mode2 -----------------//
       //------- put the following nine parameters together -------//

       ACTION_AUTO_DEFENSE_ATTACK_TARGET, // move to target for attacking
       ACTION_AUTO_DEFENSE_DETECT_TARGET, // is idle, detect target to attack or waiting for defense action, (detect range is larger as usual)
       ACTION_AUTO_DEFENSE_BACK_CAMP,     // go back to camp for training or ready for next defense action
       ACTION_DEFEND_TOWN_ATTACK_TARGET,
       ACTION_DEFEND_TOWN_DETECT_TARGET,
       ACTION_DEFEND_TOWN_BACK_TOWN,
       ACTION_MONSTER_DEFEND_ATTACK_TARGET,
       ACTION_MONSTER_DEFEND_DETECT_TARGET,
       ACTION_MONSTER_DEFEND_BACK_FIRM,
     };

//-------- define action type for action_misc ----------//

enum  {  ACTION_MISC_STOP = 0,
         ACTION_MISC_CAPTURE_TOWN_RECNO,
         ACTION_MISC_DEFENSE_CAMP_RECNO,
         ACTION_MISC_DEFEND_TOWN_RECNO,
         ACTION_MISC_MONSTER_DEFEND_FIRM_RECNO,
         ACTION_MISC_PRE_SEARCH_NODE_USED_UP,
      };

//--------- unit mode ------------//

enum { UNIT_MODE_OVERSEE=1,         // unit_mode_para is the recno of the firm the unit is overseeing
       UNIT_MODE_DEFEND_TOWN,       // unit_mode_para is the recno of the town the unit is defending
       UNIT_MODE_CONSTRUCT,         // unit_mode_para is the recno of the firm the unit is constructing
       UNIT_MODE_REBEL,             // unit_mode_para is the recno of the rebel group the unit belongs to
       UNIT_MODE_MONSTER,           // unit_mode_para is the recno of the firm recno of the monster firm it belongs to
       UNIT_MODE_ON_SHIP,           // unit_mode_para is the recno of the ship unit this unit is on
		 UNIT_MODE_IN_HARBOR,         // for ships only, unit_mode_para is the recno of the harbor this marine unit is in
		 UNIT_MODE_UNDER_TRAINING,
     };

//-------------- unit rank -------------//

enum { MAX_RANK=3 };

enum { RANK_SOLDIER,
       RANK_GENERAL,
       RANK_KING,
     };

//------------- unit salary -----------//

enum { SOLDIER_YEAR_SALARY  = 10,
		 GENERAL_YEAR_SALARY  = 50,
		 SPY_YEAR_SALARY      = 100 };

//------- other constant ----------//

enum { EFFECTIVE_LEADING_DISTANCE = 10 };

enum { ATTACK_DIR = 8 };      // define number of attacking direction

enum { MAX_TEAM_MEMBER = 9 }; // maximum no. of units a general/king can lead

enum { MAX_NATION_CONTRIBUTION = 10000 };		// there is an upper limit nation_contribution as it is a <short>  

//---------- used in set_move_to_surround -----------//

enum  { BUILDING_TYPE_FIRM_MOVE_TO,       // firm already exists
        BUILDING_TYPE_FIRM_BUILD,         // no firm there
        BUILDING_TYPE_TOWN_MOVE_TO,       // town already exists
        BUILDING_TYPE_SETTLE,             // no town there
        BUILDING_TYPE_VEHICLE,            // location is blocked by the vehicle
        BUILDING_TYPE_WALL,               // location is occupied by the wall
      };

//------------ define help mode ----------//
enum  {  HELP_NOTHING =0,
         HELP_ATTACK_UNIT,
         HELP_ATTACK_FIRM,
         HELP_ATTACK_TOWN,
         HELP_ATTACK_WALL,
       };

//---------- misc constant parameters ----------//

enum  {  KEEP_PRESERVE_ACTION = 1,  // used for stop2() to keep preserve action
         KEEP_DEFENSE_MODE = 2,     // used for stop2() to keep the defense mode
         KEEP_DEFEND_TOWN_MODE = 3, // used for stop2() to keep the defend town mode

         MAX_WAITING_TERM_SAME = 3, // wait for same nation, used in handle_blocked...()
         MAX_WAITING_TERM_DIFF = 3, // wait for diff. nation, used in handle_blocked...()

			ATTACK_DETECT_DISTANCE = 6,// the distance for the unit to detect target while idle
         ATTACK_SEARCH_TRIES = 250, // the node no. used to process searching when target is close to this unit
         ATTACK_WAITING_TERM = 10,  // terms no. to wait before calling searching to attack target

         //MAX_SEARCH_OR_STOP_WAIT_TERM = 15, // note: should be the largetest default value in waiting_term

         AUTO_DEFENSE_STAY_OUTSIDE_COUNT = 4, //4 days
         AUTO_DEFENSE_DETECT_COUNT = 3 + GAME_FRAMES_PER_DAY*AUTO_DEFENSE_STAY_OUTSIDE_COUNT,
         EFFECTIVE_AUTO_DEFENSE_DISTANCE = 9,
         AUTO_DEFENSE_SEARCH_TRIES = 100,

         UNIT_DEFEND_TOWN_DISTANCE = 8,
         UNIT_DEFEND_TOWN_STAY_OUTSIDE_COUNT = 4, // 4 days
         UNIT_DEFEND_TOWN_DETECT_COUNT = 3 + GAME_FRAMES_PER_DAY*UNIT_DEFEND_TOWN_STAY_OUTSIDE_COUNT,
         UNIT_DEFEND_TOWN_WAITING_TERM = 4,
         EFFECTIVE_DEFEND_TOWN_DISTANCE = 9,

         MONSTER_DEFEND_FIRM_DISTANCE = 8,
         MONSTER_DEFEND_STAY_OUTSIDE_COUNT = 4, // 4 days
         MONSTER_DEFEND_DETECT_COUNT = 3 + GAME_FRAMES_PER_DAY*MONSTER_DEFEND_STAY_OUTSIDE_COUNT,
         EFFECTIVE_MONSTER_DEFEND_FIRM_DISTANCE = 9,

         DO_CAST_POWER_RANGE = 3,   // for god to cast power
      };

//----------- Define TeamInfo -------------//

#pragma pack(1)
struct TeamInfo
{
	TeamInfo();

   char  member_count;
   short member_unit_array[MAX_TEAM_MEMBER];
   int   ai_last_request_defense_date;
};
#pragma pack()

//----------- Define class Unit -----------//

#pragma pack(1)
class Unit : public Sprite
{
public:
	char        unit_id;
	char        rank_id;
	char        race_id;
	char        nation_recno;
	char        ai_unit;
	uint16_t    name_id;             // id. of the unit's name in RaceRes::first_name_array;

	DWORD       unit_group_id;       // the group id this unit belong to if it is selected
	DWORD       team_id;             // id. of defined team
	char        selected_flag;       // whether the unit has been selected or not
	char        group_select_id;     // id for group selection

	char        waiting_term;        // for 2x2 unit only, the term to wait before recalling A* to get a new path
	char        blocked_by_member;
	char        swapping;

	short       leader_unit_recno;   // recno of this unit's leader

	int           is_visible()      { return cur_x >= 0; }     // whether the unit is visible on the map, it is not invisable if cur_x == -1
	virtual char* unit_name(int withTitle=1);
	uint8_t          region_id();

	//--------- action vars ------------//
	char        action_misc;
	short       action_misc_para;

	char        action_mode;
	short       action_para;
	short       action_x_loc;
	short       action_y_loc;

	char        action_mode2;  // store the existing action for speeding up the performance if same action is ordered.
	short       action_para2;  // to re-activiate the unit if its cur_action is idle
	short       action_x_loc2;
	short       action_y_loc2;

	char        blocked_edge[4];        // for calling searching in attacking
	UCHAR       attack_dir;

	//------------ attack parameters -----------//

	short       range_attack_x_loc;     // -1 for unable to do range_attack, use to store previous range attack location
	short       range_attack_y_loc;     // -1 for unable to do range_attack, use to store previous range attack location

	//------------- for unit movement ---------------//

	short       move_to_x_loc;          // the location the unit should be moving to
	short       move_to_y_loc;

	//---------- game vars -------------//

	char        loyalty;
	char        target_loyalty;

	float       hit_points;
	short       max_hit_points;

	Skill       skill;

	char        unit_mode;
	short       unit_mode_para;      // if unit_mode==UNIT_MODE_REBEL, unit_mode_para is rebel_recno this unit belongs to
	short       rebel_recno()        { return unit_mode==UNIT_MODE_REBEL ? unit_mode_para : 0; }

	short       spy_recno;              // spy parameters

	short       nation_contribution;    // For humans: contribution to the nation. For weapons: the tech level!
	short       total_reward;           // total amount of reward you have given to the unit

	int			commander_power();

	//---- share the use of nation_contribution and total_reward ----//

	int         get_monster_id()              { return nation_contribution; }
	void        set_monster_id(int monsterId) { nation_contribution = monsterId; }

	int         get_monster_soldier_id()                     { return total_reward; }
	void        set_monster_soldier_id(int monsterSoldierId) { total_reward = monsterSoldierId; }

	int         get_weapon_version()                   { return nation_contribution; }
	void        set_weapon_version(int weaponVersion)  { nation_contribution = weaponVersion; }
	int			unit_power();

	//------- attack parameters --------//

	AttackInfo* attack_info_array;
	char        attack_count;
	char        attack_range;
	short       cur_power;              // power for power attack
	short       max_power;

	//------- path seeking vars --------//

	ResultNode* result_node_array;
	int         result_node_count;
	short       result_node_recno;
	short       result_path_dist;

	//----------- way points -----------//
	enum	{ WAY_POINT_ADJUST_SIZE	= 5};
	ResultNode*	way_point_array;
	short			way_point_array_size;
	short			way_point_count;

	//--------- AI parameters ------------//

	WORD        ai_action_id;     			// an unique id. for locating the AI action node this unit belongs to in Nation::action_array

	char  		original_action_mode;
	short			original_action_para;
	short			original_action_x_loc;
	short			original_action_y_loc;

	short			original_target_x_loc;     // the original location of the attacking target when the attack() function is called
	short			original_target_y_loc;		// action_x_loc2 & action_y_loc2 will change when the unit move, but these two will not.

	short			ai_original_target_x_loc;	// for AI only
	short			ai_original_target_y_loc;

	char			ai_no_suitable_action;

	//-------- defense blocking ability flag ----------//

	char        can_guard_flag;         // bit0= standing guard, bit1=moving guard
													// copy from sprite_info->can_guard_flag when skill.combat level is high enough
	char        can_attack_flag;        // 1 able to attack, 0 unable to attack no matter what attack_count is
	char        force_move_flag;

	short			home_camp_firm_recno;

	char			aggressive_mode;

	char			seek_path_fail_count;
	char			ignore_power_nation;

	//------ TeamInfo structure for general and king only ------//

	TeamInfo*   team_info;

	int         commanded_soldier_count();

public:
	Unit();
	virtual ~Unit();

	//------- derived functions from Sprite ------//

	virtual void init(int unitId, int nationRecno, int rankId=0, int unitLoyalty=0, int startX= -1, int startY= -1);
	virtual void deinit();
	virtual void init_derived()      {;}

			  void init_sprite(int startXLoc, int startYLoc);
			  void deinit_sprite(int keepSelected=0);

			  void init_unit_id(int unitId);
			  void deinit_unit_id();
			  void deinit_unit_mode();
			  void del_team_member(int);
			  void validate_team();

			  void draw();
	virtual void draw_outlined();
			  void draw_selected();
			  void draw_skill_icon();

			  void set_spy(int spyRecno);
			  void set_name(uint16_t newNameId);
			  void set_mode(char modeId, short modePara=0) { unit_mode=modeId; unit_mode_para=modePara; }
			  int  is_shealth();
			  int  is_civilian();
			  int  is_own();
			  int  is_own_spy();
			  int  is_nation(int nationRecno);
			  int  true_nation_recno();            // the true nation recno of the unit, taking care of the situation where the unit is a spy
	virtual int  is_ai_all_stop();
			  int  get_cur_loc(short& xLoc, short& yLoc);

	virtual void die()         {;}

			  //-------------- AI functions -------------//

	virtual void process_ai();

			  int  think_king_action();
			  int  think_general_action();
			  int  think_leader_action();
			  int  think_normal_human_action();
			  int  think_weapon_action();
			  int  think_ship_action();
			  int  think_assign_weapon_to_camp();
			  int  think_build_camp();
			  int  think_reward();
			  void think_independent_unit();
			  void think_spy_action();
			  int  think_king_flee();
			  int  think_general_flee();
			  int  think_stop_chase();

			  void ai_move_to_nearby_town();
			  int  ai_escape_fire();
			  void ai_leader_being_attacked(int attackerUnitRecno);
			  int  ai_build_camp();
			  int  ai_settle_new_town();
			  int  ai_handle_seek_path_fail();

	//------- functions for unit AI mode ---------//

			  int  think_aggressive_action();
			  int  think_change_attack_target();
			  int  think_resume_original_action();

			  void save_original_action();
			  void resume_original_action();
			  void resume_original_attack_action();

			  void ask_team_help_attack(Unit* attackerUnit);

	//------------- processing functions --------------------//

	virtual void pre_process();
	virtual void process_idle();     // derived function of Sprite
	virtual void process_move();     // derived function of Sprite
	virtual void process_wait();     // derived function of Sprite
	virtual void process_extra_move() {;}// derived function of Sprite, for ship only
	virtual int  process_die();

	virtual void next_day();

			  void set_next(int nextX, int nextY, int para=0, int blockedChecked=0);

	//------------------------------------//

	virtual void disp_info(int refreshFlag);
	virtual void disp_unit_profile(int dispY1, int refreshFlag);
	virtual int  detect_unit_profile();
	virtual void detect_info();
	virtual bool is_in_build_menu();
	int			 should_show_info();

			  int  return_camp();

	//----------- parameters reseting functions ------------//
	virtual void   stop(int preserveAction=0);
	void           stop2(int preserveAction=0);
	void           reset_action_para();    // reset action_mode parameters
	void           reset_action_para2(int keepMode=0); // reset action_mode2 parameters
	void           reset_action_misc_para();

	//--------------- die actions --------------//
	int   is_unit_dead() {  return (hit_points<=0 || action_mode==ACTION_DIE || cur_action==SPRITE_DIE); }

	//------------ movement action -----------------//
	virtual void   move_to(int destX, int destY, int preserveAction=0, short searchMode=1, short miscNo=0, short numOfPath=1, short reuseMode=GENERAL_GROUP_MOVEMENT, short pathReuseStatus=0);
	void  move_to_unit_surround(int destXLoc, int destYLoc, int width, int height, int miscNo=0, int readyDist=0);
	void  move_to_firm_surround(int destXLoc, int destYLoc, int width, int height, int miscNo=0, int readyDist=0);
	void  move_to_town_surround(int destXLoc, int destYLoc, int width, int height, int miscNo=0, int readyDist=0);
	void  move_to_wall_surround(int destXLoc, int destYLoc, int width, int height, int miscNo=0, int readyDist=0);

	void  enable_force_move();
	void  disable_force_move();
	void  select_search_sub_mode(int sx, int sy, int dx, int dy, short nationRecno, short searchMode);
	void  different_territory_destination(int& destX, int& destY); // calculate new destination for move to location on different territory

	//----------------- attack action ----------------//
	void  attack_unit(int targetXLoc, int targetYLoc, int xOffset=0, int yOffset=0, int resetBlockedEdge=1);
	void  attack_unit(short targetRecno, int xOffset=0, int yOffset=0, int resetBlockedEdge=1);
	void  attack_firm(int firmXLoc, int firmYLoc, int xOffset=0, int yOffset=0, int resetBlockedEdge=1);
	void  attack_town(int townXLoc, int townYLoc, int xOffset=0, int yOffset=0, int resetBlockedEdge=1);
	void  attack_wall(int wallXLoc, int wallYLoc, int xOffset=0, int yOffset=0, int resetBlockedEdge=1);

	void  hit_target(Unit* parentUnit, Unit* targetUnit, float attackDamage);
	void  hit_building(Unit* parentUnit, int targetXLoc, int targetYLoc, float attackDamage);
	void  hit_firm(Unit* parentUnit, int targetXLoc, int targetYLoc, float attackDamage);
	void  hit_town(Unit* parentUnit, int targetXLoc, int targetYLoc, float attackDamage);
	void  hit_wall(Unit* attackUnit, int targetXLoc, int targetYLoc, float attackDamage);

	int   max_attack_range();
	void  gain_experience();
	virtual float  actual_damage();
	int   nation_can_attack(short nationRecno); // can this nation be attacked, no if alliance or etc..
	int   independent_nation_can_attack(short nationRecno);
	void  cycle_eqv_attack();
	int   is_action_attack();
	inline int  can_attack() { return (can_attack_flag && attack_count); }

	//-----------------  defense actions ---------------------//
	//========== unit's defense mode ==========//
	void  defense_attack_unit(short targetRecno);
	void  defense_attack_firm(int targetXLoc, int targetYLoc);
	void  defense_attack_town(int targetXLoc, int targetYLoc);
	void  defense_attack_wall(int targetXLoc, int targetYLoc);
	void  defense_detect_target();
	int   in_auto_defense_mode();
	int   in_defend_town_mode();
	int   in_monster_defend_mode();
	void  clear_unit_defense_mode();
	void  clear_town_defend_mode();
	void  clear_monster_defend_mode();

	//---------- embark to ship and other ship functions ---------//
	void  assign_to_ship(int destX, int destY, short shipRecno, int miscNo=0);
	void  ship_to_beach(int destX, int destY, int& finalDestX, int& finalDestY);  // for ship only

	//----------- other main action functions -------------//
	void  build_firm(int buildXLoc, int buildYLoc, int firmId, char remoteAction);
	void  burn(int burnXLoc, int burnYLoc, char remoteAction);
	void  settle(int settleXLoc, int settleYLoc, short curSettleUnitNum=1);
	void  assign(int buildXLoc, int buildYLoc, short curAssignUnitNum=1);
	void  go_cast_power(int castXLoc, int castYLoc, char castPowerType, char remoteAction);
	void	add_way_point(short x, short y);
	void	reset_way_point_array();
	void	process_way_point();

	//------------------------------------//

	void  change_nation(int newNationRecno);
	void  overseer_migrate(int destTownRecno);
	int   caravan_in_firm() { return cur_x==-2; }

	void  update_loyalty();
	void  set_combat_level(int);
	void  inc_minor_combat_level(int);
	void  inc_minor_skill_level(int);
	void  set_rank(int rankId);
	virtual int	can_resign();
	void  resign(int remoteAction);
	void  embark(int vehicleRecno);
	void  reward(int rewardNationRecno);
	void  transform();
	void  group_transform(char remoteAction, short *selectedArray=NULL, short selectedCount=0);
	void  spy_change_nation(int nationRecno, char remoteAction, int groupDefect = 0);
	int   can_spy_change_nation();

	void  change_hit_points(float changePoints);
	void  change_loyalty(int loyaltyChange);

	int   think_betray();
	int   betray(int newNationRecno);

	int   can_stand_guard()  { return can_guard_flag & 1;}
	int   can_move_guard()  { return can_guard_flag & 2;}
	int   can_attack_guard()  { return can_guard_flag & 4;}

	int   firm_can_assign(short firmRecno);

	void  set_idle();
	void  set_ready();
	void  set_move();
	void  set_wait();
	void  set_attack();
	void  set_turn();
	void  set_ship_extra_move();
	void  set_die();

	int   write_file(File* filePtr);
	int   read_file(File* filePtr);

	virtual int write_derived_file(File* filePtr);
	virtual int read_derived_file(File* filePtr);
	virtual void fix_attack_info();         // set attack_info_array appropriately

	//-------------- multiplayer checking codes ---------------//
	virtual	UCHAR crc8();
	virtual	void	clear_ptr();

private:
	//------------------ idle functions -------------------//
	int            reactivate_idle_action();
	int            idle_detect_attack(int startLoc=0, int dimensionInput=0, char defenseMode=0); // detect target to attack
	int            idle_detect_choose_target(char defenseMode);
	void           idle_detect_helper_attack(short unitRecno);
	int            idle_detect_unit_checking(short targetRecno);
	int            idle_detect_firm_checking(short targetRecno);
	int            idle_detect_town_checking(short targetRecno);
	int            idle_detect_wall_checking(int targetXLoc, int targetYLoc);

	//------------ movement action -----------------//
	int   search(int destX, int destY, int preserveAction=0, short searchMode=1, short miscNo=0, short numOfPath=1, short reuseMode=GENERAL_GROUP_MOVEMENT, short pathReuseStatus=0);
	int   searching(int destX, int destY, int preserveAction, short searchMode, short miscNo, short numOfPath, short reuseMode, short pathReuseStatus);
	int   set_move_to_surround(int buildXLoc, int buildYLoc, int width, int height, int buildingType, int miscNo=0, int readyDist=0, short curSettleUnitNum=1);
	int   edit_path_to_surround(int x1, int y1, int x2, int y2, int readyDist);
	void  search_or_stop(int destX, int destY, int preserveAction=0, short searchMode=1, short miscNo=0);
	void  search_or_wait();
	//void   move_to_surround_s2(int destXLoc, int destYLoc); // for 2x2 unit only
	int   move_to_range_attack(int targetXLoc, int targetYLoc, short miscNo, short searchMode, short maxRange); // move to target for using range attack

	void  abort_searching(int reuseSetNext);
	void  set_search_tries(int tries);     // set parameters to limit the nodes used in searching
	void  reset_search_tries();            // reset parameters for using default nodes in searching

	//---------------- handle blocked action ------------------//
	void  move_to_my_loc(Unit* unitPtr);
	void  handle_blocked_move(Location* blockedLoc); // used to determine unit size and call other handle_blocked_move.. functions
	void  handle_blocked_move_s11(Unit* unitPtr);
	//void  handle_blocked_move_s12(Unit* unitPtr);
	//void  handle_blocked_move_s21(Unit* unitPtr);
	//void  handle_blocked_move_s22(Unit* unitPtr);
	//int blocked_move_new_handle();
	//void   set_path_to(int destXLoc, int destYLoc);
	void  handle_blocked_by_idle_unit(Unit *unitPtr);
	int   on_my_path(short checkXLoc, short checkYLoc);

	void  handle_blocked_wait(Unit* unitPtr);
	void  cycle_wait_shift_recno(Unit* curUnit, Unit* nextUnit);
	void  opposite_direction_blocked(short vecX, short vecY, short unitPtrVecX, short unitPtrVecY, Unit* unitPtr);

	void  handle_blocked_attack_unit(Unit *unitPtr, Unit *targetPtr);
	void  handle_blocked_attack_firm(Unit *unitPtr);
	void  handle_blocked_attack_town(Unit *unitPtr);
	void  handle_blocked_attack_wall(Unit *unitPtr);
	void  handle_blocked_same_target_attack(Unit* unitPtr, Unit* targetPtr);

	//====== support functions for process_attack_unit()
	void  target_move(Unit* targetUnit);
	void  attack_target(Unit* targetUnit);
	int   on_way_to_attack(Unit* targetUnit);
	int   detect_surround_target();
	int   update_attack_path_dist();
	void  set_attack_dir(short curX, short curY, short targetX, short targetY);

	//====== functions for attacking between UNIT_LAND, UNIT_SEA, UNIT_AIR
	int   move_try_to_range_attack(Unit* targetUnit);
	//void   move_to_range_attack(int targetXLoc, int targetYLoc, short miscNo, short searchMode, short maxRange); //---defined above
	int   can_attack_different_target_type();
	int   possible_place_for_range_attack(int targetXLoc, int targetYLoc, int targetWidth, int targetHeight, int maxRange);

	//====== functions for reactivating idle units and blocked units that are ordered to attack
	int   space_for_attack(int targetXLoc, int targetYLoc, char targetMobileType, int targetWidth, int targetHeight);
	int   space_around_target(int squareXLoc, int squareYLoc, int width, int height);
	int   space_around_target_ver2(int targetXLoc, int targetYLoc, int targetWidth, int targetHeight);
	int   ship_surr_has_free_land(int targetXLoc, int targetYLoc, uint8_t regionId);
	int   free_space_for_range_attack(int targetXLoc, int targetYLoc, int targetWidth, int targetHeight, int targetMobileType, int maxRange);

	void  choose_best_attack_mode(int attackDistance, char targetMobileType=UNIT_LAND);
	void  unit_auto_guarding(Unit *attackUnit);
	void  set_unreachable_location(int xLoc, int yLoc);
	void  check_self_surround();
	int   can_attack_with(int i);               // 0 to attack_count-1
	int   can_attack_with(AttackInfo *attackInfo);
	void  get_hit_x_y(short *xPtr, short *yPtr);
	void  add_close_attack_effect();

	//-----------------  defense actions ---------------------//
	//=========== unit's defend mode generalized functions ============//
	int   in_any_defense_mode();
	void  general_defend_mode_detect_target(int checkDefendMode=0);
	int   general_defend_mode_process_attack_target();

	//========== unit's defense mode ==========//
	void  defense_back_camp(int targetXLoc, int targetYLoc);
	void  process_auto_defense_attack_target();
	void  process_auto_defense_detect_target();
	void  process_auto_defense_back_camp();
	int   defense_follow_target();

	//========== town unit's defend mode ==========//
	void  defend_town_attack_unit(short targetRecno);
	void  defend_town_detect_target();
	void  defend_town_back_town(short townRecno);
	void  process_defend_town_attack_target();
	void  process_defend_town_detect_target();
	void  process_defend_town_back_town();
	int   defend_town_follow_target();

	//========== monster unit's defend mode ==========//
	void  monster_defend_attack_unit(short targetRecno);
	void  monster_defend_attack_firm(int targetXLoc, int targetYLoc);
	void  monster_defend_attack_town(int targetXLoc, int targetYLoc);
	void  monster_defend_attack_wall(int targetXLoc, int targetYLoc);
	void  monster_defend_detect_target();
	void  monster_defend_back_firm(int targetXLoc, int targetYLoc);
	void  process_monster_defend_attack_target();
	void  process_monster_defend_detect_target();
	void  process_monster_defend_back_firm();
	int   monster_defend_follow_target();

	//---------- embark to ship and other ship functions ---------//
	int   ship_to_beach_path_edit(int& resultXLoc, int& resultYLoc, uint8_t regionId);
	void  ship_leave_beach(int shipOldXLoc, int shipOldYLoc);

	//---------------- other functions -----------------//
	int   cal_distance(int targetXLoc, int targetYLoc, int targetWidth, int targetHeight); // calculate distance from this unit(can be 1x1, 2x2) to a known size object
	int   is_in_surrounding(int checkXLoc, int checkYLoc, int width, int targetXLoc, int targetYLoc, int targetWidth, int targetHeight);
	int   avail_node_enough_for_search(short x1, short y1, short x2, short y2);
	// int   firm_can_assign(short firmRecno);

protected:
	void  disp_main_menu(int);
	void  detect_main_menu();
	void  disp_build_menu(int);
	void  detect_build_menu();
	void  disp_button(int dispY1);
	void  detect_button();
	void  disp_build(int);
	void  detect_build();
	void  disp_settle(int);
	void  detect_settle();
	void  disp_unit_info(int dispY1, int refreshFlag);
	void  disp_basic_info(int dispY1, int refreshFlag);
	int   detect_basic_info();
	void  disp_spy_menu(int dispY1, int refreshFlag);
	void  detect_spy_menu(int dispY1);
	int   spy_menu_height();
	void  disp_hit_point(int dispY1);

	void  process_attack_unit();
	void  process_attack_firm();
	void  process_build_firm();
	void  process_attack_town();
	void  process_attack_wall();
	void  process_assign();
	void  process_burn();
	void  process_settle();
	void  process_assign_to_ship();
	void  process_ship_to_beach();
	void  process_rebel();
	void  process_go_cast_power();

	void  next_move();

	void  terminate_move();
	void  reset_path();

	void  king_die();
	void  general_die();
	void  pay_expense();
	void  process_recover();
};
#pragma pack()

//--------------------------------------------------------------------------------------------//

//------- Define class UnitArray ---------//

class UnitArray : public SpriteArray
{
public:
	short selected_recno;
	short selected_count;

	DWORD cur_group_id;            // for Unit::unit_group_id
	DWORD cur_team_id;             // for Unit::team_id

	short idle_blocked_unit_reset_count; // used to improve performance for searching related to attack

	short visible_unit_count;
	char	mp_first_frame_to_select_caravan;	// for multiplayer, true if 1st frame to select caravan
	char	mp_first_frame_to_select_ship;		// ditto for ship
	short	mp_pre_selected_caravan_recno;		// for multiplayer, 0 or recno that caravan selected in previous frame
	short	mp_pre_selected_ship_recno;			// ditto for ship

	static short   selected_land_unit_count;
	static short   selected_sea_unit_count;
	static short   selected_air_unit_count;
	static short   *selected_land_unit_array;
	static short   *selected_sea_unit_array;
	static short   *selected_air_unit_array;

public:
	UnitArray(int);

	void  init();

	int   add_unit(int unitId, int nationRecno, int rankId=0, int unitLoyalty=0, int startXLoc= -1, int startYLoc= -1);
	Unit* create_unit(int unitId);
	int   unit_class_size(int);

	void  disappear_in_town(int unitRecno, int townRecno);
	void  disappear_in_firm(int unitRecno);
	void  die(int unitRecno);
	void	return_camp(int remoteAction, short* selectedUnitArray=NULL, int selectedCount=0);

	void  draw_dot();
	void	draw_profile();
	void  process();

	void  stop(short* selectedUnitArray, int selectedCount, char remoteAction);
	void  stop_all_war(short oldNationRecno);
	void  stop_war_between(short nationRecno1, short nationRecno2);
	void  stop_attack_unit(short unitRecno);
	void  stop_attack_firm(short firmRecno);
	void  stop_attack_town(short townRecno);

	//---------- move main functions -------------//
	void  move_to(int destX, int destY, int divided, short* selectedUnitArray, int selectedCount, char remoteAction);

	//------------- attack main functions ----------//
	// ###### patch begin Gilbert 5/8 ######//
	void  attack(int destX, int destY, int divided, short* selectedUnitArray, int selectedCount, char remoteAction, int unitRecno);
	// ###### patch end Gilbert 5/8 ######//
	void  attack_unit(int targetXLoc, int targetYLoc, short targetUnitRecno, short* selectedUnitArray, int selectedCount);
	void  attack_firm(int targetXLoc, int targetYLoc, short firmRecno, short* selectedUnitArray, int selectedCount);
	void  attack_town(int targetXLoc, int targetYLoc, short townRecno, short* selectedUnitArray, int selectedCount);
	void  attack_wall(int targetXLoc, int targetYLoc, short* selectedUnitArray, int selectedCount);

	//---------- other actions functions -----------//
	void  assign(int destX, int destY, int divided, char remoteAction, short* selectedUnitArray=NULL, int selectedCount=0);
	void  assign_to_camp(int destX, int destY, char remoteAction, short* selectedUnitArray=NULL, int selectedCount=0);
	void  settle(int destX, int destY, int divided, char remoteAction, short *selectedUnitArray=NULL, int selectedCount=0);
	// ##### patch begin Gilbert 5/8 ######//
	void  assign_to_ship(int shipXLoc, int shipYLoc, int divided, short* selectedArray, int selectedCount, char remoteAction, int shipRecno);
	// ##### patch end Gilbert 5/8 ######//
	void  ship_to_beach(int destX, int destY, int divided, short* selectedArray, int selectedCount, char remoteAction);
	void	add_way_point(int pointX, int pointY, short* selectedArray, int selectedCount, char remoteAction);

	//--------- unit filter function ----------//
	int   divide_attack_by_nation(short nationRecno, short *selectedArray, int selectedCount);

	//--------- for multiplayers' caravan and ship ---------//
	void	mp_mark_selected_caravan();
	int	mp_get_selected_caravan_count();
	void	mp_reset_selected_caravan_count();
	void	mp_add_selected_caravan(short unitRecno);
	int	mp_is_selected_caravan(short unitRecno);

	void	mp_mark_selected_ship();
	int	mp_get_selected_ship_count();
	void	mp_reset_selected_ship_count();
	void	mp_add_selected_ship(short unitRecno);
	int	mp_is_selected_ship(short unitRecno);

	void	update_selected_trade_unit_info();

	int   write_file(File* filePtr);
	int   read_file(File* filePtr);

	#ifdef DYNARRAY_DEBUG_ELEMENT_ACCESS
		Unit* operator[](int recNo);
	#else
		Unit* operator[](int recNo)   { return (Unit*) get_ptr(recNo); }
	#endif

	int   is_deleted(int recNo);
	int   is_truly_deleted(int recNo);

private:
	void  divide_array(int locX, int locY, short* selectedArray, int selectedCount, int excludeSelectedLocUnit=0);
	void  set_group_id(short* selectedArray, int selectedCount);

	//------------ move sub-functions --------------//
	void  move_to_now_with_filter(int destX, int destY, short* selectedUnitArray, int selectedCount);
	void  move_to_now(int destX, int destY, short* selectedUnitArray, int selectedCount);
	void  construct_sorted_array(short* selectedUnitArray, int selectedCount);
	void  determine_position_to_construct_table(int selectedCount, int destXLoc, int destYLoc, char mobileType);

	//-------------- attack sub-functions -----------//
	// ###### patch begin Gilbert 5/8 #######//
	void  attack_call(int destX, int destY, char mobileType, char targetMobileType, int divided, short* selectedUnitArray, int selectedCount, int targetUnitRecno);
	// ###### patch end Gilbert 5/8 #######//
	void  update_unreachable_table(int targetXLoc, int targetYLoc, int targetWidth, int targetHeight, char mobileType, int &analyseResult);
	char  get_target_surround_loc(int targetWidth, int targetHeight);
	char* get_target_x_offset(int targetWidth, int targetHeight, char curDir);
	char* get_target_y_offset(int targetWidth, int targetHeight, char curDir);

	void  arrange_units_in_group(int xLoc1, int yLoc1, int xLoc2, int yLoc2, short* selectedUnitArray, int selectedCount, DWORD unitGroupId, int targetType);
	int   analyse_surround_location(int targetXLoc, int targetYLoc, int targetWidth, int targetHeight, char mobileType);
	void  check_nearby_location(int targetXLoc, int targetYLoc, char xOffset, char yOffset, int targetWidth, int targetHeight, char targetMobileType, int& analyzeResult);
	void  handle_attack_target_totally_blocked(int targetXLoc, int targetYLoc, short targetRecno, short *selectedUnitArray, short selectedCount, int targetType);

	//---------- other actions functions -----------//
	void  group_assign(int destX, int destY, short* selectedArray, int selectedCount);
	void  group_settle(int destX, int destY, short* selectedArray, int selectedCount);
};

extern UnitArray unit_array;
extern int unit_search_node_used;
extern int     unit_search_tries;        // the number of tries used in the current searching
extern char    unit_search_tries_flag;   // indicate num of tries is set, reset after searching
#ifdef DEBUG
extern int check_unit_dir1, check_unit_dir2;
#endif

#endif

