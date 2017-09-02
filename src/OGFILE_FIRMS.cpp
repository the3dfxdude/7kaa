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


#include <OFIRM.h>
#include <OF_BASE.h>
#include <OF_CAMP.h>
#include <OF_FACT.h>
#include <OF_HARB.h>
#include <OF_INN.h>
#include <OF_MARK.h>
#include <OF_MINE.h>
#include <OF_MONS.h>
#include <OF_RESE.h>
#include <OF_WAR.h>
#include <OGFILE.h>
#include <OGF_V1.h>
#include <file_io_visitor.h>
#include <dbglog.h>

using namespace FileIOVisitor;

DBGLOG_DEFAULT_CHANNEL(GameFile);


template <typename Visitor>
static void visit_firm_members(Visitor *v, Firm *f)
{
	v->skip(4); /* virtual table pointer */

	visit<int8_t>(v, &f->firm_id);
	visit<int16_t>(v, &f->firm_build_id);
	visit<int16_t>(v, &f->firm_recno);
	visit<int8_t>(v, &f->firm_ai);
	visit<int8_t>(v, &f->ai_processed);
	visit<int8_t>(v, &f->ai_status);
	visit<int8_t>(v, &f->ai_link_checked);
	visit<int8_t>(v, &f->ai_sell_flag);
	visit<int8_t>(v, &f->race_id);
	visit<int16_t>(v, &f->nation_recno);
	visit<int16_t>(v, &f->closest_town_name_id);
	visit<int16_t>(v, &f->firm_name_instance_id);
	visit<int16_t>(v, &f->loc_x1);
	visit<int16_t>(v, &f->loc_y1);
	visit<int16_t>(v, &f->loc_x2);
	visit<int16_t>(v, &f->loc_y2);
	visit<int16_t>(v, &f->abs_x1);
	visit<int16_t>(v, &f->abs_y1);
	visit<int16_t>(v, &f->abs_x2);
	visit<int16_t>(v, &f->abs_y2);
	visit<int16_t>(v, &f->center_x);
	visit<int16_t>(v, &f->center_y);
	visit<uint8_t>(v, &f->region_id);
	visit<int8_t>(v, &f->cur_frame);
	visit<int8_t>(v, &f->remain_frame_delay);
	visit<float>(v, &f->hit_points);
	visit<float>(v, &f->max_hit_points);
	visit<int8_t>(v, &f->under_construction);
	visit<int8_t>(v, &f->firm_skill_id);
	visit<int16_t>(v, &f->overseer_recno);
	visit<int16_t>(v, &f->overseer_town_recno);
	visit<int16_t>(v, &f->builder_recno);
	visit<uint8_t>(v, &f->builder_region_id);
	visit<float>(v, &f->productivity);
	visit_pointer(v, &f->worker_array);
	visit<int8_t>(v, &f->worker_count);
	visit<int8_t>(v, &f->selected_worker_id);
	visit<int8_t>(v, &f->player_spy_count);
	visit<uint8_t>(v, &f->sabotage_level);
	visit<int8_t>(v, &f->linked_firm_count);
	visit<int8_t>(v, &f->linked_town_count);
	visit_array<int16_t>(v, f->linked_firm_array);
	visit_array<int16_t>(v, f->linked_town_array);

	visit_array<int8_t>(v, f->linked_firm_enable_array);

	visit_array<int8_t>(v, f->linked_town_enable_array);

	visit<float>(v, &f->last_year_income);
	visit<float>(v, &f->cur_year_income);
	visit<int32_t>(v, &f->setup_date);
	visit<int8_t>(v, &f->should_set_power);
	visit<int32_t>(v, &f->last_attacked_date);
	visit<int8_t>(v, &f->should_close_flag);
	visit<int8_t>(v, &f->no_neighbor_space);
	visit<int8_t>(v, &f->ai_should_build_factory_count);
}

template <typename Visitor>
static void visit_worker_members(Visitor* vis, Worker* c)
{
	visit<int8_t>(vis, &c->race_id);
	visit<int8_t>(vis, &c->unit_id);
	visit<int16_t>(vis, &c->town_recno);
	visit<uint16_t>(vis, &c->name_id);
	visit<int8_t>(vis, &c->skill_id);
	visit<int8_t>(vis, &c->skill_level);
	visit<int8_t>(vis, &c->skill_level_minor);
	visit<int8_t>(vis, &c->skill_potential);
	visit<int8_t>(vis, &c->combat_level);
	visit<int8_t>(vis, &c->combat_level_minor);
	visit<int16_t>(vis, &c->spy_recno);
	visit<int8_t>(vis, &c->rank_id);
	visit<int8_t>(vis, &c->worker_loyalty);
	visit<int16_t>(vis, &c->hit_points);
	visit<int16_t>(vis, &c->extra_para);
}

