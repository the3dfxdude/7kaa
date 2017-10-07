/*
* Seven Kingdoms: Ancient Adversaries
*
* Copyright 1997,1998 Enlight Software Ltd.
* Copyright 2010 Unavowed <unavowed@vexillium.org>
* Copyright 2017 Richard Dijk <microvirus.multiplying@gmail.com>
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

#include <ONATION.h>
#include <OGFILE.h>
#include <OGF_V1.h>
#include <file_io_visitor.h>
#include <OGFILE_DYNARRAY.inl>
#include <OGFILE_DYNARRAYB.inl>

using namespace FileIOVisitor;

//------- declare static functions -------//

static void visit_version_1_nation(FileReaderVisitor *v, Nation *n);
template <typename Visitor>
static void visit_version_1_nation_members(Visitor *v, Version_1_Nation *v1n);
template <typename Visitor>
static void visit_version_1_nation_array(Visitor *v, Version_1_NationArray *na);


template <typename Visitor>
static void visit_nation_relation(Visitor *v, NationRelation *nr)
{
	visit<int8_t>(v, &nr->has_contact);
	visit<int8_t>(v, &nr->should_attack);

	visit<int8_t>(v, &nr->trade_treaty);

	visit<int8_t>(v, &nr->status);

	visit<int32_t>(v, &nr->last_change_status_date);

	visit<int8_t>(v, &nr->ai_relation_level);
	visit<int8_t>(v, &nr->ai_secret_attack);
	visit<int8_t>(v, &nr->ai_demand_trade_treaty);

	visit<float>(v, &nr->good_relation_duration_rating);
	visit<int16_t>(v, &nr->started_war_on_us_count);

	visit_array<float>(v, nr->cur_year_import);
	visit_array<float>(v, nr->last_year_import);
	visit_array<float>(v, nr->lifetime_import);

	visit_array<int32_t>(v, nr->last_talk_reject_date_array);

	visit<int32_t>(v, &nr->last_military_aid_date);

	visit<int32_t>(v, &nr->last_give_gift_date);
	visit<int16_t>(v, &nr->total_given_gift_amount);

	visit<int8_t>(v, &nr->contact_msg_flag);
}

template <typename Visitor>
static void visit_attack_camp(Visitor *v, AttackCamp *ac)
{
	visit<int16_t>(v, &ac->firm_recno);
	visit<int16_t>(v, &ac->combat_level);
	visit<int16_t>(v, &ac->distance);
	visit<int32_t>(v, &ac->patrol_date);
}

template <typename Visitor>
static void visit_ai_region(Visitor *v, AIRegion *reg)
{
	visit<int8_t>(v, &reg->region_id);
	visit<int8_t>(v, &reg->town_count);
	visit<int8_t>(v, &reg->base_town_count);
}

template <typename Visitor>
static void visit_nation_members(Visitor *v, Nation *nat)
{
	v->skip(4); /* virtual table pointer */

	/* NationBase */
	visit<int16_t>(v, &nat->nation_recno);
	visit<int8_t>(v, &nat->nation_type);
	visit<int8_t>(v, &nat->race_id);
	visit<int8_t>(v, &nat->color_scheme_id);
	visit<int8_t>(v, &nat->nation_color);
	visit<int16_t>(v, &nat->king_unit_recno);
	visit<int8_t>(v, &nat->king_leadership);
	visit<int32_t>(v, &nat->nation_name_id);
	visit_array<int8_t>(v, nat->nation_name_str);
	visit<uint32_t>(v, &nat->player_id);
	visit<int8_t>(v, &nat->next_frame_ready);
	visit<int16_t>(v, &nat->last_caravan_id);
	visit<int16_t>(v, &nat->nation_firm_count);
	visit<int32_t>(v, &nat->last_build_firm_date);
	visit_array<int8_t>(v, nat->know_base_array);
	visit_array<int8_t>(v, nat->base_count_array);
	visit<int8_t>(v, &nat->is_at_war_today);
	visit<int8_t>(v, &nat->is_at_war_yesterday);
	visit<int32_t>(v, &nat->last_war_date);
	visit<int16_t>(v, &nat->last_attacker_unit_recno);
	visit<int32_t>(v, &nat->last_independent_unit_join_date);
	visit<int8_t>(v, &nat->cheat_enabled_flag);
	visit<float>(v, &nat->cash);
	visit<float>(v, &nat->food);
	visit<float>(v, &nat->reputation);
	visit<float>(v, &nat->kill_monster_score);
	visit<int16_t>(v, &nat->auto_collect_tax_loyalty);
	visit<int16_t>(v, &nat->auto_grant_loyalty);
	visit<float>(v, &nat->cur_year_profit);
	visit<float>(v, &nat->last_year_profit);
	visit<float>(v, &nat->cur_year_fixed_income);
	visit<float>(v, &nat->last_year_fixed_income);
	visit<float>(v, &nat->cur_year_fixed_expense);
	visit<float>(v, &nat->last_year_fixed_expense);
	visit_array<float>(v, nat->cur_year_income_array);
	visit_array<float>(v, nat->last_year_income_array);
	visit<float>(v, &nat->cur_year_income);
	visit<float>(v, &nat->last_year_income);
	visit_array<float>(v, nat->cur_year_expense_array);
	visit_array<float>(v, nat->last_year_expense_array);
	visit<float>(v, &nat->cur_year_expense);
	visit<float>(v, &nat->last_year_expense);
	visit<float>(v, &nat->cur_year_cheat);
	visit<float>(v, &nat->last_year_cheat);
	visit<float>(v, &nat->cur_year_food_in);
	visit<float>(v, &nat->last_year_food_in);
	visit<float>(v, &nat->cur_year_food_out);
	visit<float>(v, &nat->last_year_food_out);
	visit<float>(v, &nat->cur_year_food_change);
	visit<float>(v, &nat->last_year_food_change);
	visit<float>(v, &nat->cur_year_reputation_change);
	visit<float>(v, &nat->last_year_reputation_change);

	for (int n = 0; n < MAX_NATION; n++)
		visit_nation_relation(v, &nat->relation_array[n]);

	visit_array<int8_t>(v, nat->relation_status_array);
	visit_array<int8_t>(v, nat->relation_passable_array);
	visit_array<int8_t>(v, nat->relation_should_attack_array);
	visit<int8_t>(v, &nat->is_allied_with_player);
	visit<int32_t>(v, &nat->total_population);
	visit<int32_t>(v, &nat->total_jobless_population);
	visit<int32_t>(v, &nat->total_unit_count);
	visit<int32_t>(v, &nat->total_human_count);
	visit<int32_t>(v, &nat->total_general_count);
	visit<int32_t>(v, &nat->total_weapon_count);
	visit<int32_t>(v, &nat->total_ship_count);
	visit<int32_t>(v, &nat->total_firm_count);
	visit<int32_t>(v, &nat->total_spy_count);
	visit<int32_t>(v, &nat->total_ship_combat_level);
	visit<int16_t>(v, &nat->largest_town_recno);
	visit<int16_t>(v, &nat->largest_town_pop);
	visit_array<int16_t>(v, nat->raw_count_array);
	visit_array<uint16_t>(v, nat->last_unit_name_id_array);
	visit<int32_t>(v, &nat->population_rating);
	visit<int32_t>(v, &nat->military_rating);
	visit<int32_t>(v, &nat->economic_rating);
	visit<int32_t>(v, &nat->overall_rating);
	visit<int32_t>(v, &nat->enemy_soldier_killed);
	visit<int32_t>(v, &nat->own_soldier_killed);
	visit<int32_t>(v, &nat->enemy_civilian_killed);
	visit<int32_t>(v, &nat->own_civilian_killed);
	visit<int32_t>(v, &nat->enemy_weapon_destroyed);
	visit<int32_t>(v, &nat->own_weapon_destroyed);
	visit<int32_t>(v, &nat->enemy_ship_destroyed);
	visit<int32_t>(v, &nat->own_ship_destroyed);
	visit<int32_t>(v, &nat->enemy_firm_destroyed);
	visit<int32_t>(v, &nat->own_firm_destroyed);

	/* Nation */
	v->skip(29); /* action_array */

	visit<uint16_t>(v, &nat->last_action_id);
	visit_pointer(v, &nat->ai_town_array);
	visit_pointer(v, &nat->ai_base_array);
	visit_pointer(v, &nat->ai_mine_array);
	visit_pointer(v, &nat->ai_factory_array);
	visit_pointer(v, &nat->ai_camp_array);
	visit_pointer(v, &nat->ai_research_array);
	visit_pointer(v, &nat->ai_war_array);
	visit_pointer(v, &nat->ai_harbor_array);
	visit_pointer(v, &nat->ai_market_array);
	visit_pointer(v, &nat->ai_inn_array);
	visit_pointer(v, &nat->ai_general_array);
	visit_pointer(v, &nat->ai_caravan_array);
	visit_pointer(v, &nat->ai_ship_array);
	visit<int16_t>(v, &nat->ai_town_size);
	visit<int16_t>(v, &nat->ai_base_size);
	visit<int16_t>(v, &nat->ai_mine_size);
	visit<int16_t>(v, &nat->ai_factory_size);
	visit<int16_t>(v, &nat->ai_camp_size);
	visit<int16_t>(v, &nat->ai_research_size);
	visit<int16_t>(v, &nat->ai_war_size);
	visit<int16_t>(v, &nat->ai_harbor_size);
	visit<int16_t>(v, &nat->ai_market_size);
	visit<int16_t>(v, &nat->ai_inn_size);
	visit<int16_t>(v, &nat->ai_general_size);
	visit<int16_t>(v, &nat->ai_caravan_size);
	visit<int16_t>(v, &nat->ai_ship_size);
	visit<int16_t>(v, &nat->ai_town_count);
	visit<int16_t>(v, &nat->ai_base_count);
	visit<int16_t>(v, &nat->ai_mine_count);
	visit<int16_t>(v, &nat->ai_factory_count);
	visit<int16_t>(v, &nat->ai_camp_count);
	visit<int16_t>(v, &nat->ai_research_count);
	visit<int16_t>(v, &nat->ai_war_count);
	visit<int16_t>(v, &nat->ai_harbor_count);
	visit<int16_t>(v, &nat->ai_market_count);
	visit<int16_t>(v, &nat->ai_inn_count);
	visit<int16_t>(v, &nat->ai_general_count);
	visit<int16_t>(v, &nat->ai_caravan_count);
	visit<int16_t>(v, &nat->ai_ship_count);
	visit<int16_t>(v, &nat->ai_base_town_count);
	visit_array<int16_t>(v, nat->firm_should_close_array);

	for (int n = 0; n < MAX_AI_REGION; n++)
		visit_ai_region(v, &nat->ai_region_array[n]);

	visit<int8_t>(v, &nat->ai_region_count);
	visit<int8_t>(v, &nat->pref_force_projection);
	visit<int8_t>(v, &nat->pref_military_development);
	visit<int8_t>(v, &nat->pref_economic_development);
	visit<int8_t>(v, &nat->pref_inc_pop_by_capture);
	visit<int8_t>(v, &nat->pref_inc_pop_by_growth);
	visit<int8_t>(v, &nat->pref_peacefulness);
	visit<int8_t>(v, &nat->pref_military_courage);
	visit<int8_t>(v, &nat->pref_territorial_cohesiveness);
	visit<int8_t>(v, &nat->pref_trading_tendency);
	visit<int8_t>(v, &nat->pref_allying_tendency);
	visit<int8_t>(v, &nat->pref_honesty);
	visit<int8_t>(v, &nat->pref_town_harmony);
	visit<int8_t>(v, &nat->pref_loyalty_concern);
	visit<int8_t>(v, &nat->pref_forgiveness);
	visit<int8_t>(v, &nat->pref_collect_tax);
	visit<int8_t>(v, &nat->pref_hire_unit);
	visit<int8_t>(v, &nat->pref_use_weapon);
	visit<int8_t>(v, &nat->pref_keep_general);
	visit<int8_t>(v, &nat->pref_keep_skilled_unit);
	visit<int8_t>(v, &nat->pref_diplomacy_retry);
	visit<int8_t>(v, &nat->pref_attack_monster);
	visit<int8_t>(v, &nat->pref_spy);
	visit<int8_t>(v, &nat->pref_counter_spy);
	visit<int8_t>(v, &nat->pref_food_reserve);
	visit<int8_t>(v, &nat->pref_cash_reserve);
	visit<int8_t>(v, &nat->pref_use_marine);
	visit<int8_t>(v, &nat->pref_unit_chase_distance);
	visit<int8_t>(v, &nat->pref_repair_concern);
	visit<int8_t>(v, &nat->pref_scout);
	visit<int16_t>(v, &nat->ai_capture_enemy_town_recno);
	visit<int32_t>(v, &nat->ai_capture_enemy_town_plan_date);
	visit<int32_t>(v, &nat->ai_capture_enemy_town_start_attack_date);
	visit<int8_t>(v, &nat->ai_capture_enemy_town_use_all_camp);
	visit<int32_t>(v, &nat->ai_last_defend_action_date);
	visit<int16_t>(v, &nat->ai_attack_target_x_loc);
	visit<int16_t>(v, &nat->ai_attack_target_y_loc);
	visit<int16_t>(v, &nat->ai_attack_target_nation_recno);

	for (int n = 0; n < MAX_SUITABLE_ATTACK_CAMP; n++)
		visit_attack_camp(v, &nat->attack_camp_array[n]);

	visit<int16_t>(v, &nat->attack_camp_count);
	visit<int16_t>(v, &nat->lead_attack_camp_recno);
}

