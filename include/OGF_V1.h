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

#ifndef __OGF_V1_H
#define __OGF_V1_H

#ifdef AMPLUS

#ifndef __OTOWN_H
#include <OTOWN.h>
#endif

#ifndef __NATIONA_H
#include <ONATIONA.h>
#endif

#ifndef __NATION_H
#include <ONATION.h>
#endif

#ifndef __OGFILE_H
#include <OGFILE.h>
#endif

#pragma pack(1)
class Version_1_Town
{
public:
	short town_recno;
	short	town_name_id;
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
	short center_x;
	short center_y;
	BYTE	region_id;
	short layout_id;           // town layout id.
	short first_slot_id;       // the first slot id. of the layout
	short slot_object_id_array[MAX_TOWN_LAYOUT_SLOT];  // the race id. of each slot building

	//---------- game vars ----------//
	short population;
	short jobless_population;

	short	max_race_pop_array[VERSION_1_MAX_RACE];			// the MAX population the current town layout supports
	short race_pop_array[VERSION_1_MAX_RACE];     		// population of each race
	unsigned char race_pop_growth_array[VERSION_1_MAX_RACE];		// population growth, when it reaches 100, there will be one more person in the town
	short jobless_race_pop_array[VERSION_1_MAX_RACE];

	float race_loyalty_array[VERSION_1_MAX_RACE];
	char  race_target_loyalty_array[VERSION_1_MAX_RACE];
	short	race_spy_count_array[VERSION_1_MAX_RACE];		  // no. of spies in each race

	float race_resistance_array[VERSION_1_MAX_RACE][MAX_NATION];
	char  race_target_resistance_array[VERSION_1_MAX_RACE][MAX_NATION];

	short	town_defender_count;			// no. of units currently defending this town
	int	last_being_attacked_date;
	float received_hit_count;		// no. of received hit by attackers, when this > RECEIVED_HIT_PER_KILL, a town people will be killed

	char  train_queue_skill_array[MAX_TRAIN_QUEUE];	// it stores the skill id.
	char  train_queue_race_array[MAX_TRAIN_QUEUE];	// it stores the race id.
	char  train_queue_count;
	short	train_unit_recno;			// race id. of the unit the town is currently training, 0-if currently not training any
	int	train_unit_action_id;	// id. of the action to be assigned to this unit when it is finished training.
	DWORD start_train_frame_no;
	short defend_target_recno; 	// used in defend mode, store recno of latest target atttacking this town

	//-------- other vars ----------//

	int   accumulated_collect_tax_penalty;
	int   accumulated_reward_penalty;
	int   accumulated_recruit_penalty;
	int   accumulated_enemy_grant_penalty;

	int	last_rebel_date;
	short independent_unit_join_nation_min_rating;

	short quality_of_life;

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

public:
	void	convert_to_version_2(Town *townPtr);
};
#pragma pack()

#pragma pack(1)
class Version_1_NationArray// : public DynArrayB
{
	public:
		short  	nation_count;    // no. of nations, it's different from nation_array.size() which is a DynArrayB
		short  	ai_nation_count;
		int		last_del_nation_date;
		int		last_new_nation_date;

		int		max_nation_population;		// the maximum population in a nation
		int		all_nation_population;		// total population of all nations.

		short		independent_town_count;
		short		independent_town_count_race_array[VERSION_1_MAX_RACE];	// the no. of independent towns each race has

		int		max_nation_units;
		int		max_nation_humans;
		int		max_nation_generals;
		int		max_nation_weapons;
		int		max_nation_ships;
		int		max_nation_spies;

		int		max_nation_firms;
		int		max_nation_tech_level;

		int		max_population_rating;
		int		max_military_rating;
		int		max_economic_rating;
		int		max_reputation;
		int		max_kill_monster_score;
		int		max_overall_rating;

		short		max_population_nation_recno;
		short		max_military_nation_recno;
		short		max_economic_nation_recno;
		short		max_reputation_nation_recno;
		short		max_kill_monster_nation_recno;
		short		max_overall_nation_recno;

		int  	   last_alliance_id;
		int  		nation_peace_days;			// continuous peace among nations

		short  	player_recno;
		Nation* 	player_ptr;

		char		nation_color_array[MAX_NATION+1];
		char		nation_power_color_array[MAX_NATION+2];

