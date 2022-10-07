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

//Filename    : OTOWN.H
//Description : Header file of Object Town

#ifndef __OTOWN_H
#define __OTOWN_H

#ifndef __ODYNARRB_H
#include <ODYNARRB.h>
#endif

#ifndef __OSKILL_H
#include <OSKILL.h>
#endif

#ifndef __OFIRMID_H
#include <OFIRMID.h>
#endif

#ifndef __ORAWRES_H
#include <ORAWRES.h>
#endif

#ifndef __OTOWNRES_H
#include <OTOWNRES.h>
#endif

//------- define constant --------//

#define MAX_TOWN_GROWTH_POPULATION  60		// grow up to 60 persons in a town
#define MAX_TOWN_POPULATION  60		// MAX number of units in a town

//-------- Define constant -----------//

#define STD_TOWN_LOC_WIDTH       4
#define STD_TOWN_LOC_HEIGHT      4

//--------- Define training cost and level --------//

#define TRAIN_SKILL_COST				30
#define TRAIN_SKILL_LEVEL        	20
#define TOTAL_TRAIN_DAYS				 5		 // No. of days needed for training an unit

#define MIN_RECRUIT_LOYALTY			   30 	// only when loyalty > this, the unit can be recruited
#define MIN_NATION_DEFEND_LOYALTY		50    // if the loyalty of the town people > this, they will defense against the attack
#define MIN_INDEPENDENT_DEFEND_LOYALTY 30
#define SURRENDER_LOYALTY				   29	   // when the loyalty of the town is <= this, and being attacked by a nation, the town will surrender
#define REBEL_LOYALTY					   29    // if loyalty <= REBEL_LOYALTY, the unit may rebel

#define INDEPENDENT_LINK_RESISTANCE  50	 // only if the resistance of an independent town is lower than this, the town will enable its link with a foreign firm

#define TAX_PER_PERSON           	  5	 // amount of tax can be collected from each person
#define COLLECT_TAX_LOYALTY_DECREASE 10	 // Reduce 10 loyalty points per tax collection

#define TOWN_REWARD_PER_PERSON		   10
#define TOWN_REWARD_LOYALTY_INCREASE   10

#define IND_TOWN_GRANT_PER_PERSON			   30		// independent town granting cost per person
#define IND_TOWN_GRANT_RESISTANCE_DECREASE   10		// resistance decrease per grant

#define RECEIVED_HIT_PER_KILL			 (200/ATTACK_SLOW_DOWN)	 // no. of received hits will result in one death of town people

#define MAX_TRAIN_QUEUE					10

//-------- Define class Town ----------//

class Unit;
#pragma pack(1)
class Town
{
public:
	enum { TOWN_NAME_LEN=20 };

	short town_recno;
	short	town_name_id;
	char* town_name()			{ return town_res.get_name(town_name_id); }

	short nation_recno;
	short rebel_recno;		// whether this town is controlled by a rebel
	char	race_id;

	int	setup_date;			// the setup date of this town

	char  ai_town;
	char	ai_link_checked; // AI check firm and town locatin by links, disable checking by setting this parameter to 1

	char	independ_town_nation_relation; // each bit n is high representing this independent town will attack nation n.
	char	has_linked_own_camp;					 // whether the town has linked military camps of the same nation
	char	has_linked_enemy_camp;				 // whether the town has linked military camps of the same nation

	char	is_base_town;		// whether this town is base town or not

	short loc_x1, loc_y1, loc_x2, loc_y2;
	short abs_x1, abs_y1, abs_x2, abs_y2;

	short loc_width()    { return loc_x2-loc_x1+1; }
	short loc_height()   { return loc_y2-loc_y1+1; }

	short center_x;
	short center_y;
	uint8_t	region_id;

	short layout_id;           // town layout id.
	short first_slot_id;       // the first slot id. of the layout

	short slot_object_id_array[MAX_TOWN_LAYOUT_SLOT];  // the race id. of each slot building

