// Filename    : OGF_V1.CPP
// Description : Old Class structures for save game conversion

#ifdef AMPLUS

#include <ogf_v1.h>
#include <OTOWN.H>
#include <ONATIONA.H>
#include <ONATION.H>

//---------------- begin of function Version_1_Town::convert_to_version_2() -----------------//
void Version_1_Town::convert_to_version_2(Town *townPtr)
{
	townPtr->town_recno = town_recno;
	townPtr->town_name_id = town_name_id;
	townPtr->nation_recno = nation_recno;
	townPtr->rebel_recno = rebel_recno;
	townPtr->race_id = race_id;
	townPtr->setup_date = setup_date;
	townPtr->ai_town = ai_town;
	townPtr->ai_link_checked = ai_link_checked;
	townPtr->independ_town_nation_relation = independ_town_nation_relation;
	townPtr->has_linked_own_camp = has_linked_own_camp;
	townPtr->has_linked_enemy_camp = has_linked_enemy_camp;
	townPtr->is_base_town = is_base_town;
	townPtr->loc_x1 = loc_x1;
	townPtr->loc_y1 = loc_y1;
	townPtr->loc_x2 = loc_x2;
	townPtr->loc_y2 = loc_y2;
	townPtr->abs_x1 = abs_x1;
	townPtr->abs_y1 = abs_y1;
	townPtr->abs_x2 = abs_x2;
	townPtr->abs_y2 = abs_y2;
	townPtr->center_x = center_x;
	townPtr->center_y = center_y;
	townPtr->region_id = region_id;
	townPtr->layout_id = layout_id;
	townPtr->first_slot_id = first_slot_id;

	memcpy(townPtr->slot_object_id_array, slot_object_id_array, sizeof(slot_object_id_array));

	townPtr->population = population;
	townPtr->jobless_population = jobless_population;
			

	memset(townPtr->max_race_pop_array, 0, sizeof(townPtr->max_race_pop_array));
	memcpy(townPtr->max_race_pop_array, max_race_pop_array, sizeof(max_race_pop_array[0])*VERSION_1_MAX_RACE);
	memset(townPtr->race_pop_array, 0, sizeof(townPtr->race_pop_array));
	memcpy(townPtr->race_pop_array, race_pop_array, sizeof(race_pop_array[0])*VERSION_1_MAX_RACE);
	memset(townPtr->race_pop_growth_array, 0, sizeof(townPtr->race_pop_growth_array));
	memcpy(townPtr->race_pop_growth_array, race_pop_growth_array, sizeof(race_pop_growth_array[0])*VERSION_1_MAX_RACE);
	memset(townPtr->jobless_race_pop_array, 0, sizeof(townPtr->jobless_race_pop_array));
	memcpy(townPtr->jobless_race_pop_array, jobless_race_pop_array, sizeof(jobless_race_pop_array[0])*VERSION_1_MAX_RACE);
	memset(townPtr->race_loyalty_array, 0, sizeof(townPtr->race_loyalty_array));
	memcpy(townPtr->race_loyalty_array, race_loyalty_array, sizeof(race_loyalty_array[0])*VERSION_1_MAX_RACE);
	memset(townPtr->race_target_loyalty_array, 0, sizeof(townPtr->race_target_loyalty_array));
	memcpy(townPtr->race_target_loyalty_array, race_target_loyalty_array, sizeof(race_target_loyalty_array[0])*VERSION_1_MAX_RACE);
	memset(townPtr->race_spy_count_array, 0, sizeof(townPtr->race_spy_count_array));
	memcpy(townPtr->race_spy_count_array, race_spy_count_array, sizeof(race_spy_count_array[0])*VERSION_1_MAX_RACE);

	memset(townPtr->race_resistance_array, 0, sizeof(townPtr->race_resistance_array));
	memcpy(townPtr->race_resistance_array, race_resistance_array, sizeof(race_resistance_array[0][0])*VERSION_1_MAX_RACE*MAX_NATION);
	memset(townPtr->race_target_resistance_array, 0, sizeof(townPtr->race_target_resistance_array));
	memcpy(townPtr->race_target_resistance_array, race_target_resistance_array, sizeof(race_target_resistance_array[0][0])*VERSION_1_MAX_RACE*MAX_NATION);
	
	townPtr->town_defender_count = town_defender_count;
	townPtr->last_being_attacked_date = last_being_attacked_date;
	townPtr->received_hit_count = received_hit_count;
	
	memcpy(townPtr->train_queue_skill_array, train_queue_skill_array, sizeof(train_queue_skill_array));
	memcpy(townPtr->train_queue_race_array, train_queue_race_array, sizeof(train_queue_race_array));

	townPtr->train_queue_count = train_queue_count;
	townPtr->train_unit_recno = train_unit_recno;
	townPtr->train_unit_action_id = train_unit_action_id;
	townPtr->start_train_frame_no = start_train_frame_no;
	townPtr->defend_target_recno = defend_target_recno;
	townPtr->accumulated_collect_tax_penalty = accumulated_collect_tax_penalty;
	townPtr->accumulated_reward_penalty = accumulated_reward_penalty;
	townPtr->accumulated_recruit_penalty = accumulated_recruit_penalty;
	townPtr->accumulated_enemy_grant_penalty = accumulated_enemy_grant_penalty;
	townPtr->last_rebel_date = last_rebel_date;
	townPtr->independent_unit_join_nation_min_rating = independent_unit_join_nation_min_rating;
	townPtr->quality_of_life = quality_of_life;
	townPtr->auto_collect_tax_loyalty = auto_collect_tax_loyalty;
	townPtr->auto_grant_loyalty = auto_grant_loyalty;
	townPtr->town_combat_level = town_combat_level;
	
	memcpy(townPtr->has_product_supply, has_product_supply, sizeof(has_product_supply));

	townPtr->no_neighbor_space = no_neighbor_space;
	townPtr->linked_firm_count =  linked_firm_count;
	townPtr->linked_town_count =  linked_town_count;

	memcpy(townPtr->linked_firm_array, linked_firm_array, sizeof(linked_firm_array));
	memcpy(townPtr->linked_town_array, linked_town_array, sizeof(linked_town_array));
	memcpy(townPtr->linked_firm_enable_array, linked_firm_enable_array, sizeof(linked_firm_enable_array));
	memcpy(townPtr->linked_town_enable_array, linked_town_enable_array, sizeof(linked_town_enable_array));

}
//---------------- end of function Version_1_Town::convert_to_version_2() -----------------//