template <typename Visitor>
static void visit_firm_worker_array(Visitor* v, Firm* firm, bool is_reader_visitor)
{
	// Handle presence or absence of worker array following the regular Firm members
	if( firm_res[firm->firm_id]->need_worker )
	{
		if (is_reader_visitor)
			firm->worker_array = (Worker*) mem_add( MAX_WORKER*sizeof(Worker) );

		v->with_record_size(MAX_WORKER*sizeof(Worker));
		for (int i = 0; i < MAX_WORKER; ++i) {
			visit_worker_members(v, &firm->worker_array[i]);
		}
	}
}

template <typename Visitor>
static void visit_firm_base_members(Visitor* v, FirmBase* c)
{
	visit<int16_t>(v, &c->god_id);
	visit<int16_t>(v, &c->god_unit_recno);
	visit<float>(v, &c->pray_points);
}

template <typename Visitor>
static void visit_defense_unit_members(Visitor* v, DefenseUnit* c)
{
	visit<int16_t>(v, &c->unit_recno);
	visit<int8_t>(v, &c->status);
}

template <typename Visitor>
static void visit_firm_camp_members(Visitor* v, FirmCamp* c)
{
	visit_array(v, c->defense_array, visit_defense_unit_members<Visitor>);
	visit<int8_t>(v, &c->employ_new_worker);
	visit<int16_t>(v, &c->defend_target_recno);
	visit<int8_t>(v, &c->defense_flag);
	visit<int8_t>(v, &c->patrol_unit_count);
	visit_array<int16_t>(v, c->patrol_unit_array);
	visit<int8_t>(v, &c->coming_unit_count);
	visit_array<int16_t>(v, c->coming_unit_array);
	visit<int16_t>(v, &c->ai_capture_town_recno);
	visit<int8_t>(v, &c->ai_recruiting_soldier);
	visit<int8_t>(v, &c->is_attack_camp);
}

template <typename Visitor>
static void visit_firm_factory_members(Visitor* v, FirmFactory* c)
{
	visit<int32_t>(v, &c->product_raw_id);
	visit<float>(v, &c->stock_qty);
	visit<float>(v, &c->max_stock_qty);
	visit<float>(v, &c->raw_stock_qty);
	visit<float>(v, &c->max_raw_stock_qty);
	visit<float>(v, &c->cur_month_production);
	visit<float>(v, &c->last_month_production);
	visit<int16_t>(v, &c->next_output_link_id);
	visit<int16_t>(v, &c->next_output_firm_recno);
}

template <typename Visitor>
static void visit_firm_harbor_members(Visitor* v, FirmHarbor* c)
{
	visit_array<int16_t>(v, c->ship_recno_array);
	visit<int16_t>(v, &c->ship_count);
	visit<int16_t>(v, &c->build_unit_id);
	visit<uint32_t>(v, &c->start_build_frame_no);
	visit_array<int8_t>(v, c->build_queue_array);
	visit<int8_t>(v, &c->build_queue_count);
	visit<uint8_t>(v, &c->land_region_id);
	visit<uint8_t>(v, &c->sea_region_id);
	visit<int8_t>(v, &c->link_checked);
	visit<int8_t>(v, &c->linked_mine_num);
	visit<int8_t>(v, &c->linked_factory_num);
	visit<int8_t>(v, &c->linked_market_num);
	visit_array<int16_t>(v, c->linked_mine_array);
	visit_array<int16_t>(v, c->linked_factory_array);
	visit_array<int16_t>(v, c->linked_market_array);
}

template <typename Visitor>
static void visit_skill_members(Visitor* v, Skill* c)
{
	visit<int8_t>(v, &c->combat_level);
	visit<int8_t>(v, &c->skill_id);
	visit<int8_t>(v, &c->skill_level);
	visit<uint8_t>(v, &c->combat_level_minor);
	visit<uint8_t>(v, &c->skill_level_minor);
	visit<uint8_t>(v, &c->skill_potential);
}


