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

//Filename    : OMP_CRC.CPP
//Description : crc checking for multiplayer debugging
//Owner		  : Alex

#include <CRC.h>
#include <OMP_CRC.h>
#include <OU_GOD.h>
#include <OU_VEHI.h>
#include <OU_MONS.h>
#include <OU_CART.h>
#include <OU_MARI.h>
#include <OU_CARA.h>
#include <OFIRM.h>
#include <OF_BASE.h>
#include <OF_CAMP.h>
#include <OF_FACT.h>
#include <OF_INN.h>
#include <OF_MARK.h>
#include <OF_MINE.h>
#include <OF_RESE.h>
#include <OF_WAR.h>
#include <OF_HARB.h>
#include <OF_MONS.h>
#include <OTOWN.h>
#include <ONATIONB.h>
#include <OBULLET.h>
#include <OB_PROJ.h>
#include <OB_HOMIN.h>
#include <OB_FLAME.h>
// ###### patch begin Gilbert 20/1 #######//
#include <OREBEL.h>
#include <OSPY.h>
// ###### patch end Gilbert 20/1 #######//
#include <OTALKRES.h>
#include <OVQUEUE.h>


// ###### patch begin Gilbert 21/1 #####//
#define RTRIM_ARRAY(a,s) { if(s<sizeof(a)/sizeof(*a)) memset(a+s,0,sizeof(a)-s*sizeof(*a)); }
// ###### patch end Gilbert 21/1 #####//

static union
{
	char	sprite[sizeof(SpriteCrc)];
	char	unit[sizeof(UnitCrc)];
	char	unit_god[sizeof(UnitGodCrc)];
	char	unit_vehicle[sizeof(UnitVehicleCrc)];
	char	unit_monster[sizeof(UnitMonsterCrc)];
	char	unit_exp_cart[sizeof(UnitExpCartCrc)];
	char	unit_marine[sizeof(UnitMarineCrc)];
	char	unit_caravan[sizeof(UnitCaravanCrc)];
	char	firm[sizeof(FirmCrc)];
	char	firm_base[sizeof(FirmBaseCrc)];
	char	firm_camp[sizeof(FirmCampCrc)];
	char	firm_factory[sizeof(FirmFactoryCrc)];
	char	firm_inn[sizeof(FirmInnCrc)];
	char	firm_market[sizeof(FirmMarketCrc)];
	char	firm_mine[sizeof(FirmMineCrc)];
	char	firm_research[sizeof(FirmResearchCrc)];
	char	firm_war[sizeof(FirmWarCrc)];
	char	firm_harbor[sizeof(FirmHarborCrc)];
	char	firm_monster[sizeof(FirmMonsterCrc)];
	char	town[sizeof(Town)];
	char	nation[sizeof(NationBase)];
	char	bullet[sizeof(BulletCrc)];
	char	projectile[sizeof(ProjectileCrc)];
	char	bullet_homing[sizeof(BulletHomingCrc)];
	char	bullet_flame[sizeof(BulletFlameCrc)];
	// ###### patch begin Gilbert 20/1 #######//
	char	rebel[sizeof(Rebel)];
	char	spy[sizeof(Spy)];
	// ###### patch end Gilbert 20/1 #######//
	char	talk_msg[sizeof(TalkMsg)];
} temp_obj;


//----------- End of function Sprite::crc8 -----------//
uint8_t Sprite::crc8()
{
	SpriteCrc& dummySprite = *(SpriteCrc*)temp_obj.sprite;
	init_crc(&dummySprite);

	uint8_t c = ::crc8((uint8_t*)&dummySprite, sizeof(SpriteCrc));
	return c;
}
//----------- End of function Sprite::crc8 -----------//


//----------- End of function Sprite::clear_ptr -----------//
void Sprite::clear_ptr()
{
	sprite_info = NULL;
}
//----------- End of function Sprite::clear_ptr -----------//


//--------- Begin of function Sprite::init_crc -----------//
void Sprite::init_crc(SpriteCrc *c)
{
	c->sprite_id = sprite_id;
	c->sprite_recno = sprite_recno;

	c->mobile_type = mobile_type;

	c->cur_action = cur_action;
	c->cur_dir = cur_dir;
	c->cur_frame = cur_frame;
	c->cur_attack = cur_attack;
	c->final_dir = final_dir;
	c->turn_delay = turn_delay;
	c->guard_count = guard_count;

	c->remain_attack_delay = remain_attack_delay;
	c->remain_frames_per_step = remain_frames_per_step;

	c->cur_x = cur_x;
	c->cur_y = cur_y;
	c->go_x = go_x;
	c->go_y = go_y;
	c->next_x = next_x;
	c->next_y = next_y;
}
//----------- End of function Sprite::init_crc -----------//


//----------- End of function Unit::crc8 -----------//
uint8_t Unit::crc8()
{
	UnitCrc &dummyUnit = *(UnitCrc *)temp_obj.unit;
	init_crc(&dummyUnit);

	uint8_t c = ::crc8((uint8_t*)&dummyUnit, sizeof(UnitCrc));
	return c;
}
//----------- End of function Unit::crc8 -----------//


//----------- End of function Unit::clear_ptr -----------//
void Unit::clear_ptr()
{
	Sprite::clear_ptr();

	selected_flag = 0;
	group_select_id = 0;
	
	attack_info_array = NULL;
   result_node_array = NULL;
	way_point_array	= NULL;
   team_info = NULL;

	if( !original_action_mode )
	{
		original_action_para = 0;
		original_action_x_loc = 0;
		original_action_y_loc = 0;
		original_target_x_loc = 0;
		original_target_y_loc = 0;

		ai_original_target_x_loc = 0;
		ai_original_target_y_loc = 0;
	}
}
//----------- End of function Unit::clear_ptr -----------//


