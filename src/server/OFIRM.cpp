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

//Filename    : OFIRM.CPP
//Description : Object Firm

#include <string.h>
#include <OVGA.h>
#include <ODATE.h>
#include <OMLINK.h>
#include <OWORLD.h>
#include <OPOWER.h>
#include <OCONFIG.h>
#include <OGAME.h>
#include <OUNIT.h>
#include <ONEWS.h>
#include <OSYS.h>
#include <OSPY.h>
#include <OSITE.h>
#include <OINFO.h>
#include <ONEWS.h>
#include <ONATION.h>
#include <OFIRM.h>
#include <ORACERES.h>
#include <OTOWN.h>
#include <OSPRITE.h>
#include <OFIRMRES.h>
#include <OF_MARK.h>
#ifdef USE_DPLAY
#include <OREMOTE.h>
#endif
#include <OF_CAMP.h>
#include <OF_HARB.h>
#include <OSERES.h>
#include <OREBEL.h>
// ###### begin Gilbert 2/10 ######//
#include <OFIRMDIE.h>
// ###### end Gilbert 2/10 ######//
#include <OUNITRES.h>


//---------- define static member vars -------------//

char  Firm::firm_menu_mode=FIRM_MENU_MAIN;		// whether the firm is in spy menu mode
short Firm::action_spy_recno;
char	Firm::bribe_result=BRIBE_NONE;
char	Firm::assassinate_result=0;

//----------- define static parameters -------------//

static int remove_firm = 0; // true only when the firm is to be removed from the firm_array

//--------- Begin of function Firm::Firm --------//
//
// After created a Firm, you must either call Firm::set_world_matrix()
// to set the record no. of the firm in the matrix or set it yourself
//
// NOTE : this function will be called by firm_array.read_file()
//        it CANNOT change any settings in nation_array
//
Firm::Firm()
{
    firm_id = 0;
    firm_build_id = 0;
    firm_recno = 0;
    firm_ai = 0;
    ai_processed = 0;
    ai_status = 0;
    ai_link_checked = 0;
    ai_sell_flag = 0;
    race_id = 0;
    nation_recno = 0;
    closest_town_name_id = 0;
    firm_name_instance_id = 0;
    loc_x1 = loc_y1 = loc_x2 = loc_y2 = 0;
    abs_x1 = abs_y1 = abs_x2 = abs_y2 = 0;
    center_x = center_y = 0;
    region_id = 0;
    cur_frame = 0;
    remain_frame_delay = 0;
    hit_points = 0.0f;
    max_hit_points = 0.0f;
    under_construction = 0;
    firm_skill_id = 0;
    overseer_recno = 0;
    overseer_town_recno = 0;
    builder_recno = 0;
    builder_region_id = 0;
    productivity = 0.0f;
    worker_array = NULL;
    worker_count = 0;
    selected_worker_id = 0;
    player_spy_count = 0;
    sabotage_level = 0;
    linked_firm_count = 0;
    linked_town_count = 0;
    memset(linked_firm_array, 0, sizeof(short) * MAX_LINKED_FIRM_FIRM);
    memset(linked_town_array, 0, sizeof(short) * MAX_LINKED_FIRM_TOWN);
    memset(linked_firm_enable_array, 0, MAX_LINKED_FIRM_FIRM);
    memset(linked_town_enable_array, 0, MAX_LINKED_FIRM_TOWN);
    last_year_income = 0.0f;
    cur_year_income = 0.0f;
    setup_date = 0;
    should_set_power = 0;
    last_attacked_date = 0;
    should_close_flag = 0;
    no_neighbor_space = 0;
    ai_should_build_factory_count = 0;
}
//----------- End of function Firm::Firm ---------//


//--------- Begin of function Firm::~Firm --------//
//
// Two ways to terminate a Firm :
//
// 1.call Firm::deinit() first and then delete the firm
// 2.delete the firm directly
//
Firm::~Firm()
{
   deinit();
}
//----------- End of function Firm::~Firm --------//


//--------- Begin of function Firm::init --------//
//
// It will initialize vars, and set the world matrix.
// Before calling init(), firm_recno should be set
//
// Note : it will set world matrix regardless the existing location content,
//        so you must ensure that the location is clean by calling
//        world.zoom_matrix->add_firm_test()
//
// <int> xLoc, yLoc  = the location of firm in the world map
// <int> nationRecno = the recno of nation which build this firm
// <int> firmId      = id(type) of the firm
// [char*] buildCode = the build code of the firm, no need to give if the firm just have one build type
// [short] builderRecno = recno of the builder unit
//
void Firm::init(int xLoc, int yLoc, int nationRecno, int firmId, const char* buildCode, short builderRecno)
{
	FirmInfo* firmInfo = firm_res[firmId];

	firm_id = firmId;

	if( buildCode )
		firm_build_id = firmInfo->get_build_id(buildCode);
	else
		firm_build_id = firmInfo->first_build_id;

	//----------- set vars -------------//

	nation_recno   = nationRecno;
	setup_date     = info.game_date;

	overseer_recno = 0;

	if( firmInfo->need_worker )
		worker_array = (Worker*) mem_add( MAX_WORKER * sizeof(Worker) );
	else
		worker_array = NULL;

	//----- set the firm's absolute positions on the map -----//

	FirmBuild* firmBuild = firm_res.get_build(firm_build_id);

	race_id = firmBuild->race_id;

   loc_x1 = xLoc;
	loc_y1 = yLoc;
	loc_x2 = loc_x1 + firmBuild->loc_width  - 1;
   loc_y2 = loc_y1 + firmBuild->loc_height - 1;

   center_x = (loc_x1 + loc_x2) / 2;
   center_y = (loc_y1 + loc_y2) / 2;

	region_id = world.get_region_id( center_x, center_y );

	abs_x1 = xLoc * ZOOM_LOC_WIDTH  + firmBuild->min_offset_x;
	abs_y1 = yLoc * ZOOM_LOC_HEIGHT + firmBuild->min_offset_y;
   abs_x2 = abs_x1 + firmBuild->max_bitmap_width  - 1;
   abs_y2 = abs_y1 + firmBuild->max_bitmap_height - 1;

   //--------- set animation frame vars ---------//

   if( firmBuild->animate_full_size )
		cur_frame = 1;
   else
   {
      cur_frame = 2;                // start with the 2nd frame as the 1st frame is the common frame
      err_when( firmBuild->frame_count <=2 );   // for segmented animation, the minimum no. of frames must be 3, as the first one is the common frame
   }

   remain_frame_delay = (char) firmBuild->frame_delay(cur_frame);

   //--------- initialize gaming vars ----------//

   hit_points     = (float) 0;
   max_hit_points = firmInfo->max_hit_points;

	//------ set construction and builder -------//

	under_construction = firmInfo->buildable;       // whether the firm is under construction, if the firm is not buildable it is completed in the first place

	if( !under_construction )			// if this firm doesn't been to be constructed, set its hit points to the maximum
		hit_points = max_hit_points;

	if( builderRecno )
		set_builder(builderRecno);
	else
		builder_recno = 0;

	//------ update firm counter -------//

   firmInfo->total_firm_count++;
	
	if( nation_recno )
		firmInfo->inc_nation_firm_count(nation_recno);

   //-------------------------------------------//

   if( nation_recno > 0 )
   {
		Nation* nationPtr = nation_array[nation_recno];

      firm_ai = nationPtr->is_ai();
		ai_processed = 1;

      //--------- increase firm counter -----------//

      nationPtr->nation_firm_count++;

      //-------- update last build date ------------//

      nationPtr->last_build_firm_date = info.game_date;
   }
   else
   {
      firm_ai = 0;
      ai_processed = 0;
   }

   ai_status = FIRM_WITHOUT_ACTION;
   ai_link_checked = 1;       // check the connected firms if ai_link_checked = 0;

   //--------------------------------------------//

	setup_link();

	set_world_matrix();

	init_name();

	//----------- init AI -----------//

	if( firm_ai )
		nation_array[nation_recno]->add_firm_info(firm_id, firm_recno);

	//-------- init derived ---------//

	init_derived();         // init_derived() before set_world_matrix() so that init_derived has access to the original land info.
}
//----------- End of function Firm::init ---------//


//--------- Begin of function Firm::deinit --------//
//
void Firm::deinit()
{
	if( !firm_recno )    // already deleted
      return;

	deinit_derived();

	remove_firm = 1; // set static parameter

	//------- delete AI info ----------//

	if(firm_ai)
	{
		Nation* nationPtr = nation_array[nation_recno];

		if( should_close_flag )
			nationPtr->firm_should_close_array[firm_id-1]--;

		err_when( nationPtr->firm_should_close_array[firm_id-1] < 0 );

		nationPtr->del_firm_info(firm_id, firm_recno);
	}

	//--------- clean up related stuff -----------//

	restore_world_matrix();
	release_link();

	//------ all workers and the overseer resign ------//

	if( !sys.signal_exit_flag )
	{
		// ##### begin Gilbert 28/10 ########//
		if( !under_construction )
		{
			// -------- create a firm die record ------//
			// can be called as soon as restore_world_matrix
			FirmDie firmDie;
			firmDie.init(this);
			firm_die_array.add(&firmDie);
		}
		// ##### end Gilbert 28/10 ########//

		assign_overseer(0);     // this function must be called before restore_world_matrix(), otherwise the power area can't be completely reset

		if( worker_array )
		{
			resign_all_worker(); // the workers in the firm will be killed if there is no space for creating the workers
			mem_del( worker_array );
			worker_array = NULL;
		}

		if(builder_recno)
			mobilize_builder(builder_recno);
	}
	else
	{
		if(builder_recno)
			kill_builder(builder_recno);

		kill_overseer();

		if(worker_array)
		{
			kill_all_worker();
			mem_del(worker_array);
			worker_array = NULL;
		}
	}

   //--------- decrease firm counter -----------//

   if( nation_recno )
      nation_array[nation_recno]->nation_firm_count--;

	//------ update firm counter -------//

	FirmInfo* firmInfo = firm_res[firm_id];

	firmInfo->total_firm_count--;

	if( nation_recno )
		firmInfo->dec_nation_firm_count(nation_recno);

   //------- update town border ---------//

   loc_x1 = -1;      // mark deleted

   //------- if the current firm is the selected -----//

   if( firm_array.selected_recno == firm_recno )
   {
		firm_array.selected_recno = 0;
      info.disp();
   }

   //-------------------------------------------------//

   firm_recno = 0;
	remove_firm = 0;	// reset static parameter
}
//----------- End of function Firm::deinit ---------//


//--------- Begin of function Firm::init_name --------//
//
// Set the name of this firm. Name related vars are set.
//
void Firm::init_name()
{
	char t=firm_res[firm_id]->short_name[0];

	if( t==' ' || !t )		// if this firm does not have any short name, display the full name without displaying the town name together
		return;

	//---- find the closest town and set closest_town_name_id -----//

	closest_town_name_id = get_closest_town_name_id();

	//--------- set firm_name_instance_id -----------//

	char  usedInstanceArray[256];
	Firm* firmPtr;

	memset( usedInstanceArray, 0, sizeof(usedInstanceArray) );

	int i;
	for( i=firm_array.size() ; i>0 ; i-- )
	{
		if( firm_array.is_deleted(i) )
			continue;

		firmPtr = firm_array[i];

		if( firmPtr->firm_id == firm_id &&
			 firmPtr->closest_town_name_id == closest_town_name_id &&
			 firmPtr->firm_name_instance_id )
		{
			usedInstanceArray[firmPtr->firm_name_instance_id-1] = 1;
		}
	}

	for( i=0 ; i<256 ; i++ )		// get the smallest id. which are not used by existing firms
	{
		if( !usedInstanceArray[i] )
		{
			firm_name_instance_id = i+1;
			break;
		}
	}
}
//--------- End of function Firm::init_name --------//


//------- Begin of function Firm::get_closest_town_name_id -----------//
//
// return the name id. of the closest town.
//
int Firm::get_closest_town_name_id()
{
	//---- find the closest town and set closest_town_name_id -----//

	int 	townDistance, minTownDistance=0x7FFF;
	int   closestTownNameId=0;
	Town* townPtr;

	for( int i=town_array.size() ; i>0 ; i-- )
	{
		if( town_array.is_deleted(i) )
			continue;

		townPtr = town_array[i];

		townDistance = m.points_distance( townPtr->center_x, townPtr->center_y,
							center_x, center_y );

		if( townDistance < minTownDistance )
		{
			minTownDistance = townDistance;
			closestTownNameId = townPtr->town_name_id;
		}
	}

	return closestTownNameId;
}
//--------- End of function Firm::get_closest_town_name_id -----------//


//------- Begin of function Firm::firm_name -----------//
//
char* Firm::firm_name()
{
	static String str;

	if( !closest_town_name_id )
	{
		str = firm_res[firm_id]->name;
	}
	else
	{
#if(defined(SPANISH))
		str  = firm_res[firm_id]->short_name;
		str += " de ";
		str += town_res.get_name(closest_town_name_id);
#else
		// FRENCH, GERMAN and US
		str  = town_res.get_name(closest_town_name_id);
		str += " ";
		str += firm_res[firm_id]->short_name;
#endif

		if( firm_name_instance_id > 1 )		// don't display number for the first firm
		{
			str += " ";
			str += firm_name_instance_id;
		}
	}

	return str;
}
//--------- End of function Firm::firm_name -----------//


//------- Begin of function Firm::complete_construction -----------//
//
// Complete construction instantly.
//
void Firm::complete_construction()
{
	if( under_construction )
   {
      hit_points = max_hit_points;
      under_construction = 0;
   }
}
//--------- End of function Firm::complete_construction -----------//