template <typename Visitor>
static void visit_inn_unit_members(Visitor* v, InnUnit* c)
{
	visit<int8_t>(v, &c->unit_id);
	visit_skill_members(v, &c->skill);
	visit<int16_t>(v, &c->hire_cost);
	visit<int16_t>(v, &c->stay_count);
	visit<int16_t>(v, &c->spy_recno);
}

template <typename Visitor>
static void visit_firm_inn_members(Visitor* v, FirmInn* c)
{
	visit<int16_t>(v, &c->next_skill_id);
	visit_array(v, c->inn_unit_array, visit_inn_unit_members<Visitor>);
	visit<int16_t>(v, &c->inn_unit_count);
}

template <typename Visitor>
static void visit_market_goods_members(Visitor* v, MarketGoods* c)
{
	visit<int8_t>(v, &c->raw_id);
	visit<int8_t>(v, &c->product_raw_id);
	visit<int16_t>(v, &c->input_firm_recno);
	visit<float>(v, &c->stock_qty);
	visit<float>(v, &c->cur_month_supply);
	visit<float>(v, &c->last_month_supply);
	visit<float>(v, &c->month_demand);
	visit<float>(v, &c->cur_month_sale_qty);
	visit<float>(v, &c->last_month_sale_qty);
	visit<float>(v, &c->cur_year_sales);
	visit<float>(v, &c->last_year_sales);
}

template <typename Visitor>
static void visit_firm_market_members(Visitor* v, FirmMarket* c)
{
	visit<float>(v, &c->max_stock_qty);
	visit_array(v, c->market_goods_array, visit_market_goods_members<Visitor>);
	for (int i = 0; i < MAX_RAW; ++i) v->skip(4); // Skip market_raw_array
	for (int i = 0; i < MAX_PRODUCT; ++i) v->skip(4); // Skip market_product_array
	visit<int16_t>(v, &c->next_output_link_id);
	visit<int16_t>(v, &c->next_output_firm_recno);
	visit<int32_t>(v, &c->no_linked_town_since_date);
	visit<int32_t>(v, &c->last_import_new_goods_date);
	visit<int8_t>(v, &c->is_retail_market);
}

template <typename Visitor>
static void visit_firm_mine_members(Visitor* v, FirmMine* c)
{
	visit<int16_t>(v, &c->raw_id);
	visit<int16_t>(v, &c->site_recno);
	visit<float>(v, &c->reserve_qty);
	visit<float>(v, &c->stock_qty);
	visit<float>(v, &c->max_stock_qty);
	visit<int16_t>(v, &c->next_output_link_id);
	visit<int16_t>(v, &c->next_output_firm_recno);
	visit<float>(v, &c->cur_month_production);
	visit<float>(v, &c->last_month_production);
}

template <typename Visitor>
static void visit_monster_in_firm_members(Visitor* v, MonsterInFirm* c)
{
	visit<int8_t>(v, &c->monster_id);
	visit<int8_t>(v, &c->_unused);
	visit<int16_t>(v, &c->mobile_unit_recno);
	visit<int8_t>(v, &c->combat_level);
	visit<int16_t>(v, &c->hit_points);
	visit<int16_t>(v, &c->max_hit_points);
	visit<int8_t>(v, &c->soldier_monster_id);
	visit<int8_t>(v, &c->soldier_count);
}

template <typename Visitor>
static void visit_firm_monster_members(Visitor* v, FirmMonster* c)
{
	visit<int16_t>(v, &c->monster_id);
	visit<int16_t>(v, &c->monster_general_count);
	visit<int8_t>(v, &c->monster_aggressiveness);
	visit<int8_t>(v, &c->defending_king_count);
	visit<int8_t>(v, &c->defending_general_count);
	visit<int8_t>(v, &c->defending_soldier_count);
	visit_monster_in_firm_members(v, &c->monster_king);
	visit_array(v, c->monster_general_array, visit_monster_in_firm_members<Visitor>);
	visit<int8_t>(v, &c->waiting_soldier_count);
	visit_array<int16_t>(v, c->waiting_soldier_array);
	visit<int8_t>(v, &c->monster_nation_relation);
	visit<int16_t>(v, &c->defend_target_recno);
	visit<int8_t>(v, &c->patrol_unit_count);
	visit_array<int16_t>(v, c->patrol_unit_array);
}

template <typename Visitor>
static void visit_firm_research_members(Visitor* v, FirmResearch* c)
{
	visit<int16_t>(v, &c->tech_id);
	visit<float>(v, &c->complete_percent);
}

