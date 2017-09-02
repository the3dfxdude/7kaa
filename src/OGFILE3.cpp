/*
 * Seven Kingdoms: Ancient Adversaries
 *
 * Copyright 1997,1998 Enlight Software Ltd.
 * Copyright 2010 Unavowed <unavowed@vexillium.org>
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

//Filename    : OGFILE3.CPP
//Description : Object Game file, save game and restore game, part 3

#include <OGFILE.h>
#include <OGF_V1.h>
#include <ONATION.h>
#include <ONEWS.h>
#include <OREBEL.h>
#include <OREGION.h>
#include <OREGIONS.h>
#include <OSITE.h>
#include <OSNOWG.h>
#include <OSPY.h>
#include <file_io_visitor.h>
#include <OGFILE_DYNARRAYB.inl>
#include <dbglog.h>

using namespace FileIOVisitor;

DBGLOG_DEFAULT_CHANNEL(GameFile);

//------- declare static functions -------//

static void write_ai_info(File* filePtr, short* aiInfoArray, short aiInfoCount, short aiInfoSize);
static void read_ai_info(File* filePtr, short** aiInfoArrayPtr, short& aiInfoCount, short& aiInfoSize);


//*****//


//-------- Start of function SiteArray::write_file -------------//
//
int SiteArray::write_file(File* filePtr)
{
	filePtr->file_put_short(selected_recno);
	filePtr->file_put_short(untapped_raw_count);
	filePtr->file_put_short(scroll_count);
	filePtr->file_put_short(gold_coin_count);
	filePtr->file_put_short(std_raw_site_count);

	FileWriterVisitor v(filePtr);
	accept_visitor_as_value_array(&v, visit_raw<FileWriterVisitor, Site>);
	return v.good();
}
//--------- End of function SiteArray::write_file ---------------//


//-------- Start of function SiteArray::read_file -------------//
//
int SiteArray::read_file(File* filePtr)
{
	selected_recno		 = filePtr->file_get_short();
	untapped_raw_count =	filePtr->file_get_short();
	scroll_count		 = filePtr->file_get_short();
	gold_coin_count	 =	filePtr->file_get_short();
	std_raw_site_count =	filePtr->file_get_short();

	FileReaderVisitor v(filePtr);
	accept_visitor_as_value_array(&v, visit_raw<FileReaderVisitor, Site>);
	return v.good();
}
//--------- End of function SiteArray::read_file ---------------//


//*****//


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

static bool read_nation_array(File *file, NationArray *na)
{
	return visit_with_record_size<FileReaderVisitor>(file, na, &visit_nation_array<FileReaderVisitor>,
										  NATION_ARRAY_RECORD_SIZE);
}

//-------- Start of function NationArray::write_file -------------//
//
int NationArray::write_file(File* filePtr)
{
	//------ write info in NationArray ------//
	
	if (!visit_with_record_size<FileWriterVisitor>(filePtr, this, &visit_nation_array<FileWriterVisitor>,
										 NATION_ARRAY_RECORD_SIZE))
		return 0;

   //---------- write Nations --------------//

   int    i;
   Nation *nationPtr;

   filePtr->file_put_short( size() );  // no. of nations in nation_array

   for( i=1; i<=size() ; i++ )
   {
      nationPtr = (Nation*) get_ptr(i);

      //----- write nationId or 0 if the nation is deleted -----//

      if( !nationPtr )    // the nation is deleted
      {
         filePtr->file_put_short(0);
      }
      else
      {
         filePtr->file_put_short(1);      // there is a nation in this record

         //------ write data in the base class ------//

         if( !nationPtr->write_file(filePtr) )
            return 0;
      }
   }

   //------- write empty room array --------//

   {
	   FileWriterVisitor v(filePtr);
	   visit_empty_room_array(&v);
   }

   return 1;
}
//--------- End of function NationArray::write_file -------------//

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

enum { VERSION_1_NATION_ARRAY_RECORD_SIZE = 282 };