//--------- Begin of function Unit::init_crc -----------//
void Unit::init_crc(UnitCrc *c)
{
	Sprite::init_crc((SpriteCrc*)c);

	c->unit_id = unit_id;
	c->rank_id = rank_id;
	c->race_id = race_id;
	c->nation_recno = nation_recno;
	c->ai_unit = ai_unit;
	c->name_id = name_id;

	c->unit_group_id = unit_group_id;
	c->team_id = team_id;

	c->waiting_term = waiting_term;
	c->blocked_by_member = blocked_by_member;
	c->swapping = swapping;

	c->leader_unit_recno = leader_unit_recno;

	c->action_misc = action_misc;
	c->action_misc_para = action_misc_para;

	c->action_mode = action_mode;
	c->action_para = action_para;
	c->action_x_loc = action_x_loc;
	c->action_y_loc = action_y_loc;

	c->action_mode2 = action_mode2;
	c->action_para2 = action_para2;
	c->action_x_loc2 = action_x_loc2;
	c->action_y_loc2 = action_y_loc2;

	memcpy(c->blocked_edge, blocked_edge, 4);
	c->attack_dir = attack_dir;

	c->range_attack_x_loc = range_attack_x_loc;
	c->range_attack_y_loc = range_attack_y_loc;

	c->move_to_x_loc = move_to_x_loc;
	c->move_to_y_loc = move_to_y_loc;

	c->loyalty = loyalty;
	c->target_loyalty = target_loyalty;

	c->hit_points = hit_points;
	c->max_hit_points = max_hit_points;

	c->skill = skill;

	c->unit_mode = unit_mode;
	c->unit_mode_para = unit_mode_para;

	c->spy_recno = spy_recno;

	c->nation_contribution = nation_contribution;
	c->total_reward = total_reward;

	c->attack_count = attack_count;
	c->attack_range = attack_range;
	c->cur_power = cur_power;
	c->max_power = max_power;

	c->result_node_count = result_node_count;
	c->result_node_recno = result_node_recno;
	c->result_path_dist = result_path_dist;

	c->way_point_array_size = way_point_array_size;
	c->way_point_count = way_point_count;

	c->ai_action_id = ai_action_id;

	c->original_action_mode = original_action_mode;
	if( original_action_mode )
	{
		c->original_action_para = original_action_para;
		c->original_action_x_loc = original_action_x_loc;
		c->original_action_y_loc = original_action_y_loc;

		c->original_target_x_loc = original_target_x_loc;
		c->original_target_y_loc = original_target_y_loc;

		c->ai_original_target_x_loc = ai_original_target_x_loc;
		c->ai_original_target_y_loc = ai_original_target_y_loc;
	}
	else
	{
		c->original_action_para = 0;
		c->original_action_x_loc = 0;
		c->original_action_y_loc = 0;

		c->original_target_x_loc = 0;
		c->original_target_y_loc = 0;

		c->ai_original_target_x_loc = 0;
		c->ai_original_target_y_loc = 0;
	}

	c->ai_no_suitable_action = ai_no_suitable_action;

	c->can_guard_flag = can_guard_flag;

	c->can_attack_flag = can_attack_flag;
	c->force_move_flag = force_move_flag;

	c->home_camp_firm_recno = home_camp_firm_recno;

	c->aggressive_mode = aggressive_mode;

	c->seek_path_fail_count = seek_path_fail_count;
	c->ignore_power_nation = ignore_power_nation;
}
//----------- End of function Unit::init_crc -----------//


//----------- End of function UnitGod::crc8 -----------//
uint8_t UnitGod::crc8()
{
	UnitGodCrc &dummyUnitGod = *(UnitGodCrc *)temp_obj.unit_god;
	init_crc(&dummyUnitGod);

	uint8_t c = ::crc8((uint8_t*)&dummyUnitGod, sizeof(UnitGodCrc));
	return c;
}
//----------- End of function UnitGod::crc8 -----------//


//----------- End of function UnitGod::clear_ptr -----------//
void UnitGod::clear_ptr()
{
	Unit::clear_ptr();

	if( !cast_power_type )
	{
		cast_origin_x = 0;
		cast_origin_y = 0;
		cast_target_x = 0;
		cast_target_y = 0;
	}

}
//----------- End of function UnitGod::clear_ptr -----------//


//--------- Begin of function UnitGod::init_crc -----------//
void UnitGod::init_crc(UnitGodCrc *c)
{
	Unit::init_crc((UnitCrc*)c);

	c->god_id = god_id;
	c->base_firm_recno = base_firm_recno;
	c->cast_power_type = cast_power_type;
	if( cast_power_type )
	{
		c->cast_origin_x = cast_origin_x;
		c->cast_origin_y = cast_origin_y;
		c->cast_target_x = cast_target_x;
		c->cast_target_y = cast_target_y;
	}
	else
	{
		c->cast_origin_x = 0;
		c->cast_origin_y = 0;
		c->cast_target_x = 0;
		c->cast_target_y = 0;
	}
}
//----------- End of function UnitGod::init_crc -----------//


//----------- End of function UnitVehicle::crc8 -----------//
uint8_t UnitVehicle::crc8()
{
	UnitVehicleCrc &dummyUnitVehicle = *(UnitVehicleCrc *)temp_obj.unit_vehicle;
	init_crc(&dummyUnitVehicle);

	uint8_t c = ::crc8((uint8_t*)&dummyUnitVehicle, sizeof(UnitVehicleCrc));
	return c;
}
//----------- End of function UnitVehicle::crc8 -----------//


//----------- End of function UnitVehicle::clear_ptr -----------//
void UnitVehicle::clear_ptr()
{
	Unit::clear_ptr();
}
//----------- End of function UnitVehicle::clear_ptr -----------//


//--------- Begin of function UnitVehicle::init_crc -----------//
void UnitVehicle::init_crc(UnitVehicleCrc *c)
{
	Unit::init_crc((UnitCrc*)c);

	c->solider_hit_points = solider_hit_points;
	c->vehicle_hit_points = vehicle_hit_points;
}
//----------- End of function UnitVehicle::init_crc -----------//


//----------- End of function UnitMonster::crc8 -----------//
uint8_t UnitMonster::crc8()
{
	UnitMonsterCrc &dummyUnitMonster = *(UnitMonsterCrc *)temp_obj.unit_monster;
	init_crc(&dummyUnitMonster);

	uint8_t c = ::crc8((uint8_t*)&dummyUnitMonster, sizeof(UnitMonsterCrc));
	return c;
}
//----------- End of function UnitMonster::crc8 -----------//


//----------- End of function UnitMonster::clear_ptr -----------//
void UnitMonster::clear_ptr()
{
	Unit::clear_ptr();
}
//----------- End of function UnitMonster::clear_ptr -----------//