template <typename Visitor>
static void visit_firm_war_members(Visitor* v, FirmWar* c)
{
	visit<int16_t>(v, &c->build_unit_id);
	visit<uint32_t>(v, &c->last_process_build_frame_no);
	visit<float>(v, &c->build_progress_days);
	visit_array<int8_t>(v, c->build_queue_array);
	visit<int8_t>(v, &c->build_queue_count);
}

void Firm::accept_file_visitor(FileReaderVisitor* v)
{
	visit_firm_members(v, this);
	visit_firm_worker_array(v, this, true);

	// Hack: the derived part is written separately with its own record size (if sizeof(DerivedFirm)-sizeof(Firm)>0),
	//       but since any Firm is a derived instance with at least one member, we can consume the record size here.
	uint16_t derivedRecordSize;
	v->visit<uint16_t>(&derivedRecordSize);
}

void Firm::accept_file_visitor(FileWriterVisitor* v)
{
	visit_firm_members(v, this);
	visit_firm_worker_array(v, this, false);

	// Hack: the derived part is written separately with its own record size (if sizeof(DerivedFirm)-sizeof(Firm)>0),
	//       but since any Firm is a derived instance with at least one member, we can produce the record size here.
	uint16_t derivedRecordSize = 0; // 0 means don't use stored record size, and just go with the expected record size.
	v->visit<uint16_t>(&derivedRecordSize);
}

void FirmBase::accept_file_visitor(FileReaderVisitor* v)
{
	Firm::accept_file_visitor(v);
	visit_firm_base_members(v, this);
}

void FirmBase::accept_file_visitor(FileWriterVisitor* v)
{
	Firm::accept_file_visitor(v);
	visit_firm_base_members(v, this);
}

void FirmCamp::accept_file_visitor(FileReaderVisitor* v)
{
	Firm::accept_file_visitor(v);
	visit_firm_camp_members(v, this);
}

void FirmCamp::accept_file_visitor(FileWriterVisitor* v)
{
	Firm::accept_file_visitor(v);
	visit_firm_camp_members(v, this);
}

void FirmFactory::accept_file_visitor(FileReaderVisitor* v)
{
	Firm::accept_file_visitor(v);
	visit_firm_factory_members(v, this);
}

void FirmFactory::accept_file_visitor(FileWriterVisitor* v)
{
	Firm::accept_file_visitor(v);
	visit_firm_factory_members(v, this);
}

void FirmHarbor::accept_file_visitor(FileReaderVisitor* v)
{
	Firm::accept_file_visitor(v);
	visit_firm_harbor_members(v, this);
}

void FirmHarbor::accept_file_visitor(FileWriterVisitor* v)
{
	Firm::accept_file_visitor(v);
	visit_firm_harbor_members(v, this);
}

void FirmInn::accept_file_visitor(FileReaderVisitor* v)
{
	Firm::accept_file_visitor(v);
	visit_firm_inn_members(v, this);
}

void FirmInn::accept_file_visitor(FileWriterVisitor* v)
{
	Firm::accept_file_visitor(v);
	visit_firm_inn_members(v, this);
}

void FirmMarket::accept_file_visitor(FileReaderVisitor* v)
{
	Firm::accept_file_visitor(v);
	visit_firm_market_members(v, this);

	//----- rebuild market_raw_array[] & market_product_array[] ----//

	for( int i=0 ; i<MAX_RAW ; i++ )
	{
		market_raw_array[i]	   = NULL;
		market_product_array[i] = NULL;
	}

	for( int i=0 ; i<MAX_MARKET_GOODS ; i++ )
	{
		int rawId 	 = market_goods_array[i].raw_id;
		int productId = market_goods_array[i].product_raw_id;

		if( rawId )
			market_raw_array[rawId-1] = market_goods_array + i;

		if( productId )
			market_product_array[productId-1] = market_goods_array + i;
	}
}

void FirmMarket::accept_file_visitor(FileWriterVisitor* v)
{
	Firm::accept_file_visitor(v);
	visit_firm_market_members(v, this);
}

void FirmMine::accept_file_visitor(FileReaderVisitor* v)
{
	Firm::accept_file_visitor(v);
	visit_firm_mine_members(v, this);
}

void FirmMine::accept_file_visitor(FileWriterVisitor* v)
{
	Firm::accept_file_visitor(v);
	visit_firm_mine_members(v, this);
}