//------- Begin of function Firm::assign_unit -----------//
//
void Firm::assign_unit(int unitRecno)
{
	err_when( !unitRecno );

	Unit* unitPtr  = unit_array[unitRecno];

	//------- if this is a construction worker -------//

	if( unitPtr->skill.skill_id == SKILL_CONSTRUCTION )
	{
		set_builder(unitRecno);
		return;
	}

	//---- if the unit does not belong to the firm's nation ----//

	if( unitPtr->nation_recno != nation_recno )
	{
   	// can no longer capture a firm with a normal unit - must use spy  

		//----- capture this firm if there is nobody here -----//
/*
		if( worker_array && worker_count==0 && overseer_recno==0 &&		// if the firm is empty, assign to take over the firm
			 unitPtr->skill.skill_id == firm_skill_id )						// the takeover unit must have the skill of this firm
		{
			change_nation(unitPtr->nation_recno);
		}
		else
*/			return;			// if cannot capture, the nations are not the same, return now. This will happen if the unit's nation was changed during his moving to the firm.
	}

	//-- if there isn't any overseer in this firm or this unit's skill is higher than the current overseer's skill --//

	//### begin alex 18/10 ###//
	unitPtr->group_select_id = 0; // clear group select id
	//#### end alex 18/10 ####//

	FirmInfo* firmInfo = firm_res[firm_id];

	if( firmInfo->need_overseer &&
		 ( !overseer_recno ||
			( unitPtr->skill.skill_id == firm_skill_id &&
			  unit_array[overseer_recno]->skill.skill_id != firm_skill_id ) ||     // the current overseer does not have the required skill
			( unitPtr->skill.skill_id == firm_skill_id &&
			  unitPtr->skill.skill_level > unit_array[overseer_recno]->skill.skill_level )
		 ) )
	{
		assign_overseer(unitRecno);
	}
	else if( firmInfo->need_worker )
	{
		assign_worker(unitRecno);
	}
}
//--------- End of function Firm::assign_unit -----------//


//--------- Begin of function Firm::assign_overseer --------//
//
// Assign an unit as the overseer of this firm
//
// <int> newOverseerRecno - recno of the new overseer unit.
//									 0 means resign the current overseer
//
// Note: If a new overseer is assigned to the firm, there should be
//			space for the old overseer to initialize and appear in the
//			map (the space the new overseer occupied).
//
//			** If the newOverseerRecno==0, there may be no space for
//			creating the old overseer.  Then, the old overseer will be
//			deleted. **
//			** else there must be space for creating the old overseer,
//			at least the space occupied by the new overseer **
//
void Firm::assign_overseer(int newOverseerRecno)
{
   if( !firm_res[firm_id]->need_overseer )
		return;

	if( !newOverseerRecno && !overseer_recno )
		return;

	//--- if the new overseer's nation is not the same as the firm's nation, don't assign ---//

	if( newOverseerRecno && unit_array[newOverseerRecno]->nation_recno != nation_recno )
		return;

	//------------------------------------------//

	int oldOverseerRecno = overseer_recno;

	if(!newOverseerRecno)
	{
		//------------------------------------------------------------------------------------------------//
		// the old overseer may be kept in firm or killed if remove_firm is true
		//------------------------------------------------------------------------------------------------//
		err_when(!overseer_recno);
		Unit *oldUnitPtr = unit_array[overseer_recno];
		SpriteInfo *spriteInfo = sprite_res[unit_res[oldUnitPtr->unit_id]->sprite_id];
		int xLoc = loc_x1;
		int yLoc = loc_y1;

		if(!locate_space(remove_firm, xLoc, yLoc, loc_x2, loc_y2, spriteInfo->loc_width, spriteInfo->loc_height))
		{
			if(remove_firm)
				kill_overseer();
		}
		else
		{
			//------ there should be space for creating the overseer -----//

			mobilize_overseer();
/*
			//-- if the overseer is resigned without successor, mobilize a worker as overseer --//
			if(!newOverseerRecno && worker_array)
			{
				int bestWorkerId = best_worker_id();      // find the most skilled worker
				if( bestWorkerId )
					newOverseerRecno = mobilize_worker(bestWorkerId,1);
			}
*/
		}
	}
	else
	{
		//----------- there should be space for creating the overseer ---------//
		err_when(!newOverseerRecno);

		Unit *unitPtr = unit_array[newOverseerRecno];

		int originalXLoc = unitPtr->next_x_loc();
		int originalYLoc = unitPtr->next_y_loc();

		err_when( unitPtr->hit_points <= 0 );

		unitPtr->deinit_sprite();

      //----------------------------------------------------------------------------------------//
		// There should be at least one location (occupied by the new overseer) for creating the old
		//	overseer.
		//
		// 1) If a town is already created, the new overseer settle down there, free its space for
		//		creating the new overseer.
		// 2) If the overseer and the workers live in the firm, no town will be created.  Thus, the
		//		space occupied by the old overseer is free for creating the new overseer.
		// 3) If the overseer and the workers need live in town, and a town is created.  i.e. there
		//		is no overseer or worker in the firm, so just assign the new overseer in the firm
		//----------------------------------------------------------------------------------------//
		
		if(!overseer_recno && !worker_count)
		{
			//------------------------------------------------------------------------------------------------//
			// the firm is empty
			//------------------------------------------------------------------------------------------------//
			if(firm_res[firm_id]->live_in_town)
			{
				overseer_town_recno = assign_settle(unitPtr->race_id, unitPtr->loyalty, 1); // the overseer settles down
				if(!overseer_town_recno)
					return; // no space for creating the town, just return without assigning
			}

			//------- set the unit to overseer mode and deinit the sprite ------//
			overseer_recno = newOverseerRecno;
			Unit *unitPtr = unit_array[overseer_recno];
			unitPtr->set_mode(UNIT_MODE_OVERSEE, firm_recno);
			unitPtr->deinit_sprite();     // hide the unit from the world map

			//--------- if the unit is a spy -----------//

			if( unitPtr->spy_recno )
				spy_array[unitPtr->spy_recno]->set_place( SPY_FIRM, firm_recno );
/*
			//------ capture the firm if the overseer is from another nation ---//
			if(unit_array[overseer_recno]->nation_recno != nation_recno)
				change_nation(unit_array[overseer_recno]->nation_recno);
*/
		}
		else
		{
			//------------------------------------------------------------------------------------------------//
			// a town should exist if the overseer need live in town
			//------------------------------------------------------------------------------------------------//
			if(firm_res[firm_id]->live_in_town)
			{
				overseer_town_recno = assign_settle(unitPtr->race_id, unitPtr->loyalty, 1); // the overseer settles down

				if(!overseer_town_recno)
					return; // reach MAX population and no space to create town, return without assigning
			}

			Unit *unitPtr = unit_array[newOverseerRecno];
			unitPtr->deinit_sprite();

			if(overseer_recno)
				mobilize_overseer();

			overseer_recno = newOverseerRecno;
			unitPtr->set_mode(UNIT_MODE_OVERSEE, firm_recno);

			//--------- if the unit is a spy -----------//

			if( unitPtr->spy_recno )
				spy_array[unitPtr->spy_recno]->set_place( SPY_FIRM, firm_recno );
/*
			//------ capture the firm if the overseer is from another nation ---//
			if(unit_array[overseer_recno]->nation_recno != nation_recno)
				change_nation(unit_array[overseer_recno]->nation_recno);
*/
		}
	}

	//------- update loyalty -------//

	if( newOverseerRecno && !unit_array.is_deleted(newOverseerRecno) )
		unit_array[newOverseerRecno]->update_loyalty();

	//----------- refresh display if this firm is selected ----------//

	if(firm_array.selected_recno == firm_recno)
		info.disp();
}
//----------- End of function Firm::assign_overseer --------//


//--------- Begin of function Firm::mobilize_overseer --------//
//
int Firm::mobilize_overseer()
{
	if( !overseer_recno )
		return 0;

	//--------- restore overseer's harmony ---------//

	int overseerRecno = overseer_recno;

	Unit* unitPtr = unit_array[overseer_recno];

	//-------- if the overseer is a spy -------//

	if( unitPtr->spy_recno )
		spy_array[unitPtr->spy_recno]->set_place(SPY_MOBILE, unitPtr->sprite_recno);

	//---- cancel the overseer's presence in the town -----//

	if( firm_res[firm_id]->live_in_town )
		town_array[overseer_town_recno]->dec_pop(unitPtr->race_id, 1);

	//----- get this overseer out of the firm -----//

	SpriteInfo* spriteInfo = sprite_res[unit_res[unitPtr->unit_id]->sprite_id];
	int         xLoc=loc_x1, yLoc=loc_y1;     // xLoc & yLoc are used for returning results

	int spaceFound = locate_space(remove_firm, xLoc, yLoc, loc_x2, loc_y2, spriteInfo->loc_width, spriteInfo->loc_height);

	if(spaceFound)
	{
		unitPtr->init_sprite(xLoc, yLoc);
		unitPtr->set_mode(0);        // reset overseen firm recno
	}
	else
	{
		unit_array.del(overseer_recno);		// delete it when there is no space for the unit
		return 0;
	}

	//--------- reset overseer_recno -------------//

	overseer_recno      = 0;
	overseer_town_recno = 0;

	//------- update loyalty -------//

	if( overseerRecno && !unit_array.is_deleted(overseerRecno) )
		unit_array[overseerRecno]->update_loyalty();

	return overseerRecno;
}
//----------- End of function Firm::mobilize_overseer --------//


//--------- Begin of function Firm::mobilize_builder --------//
int Firm::mobilize_builder(short recno)
{
	//----------- mobilize the builder -------------//
	Unit* unitPtr = unit_array[recno];

	SpriteInfo *spriteInfo = unitPtr->sprite_info;
	int xLoc=loc_x1, yLoc=loc_y1;

	if(!locate_space(remove_firm, xLoc, yLoc, loc_x2, loc_y2, spriteInfo->loc_width, spriteInfo->loc_height, UNIT_LAND, builder_region_id) &&
		!world.locate_space(xLoc, yLoc, loc_x2, loc_y2, spriteInfo->loc_width, spriteInfo->loc_height, UNIT_LAND, builder_region_id))
	{
		kill_builder(recno);
		return 0;
	}

	unitPtr->init_sprite(xLoc, yLoc);
	unitPtr->stop2();  // clear all previously defined action

	err_when(unitPtr->unit_mode != UNIT_MODE_CONSTRUCT);

	unitPtr->set_mode(0);
	return 1;
}
//----------- End of function Firm::mobilize_builder --------//


//--------- Begin of function Firm::best_worker_id --------//
//
int Firm::best_worker_id()
{
   int bestWorkerId=0, maxWorkerSkill=0;
   char rankId;
   int  liveInTown = firm_res[firm_id]->live_in_town;

   err_when( !worker_array );    // this function shouldn't be called if this firm does not need worker

   for( int i=0 ; i<worker_count ; i++ )
   {
      //--- if the town the worker lives and the firm are of the same nation ---//

      if( !liveInTown || town_array[ worker_array[i].town_recno ]->nation_recno == nation_recno )
      {
         if(firm_id==FIRM_CAMP)
			{
            rankId = worker_array[i].rank_id;
            if(rankId!=RANK_GENERAL && rankId!=RANK_KING)
               continue;
         }

         if( worker_array[i].skill_level > maxWorkerSkill )
         {
            maxWorkerSkill = worker_array[i].skill_level;
            bestWorkerId   = i+1;
         }
      }
   }

   return bestWorkerId;
}
//----------- End of function Firm::best_worker_id --------//


//--------- Begin of function Firm::free_worker_room --------//
//
// Resign the worst worker from the firm to free up a room for
// a new worker.
//
void Firm::free_worker_room()
{
   err_when( !worker_array );    // this function shouldn't be called if this firm does not need worker

   //---- if there is space for one more worker, demote the overseer to worker ----//

   if( worker_count < MAX_WORKER || worker_count==0 )
		return;

	//---- if all worker space are full, resign the worst worker to release one worker space for the overseer ----//

	int worestWorkerId=0, minWorkerSkill=0x7FFF;

   for( int i=0 ; i<MAX_WORKER ; i++ )
   {
      if( worker_array[i].skill_level < minWorkerSkill )
      {
			minWorkerSkill = worker_array[i].skill_level;
			worestWorkerId = i+1;
      }
   }

	if( worestWorkerId )
		resign_worker(worestWorkerId);

   err_here();
}
//----------- End of function Firm::free_worker_room --------//


//--------- Begin of function Firm::assign_worker --------//
//
// Assign an unit as one of the workers of this firm
//
void Firm::assign_worker(int workerUnitRecno)
{
	err_when( !worker_array );    // this function shouldn't be called if this firm does not need worker

	//-- if the unit is a spy, only allow assign when there is room in the firm --//

	Unit* unitPtr = unit_array[workerUnitRecno];

	if( unitPtr->true_nation_recno() != nation_recno &&
		 worker_count == MAX_WORKER )
	{
		return;
	}

	//---- if all worker space are full, resign the worst worker to release one worker space for the overseer ----//

	err_when( unitPtr->rank_id == RANK_KING );
	err_when( unitPtr->hit_points <= 0 );

	int unitXLoc= -1, unitYLoc;

	if( worker_count == MAX_WORKER )
	{
		int worstWorkerId=0, minWorkerSkill=0x7FFF, workerSkill;

		for(int i=0; i<MAX_WORKER; i++)
		{
			workerSkill = worker_array[i].skill_level;

			if( workerSkill < minWorkerSkill)
			{
				minWorkerSkill = workerSkill;
				worstWorkerId = i+1;
			}
		}

		err_when(worstWorkerId<1 || worstWorkerId>MAX_WORKER);

		unitXLoc = unitPtr->next_x_loc();		// save the location for later init_sprite() if the assign settle action failed
		unitYLoc = unitPtr->next_y_loc();

		unitPtr->deinit_sprite(); // free the location for creating the worst unit

		#ifdef DEBUG
			int oldWorkerCount = worker_count;
			int resignResult = resign_worker(worstWorkerId);
			err_when(!resignResult && oldWorkerCount==worker_count);
		#else
			resign_worker(worstWorkerId);
		#endif
	}

	// err_when( worker_count >= MAX_WORKER );

	//---------- there is room for the new worker ------------//

	Worker* workerPtr = worker_array + worker_count;

	memset( workerPtr, 0, sizeof(Worker) );

	if( firm_res[firm_id]->live_in_town )
	{
		workerPtr->town_recno = assign_settle(unitPtr->race_id, unitPtr->loyalty, 0);     // the worker settles down

		if( !workerPtr->town_recno )
		{
			//--- the unit was deinit_sprite(), and now the assign settle action failed, we need to init_sprite() to restore it ---//

			if( unitXLoc>=0 && !unitPtr->is_visible() )
				unitPtr->init_sprite(unitXLoc, unitYLoc);

			return;
		}
	}
	else
	{
		workerPtr->town_recno = 0;
		workerPtr->worker_loyalty = unitPtr->loyalty;
	}

	//------- add the worker to the firm -------//

	worker_count++;

	err_when( worker_count > MAX_WORKER );

	workerPtr->name_id		= unitPtr->name_id;
	workerPtr->race_id      = unitPtr->race_id;
	workerPtr->unit_id		= unitPtr->unit_id;
	workerPtr->rank_id      = unitPtr->rank_id;

	workerPtr->skill_id = firm_skill_id;
	workerPtr->skill_level = unitPtr->skill.get_skill(firm_skill_id);

	if( workerPtr->skill_level == 0 )
		workerPtr->skill_level = CITIZEN_SKILL_LEVEL;

	err_when( workerPtr->skill_level<0 );
	err_when( workerPtr->skill_level>100 );

	/*#ifdef DEBUG2
		if(unit_res[unitPtr->unit_id]->unit_class==UNIT_CLASS_HUMAN)
		{
			unitPtr->skill.combat_level = 60;
			unitPtr->hit_points = unitPtr->skill.combat_level*2;
			unitPtr->max_hit_points = unitPtr->hit_points;
		}
	#endif*/

	workerPtr->combat_level = unitPtr->skill.combat_level;
	workerPtr->hit_points   = (int) unitPtr->hit_points;

	err_when( workerPtr->combat_level <= 0 || workerPtr->combat_level > 100 );

	err_when( workerPtr->hit_points < 0 );

	if( workerPtr->hit_points == 0 )		// 0.? will become 0 in (float) to (int) conversion
		workerPtr->hit_points = 1;

	if( unit_res[unitPtr->unit_id]->unit_class == UNIT_CLASS_WEAPON )
	{
		workerPtr->extra_para = unitPtr->get_weapon_version();
	}
	else if( unitPtr->race_id )
	{
		workerPtr->extra_para = unitPtr->cur_power;
	}
	else
	{
		workerPtr->extra_para = 0;
	}

	workerPtr->init_potential();

	//------ if the recruited worker is a spy -----//

	if( unitPtr->spy_recno )
	{
		spy_array[unitPtr->spy_recno]->set_place( SPY_FIRM, firm_recno );

		workerPtr->spy_recno = unitPtr->spy_recno;
		unitPtr->spy_recno = 0;								// reset it now so Unit::deinit() won't delete the Spy in spy_array
	}

	//--------- the unit disappear in firm -----//

	if( !firm_res[firm_id]->live_in_town )		// if the unit does not live in town, increase the unit count now
		unit_res[unitPtr->unit_id]->inc_nation_unit_count(nation_recno);

	unit_array.disappear_in_firm(workerUnitRecno);
}
//----------- End of function Firm::assign_worker --------//