	//---------- game vars ----------//

	short population;
	short jobless_population;
	short worker_population()		{ return population-jobless_population; }
			
	short	max_race_pop_array[MAX_RACE];			// the MAX population the current town layout supports
	short race_pop_array[MAX_RACE];     		// population of each race
	unsigned char race_pop_growth_array[MAX_RACE];		// population growth, when it reaches 100, there will be one more person in the town
	short jobless_race_pop_array[MAX_RACE];
	int   recruitable_race_pop(int raceId, int recruitSpy);

	float race_loyalty_array[MAX_RACE];
	char  race_target_loyalty_array[MAX_RACE];
	short	race_spy_count_array[MAX_RACE];		  // no. of spies in each race

	float race_resistance_array[MAX_RACE][MAX_NATION];
	char  race_target_resistance_array[MAX_RACE][MAX_NATION];

	int   race_harmony(int raceId);
	int   majority_race();		// the race that has the majority of the population

	int	average_loyalty();
	int	average_target_loyalty();
	int	average_resistance(int nationRecno);
	int	average_target_resistance(int nationRecno);

	short	town_defender_count;			// no. of units currently defending this town
	int	last_being_attacked_date;
	float received_hit_count;		// no. of received hit by attackers, when this > RECEIVED_HIT_PER_KILL, a town people will be killed

	char  train_queue_skill_array[MAX_TRAIN_QUEUE];	// it stores the skill id.
	char  train_queue_race_array[MAX_TRAIN_QUEUE];	// it stores the race id.
	char  train_queue_count;
	short	train_unit_recno;			// race id. of the unit the town is currently training, 0-if currently not training any
	int	train_unit_action_id;	// id. of the action to be assigned to this unit when it is finished training.
	uint32_t start_train_frame_no;
	short defend_target_recno; 	// used in defend mode, store recno of latest target atttacking this town
	
	enum {TOWN_TRAIN_BATCH_COUNT = 8}; // Number of units enqueued when holding shift - ensure this is less than MAX_TRAIN_QUEUE

	//-------- other vars ----------//

	int   accumulated_collect_tax_penalty;
	int   accumulated_reward_penalty;
	int   accumulated_recruit_penalty;
	int   accumulated_enemy_grant_penalty;

	int	last_rebel_date;
	short independent_unit_join_nation_min_rating;

	short quality_of_life;
	void	update_quality_of_life();

	//------- auto policy -------------//

	short	auto_collect_tax_loyalty;		// auto collect tax if the loyalty reaches this level
	short	auto_grant_loyalty;				// auto grant if the loyalty drop below this level

	//----------- AI vars ------------//

	char	town_combat_level;						// combat level of the people in this town
	char	has_product_supply[MAX_PRODUCT];		// whether this town has the supply of these products
	char	no_neighbor_space;						// 1 if there is no space to build firms/towns next to this town

	//------ inter-relationship -------//

	short  linked_firm_count;
	short  linked_town_count;

	short  linked_firm_array[MAX_LINKED_FIRM_TOWN];
	short  linked_town_array[MAX_LINKED_TOWN_TOWN];

	char   linked_firm_enable_array[MAX_LINKED_FIRM_TOWN];
	char   linked_town_enable_array[MAX_LINKED_TOWN_TOWN];

	int 	 closest_own_camp();

	//========== NOTE: The following members are not loaded from/saved to file ==========//
	enum {SIZEOF_NONSAVED_ELEMENTS = sizeof(int)+sizeof(bool)};

	//--------- town network ----------//
	int		town_network_recno;						// The recno of the town network this town belongs to. Note: this value can change between saving and loading.
	bool	town_network_pulsed;					// Used for pulsing the town network to check which parts are still connected. Must always be set to false, and can only be true during a pulse-operation

	//------ static class member var ------//

	static short  if_town_recno;

public:
	Town();

	void  init(int,int,int,int);
	void  deinit();