//--------- Begin of function UnitMonster::init_crc -----------//
void UnitMonster::init_crc(UnitMonsterCrc *c)
{
	Unit::init_crc((UnitCrc*)c);

	c->monster_action_mode = monster_action_mode;
}
//----------- End of function UnitMonster::init_crc -----------//


//----------- End of function UnitExpCart::crc8 -----------//
uint8_t UnitExpCart::crc8()
{
	UnitExpCartCrc &dummyUnitExpCart = *(UnitExpCartCrc *)temp_obj.unit_exp_cart;
	init_crc(&dummyUnitExpCart);

	uint8_t c = ::crc8((uint8_t*)&dummyUnitExpCart, sizeof(UnitExpCartCrc));
	return c;
}
//----------- End of function UnitExpCart::crc8 -----------//


//----------- End of function UnitExpCart::clear_ptr -----------//
void UnitExpCart::clear_ptr()
{
	Unit::clear_ptr();
}
//----------- End of function UnitExpCart::clear_ptr -----------//


//--------- Begin of function UnitExpCart::init_crc -----------//
void UnitExpCart::init_crc(UnitExpCartCrc *c)
{
	Unit::init_crc((UnitCrc*)c);

	c->triggered = triggered;
}
//----------- End of function UnitExpCart::init_crc -----------//


//----------- End of function UnitMarine::crc8 -----------//
uint8_t UnitMarine::crc8()
{
	UnitMarineCrc &dummyUnitMarine = *(UnitMarineCrc *)temp_obj.unit_marine;
	init_crc(&dummyUnitMarine);

	uint8_t c = ::crc8((uint8_t*)&dummyUnitMarine, sizeof(UnitMarineCrc));
	return c;
}
//----------- End of function UnitMarine::crc8 -----------//


//----------- End of function UnitMarine::clear_ptr -----------//
void UnitMarine::clear_ptr()
{
	memset(&splash, 0, sizeof(splash));
	
	//### begin alex 23/10 ###//
	selected_unit_id = 0;
	menu_mode = 0;
	//#### end alex 23/10 ####//

	// ###### patch begin Gilbert 21/1 ######//
	// must do this step before clear_ptr(), attack_info_array is reset there
	if( !attack_info_array )
		memset(&ship_attack_info, 0, sizeof(ship_attack_info));
	// ###### patch end Gilbert 21/1 ######//

	Unit::clear_ptr();

	RTRIM_ARRAY(unit_recno_array, unit_count);
	for( int i = 0; i < sizeof(stop_array)/sizeof(*stop_array); ++i)
	{
		if( !stop_array[i].firm_recno )
		{
			memset(&stop_array[i], 0, sizeof(*stop_array));
		}
	}
}
//----------- End of function UnitMarine::clear_ptr -----------//


//--------- Begin of function UnitMarine::init_crc -----------//
void UnitMarine::init_crc(UnitMarineCrc *c)
{
	Unit::init_crc((UnitCrc*)c);

	c->extra_move_in_beach = extra_move_in_beach;
	c->in_beach = in_beach;

	memcpy(&c->unit_recno_array, &unit_recno_array, sizeof(short)*MAX_UNIT_IN_SHIP);
	c->unit_count = unit_count;
	RTRIM_ARRAY(&c->unit_recno_array, unit_count);

	c->journey_status = journey_status;
	c->dest_stop_id = dest_stop_id;
	c->stop_defined_num = stop_defined_num;
	c->wait_count = wait_count;

	c->stop_x_loc = stop_x_loc;
	c->stop_y_loc = stop_y_loc;

	c->auto_mode = auto_mode;
	c->cur_firm_recno = cur_firm_recno;
	c->carry_goods_capacity = carry_goods_capacity;

	for( int i = 0; i < sizeof(stop_array)/sizeof(*stop_array); ++i)
	{
		if( stop_array[i].firm_recno )
		{
			memcpy(&c->stop_array[i], &stop_array[i], sizeof(*stop_array));
		}
		else
		{
			memset(&c->stop_array[i], 0, sizeof(*stop_array));
		}
	}

	memcpy(&c->raw_qty_array, raw_qty_array, sizeof(raw_qty_array));
	memcpy(&c->product_raw_qty_array, product_raw_qty_array, sizeof(product_raw_qty_array));

	if( attack_info_array )
		memcpy(&c->ship_attack_info, &ship_attack_info, sizeof(ship_attack_info));
	else
		memset(&c->ship_attack_info, 0, sizeof(ship_attack_info));
	c->attack_mode_selected = attack_mode_selected;

	c->last_load_goods_date = last_load_goods_date;
}
//----------- End of function UnitMarine::init_crc -----------//


//----------- End of function UnitCaravan::crc8 -----------//
uint8_t UnitCaravan::crc8()
{
	UnitCaravanCrc &dummyUnitCaravan = *(UnitCaravanCrc *)temp_obj.unit_caravan;
	init_crc(&dummyUnitCaravan);

	uint8_t c = ::crc8((uint8_t*)&dummyUnitCaravan, sizeof(UnitCaravanCrc));
	return c;
}
//----------- End of function UnitCaravan::crc8 -----------//


//----------- End of function UnitCaravan::clear_ptr -----------//
void UnitCaravan::clear_ptr()
{
	Unit::clear_ptr();

	caravan_id = 0;	// caravan_id is no longer valid

	for( int i = 0; i < sizeof(stop_array)/sizeof(*stop_array); ++i)
	{
		if( !stop_array[i].firm_recno )
		{
			memset(&stop_array[i], 0, sizeof(*stop_array));
		}
	}
}
//----------- End of function UnitCaravan::clear_ptr -----------//


//--------- Begin of function UnitCaravan::init_crc -----------//
void UnitCaravan::init_crc(UnitCaravanCrc *c)
{
	Unit::init_crc((UnitCrc*)c);

	c->journey_status = journey_status;
	c->dest_stop_id = dest_stop_id;
	c->stop_defined_num = stop_defined_num;
	c->wait_count = wait_count;

	c->stop_x_loc = stop_x_loc;
	c->stop_y_loc = stop_y_loc;

	for( int i = 0; i < sizeof(stop_array)/sizeof(*stop_array); ++i)
	{
		if( stop_array[i].firm_recno )
		{
			memcpy(&c->stop_array[i], &stop_array[i], sizeof(*stop_array));
		}
		else
		{
			memset(&c->stop_array[i], 0, sizeof(*stop_array));
		}
	}

	c->last_set_stop_date = last_set_stop_date;
	c->last_load_goods_date = last_load_goods_date;

	memcpy(&c->raw_qty_array, raw_qty_array, sizeof(raw_qty_array));
	memcpy(&c->product_raw_qty_array, product_raw_qty_array, sizeof(product_raw_qty_array));
}
//----------- End of function UnitCaravan::init_crc -----------//