//-------- Start of function NationArray::read_file -------------//
//
int NationArray::read_file(File* filePtr)
{
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
	}
	else
	{
		if (!read_nation_array(filePtr, this))
			return 0;
	}

   //---------- read Nations --------------//

   int     i, nationRecno, nationCount;
   Nation* nationPtr;

   nationCount = filePtr->file_get_short();  // get no. of nations from file

   for( i=1 ; i<=nationCount ; i++ )
   {
      if( filePtr->file_get_short() == 0 )
      {
         add_blank(1);     // it's a DynArrayB function
      }
      else
      {
         //----- create nation object -----------//

         nationRecno = create_nation();
         nationPtr   = nation_array[nationRecno];

         //----- read data in base class --------//

         if( !nationPtr->read_file( filePtr ) )
            return 0;
      }
   }

   //-------- linkout() those record added by add_blank() ----------//
   //-- So they will be marked deleted in DynArrayB and can be -----//
   //-- undeleted and used when a new record is going to be added --//

   for( i=size() ; i>0 ; i-- )
   {
      DynArrayB::go(i);             // since NationArray has its own go() which will call GroupArray::go()

      if( get_ptr() == NULL )       // add_blank() record
         linkout();
   }

	//-------- set NationArray::player_ptr -----------//

   player_ptr = nation_array[player_recno];

	//------- read empty room array --------//

   {
	   FileReaderVisitor v(filePtr);
	   visit_empty_room_array(&v);
   }

	return 1;
}
//--------- End of function NationArray::read_file ---------------//

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
static void visit_version_1_nation(Visitor *v, Version_1_Nation *v1n)
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

enum { VERSION_1_NATION_RECORD_SIZE = 2182 };

static bool read_version_1_nation(File *file, Version_1_Nation *v1n)
{
	if (!visit_with_record_size<FileReaderVisitor>(file, v1n, &visit_version_1_nation<FileReaderVisitor>,
										VERSION_1_NATION_RECORD_SIZE))
		return false;

	memset(&v1n->action_array, 0, sizeof(v1n->action_array));
	return true;
}

template <typename Visitor>
static void visit_nation(Visitor *v, Nation *nat)
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

enum { NATION_RECORD_SIZE = 2202 };

//--------- Begin of function Nation::write_file ---------//
//
int Nation::write_file(File* filePtr)
{
	if (!visit_with_record_size<FileWriterVisitor>(filePtr, this, &visit_nation<FileWriterVisitor>,
										 NATION_RECORD_SIZE))
		return 0;

	//----------- write AI Action Array ------------//

	action_array.write_file(filePtr);

	//------ write AI info array ---------//

	write_ai_info(filePtr, ai_town_array, ai_town_count, ai_town_size);

	write_ai_info(filePtr, ai_base_array, ai_base_count, ai_base_size);
	write_ai_info(filePtr, ai_mine_array, ai_mine_count, ai_mine_size);
	write_ai_info(filePtr, ai_factory_array, ai_factory_count, ai_factory_size);
	write_ai_info(filePtr, ai_market_array, ai_market_count, ai_market_size);
	write_ai_info(filePtr, ai_inn_array, ai_inn_count, ai_inn_size);
	write_ai_info(filePtr, ai_camp_array, ai_camp_count, ai_camp_size);
	write_ai_info(filePtr, ai_research_array, ai_research_count, ai_research_size);
	write_ai_info(filePtr, ai_war_array, ai_war_count, ai_war_size);
	write_ai_info(filePtr, ai_harbor_array, ai_harbor_count, ai_harbor_size);

	write_ai_info(filePtr, ai_caravan_array, ai_caravan_count, ai_caravan_size);
	write_ai_info(filePtr, ai_ship_array, ai_ship_count, ai_ship_size);
	write_ai_info(filePtr, ai_general_array, ai_general_count, ai_general_size);

	return 1;
}
//----------- End of function Nation::write_file ---------//


//--------- Begin of static function write_ai_info ---------//
//
static void write_ai_info(File* filePtr, short* aiInfoArray, short aiInfoCount, short aiInfoSize)
{
	filePtr->file_put_short( aiInfoCount );
	filePtr->file_put_short( aiInfoSize  );
	filePtr->file_write( aiInfoArray, sizeof(short) * aiInfoCount );
}
//----------- End of static function write_ai_info ---------//

