/*
* Seven Kingdoms: Ancient Adversaries
*
* Copyright 1997,1998 Enlight Software Ltd.
* Copyright 2010 Unavowed <unavowed@vexillium.org>
* Copyright 2018 Richard Dijk <microvirus.multiplying@gmail.com>
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

#include <OTOWN.h>
#include <OGFILE.h>
#include <OGF_V1.h>
#include <file_io_visitor.h>
#include <visit_sprite.h>
#include <OGFILE_DYNARRAYB.inl>

using namespace FileIOVisitor;


template <typename Visitor>
static void visit_version_1_town_members(Visitor* v, Version_1_Town* c)
{
	visit<int16_t>(v, &c->town_recno);
	visit<int16_t>(v, &c->town_name_id);
	visit<int16_t>(v, &c->nation_recno);
	visit<int16_t>(v, &c->rebel_recno);
	visit<int8_t>(v, &c->race_id);
	visit<int32_t>(v, &c->setup_date);
	visit<int8_t>(v, &c->ai_town);
	visit<int8_t>(v, &c->ai_link_checked);
	visit<int8_t>(v, &c->independ_town_nation_relation);
	visit<int8_t>(v, &c->has_linked_own_camp);
	visit<int8_t>(v, &c->has_linked_enemy_camp);
	visit<int8_t>(v, &c->is_base_town);
	visit<int16_t>(v, &c->loc_x1);
	visit<int16_t>(v, &c->loc_y1);
	visit<int16_t>(v, &c->loc_x2);
	visit<int16_t>(v, &c->loc_y2);
	visit<int16_t>(v, &c->abs_x1);
	visit<int16_t>(v, &c->abs_y1);
	visit<int16_t>(v, &c->abs_x2);
	visit<int16_t>(v, &c->abs_y2);
	visit<int16_t>(v, &c->center_x);
	visit<int16_t>(v, &c->center_y);
	visit<uint8_t>(v, &c->region_id);
	visit<int16_t>(v, &c->layout_id);
	visit<int16_t>(v, &c->first_slot_id);
	visit_array<int16_t>(v, c->slot_object_id_array);
	visit<int16_t>(v, &c->population);
	visit<int16_t>(v, &c->jobless_population);
	visit_array<int16_t>(v, c->max_race_pop_array);
	visit_array<int16_t>(v, c->race_pop_array);
	visit_array<uint8_t>(v, c->race_pop_growth_array);
	visit_array<int16_t>(v, c->jobless_race_pop_array);
	visit_array<float>(v, c->race_loyalty_array);
	visit_array<int8_t>(v, c->race_target_loyalty_array);
	visit_array<int16_t>(v, c->race_spy_count_array);
	for (int i = 0; i < VERSION_1_MAX_RACE; ++i) {
		visit_array<float>(v, c->race_resistance_array[i]);
	}
	for (int i = 0; i < VERSION_1_MAX_RACE; ++i) {
		visit_array<int8_t>(v, c->race_target_resistance_array[i]);
	}
	visit<int16_t>(v, &c->town_defender_count);
	visit<int32_t>(v, &c->last_being_attacked_date);
	visit<float>(v, &c->received_hit_count);
	visit_array<int8_t>(v, c->train_queue_skill_array);
	visit_array<int8_t>(v, c->train_queue_race_array);
	visit<int8_t>(v, &c->train_queue_count);
	visit<int16_t>(v, &c->train_unit_recno);
	visit<int32_t>(v, &c->train_unit_action_id);
	visit<uint32_t>(v, &c->start_train_frame_no);
	visit<int16_t>(v, &c->defend_target_recno);
	visit<int32_t>(v, &c->accumulated_collect_tax_penalty);
	visit<int32_t>(v, &c->accumulated_reward_penalty);
	visit<int32_t>(v, &c->accumulated_recruit_penalty);
	visit<int32_t>(v, &c->accumulated_enemy_grant_penalty);
	visit<int32_t>(v, &c->last_rebel_date);
	visit<int16_t>(v, &c->independent_unit_join_nation_min_rating);
	visit<int16_t>(v, &c->quality_of_life);
	visit<int16_t>(v, &c->auto_collect_tax_loyalty);
	visit<int16_t>(v, &c->auto_grant_loyalty);
	visit<int8_t>(v, &c->town_combat_level);
	visit_array<int8_t>(v, c->has_product_supply);
	visit<int8_t>(v, &c->no_neighbor_space);
	visit<int16_t>(v, &c->linked_firm_count);
	visit<int16_t>(v, &c->linked_town_count);
	visit_array<int16_t>(v, c->linked_firm_array);
	visit_array<int16_t>(v, c->linked_town_array);
	visit_array<int8_t>(v, c->linked_firm_enable_array);
	visit_array<int8_t>(v, c->linked_town_enable_array);
}

template <typename Visitor>
static void visit_town(Visitor* v, Town* c)
{
	visit<int16_t>(v, &c->town_recno);
	visit<int16_t>(v, &c->town_name_id);
	visit<int16_t>(v, &c->nation_recno);
	visit<int16_t>(v, &c->rebel_recno);
	visit<int8_t>(v, &c->race_id);
	visit<int32_t>(v, &c->setup_date);
	visit<int8_t>(v, &c->ai_town);
	visit<int8_t>(v, &c->ai_link_checked);
	visit<int8_t>(v, &c->independ_town_nation_relation);
	visit<int8_t>(v, &c->has_linked_own_camp);
	visit<int8_t>(v, &c->has_linked_enemy_camp);
	visit<int8_t>(v, &c->is_base_town);
	visit<int16_t>(v, &c->loc_x1);
	visit<int16_t>(v, &c->loc_y1);
	visit<int16_t>(v, &c->loc_x2);
	visit<int16_t>(v, &c->loc_y2);
	visit<int16_t>(v, &c->abs_x1);
	visit<int16_t>(v, &c->abs_y1);
	visit<int16_t>(v, &c->abs_x2);
	visit<int16_t>(v, &c->abs_y2);
	visit<int16_t>(v, &c->center_x);
	visit<int16_t>(v, &c->center_y);
	visit<uint8_t>(v, &c->region_id);
	visit<int16_t>(v, &c->layout_id);
	visit<int16_t>(v, &c->first_slot_id);
	visit_array<int16_t>(v, c->slot_object_id_array);
	visit<int16_t>(v, &c->population);
	visit<int16_t>(v, &c->jobless_population);
	visit_array<int16_t>(v, c->max_race_pop_array);
	visit_array<int16_t>(v, c->race_pop_array);
	visit_array<uint8_t>(v, c->race_pop_growth_array);
	visit_array<int16_t>(v, c->jobless_race_pop_array);
	visit_array<float>(v, c->race_loyalty_array);
	visit_array<int8_t>(v, c->race_target_loyalty_array);
	visit_array<int16_t>(v, c->race_spy_count_array);
	for (int i = 0; i < MAX_RACE; ++i) {
		visit_array<float>(v, c->race_resistance_array[i]);
	}
	for (int i = 0; i < MAX_RACE; ++i) {
		visit_array<int8_t>(v, c->race_target_resistance_array[i]);
	}
	visit<int16_t>(v, &c->town_defender_count);
	visit<int32_t>(v, &c->last_being_attacked_date);
	visit<float>(v, &c->received_hit_count);
	visit_array<int8_t>(v, c->train_queue_skill_array);
	visit_array<int8_t>(v, c->train_queue_race_array);
	visit<int8_t>(v, &c->train_queue_count);
	visit<int16_t>(v, &c->train_unit_recno);
	visit<int32_t>(v, &c->train_unit_action_id);
	visit<uint32_t>(v, &c->start_train_frame_no);
	visit<int16_t>(v, &c->defend_target_recno);
	visit<int32_t>(v, &c->accumulated_collect_tax_penalty);
	visit<int32_t>(v, &c->accumulated_reward_penalty);
	visit<int32_t>(v, &c->accumulated_recruit_penalty);
	visit<int32_t>(v, &c->accumulated_enemy_grant_penalty);
	visit<int32_t>(v, &c->last_rebel_date);
	visit<int16_t>(v, &c->independent_unit_join_nation_min_rating);
	visit<int16_t>(v, &c->quality_of_life);
	visit<int16_t>(v, &c->auto_collect_tax_loyalty);
	visit<int16_t>(v, &c->auto_grant_loyalty);
	visit<int8_t>(v, &c->town_combat_level);
	visit_array<int8_t>(v, c->has_product_supply);
	visit<int8_t>(v, &c->no_neighbor_space);
	visit<int16_t>(v, &c->linked_firm_count);
	visit<int16_t>(v, &c->linked_town_count);
	visit_array<int16_t>(v, c->linked_firm_array);
	visit_array<int16_t>(v, c->linked_town_array);
	visit_array<int8_t>(v, c->linked_firm_enable_array);
	visit_array<int8_t>(v, c->linked_town_enable_array);
	//visit<int32_t>(v, &c->town_network_recno);
	//visit<int8_t>(v, &c->town_network_pulsed);
}

template <typename Visitor>
static void visit_version_1_town(Visitor* v, Town* c)
{
	// Visit version 1 town, then convert it to Town
	Version_1_Town *oldTown = (Version_1_Town*) mem_add(sizeof(Version_1_Town));
	visit_version_1_town_members(v, oldTown);
	oldTown->convert_to_version_2(c);
	mem_del(oldTown);
}

template <typename Visitor>
static void visit_town_array(Visitor* v, TownArray* c)
{
	enum { TOWN_RECORD_SIZE = 812 };

	c->visit_array_size(v);

	visit<int16_t>(v, &c->selected_recno);
	v->with_record_size(MAX_RACE * 4);
	visit_array<int32_t>(v, c->race_wander_pop_array);
	visit<int16_t>(v, &Town::if_town_recno);

	c->visit_ptr_array<Town>(v, yes_or_no_object_id<Town>, [](short) {return new Town;}, visit_town<Visitor>, TOWN_RECORD_SIZE);
}

template <typename Visitor>
static void visit_version_1_town_array(Visitor* v, TownArray* c)
{
	c->visit_array_size(v);

	visit<int16_t>(v, &c->selected_recno);
	for (int i = 0; i < MAX_RACE; ++i)
	{
		if (i < VERSION_1_MAX_RACE)
			visit<int32_t>(v, &c->race_wander_pop_array[i]);
		else
			c->race_wander_pop_array[i] = 0;
	}
	visit<int16_t>(v, &Town::if_town_recno);

	enum { VERSION_1_TOWN_RECORD_SIZE = 665 };

	c->visit_ptr_array<Town>(v, yes_or_no_object_id<Town>, [](short) {return new Town;}, visit_version_1_town<Visitor>, VERSION_1_TOWN_RECORD_SIZE);
}


//-------- Start of function TownArray::write_file -------------//
//
int TownArray::write_file(File* filePtr)
{
	FileWriterVisitor v(filePtr);
	visit_town_array(&v, this);
	return v.good();
}
//--------- End of function TownArray::write_file ---------------//


//-------- Start of function TownArray::read_file -------------//
//
int TownArray::read_file(File* filePtr)
{
	FileReaderVisitor v(filePtr);

	if (GameFile::read_file_same_version)
		visit_town_array(&v, this);
	else
		visit_version_1_town_array(&v, this);

	return v.good();
}
//--------- End of function TownArray::read_file ---------------//