//----------- End of function Firm::crc8 -----------//
uint8_t Firm::crc8()
{
	FirmCrc &dummyFirm = *(FirmCrc *)temp_obj.firm;
	init_crc(&dummyFirm);

	uint8_t c = ::crc8((uint8_t*)&dummyFirm, sizeof(FirmCrc));
	return c;
}
//----------- End of function Firm::crc8 -----------//


//----------- End of function Firm::clear_ptr -----------//
void Firm::clear_ptr()
{
	worker_array = NULL;
	selected_worker_id = 0;
	player_spy_count = 0;

	// clear unused element in linked_firm_array, linked_firm_enable_array
	RTRIM_ARRAY(linked_firm_array, linked_firm_count);
	RTRIM_ARRAY(linked_firm_enable_array, linked_firm_count);

	// clear unused element in linked_town_array, linked_town_enable_array
	RTRIM_ARRAY(linked_town_array, linked_town_count);
	RTRIM_ARRAY(linked_town_enable_array, linked_town_count);
}
//----------- End of function Firm::clear_ptr -----------//


//--------- Begin of function Firm::init_crc -----------//
void Firm::init_crc(FirmCrc *c)
{
	c->firm_id = firm_id;
	c->firm_build_id = firm_build_id;
	c->firm_recno = firm_recno;
	c->firm_ai = firm_ai;
	c->ai_processed = ai_processed;
	c->ai_status = ai_status;
	c->ai_link_checked = ai_link_checked;
	c->ai_sell_flag = ai_sell_flag;

	c->race_id = race_id;
	c->nation_recno = nation_recno;

	c->closest_town_name_id = closest_town_name_id;
	c->firm_name_instance_id = firm_name_instance_id;

	c->loc_x1 = loc_x1;
	c->loc_y1 = loc_y1;
	c->loc_x2 = loc_x2;
	c->loc_y2 = loc_y2;
	c->abs_x1 = abs_x1;
	c->abs_y1 = abs_y1;
	c->abs_x2 = abs_x2;
	c->abs_y2 = abs_y2;
	c->center_x = center_x;
	c->center_y = center_y;
	c->region_id = region_id;

	c->cur_frame = cur_frame;
	c->remain_frame_delay = remain_frame_delay;

	c->hit_points = hit_points;
	c->max_hit_points = max_hit_points;
	c->under_construction = under_construction;

	c->firm_skill_id = firm_skill_id;
	c->overseer_recno = overseer_recno;
	c->overseer_town_recno = overseer_town_recno;
	c->builder_recno = builder_recno;
	c->builder_region_id = builder_region_id;
	c->productivity = productivity;

	c->worker_count = worker_count;

	c->sabotage_level = sabotage_level;

	c->linked_firm_count = linked_firm_count;
	c->linked_town_count = linked_town_count;

	memcpy(&c->linked_firm_array, linked_firm_array, sizeof(linked_firm_array));
	RTRIM_ARRAY(c->linked_firm_array, linked_firm_count);
	memcpy(&c->linked_town_array, linked_town_array, sizeof(linked_town_array));
	RTRIM_ARRAY(c->linked_firm_enable_array, linked_firm_count);

	memcpy(&c->linked_firm_enable_array, linked_firm_enable_array, sizeof(linked_firm_enable_array));
	RTRIM_ARRAY(c->linked_town_array, linked_town_count);
	memcpy(&c->linked_town_enable_array, linked_town_enable_array, sizeof(linked_town_enable_array));
	RTRIM_ARRAY(c->linked_town_enable_array, linked_town_count);

	c->last_year_income = last_year_income;
	c->cur_year_income = cur_year_income;

	c->setup_date = setup_date;

	c->should_set_power = should_set_power;
	c->last_attacked_date = last_attacked_date;

	c->should_close_flag = should_close_flag;
	c->no_neighbor_space = no_neighbor_space;
	c->ai_should_build_factory_count = ai_should_build_factory_count;
}
//----------- End of function Firm::init_crc -----------//


//----------- End of function FirmBase::crc8 -----------//
uint8_t FirmBase::crc8()
{
	FirmBaseCrc &dummyFirmBase = *(FirmBaseCrc *)temp_obj.firm_base;
	init_crc(&dummyFirmBase);

	uint8_t c = ::crc8((uint8_t*)&dummyFirmBase, sizeof(FirmBaseCrc));
	return c;
}
//----------- End of function FirmBase::crc8 -----------//


//----------- End of function FirmBase::clear_ptr -----------//
void FirmBase::clear_ptr()
{
	Firm::clear_ptr();
}
//----------- End of function FirmBase::clear_ptr -----------//


//--------- Begin of function FirmBase::init_crc -----------//
void FirmBase::init_crc(FirmBaseCrc *c)
{
	Firm::init_crc((FirmCrc*)c);

	c->god_id = god_id;
	c->god_unit_recno = god_unit_recno;

	c->pray_points = pray_points;
}
//----------- End of function FirmBase::init_crc -----------//


//----------- End of function FirmCamp::crc8 -----------//
uint8_t FirmCamp::crc8()
{
	FirmCampCrc &dummyFirmCamp = *(FirmCampCrc *)temp_obj.firm_camp;
	init_crc(&dummyFirmCamp);

	uint8_t c = ::crc8((uint8_t*)&dummyFirmCamp, sizeof(FirmCampCrc));
	return c;
}
//----------- End of function FirmCamp::crc8 -----------//