template <typename Visitor>
static void visit_ai_info(Visitor* v, short*& aiInfoArray, short& aiInfoCount, short& aiInfoSize)
{
	visit<int16_t>(v, &aiInfoCount);
	visit_property<short, int16_t>(v, [aiInfoSize]() {return aiInfoSize;},
		[&aiInfoSize, &aiInfoArray](short visitSize) {
			aiInfoSize = visitSize;
			aiInfoArray = (short*) mem_add(visitSize * sizeof(short));
		});

	v->with_record_size(aiInfoCount * sizeof(short));
	for (int i = 0; i < aiInfoCount; ++i) {
		visit<int16_t>(v, &aiInfoArray[i]);
	}
}

template <typename Visitor>
static void visit_nation(Visitor *v, Nation *nat)
{
	visit_nation_members(v, nat);
	if (!v->good())
		return;

	//-------------- read AI Action Array --------------//

	nat->action_array.accept_visitor_as_value_array(v, visit_raw<Visitor, ActionNode>);

	//------ write AI info array ---------//

	visit_ai_info(v, nat->ai_town_array, nat->ai_town_count, nat->ai_town_size);
	visit_ai_info(v, nat->ai_base_array, nat->ai_base_count, nat->ai_base_size);
	visit_ai_info(v, nat->ai_mine_array, nat->ai_mine_count, nat->ai_mine_size);
	visit_ai_info(v, nat->ai_factory_array, nat->ai_factory_count, nat->ai_factory_size);
	visit_ai_info(v, nat->ai_market_array, nat->ai_market_count, nat->ai_market_size);
	visit_ai_info(v, nat->ai_inn_array, nat->ai_inn_count, nat->ai_inn_size);
	visit_ai_info(v, nat->ai_camp_array, nat->ai_camp_count, nat->ai_camp_size);
	visit_ai_info(v, nat->ai_research_array, nat->ai_research_count, nat->ai_research_size);
	visit_ai_info(v, nat->ai_war_array, nat->ai_war_count, nat->ai_war_size);
	visit_ai_info(v, nat->ai_harbor_array, nat->ai_harbor_count, nat->ai_harbor_size);
	visit_ai_info(v, nat->ai_caravan_array, nat->ai_caravan_count, nat->ai_caravan_size);
	visit_ai_info(v, nat->ai_ship_array, nat->ai_ship_count, nat->ai_ship_size);
	visit_ai_info(v, nat->ai_general_array, nat->ai_general_count, nat->ai_general_size);
}