//---------------- begin of function Version_1_NationArray::convert_to_version_2() -----------------//
void Version_1_NationArray::convert_to_version_2(NationArray *nationArrayPtr)
{
	nationArrayPtr->nation_count = nation_count;
	nationArrayPtr->ai_nation_count = ai_nation_count;
	nationArrayPtr->last_del_nation_date = last_del_nation_date;
	nationArrayPtr->last_new_nation_date = last_new_nation_date;
	nationArrayPtr->max_nation_population = max_nation_population;
	nationArrayPtr->all_nation_population = all_nation_population;
	nationArrayPtr->independent_town_count = independent_town_count;

	memset(nationArrayPtr->independent_town_count_race_array, 0, sizeof(nationArrayPtr->independent_town_count_race_array));
	memcpy(nationArrayPtr->independent_town_count_race_array, independent_town_count_race_array, sizeof(independent_town_count_race_array));
	
	nationArrayPtr->max_nation_units = max_nation_units;
	nationArrayPtr->max_nation_humans = max_nation_humans;
	nationArrayPtr->max_nation_generals = max_nation_generals;
	nationArrayPtr->max_nation_weapons = max_nation_weapons;
	nationArrayPtr->max_nation_ships = max_nation_ships;
	nationArrayPtr->max_nation_spies = max_nation_spies;
	nationArrayPtr->max_nation_firms = max_nation_firms;
	nationArrayPtr->max_nation_tech_level = max_nation_tech_level;
	nationArrayPtr->max_population_rating = max_population_rating;
	nationArrayPtr->max_military_rating = max_military_rating;
	nationArrayPtr->max_economic_rating = max_economic_rating;
	nationArrayPtr->max_reputation = max_reputation;
	nationArrayPtr->max_kill_monster_score = max_kill_monster_score;
	nationArrayPtr->max_overall_rating = max_overall_rating;
	nationArrayPtr->max_population_nation_recno = max_population_nation_recno;
	nationArrayPtr->max_military_nation_recno = max_military_nation_recno;
	nationArrayPtr->max_economic_nation_recno = max_economic_nation_recno;
	nationArrayPtr->max_reputation_nation_recno = max_reputation_nation_recno;
	nationArrayPtr->max_kill_monster_nation_recno = max_kill_monster_nation_recno;
	nationArrayPtr->max_overall_nation_recno = max_overall_nation_recno;
	nationArrayPtr->last_alliance_id = last_alliance_id;
	nationArrayPtr->nation_peace_days = nation_peace_days;
	nationArrayPtr->player_recno = player_recno;
	nationArrayPtr->player_ptr = player_ptr;

	memcpy(nationArrayPtr->nation_color_array, nation_color_array, sizeof(nation_color_array[0])*(MAX_NATION+1));
	memcpy(nationArrayPtr->nation_power_color_array, nation_power_color_array, sizeof(nation_power_color_array[0])*(MAX_NATION+2));
	memcpy(nationArrayPtr->human_name_array, human_name_array, sizeof(human_name_array[0])*(MAX_NATION));
}
//---------------- end of function Version_1_NationArray::convert_to_version_2() -----------------//