	void  next_day();
	void  draw(int displayLayer=1);
	void  process_ai();

	void  disp_info(int refreshFlag);
	void  detect_info();
	int	browse_selected_race_id();

	void  assign_unit(int unitRecno);
	int   recruit(int withTraining, int raceId, char remoteAction);
	int  	recruit_dec_loyalty(int raceId, int decNow=1);
	void	cancel_train_unit();
	int  	form_new_nation();

	int	can_recruit(int raceId);
	int	can_train(int raceId);
	bool   can_migrate(int destTownRecno, bool migrateNow=false, int raceId=0);		 // if not called by Town::migrate, don't set migrateNow to TRUE
	void	move_pop(Town* destTown, int raceId, int hasJob);
	int 	pick_random_race(int pickNonRecruitableAlso, int pickSpyFlag);
	int 	camp_influence(int unitRecno);

	void  setup_link();
	void  release_link();
	void  release_firm_link(int);
	void  release_town_link(int);
	int	  linked_active_camp_count();
	int   can_toggle_firm_link(int firmRecno);
	void  update_camp_link();

	void  init_pop(int raceId, int addPop, int loyalty, int hasJob=0, int firstInit=0);
	void  inc_pop(int raceId, int unitHasJob, int unitLoyalty);
	void  dec_pop(int raceId, int unitHasJob);

	void  draw_selected();
	int   draw_detect_link_line(int);
	int   is_in_zoom_win();

	int 	has_linked_camp(int nationRecno, int needOverseer);

	void	auto_set_layout();
	void  set_nation(int nationRecno);
	void 	surrender(int toNationRecno);

	void	set_hostile_nation(int nationRecno);
	void	reset_hostile_nation(int nationRecno);
	int	is_hostile_nation(int nationRecno);

	int 	create_rebel_unit(int raceId, int isLeader);
	int   mobilize_town_people(int raceId, int decPop, int mobilizeSpy);
	int	mobilize_defender(int attackerNationRecno);

	int	migrate_to(int destTownRecno, char remoteAction, int raceId=0, int count=1);
	void	collect_yearly_tax();
	void  collect_tax(char remoteAction);
	void  reward(char remoteAction);
	void  distribute_demand();
	void	being_attacked(int attackerUnitRecno, float attackDamage);
	void	clear_defense_mode();
	void	reduce_defender_count();
	void	kill_town_people(int raceId, int attackerUnitRecno=0);

	int 	can_grant_to_non_own_town(int grantNationRecno);
	int 	grant_to_non_own_town(int grantNationRecno, int remoteAction);

	void 	get_most_populated_race(int& mostRaceId1, int& mostRaceId2);

	void  update_target_loyalty();
	void  update_target_resistance();
	void  update_loyalty();
	void  update_resistance();
	void	update_product_supply();
	void	change_loyalty(int raceId, float loyaltyChange);
	void	change_resistance(int raceId, int nationRecno, float loyaltyChange);

	void  toggle_firm_link(int linkId, int toggleFlag, char remoteAction, int setBoth=0);
	void  toggle_town_link(int linkId, int toggleFlag, char remoteAction, int setBoth=0);

	void	auto_defense(short targetRecno);
	int	has_player_spy();
	void 	verify_slot_object_id_array();

	void 	set_auto_collect_tax_loyalty(int loyaltyLevel);
	void 	set_auto_grant_loyalty(int loyaltyLevel);
	void 	disp_auto_loyalty();

	void	add_queue(char skillId, char raceId, int amount = 1);
	void	remove_queue(char skillId, int amount = 1);

	int   write_file(File*);
	int   read_file(File*);

	//-------- ai functions ---------//