//--------- Begin of function Firm::assign_settle --------//
//
// The newly assigned overseer / worker settles down.
//
// <int> raceId      - race id. of the unit
// <int> unitLoyalty - loyalty of the unit
// <int> isOverseer  - whether the unit is an overseer, if not,
//                     it is then a worker.
//
// return: <int> townRecno - the home town of the overseer/worker
//                           0 - no space to settle.
//
int Firm::assign_settle(int raceId, int unitLoyalty, int isOverseer)
{
	err_when( !firm_res[firm_id]->live_in_town );
	err_when( unitLoyalty < 0 || unitLoyalty > 100 );

   //--- if there is a town of our nation within the effective distance ---//

   int townRecno = find_settle_town();

   if( townRecno )
   {
      town_array[townRecno]->inc_pop(raceId, 1, unitLoyalty);
      return townRecno;
   }

   //--- should create a town near the this firm, if there is no other town in the map ---//

   int xLoc=loc_x1, yLoc=loc_y1;    // xLoc & yLoc are used for returning results

	if( world.locate_space( xLoc, yLoc, loc_x2, loc_y2, STD_TOWN_LOC_WIDTH,
									STD_TOWN_LOC_HEIGHT, UNIT_LAND, region_id, 1 ) )		// the town must be in the same region as this firm.
   {
      if( m.points_distance( center_x, center_y, xLoc+(STD_TOWN_LOC_WIDTH-1)/2,
          yLoc+(STD_TOWN_LOC_HEIGHT-1)/2 ) <= EFFECTIVE_FIRM_TOWN_DISTANCE )
      {
			int townRecno = town_array.add_town( nation_recno, raceId, xLoc, yLoc );

         Town* townPtr = town_array[townRecno];

			townPtr->init_pop( raceId, 1, unitLoyalty, 1 );    // 1st 1 - population, 2nd 1 - the unit has a job already

         townPtr->auto_set_layout();

         return townRecno;
      }
   }

   //---- not able to find a space for a new town within the effective distance ----//

   return 0;
}
//----------- End of function Firm::assign_settle --------//


//--------- Begin of function Firm::find_settle_town --------//
//
// Find a suitable town for the unit to settle.
//
int Firm::find_settle_town()
{
	int 		townDistance, minDistance=0x7FFF, nearestTownRecno=0;
	Town* 	townPtr;
	Nation* 	nationPtr = nation_array[nation_recno];

	//-------- scan for our own town first -----------//

	for( int i=0 ; i<linked_town_count ; i++ )
	{
		townPtr = town_array[ linked_town_array[i] ];

		if( townPtr->population>=MAX_TOWN_POPULATION )
			continue;

		if( townPtr->nation_recno != nation_recno )
			continue;

		townDistance = m.points_distance( townPtr->center_x, townPtr->center_y, center_x, center_y );

		if( townDistance < minDistance )
		{
			minDistance = townDistance;
			nearestTownRecno = townPtr->town_recno;
		}
	}

	if( nearestTownRecno )
		return nearestTownRecno;
	else
		return 0;
}
//----------- End of function Firm::find_settle_town --------//


//--------- Begin of function Firm::set_world_matrix --------//
//
// Set the cargo id of current firm int he world matrix
//
void Firm::set_world_matrix()
{
	//--- if a nation set up a firm in a location that the player has explored, contact between the nation and the player is established ---//

	int xLoc, yLoc;

	for( yLoc=loc_y1 ; yLoc<=loc_y2 ; yLoc++ )
	{
		for( xLoc=loc_x1 ; xLoc<=loc_x2 ; xLoc++ )
		{
			world.get_loc(xLoc, yLoc)->set_firm(firm_recno);
		}
	}

	//--- if a nation set up a town in a location that the player has explored, contact between the nation and the player is established ---//

	establish_contact_with_player();

	//------------ reveal new land ----------//

	if( nation_recno == nation_array.player_recno ||
		 (nation_recno && nation_array[nation_recno]->is_allied_with_player) )
	{
		world.unveil( loc_x1, loc_y1, loc_x2, loc_y2 );
		world.visit( loc_x1, loc_y1, loc_x2, loc_y2, EXPLORE_RANGE-1 );
	}

	//-------- set should_set_power --------//

	should_set_power = get_should_set_power();

	//---- set this town's influence on the map ----//

	if( should_set_power )
		world.set_power(loc_x1, loc_y1, loc_x2, loc_y2, nation_recno);

	//---- if the newly built firm is visual in the zoom window, redraw the zoom buffer ----//

	if( is_in_zoom_win() )
		sys.zoom_need_redraw = 1;  // set the flag on so it will be redrawn in the next frame
}
//----------- End of function Firm::set_world_matrix --------//


//--------- Begin of function Firm::get_should_set_power --------//
//
int Firm::get_should_set_power()
{
	int shouldSetPower = 1;

	if( firm_id == FIRM_HARBOR )		// don't set power for harbors
	{
		shouldSetPower = 0;
	}
	else if( firm_id == FIRM_MARKET )
	{
		//--- don't set power for a market if it's linked to another nation's town ---//

		Town *townPtr;

		shouldSetPower = 0;

		//--- only set the shouldSetPower to 1 if the market is linked to a firm of ours ---//

		for( int i=0 ; i<linked_town_count ; i++ )
		{
			townPtr = town_array[ linked_town_array[i] ];

			if( townPtr->nation_recno == nation_recno )
			{
				shouldSetPower = 1;
				break;
			}
		}
	}

	return shouldSetPower;
}
//----------- End of function Firm::get_should_set_power --------//


//------- Begin of function Firm::establish_contact_with_player --------//
//
// See if the town's location is an explored area, establish contact
// with the player.
//
void Firm::establish_contact_with_player()
{
	if( !nation_recno )
		return;

	int xLoc, yLoc;
	Location* locPtr;

	for( yLoc=loc_y1 ; yLoc<=loc_y2 ; yLoc++ )
	{
		for( xLoc=loc_x1 ; xLoc<=loc_x2 ; xLoc++ )
		{
			locPtr = world.get_loc(xLoc, yLoc);

			locPtr->set_firm(firm_recno);

			if( locPtr->explored() && nation_array.player_recno )
			{
				NationRelation *relation = (~nation_array)->get_relation(nation_recno);

#ifdef USE_DPLAY
				if( !remote.is_enable() )
				{
#endif
					relation->has_contact = 1;
#ifdef USE_DPLAY
				}
				else
				{
					if( !relation->has_contact && !relation->contact_msg_flag )
					{
						// packet structure : <player nation> <explored nation>
						short *shortPtr = (short *)remote.new_send_queue_msg(MSG_NATION_CONTACT, 2*sizeof(short));
						*shortPtr = nation_array.player_recno;
						shortPtr[1] = nation_recno;
						relation->contact_msg_flag = 1;
					}
				}
#endif
			}
		}
	}
}
//-------- End of function Firm::establish_contact_with_player --------//


//--------- Begin of function Firm::restore_world_matrix --------//
//
// When the firm is destroyed, restore the original land id
//
void Firm::restore_world_matrix()
{
   int xLoc, yLoc;

   for( yLoc=loc_y1 ; yLoc<=loc_y2 ; yLoc++ )
   {
      for( xLoc=loc_x1 ; xLoc<=loc_x2 ; xLoc++ )
      {
         err_when( world.get_loc(xLoc,yLoc)->firm_recno() != firm_recno );

         world.get_loc(xLoc,yLoc)->remove_firm();
      }
	}

	//---- restore this town's influence on the map ----//

	if( should_set_power ) 			// no power region for harbor as it build on coast which cannot be set with power region
		world.restore_power(loc_x1, loc_y1, loc_x2, loc_y2, 0, firm_recno);

   //---- if the newly built firm is visual in the zoom window, redraw the zoom buffer ----//

   if( is_in_zoom_win() )
      sys.zoom_need_redraw = 1;
}
//----------- End of function Firm::restore_world_matrix --------//


//---------- Begin of function Firm::own_firm --------//
//
int Firm::own_firm()
{
   return nation_recno == nation_array.player_recno;
}
//----------- End of function Firm::own_firm ---------//


//---------- Begin of function Firm::process_animation --------//
//
void Firm::process_animation()
{
   //-------- process animation ----------//

   FirmBuild* firmBuild  = firm_res.get_build(firm_build_id);
   int        frameCount = firmBuild->frame_count;

   if( frameCount==1 )     // no animation for this firm
      return;

   //---------- next frame -----------//

   if( --remain_frame_delay==0 )    // if it is in the delay between frames
   {
      remain_frame_delay = (char) firmBuild->frame_delay(cur_frame);

      if( ++cur_frame > frameCount )
      {
         if( firmBuild->animate_full_size )
            cur_frame = 1;
         else
         {
				cur_frame = 2;                // start with the 2nd frame as the 1st frame is the common frame
            err_when( frameCount <=2 );   // for segmented animation, the minimum no. of frames must be 3, as the first one is the common frame
         }
      }
   }
}
//---------- End of function Firm::process_animation --------//


//---------- Begin of function Firm::process_construction --------//
//
void Firm::process_construction()
{
	err_when(firm_id!=FIRM_MONSTER && builder_recno<=0);
	if(firm_id==FIRM_MONSTER)
	{
		//--------- process construction for monster firm ----------//
		hit_points++;

		#ifdef DEBUG
		if( config.fast_build && nation_recno==nation_array.player_recno )
			hit_points += 10;
		#endif

		if(hit_points>=max_hit_points)
		{
			hit_points = max_hit_points;
			under_construction = 0;
		}
		return;
	}

	err_when(firm_id==FIRM_MONSTER);

	if( !under_construction )
		return;

	//--- can only do construction when the firm is not under attack ---//

	if( info.game_date <= last_attacked_date+1 )
		return;

	if( sys.frame_count%2!=0 )		// one build every 2 frames
		return;

	//------ increase the construction progress ------//

	Unit *unitPtr = unit_array[builder_recno];

	if( unitPtr->skill.skill_id == SKILL_CONSTRUCTION )	// if builder unit has construction skill
		hit_points += 1+unitPtr->skill.skill_level/30;
	else
		hit_points++;

	if( config.fast_build && nation_recno==nation_array.player_recno )
		hit_points += 10;

	//----- increase skill level of the builder unit -----//

	if( unitPtr->skill.skill_id == SKILL_CONSTRUCTION )	// if builder unit has construction skill
	{
		if( ++unitPtr->skill.skill_level_minor > 100 )
		{
			unitPtr->skill.skill_level_minor = 0;

			if( unitPtr->skill.skill_level < 100 )
				unitPtr->skill.skill_level++;
		}
	}

	//------- when the construction is complete ----------//

	if( hit_points >= max_hit_points )     // finished construction
	{
		hit_points = max_hit_points;

		int needAssignUnit=0;

		under_construction = 0;

		// ##### begin Gilbert 10/10 #######//
		if( nation_recno == nation_array.player_recno )
			se_res.far_sound(center_x, center_y, 1, 'S', unitPtr->sprite_id,
				"FINS", 'F',  firm_id);
		// ##### end Gilbert 10/10 #######//

		err_when(builder_recno<=0 || unit_array.is_deleted(builder_recno));
		err_when(unitPtr->nation_recno!=nation_recno);

		FirmInfo* firmInfo=firm_res[firm_id];

		if( (firmInfo->need_overseer || firmInfo->need_worker) &&
			 (firmInfo->firm_skill_id==0 || firmInfo->firm_skill_id == (unitPtr->skill).skill_id) )   // the builder with the skill required
		{
			unitPtr->set_mode(0);		// reset it from UNIT_MODE_CONSTRUCT

			needAssignUnit=1;
		}
		else
		{
			set_builder(0);
		}

		//---------------------------------------------------------------------------------------//
		// should call assign_unit() first before calling action_finished(...UNDER_CONSTRUCTION)
		//---------------------------------------------------------------------------------------//

		if( needAssignUnit )
		{
			assign_unit(builder_recno);
			//------------------------------------------------------------------------------//
			// Note: there may be chance the unit cannot be assigned into the firm
			//------------------------------------------------------------------------------//
			if(!worker_count && !overseer_recno) // no assignment, can't assign
			{
				//------- init_sprite or delete the builder ---------//
				int xLoc=loc_x1, yLoc=loc_y1;     // xLoc & yLoc are used for returning results
				SpriteInfo *spriteInfo = unitPtr->sprite_info;
				if(!locate_space(remove_firm, xLoc, yLoc, loc_x2, loc_y2, spriteInfo->loc_width, spriteInfo->loc_height))
					unit_array.disappear_in_firm(builder_recno); // kill the unit
				else
					unitPtr->init_sprite(xLoc, yLoc); // restore the unit
			}
		}

		// ##### begin Gilbert 10/10 #######//
		//if( nation_recno == nation_array.player_recno )
		//	se_res.far_sound(center_x, center_y, 1, 'S', unitPtr->sprite_id,
		//		"FINS", 'F',  firm_id);
		// ##### end Gilbert 10/10 #######//

		builder_recno = 0;
	}

	err_when (hit_points < 0 || hit_points > max_hit_points );
}
//---------- End of function Firm::process_construction --------//