//---------------- begin of function Version_1_NationBase::convert_to_version_2() -----------------//
void Version_1_NationBase::convert_to_version_2(Nation *nationPtr)
{
	nationPtr->nation_recno = nation_recno;
	nationPtr->nation_type = nation_type;
	nationPtr->race_id = race_id;
	nationPtr->color_scheme_id = color_scheme_id;
	nationPtr->nation_color = nation_color;
	nationPtr->king_unit_recno = king_unit_recno;
	nationPtr->king_leadership = king_leadership;
	nationPtr->nation_name_id = nation_name_id;

	memcpy(nationPtr->nation_name_str, nation_name_str, sizeof(nation_name_str));

	nationPtr->player_id = player_id;
	nationPtr->next_frame_ready = next_frame_ready;
	nationPtr->last_caravan_id = last_caravan_id;
	nationPtr->nation_firm_count = nation_firm_count;
	nationPtr->last_build_firm_date = last_build_firm_date;

	memset(nationPtr->know_base_array, 0, sizeof(nationPtr->know_base_array));
	memcpy(nationPtr->know_base_array, know_base_array, sizeof(know_base_array));
	memset(nationPtr->base_count_array, 0, sizeof(nationPtr->base_count_array));
	memcpy(nationPtr->base_count_array, base_count_array, sizeof(base_count_array));

	nationPtr->is_at_war_today = is_at_war_today;
	nationPtr->is_at_war_yesterday = is_at_war_yesterday;
	nationPtr->last_war_date = last_war_date;
	nationPtr->last_attacker_unit_recno = last_attacker_unit_recno;
	nationPtr->last_independent_unit_join_date = last_independent_unit_join_date;
	nationPtr->cheat_enabled_flag = cheat_enabled_flag;
	//----------------------------------//
	nationPtr->cash = cash;
	nationPtr->food = food;
	nationPtr->reputation = reputation;
	nationPtr->kill_monster_score = kill_monster_score;
	//------- town auto policy -------------//
	nationPtr->auto_collect_tax_loyalty = auto_collect_tax_loyalty;
	nationPtr->auto_grant_loyalty = auto_grant_loyalty;

	//----- yearly income, expense and profit ------//
	nationPtr->cur_year_profit = cur_year_profit;
	nationPtr->last_year_profit = last_year_profit;
	nationPtr->cur_year_fixed_income = cur_year_fixed_income;
	nationPtr->last_year_fixed_income = last_year_fixed_income;
	nationPtr->cur_year_fixed_expense = cur_year_fixed_expense;
	nationPtr->last_year_fixed_expense = last_year_fixed_expense;
	//------- yearly income ------//
	memcpy(nationPtr->cur_year_income_array, cur_year_income_array, sizeof(cur_year_income_array));
	memcpy(nationPtr->last_year_income_array, last_year_income_array, sizeof(last_year_income_array));

	nationPtr->cur_year_income = cur_year_income;
	nationPtr->last_year_income = last_year_income;

	//------- yearly expense ------//
	memcpy(nationPtr->cur_year_expense_array, cur_year_expense_array, sizeof(cur_year_expense_array));
	memcpy(nationPtr->last_year_expense_array, last_year_expense_array, sizeof(last_year_expense_array));

	nationPtr->cur_year_expense = cur_year_expense;
	nationPtr->last_year_expense = last_year_expense;
	//------- yearly expense ------//
	nationPtr->cur_year_cheat = cur_year_cheat;
	nationPtr->last_year_cheat = last_year_cheat;

	//----- yearly food in, out and change ------//
	nationPtr->cur_year_food_in = cur_year_food_in;
	nationPtr->last_year_food_in = last_year_food_in;

	nationPtr->cur_year_food_out = cur_year_food_out;
	nationPtr->last_year_food_out = last_year_food_out;

	nationPtr->cur_year_food_change = cur_year_food_change;
	nationPtr->last_year_food_change = last_year_food_change;
	//----- yearly reputatino change ------//
	nationPtr->cur_year_reputation_change = cur_year_reputation_change;
	nationPtr->last_year_reputation_change = last_year_reputation_change;

	//--------- er-nation relationship -----------//
	memcpy(nationPtr->relation_array, relation_array, sizeof(relation_array));
	memcpy(nationPtr->relation_status_array, relation_status_array, sizeof(relation_status_array));
	memcpy(nationPtr->relation_passable_array, relation_passable_array, sizeof(relation_passable_array));
	memcpy(nationPtr->relation_should_attack_array, relation_should_attack_array, sizeof(relation_should_attack_array));

	nationPtr->is_allied_with_player = is_allied_with_player;
	//---------- statistic ------------//
	nationPtr->total_population = total_population;
	nationPtr->total_jobless_population = total_jobless_population;

	nationPtr->total_unit_count = total_unit_count;
	nationPtr->total_human_count = total_human_count;
	nationPtr->total_general_count = total_general_count;
	nationPtr->total_weapon_count = total_weapon_count;
	nationPtr->total_ship_count = total_ship_count;
	nationPtr->total_firm_count = total_firm_count;
	nationPtr->total_spy_count = total_spy_count;
	nationPtr->total_ship_combat_level = total_ship_combat_level;

	nationPtr->largest_town_recno = largest_town_recno;
	nationPtr->largest_town_pop = largest_town_pop;

	memcpy(nationPtr->raw_count_array, raw_count_array, sizeof(raw_count_array));
	memset(nationPtr->last_unit_name_id_array, 0, sizeof(nationPtr->last_unit_name_id_array));
	memcpy(nationPtr->last_unit_name_id_array, last_unit_name_id_array, sizeof(last_unit_name_id_array));

	//--------- rank ratings ---------//
	nationPtr->population_rating = population_rating;
	nationPtr->military_rating = military_rating;
	nationPtr->economic_rating = economic_rating;
	nationPtr->overall_rating = overall_rating;

	//------ additional statistic ------//
	nationPtr->enemy_soldier_killed = enemy_soldier_killed;
	nationPtr->own_soldier_killed = own_soldier_killed;
	nationPtr->enemy_civilian_killed = enemy_civilian_killed;
	nationPtr->own_civilian_killed = own_civilian_killed;
	nationPtr->enemy_weapon_destroyed = enemy_weapon_destroyed;
	nationPtr->own_weapon_destroyed = own_weapon_destroyed;
	nationPtr->enemy_ship_destroyed = enemy_ship_destroyed;
	nationPtr->own_ship_destroyed = own_ship_destroyed;
	nationPtr->enemy_firm_destroyed = enemy_firm_destroyed;
	nationPtr->own_firm_destroyed = own_firm_destroyed;
}
//---------------- end of function Version_1_NationBase::convert_to_version_2() -----------------//