		char		human_name_array[MAX_NATION][NationArray::HUMAN_NAME_LEN+1];

	public:
		void	convert_to_version_2(NationArray *nationArrayPtr);
};
#pragma pack()

#pragma pack(1)
class Version_1_NationBase
{
	public:
		enum { NATION_NAME_LEN=50 };
		short	nation_recno;
		char  nation_type;
		char  race_id;
		char  color_scheme_id;
		char	nation_color;				// main color of the nation, based on from color_scheme_id
		short	king_unit_recno;			// recno of the king
		char	king_leadership;
		int 	nation_name_id;			// name of the king/nation
		char  nation_name_str[NATION_NAME_LEN+1];		// for nation_name()'s use
		DWORD player_id;				   // player id for multiplayer game
		char  next_frame_ready;				// for indicating whether the next frame is ready or not
		short last_caravan_id;				// id. of the nation's caravan.
		short	nation_firm_count;			// total no. of firms the nation has built
		int   last_build_firm_date;
		char	know_base_array[VERSION_1_MAX_RACE];		// whether the unit knows how to constructure seat of power or not
		char	base_count_array[VERSION_1_MAX_RACE];   // no. of seat of power this nation has
		char	is_at_war_today;
		char	is_at_war_yesterday;
		int	last_war_date;
		short last_attacker_unit_recno;
		int	last_independent_unit_join_date;
		char  cheat_enabled_flag;
		//----------------------------------//
		float		 cash;
		float		 food;
		float		 reputation;				// can be negative, means bad reputation
		float		 kill_monster_score;
		//------- town auto policy -------------//
		short		 auto_collect_tax_loyalty;		// auto collect tax if the loyalty reaches this level
		short		 auto_grant_loyalty;				// auto grant if the loyalty drop below this level

		//----- yearly income, expense and profit ------//
		float		 cur_year_profit;
		float		 last_year_profit;
		float		 cur_year_fixed_income;
		float		 last_year_fixed_income;
		float		 cur_year_fixed_expense;
		float		 last_year_fixed_expense;
		//------- yearly income ------//
		float		 cur_year_income_array[INCOME_TYPE_COUNT];
		float		 last_year_income_array[INCOME_TYPE_COUNT];

		float		 cur_year_income;		// total income
		float		 last_year_income;

		//------- yearly expense ------//
		float		 cur_year_expense_array[EXPENSE_TYPE_COUNT];
		float		 last_year_expense_array[EXPENSE_TYPE_COUNT];

		float		 cur_year_expense;		// total expense
		float		 last_year_expense;
		//------- yearly expense ------//
		float		 cur_year_cheat;		// total cheat
		float		 last_year_cheat;

		//----- yearly food in, out and change ------//
		float		 cur_year_food_in;
		float		 last_year_food_in;

		float		 cur_year_food_out;
		float		 last_year_food_out;

		float		 cur_year_food_change;
		float		 last_year_food_change;
		//----- yearly reputatino change ------//
		float     cur_year_reputation_change;
		float     last_year_reputation_change;

		//--------- inter-nation relationship -----------//
		NationRelation relation_array[MAX_NATION];				// inter-relationship with other nations
		char		 relation_status_array[MAX_NATION];				// replace status in struct NationRelation
		char		 relation_passable_array[MAX_NATION]; 			// for seeking to indicate whether passing other nation region
		char		 relation_should_attack_array[MAX_NATION];
		char		 is_allied_with_player;								// for fast access in visiting world functions
		//---------- statistic ------------//
		int  		 total_population;
		int		 total_jobless_population;

		int		 total_unit_count;
		int		 total_human_count;
		int		 total_general_count;
		int		 total_weapon_count;
		int		 total_ship_count;
		int		 total_firm_count;
		int		 total_spy_count;
		int		 total_ship_combat_level;

		short		 largest_town_recno;		// the recno of the biggest town of this nation
		short		 largest_town_pop;

		short		 raw_count_array[MAX_RAW];		// no. of natural resources site this nation possesses
		short		 last_unit_name_id_array[VERSION_1_MAX_UNIT_TYPE];

		//--------- rank ratings ---------//
		int		 population_rating;
		int		 military_rating;
		int		 economic_rating;
		int		 overall_rating;