//---------- Begin of function Firm::set_builder --------//
//
// <short> newBuilderRecno - >0 the recno of the new builder unit.
//									   0 just remove the existing builder, do not assign new one.
//
// return: <int> 0-the old builder is killed
//					  1-the builder is changed successfully.
//
int Firm::set_builder(short newBuilderRecno)
{
	err_when( under_construction && newBuilderRecno==0 );		// can't remove the construction worker when the firm is under construction

	//------------------------------------//

	short oldBuilderRecno = builder_recno; // store the old builder recno

	builder_recno = newBuilderRecno;

	//-------- assign the new builder ---------//

	if(builder_recno)
	{
		Unit* unitPtr = unit_array[builder_recno];
		//### begin alex 18/10 ###//
		unitPtr->group_select_id = 0; // clear group select id
		//#### end alex 18/10 ####//
		if(unitPtr->is_visible())	 // is visible if the unit is not inside the firm location
		{
			builder_region_id = world.get_region_id( unitPtr->cur_x_loc(), unitPtr->cur_y_loc() );
			unitPtr->deinit_sprite();

			if( unitPtr->selected_flag )
			{
				unitPtr->selected_flag = 0;
				unit_array.selected_count--;
			}
		}

		err_when( unitPtr->unit_mode != 0 );
		unitPtr->set_mode( UNIT_MODE_CONSTRUCT, firm_recno );
	}

	if(oldBuilderRecno)
		mobilize_builder(oldBuilderRecno);

	return 1;
}
//---------- End of function Firm::set_builder --------//


//---------- Begin of function Firm::next_day --------//
//
void Firm::next_day()
{
	if( !nation_recno )
		return;

	//------ think about updating link status -------//
	//
	// This part must be done here instead of in
	// process_ai() because it will be too late to do
	// it in process_ai() as the next_day() will call
	// first and some wrong goods may be input to markets.
	//
	//-----------------------------------------------//

	if( firm_ai )
	{
		if( info.game_date%30==firm_recno%30 || !ai_link_checked )	// once 30 days or when the link has been changed.
		{
			ai_update_link_status();
			ai_link_checked = 1;
		}
	}

	//-------- pay expenses ----------//

	pay_expense();

	//------- update loyalty --------//

	if( info.game_date%30 == firm_recno%30 )
		update_loyalty();

	//-------- consume food --------//

	if( !firm_res[firm_id]->live_in_town && worker_count>0 )
		consume_food();

	//------ think worker migration -------//

	if( worker_array && info.game_date%30 == firm_recno%30 )
		think_worker_migrate();

	//--------- repairing ----------//

	process_repair();

	//------ catching spies -------//

	if( info.game_date%30 == firm_recno%30 )
		spy_array.catch_spy(SPY_FIRM, firm_recno);

	//----- process workers from other town -----//

	if( firm_res[firm_id]->live_in_town )
	{
		process_independent_town_worker();
	}

	//--- recheck no_neighbor_space after a period, there may be new space available now ---//

	if( no_neighbor_space && info.game_date%180 == firm_recno%180 )
	{
		short buildXLoc, buildYLoc;

		if( nation_array[nation_recno]->find_best_firm_loc(FIRM_INN, loc_x1, loc_y1, buildXLoc, buildYLoc) )		// whether it's FIRM_INN or not really doesn't matter, just any firm type will do
			no_neighbor_space = 0;
	}

	//-------- debug code ---------//

#ifdef DEBUG
	err_when( builder_recno && unit_array.is_deleted(builder_recno) );
		
	if( worker_array )
	{
		for( int i=0 ; i<worker_count ; i++ )
			err_when( worker_array[i].hit_points <= 0 );
	}

	if( overseer_recno )
	{
		err_when( unit_array[overseer_recno]->rank_id == RANK_SOLDIER );
	}
#endif
}
//----------- End of function Firm::next_day ---------//


//---------- Begin of function Firm::next_month --------//
//
void Firm::next_month()
{
	//------ update nation power recno ------//

	int newShouldSetPower = get_should_set_power();

	if( newShouldSetPower == should_set_power )
		return;

	if( should_set_power )
		world.restore_power(loc_x1, loc_y1, loc_x2, loc_y2, 0, firm_recno);

	should_set_power = newShouldSetPower;

	if( should_set_power )
		world.set_power(loc_x1, loc_y1, loc_x2, loc_y2, nation_recno);
}
//----------- End of function Firm::next_month ---------//


//---------- Begin of function Firm::next_year --------//
//
void Firm::next_year()
{
	//------- post income data --------//

	last_year_income = cur_year_income;
	cur_year_income  = (float) 0;
}
//----------- End of function Firm::next_year ---------//


//---------- Begin of function Firm::update_loyalty --------//
//
void Firm::update_loyalty()
{
	if( firm_res[firm_id]->live_in_town )		// only for those who do not live in town
		return;

	//----- update loyalty of the soldiers -----//

	Worker* workerPtr = worker_array;

	for( int i=0 ; i<worker_count ; i++, workerPtr++ )
	{
		int targetLoyalty = workerPtr->target_loyalty(firm_recno);

		if( targetLoyalty > workerPtr->worker_loyalty )
		{
			int incValue = (targetLoyalty - workerPtr->worker_loyalty)/10;

			int newLoyalty = (int) workerPtr->worker_loyalty + MAX(1, incValue);

			if( newLoyalty > targetLoyalty )
				newLoyalty = targetLoyalty;

			workerPtr->worker_loyalty = newLoyalty;
		}
		else if( targetLoyalty < workerPtr->worker_loyalty )
		{
			workerPtr->worker_loyalty--;
		}
	}
}
//----------- End of function Firm::update_loyalty ---------//


//---------- Begin of function Firm::process_repair --------//
//
void Firm::process_repair()
{
	if( nation_array[nation_recno]->cash < 0 )		// if you don't have cash, the repair workers will not work
		return;

	if( !builder_recno )
		return;

	Unit *unitPtr = unit_array[builder_recno];

	//--- can only do construction when the firm is not under attack ---//

	if( info.game_date <= last_attacked_date+1 )
	{
		//---- if the construction worker is a spy, it will damage the building when the building is under attack ----//

		if( unitPtr->spy_recno &&
			 unitPtr->true_nation_recno() != nation_recno )
		{
			hit_points -= (float) spy_array[unitPtr->spy_recno]->spy_skill / 30;

			if( hit_points < 0 )
				hit_points = (float) 0;
		}

		return;
	}

	//------- repair now - only process once every 3 days -----//

	if( hit_points >= max_hit_points )
		return;

	err_when( unitPtr->skill.skill_id != SKILL_CONSTRUCTION );

	int dayInterval = (100-unitPtr->skill.skill_level)/20+1;			// repair once every 1 to 6 days, depending on the skill level of the construction worker

	if( firm_recno % dayInterval == info.game_date % dayInterval )
	{
		hit_points++;

		if( hit_points > max_hit_points )
			hit_points = max_hit_points;
	}
}
//----------- End of function Firm::process_repair ---------//


//---------- Begin of function Firm::pay_expense --------//
//
void Firm::pay_expense()
{
	if( !nation_recno )
		return;

   Nation* nationPtr = nation_array[nation_recno];

	//-------- fixed expenses ---------//

	float dayExpense = (float) firm_res[firm_id]->year_cost / 365;

	if( nationPtr->cash >= dayExpense )
	{
		nationPtr->add_expense( EXPENSE_FIRM, dayExpense, 1 );
	}
	else
	{
		if( hit_points > 0 )
			hit_points--;

		if( hit_points < 0 )
			hit_points = (float) 0;

		//--- when the hit points drop to zero and the firm is destroyed ---//

		if( hit_points==0 && nation_recno == nation_array.player_recno )
			news_array.firm_worn_out(firm_recno);
	}

	//----- paying salary to workers from other nations -----//

	if( worker_array && firm_res[firm_id]->live_in_town )
	{
		int     townNationRecno, payWorkerCount=0;
		Worker* workerPtr;

		for( int i=worker_count-1 ; i>=0 ; i-- )
		{
			workerPtr = worker_array+i;

			townNationRecno = town_array[workerPtr->town_recno]->nation_recno;

			if( townNationRecno != nation_recno )
			{
				//--- if we don't have cash to pay the foreign workers, resign them ---//

				if( nationPtr->cash < 0 )
				{
					resign_worker(i+1);
				}
				else  //----- pay salaries to the foreign workers now -----//
				{
					payWorkerCount++;

					if( townNationRecno )      // the nation of the worker will get income
						nation_array[townNationRecno]->add_income( INCOME_FOREIGN_WORKER, (float) WORKER_YEAR_SALARY / 365, 1 );
				}
			}
		}

		nationPtr->add_expense( EXPENSE_FOREIGN_WORKER, (float) WORKER_YEAR_SALARY * payWorkerCount / 365, 1 );
	}
}
//----------- End of function Firm::pay_expense ---------//


//--------- Begin of function Firm::consume_food ---------//
//
void Firm::consume_food()
{
	if( nation_array[nation_recno]->food > 0 )
	{
		int humanUnitCount=0;

		for( int i=0 ; i<worker_count ; i++ )
		{
			if( worker_array[i].race_id )
				humanUnitCount++;
		}

		nation_array[nation_recno]->consume_food((float) humanUnitCount * PERSON_FOOD_YEAR_CONSUMPTION / 365);
	}
	else	//--- decrease loyalty if the food has been run out ---//
	{
		if( info.game_date%NO_FOOD_LOYALTY_DECREASE_INTERVAL == 0 )		// decrease 1 loyalty point every 2 days
		{
			for( int i=0 ; i<worker_count ; i++ )
			{
				if( worker_array[i].race_id )
					worker_array[i].change_loyalty(-1);
			}
		}
	}
}
//----------- End of function Firm::consume_food -----------//


//---------- Begin of function Firm::add_income --------//
//
void Firm::add_income(int incomeType, float incomeAmt)
{
   cur_year_income += incomeAmt;

	nation_array[nation_recno]->add_income(incomeType, incomeAmt, 1);
}
//----------- End of function Firm::add_income ---------//


//--------- Begin of function Firm::year_expense ---------//
//
// Return the yearly expense for this firm.
//
int Firm::year_expense()
{
	int totalExpense = firm_res[firm_id]->year_cost;

	//---- pay salary to workers from foreign towns ----//

	int payWorkerCount=0;

	if( worker_array && firm_res[firm_id]->live_in_town )
	{
		int     payWorkerCount=0;
		Worker* workerPtr = worker_array;

		for( int i=0 ; i<worker_count ; i++, workerPtr++ )
		{
			if( town_array[workerPtr->town_recno]->nation_recno != nation_recno )
				payWorkerCount++;
		}

		totalExpense += WORKER_YEAR_SALARY * payWorkerCount;
	}

	return totalExpense;
}
//----------- End of function Firm::year_expense -----------//


//--------- Begin of function Firm::sell_firm ---------//

void Firm::sell_firm(char remoteAction)
{
#ifdef USE_DPLAY
   if( !remoteAction && remote.is_enable() )
   {
      // packet structure : <firm recno>
      short *shortPtr = (short *)remote.new_send_queue_msg(MSG_FIRM_SELL, sizeof(short));
      *shortPtr = firm_recno;
      return;
   }
#endif
   //------- sell at 50% of the original cost -------//

	Nation* nationPtr = nation_array[nation_recno];

	int sellIncome = firm_res[firm_id]->setup_cost / 2 * (int) hit_points / (int) max_hit_points;

	nationPtr->add_income(INCOME_SELL_FIRM, (float)sellIncome);

	se_res.sound(center_x, center_y, 1, 'F', firm_id, "SELL" );

   firm_array.del_firm(firm_recno);
}
//----------- End of function Firm::sell_firm -----------//


//--------- Begin of function Firm::destruct_firm ---------//

void Firm::destruct_firm(char remoteAction)
{
#ifdef USE_DPLAY
	if( !remoteAction && remote.is_enable() )
	{
		// packet structure : <firm recno>
		short *shortPtr = (short *)remote.new_send_queue_msg(MSG_FIRM_DESTRUCT, sizeof(short));
		*shortPtr = firm_recno;
		return;
	}
#endif

	se_res.sound(center_x, center_y, 1, 'F', firm_id, "DEST" );
	
	firm_array.del_firm(firm_recno);
}
//----------- End of function Firm::destruct_firm -----------//


//--------- Begin of function Firm::cancel_construction ---------//
//
// Cancel construction
//
void Firm::cancel_construction(char remoteAction)
{
#ifdef USE_DPLAY
	if( !remoteAction && remote.is_enable())
	{
		short *shortPtr = (short *)remote.new_send_queue_msg(MSG_FIRM_CANCEL, sizeof(short));
		shortPtr[0] = firm_recno;
		return;
	}
#endif
	//------ get half of the construction cost back -------//

	Nation* nationPtr = nation_array[nation_recno];

	nationPtr->add_expense( EXPENSE_FIRM, (float) -firm_res[firm_id]->setup_cost/2 );

   firm_array.del_firm(firm_recno);
}
//----------- End of function Firm::cancel_construction -----------//