static bool read_nation(File *file, Nation *nat)
{
	if (!visit_with_record_size<FileReaderVisitor>(file, nat, &visit_nation<FileReaderVisitor>, NATION_RECORD_SIZE))
		return false;

	memset(&nat->action_array, 0, sizeof(nat->action_array));
	return true;
}

//--------- Begin of function Nation::read_file ---------//
//
int Nation::read_file(File* filePtr)
{
	if(!GameFile::read_file_same_version)
	{
		Version_1_Nation *oldNationPtr = (Version_1_Nation*) mem_add(sizeof(Version_1_Nation));

		if (!read_version_1_nation(filePtr, oldNationPtr))
		{
			mem_del(oldNationPtr);
			return 0;
		}

		oldNationPtr->convert_to_version_2(this);
		mem_del(oldNationPtr);
	}
	else
	{
		if (!read_nation(filePtr, this))
			return 0;
	}

	//-------------- read AI Action Array --------------//

	action_array.read_file(filePtr);

	//------ write AI info array ---------//

	read_ai_info(filePtr, &ai_town_array, ai_town_count, ai_town_size);

	read_ai_info(filePtr, &ai_base_array, ai_base_count, ai_base_size);
	read_ai_info(filePtr, &ai_mine_array, ai_mine_count, ai_mine_size);
	read_ai_info(filePtr, &ai_factory_array, ai_factory_count, ai_factory_size);
	read_ai_info(filePtr, &ai_market_array, ai_market_count, ai_market_size);
	read_ai_info(filePtr, &ai_inn_array, ai_inn_count, ai_inn_size);
	read_ai_info(filePtr, &ai_camp_array, ai_camp_count, ai_camp_size);
	read_ai_info(filePtr, &ai_research_array, ai_research_count, ai_research_size);
	read_ai_info(filePtr, &ai_war_array, ai_war_count, ai_war_size);
	read_ai_info(filePtr, &ai_harbor_array, ai_harbor_count, ai_harbor_size);

	read_ai_info(filePtr, &ai_caravan_array, ai_caravan_count, ai_caravan_size);
	read_ai_info(filePtr, &ai_ship_array, ai_ship_count, ai_ship_size);
	read_ai_info(filePtr, &ai_general_array, ai_general_count, ai_general_size);

	return 1;
}
//----------- End of function Nation::read_file ---------//


//--------- Begin of static function read_ai_info ---------//
//
static void read_ai_info(File* filePtr, short** aiInfoArrayPtr, short& aiInfoCount, short& aiInfoSize)
{
	aiInfoCount = filePtr->file_get_short();
	aiInfoSize  = filePtr->file_get_short();

	*aiInfoArrayPtr = (short*) mem_add( aiInfoSize * sizeof(short) );

	filePtr->file_read( *aiInfoArrayPtr, sizeof(short) * aiInfoCount );
}
//----------- End of static function read_ai_info ---------//


//*****//


//-------- Start of function SpyArray::write_file -------------//
//
int SpyArray::write_file(File* filePtr)
{
	FileWriterVisitor v(filePtr);
	accept_visitor_as_value_array(&v, visit_raw<FileWriterVisitor, Spy>);
	return v.good();
}
//--------- End of function SpyArray::write_file ---------------//


//-------- Start of function SpyArray::read_file -------------//
//
int SpyArray::read_file(File* filePtr)
{
	FileReaderVisitor v(filePtr);
	accept_visitor_as_value_array(&v, visit_raw<FileReaderVisitor, Spy>);
	return v.good();
}
//--------- End of function SpyArray::read_file ---------------//


//*****//


//-------- Start of function SnowGroundArray::write_file -------------//
//
int SnowGroundArray::write_file(File* filePtr)
{
	MSG(__FILE__":%d: file_write(this, ...);\n", __LINE__);

   if( !filePtr->file_write( this, sizeof(SnowGroundArray)) )
      return 0;

   return 1;
}
//--------- End of function SnowGroundArray::write_file ---------------//


//-------- Start of function SnowGroundArray::read_file -------------//
//
int SnowGroundArray::read_file(File* filePtr)
{
	MSG(__FILE__":%d: file_read(this, ...);\n", __LINE__);

   if( !filePtr->file_read( this, sizeof(SnowGroundArray)) )
      return 0;

   return 1;
}
//--------- End of function SnowGroundArray::read_file ---------------//