void FirmMonster::accept_file_visitor(FileReaderVisitor* v)
{
	Firm::accept_file_visitor(v);
	visit_firm_monster_members(v, this);
}

void FirmMonster::accept_file_visitor(FileWriterVisitor* v)
{
	Firm::accept_file_visitor(v);
	visit_firm_monster_members(v, this);
}

void FirmResearch::accept_file_visitor(FileReaderVisitor* v)
{
	Firm::accept_file_visitor(v);
	visit_firm_research_members(v, this);
}

void FirmResearch::accept_file_visitor(FileWriterVisitor* v)
{
	Firm::accept_file_visitor(v);
	visit_firm_research_members(v, this);
}

void FirmWar::accept_file_visitor(FileReaderVisitor* v)
{
	Firm::accept_file_visitor(v);
	visit_firm_war_members(v, this);
}

void FirmWar::accept_file_visitor(FileWriterVisitor* v)
{
	Firm::accept_file_visitor(v);
	visit_firm_war_members(v, this);
}


enum { FIRM_RECORD_SIZE = 254 };

//-------- Start of function FirmArray::write_file -------------//
//
int FirmArray::write_file(File* filePtr)
{
	int  i;
	Firm *firmPtr;

	filePtr->file_put_short( size()  );  // no. of firms in firm_array
	filePtr->file_put_short( process_recno );
	filePtr->file_put_short( selected_recno );

	filePtr->file_put_short( Firm::firm_menu_mode );
	filePtr->file_put_short( Firm::action_spy_recno );
	filePtr->file_put_short( Firm::bribe_result );
	filePtr->file_put_short( Firm::assassinate_result );

	for( i=1; i<=size() ; i++ )
	{
		firmPtr = (Firm*) get_ptr(i);

		//----- write firmId or 0 if the firm is deleted -----//

		if( !firmPtr )    // the firm is deleted
		{
			filePtr->file_put_short(0);
		}
		else
		{
			//--------- write firm_id -------------//

			filePtr->file_put_short(firmPtr->firm_id);

			//------ write data from (derived) class --------//

			if (!polymorphic_visit_with_record_size<FileWriterVisitor>(filePtr, firmPtr, FIRM_RECORD_SIZE))
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
//--------- End of function FirmArray::write_file ---------------//

//-------- Start of function FirmArray::read_file -------------//
//
int FirmArray::read_file(File* filePtr)
{
	Firm*   firmPtr;
	int     i, firmId, firmRecno;

	int firmCount      = filePtr->file_get_short();  // get no. of firms from file
	process_recno      = filePtr->file_get_short();
	selected_recno     = filePtr->file_get_short();

	Firm::firm_menu_mode  	 = (char) filePtr->file_get_short();
	Firm::action_spy_recno   = filePtr->file_get_short();
	Firm::bribe_result    	 = (char) filePtr->file_get_short();
	Firm::assassinate_result = (char) filePtr->file_get_short();

	for( i=1 ; i<=firmCount ; i++ )
	{
		firmId = filePtr->file_get_short();

		if( firmId==0 )  // the firm has been deleted
		{
			add_blank(1);     // it's a DynArrayB function
		}
		else
		{
			//----- create firm object -----------//

			firmRecno = create_firm( firmId );
			firmPtr   = firm_array[firmRecno];

			//------ read data into (derived) class --------//

			if (!polymorphic_visit_with_record_size<FileReaderVisitor>(filePtr, firmPtr, FIRM_RECORD_SIZE))
				return 0;

			//---- fixup version difference between v1 and v2 -----//

			if(!GameFile::read_file_same_version && firmPtr->firm_id > FIRM_BASE)
				firmPtr->firm_build_id += MAX_RACE - VERSION_1_MAX_RACE;
		}
	}

	//-------- linkout() those record added by add_blank() ----------//
	//-- So they will be marked deleted in DynArrayB and can be -----//
	//-- undeleted and used when a new record is going to be added --//

	for( i=size() ; i>0 ; i-- )
	{
		DynArrayB::go(i);             // since FirmArray has its own go() which will call GroupArray::go()

		if( get_ptr() == NULL )       // add_blank() record
			linkout();
	}

	//------- read empty room array --------//

	{
		FileReaderVisitor v(filePtr);
		visit_empty_room_array(&v);
	}

	return 1;
}
//--------- End of function FirmArray::read_file ---------------//