//---------- Begin of function Firm::recruit_worker --------//
//
void Firm::recruit_worker()
{
	if( MAX_WORKER==worker_count )
      return;

   if( info.game_date%5 != firm_recno%5 )    // update population once 10 days
      return;

	err_when( worker_count > MAX_WORKER );

   //-------- pull from neighbor towns --------//

   int       i;
	Town*     townPtr;
   Nation*   nationPtr = nation_array[nation_recno];

	for( i=0 ; i<linked_town_count ; i++ )
	{
		if( linked_town_enable_array[i] != LINK_EE )
			continue;

      townPtr = town_array[linked_town_array[i]];

		//--- don't hire foreign workers if we don't have cash to pay them ---//

		if( nationPtr->cash < 0 && nation_recno != townPtr->nation_recno )
			continue;

		//-------- if the town has any unit ready for jobs -------//

		if( townPtr->jobless_population == 0 )
			continue;

		//---- if nation of the town is not hositle to this firm's nation ---//

		if( pull_town_people(townPtr->town_recno, COMMAND_AUTO) )
			return;
	}
}
//----------- End of function Firm::recruit_worker ---------//


//---------- Begin of function Firm::pull_town_people --------//
//
// Pull people from the town. Also called by Town::draw_detect_link_line()
//
// <int> townRecno - the town recno which the people are pulled from.
// [int] raceId	 - the race of the people to be pulled.
//                   if not given, pick one randomly.
// [int] forcePull - force pull people to to the firm.
//							(default: 0)
//
int Firm::pull_town_people(int townRecno, char remoteAction, int raceId, int forcePull)
{
	if( worker_count == MAX_WORKER )		// this can happen in a multiplayer game as Town::draw_detect_link_line() still have the old worker_count and thus allow this function being called.
		return 0;

	err_when( worker_count > MAX_WORKER );
	err_when( !worker_array );    // this function shouldn't be called if this firm does not need worker

#ifdef USE_DPLAY
	if(!remoteAction && remote.is_enable() )
	{
		// packet structure : <firm recno> <town recno> <race Id or 0> <force Pull>
		short *shortPtr = (short *)remote.new_send_queue_msg(MSG_FIRM_PULL_TOWN_PEOPLE, 4*sizeof(short));
		shortPtr[0] = firm_recno;
		shortPtr[1] = townRecno;
		shortPtr[2] = raceId;	
		// if raceId == 0, let each player choose the race by random number,
		// to sychronize the random number
		shortPtr[3] = forcePull;
		return 0;
	}
#endif

	//---- people in the town go to work for the firm ---//

	Town* townPtr = town_array[townRecno];
	int   i, popAdded=0;

	//---- if doesn't specific a race, randomly pick one ----//

	if( !raceId )
		raceId = m.random(MAX_RACE)+1;

	//----------- scan the races -----------//

	for( i=0 ; i<MAX_RACE ; i++ )    // maximum 8 tries
	{
		//---- see if there is any population of this race to move to the firm ----//

		int recruitableCount = townPtr->recruitable_race_pop(raceId,1);		// 1-allow recruiting spies

		if( recruitableCount > 0 )
		{
			//----- if the unit is forced to move to the firm ---//

			if( forcePull )			// right-click to force pulling a worker from the village
			{
				if( townPtr->race_loyalty_array[raceId-1] < MIN_RECRUIT_LOYALTY )
					return 0;

				townPtr->recruit_dec_loyalty(raceId);
			}
			else //--- see if the unit will voluntarily move to the firm ---//
			{
				//--- the higher the loyalty is, the higher the chance of working for the firm ---//

				if( townPtr->nation_recno )
				{
					if( m.random( (100-(int)townPtr->race_loyalty_array[raceId-1])/10 ) > 0 )
						return 0;
				}
				else
				{
					if( m.random( (100-(int)townPtr->race_resistance_array[raceId-1][nation_recno-1])/10 ) > 0 )
						return 0;
				}
			}

			//----- get the chance of getting people to your command base is higher when the loyalty is higher ----//

			if( firm_res[firm_id]->live_in_town )
			{
				townPtr->jobless_race_pop_array[raceId-1]--;    // decrease the town's population
				townPtr->jobless_population--;

				err_when( townPtr->recruitable_race_pop(raceId,1) < 0 );
				err_when( townPtr->jobless_population < 0 );
			}
			else
			{
				townPtr->dec_pop(raceId, 0);
			}

			//------- add the worker to the firm -----//

			worker_count++;

			err_when( worker_count > MAX_WORKER );

			Worker* workerPtr = worker_array + worker_count - 1;

			memset( workerPtr, 0, sizeof(Worker) );

			workerPtr->race_id = raceId;
			workerPtr->rank_id = RANK_SOLDIER;
			workerPtr->unit_id = (char) race_res[raceId]->basic_unit_id;
			workerPtr->worker_loyalty = (char) townPtr->race_loyalty_array[raceId-1];

			if( firm_res[firm_id]->live_in_town )
				workerPtr->town_recno = townRecno;

			workerPtr->combat_level = CITIZEN_COMBAT_LEVEL;
			workerPtr->hit_points   = CITIZEN_HIT_POINTS;

			workerPtr->skill_id     = firm_skill_id;
			workerPtr->skill_level  = CITIZEN_SKILL_LEVEL;

			workerPtr->init_potential();

			//--------- if this is a military camp ---------//
			//
			// Increase armed unit count of the race of the worker assigned,
			// as when a unit is assigned to a camp, Unit::deinit() will decrease
			// the counter, so we need to increase it back here.
			//
			//---------------------------------------------------//

			if( !firm_res[firm_id]->live_in_town )
				unit_res[workerPtr->unit_id]->inc_nation_unit_count(nation_recno);

			//------ if the recruited worker is a spy -----//

			int spyCount = townPtr->race_spy_count_array[raceId-1];

			if( spyCount >= m.random(recruitableCount)+1 )	
			{
				int spyRecno = spy_array.find_town_spy(townRecno, raceId, m.random(spyCount)+1 );		// the 3rd parameter is which spy to recruit

				err_when( !spyRecno );

				workerPtr->spy_recno = spyRecno;

				spy_array[spyRecno]->set_place(SPY_FIRM, firm_recno);
			}

			return 1;
		}

		if( ++raceId > MAX_RACE )
			raceId = 1;
	}

	return 0;
}
//----------- End of function Firm::pull_town_people ---------//


//------ Begin of function Firm::process_independent_town_worker -----//
//
// Process workers from independent towns.
//
// When workers work for a foreign firm, the overall resistance of
// the worker's town towards that nation decreases.
//
void Firm::process_independent_town_worker()
{
	if( firm_recno%15 != info.game_date%15 )
		return;

	#define RESISTANCE_DECREASE_PER_WORKER	 float(0.2)		// resistance decrease per month every 15 days

	Town* townPtr;

	for( int i=0 ; i<worker_count ; i++ )
	{
		err_when( !worker_array[i].town_recno );

		townPtr = town_array[ worker_array[i].town_recno ];

		if( townPtr->nation_recno==0 )		// if it's an independent town
		{
			townPtr->race_resistance_array[worker_array[i].race_id-1][nation_recno-1] -= RESISTANCE_DECREASE_PER_WORKER;

			if( townPtr->race_resistance_array[worker_array[i].race_id-1][nation_recno-1] < 0 )
				townPtr->race_resistance_array[worker_array[i].race_id-1][nation_recno-1] = (float) 0;
		}
	}
}
//------- End of function Firm::process_independent_town_worker ------//


//---------- Begin of function Worker::init_potential --------//
//
void Worker::init_potential()
{
	if( m.random(10)==0 )		// 1 out of 10 has a higher than normal potential in this skill
	{
		skill_potential = 50+m.random(51);	 // 50 to 100 potential
	}
}
//----------- End of function Worker::init_potential ---------//


//---------- Begin of function Firm::calc_productivity --------//
//
// Calculate the productivity of the firm.
//
void Firm::calc_productivity()
{
   err_when( !worker_array );    // this function shouldn't be called if this firm does not need worker

   #define  RACE_SKILL_MULTIPLE     (float)2.0

   productivity = (float) 0;

   //------- calculate the productivity of the workers -----------//

   int      i;
	float    totalSkill=(float)0;
	Worker*  workerPtr = worker_array;

	for( i=0 ; i<worker_count ; i++, workerPtr++ )
	{
		totalSkill += (int) workerPtr->skill_level 
						  * workerPtr->hit_points / workerPtr->max_hit_points();
	}

   //----- include skill in the calculation ------//

	productivity = totalSkill / MAX_WORKER - sabotage_level;

	if( productivity < 0 )
		productivity = (float) 0;
}
//----------- End of function Firm::calc_productivity ---------//


//---------- Begin of function Firm::average_worker_skill --------//
//
// Return the average skill level of the workers in this firm.
//
int Firm::average_worker_skill()
{
	err_when( !worker_array );    // this function shouldn't be called if this firm does not need worker

	if( worker_count==0 )
      return 0;

	//------- calculate the productivity of the workers -----------//

   int      i;
	int      totalSkill = 0;
   Worker*  workerPtr = worker_array;

	for( i=0 ; i<worker_count ; i++, workerPtr++ )
      totalSkill += workerPtr->skill_level;

   //----- include skill in the calculation ------//

	return totalSkill / worker_count;
}
//----------- End of function Firm::average_worker_skill ---------//


//---------- Begin of function Firm::update_worker --------//
//
void Firm::update_worker()
{
   err_when( !worker_array );    // this function shouldn't be called if this firm does not need worker

	if( info.game_date%15 != firm_recno%15 )
      return;

	if( worker_count==0 )
      return;

   //------- update the worker's para ---------//

   int     incValue, levelMinor;
   Worker* workerPtr = worker_array;

	for( int i=0 ; i<worker_count ; i++, workerPtr++ )
	{
		//------- increase worker skill -----------//

		if( is_operating() && workerPtr->skill_level < 100 )		// only train when the workers are working
		{
			err_when(workerPtr->skill_level<0 || workerPtr->skill_level>100);

			incValue = MAX(10, 100-workerPtr->skill_level)
						  * workerPtr->hit_points / workerPtr->max_hit_points()
						  * (100+workerPtr->skill_potential) / 100 / 2;

			//-------- increase level minor now --------//

			levelMinor = workerPtr->skill_level_minor + incValue * (75+m.random(50)) / 100;     // with random factors, resulting in 75% to 125% of the original

			int loopCount=0;

			while( levelMinor >= 100 )
			{
				levelMinor -= 100;
				workerPtr->skill_level++;

				err_when( loopCount++ > 1000 );
			}

			workerPtr->skill_level_minor = levelMinor;
		}

		//------- increase worker hit points --------//

		int maxHitPoints = workerPtr->max_hit_points();

		err_when( maxHitPoints <= 0 );

		if( workerPtr->hit_points < maxHitPoints )
		{
			workerPtr->hit_points += 2;	// units in firms recover twice as fast as they are mobile

			if( workerPtr->hit_points > maxHitPoints )
				workerPtr->hit_points = maxHitPoints;
		}
	}
}
//----------- End of function Firm::update_worker ---------//


//---------- Begin of function Firm::create_unit --------//
//
// Create an unit and place it below the firm.
//
// <int> unitId        - id. of the unit
// [int] townRecno     - recno of the town from which the unit comes from
//                       if given, it means the unit comes from the town and
//                       should decrease the town population.
//                       (default: 0)
// [int] unitHasJob    - whether the unit current has a job or not
//                       (default: 0)
//
// return : <int> unitRecno - the recno of the unit created
//
int Firm::create_unit(int unitId, int townRecno, int unitHasJob)
{
	//----look for an empty locatino for the unit to stand ----//
   //--- scan for the 5 rows right below the building ---//

   SpriteInfo* spriteInfo = sprite_res[unit_res[unitId]->sprite_id];
   int         xLoc=loc_x1, yLoc=loc_y1;     // xLoc & yLoc are used for returning results

	if(!locate_space(remove_firm, xLoc, yLoc, loc_x2, loc_y2, spriteInfo->loc_width, spriteInfo->loc_height))
		return 0;

	//------------ add the unit now ----------------//

	int unitNationRecno;

	if( townRecno )
		unitNationRecno = town_array[townRecno]->nation_recno;
	else
		unitNationRecno = nation_recno;

	int unitRecno = unit_array.add_unit( unitId, unitNationRecno, RANK_SOLDIER, 0, xLoc, yLoc );

	//----- update the population of the town ------//

	if( townRecno )
		town_array[townRecno]->dec_pop(unit_array[unitRecno]->race_id, unitHasJob);

	return unitRecno;
}
//----------- End of function Firm::create_unit ---------//


//--------- Begin of function Firm::mobilize_worker ---------//
//
// Promote a firm worker as a unit.
//
// return: <int> the recno of the unit created.
//
int Firm::mobilize_worker(int workerId, char remoteAction)
{
#ifdef USE_DPLAY
	if(!remoteAction && remote.is_enable() )
	{
		// packet strcture : <firm_recno> <workerId>
		short *shortPtr = (short *)remote.new_send_queue_msg(MSG_FIRM_MOBL_WORKER, 2*sizeof(short) );
		shortPtr[0] = firm_recno;
		shortPtr[1] = workerId;
		return 0;
	}
#endif

	err_when( !worker_array );    // this function shouldn't be called if this firm does not need worker

   err_when( workerId<1 || workerId>worker_count );

	//------------- resign worker --------------//

	Worker thisWorker = worker_array[workerId-1];

	int oldWorkerCount = worker_count;

	int unitRecno2 = resign_worker(workerId);

	if(!unitRecno2 && worker_count==oldWorkerCount)
		return 0;

	//------ create a mobile unit -------//

	int unitRecno=0;

	if( firm_res[firm_id]->live_in_town )			// if does not live_in_town, resign_worker() create the unit already, so don't create it again here.
	{
		unitRecno = create_worker_unit(thisWorker);

		if( !unitRecno )		// no space for creating units
			return 0;
	}

	//------------------------------------//

	err_when( unitRecno2 && unitRecno );		// only one of them should have value
	err_when( !unitRecno2 && !unitRecno );		// one of them must have a value

	if( unitRecno )
		return unitRecno;
	else
		return unitRecno2;
}
//----------- End of function Firm::mobilize_worker -----------//


