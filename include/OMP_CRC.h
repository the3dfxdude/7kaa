/*
 * Seven Kingdoms: Ancient Adversaries
 *
 * Copyright 2018 Jesse Allen
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

//Filename    : OMP_CRC.H
//Description : crc checking for multiplayer debugging

#ifndef _OMP_CRC_H
#define _OMP_CRC_H

#include <stdint.h>

#include <OF_CAMP.h>
#include <OF_HARB.h>
#include <OF_INN.h>
#include <OF_MARK.h>
#include <OF_MONS.h>
#include <OF_WAR.h>
#include <OSKILL.h>
#include <OU_CARA.h>
#include <OU_MARI.h>

#pragma pack(1)
struct FirmCrc
{
	char		firm_id;
	short		firm_build_id;
	short		firm_recno;
	char		firm_ai;
	char		ai_processed;
	char		ai_status;
	char		ai_link_checked;
	char		ai_sell_flag;

	char		race_id;
	short		nation_recno;

	short		closest_town_name_id;
	short		firm_name_instance_id;

	short		loc_x1;
	short		loc_y1;
	short		loc_x2;
	short		loc_y2;
	short		abs_x1;
	short		abs_y1;
	short		abs_x2;
	short		abs_y2;
	short		center_x;
	short		center_y;
	uint8_t		region_id;

	char		cur_frame;
	char		remain_frame_delay;

	float		hit_points;
	float		max_hit_points;
	char		under_construction;

	char		firm_skill_id;
	short		overseer_recno;
	short		overseer_town_recno;
	short		builder_recno;
	uint8_t		builder_region_id;
	float		productivity;

	char		worker_count;

	uint8_t		sabotage_level;

	char		linked_firm_count;
	char		linked_town_count;

	short		linked_firm_array[MAX_LINKED_FIRM_FIRM];
	short		linked_town_array[MAX_LINKED_FIRM_TOWN];

	char		linked_firm_enable_array[MAX_LINKED_FIRM_FIRM];
	char		linked_town_enable_array[MAX_LINKED_FIRM_TOWN];

	float		last_year_income;
	float		cur_year_income;

	int		setup_date;

	char		should_set_power;
	int		last_attacked_date;

	char		should_close_flag;
	char		no_neighbor_space;
	char		ai_should_build_factory_count;
};

struct FirmBaseCrc : FirmCrc
{
	short		god_id;
	short		god_unit_recno;

	float		pray_points;
};

struct FirmCampCrc : FirmCrc
{
	DefenseUnit	defense_array[MAX_WORKER+1];
	char		employ_new_worker;
	short		defend_target_recno;
	char		defense_flag;

	char		patrol_unit_count;
	short		patrol_unit_array[MAX_WORKER+1];

	char		coming_unit_count;
	short		coming_unit_array[MAX_WORKER+1];

	short		ai_capture_town_recno;
	char		ai_recruiting_soldier;

	char		is_attack_camp;
};

struct FirmFactoryCrc : FirmCrc
{
	int		product_raw_id;

	float		stock_qty;
	float		max_stock_qty;

	float		raw_stock_qty;
	float		max_raw_stock_qty;

	float		cur_month_production;
	float		last_month_production;
	short		next_output_link_id;
	short		next_output_firm_recno;
};


struct FirmHarborCrc : FirmCrc
{
	short		ship_recno_array[MAX_SHIP_IN_HARBOR];
	short		ship_count;

	short		build_unit_id;
	uint32_t	start_build_frame_no;

	char		build_queue_array[MAX_BUILD_SHIP_QUEUE];
	char		build_queue_count;

	uint8_t		land_region_id;
	uint8_t		sea_region_id;

	char		link_checked;
	char		linked_mine_num;
	char		linked_factory_num;
	char		linked_market_num;
	short		linked_mine_array[MAX_LINKED_FIRM_FIRM];
	short		linked_factory_array[MAX_LINKED_FIRM_FIRM];
	short		linked_market_array[MAX_LINKED_FIRM_FIRM];
};

struct FirmInnCrc : FirmCrc
{
	short		next_skill_id;

	InnUnit		inn_unit_array[MAX_INN_UNIT];
	short		inn_unit_count;
};

struct FirmMarketCrc : FirmCrc
{
	float		max_stock_qty;

	MarketGoods	market_goods_array[MAX_MARKET_GOODS];

	short		next_output_link_id;
	short		next_output_firm_recno;

	int		no_linked_town_since_date;
	int		last_import_new_goods_date;
	char		restock_type;
};

struct FirmMineCrc : FirmCrc
{
	short		raw_id;
	short		site_recno;
	float		reserve_qty;
	float		stock_qty;
	float		max_stock_qty;

	short		next_output_link_id;
	short		next_output_firm_recno;

	float		cur_month_production;
	float		last_month_production;
};

struct FirmMonsterCrc : FirmCrc
{
	short		monster_id;
	short		monster_general_count;

	char		monster_aggressiveness;

	char		defending_king_count;
	char		defending_general_count;
	char		defending_soldier_count;

	MonsterInFirm	monster_king;
	MonsterInFirm	monster_general_array[MAX_MONSTER_GENERAL_IN_FIRM];

	char		waiting_soldier_count;
	short		waiting_soldier_array[FirmMonster::MAX_WAITING_SOLDIER];

	char		monster_nation_relation;
	short		defend_target_recno;

	char		patrol_unit_count;
	short		patrol_unit_array[MAX_SOLDIER_PER_GENERAL+1];
};

struct FirmResearchCrc : FirmCrc
{
	short		tech_id;
	float		complete_percent;
};

struct FirmWarCrc : FirmCrc
{
	short		build_unit_id;
	uint32_t	last_process_build_frame_no;
	float		build_progress_days;

	char		build_queue_array[MAX_BUILD_QUEUE];
	char		build_queue_count;
};

struct SpriteCrc
{
	short		sprite_id;
	short		sprite_recno;

	char		mobile_type;

	uint8_t		cur_action;
	uint8_t		cur_dir;
	uint8_t		cur_frame;
	uint8_t		cur_attack;
	uint8_t		final_dir;
	char		turn_delay;
	char		guard_count;

	uint8_t		remain_attack_delay;
	uint8_t		remain_frames_per_step;

	short		cur_x;
	short		cur_y;
	short		go_x;
	short		go_y;
	short		next_x;
	short		next_y;
};

struct BulletCrc : SpriteCrc
{
	char		parent_type;
	short		parent_recno;

	char		target_mobile_type;
	float		attack_damage;
	short		damage_radius;
	short		nation_recno;
	char		fire_radius;

	short		origin_x;
	short		origin_y;
	short		target_x_loc;
	short		target_y_loc;
	char		cur_step;
	char		total_step;
};

struct BulletFlameCrc : BulletCrc
{
};

struct BulletHomingCrc : BulletCrc
{
	char		max_step;
	char		target_type;
	short		target_recno;
	short		speed;
	short		origin2_x;
	short		origin2_y;
};

struct ProjectileCrc : BulletCrc
{
	float		z_coff;
};

struct UnitCrc : SpriteCrc
{
	char		unit_id;
	char		rank_id;
	char		race_id;
	char		nation_recno;
	char		ai_unit;
	uint16_t	name_id;

	uint32_t	unit_group_id;
	uint32_t	team_id;

	char		waiting_term;
	char		blocked_by_member;
	char		swapping;

	short		leader_unit_recno;

	char		action_misc;
	short		action_misc_para;

	char		action_mode;
	short		action_para;
	short		action_x_loc;
	short		action_y_loc;

	char		action_mode2;
	short		action_para2;
	short		action_x_loc2;
	short		action_y_loc2;

	char		blocked_edge[4];
	uint8_t		attack_dir;

	short		range_attack_x_loc;
	short		range_attack_y_loc;

	short		move_to_x_loc;
	short		move_to_y_loc;

	char		loyalty;
	char		target_loyalty;

	float		hit_points;
	short		max_hit_points;

	Skill		skill;

	char		unit_mode;
	short		unit_mode_para;

	short		spy_recno;

	short		nation_contribution;
	short		total_reward;

	char		attack_count;
	char		attack_range;
	short		cur_power;
	short		max_power;

	int		result_node_count;
	short		result_node_recno;
	short		result_path_dist;

	short		way_point_array_size;
	short		way_point_count;

	uint16_t	ai_action_id;

	char		original_action_mode;
	short		original_action_para;
	short		original_action_x_loc;
	short		original_action_y_loc;

	short		original_target_x_loc;
	short		original_target_y_loc;

	short		ai_original_target_x_loc;
	short		ai_original_target_y_loc;

	char		ai_no_suitable_action;

	char		can_guard_flag;
	char		can_attack_flag;
	char		force_move_flag;

	short		home_camp_firm_recno;

	char		aggressive_mode;

	char		seek_path_fail_count;
	char		ignore_power_nation;
};

struct UnitGodCrc : UnitCrc
{
	short		god_id;
	short		base_firm_recno;
	char		cast_power_type;
	short		cast_origin_x;
	short		cast_origin_y;
	short		cast_target_x;
	short		cast_target_y;
};

struct UnitVehicleCrc : UnitCrc
{
	short		solider_hit_points;
	short		vehicle_hit_points;
};

struct UnitMonsterCrc : UnitCrc
{
	char		monster_action_mode;
};

struct UnitExpCartCrc : UnitCrc
{
	char		triggered;
};

struct UnitMarineCrc : UnitCrc
{
	char		extra_move_in_beach;
	char		in_beach;

	short		unit_recno_array[MAX_UNIT_IN_SHIP];
	char		unit_count;

	char		journey_status;
	char		dest_stop_id;
	char		stop_defined_num;
	char		wait_count;

	short		stop_x_loc;
	short		stop_y_loc;

	char		auto_mode;
	short		cur_firm_recno;
	short		carry_goods_capacity;

	ShipStop	stop_array[MAX_STOP_FOR_SHIP];

	short		raw_qty_array[MAX_RAW];
	short		product_raw_qty_array[MAX_PRODUCT];

	AttackInfo	ship_attack_info;
	uint8_t		attack_mode_selected;

	int		last_load_goods_date;
};

struct UnitCaravanCrc : UnitCrc
{
	char		journey_status;
	char		dest_stop_id;
	char		stop_defined_num;
	char		wait_count;

	short		stop_x_loc;
	short		stop_y_loc;

	CaravanStop	stop_array[MAX_STOP_FOR_CARAVAN];

	int		last_set_stop_date;
	int		last_load_goods_date;

	short		raw_qty_array[MAX_RAW];
	short		product_raw_qty_array[MAX_PRODUCT];
};
#pragma pack()

#endif