//*****//

template <typename Visitor>
static void visit_region_array(Visitor *v, RegionArray *ra)
{
	visit<int32_t>(v, &ra->init_flag);
	visit_pointer(v, &ra->region_info_array);
	visit<int32_t>(v, &ra->region_info_count);
	visit_pointer(v, &ra->region_stat_array);
	visit<int32_t>(v, &ra->region_stat_count);
	visit_pointer(v, &ra->connect_bits);
	visit_array<uint8_t>(v, ra->region_sorted_array);
}

enum { REGION_ARRAY_RECORD_SIZE = 279 };

//-------- Start of function RegionArray::write_file -------------//
//
int RegionArray::write_file(File* filePtr)
{
	if (!visit_with_record_size<FileWriterVisitor>(filePtr, this, &visit_region_array<FileWriterVisitor>,
										 REGION_ARRAY_RECORD_SIZE))
		return 0;

	if( !filePtr->file_write( region_info_array, sizeof(RegionInfo)*region_info_count ) )
		return 0;

	//-------- write RegionStat ----------//

	filePtr->file_put_short( region_stat_count );

	if( !filePtr->file_write( region_stat_array, sizeof(RegionStat)*region_stat_count ) )
		return 0;

	//--------- write connection bits ----------//

	int connectBit = (region_info_count -1) * (region_info_count) /2;
	int connectByte = (connectBit +7) /8;

	if( connectByte > 0)
	{
		if( !filePtr->file_write(connect_bits, connectByte) )
			return 0;
	}

	return 1;
}
//--------- End of function RegionArray::write_file ---------------//


//-------- Start of function RegionArray::read_file -------------//
//
int RegionArray::read_file(File* filePtr)
{
	if (!visit_with_record_size<FileReaderVisitor>(filePtr, this, &visit_region_array<FileReaderVisitor>,
										REGION_ARRAY_RECORD_SIZE))
		return 0;

   if( region_info_count > 0 )
      region_info_array = (RegionInfo *) mem_add(sizeof(RegionInfo)*region_info_count);
   else
      region_info_array = NULL;

   if( !filePtr->file_read( region_info_array, sizeof(RegionInfo)*region_info_count))
      return 0;

	//-------- read RegionStat ----------//

	region_stat_count = filePtr->file_get_short();

	region_stat_array = (RegionStat*) mem_add( region_stat_count * sizeof(RegionStat) );

	if( !filePtr->file_read( region_stat_array, sizeof(RegionStat)*region_stat_count ) )
		return 0;

	//--------- read connection bits ----------//

	int connectBit = (region_info_count -1) * (region_info_count) /2;
	int connectByte = (connectBit +7) /8;

	if( connectByte > 0)
	{
		connect_bits = (unsigned char *)mem_add(connectByte);
		if( !filePtr->file_read(connect_bits, connectByte) )
			return 0;
	}
	else
	{
		connect_bits = NULL;
	}

	return 1;
}
//--------- End of function RegionArray::read_file ---------------//

//*****//

//-------- Start of function NewsArray::write_file -------------//
//
int NewsArray::write_file(File* filePtr)
{
   //----- save news_array parameters -----//

   filePtr->file_write( news_type_option, sizeof(news_type_option) );

   filePtr->file_put_short(news_who_option);
   filePtr->file_put_long (last_clear_recno);

   //---------- save news data -----------//

   return DynArray::write_file(filePtr);
}
//--------- End of function NewsArray::write_file ---------------//


//-------- Start of function NewsArray::read_file -------------//
//
int NewsArray::read_file(File* filePtr)
{
   //----- read news_array parameters -----//

   filePtr->file_read( news_type_option, sizeof(news_type_option) );

   news_who_option   = (char) filePtr->file_get_short();
   last_clear_recno  = filePtr->file_get_long();

   //---------- read news data -----------//

   return DynArray::read_file(filePtr);
}
//--------- End of function NewsArray::read_file ---------------//
/* vim:set ts=3 sw=3: */