//--------- Begin of function Firm::create_worker_unit ---------//
//
int Firm::create_worker_unit(Worker& thisWorker)
{
	//--------- copy the worker's info --------//

	int unitLoyalty = thisWorker.loyalty();

	//------------ create an unit --------------//

	int unitId = thisWorker.unit_id;
	int unitRecno = create_unit( unitId, thisWorker.town_recno, 0 );      // this worker no longer has a job as it has been resigned

	if( !unitRecno )
		return 0;

	Unit* unitPtr   = unit_array[unitRecno];
	UnitInfo *unitInfo = unit_res[unitId];

	//------- set the unit's parameters --------//

	unitPtr->skill.skill_id           = thisWorker.skill_id;
	unitPtr->skill.skill_level        = thisWorker.skill_level;
	unitPtr->skill.skill_level_minor  = thisWorker.skill_level_minor;

	err_when( unitPtr->skill.skill_level<0 || unitPtr->skill.skill_level>100 );

	unitPtr->set_combat_level(thisWorker.combat_level);
	unitPtr->skill.combat_level_minor = thisWorker.combat_level_minor;

	err_when( unitPtr->skill.combat_level<=0 || unitPtr->skill.combat_level>100 );

	unitPtr->loyalty    = unitLoyalty;
	unitPtr->hit_points = thisWorker.hit_points;
	unitPtr->rank_id    = thisWorker.rank_id;

	if( unit_res[unitPtr->unit_id]->unit_class == UNIT_CLASS_WEAPON )
	{
		unitPtr->set_weapon_version( thisWorker.extra_para );	// restore nation contribution
	}
	else if( unitPtr->race_id )
	{
		unitPtr->cur_power = thisWorker.extra_para;

		if( unitPtr->cur_power < 0 )
			unitPtr->cur_power = 0;

		if( unitPtr->cur_power > 150 )
			unitPtr->cur_power = 150;
	}

	err_when( unitPtr->hit_points <= 0 );

	unitPtr->fix_attack_info();

	//if( unitInfo->unit_class == UNIT_CLASS_WEAPON )
	//{
	//	switch( unitId )
	//	{
	//		case UNIT_BALLISTA:
	//			unitPtr->attack_count = 2;
	//			break;
	//		case UNIT_EXPLOSIVE_CART:
	//			unitPtr->attack_count = 0;
	//			break;
	//		default:
	//			unitPtr->attack_count = 1;
	//}
//		if( unitPtr->attack_count > 0)
//		{
//			unitPtr->attack_info_array = unit_res.attack_info_array
//				+ unitInfo->first_attack-1
//				+ (thisWorker.extra_para -1) * unitPtr->attack_count;		// extra para keeps the weapon version
//	}
//		else
//		{
//			// no attack like explosive cart
//			unitPtr->attack_info_array = NULL;
//		}
//	}

	if( thisWorker.name_id && thisWorker.race_id )		// if this worker is formerly an unit who has a name
		unitPtr->set_name(thisWorker.name_id);

	err_when( !unitPtr->is_visible() );

	//------ if the unit is a spy -------//

	if( thisWorker.spy_recno )
	{
		Spy* spyPtr = spy_array[thisWorker.spy_recno];

		unitPtr->spy_recno = thisWorker.spy_recno;
		unitPtr->ai_unit   = spyPtr->cloaked_nation_recno &&
									nation_array[spyPtr->cloaked_nation_recno]->is_ai();

		unitPtr->set_name(spyPtr->name_id);		// set the name id. of this unit

		spyPtr->set_place(SPY_MOBILE, unitRecno);
	}

	//--- decrease the nation unit count as the Unit has already increased it ----//

	if( !firm_res[firm_id]->live_in_town )		// if the unit does not live in town, increase the unit count now
		unit_res[unitPtr->unit_id]->dec_nation_unit_count(nation_recno);

	return unitRecno;
}
//----------- End of function Firm::create_worker_unit -----------//


//--------- Begin of function Firm::mobilize_all_worker ---------//
//
// mobilize as many as workers if there is space for creating the
// workers
//
// [int] leaderUnitRecno - if given, the workers are assigned as
//									a team and their leader_unit_recno are set.
//									(default: 0)
//
void Firm::mobilize_all_worker(int leaderUnitRecno)
{
	err_when( !worker_array );    // this function shouldn't be called if this firm does not need worker

	//------- detect buttons on hiring firm workers -------//

	int   loopCount = 0;
	short unitRecno;

	err_when( worker_count > MAX_WORKER );

	while( worker_count > 0 )
	{
		err_when(++loopCount > 100);

		unitRecno = mobilize_worker(1, COMMAND_AUTO);        // always record 1 as the workers info are moved forward from the back to the front

		if(!unitRecno)
			break; // keep the rest workers as there is no space for creating the unit

		if( leaderUnitRecno )
		{
			Unit* unitPtr = unit_array[unitRecno];

			unitPtr->team_id = unit_array.cur_team_id;   // define it as a team
			unitPtr->leader_unit_recno = leaderUnitRecno;
			unitPtr->update_loyalty();							// the unit is just assigned to a new leader, set its target loyalty

			err_when( unitPtr->rank_id != RANK_KING && unitPtr->rank_id != RANK_GENERAL );

			if( nation_recno == nation_array.player_recno )
				unitPtr->selected_flag = 1;
      }
	}

   unit_array.cur_team_id++;
}
//----------- End of function Firm::mobilize_all_worker -----------//


//--------- Begin of function Firm::resign_all_worker ---------//
//
// Resign all workers in the firm.
//
// [int] disappearFlag - whether the worker should disappear after
//                       resigning, and does not go back to the town.
//
void Firm::resign_all_worker(int disappearFlag)
{
   err_when( !worker_array );    // this function shouldn't be called if this firm does not need worker

   //------- detect buttons on hiring firm workers -------//

   int loopCount=0, townRecno, raceId;
	int oldWorkerCount;

	while( worker_count > 0 )
	{
		err_when(++loopCount > 100);

		townRecno = worker_array[0].town_recno;
		raceId    = worker_array[0].race_id;

		oldWorkerCount = worker_count;

		if(!resign_worker(1))
		{
			if(oldWorkerCount==worker_count)
				break; // no space to resign the worker, keep them in firm
		}

		if( disappearFlag && townRecno )
			town_array[townRecno]->dec_pop(raceId, 0);
	}
}
//----------- End of function Firm::resign_all_worker -----------//


//--------- Begin of function Firm::resign_worker ---------//
//
// Resign the worker from the firm.
//
// return: <int> recno of the mobile unit created if there is one created. 
//
int Firm::resign_worker(int workerId)
{
	err_when( !worker_array );    // this function shouldn't be called if this firm does not need worker

	err_when( workerId<1 || workerId>worker_count );

	//------- decrease worker no. and create an unit -----//

	Worker* workerPtr = worker_array+workerId-1;
	int	  unitRecno = 0;

	if( workerPtr->race_id && workerPtr->name_id )
		race_res[workerPtr->race_id]->free_name_id(workerPtr->name_id);

	if( workerPtr->town_recno )      // town_recno is 0 if the workers in the firm do not live in towns
	{
		Town* townPtr = town_array[workerPtr->town_recno];

		townPtr->jobless_race_pop_array[workerPtr->race_id-1]++; // decrease the town's population
		townPtr->jobless_population++;

		//------ put the spy in the town -------//

		if( workerPtr->spy_recno )
			spy_array[workerPtr->spy_recno]->set_place(SPY_TOWN, workerPtr->town_recno);
	}
	else
	{
		Worker thisWorker = worker_array[workerId-1];

		unitRecno = create_worker_unit(thisWorker);	 	// if he is a spy, create_worker_unit wil call set_place(SPY_MOBILE)

		if(!unitRecno)
			return 0; // return 0 eg there is no space to create the unit
	}

	//------- delete the record from the worker_array ------//

	err_when( worker_count > MAX_WORKER );
	err_when( selected_worker_id > worker_count );

	m.del_array_rec(worker_array, worker_count, sizeof(Worker), workerId);

	if( selected_worker_id > workerId || selected_worker_id == worker_count )
		selected_worker_id--;

	worker_count--;

	err_when( worker_count < 0 );
	err_when( selected_worker_id > worker_count );

	return unitRecno;
}
//----------- End of function Firm::resign_worker -----------//


//------- Begin of function Firm::think_worker_migrate ---------//
//
// Let the workers think if they want to worker_migrate or not.
//
void Firm::think_worker_migrate()
{
	#define MIN_MIGRATE_ATTRACT_LEVEL	30

	if( worker_count==0 || !firm_res[firm_id]->live_in_town )
		return;

	int   townPtrCount = town_array.size();
	int   townRecno = m.random(townPtrCount)+1;
	int   firmXLoc = center_x, firmYLoc = center_y;
	int   i, j, raceId, workerId;
	Town  *townPtr, *workerTownPtr;
	Worker *workerPtr;

	int curBaseAttractLevel, targetBaseAttractLevel, curAttractLevel, targetAttractLevel;

	for( i=townPtrCount ; i>0 ; i-- )
	{
		if( ++townRecno > townPtrCount )
			townRecno=1;

		if( town_array.is_deleted(townRecno) )
			continue;

		townPtr = town_array[townRecno];

		if(townPtr->population>=MAX_TOWN_POPULATION)
			continue;

		//------ check if this town is linked to the current firm -----//

		for( j=townPtr->linked_firm_count-1 ; j>=0 ; j-- )
		{
			if( townPtr->linked_firm_array[j] == firm_recno && 
				 townPtr->linked_firm_enable_array[j] )
			{
				break;
			}
		}

		if( j<0 )
			continue;

		//------------------------------------------------//
		//
		// Calculate the attractive factor, it is based on:
		//
		// - the reputation of the target nation (+0 to 100)
		// - the racial harmony of the race in the target town (+0 to 100)
		// - the no. of people of the race in the target town
		// - distance between the current town and the target town (-0 to 100)
		//
		// Attractiveness level range: 0 to 200
		//
		//------------------------------------------------//

		targetBaseAttractLevel = 0;

		if( townPtr->nation_recno )
			targetBaseAttractLevel += (int) nation_array[townPtr->nation_recno]->reputation;

		//---- scan all workers, see if any of them want to worker_migrate ----//

		workerId=m.random(worker_count)+1;

		for(j=0 ; j<worker_count ; j++ )
		{
			if( ++workerId > worker_count )
				workerId = 1;

			workerPtr = worker_array+workerId-1;

			if( workerPtr->town_recno == townRecno )
				continue;

			//-- do not migrate if the target town's population of that race is less than half of the population of the current town --//

			raceId = workerPtr->race_id;
			workerTownPtr = town_array[workerPtr->town_recno];

			if( townPtr->race_pop_array[raceId-1] < workerTownPtr->race_pop_array[raceId-1]/2 )
				continue;

			//------ calc the current and target attractiveness level ------//

			workerTownPtr = town_array[workerPtr->town_recno];

			if( workerTownPtr->nation_recno )
				curBaseAttractLevel = (int) nation_array[workerTownPtr->nation_recno]->reputation;
			else
				curBaseAttractLevel = 0;

			targetAttractLevel = targetBaseAttractLevel +
										townPtr->race_harmony(raceId);

			if( targetAttractLevel < MIN_MIGRATE_ATTRACT_LEVEL )
				continue;

			curAttractLevel = curBaseAttractLevel +
									workerTownPtr->race_harmony(raceId) +
									((int)workerPtr->loyalty() - 40);     		 // loyalty > 40 is considered as positive force, < 40 is considered as negative force

			if( targetAttractLevel > curAttractLevel )
			{
				int newLoyalty = MAX( REBEL_LOYALTY+1, targetAttractLevel/2 );

				worker_migrate(workerId, townRecno, newLoyalty);
				return;
			}
		}
	}
}
//-------- End of function Firm::think_worker_migrate -----------//


//------- Begin of function Firm::worker_migrate ---------//
//
// Worker worker_migrate from one town to another.
//
// <int> workerId          - id. of the worker
// <int> destTownRecno - recno of the destination town.
// <int> newLoyalty        - loyalty of the unit in the target town.
//
void Firm::worker_migrate(int workerId, int destTownRecno, int newLoyalty)
{
   err_when( !worker_array );    // this function shouldn't be called if this firm does not need worker

	err_when( !firm_res[firm_id]->live_in_town );

	Worker* workerPtr = worker_array+workerId-1;

	int   raceId		 = workerPtr->race_id;
	Town* srcTown		 = town_array[workerPtr->town_recno];
	Town* destTown		 = town_array[destTownRecno];

   err_when( !raceId );
   err_when( m.points_distance( center_x, center_y, destTown->center_x,
				 destTown->center_y ) > EFFECTIVE_FIRM_TOWN_DISTANCE );

	//------------- add news --------------//

	if( srcTown->nation_recno==nation_array.player_recno ||
		 destTown->nation_recno==nation_array.player_recno )
	{
		if( srcTown->nation_recno != destTown->nation_recno )			// don't add news for migrating between own towns 
			news_array.migrate(srcTown->town_recno, destTownRecno, raceId, 1, firm_recno);
	}

	//--------- migrate now ----------//

	int keepJob = 1;

	workerPtr->town_recno = destTownRecno;

	//--------- decrease the population of the home town ------//

	srcTown->dec_pop(raceId, keepJob);

	//--------- increase the population of the target town ------//

	destTown->inc_pop(raceId, keepJob, newLoyalty);
}
//-------- End of function Firm::worker_migrate -----------//


//-------- Begin of function Firm::set_worker_home_town --------//
//
// This function has two purposes.
//
// If the worker's home town is already the given one,
// then resign the worker.
//
// Otherwise, set the worker's home town to the new onel.
//
// <int> townRecno - the new home town recno
// [int] workerId  - the id. of the worker to be set to a new home town
//							(default: the currently selected worker, selected_worker_id)
//
void Firm::set_worker_home_town(int townRecno, char remoteAction, int workerId)
{
	if( !workerId )
		workerId = selected_worker_id;

	if( !workerId || workerId > worker_count )
		return;

#ifdef USE_DPLAY
	if(!remoteAction && remote.is_enable() )
	{
		// packet structure : <firm recno> <town recno> <workderId>
		short *shortPtr = (short *)remote.new_send_queue_msg(MSG_FIRM_SET_WORKER_HOME, 3*sizeof(short));
		shortPtr[0] = firm_recno;
		shortPtr[1] = townRecno;
		shortPtr[2] = workerId;
		return;
	}
#endif

	err_when( workerId<1 || workerId>worker_count );

	//-------------------------------------------------//

	Worker* workerPtr = worker_array+workerId-1;

   err_when( !workerPtr->race_id );

	if( workerPtr->town_recno == townRecno )
	{
		resign_worker(workerId);
	}

	//--- otherwise, set the worker's home town to the new one ---//

	else if( workerPtr->is_nation(firm_recno, nation_recno) )		// only allow when the worker lives in a town belonging to the same nation
	{
		int workerLoyalty = workerPtr->loyalty();

		town_array[workerPtr->town_recno]->dec_pop(workerPtr->race_id, 1);
		town_array[townRecno]->inc_pop(workerPtr->race_id, 1, workerLoyalty);

		workerPtr->town_recno = townRecno;
	}
}
//-------- End of function Firm::set_worker_home_town --------//