//----------- End of function FirmCamp::clear_ptr -----------//
void FirmCamp::clear_ptr()
{
	Firm::clear_ptr();

	// clear unused element in defense_array
	for( int i = 0; i < sizeof(defense_array)/sizeof(*defense_array); ++i )
	{
		if( !defense_array[i].unit_recno )
		{
			memset(&defense_array[i], 0, sizeof(*defense_array));
		}
	}

	// clear unused element in patrol_unit_array
	RTRIM_ARRAY(patrol_unit_array, patrol_unit_count);
	
	// clear unused element in coming_unit_array
	RTRIM_ARRAY(coming_unit_array, coming_unit_count);
}
//----------- End of function FirmCamp::clear_ptr -----------//


//--------- Begin of function FirmCamp::init_crc -----------//
void FirmCamp::init_crc(FirmCampCrc *c)
{
	Firm::init_crc((FirmCrc*)c);

	for( int i = 0; i < sizeof(defense_array)/sizeof(*defense_array); ++i )
	{
		if( defense_array[i].unit_recno )
		{
			memcpy(&c->defense_array[i], &defense_array[i], sizeof(*defense_array));
		}
		else
		{
			memset(&c->defense_array[i], 0, sizeof(*defense_array));
		}
	}
	c->employ_new_worker = employ_new_worker;
	c->defend_target_recno = defend_target_recno;
	c->defense_flag = defense_flag;

	c->patrol_unit_count = patrol_unit_count;
	memcpy(&c->patrol_unit_array, &patrol_unit_array, sizeof(patrol_unit_array));
	RTRIM_ARRAY(c->patrol_unit_array, patrol_unit_count);

	c->coming_unit_count = coming_unit_count;
	memcpy(&c->coming_unit_array, &coming_unit_array, sizeof(coming_unit_array));
	RTRIM_ARRAY(c->coming_unit_array, coming_unit_count);

	c->ai_capture_town_recno = ai_capture_town_recno;
	c->ai_recruiting_soldier = ai_recruiting_soldier;

	c->is_attack_camp = is_attack_camp;
}
//----------- End of function FirmCamp::init_crc -----------//


//----------- End of function FirmFactory::crc8 -----------//
uint8_t FirmFactory::crc8()
{
	FirmFactoryCrc &dummyFirmFactory = *(FirmFactoryCrc *)temp_obj.firm_factory;
	init_crc(&dummyFirmFactory);

	uint8_t c = ::crc8((uint8_t*)&dummyFirmFactory, sizeof(FirmFactoryCrc));
	return c;
}
//----------- End of function FirmFactory::crc8 -----------//


//----------- End of function FirmFactory::clear_ptr -----------//
void FirmFactory::clear_ptr()
{
	Firm::clear_ptr();
}
//----------- End of function FirmFactory::clear_ptr -----------//


//--------- Begin of function FirmFactory::init_crc -----------//
void FirmFactory::init_crc(FirmFactoryCrc *c)
{
	Firm::init_crc((FirmCrc*)c);

	c->product_raw_id = product_raw_id;

	c->stock_qty = stock_qty;
	c->max_stock_qty = max_stock_qty;

	c->raw_stock_qty = raw_stock_qty;
	c->max_raw_stock_qty = max_raw_stock_qty;

	c->cur_month_production = cur_month_production;
	c->last_month_production = last_month_production;
	c->next_output_link_id = next_output_link_id;
	c->next_output_firm_recno = next_output_firm_recno;
}
//----------- End of function FirmFactory::init_crc -----------//


//----------- End of function FirmInn::crc8 -----------//
uint8_t FirmInn::crc8()
{
	FirmInnCrc &dummyFirmInn = *(FirmInnCrc *)temp_obj.firm_inn;
	init_crc(&dummyFirmInn);

	uint8_t c = ::crc8((uint8_t*)&dummyFirmInn, sizeof(FirmInnCrc));
	return c;
}
//----------- End of function FirmInn::crc8 -----------//


//----------- End of function FirmInn::clear_ptr -----------//
void FirmInn::clear_ptr()
{
	Firm::clear_ptr();

	// clear unused element in inn_unit_array
	RTRIM_ARRAY(inn_unit_array, inn_unit_count);
}
//----------- End of function FirmInn::clear_ptr -----------//


//--------- Begin of function FirmInn::init_crc -----------//
void FirmInn::init_crc(FirmInnCrc *c)
{
	Firm::init_crc((FirmCrc*)c);

	c->next_skill_id = next_skill_id;

	memcpy(&c->inn_unit_array, &inn_unit_array, sizeof(inn_unit_array));
	RTRIM_ARRAY(c->inn_unit_array, inn_unit_count);
	c->inn_unit_count = inn_unit_count;
}
//----------- End of function FirmInn::init_crc -----------//


//----------- End of function FirmMarket::crc8 -----------//
uint8_t FirmMarket::crc8()
{
	FirmMarketCrc &dummyFirmMarket = *(FirmMarketCrc *)temp_obj.firm_market;
	init_crc(&dummyFirmMarket);

	uint8_t c = ::crc8((uint8_t*)&dummyFirmMarket, sizeof(FirmMarketCrc));
	return c;
}
//----------- End of function FirmMarket::crc8 -----------//


//----------- End of function FirmMarket::clear_ptr -----------//
void FirmMarket::clear_ptr()
{
	int i;
	for(i=0; i<MAX_RAW; ++i)
		market_raw_array[i] = NULL;

	for(i=0; i<MAX_PRODUCT; ++i)
		market_product_array[i] = NULL;

	// clear unused element in defense_array
	for( i = 0; i < sizeof(market_goods_array)/sizeof(*market_goods_array); ++i )
	{
		if( !market_goods_array[i].raw_id && !market_goods_array[i].product_raw_id )
		{
			memset(&market_goods_array[i], 0, sizeof(*market_goods_array));
		}
	}

	Firm::clear_ptr();
}
//----------- End of function FirmMarket::clear_ptr -----------//


//--------- Begin of function FirmMarket::init_crc -----------//
void FirmMarket::init_crc(FirmMarketCrc *c)
{
	Firm::init_crc((FirmCrc*)c);

	c->max_stock_qty = max_stock_qty;

	for( int i = 0; i < sizeof(market_goods_array)/sizeof(*market_goods_array); ++i )
	{
		if( market_goods_array[i].raw_id || market_goods_array[i].product_raw_id )
		{
			memcpy(&c->market_goods_array[i], &market_goods_array[i], sizeof(*market_goods_array));
		}
		else
		{
			memset(&c->market_goods_array[i], 0, sizeof(*market_goods_array));
		}
	}

	c->next_output_link_id = next_output_link_id;
	c->next_output_firm_recno = next_output_firm_recno;

	c->no_linked_town_since_date = no_linked_town_since_date;
	c->last_import_new_goods_date = last_import_new_goods_date;
	c->is_retail_market = is_retail_market;
}
//----------- End of function FirmMarket::init_crc -----------//