template <typename Visitor>
static void visit_nation_array(Visitor *v, NationArray *na)
{
	/* DynArray and DynArrayB skipped */

	visit<int16_t>(v, &na->nation_count);
	visit<int16_t>(v, &na->ai_nation_count);
	visit<int32_t>(v, &na->last_del_nation_date);
	visit<int32_t>(v, &na->last_new_nation_date);
	visit<int32_t>(v, &na->max_nation_population);
	visit<int32_t>(v, &na->all_nation_population);
	visit<int16_t>(v, &na->independent_town_count);
	visit_array<int16_t>(v, na->independent_town_count_race_array);
	visit<int32_t>(v, &na->max_nation_units);
	visit<int32_t>(v, &na->max_nation_humans);
	visit<int32_t>(v, &na->max_nation_generals);
	visit<int32_t>(v, &na->max_nation_weapons);
	visit<int32_t>(v, &na->max_nation_ships);
	visit<int32_t>(v, &na->max_nation_spies);
	visit<int32_t>(v, &na->max_nation_firms);
	visit<int32_t>(v, &na->max_nation_tech_level);
	visit<int32_t>(v, &na->max_population_rating);
	visit<int32_t>(v, &na->max_military_rating);
	visit<int32_t>(v, &na->max_economic_rating);
	visit<int32_t>(v, &na->max_reputation);
	visit<int32_t>(v, &na->max_kill_monster_score);
	visit<int32_t>(v, &na->max_overall_rating);
	visit<int16_t>(v, &na->max_population_nation_recno);
	visit<int16_t>(v, &na->max_military_nation_recno);
	visit<int16_t>(v, &na->max_economic_nation_recno);
	visit<int16_t>(v, &na->max_reputation_nation_recno);
	visit<int16_t>(v, &na->max_kill_monster_nation_recno);
	visit<int16_t>(v, &na->max_overall_nation_recno);
	visit<int32_t>(v, &na->last_alliance_id);
	visit<int32_t>(v, &na->nation_peace_days);
	visit<int16_t>(v, &na->player_recno);
	visit_pointer(v, &na->player_ptr);
	visit_array<int8_t>(v, na->nation_color_array);
	visit_array<int8_t>(v, na->nation_power_color_array);

	for (int n = 0; n < MAX_NATION; n++)
		visit_array<int8_t>(v, na->human_name_array[n]);
}