	void	think_collect_tax();
	void	think_reward();
	int	think_build_firm(int firmId, int maxFirm);
	int	think_build_market();
	int	think_build_camp();
	int 	think_build_research();
	int 	think_build_war_factory();
	int 	think_build_base();
	int 	think_build_inn();
	int 	think_ai_migrate();
	int 	think_ai_migrate_to_town();
	void	think_defense();
	int 	think_split_town();
	void	think_move_between_town();
	int 	think_attack_nearby_enemy();
	int 	think_attack_linked_enemy();
	void	think_capture_linked_firm();
	int 	think_capture_enemy_town();
	int   think_scout();

	void 	update_base_town_status();
	int	new_base_town_status();

	int 	think_counter_spy();
	int 	needed_anti_spy_level();

	int 	think_spying_town();
	int 	think_spying_town_assign_to(int raceId);

	int	should_ai_migrate();
	int   detect_enemy(int);

	int	protection_needed();			// an index from 0 to 100 indicating the military protection needed for this town
	int	protection_available();

	int 	ai_build_neighbor_firm(int firmId, int firmRaceId=0);
	int 	ai_settle_new(int raceId);

	//-------- independent town ai functions ---------//

	void	think_independent_town();
	void 	think_independent_set_link();
	int  	think_independent_form_new_nation();
	int  	think_independent_unit_join_nation();
	int  	independent_unit_join_nation(int raceId, int toNationRecno);

	//--------- function for cheat key ----------//
	int	get_selected_race();

	//-------------- multiplayer checking codes ---------------//
	uint8_t crc8();
	void	clear_ptr();

	//-------------------------------//

private:
	void  set_world_matrix();
	void  restore_world_matrix();
	void 	establish_contact_with_player();

	void 	process_food();
	void	process_auto();
	void  process_train();
	void	finish_train(Unit*);
	void  population_grow();
	void	process_queue();
	void  think_migrate();
	int 	think_migrate_one(Town* targetTown, int raceId, int townDistance);
	void  migrate(int raceId, int destTownZoneRecno, int newLoyalty);
	int	unjob_town_people(int raceId, int unjobSpy, int unjobOverseer, int killOverseer=0);

	int	think_layout_id();

	void  draw_flag(int,int);
	void  draw_farm(int,int,int);

	void  disp_basic_info(int refreshFlag);
	void	disp_train_info(int refreshFlag);

	void  disp_main_menu(int refreshFlag);
	int   detect_main_menu();
	void 	disp_debug_resistance(int refreshFlag);

	void  disp_train_menu(int refreshFlag);
	int   detect_train_menu();

	void  disp_auto_menu(int modeCollectTax);
	int   detect_auto_menu(int modeCollectTax);

	void  disp_spy_menu(int refreshFlag);
	int   detect_spy_menu();

	void  think_rebel();
	int	think_surrender();
};
#pragma pack()

//-------- Begin of class TownArray ------------//

class TownArray : public DynArrayB
{
public:
	int   selected_recno;      // the firm current being selected
	int	race_wander_pop_array[MAX_RACE];		// no. of wandering people of each race. They are people for setting up independent towns later

public:
   TownArray();
   ~TownArray();

	void  init();
   void  deinit();

   int   add_town(int nationRecno, int raceId, int xLoc, int yLoc);
	void  del_town(int townRecno);
	Town* create_town();

   void  draw();
   void  draw_dot();
	void	draw_profile();

	void	process();

	int	independent_town_resistance();
	void	think_new_independent_town();

	int 	think_town_loc(int maxTries, int& xLoc, int& yLoc);
	int   find_nearest_town(int xLoc, int yLoc, int nationRecno=0);

	int   settle(int unitRecno, int xLoc, int yLoc);
	void  distribute_demand();

	void	stop_attack_nation(short nationRecno);

   int   write_file(File*);
   int   read_file(File*);

	int   is_deleted(int recNo);

   #ifdef DYNARRAY_DEBUG_ELEMENT_ACCESS
		Town* operator[](int recNo);
	#else
		Town* operator[](int recNo)  { return (Town*) get_ptr(recNo); }
	#endif

	void  disp_next(int seekDir, int sameNation);
};

extern TownArray town_array;

//---------------------------------------------------//

#endif