//----------- End of function FirmMine::crc8 -----------//
uint8_t FirmMine::crc8()
{
	FirmMineCrc &dummyFirmMine = *(FirmMineCrc *)temp_obj.firm_mine;
	init_crc(&dummyFirmMine);

	uint8_t c = ::crc8((uint8_t*)&dummyFirmMine, sizeof(FirmMineCrc));
	return c;
}
//----------- End of function FirmMine::crc8 -----------//


//----------- End of function FirmMine::clear_ptr -----------//
void FirmMine::clear_ptr()
{
	Firm::clear_ptr();
}
//----------- End of function FirmMine::clear_ptr -----------//


//--------- Begin of function FirmMine::init_crc -----------//
void FirmMine::init_crc(FirmMineCrc *c)
{
	Firm::init_crc((FirmCrc*)c);

	c->raw_id = raw_id;
	c->site_recno = site_recno;
	c->reserve_qty = reserve_qty;
	c->stock_qty = stock_qty;
	c->max_stock_qty = max_stock_qty;

	c->next_output_link_id = next_output_link_id;
	c->next_output_firm_recno = next_output_firm_recno;

	c->cur_month_production = cur_month_production;
	c->last_month_production = last_month_production;
}
//----------- End of function FirmMine::init_crc -----------//


//----------- End of function FirmResearch::crc8 -----------//
uint8_t FirmResearch::crc8()
{
	FirmResearchCrc &dummyFirmResearch = *(FirmResearchCrc *)temp_obj.firm_research;
	init_crc(&dummyFirmResearch);

	uint8_t c = ::crc8((uint8_t*)&dummyFirmResearch, sizeof(FirmResearchCrc));
	return c;
}
//----------- End of function FirmResearch::crc8 -----------//


//----------- End of function FirmResearch::clear_ptr -----------//
void FirmResearch::clear_ptr()
{
	Firm::clear_ptr();
}
//----------- End of function FirmResearch::clear_ptr -----------//


//--------- Begin of function FirmResearch::init_crc -----------//
void FirmResearch::init_crc(FirmResearchCrc *c)
{
	Firm::init_crc((FirmCrc*)c);

	c->tech_id = tech_id;
	c->complete_percent = complete_percent;
}
//----------- End of function FirmResearch::init_crc -----------//


//----------- End of function FirmWar::crc8 -----------//
uint8_t FirmWar::crc8()
{
	FirmWarCrc &dummyFirmWar = *(FirmWarCrc *)temp_obj.firm_war;
	init_crc(&dummyFirmWar);

	uint8_t c = ::crc8((uint8_t*)&dummyFirmWar, sizeof(FirmWarCrc));
	return c;
}
//----------- End of function FirmWar::crc8 -----------//


//----------- End of function FirmWar::clear_ptr -----------//
void FirmWar::clear_ptr()
{
	Firm::clear_ptr();

	// if nothing building clear some variables
	if( !build_unit_id )
	{
		last_process_build_frame_no = 0;
		build_progress_days = (float)0.0;
	}

	// clear unused build_queue_array
	RTRIM_ARRAY(build_queue_array, build_queue_count);
}
//----------- End of function FirmWar::clear_ptr -----------//


//--------- Begin of function FirmWar::init_crc -----------//
void FirmWar::init_crc(FirmWarCrc *c)
{
	Firm::init_crc((FirmCrc*)c);

	c->build_unit_id = build_unit_id;
	if( build_unit_id )
	{
		c->last_process_build_frame_no = last_process_build_frame_no;
		c->build_progress_days = build_progress_days;
	}
	else
	{
		c->last_process_build_frame_no = 0;
		c->build_progress_days = (float)0.0;
	}

	memcpy(&c->build_queue_array, &build_queue_array, sizeof(build_queue_array));
	RTRIM_ARRAY(c->build_queue_array, build_queue_count);
	c->build_queue_count = build_queue_count;
}
//----------- End of function FirmWar::init_crc -----------//


//----------- End of function FirmHarbor::crc8 -----------//
uint8_t FirmHarbor::crc8()
{
	FirmHarborCrc &dummyFirmHarbor = *(FirmHarborCrc *)temp_obj.firm_harbor;
	init_crc(&dummyFirmHarbor);

	uint8_t c = ::crc8((uint8_t*)&dummyFirmHarbor, sizeof(FirmHarborCrc));
	return c;
}
//----------- End of function FirmHarbor::crc8 -----------//


//----------- End of function FirmHarbor::clear_ptr -----------//
void FirmHarbor::clear_ptr()
{
	Firm::clear_ptr();

	if(!build_unit_id)
		start_build_frame_no = 0;

	RTRIM_ARRAY(ship_recno_array, ship_count);
	RTRIM_ARRAY(build_queue_array, build_queue_count);
	RTRIM_ARRAY(linked_mine_array, linked_mine_num);
	RTRIM_ARRAY(linked_factory_array, linked_factory_num);
	RTRIM_ARRAY(linked_market_array, linked_market_num);
}
//----------- End of function FirmHarbor::clear_ptr -----------//


//--------- Begin of function FirmHarbor::init_crc -----------//
void FirmHarbor::init_crc(FirmHarborCrc *c)
{
	Firm::init_crc((FirmCrc*)c);

	memcpy(&c->ship_recno_array, &ship_recno_array, sizeof(ship_recno_array));
	RTRIM_ARRAY(c->ship_recno_array, ship_count);
	c->ship_count = ship_count;

	c->build_unit_id = build_unit_id;
	if( build_unit_id )
		c->start_build_frame_no = start_build_frame_no;
	else
		c->start_build_frame_no = 0;

	memcpy(&c->build_queue_array, &build_queue_array, sizeof(build_queue_array));
	RTRIM_ARRAY(c->build_queue_array, build_queue_count);
	c->build_queue_count = build_queue_count;

	c->land_region_id = land_region_id;
	c->sea_region_id = sea_region_id;

	c->link_checked = link_checked;
	c->linked_mine_num = linked_mine_num;
	c->linked_factory_num = linked_factory_num;
	c->linked_market_num = linked_market_num;
	memcpy(&c->linked_mine_array, &linked_mine_array, sizeof(linked_mine_array));
	RTRIM_ARRAY(c->linked_mine_array, linked_mine_num);
	memcpy(&c->linked_factory_array, &linked_factory_array, sizeof(linked_factory_array));
	RTRIM_ARRAY(c->linked_factory_array, linked_factory_num);
	memcpy(&c->linked_market_array, &linked_market_array, sizeof(linked_market_array));
	RTRIM_ARRAY(c->linked_market_array, linked_market_num);
}
//----------- End of function FirmHarbor::init_crc -----------//