enum { NATION_ARRAY_RECORD_SIZE = 288 };
enum { NATION_RECORD_SIZE = 2202 };

//-------- Start of function NationArray::write_file -------------//
//
int NationArray::write_file(File* filePtr)
{
	FileWriterVisitor v(filePtr);

	//------ write info in NationArray ------//

	v.with_record_size(NATION_ARRAY_RECORD_SIZE);
	visit_nation_array(&v, this);
	if (!v.good())
		return 0;

	//---------- write Nations --------------//

	accept_visitor_as_ptr_array<Nation>(&v, []() {return new Nation ();}, visit_nation<FileWriterVisitor>, NATION_RECORD_SIZE);

	return v.good();
}
//--------- End of function NationArray::write_file -------------//


enum { VERSION_1_NATION_ARRAY_RECORD_SIZE = 282 };
enum { VERSION_1_NATION_RECORD_SIZE = 2182 };

//-------- Start of function NationArray::read_file -------------//
//
int NationArray::read_file(File* filePtr)
{
	FileReaderVisitor v(filePtr);

	//------ read info in NationArray ------//
	if(!GameFile::read_file_same_version)
	{
		Version_1_NationArray *oldNationArrayPtr = (Version_1_NationArray*) mem_add(sizeof(Version_1_NationArray));
		if (!visit_with_record_size<FileReaderVisitor>(filePtr, oldNationArrayPtr,
			&visit_version_1_nation_array<FileReaderVisitor>,
			VERSION_1_NATION_ARRAY_RECORD_SIZE))
		{
			mem_del(oldNationArrayPtr);
			return 0;
		}
		oldNationArrayPtr->convert_to_version_2(this);
		mem_del(oldNationArrayPtr);

		accept_visitor_as_ptr_array<Nation>(&v, []() {return new Nation ();}, visit_version_1_nation, VERSION_1_NATION_RECORD_SIZE);
	}
	else
	{
		v.with_record_size(NATION_ARRAY_RECORD_SIZE);
		visit_nation_array(&v, this);
		if (!v.good())
			return 0;

		accept_visitor_as_ptr_array<Nation>(&v, []() {return new Nation ();}, visit_nation<FileReaderVisitor>, NATION_RECORD_SIZE);
	}

	player_ptr = nation_array[player_recno];

	return v.good();
}
//--------- End of function NationArray::read_file ---------------//