		//------ additional statistic ------//
		int		 enemy_soldier_killed;
		int		 own_soldier_killed;
		int		 enemy_civilian_killed;
		int		 own_civilian_killed;
		int		 enemy_weapon_destroyed;
		int		 own_weapon_destroyed;
		int		 enemy_ship_destroyed;
		int		 own_ship_destroyed;
		int		 enemy_firm_destroyed;
		int		 own_firm_destroyed;

	public:
		void	convert_to_version_2(Nation *nationPtr);

		virtual void dummy();
};
#pragma pack()

#pragma pack(1)
class Version_1_Nation : public Version_1_NationBase
{
	public:
		DynArray		action_array;
		WORD			last_action_id; 	// a 16-bit id. for identifying ActionNode
		short*		ai_town_array;
		short* 		ai_base_array;
		short* 		ai_mine_array;
		short*		ai_factory_array;
		short* 		ai_camp_array;
		short*		ai_research_array;
		short*		ai_war_array;
		short*		ai_harbor_array;
		short*      ai_market_array;
		short*		ai_inn_array;
		short*    	ai_general_array;
		short*		ai_caravan_array;
		short*		ai_ship_array;

		short			ai_town_size;
		short			ai_base_size;
		short			ai_mine_size;
		short			ai_factory_size;
		short			ai_camp_size;
		short			ai_research_size;
		short			ai_war_size;
		short			ai_harbor_size;
		short			ai_market_size;
		short			ai_inn_size;
		short			ai_general_size;
		short			ai_caravan_size;
		short			ai_ship_size;
		short			ai_town_count;
		short			ai_base_count;
		short			ai_mine_count;
		short			ai_factory_count;
		short			ai_camp_count;
		short			ai_research_count;
		short			ai_war_count;
		short			ai_harbor_count;
		short			ai_market_count;
		short			ai_inn_count;
		short			ai_general_count;
		short			ai_caravan_count;
		short			ai_ship_count;
		short			ai_base_town_count;
		short			firm_should_close_array[MAX_FIRM_TYPE];

		//------------------------------------------------------//
		// parameters about the nation itself
		//------------------------------------------------------//
		AIRegion		ai_region_array[MAX_AI_REGION];
		char			ai_region_count;

		//------------------------------------------------------//
		// AI personalties
		//------------------------------------------------------//
		char 			pref_force_projection;
		char			pref_military_development;		// pref_military_development + pref_economic_development = 100
		char			pref_economic_development;
		char			pref_inc_pop_by_capture;		// pref_inc_pop_by_capture + pref_inc_pop_by_growth = 100
		char			pref_inc_pop_by_growth;
		char			pref_peacefulness;
		char			pref_military_courage;
		char			pref_territorial_cohesiveness;
		char			pref_trading_tendency;
		char			pref_allying_tendency;
		char			pref_honesty;
		char			pref_town_harmony;
		char			pref_loyalty_concern;
		char			pref_forgiveness;
		char			pref_collect_tax;
		char			pref_hire_unit;
		char			pref_use_weapon;
		char			pref_keep_general;          	// whether to keep currently non-useful the general, or demote them.
		char			pref_keep_skilled_unit; 		// whether to keep currently non-useful skilled units, or assign them to towns.
		char			pref_diplomacy_retry;			// tedency to retry diplomatic actions after previous ones have been rejected.
		char			pref_attack_monster;
		char			pref_spy;
		char			pref_counter_spy;
		char			pref_food_reserve;
		char			pref_cash_reserve;
		char			pref_use_marine;
		char			pref_unit_chase_distance;
		char			pref_repair_concern;
		char			pref_scout;

		//------- AI action vars --------//
		short			ai_capture_enemy_town_recno;
		int  			ai_capture_enemy_town_plan_date;
		int  			ai_capture_enemy_town_start_attack_date;
		char			ai_capture_enemy_town_use_all_camp;

		int			ai_last_defend_action_date;

		short			ai_attack_target_x_loc;
		short			ai_attack_target_y_loc;
		short			ai_attack_target_nation_recno;		//	nation recno of the target

		AttackCamp  attack_camp_array[MAX_SUITABLE_ATTACK_CAMP];
		short			attack_camp_count;
		short			lead_attack_camp_recno;		// the firm recno of the lead attacking firm

	public:
		void			convert_to_version_2(Nation *nationPtr);

		virtual void dummy();
};
#pragma pack()

#endif	// AMPLUS
#endif