//----------- End of function FirmMonster::crc8 -----------//
uint8_t FirmMonster::crc8()
{
	FirmMonsterCrc &dummyFirmMonster = *(FirmMonsterCrc *)temp_obj.firm_monster;
	init_crc(&dummyFirmMonster);

	uint8_t c = ::crc8((uint8_t*)&dummyFirmMonster, sizeof(FirmMonsterCrc));
	return c;
}
//----------- End of function FirmMonster::crc8 -----------//


//----------- End of function FirmMonster::clear_ptr -----------//
void FirmMonster::clear_ptr()
{
	Firm::clear_ptr();

	if( !monster_king.monster_id )
	{
		monster_king.mobile_unit_recno = 0;
		monster_king.combat_level = 0;
		monster_king.hit_points = 0;
		monster_king.max_hit_points = 0;
	}

	RTRIM_ARRAY(monster_general_array, monster_general_count);
	RTRIM_ARRAY(waiting_soldier_array, waiting_soldier_count);
	RTRIM_ARRAY(patrol_unit_array, patrol_unit_count);
}
//----------- End of function FirmMonster::clear_ptr -----------//


//--------- Begin of function FirmMonster::init_crc -----------//
void FirmMonster::init_crc(FirmMonsterCrc *c)
{
	Firm::init_crc((FirmCrc*)c);

	c->monster_id = monster_id;
	c->monster_general_count = monster_general_count;

	c->monster_aggressiveness = monster_aggressiveness;

	c->defending_king_count = defending_king_count;
	c->defending_general_count = defending_general_count;
	c->defending_soldier_count = defending_soldier_count;

	if( monster_king.monster_id )
		c->monster_king = monster_king;
	else
		memset(&c->monster_king, 0, sizeof(monster_king));
	memcpy(&c->monster_general_array, &monster_general_array, sizeof(monster_general_array));
	RTRIM_ARRAY(c->monster_general_array, monster_general_count);

	c->waiting_soldier_count = waiting_soldier_count;
	memcpy(&c->waiting_soldier_array, &waiting_soldier_array, sizeof(waiting_soldier_array));
	RTRIM_ARRAY(c->waiting_soldier_array, waiting_soldier_count);

	c->monster_nation_relation = monster_nation_relation;
	c->defend_target_recno = defend_target_recno;

	c->patrol_unit_count = patrol_unit_count;
	memcpy(&c->patrol_unit_array, &patrol_unit_array, sizeof(patrol_unit_array));
	RTRIM_ARRAY(c->patrol_unit_array, patrol_unit_count);
}
//----------- End of function FirmMonster::init_crc -----------//


//----------- End of function Town::crc8 -----------//
uint8_t Town::crc8()
{
	Town &dummyTown = *(Town *)temp_obj.town;
	memcpy(&dummyTown, this, sizeof(Town));

	dummyTown.clear_ptr();
	// to clear virtual table pointer, possibly in the future
	if( (void *)&dummyTown != (void *)&dummyTown.town_recno )
		*((char**) &dummyTown) = NULL;

	uint8_t c = ::crc8((uint8_t*)&dummyTown, sizeof(Town));
	return c;
}
//----------- End of function Town::crc8 -----------//


//----------- End of function Town::clear_ptr -----------//
void Town::clear_ptr()
{
	int layoutCount = town_res.get_layout(layout_id)->slot_count;
	RTRIM_ARRAY(slot_object_id_array, layoutCount);

	RTRIM_ARRAY(train_queue_skill_array, train_queue_count);
	RTRIM_ARRAY(train_queue_race_array, train_queue_count);

	RTRIM_ARRAY(linked_firm_array, linked_firm_count);
	RTRIM_ARRAY(linked_firm_enable_array, linked_firm_count);

	RTRIM_ARRAY(linked_town_array, linked_town_count);
	RTRIM_ARRAY(linked_town_enable_array, linked_town_count);
}
//----------- End of function Town::clear_ptr -----------//


//----------- End of function NationBase::crc8 -----------//
uint8_t NationBase::crc8()
{
	NationBase &dummyNationBase = *(NationBase *)temp_obj.nation;
	memcpy(&dummyNationBase, this, sizeof(NationBase));

	dummyNationBase.clear_ptr();
	*((char**) &dummyNationBase) = NULL;

	uint8_t c = ::crc8((uint8_t*)&dummyNationBase, sizeof(NationBase));
	return c;
}
//----------- End of function NationBase::crc8 -----------//


//----------- End of function NationBase::clear_ptr -----------//
void NationBase::clear_ptr()
{
	nation_type = 0;
	memset(nation_name_str, 0, sizeof(nation_name_str) );    // garbage may exist after the '\0'
	player_id = 0;
	next_frame_ready = 0;
	is_allied_with_player = 0;

	// ignore contact_msg_flag in relation_array
	for(short nationRecno=1; nationRecno <= MAX_NATION; ++nationRecno)
	{
		get_relation(nationRecno)->contact_msg_flag = 0;
	}
}
//----------- End of function NationBase::clear_ptr -----------//


//----------- End of function Bullet::crc8 -----------//
uint8_t Bullet::crc8()
{
	BulletCrc &dummyBullet = *(BulletCrc *)temp_obj.bullet;
	init_crc(&dummyBullet);

	uint8_t c = ::crc8((uint8_t*)&dummyBullet, sizeof(BulletCrc));
	return c;
}
//----------- End of function Bullet::crc8 -----------//