//------- Begin of function Worker::loyalty ---------//
//
int Worker::loyalty()
{
   if( town_recno )     // if the worker lives in a town
      return (int) town_array[town_recno]->race_loyalty_array[race_id-1];
   else
      return (int) worker_loyalty;
}
//-------- End of function Worker::loyalty -----------//


//------- Begin of function Worker::target_loyalty ---------//
//
int Worker::target_loyalty(int firmRecno)
{
	if( town_recno )     // if the worker lives in a town
	{
		return (int) town_array[town_recno]->race_loyalty_array[race_id-1];
	}
	else
	{
		Firm* firmPtr = firm_array[firmRecno];

		if( firmPtr->overseer_recno )
		{
			Unit* overseerUnit = unit_array[firmPtr->overseer_recno];

			int overseerSkill = overseerUnit->skill.get_skill(SKILL_LEADING);
			int targetLoyalty = 30 + overseerSkill/2;

			//---------------------------------------------------//
			//
			// Soldiers with higher combat and leadership skill
			// will get discontented if they are led by a general
			// with low leadership.
			//
			//---------------------------------------------------//

			targetLoyalty -= combat_level/2;

			if( skill_level > overseerSkill )
				targetLoyalty -= skill_level - overseerSkill;

			if( overseerUnit->rank_id == RANK_KING )
				targetLoyalty += 20;

			if( race_res.is_same_race(race_id, overseerUnit->race_id) )
				targetLoyalty += 20;

			if( targetLoyalty < 0 )
				targetLoyalty = 0;

			if( targetLoyalty > 100 )
				targetLoyalty = 100;

			return targetLoyalty;
		}
		else	//-- if there is no overseer, just return the current loyalty --//
		{
			return worker_loyalty;
		}
	}
}
//-------- End of function Worker::target_loyalty -----------//


//------- Begin of function Firm::setup_link ---------//
//
void Firm::setup_link()
{
   //-----------------------------------------------------------------------------//
	// check the connected firms location and structure if ai_link_checked is true
	//-----------------------------------------------------------------------------//

	if(firm_ai)
		ai_link_checked = 0;

	//----- build firm-to-firm link relationship -------//

	int   firmRecno, defaultLinkStatus;
	Firm* firmPtr;
	FirmInfo* firmInfo = firm_res[firm_id];

	linked_firm_count = 0;

	for( firmRecno=firm_array.size() ; firmRecno>0 ; firmRecno-- )
	{
		if( firm_array.is_deleted(firmRecno) || firmRecno==firm_recno )
			continue;

		firmPtr = firm_array[firmRecno];

		//---- do not allow links between firms of different nation ----//

		if( firmPtr->nation_recno != nation_recno )
			continue;

		//---------- check if the firm is close enough to this firm -------//

		if( m.points_distance( firmPtr->center_x, firmPtr->center_y,
			 center_x, center_y ) > EFFECTIVE_FIRM_FIRM_DISTANCE )
		{
			continue;
		}

		//------ check if both are on the same terrain type ------//

		if( world.get_loc(firmPtr->center_x, firmPtr->center_y)->is_plateau()
			 != world.get_loc(center_x, center_y)->is_plateau() )
		{
			continue;
		}

		//----- if the firms are linkable to each other -----//

		if( !firmInfo->is_linkable_to_firm(firmPtr->firm_id) )
			continue;

		//------- determine the default link status ------//

		if( firmPtr->nation_recno == nation_recno )   // if the two firms are of the same nation, get the default link status which is based on the types of the firms
			defaultLinkStatus = firmInfo->default_link_status(firmPtr->firm_id);
		else
			defaultLinkStatus = LINK_DD;     // if the two firms are of different nations, default link status is both side disabled

		//-------- add the link now -------//

		if( linked_firm_count < MAX_LINKED_FIRM_FIRM )
		{
			linked_firm_array[linked_firm_count] = firmRecno;
			linked_firm_enable_array[linked_firm_count] = defaultLinkStatus;

			linked_firm_count++;
		}
		else		// we must link it as it is linked both sides, if one side is linked and the other is not, that will cause a bug
		{
			err_here();
		}

		if( firmPtr->linked_firm_count < MAX_LINKED_FIRM_FIRM )
		{
			if( defaultLinkStatus==LINK_ED )    // Reverse the link status for the opposite linker
				defaultLinkStatus=LINK_DE;

			else if( defaultLinkStatus==LINK_DE )
				defaultLinkStatus=LINK_ED;

			firmPtr->linked_firm_array[firmPtr->linked_firm_count] = firm_recno;
			firmPtr->linked_firm_enable_array[firmPtr->linked_firm_count] = defaultLinkStatus;

			firmPtr->linked_firm_count++;
			if(firmPtr->firm_ai)
				firmPtr->ai_link_checked = 0;

			if(firmPtr->firm_id==FIRM_HARBOR)
			{
				FirmHarbor *harborPtr = (FirmHarbor*) firmPtr;
				harborPtr->link_checked = 0;
			}
      }
		else
		{
			err_here();
		}
   }

   //----- build firm-to-town link relationship -------//

   linked_town_count = 0;

	if( !firmInfo->is_linkable_to_town )
      return;

   int   townRecno;
   Town* townPtr;

   for( townRecno=town_array.size() ; townRecno>0 ; townRecno-- )
   {
      if( town_array.is_deleted(townRecno) )
         continue;

		townPtr = town_array[townRecno];

      //------ check if the town is close enough to this firm -------//

      if( m.points_distance( townPtr->center_x, townPtr->center_y,
			 center_x, center_y ) > EFFECTIVE_FIRM_TOWN_DISTANCE )
      {
         continue;
      }

      //------ check if both are on the same terrain type ------//

      if( (world.get_loc(townPtr->center_x, townPtr->center_y)->is_plateau()==1)
          != (world.get_loc(center_x, center_y)->is_plateau()==1) )
      {
         continue;
		}

		//------- determine the default link status ------//

		if( townPtr->nation_recno == nation_recno )       // if the two firms are of the same nation, get the default link status which is based on the types of the firms
			defaultLinkStatus = LINK_EE;
		else
			defaultLinkStatus = LINK_DD;			  // if the two firms are of different nations, default link status is both side disabled

		//---------------------------------------------------//
		//
		// If this is a camp, it can be linked to the town when
		// either the town is an independent one or the town
		// is not linked to any camps of its own.
		//
		//---------------------------------------------------//

		if( firm_id==FIRM_CAMP )
		{
			if( townPtr->nation_recno==0 || !townPtr->has_linked_own_camp )
				defaultLinkStatus = LINK_EE;
		}

		//-------- add the link now -------//

		if( linked_town_count < MAX_LINKED_FIRM_TOWN )
		{
			linked_town_array[linked_town_count] = townRecno;
			linked_town_enable_array[linked_town_count] = defaultLinkStatus;

			linked_town_count++;
      }
		else
		{
			err_here();
		}

		if( townPtr->linked_firm_count < MAX_LINKED_FIRM_TOWN )
      {
			if( defaultLinkStatus==LINK_ED )    // Reverse the link status for the opposite linker
            defaultLinkStatus=LINK_DE;

         else if( defaultLinkStatus==LINK_DE )
            defaultLinkStatus=LINK_ED;

			townPtr->linked_firm_array[townPtr->linked_firm_count] = firm_recno;
         townPtr->linked_firm_enable_array[townPtr->linked_firm_count] = defaultLinkStatus;

         townPtr->linked_firm_count++;
         if(townPtr->ai_town)
            townPtr->ai_link_checked = 0;
		}
		else
		{
			err_here();
		}
   }
}
//-------- End of function Firm::setup_link -----------//


//------- Begin of function Firm::release_link ---------//
//
void Firm::release_link()
{
   int i;
   Firm *firmPtr;
   Town *townPtr;

   //------ release linked firms ------//

   for( i=0 ; i<linked_firm_count ; i++ )
   {
      firmPtr = firm_array[linked_firm_array[i]];
      firmPtr->release_firm_link(firm_recno);

      if(firmPtr->firm_ai)
         firmPtr->ai_link_checked = 0;
   }

   //------ release linked towns ------//

   for( i=0 ; i<linked_town_count ; i++ )
   {
      townPtr = town_array[linked_town_array[i]];
      townPtr->release_firm_link(firm_recno);

      if(townPtr->ai_town)
         townPtr->ai_link_checked = 0;
   }
}
//-------- End of function Firm::release_link -----------//


//------- Begin of function Firm::release_firm_link ---------//
//
void Firm::release_firm_link(int releaseFirmRecno)
{
   //-----------------------------------------------------------------------------//
   // check the connected firms location and structure if ai_link_checked is true
   //-----------------------------------------------------------------------------//
   if(firm_ai)
      ai_link_checked = 0;

   for( int i=0 ; i<linked_firm_count ; i++ )
   {
      if( linked_firm_array[i] == releaseFirmRecno )
		{
			err_when( linked_firm_count > MAX_LINKED_FIRM_FIRM );

			m.del_array_rec( linked_firm_array, linked_firm_count, sizeof(linked_firm_array[0]), i+1 );
         m.del_array_rec( linked_firm_enable_array, linked_firm_count, sizeof(linked_firm_enable_array[0]), i+1 );
         linked_firm_count--;
         return;
      }
   }

   err_here();
}
//------- End of function Firm::release_firm_link ---------//


//------- Begin of function Firm::release_town_link ---------//
//
void Firm::release_town_link(int releaseTownRecno)
{
   //-----------------------------------------------------------------------------//
   // check the connected firms location and structure if ai_link_checked is true
   //-----------------------------------------------------------------------------//

	if(firm_ai)
		ai_link_checked = 0;

	for( int i=0 ; i<linked_town_count ; i++ )
   {
      if( linked_town_array[i] == releaseTownRecno )
		{
			err_when( linked_town_count > MAX_LINKED_FIRM_TOWN );

			m.del_array_rec( linked_town_array, linked_town_count, sizeof(linked_town_array[0]), i+1 );
			m.del_array_rec( linked_town_enable_array, linked_town_count, sizeof(linked_town_enable_array[0]), i+1 );
         linked_town_count--;
         return;
      }
   }

   err_here();
}
//------- End of function Firm::release_town_link ---------//


//--------- Begin of function Firm::capture_firm --------//
//
// The firm is being captured by another nation.
//
void Firm::capture_firm(int newNationRecno)
{
	if( nation_recno == nation_array.player_recno )
		news_array.firm_captured(firm_recno, newNationRecno, 0);		// 0 - the capturer is not a spy

	//-------- if this is an AI firm --------//

	if( firm_ai )
		ai_firm_captured(newNationRecno);

	//------------------------------------------//
	//
	// If there is an overseer in this firm, then the only
	// unit who can capture this firm will be the overseer only,
	// so calling its betray() function will capture the whole
	// firm already.
	//
	//------------------------------------------//

	if( overseer_recno && unit_array[overseer_recno]->spy_recno )
		unit_array[overseer_recno]->spy_change_nation(newNationRecno, COMMAND_AUTO);
	else
		change_nation(newNationRecno);
}
//--------- End of function Firm::capture_firm --------//


//------- Begin of function Firm::change_nation ---------//
//
void Firm::change_nation(int newNationRecno)
{
	if( nation_recno == newNationRecno )
      return;

	//---------- stop all attack actions to this firm ----------//
	unit_array.stop_attack_firm(firm_recno);
	rebel_array.stop_attack_firm(firm_recno);

	Nation *oldNationPtr = nation_array[nation_recno];
	Nation *newNationPtr = nation_array[newNationRecno];

	//------ if there is a builder in this firm, change its nation also ----//

	if( builder_recno )
	{
		Unit* unitPtr = unit_array[builder_recno];

		unitPtr->change_nation(newNationRecno);

      //--- if this is a spy, chance its cloak ----//

		if( unitPtr->spy_recno )
			spy_array[unitPtr->spy_recno]->cloaked_nation_recno = newNationRecno;
	}

	//---------- stop all actions attacking this firm --------//

	unit_array.stop_attack_firm(firm_recno);

	//------ clear defense mode for military camp -----//

	if(firm_id==FIRM_CAMP)
		((FirmCamp*)this)->clear_defense_mode();

	//---- update nation_unit_count_array[] ----//

	FirmInfo* firmInfo = firm_res[firm_id];

	if( nation_recno )
		firmInfo->dec_nation_firm_count(nation_recno);

	if( newNationRecno )
		firmInfo->inc_nation_firm_count(newNationRecno);

	//---- reset should_close_flag -----//

	if( firm_ai )
	{
		if( should_close_flag )
		{
			oldNationPtr->firm_should_close_array[firm_id-1]--;
			should_close_flag = 0;

			err_when( oldNationPtr->firm_should_close_array[firm_id-1] < 0 );
		}
	}

	//------- update player_spy_count -------//

	spy_array.update_firm_spy_count(firm_recno);

	//--- update the cloaked_nation_recno of all spies in the firm ---//

	spy_array.change_cloaked_nation(SPY_FIRM, firm_recno, nation_recno, newNationRecno);		// check the cloaked nation recno of all spies in the firm

	//-----------------------------------------//

	if(firm_ai)
		oldNationPtr->del_firm_info(firm_id, firm_recno);

	//------ update power nation recno ----------//

	if( should_set_power )
		world.restore_power(loc_x1, loc_y1, loc_x2, loc_y2, 0, firm_recno);

	should_set_power = get_should_set_power();

	if( should_set_power )
		world.set_power(loc_x1, loc_y1, loc_x2, loc_y2, newNationRecno);        // set power of the new nation

	//------------ update link --------------//

	release_link();		// need to update link because firms are only linked to firms of the same nation

	nation_recno = newNationRecno;

	setup_link();

	//---------------------------------------//

	firm_ai = nation_array[nation_recno]->is_ai();

	if(firm_ai)
		newNationPtr->add_firm_info(firm_id, firm_recno);

	//--- if a nation set up a town in a location that the player has explored, contact between the nation and the player is established ---//

	establish_contact_with_player();

	//---- reset the action mode of all spies in this town ----//

	spy_array.set_action_mode( SPY_FIRM, firm_recno, SPY_IDLE );      // we need to reset it. e.g. when we have captured an enemy town, SPY_SOW_DISSENT action must be reset to SPY_IDLE

	//-- refresh display if this firm is currently selected --//

	if( firm_array.selected_recno == firm_recno )
		info.disp();
}
//-------- End of function Firm::change_nation ---------//