// ================= Version 1 file =================


// Note: only have an overload for FileReaderVisitor, because we'll only ever read v1 nations.
// Note 2: we're visiting version 1 nation, but then converting directly to Nation.
static void visit_version_1_nation(FileReaderVisitor *v, Nation *n)
{
	Version_1_Nation *oldNationPtr = (Version_1_Nation*) mem_add(sizeof(Version_1_Nation));
	visit_version_1_nation_members(v, oldNationPtr);
	if (v->good())
	{
		oldNationPtr->convert_to_version_2(n);
	}
	mem_del(oldNationPtr);
}

template <typename Visitor>
static void visit_version_1_nation_members(Visitor *v, Version_1_Nation *v1n)
{
	v->skip(4); /* virtual table pointer */

	/* NationBase */
	visit<int16_t>(v, &v1n->nation_recno);
	visit<int8_t>(v, &v1n->nation_type);
	visit<int8_t>(v, &v1n->race_id);
	visit<int8_t>(v, &v1n->color_scheme_id);
	visit<int8_t>(v, &v1n->nation_color);
	visit<int16_t>(v, &v1n->king_unit_recno);
	visit<int8_t>(v, &v1n->king_leadership);
	visit<int32_t>(v, &v1n->nation_name_id);

	visit_array<int8_t>(v, v1n->nation_name_str);

	visit<uint32_t>(v, &v1n->player_id);
	visit<int8_t>(v, &v1n->next_frame_ready);
	visit<int16_t>(v, &v1n->last_caravan_id);
	visit<int16_t>(v, &v1n->nation_firm_count);
	visit<int32_t>(v, &v1n->last_build_firm_date);
	visit_array<int8_t>(v, v1n->know_base_array);
	visit_array<int8_t>(v, v1n->base_count_array);
	visit<int8_t>(v, &v1n->is_at_war_today);
	visit<int8_t>(v, &v1n->is_at_war_yesterday);
	visit<int32_t>(v, &v1n->last_war_date);
	visit<int16_t>(v, &v1n->last_attacker_unit_recno);
	visit<int32_t>(v, &v1n->last_independent_unit_join_date);
	visit<int8_t>(v, &v1n->cheat_enabled_flag);
	visit<float>(v, &v1n->cash);
	visit<float>(v, &v1n->food);
	visit<float>(v, &v1n->reputation);
	visit<float>(v, &v1n->kill_monster_score);
	visit<int16_t>(v, &v1n->auto_collect_tax_loyalty);
	visit<int16_t>(v, &v1n->auto_grant_loyalty);
	visit<float>(v, &v1n->cur_year_profit);
	visit<float>(v, &v1n->last_year_profit);
	visit<float>(v, &v1n->cur_year_fixed_income);
	visit<float>(v, &v1n->last_year_fixed_income);
	visit<float>(v, &v1n->cur_year_fixed_expense);
	visit<float>(v, &v1n->last_year_fixed_expense);
	visit_array<float>(v, v1n->cur_year_income_array);
	visit_array<float>(v, v1n->last_year_income_array);
	visit<float>(v, &v1n->cur_year_income);
	visit<float>(v, &v1n->last_year_income);
	visit_array<float>(v, v1n->cur_year_expense_array);
	visit_array<float>(v, v1n->last_year_expense_array);
	visit<float>(v, &v1n->cur_year_expense);
	visit<float>(v, &v1n->last_year_expense);
	visit<float>(v, &v1n->cur_year_cheat);
	visit<float>(v, &v1n->last_year_cheat);
	visit<float>(v, &v1n->cur_year_food_in);
	visit<float>(v, &v1n->last_year_food_in);
	visit<float>(v, &v1n->cur_year_food_out);
	visit<float>(v, &v1n->last_year_food_out);
	visit<float>(v, &v1n->cur_year_food_change);
	visit<float>(v, &v1n->last_year_food_change);
	visit<float>(v, &v1n->cur_year_reputation_change);
	visit<float>(v, &v1n->last_year_reputation_change);

	for (int n = 0; n < MAX_NATION; n++)
		visit_nation_relation(v, &v1n->relation_array[n]);

	visit_array<int8_t>(v, v1n->relation_status_array);
	visit_array<int8_t>(v, v1n->relation_passable_array);

	visit_array<int8_t>(v, v1n->relation_should_attack_array);
	visit<int8_t>(v, &v1n->is_allied_with_player);
	visit<int32_t>(v, &v1n->total_population);
	visit<int32_t>(v, &v1n->total_jobless_population);
	visit<int32_t>(v, &v1n->total_unit_count);
	visit<int32_t>(v, &v1n->total_human_count);
	visit<int32_t>(v, &v1n->total_general_count);
	visit<int32_t>(v, &v1n->total_weapon_count);
	visit<int32_t>(v, &v1n->total_ship_count);
	visit<int32_t>(v, &v1n->total_firm_count);
	visit<int32_t>(v, &v1n->total_spy_count);
	visit<int32_t>(v, &v1n->total_ship_combat_level);
	visit<int16_t>(v, &v1n->largest_town_recno);
	visit<int16_t>(v, &v1n->largest_town_pop);
	visit_array<int16_t>(v, v1n->raw_count_array);
	visit_array<uint16_t>(v, v1n->last_unit_name_id_array);
	visit<int32_t>(v, &v1n->population_rating);
	visit<int32_t>(v, &v1n->military_rating);
	visit<int32_t>(v, &v1n->economic_rating);
	visit<int32_t>(v, &v1n->overall_rating);
	visit<int32_t>(v, &v1n->enemy_soldier_killed);
	visit<int32_t>(v, &v1n->own_soldier_killed);
	visit<int32_t>(v, &v1n->enemy_civilian_killed);
	visit<int32_t>(v, &v1n->own_civilian_killed);
	visit<int32_t>(v, &v1n->enemy_weapon_destroyed);
	visit<int32_t>(v, &v1n->own_weapon_destroyed);
	visit<int32_t>(v, &v1n->enemy_ship_destroyed);
	visit<int32_t>(v, &v1n->own_ship_destroyed);
	visit<int32_t>(v, &v1n->enemy_firm_destroyed);
	visit<int32_t>(v, &v1n->own_firm_destroyed);

	/* Nation */
	v->skip(29); /* action_array */

	visit<uint16_t>(v, &v1n->last_action_id);
	visit_pointer(v, &v1n->ai_town_array);
	visit_pointer(v, &v1n->ai_base_array);
	visit_pointer(v, &v1n->ai_mine_array);
	visit_pointer(v, &v1n->ai_factory_array);
	visit_pointer(v, &v1n->ai_camp_array);
	visit_pointer(v, &v1n->ai_research_array);
	visit_pointer(v, &v1n->ai_war_array);
	visit_pointer(v, &v1n->ai_harbor_array);
	visit_pointer(v, &v1n->ai_market_array);
	visit_pointer(v, &v1n->ai_inn_array);
	visit_pointer(v, &v1n->ai_general_array);
	visit_pointer(v, &v1n->ai_caravan_array);
	visit_pointer(v, &v1n->ai_ship_array);
	visit<int16_t>(v, &v1n->ai_town_size);
	visit<int16_t>(v, &v1n->ai_base_size);
	visit<int16_t>(v, &v1n->ai_mine_size);
	visit<int16_t>(v, &v1n->ai_factory_size);
	visit<int16_t>(v, &v1n->ai_camp_size);
	visit<int16_t>(v, &v1n->ai_research_size);
	visit<int16_t>(v, &v1n->ai_war_size);
	visit<int16_t>(v, &v1n->ai_harbor_size);
	visit<int16_t>(v, &v1n->ai_market_size);
	visit<int16_t>(v, &v1n->ai_inn_size);
	visit<int16_t>(v, &v1n->ai_general_size);
	visit<int16_t>(v, &v1n->ai_caravan_size);
	visit<int16_t>(v, &v1n->ai_ship_size);
	visit<int16_t>(v, &v1n->ai_town_count);
	visit<int16_t>(v, &v1n->ai_base_count);
	visit<int16_t>(v, &v1n->ai_mine_count);
	visit<int16_t>(v, &v1n->ai_factory_count);
	visit<int16_t>(v, &v1n->ai_camp_count);
	visit<int16_t>(v, &v1n->ai_research_count);
	visit<int16_t>(v, &v1n->ai_war_count);
	visit<int16_t>(v, &v1n->ai_harbor_count);
	visit<int16_t>(v, &v1n->ai_market_count);
	visit<int16_t>(v, &v1n->ai_inn_count);
	visit<int16_t>(v, &v1n->ai_general_count);
	visit<int16_t>(v, &v1n->ai_caravan_count);
	visit<int16_t>(v, &v1n->ai_ship_count);
	visit<int16_t>(v, &v1n->ai_base_town_count);
	visit_array<int16_t>(v, v1n->firm_should_close_array);

	for (int n = 0; n < MAX_AI_REGION; n++)
		visit_ai_region(v, &v1n->ai_region_array[n]);

	visit<int8_t>(v, &v1n->ai_region_count);
	visit<int8_t>(v, &v1n->pref_force_projection);
	visit<int8_t>(v, &v1n->pref_military_development);
	visit<int8_t>(v, &v1n->pref_economic_development);
	visit<int8_t>(v, &v1n->pref_inc_pop_by_capture);
	visit<int8_t>(v, &v1n->pref_inc_pop_by_growth);
	visit<int8_t>(v, &v1n->pref_peacefulness);
	visit<int8_t>(v, &v1n->pref_military_courage);
	visit<int8_t>(v, &v1n->pref_territorial_cohesiveness);
	visit<int8_t>(v, &v1n->pref_trading_tendency);
	visit<int8_t>(v, &v1n->pref_allying_tendency);
	visit<int8_t>(v, &v1n->pref_honesty);
	visit<int8_t>(v, &v1n->pref_town_harmony);
	visit<int8_t>(v, &v1n->pref_loyalty_concern);
	visit<int8_t>(v, &v1n->pref_forgiveness);
	visit<int8_t>(v, &v1n->pref_collect_tax);
	visit<int8_t>(v, &v1n->pref_hire_unit);
	visit<int8_t>(v, &v1n->pref_use_weapon);
	visit<int8_t>(v, &v1n->pref_keep_general);
	visit<int8_t>(v, &v1n->pref_keep_skilled_unit);
	visit<int8_t>(v, &v1n->pref_diplomacy_retry);
	visit<int8_t>(v, &v1n->pref_attack_monster);
	visit<int8_t>(v, &v1n->pref_spy);
	visit<int8_t>(v, &v1n->pref_counter_spy);
	visit<int8_t>(v, &v1n->pref_food_reserve);
	visit<int8_t>(v, &v1n->pref_cash_reserve);
	visit<int8_t>(v, &v1n->pref_use_marine);
	visit<int8_t>(v, &v1n->pref_unit_chase_distance);
	visit<int8_t>(v, &v1n->pref_repair_concern);
	visit<int8_t>(v, &v1n->pref_scout);
	visit<int16_t>(v, &v1n->ai_capture_enemy_town_recno);
	visit<int32_t>(v, &v1n->ai_capture_enemy_town_plan_date);
	visit<int32_t>(v, &v1n->ai_capture_enemy_town_start_attack_date);
	visit<int8_t>(v, &v1n->ai_capture_enemy_town_use_all_camp);
	visit<int32_t>(v, &v1n->ai_last_defend_action_date);
	visit<int16_t>(v, &v1n->ai_attack_target_x_loc);
	visit<int16_t>(v, &v1n->ai_attack_target_y_loc);
	visit<int16_t>(v, &v1n->ai_attack_target_nation_recno);

	for (int n = 0; n < MAX_SUITABLE_ATTACK_CAMP; n++)
		visit_attack_camp(v, &v1n->attack_camp_array[n]);

	visit<int16_t>(v, &v1n->attack_camp_count);
	visit<int16_t>(v, &v1n->lead_attack_camp_recno);
}