//----------- End of function Bullet::clear_ptr -----------//
void Bullet::clear_ptr()
{
	Sprite::clear_ptr();
}
//----------- End of function Bullet::clear_ptr -----------//


//--------- Begin of function Bullet::init_crc -----------//
void Bullet::init_crc(BulletCrc *c)
{
	Sprite::init_crc((SpriteCrc*)c);

	c->parent_type = parent_type;
	c->parent_recno = parent_recno;

	c->mobile_type = mobile_type;
	c->target_mobile_type = target_mobile_type;
	c->attack_damage = attack_damage;
	c->damage_radius = damage_radius;
	c->nation_recno = nation_recno;
	c->fire_radius = fire_radius;

	c->origin_x = origin_x;
	c->origin_y = origin_y;
	c->target_x_loc = target_x_loc;
	c->target_y_loc = target_y_loc;
	c->cur_step = cur_step;
	c->total_step = total_step;
}
//----------- End of function Bullet::init_crc -----------//


//----------- End of function Projectile::crc8 -----------//
uint8_t Projectile::crc8()
{
	ProjectileCrc &dummyProjectile = *(ProjectileCrc *)temp_obj.projectile;
	init_crc(&dummyProjectile);

	uint8_t c = ::crc8((uint8_t*)&dummyProjectile, sizeof(ProjectileCrc));
	return c;
}
//----------- End of function Projectile::crc8 -----------//


//----------- End of function Projectile::clear_ptr -----------//
void Projectile::clear_ptr()
{
	memset(&act_bullet, 0, sizeof(act_bullet));
	memset(&bullet_shadow, 0, sizeof(bullet_shadow));
	
	Bullet::clear_ptr();
}
//----------- End of function Projectile::clear_ptr -----------//


//--------- Begin of function Projectile::init_crc -----------//
void Projectile::init_crc(ProjectileCrc *c)
{
	Bullet::init_crc((BulletCrc*)c);

	c->z_coff = z_coff;
}
//----------- End of function Projectile::init_crc -----------//


//----------- End of function BulletHoming::crc8 -----------//
uint8_t BulletHoming::crc8()
{
	BulletHomingCrc &dummyBulletHoming = *(BulletHomingCrc *)temp_obj.bullet_homing;
	init_crc(&dummyBulletHoming);

	uint8_t c = ::crc8((uint8_t*)&dummyBulletHoming, sizeof(BulletHomingCrc));
	return c;
}
//----------- End of function BulletHoming::crc8 -----------//


//----------- End of function BulletHoming::clear_ptr -----------//
void BulletHoming::clear_ptr()
{
	Bullet::clear_ptr();
}
//----------- End of function BulletHoming::clear_ptr -----------//


//--------- Begin of function BulletHoming::init_crc -----------//
void BulletHoming::init_crc(BulletHomingCrc *c)
{
	Bullet::init_crc((BulletCrc*)c);

	c->max_step = max_step;
	c->target_type = target_type;
	c->target_recno = target_recno;
	c->speed = speed;
	c->origin2_x = origin2_x;
	c->origin2_y = origin2_y;
}
//----------- End of function BulletHoming::init_crc -----------//


//----------- End of function BulletFlame::crc8 -----------//
uint8_t BulletFlame::crc8()
{
	BulletFlameCrc &dummyBulletFlame = *(BulletFlameCrc *)temp_obj.bullet_flame;
	init_crc(&dummyBulletFlame);

	uint8_t c = ::crc8((uint8_t*)&dummyBulletFlame, sizeof(BulletFlameCrc));
	return c;
}
//----------- End of function BulletFlame::crc8 -----------//


//----------- Begin of function BulletFlame::clear_ptr -----------//
void BulletFlame::clear_ptr()
{
	Bullet::clear_ptr();
}
//----------- End of function BulletFlame::clear_ptr -----------//


//--------- Begin of function BulletFlame::init_crc -----------//
void BulletFlame::init_crc(BulletFlameCrc *c)
{
	Bullet::init_crc((BulletCrc*)c);
}
//----------- End of function BulletFlame::init_crc -----------//


// ###### patch begin Gilbert 20/1 #######//
//----------- Begin of function Rebel::crc8 -----------//
uint8_t Rebel::crc8()
{
	Rebel &dummyRebel = *(Rebel *)temp_obj.rebel;
	memcpy(&dummyRebel, this, sizeof(Rebel));

	dummyRebel.clear_ptr();

	uint8_t c = ::crc8((uint8_t *)&dummyRebel, sizeof(Rebel));
	return c;
}
//----------- End of function Rebel::crc8 -----------//


//----------- Begin of function Rebel::clear_ptr -----------//
void Rebel::clear_ptr()
{
}
//----------- End of function Rebel::clear_ptr -----------//


//----------- Begin of function Spy::crc8 -----------//
uint8_t Spy::crc8()
{
	Spy &dummySpy = *(Spy *)temp_obj.spy;
	memcpy(&dummySpy, this, sizeof(Spy));

	dummySpy.clear_ptr();

	uint8_t c = ::crc8((uint8_t *)&dummySpy, sizeof(Spy));
	return c;
}
//----------- End of function Spy::crc8 -----------//


//----------- Begin of function Spy::clear_ptr -----------//
void Spy::clear_ptr()
{
}
//----------- End of function Spy::clear_ptr -----------//
// ###### patch end Gilbert 20/1 #######//

//----------- Begin of function TalkMsg::crc8 -----------//
uint8_t TalkMsg::crc8()
{
	TalkMsg &dummyTalkMsg = *(TalkMsg *)temp_obj.talk_msg;
	memcpy(&dummyTalkMsg, this, sizeof(TalkMsg));

	dummyTalkMsg.clear_ptr();
	// *((char**) &dummyTalkMsg) = NULL;

	uint8_t c = ::crc8((uint8_t*)&dummyTalkMsg, sizeof(TalkMsg));
	return c;
}
//----------- End of function TalkMsg::crc8 -----------//

//----------- Begin of function TalkMsg::clear_ptr -----------//
void TalkMsg::clear_ptr()
{
}
//----------- End of function TalkMsg::clear_ptr -----------//


//----------- Begin of function VLenQueue::crc8 -----------//
uint8_t VLenQueue::crc8()
{
	return ::crc8((uint8_t*)queue_buf, queued_size);
}
//----------- End of function VLenQueue::crc8 -----------//