//------- Begin of function Firm::toggle_firm_link ---------//
//
// Toggle the firm link of the current firm.
//
// <int> linkId     - id. of the link
// <int> toggleFlag - 1-enable, 0-disable
// [int] setBoth - if this is 1, it will set the link to either LINK_EE or LINK_DD (and no LINK_ED or LINK_DD)
//						 if this is -1, the only one side will be set even though the nation recno of the firm and town are the same
//					    (default: 0)
//
void Firm::toggle_firm_link(int linkId, int toggleFlag, char remoteAction, int setBoth)
{
#ifdef USE_DPLAY
	if( !remoteAction && remote.is_enable() )
	{
		// packet structure : <firm recno> <link Id> <toggle Flag>
		short *shortPtr = (short *)remote.new_send_queue_msg(MSG_FIRM_TOGGLE_LINK_FIRM, 3*sizeof(short));
		shortPtr[0] = firm_recno;
		shortPtr[1] = linkId;
		shortPtr[2] = toggleFlag;
		return;
	}
#endif

	int linkedNationRecno = firm_array[linked_firm_array[linkId-1]]->nation_recno;

	int sameNation = linkedNationRecno == nation_recno ||    // if one of the linked end is an indepdendent firm/nation, consider this link as a single nation link
						  linkedNationRecno == 0 ||
                    nation_recno == 0;

	if( toggleFlag )
   {
		if( (sameNation && setBoth==0) || setBoth==1 )
			linked_firm_enable_array[linkId-1] = LINK_EE;
		else
			linked_firm_enable_array[linkId-1] |= LINK_ED;
	}
	else
	{
		if( (sameNation && setBoth==0) || setBoth==1 )
			linked_firm_enable_array[linkId-1] = LINK_DD;
      else
         linked_firm_enable_array[linkId-1] &= ~LINK_ED;
   }

	//---------- if this firm is harbor, set FirmHarbor's parameter link_checked to 0

	if(firm_id == FIRM_HARBOR)
	{
		FirmHarbor *harborPtr = (FirmHarbor*) this;
		harborPtr->link_checked = 0;
	}

	//------ set the linked flag of the opposite firm -----//

	Firm* firmPtr = firm_array[ linked_firm_array[linkId-1] ];

	//---------- if firm is harbor, set FirmHarbor's parameter link_checked to 0

	if(firmPtr->firm_id==FIRM_HARBOR)
	{
		FirmHarbor *harborPtr = (FirmHarbor*) firmPtr;
		harborPtr->link_checked = 0;
	}

   int   i;

   for( i=0 ; i<firmPtr->linked_firm_count ; i++ )
   {
      if( firmPtr->linked_firm_array[i] == firm_recno )
      {
			if( toggleFlag )
			{
				if( (sameNation && setBoth==0) || setBoth==1 )
					firmPtr->linked_firm_enable_array[i]  = LINK_EE;
				else
					firmPtr->linked_firm_enable_array[i] |= LINK_DE;
			}
			else
			{
				if( (sameNation && setBoth==0) || setBoth==1 )
					firmPtr->linked_firm_enable_array[i]  = LINK_DD;
            else
               firmPtr->linked_firm_enable_array[i] &= ~LINK_DE;
         }

			break;
      }
   }
}
//-------- End of function Firm::toggle_firm_link ---------//


//------- Begin of function Firm::toggle_town_link ---------//
//
// Toggle the town link of the current firm.
//
// <int> linkId     - id. of the link
// <int> toggleFlag - 1-enable, 0-disable
// [int] setBoth - if this is 1, it will set the link to either LINK_EE or LINK_DD (and no LINK_ED or LINK_DD)
//						 if this is -1, the only one side will be set even though the nation recno of the firm and town are the same
//					    (default: 0)
//
void Firm::toggle_town_link(int linkId, int toggleFlag, char remoteAction, int setBoth)
{
#ifdef USE_DPLAY
	if( !remoteAction && remote.is_enable() )
	{
		// packet structure : <firm recno> <link Id> <toggle Flag>
		short *shortPtr = (short *)remote.new_send_queue_msg(MSG_FIRM_TOGGLE_LINK_TOWN, 3*sizeof(short));
		shortPtr[0] = firm_recno;
		shortPtr[1] = linkId;
		shortPtr[2] = toggleFlag;
		return;
	}
#endif

   int linkedNationRecno = town_array[linked_town_array[linkId-1]]->nation_recno;

   int sameNation = linkedNationRecno == nation_recno ||    // if one of the linked end is an indepdendent firm/nation, consider this link as a single nation link
						  firm_id==FIRM_BASE;      // town cannot decide whether it wants to link to Command Base or not, it is the Command Base which influences the town.

   if( toggleFlag )
   {
		if( (sameNation && setBoth==0) || setBoth==1 )
			linked_town_enable_array[linkId-1]  = LINK_EE;
		else
			linked_town_enable_array[linkId-1] |= LINK_ED;
	}
	else
	{
		if( (sameNation && setBoth==0) || setBoth==1 )
			linked_town_enable_array[linkId-1]  = LINK_DD;
		else
			linked_town_enable_array[linkId-1] &= ~LINK_ED;
	}

	//------ set the linked flag of the opposite town -----//

	Town* townPtr = town_array[ linked_town_array[linkId-1] ];
	int   i;

	for( i=0 ; i<townPtr->linked_firm_count ; i++ )
	{
		if( townPtr->linked_firm_array[i] == firm_recno )
		{
			if( toggleFlag )
			{
				if( (sameNation && setBoth==0) || setBoth==1 )
					townPtr->linked_firm_enable_array[i]  = LINK_EE;
				else
					townPtr->linked_firm_enable_array[i] |= LINK_DE;
			}
			else
			{
				if( (sameNation && setBoth==0) || setBoth==1 )
               townPtr->linked_firm_enable_array[i]  = LINK_DD;
            else
               townPtr->linked_firm_enable_array[i] &= ~LINK_DE;
         }

			break;
		}
	}

	//-------- update the town's influence --------//

	if( townPtr->nation_recno==0 )
		townPtr->update_target_resistance();

	//--- redistribute demand if a link to market place has been toggled ---//

	if( firm_id == FIRM_MARKET )
		town_array.distribute_demand();
}
//-------- End of function Firm::toggle_town_link ---------//


//------- Begin of function Firm::auto_defense -----------//
void Firm::auto_defense(short targetRecno)
{
	//--------------------------------------------------------//
	// if the firm_id is FIRM_CAMP, send the units to defense
	// the firm
	//--------------------------------------------------------//
	if(firm_id == FIRM_CAMP)
	{
		FirmCamp *campPtr = cast_to_FirmCamp();
		campPtr->defend_target_recno = targetRecno;
		campPtr->defense(targetRecno);
	}

   Town *townPtr;

   for(int i=linked_town_count-1; i>=0; i--)
   {
      if(!linked_town_array[i] || town_array.is_deleted(linked_town_array[i]))
         continue;

      townPtr = town_array[linked_town_array[i]];

      //-------------------------------------------------------//
      // find whether military camp is linked to this town. If
      // so, defense for this firm
      //-------------------------------------------------------//
      if(townPtr->nation_recno == nation_recno)
         townPtr->auto_defense(targetRecno);

      //-------------------------------------------------------//
      // some linked town may be deleted after calling auto_defense().
      // Also, the data in the linked_town_array may also be changed.
      //-------------------------------------------------------//
      if(i>linked_town_count)
         i = linked_town_count;
   }
}
//--------- End of function Firm::auto_defense -----------//


//------- Begin of function Worker::Worker -----------//
//
Worker::Worker()
{
    race_id = 0;
    unit_id = 0;
    town_recno = 0;
    name_id = 0;
    skill_id = 0;
    skill_level = 0;
    skill_level_minor = 0;
    skill_potential = 0;
    combat_level = 0;
    combat_level_minor = 0;
    spy_recno = 0;
    rank_id = 0;
    worker_loyalty = 0;
    hit_points = 0;
    extra_para = 0;
}
//--------- End of function Worker::Worker -----------//


//------- Begin of function Worker::max_hit_points -----------//
//
short Worker::max_hit_points()
{
	err_when( combat_level <= 0 );
	err_when( combat_level > 100 );

	return (int) unit_res[unit_id]->hit_points * combat_level / 100;
}
//--------- End of function Worker::max_hit_points -----------//


//--------- Begin of function Worker::max_attack_range ---------//
int Worker::max_attack_range()
{
	int maxRange=0;
	AttackInfo *attackInfo = unit_res.get_attack_info(unit_res[unit_id]->first_attack);
	int attackCount = unit_res[unit_id]->attack_count;

	for(int i=0; i<attackCount; i++, attackInfo++)
	{
		if(combat_level >= attackInfo->combat_level &&
			attackInfo->attack_range>maxRange)
			maxRange = attackInfo->attack_range;
	}
	
	return maxRange;
}
//--------- End of function Worker::max_attack_range -----------//


//--------- Begin of function Worker::is_nation ---------//
//
// Whether this worker belongs to the specific nation.
//
// <int> firmRecno - the recno of the firm the worker works in
// <int> nationRecno - the recno of th nation to check against. 
//
int Worker::is_nation(int firmRecno, int nationRecno)
{
	if( spy_recno && spy_array[spy_recno]->true_nation_recno == nationRecno )
		return 1;

	if( town_recno )
		return town_array[town_recno]->nation_recno == nationRecno;
	else
		return firm_array[firmRecno]->nation_recno == nationRecno;
}
//----------- End of function Worker::is_nation ---------//


//-------- Begin of function Firm::can_assign_capture ------//
//
// Return whether new units assigned to this firm can capture
// this firm.
//
int Firm::can_assign_capture()
{
	return (overseer_recno==0 && worker_count==0);
}
//----------- End of function Worker::can_assign_capture ---------//


//-------- Begin of function Firm::should_show_info ------//
//
// Whether information of this firm should be shown.
//
int Firm::should_show_info()
{
	if( config.show_ai_info || nation_recno==nation_array.player_recno ||
		 player_spy_count > 0 )
	{
		return 1;
	}

	//------ if the builder is a spy of the player ------//

	if( builder_recno )
	{
		if( unit_array[builder_recno]->true_nation_recno() == nation_array.player_recno )
			return 1;
	}

	//----- if any of the workers belong to the player, show the info of this firm -----//

	Worker* workerPtr = worker_array;

	for( int i=0 ; i<worker_count ; i++, workerPtr++ )
	{
		if( workerPtr->is_nation(firm_recno, nation_array.player_recno) )
			return 1;
	}

	//---- if there is a phoenix of the player over this firm ----//

	if( nation_array.player_recno && (~nation_array)->revealed_by_phoenix(loc_x1, loc_y1) )
		return 1;

	return 0;
}
//---------- End of function Firm::should_show_info --------//


//-------- Begin of function Firm::majority_race ------//
//
char Firm::majority_race()
{
	//--- if there is a overseer, return the overseer's race ---//

	if( overseer_recno )
		return unit_array[overseer_recno]->race_id;

	if( worker_count==0 )
		return 0;

	//----- count the no. people in each race ------//

	char raceCountArray[MAX_RACE];

	memset( raceCountArray, 0, sizeof(raceCountArray) );

	int i;
	for( i=0 ; i<worker_count ; i++ )
	{
		if( worker_array[i].race_id )
			raceCountArray[ worker_array[i].race_id-1 ]++;
	}

	//---------------------------------------------//

	int mostRaceCount=0, mostRaceId=0;

	for( i=0 ; i<MAX_RACE ; i++ )
	{
		if( raceCountArray[i] > mostRaceCount )
		{
			mostRaceCount = raceCountArray[i];
			mostRaceId 	  = i+1;
		}
	}

	return mostRaceId;
}
//---------- End of function Firm::majority_race --------//


//---------- Begin of function Worker::small_icon_ptr --------//

char* Worker::small_icon_ptr()
{
	// ###### begin Gilbert 17/10 ########//
	return unit_res[unit_id]->get_small_icon_ptr(rank_id);
	// ###### end Gilbert 17/10 ########//
}
//---------- End of function Worker::small_icon_ptr --------//


//---------- Begin of function Worker::change_loyalty --------//

void Worker::change_loyalty(int loyaltyChange)
{
	if( town_recno ) 		// for those live in town, their loyalty are based on town people loyalty.
		return;

	int newLoyalty = worker_loyalty + loyaltyChange;

	newLoyalty 		= MIN( 100, newLoyalty );
	worker_loyalty = MAX( 0, newLoyalty );
}
//---------- End of function Worker::change_loyalty --------//


//---------- Begin of function Worker::change_hit_points --------//

void Worker::change_hit_points(int changePoints)
{
	err_when( town_recno );		// for those live in town, their loyalty are based on town people loyalty.

	int newHitPoints = hit_points + changePoints;
	int maxHitPoints = max_hit_points();

	newHitPoints = MIN( maxHitPoints, newHitPoints );
	hit_points   = MAX( 0, newHitPoints );
}
//---------- End of function Worker::change_hit_points --------//


//-------- Begin of function Firm::is_worker_full ------//
//
int Firm::is_worker_full()
{
	return worker_count == MAX_WORKER;
}
//----------- End of function Firm::is_worker_full ---------//


//--------- Begin of function Firm::reward ---------//
//
// Only military camp has the "reward" option and not the other firms also
// because workers in other firms live in the towns and their loyalty
// are based on the town they live. Military camp is not linked to a town.
//
// <int> workerId	 - 0 - commander, >0 - id. of the soldier
// <int> remoteAction - either COMMAND_PLAYER or COMMAND_REMOTE
//
void Firm::reward(int workerId, int remoteAction)
{
#ifdef USE_DPLAY
	if( remoteAction==COMMAND_PLAYER && remote.is_enable() )
	{
		if( !remoteAction && remote.is_enable() )
		{
			// packet structure : <firm recno> <worker id>
			short *shortPtr = (short *)remote.new_send_queue_msg(MSG_FIRM_REWARD, 2*sizeof(short) );
			*shortPtr = firm_recno;
			shortPtr[1] = workerId;
		}
	}
	else
#endif
	{
		if( workerId == 0 )
		{
			if( overseer_recno )
				unit_array[overseer_recno]->reward(nation_recno);
		}
		else
		{
			err_when( workerId < 1 || workerId > worker_count );

			worker_array[workerId-1].change_loyalty( REWARD_LOYALTY_INCREASE );

			nation_array[nation_recno]->add_expense( EXPENSE_REWARD_UNIT, (float)REWARD_COST);
		}
	}
}
//----------- End of function Firm::reward -----------//