template <typename Visitor>
static void visit_version_1_nation_array(Visitor *v, Version_1_NationArray *na)
{
	visit<int16_t>(v, &na->nation_count);
	visit<int16_t>(v, &na->ai_nation_count);
	visit<int32_t>(v, &na->last_del_nation_date);
	visit<int32_t>(v, &na->last_new_nation_date);
	visit<int32_t>(v, &na->max_nation_population);
	visit<int32_t>(v, &na->all_nation_population);
	visit<int16_t>(v, &na->independent_town_count);
	visit_array<int16_t>(v, na->independent_town_count_race_array);
	visit<int32_t>(v, &na->max_nation_units);
	visit<int32_t>(v, &na->max_nation_humans);
	visit<int32_t>(v, &na->max_nation_generals);
	visit<int32_t>(v, &na->max_nation_weapons);
	visit<int32_t>(v, &na->max_nation_ships);
	visit<int32_t>(v, &na->max_nation_spies);
	visit<int32_t>(v, &na->max_nation_firms);
	visit<int32_t>(v, &na->max_nation_tech_level);
	visit<int32_t>(v, &na->max_population_rating);
	visit<int32_t>(v, &na->max_military_rating);
	visit<int32_t>(v, &na->max_economic_rating);
	visit<int32_t>(v, &na->max_reputation);
	visit<int32_t>(v, &na->max_kill_monster_score);
	visit<int32_t>(v, &na->max_overall_rating);
	visit<int16_t>(v, &na->max_population_nation_recno);
	visit<int16_t>(v, &na->max_military_nation_recno);
	visit<int16_t>(v, &na->max_economic_nation_recno);
	visit<int16_t>(v, &na->max_reputation_nation_recno);
	visit<int16_t>(v, &na->max_kill_monster_nation_recno);
	visit<int16_t>(v, &na->max_overall_nation_recno);
	visit<int32_t>(v, &na->last_alliance_id);
	visit<int32_t>(v, &na->nation_peace_days);
	visit<int16_t>(v, &na->player_recno);
	visit_pointer(v, &na->player_ptr);
	visit_array<int8_t>(v, na->nation_color_array);
	visit_array<int8_t>(v, na->nation_power_color_array);

	for (int n = 0; n < MAX_NATION; n++)
		visit_array<int8_t>(v, na->human_name_array[n]);
}