//---------------- begin of function Version_1_Nation::convert_to_version_2() -----------------//
void Version_1_Nation::convert_to_version_2(Nation *nationPtr)
{
	Version_1_NationBase::convert_to_version_2(nationPtr);

	nationPtr->action_array = action_array;
	nationPtr->last_action_id = last_action_id;
	nationPtr->ai_town_size = ai_town_size;
	nationPtr->ai_base_size = ai_base_size;
	nationPtr->ai_mine_size = ai_mine_size;
	nationPtr->ai_factory_size = ai_factory_size;
	nationPtr->ai_camp_size = ai_camp_size;
	nationPtr->ai_research_size = ai_research_size;
	nationPtr->ai_war_size = ai_war_size;
	nationPtr->ai_harbor_size = ai_harbor_size;
	nationPtr->ai_market_size = ai_market_size;
	nationPtr->ai_inn_size = ai_inn_size;
	nationPtr->ai_general_size = ai_general_size;
	nationPtr->ai_caravan_size = ai_caravan_size;
	nationPtr->ai_ship_size = ai_ship_size;
	nationPtr->ai_town_count = ai_town_count;
	nationPtr->ai_base_count = ai_base_count;
	nationPtr->ai_mine_count = ai_mine_count;
	nationPtr->ai_factory_count = ai_factory_count;
	nationPtr->ai_camp_count = ai_camp_count;
	nationPtr->ai_research_count = ai_research_count;
	nationPtr->ai_war_count = ai_war_count;
	nationPtr->ai_harbor_count = ai_harbor_count;
	nationPtr->ai_market_count = ai_market_count;
	nationPtr->ai_inn_count = ai_inn_count;
	nationPtr->ai_general_count = ai_general_count;
	nationPtr->ai_caravan_count = ai_caravan_count;
	nationPtr->ai_ship_count = ai_ship_count;
	nationPtr->ai_base_town_count = ai_base_town_count;

	memcpy(nationPtr->firm_should_close_array, firm_should_close_array, sizeof(firm_should_close_array));

	//------------------------------------------------------//
	// parameters about the nation itself
	//------------------------------------------------------//
	memcpy(nationPtr->ai_region_array, ai_region_array, sizeof(ai_region_array));

	nationPtr->ai_region_count = ai_region_count;

	//------------------------------------------------------//
	// AI personalties
	//------------------------------------------------------//
	nationPtr->pref_force_projection = pref_force_projection;
	nationPtr->pref_military_development = pref_military_development;
	nationPtr->pref_economic_development = pref_economic_development;
	nationPtr->pref_inc_pop_by_capture = pref_inc_pop_by_capture;
	nationPtr->pref_inc_pop_by_growth = pref_inc_pop_by_growth;
	nationPtr->pref_peacefulness = pref_peacefulness;
	nationPtr->pref_military_courage = pref_military_courage;
	nationPtr->pref_territorial_cohesiveness = pref_territorial_cohesiveness;
	nationPtr->pref_trading_tendency = pref_trading_tendency;
	nationPtr->pref_allying_tendency = pref_allying_tendency;
	nationPtr->pref_honesty = pref_honesty;
	nationPtr->pref_town_harmony = pref_town_harmony;
	nationPtr->pref_loyalty_concern = pref_loyalty_concern;
	nationPtr->pref_forgiveness = pref_forgiveness;
	nationPtr->pref_collect_tax = pref_collect_tax;
	nationPtr->pref_hire_unit = pref_hire_unit;
	nationPtr->pref_use_weapon = pref_use_weapon;
	nationPtr->pref_keep_general = pref_keep_general;
	nationPtr->pref_keep_skilled_unit = pref_keep_skilled_unit;
	nationPtr->pref_diplomacy_retry = pref_diplomacy_retry;
	nationPtr->pref_attack_monster = pref_attack_monster;
	nationPtr->pref_spy = pref_spy;
	nationPtr->pref_counter_spy = pref_counter_spy;
	nationPtr->pref_food_reserve = pref_food_reserve;
	nationPtr->pref_cash_reserve = pref_cash_reserve;
	nationPtr->pref_use_marine = pref_use_marine;
	nationPtr->pref_unit_chase_distance = pref_unit_chase_distance;
	nationPtr->pref_repair_concern = pref_repair_concern;
	nationPtr->pref_scout = pref_scout;

	//------- AI action vars --------//
	nationPtr->ai_capture_enemy_town_recno = ai_capture_enemy_town_recno;
	nationPtr->ai_capture_enemy_town_plan_date = ai_capture_enemy_town_plan_date;
	nationPtr->ai_capture_enemy_town_start_attack_date = ai_capture_enemy_town_start_attack_date;
	nationPtr->ai_capture_enemy_town_use_all_camp = ai_capture_enemy_town_use_all_camp;

	nationPtr->ai_last_defend_action_date = ai_last_defend_action_date;

	nationPtr->ai_attack_target_x_loc = ai_attack_target_x_loc;
	nationPtr->ai_attack_target_y_loc = ai_attack_target_y_loc;
	nationPtr->ai_attack_target_nation_recno = ai_attack_target_nation_recno;

	memcpy(nationPtr->attack_camp_array, attack_camp_array, sizeof(attack_camp_array));

	nationPtr->attack_camp_count = attack_camp_count;
	nationPtr->lead_attack_camp_recno = lead_attack_camp_recno;
}
//---------------- end of function Version_1_Nation::convert_to_version_2() -----------------//


#endif