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

//Filename    : OUNIT.CPP
//Description : Object Unit

#include <ALL.h>
#include <ODATE.h>
#include <OWORLD.h>
#include <OVGA.h>
#include <OSTR.h>
#include <ONEWS.h>
#include <OREBEL.h>
#include <OSPY.h>
#include <ONATION.h>
#include <OFONT.h>
#include <OBULLET.h>
#include <OGAME.h>
#include <OTOWN.h>
#include <ORACERES.h>
#include <ORAWRES.h>
#include <OPOWER.h>
#include <OU_VEHI.h>
#include <OU_MARI.h>
#include <OU_MONS.h>
#include <OF_CAMP.h>
#include <OF_MONS.h>
#include <OF_HARB.h>
#include <OMONSRES.h>
#include <OREMOTE.h>
#include <OSYS.h>
#include "gettext.h"
#include <ConfigAdv.h>

#if(GAME_FRAMES_PER_DAY!=FRAMES_PER_DAY)
#error
#endif

#ifdef NO_DEBUG_UNIT
#undef err_when
#undef err_here
#undef err_if
#undef err_else
#undef err_now
#define err_when(cond)
#define err_here()
#define err_if(cond)
#define err_else
#define err_now(msg)
#undef DEBUG
#endif

//--------- Begin of function Unit::Unit ---------//
//
Unit::Unit()
{
	// ##### patch begin Gilbert 21/1 ######//
   // unit_id = 0;
	memset( sizeof(Sprite) + (char *)this, 0, sizeof(Unit) - sizeof(Sprite));
	// ##### patch end Gilbert 21/1 ######//
}
//----------- End of function Unit::Unit -----------//


//--------- Begin of function Unit::~Unit ---------//
//
Unit::~Unit()
{
   deinit();
}
//----------- End of function Unit::~Unit -----------//


//--------- Begin of function Unit::init ---------//
//
// <int> unitId               - the id. of the unit
// <int> nationRecno          - the recno of nation
// [int] rankId               - rank id. of the unit (none for non-human unit)
// [int] unitLoyalty          - loyalty of the unit  (none for non-human unit)
// [int] startXLoc, startYLoc - the starting location of the unit
//                              (if startXLoc < 0, this is a unit for hire, and is not a unit of the game yet. init_sprite() won't be called for this unit)
//                              (default: -1, -1)
//
// Note: sprite_recno must be initialized first before calling Unit::init()
//
void Unit::init(int unitId, int nationRecno, int rankId, int unitLoyalty, int startXLoc, int startYLoc)
{
   //------------ set basic vars -------------//

   nation_recno  = (char) nationRecno;
   rank_id       = rankId;       // rank_id must be initialized before init_unit_id() as init_unit_id() may overwrite it
   nation_contribution = 0;      // nation_contribution must be initialized before init_unit_id() as init_unit_id() may overwrite it

   if( rank_id == RANK_GENERAL || rank_id == RANK_KING )
   {
      team_info = (TeamInfo*) mem_add( sizeof(TeamInfo) );
      team_info->member_count = 0;
      team_info->ai_last_request_defense_date = 0;
   }
   else
      team_info = NULL;

   init_unit_id(unitId);

	group_select_id = 0;
   unit_group_id = unit_array.cur_group_id++;
   race_id       = (char) unit_res[unit_id]->race_id;

   //------- init unit name ---------//

   if( race_id )
   {
      name_id = race_res[race_id]->get_new_name_id();
   }
   else  //---- init non-human unit series no. ----//
   {
		if( nation_recno )
         name_id = ++nation_array[nation_recno]->last_unit_name_id_array[unit_id-1];
      else
         name_id = 0;
   }

   //------- init ai_unit ----------//

   if( nation_recno )
      ai_unit = nation_array[nation_recno]->nation_type == NATION_AI;
   else
      ai_unit = 0;

   err_when( unitLoyalty < 0 || unitLoyalty > 100 );

   //----------------------------------------------//

   ai_action_id = 0;
   action_misc = ACTION_MISC_STOP;
   action_misc_para = 0;

   action_mode2 = action_mode = ACTION_STOP;
   action_para2 = action_para = 0;
   action_x_loc2 = action_y_loc2 = action_x_loc = action_y_loc = -1;
   memset(blocked_edge, 0, sizeof(char)*4);

   attack_range = 0; //store the attack range of the current attack mode if the unit is ordered to attack

	leader_unit_recno= 0;
   team_id          = 0;
   selected_flag    = 0;

   waiting_term     = 0;
   swapping         = 0;      // indicate whether swapping is processed.

   spy_recno = 0;

   range_attack_x_loc = -1;
   range_attack_y_loc = -1;

   //------- initialize path seek vars -------//

   result_node_array = NULL;
   result_node_count = result_node_recno = result_path_dist = 0;

	//------- initialize way point vars -------//
	way_point_array	= NULL;
	way_point_array_size = 0;
	way_point_count	= 0;

   //---------- initialize game vars ----------//

   unit_mode      = 0;
   unit_mode_para = 0;

   max_hit_points = unit_res[unit_id]->hit_points;
   hit_points     = max_hit_points;

	loyalty        = unitLoyalty;

	can_guard_flag  = 0;
	can_attack_flag = 1;
	force_move_flag = 0;
	ai_no_suitable_action = 0;
	cur_power       = 0;
	max_power       = 0;

	total_reward = 0;

	home_camp_firm_recno = 0;

	seek_path_fail_count = 0;
	ignore_power_nation  = 0;
	aggressive_mode = 1;			// the default mode is 1

	err_when( loyalty<0 || loyalty>100 );

	//--------- init skill potential ---------//

	if( misc.random(10)==0 )		// 1 out of 10 has a higher than normal potential in this skill
	{
		skill.skill_potential = 50+misc.random(51);	 // 50 to 100 potential
	}

	//------ initialize the base Sprite class -------//

   if( startXLoc >= 0 )
      init_sprite( startXLoc, startYLoc );
   else
   {
      cur_x = -1;
   }

   //------------- set attack_dir ------------//

   attack_dir = final_dir;

   //-------------- update loyalty -------------//

	update_loyalty();

   //--------------- init AI info -------------//

   if( ai_unit )
   {
      Nation* nationPtr = nation_array[nation_recno];

      if( rank_id==RANK_GENERAL || rank_id==RANK_KING )
         nationPtr->add_general_info(sprite_recno);

		switch( unit_res[unit_id]->unit_class )
      {
         case UNIT_CLASS_CARAVAN:
            nationPtr->add_caravan_info(sprite_recno);
            break;

         case UNIT_CLASS_SHIP:
            nationPtr->add_ship_info(sprite_recno);
				break;
      }
   }

   //----------- init derived class ----------//

   init_derived();
}
//----------- End of function Unit::init -----------//


//--------- Begin of function Unit::init_unit_id ---------//

void Unit::init_unit_id(int unitId)
{
   unit_id = unitId;

   UnitInfo* unitInfo = unit_res[unit_id];

   sprite_id   = unitInfo->sprite_id;
   sprite_info = sprite_res[sprite_id];

   mobile_type = unitInfo->mobile_type;

	//--- if this unit is a weapon unit with multiple versions ---//

	set_combat_level(100);     // set combat level default to 100, for human units, it will be adjusted later by individual functions

	int techLevel;
	if( nation_recno &&
		 unitInfo->unit_class == UNIT_CLASS_WEAPON &&
		(techLevel=unitInfo->nation_tech_level_array[nation_recno-1]) > 0 )
	{
		set_weapon_version(techLevel);
	}

	fix_attack_info();

   //-------- set unit count ----------//

   if( nation_recno )
   {
      if( rank_id != RANK_KING )
         unitInfo->inc_nation_unit_count(nation_recno);

      if( rank_id == RANK_GENERAL )
         unitInfo->inc_nation_general_count(nation_recno);
   }

	//--------- increase monster count ----------//

	if( unit_res[unit_id]->unit_class == UNIT_CLASS_MONSTER )
		unit_res.mobile_monster_count++;
}
//----------- End of function Unit::init_unit_id -----------//


//--------- Begin of function Unit::deinit_unit_id ---------//

void Unit::deinit_unit_id()
{
   if( sys.signal_exit_flag )
      return;

   //-----------------------------------------//

   UnitInfo *unitInfo = unit_res[unit_id];

   if( nation_recno )
   {
      if( rank_id != RANK_KING )
         unitInfo->dec_nation_unit_count(nation_recno);

      if( rank_id == RANK_GENERAL )
         unitInfo->dec_nation_general_count(nation_recno);
   }

   //--------- if the unit is a spy ----------//
   //
   // A spy has double identity and is counted
   // by both the true controlling nation and
   // the deceiving nation.
   //
   //-----------------------------------------//

   if( spy_recno && true_nation_recno() != nation_recno )
   {
      err_when( !race_id );

      int trueNationRecno = true_nation_recno();

      if( rank_id != RANK_KING )
         unitInfo->dec_nation_unit_count(trueNationRecno);

      if( rank_id == RANK_GENERAL )
         unitInfo->dec_nation_general_count(trueNationRecno);
	}

	//--------- decrease monster count ----------//

	if( unit_res[unit_id]->unit_class == UNIT_CLASS_MONSTER )
	{
		unit_res.mobile_monster_count--;

		err_when( unit_res.mobile_monster_count < 0 );
	}
}
//----------- End of function Unit::deinit_unit_id -----------//


//--------- Begin of function Unit::set_spy ---------//

void Unit::set_spy(int spyRecno)
{
   spy_recno = spyRecno;
}
//---------- End of function Unit::set_spy ---------//


//--------- Begin of function Unit::init_sprite ---------//
//
// <int> startXLoc, startYLoc - the starting location of the unit
//
void Unit::init_sprite(int startXLoc, int startYLoc)
{
   err_when( !world.get_loc(startXLoc, startYLoc)->can_move(unit_res[unit_id]->mobile_type) );
   err_when(unit_res[unit_id]->unit_class==UNIT_CLASS_SHIP && (startXLoc%2 || startYLoc%2));

   Sprite::init(unit_res[unit_id]->sprite_id, startXLoc, startYLoc);

   //--------------------------------------------------------------------//
   // move_to_?_loc is always the current location of the unit as
   // cur_action == SPRITE_IDLE
   //--------------------------------------------------------------------//
	original_action_mode = 0;
	ai_original_target_x_loc = -1;

	attack_range = 0;

   move_to_x_loc = next_x_loc();
   move_to_y_loc = next_y_loc();

   go_x = next_x;
   go_y = next_y;

   //-------- set the cargo_recno -------------//

   char  w, h;
   short x, y;

   err_if(!sprite_recno)      // sprite_recno must be initialized first before calling Unit::init()
      err_here();

   for(h=0, y=startYLoc; h<sprite_info->loc_height; h++, y++)
   {
      for(w=0, x=startXLoc; w<sprite_info->loc_width; w++, x++)
      {
         err_if( world.get_unit_recno(x, y, mobile_type) )    // it must be 0 to put the sprite in this location
            err_here();
         world.set_unit_recno(x, y, mobile_type, sprite_recno);
      }
   }

   if( is_own() ||
       (nation_recno && nation_array[nation_recno]->is_allied_with_player) )
   {
      world.unveil(startXLoc, startYLoc, startXLoc+sprite_info->loc_width-1,
                   startYLoc+sprite_info->loc_height-1 );

      world.visit(startXLoc, startYLoc, startXLoc+sprite_info->loc_width-1,
         startYLoc+sprite_info->loc_height-1, unit_res[unit_id]->visual_range,
         unit_res[unit_id]->visual_extend);
   }

	err_when(result_node_array || result_path_dist);
}
//----------- End of function Unit::init_sprite -----------//


//--------- Begin of function Unit::deinit ---------//

void Unit::deinit()
{
	err_when( unit_array.is_truly_deleted(sprite_recno) );

   if( !unit_id )
      return;

   //-------- if this is a king --------//

   if( !sys.signal_exit_flag && nation_recno )
   {
      if( rank_id == RANK_KING )    // check nation_recno because monster kings will die also.
		{
         king_die();
			err_when( unit_array.is_truly_deleted(sprite_recno) );
		}
      else if( rank_id == RANK_GENERAL )
		{
         general_die();
			err_when( unit_array.is_truly_deleted(sprite_recno) );
		}
   }

   //------------ free up team_info -----------//

   if( team_info )
   {
      mem_del(team_info);
      team_info = NULL;
   }

   //---- if this is a general, deinit its link with its soldiers ----//
   //
   // We do not use team_info because monsters and rebels also use
	// leader_unit_recno and they do not use keep the member info
   // in team_info.
   //
   //-----------------------------------------------------------------//

   if( rank_id == RANK_GENERAL || rank_id == RANK_KING )
   {
		for( int i=unit_array.size() ; i>0 ; i-- )
      {
         if( unit_array.is_deleted(i) )
            continue;

			if( unit_array[i]->leader_unit_recno == sprite_recno )
				unit_array[i]->leader_unit_recno = 0;
      }
   }

   //----- if this is a unit on a ship ------//

	if( unit_mode == UNIT_MODE_ON_SHIP )
	{
		if( !unit_array.is_deleted(unit_mode_para) )    // the ship may have been destroyed at the same time. Actually when the ship is destroyed, all units onboard are killed and this function is called.
		{
			Unit* unitPtr = unit_array[unit_mode_para];

			err_when( unit_res[unitPtr->unit_id]->unit_class != UNIT_CLASS_SHIP );

			((UnitMarine*)unitPtr)->del_unit(sprite_recno);
		}
	}

	//----- if this is a ship in the harbor -----//

	else if( unit_mode == UNIT_MODE_IN_HARBOR )
	{
		if( !firm_array.is_deleted(unit_mode_para) )    // the ship may have been destroyed at the same time. Actually when the ship is destroyed, all firms onboard are killed and this function is called.
		{
			Firm* firmPtr = firm_array[unit_mode_para];

			err_when( firmPtr->firm_id != FIRM_HARBOR );

			((FirmHarbor*)firmPtr)->del_hosted_ship(sprite_recno);
		}
	}

	//----- if this unit is a constructor in a firm -------//

	else if( unit_mode == UNIT_MODE_CONSTRUCT )
	{
		err_when( firm_array[unit_mode_para]->builder_recno != sprite_recno );

		firm_array[unit_mode_para]->builder_recno = 0;
	}

   //-------- if this is a spy ---------//

   if( spy_recno )
   {
      spy_array.del_spy( spy_recno );
		spy_recno = 0;
   }

   //---------- reset command ----------//

   if( power.command_unit_recno == sprite_recno )
      power.reset_command();

   //-----------------------------------//

   deinit_unit_id();

   //-------- reset seek path ----------//

   reset_path();

   //----- if cur_x == -1, the unit has not yet been hired -----//

   if( cur_x >= 0 )
      deinit_sprite();

   //------------------------------------------------//
   //
   // Prime rule:
   //
   // world.get_loc(next_x_loc() and next_y_loc())->cargo_recno
   // is always = sprite_recno
   //
   // no matter what cur_action is.
   //
   //------------------------------------------------//
   //
   // Relationship between (next_x, next_y) and (cur_x, cur_y)
   //
   // when SPRITE_WAIT, SPRITE_IDLE, SPRITE_READY_TO_MOVE,
   //      SPRITE_ATTACK, SPRITE_DIE:
   //
   // (next_x, next_y) == (cur_x, cur_y), it's the location of the sprite.
   //
   // when SPRITE_MOVE:
   //
   // (next_x, next_y) != (cur_x, cur_y)
   // (next_x, next_y) is where the sprite is moving towards.
   // (cur_x , cur_y ) is the location of the sprite.
   //
	//------------------------------------------------//

	//--------------- deinit AI info -------------//

   if( ai_unit )
   {
      if( !nation_array.is_deleted(nation_recno) )
		{
         Nation* nationPtr = nation_array[nation_recno];

         if( rank_id==RANK_GENERAL || rank_id==RANK_KING )
            nationPtr->del_general_info(sprite_recno);

         switch( unit_res[unit_id]->unit_class )
         {
            case UNIT_CLASS_CARAVAN:
               nationPtr->del_caravan_info(sprite_recno);
               break;

            case UNIT_CLASS_SHIP:
               nationPtr->del_ship_info(sprite_recno);
               break;
         }
      }
   }

   //-------------- reset unit_id ---------------//

   unit_id = 0;
}
//----------- End of function Unit::deinit -----------//


//--------- Begin of function Unit::deinit_sprite ---------//
//
// [int] keepSelected - keep it selected if it is current selected.
//                      (default: 0)
//
void Unit::deinit_sprite(int keepSelected)
{
   err_when(result_node_array!=NULL);

   if( cur_x == -1 )
      return;

   //---- if this unit is led by a leader, only mobile units has leader_unit_recno assigned to a leader -----//
   // units are still considered mobile when boarding a ship

   if( leader_unit_recno && unit_mode != UNIT_MODE_ON_SHIP )
   {
      if( !unit_array.is_deleted(leader_unit_recno) )    // the leader unit may have been killed at the same time
         unit_array[leader_unit_recno]->del_team_member(sprite_recno);

		leader_unit_recno = 0;
   }

   //-------- clear the cargo_recno ----------//

   short w, h;
   short x, y;

   for(h=0, y=next_y_loc(); h<sprite_info->loc_height; h++, y++)
   {
      for(w=0, x=next_x_loc(); w<sprite_info->loc_width; w++, x++)
      {
         err_if( world.get_unit_recno(x, y, mobile_type) != sprite_recno )    // it must be 0 to put the sprite in this location
            err_here();
         world.set_unit_recno(x, y, mobile_type, 0);
      }
   }

   cur_x = -1;

   //---- reset other parameters related to this unit ----//

   if( !keepSelected )
   {
      if( unit_array.selected_recno == sprite_recno )
		{
         unit_array.selected_recno = 0;
         info.disp();
      }

      if( power.command_unit_recno == sprite_recno )
         power.command_id = 0;
   }

	//------- deinit unit mode -------//

	deinit_unit_mode();
}
//----------- End of function Unit::deinit_sprite -----------//


//--------- Begin of function Unit::deinit_unit_mode ---------//
//
void Unit::deinit_unit_mode()
{
	if( sys.signal_exit_flag )
		return;

   //----- this unit was defending the town before it gets killed ----//

   if(unit_mode==UNIT_MODE_DEFEND_TOWN)
   {
      if(!town_array.is_deleted(unit_mode_para))
      {
         Town *townPtr = town_array[unit_mode_para];

         if(nation_recno == townPtr->nation_recno)
            townPtr->reduce_defender_count();
      }
      set_mode(0);      // reset mode
   }

   //----- this is a monster unit defending its town ------//

	else if( unit_mode==UNIT_MODE_MONSTER && unit_mode_para )
	{
		if(((UnitMonster*)this)->monster_action_mode!=MONSTER_ACTION_DEFENSE)
			return;

		FirmMonster* firmMonster = (FirmMonster*) firm_array[unit_mode_para];

		err_when( firmMonster->firm_id != FIRM_MONSTER );

		firmMonster->reduce_defender_count(rank_id);
	}
}
//----------- End of function Unit::deinit_unit_mode -----------//


//--------- Begin of function Unit::king_die ---------//
//
void Unit::king_die()
{
   //--------- add news ---------//

   news_array.king_die(nation_recno);

   //--- see if the units, firms and towns of the nation are all destroyed ---//

   Nation* nationPtr = nation_array[nation_recno];

	nationPtr->king_unit_recno = 0;
}
//----------- End of function Unit::king_die -----------//


//--------- Begin of function Unit::general_die ---------//
//
void Unit::general_die()
{
   //--------- add news ---------//

   if( nation_recno == nation_array.player_recno )
      news_array.general_die(sprite_recno);
}
//----------- End of function Unit::general_die -----------//


//--------- Begin of function Unit::unit_name ---------//
//
// [int] withTitle - whether return a string with the title of the unit
//                   or not. (default: 1)
//
char* Unit::unit_name(int withTitle)
{
   static String str;

   UnitInfo* unitInfo = unit_res[unit_id];

	//------------------------------------//

   if( race_id )
   {
      str = "";

      if( withTitle )
      {
         if( unit_mode == UNIT_MODE_REBEL )
         {
            if( rank_id == RANK_GENERAL )
            {
               str = _("Rebel Leader");
               str += " ";
            }
         }
         else
         {
            if( rank_id == RANK_KING )
            {
               str = _("King");
               str += " ";
            }
            else if( rank_id == RANK_GENERAL )
            {
               str = _("General");
               str += " ";
            }
         }
		}

		if( rank_id == RANK_KING )		// use the player name
			str += nation_array[nation_recno]->king_name();
		else
			str += race_res[race_id]->get_name(name_id);
   }
   else
   {
      str = _(unitInfo->name);

      //--- for weapons, the rank_id is used to store the version of the weapon ---//

      if( unitInfo->unit_class == UNIT_CLASS_WEAPON && get_weapon_version() > 1 )
		{
			str += " ";
			str += misc.roman_number(get_weapon_version());
		}

		if( unitInfo->unit_class != UNIT_CLASS_GOD )		// God doesn't have any series no.
		{
			str += " ";
			str += name_id;      // name id is the series no. of the unit
		}
   }

   return str;
}
//----------- End of function Unit::unit_name ---------//


//--------- Begin of function Unit::set_name ---------//
//
// Set the name id. of this unit.
//
void Unit::set_name(uint16_t newNameId)
{
   //------- free up the existing name id. ------//

   race_res[race_id]->free_name_id(name_id);

   //------- set the new name id. ---------//

   name_id = newNameId;

   //-------- register usage of the new name id. ------//

   race_res[race_id]->use_name_id(name_id);
}
//----------- End of function Unit::set_name ---------//


//--------- Begin of function Unit::is_own ---------//
//
int Unit::is_own()
{
   return is_nation(nation_array.player_recno);
}
//----------- End of function Unit::is_own ---------//


//--------- Begin of function Unit::is_own_spy ---------//
//
int Unit::is_own_spy()
{
   return spy_recno && spy_array[spy_recno]->true_nation_recno == nation_array.player_recno;
}
//----------- End of function Unit::is_own_spy ---------//


//--------- Begin of function Unit::is_nation ---------//
//
// Whether the unit belongs to the specific nation.
//
int Unit::is_nation(int nationRecno)
{
   if( nation_recno == nationRecno )
      return 1;

   if( spy_recno && spy_array[spy_recno]->true_nation_recno == nationRecno )
      return 1;

   return 0;
}
//----------- End of function Unit::is_nation ---------//


//--------- Begin of function Unit::is_civilian ---------//
//
int Unit::is_civilian()
{
	return race_id>0 && skill.skill_id != SKILL_LEADING &&
			 unit_mode != UNIT_MODE_REBEL;
}
//----------- End of function Unit::is_civilian ---------//


//--------- Begin of function Unit::true_nation_recno ---------//
//
// The true nation recno of the unit, taking care of the
// situation where the unit is a spy.
//
int Unit::true_nation_recno()
{
   if( spy_recno )
      return spy_array[spy_recno]->true_nation_recno;
   else
      return nation_recno;
}
//----------- End of function Unit::true_nation_recno ---------//


//--------- Begin of function Unit::next_day ---------//
//
void Unit::next_day()
{
	int unitRecno = sprite_recno;

   err_when( unit_array.is_deleted(unitRecno) );

	err_when( race_id && !is_visible() && unit_mode==0 );

	#ifdef DEBUG

	if( unit_mode == UNIT_MODE_UNDER_TRAINING )
	{
		Town* townPtr =town_array[unit_mode_para];

		err_when( townPtr->train_unit_recno != sprite_recno );
		err_when( townPtr->nation_recno     != nation_recno );
	}

	#endif

   //------- functions for non-independent nations only ------//

   if( nation_recno )
   {
      pay_expense();

		if( unit_array.is_deleted(unitRecno) )		// if its hit points go down to 0, is_deleted() will return 1.
			return;

		//------- update loyalty -------------//

		if( info.game_date%30 == sprite_recno%30 )
		{
			update_loyalty();
		   err_when( unit_array.is_deleted(unitRecno) );
		}

		//------- think about rebeling -------------//

		if( info.game_date%15 == sprite_recno%15 )
		{
			if( think_betray() )
				return;
		}
	}

	//------- recover from damage -------//

	if( info.game_date%15 == sprite_recno%15 )   // recover one point per two weeks
	{
		process_recover();
	   err_when( unit_array.is_deleted(unitRecno) );
	}

   //------- restore cur_power --------//

   cur_power += 5;

	if( cur_power > max_power)
      cur_power = max_power;

   //------- king undie flag (for testing games only) --------//

   if( config.king_undie_flag && rank_id == RANK_KING &&
       nation_recno && !nation_array[nation_recno]->is_ai() )
   {
		hit_points = max_hit_points;
	}

	//-------- if aggresive_mode is 1 --------//

	if( nation_recno && is_visible() )
		think_aggressive_action();

	//---------- debug ------------//

#ifdef DEBUG
   err_when( unit_res[unit_id]->unit_class != UNIT_CLASS_HUMAN && race_id );

   if( spy_recno )
   {
      err_when( spy_array.is_deleted(spy_recno) );

		Spy* spyPtr = spy_array[spy_recno];

		err_when( nation_recno != spyPtr->cloaked_nation_recno );

		if( unit_mode == UNIT_MODE_OVERSEE )
		{
			err_when( spyPtr->spy_place != SPY_FIRM );
			err_when( spyPtr->spy_place_para != unit_mode_para );
		}
		else
		{
			err_when( spyPtr->spy_place != SPY_MOBILE );
			err_when( spyPtr->spy_place_para != sprite_recno );
		}
   }

   if( leader_unit_recno )
   {
      Unit* unitPtr = unit_array[leader_unit_recno];

		err_when( unitPtr->rank_id != RANK_GENERAL && unitPtr->rank_id != RANK_KING );
	}

	err_when( (rank_id == RANK_GENERAL || rank_id == RANK_KING) &&
				 !team_info );

	if( leader_unit_recno )
	{
		err_when( unit_array.is_truly_deleted(leader_unit_recno) );
		err_when( unit_array[leader_unit_recno]->nation_recno != nation_recno );
//		err_when( unit_array[leader_unit_recno]->team_id != team_id );
	}

	err_when( hit_points > max_hit_points );
	err_when( max_hit_points == 0 );

   err_when( skill.combat_level<=0 );
	err_when( skill.combat_level>100 );

	err_when( unit_mode==UNIT_MODE_REBEL && spy_recno );			// no rebel spies
	err_when( unit_mode==UNIT_MODE_REBEL && nation_recno );		// all rebels must be independent units

	err_when( unit_mode==UNIT_MODE_DEFEND_TOWN && spy_recno );			// no rebel spies

	err_when( loyalty < 0 || loyalty > 100 );

	err_when( skill.skill_id == SKILL_SPYING );		// skill.skill_id should never be SKILL_SPYING, it will be shown in spy_recno if it's a spy

	err_when( nation_contribution < 0 );
	err_when( nation_contribution > MAX_NATION_CONTRIBUTION );

	err_when( ai_unit && ( !nation_recno || !nation_array[nation_recno]->is_ai() ) );

#else		// fix bug on fly in the release version

	if( skill.combat_level > 100 )
		skill.combat_level = 100;

#endif
}
//----------- End of function Unit::next_day -----------//


//--------- Begin of function Unit::process_recover ---------//
//
void Unit::process_recover()
{
	if( hit_points==0 || hit_points == max_hit_points )      // this unit is dead already
		return;

	err_when( hit_points > max_hit_points );

	//---- overseers in firms and ships in harbors recover faster ----//

   int hitPointsInc;

   if( unit_mode == UNIT_MODE_OVERSEE ||
       unit_mode == UNIT_MODE_IN_HARBOR )
   {
      hitPointsInc = 2;
   }

   //------ for units on ships --------//

	else if( unit_mode == UNIT_MODE_ON_SHIP )
	{
      //--- if the ship where the unit is on is in the harbor, the unit recovers faster ---//

      if( unit_array[unit_mode_para]->unit_mode == UNIT_MODE_IN_HARBOR )
         hitPointsInc = 2;
      else
         hitPointsInc = 1;
   }

   //----- only recover when the unit is not moving -----//

   else if( cur_action == SPRITE_IDLE )
   {
      hitPointsInc = 1;
   }
   else
      return;

   //---------- recover now -----------//

   hit_points += hitPointsInc;

   if( hit_points > max_hit_points )
      hit_points = max_hit_points;
}
//----------- End of function Unit::process_recover -----------//


//--------- Begin of function Unit::update_loyalty ---------//
//
// How loyalty of units are updated:
//
// General: in a military camp - updated in FirmCamp::update_loyalty()
//          mobile - no update
//
// Soldiers led by a general: in a military camp - updated in FirmCamp::update_loyalty()
//                            mobile - updated here
//
// Other units: no update.
//
void Unit::update_loyalty()
{
   if( !nation_recno || rank_id==RANK_KING || !unit_res[unit_id]->race_id )
		return;

	if( unit_mode == UNIT_MODE_CONSTRUCT )		// constructor worker will not change their loyalty when they are in a building
		return;

	// The following never really worked that well, since it created a dead give away due to the constant loyalty.
	//----- if this unit is a spy, set its fake loyalty ------//

	if( config_adv.unit_spy_fixed_target_loyalty && spy_recno )      // a spy's loyalty is always >= 70
	{
		if( loyalty < 70 )
			loyalty = 70+misc.random(20);    // initialize it to be a number between 70 and 90

		target_loyalty = loyalty;
		return;
	}

	//-------- if this is a general ---------//

	Nation* ownNation = nation_array[nation_recno];
	int 	  rc=0;

	if( rank_id==RANK_GENERAL )
	{
		//----- the general's power affect his loyalty ----//

		int targetLoyalty = commander_power();

		//----- the king's race affects the general's loyalty ----//

		if( ownNation->race_id == race_id )
			targetLoyalty += 20;

		//----- the kingdom's reputation affects the general's loyalty ----//

		targetLoyalty += (int)ownNation->reputation/4;

		//--- the king's leadership also affect the general's loyalty -----//

		if( ownNation->king_unit_recno )
			targetLoyalty += unit_array[ownNation->king_unit_recno]->skill.skill_level / 4;

		//-- if the unit is rewarded less than the amount of contribution he made, he will become unhappy --//

		if( nation_contribution > total_reward*2 )
		{
			int decLoyalty = (nation_contribution - total_reward*2)/2;
			targetLoyalty -= MIN(50, decLoyalty);		// this affect 50 points at maximum
		}

		targetLoyalty = MIN( targetLoyalty, 100 );
		target_loyalty = MAX( targetLoyalty, 0 );
	}

	//-------- if this is a soldier ---------//

	else if( rank_id==RANK_SOLDIER )
	{
		int leader_bonus = config_adv.unit_loyalty_require_local_leader ?
			is_leader_in_range() : leader_unit_recno;
		if( leader_bonus )
		{
			//----------------------------------------//
			//
			// If this soldier is led by a general,
			// the targeted loyalty
			//
			// = race friendliness between the unit and the general / 2
			//   + the leader unit's leadership / 2
			//
			//----------------------------------------//

			Unit* leaderUnit = unit_array[leader_unit_recno];

			int targetLoyalty = 30 + leaderUnit->skill.get_skill(SKILL_LEADING);

			//---------------------------------------------------//
			//
			// Soldiers with higher combat and leadership skill
			// will get discontented if they are led by a general
			// with low leadership.
			//
			//---------------------------------------------------//

			targetLoyalty -= skill.combat_level/2;
			targetLoyalty -= skill.skill_level;

			if( leaderUnit->rank_id == RANK_KING )
				targetLoyalty += 20;

			if( race_res.is_same_race(race_id, leaderUnit->race_id) )
				targetLoyalty += 20;

			if( targetLoyalty < 0 )
				targetLoyalty = 0;

			targetLoyalty = MIN( targetLoyalty, 100 );
			target_loyalty = MAX( targetLoyalty, 0 );
		}
		else
		{
			target_loyalty = 0;
		}
	}

	//--------- update loyalty ---------//

	err_when( target_loyalty < 0 || target_loyalty > 100 );

	if( target_loyalty > loyalty )      // only increase, no decrease. Decrease are caused by events. Increases are made gradually
	{
		int incValue = (target_loyalty - loyalty)/10;

		int newLoyalty = (int) loyalty + MAX(1, incValue);

		if( newLoyalty > target_loyalty )
			newLoyalty = target_loyalty;

		loyalty = newLoyalty;
	}
	else if( target_loyalty < loyalty )      // only increase, no decrease. Decrease are caused by events. Increases are made gradually
	{
		loyalty--;
	}

	err_when( loyalty < 0 || loyalty > 100 );
}
//-------- End of function Unit::update_loyalty -----------//


//--------- Begin of function Unit::commander_power ---------//
//
// A commander's power is determined:
//
// -Population of the towns he controls
// -The employment rate of the towns he controls, the higher the
//  employment rate, the higher his power is
// -If there are any other commanders controls the towns at the same time.
// -the no. of soldiers led by the commander and their combat levels.
//
int Unit::commander_power()
{
	//---- if the commander is in a military camp -----//

	int commanderPower=0;

	if( unit_mode == UNIT_MODE_OVERSEE )
	{
		Firm* firmPtr = firm_array[unit_mode_para];

		if( firmPtr->firm_id == FIRM_CAMP )
		{
			Town* townPtr;

			for( int i=firmPtr->linked_town_count-1 ; i>=0 ; i-- )
			{
				if( firmPtr->linked_town_enable_array[i] == LINK_EE )
				{
					townPtr = town_array[firmPtr->linked_town_array[i]];

					commanderPower += townPtr->population / townPtr->linked_active_camp_count();
				}
			}

			commanderPower += firmPtr->worker_count*3;		// 0 to 24
		}
		else if( firmPtr->firm_id == FIRM_BASE )
		{
			commanderPower = 60;
		}
	}
	else
	{
		commanderPower = team_info->member_count*3;		// 0 to 24
	}

	return commanderPower;
}
//----------- End of function Unit::commander_power -----------//


//--------- Begin of function Unit::think_betray ---------//
//
int Unit::think_betray()
{
	int unitRecno = sprite_recno;

   err_when( unit_array.is_deleted(unitRecno) );

	if( spy_recno )			// spies do not betray here, spy has its own functions for betrayal
		return 0;

	//----- if the unit is in training or is constructing a building, do not rebel -------//

	if( !is_visible() && unit_mode != UNIT_MODE_OVERSEE )
		return 0;

	if( loyalty >= UNIT_BETRAY_LOYALTY )      // you when unit is
		return 0;

	if( !unit_res[unit_id]->race_id || !nation_recno ||
		 rank_id==RANK_KING || spy_recno )
	{
		return 0;
	}

	err_when(unit_res[unit_id]->unit_class == UNIT_CLASS_GOD);
	err_when(unit_id==UNIT_CARAVAN);

	//------ turn towards other nation --------//

	int    i, bestNationRecno=0, nationScore, bestScore=loyalty;      // the score must be larger than the current loyalty
	Nation *curNation, *nationPtr;
	int	 unitRegionId = region_id();

	if( loyalty==0 )        // if the loyalty is 0, it will definitely betray
		bestScore = -100;

	curNation = nation_array[nation_recno];

	for( i=nation_array.size() ; i>0 ; i-- )
	{
		if( nation_array.is_deleted(i) )
			continue;

		if( !curNation->get_relation(i)->has_contact || i==nation_recno )
			continue;

		nationPtr = nation_array[i];

		//--- only if the nation has a base town in the region where the unit stands ---//

		if( !region_array.nation_has_base_town(unitRegionId, i) )
			continue;

		//------------------------------------------------//

		nationScore = (int) nationPtr->reputation
						  + (nationPtr->overall_rating - curNation->overall_rating);

		if( race_res.is_same_race(nationPtr->race_id, race_id) )
			nationScore += 30;

		if( nationScore > bestScore )
		{
			bestScore       = nationScore;
			bestNationRecno = i;
		}
	}

	err_when( unit_array.is_deleted(unitRecno) );

	if( bestNationRecno )
	{
		return betray(bestNationRecno);
	}
	else if( loyalty==0 )
	{
		//----------------------------------------------//
		// If there is no good nation to turn towards to and
		// the loyalty has dropped to 0, resign itself and
		// leave the nation.
		//
		// However, if the unit is spy, it will stay with the
		// nation as it has never been really loyal to the nation.
		//---------------------------------------------//

		if( rank_id != RANK_KING && is_visible() &&
			 !spy_recno )
		{
			resign(COMMAND_AUTO);
			return 1;
		}
	}

	return 0;
}
//-------- End of function Unit::think_betray -----------//


//--------- Begin of function Unit::betray ---------//
//
// If this unit is a spy, this function betray() will be
// called by Unit::spy_change_nation() or Firm::capture_firm().
//
// If this is not a spy, this function will only be called
// by think_betray() and other nation deinit functions.
//
int Unit::betray(int newNationRecno)
{
	int unitRecno = sprite_recno;

	//### begin alex 18/3 ###//
	//err_when( unit_array.is_deleted(unitRecno) );
	err_when( unit_array.is_truly_deleted(unitRecno) );
	//#### end alex 18/3 ####//

	err_when( rank_id == RANK_KING );

	if( nation_recno == newNationRecno )
		return 0;

	if( unit_mode == UNIT_MODE_CONSTRUCT ||	 // don't change nation when the unit is constructing a firm
		 unit_mode == UNIT_MODE_ON_SHIP   )  	// don't change nation when the unit is constructing a firm
	{
		return 0;
	}

	//---------- add news -----------//

	if( nation_recno == nation_array.player_recno ||
		 newNationRecno == nation_array.player_recno )
	{
		//--- if this is a spy, don't display news message for betrayal as it is already displayed in Unit::spy_change_nation() ---//

		if( !spy_recno )
			news_array.unit_betray(sprite_recno, newNationRecno);
	}

	//------ change nation now ------//

	//### begin alex 18/3 ###//
	//err_when( unit_array.is_deleted(unitRecno) );
	err_when( unit_array.is_truly_deleted(unitRecno) );
	//#### end alex 18/3 ####//

	change_nation(newNationRecno);

	//### begin alex 18/3 ###//
	//err_when( unit_array.is_deleted(unitRecno) );
	err_when( unit_array.is_truly_deleted(unitRecno) );
	//#### end alex 18/3 ####//

	//-------- set the loyalty of the unit -------//

	if( nation_recno )
	{
		Nation* nationPtr = nation_array[nation_recno];

		loyalty = UNIT_BETRAY_LOYALTY + misc.random(5);

		if( nationPtr->reputation > 0 )
			change_loyalty( (int) nationPtr->reputation );

		if( race_res.is_same_race( nationPtr->race_id, race_id ) )
			change_loyalty( 30 );

		err_when( loyalty < 0 || loyalty > 100 );

		update_loyalty();		// update target loyalty
	}
	else  //------ if change to independent rebel -------//
	{
		loyalty = 0;      // no loyalty needed
	}

	//--- if this unit is a general, change nation for the units he commands ---//

	uint32_t newTeamId = unit_array.cur_team_id++;

	if( rank_id==RANK_GENERAL )
	{
		Unit* unitPtr;
		int   i, nationReputation = (int) nation_array[nation_recno]->reputation;

		for( i=unit_array.size() ; i>0 ; i-- )
		{
			if( unit_array.is_deleted(i) )
				continue;

			unitPtr = unit_array[i];

			//---- capture the troop this general commands -----//

			if( unitPtr->leader_unit_recno == sprite_recno &&
				 unitPtr->rank_id == RANK_SOLDIER && unitPtr->is_visible() )
			{
				if( unitPtr->spy_recno )		// if the unit is a spy
					unitPtr->spy_change_nation(newNationRecno, COMMAND_AUTO, 1); // 1-group defection of this unit, allowing us to hande the change of nation

				unitPtr->change_nation(newNationRecno);

				unitPtr->team_id = newTeamId; // assign new team_id or checking for nation_recno
			}
		}
	}

	team_id = newTeamId;

	//### begin alex 18/3 ###//
	//err_when( unit_array.is_deleted(unitRecno) );
	err_when( unit_array.is_truly_deleted(unitRecno) );
	//#### end alex 18/3 ####//

	//------ go to meet the new master -------//

	if( is_visible() && nation_recno )
	{
		if( !spy_recno || spy_array[spy_recno]->notify_cloaked_nation_flag )
		{
			if( rank_id == RANK_GENERAL )		// generals shouldn't automatically be assigned to camps, they should just move near your villages
				ai_move_to_nearby_town();
			else
				think_normal_human_action();		// this is an AI function in OUNITAI.CPP
		}
	}

	//### begin alex 18/3 ###//
	//err_when( unit_array.is_deleted(unitRecno) );
	err_when( unit_array.is_truly_deleted(unitRecno) );
	//#### end alex 18/3 ####//

	return 1;
}
//-------- End of function Unit::betray -----------//


//--------- Begin of function Unit::change_nation ---------//
//
// This function is called when a unit change nation.
// It is not necessarily a result of betray, when a spy
// hands over his new nation to his parent nation, this
// function will also be called.
//
// <int> newNationRecno - change the nation of the unit.
//
void Unit::change_nation(int newNationRecno)
{
	err_when( newNationRecno && nation_array.is_deleted(newNationRecno) );
	err_when( unit_mode == UNIT_MODE_REBEL );		// rebels do not change nation

	//---------------------------------//

	int oldAiUnit = ai_unit;
	int oldNationRecno = nation_recno;

	group_select_id = 0; // clear group select id
	if(way_point_count)
		reset_way_point_array();

	//-- if the player is giving a command to this unit, cancel the command --//

   if( nation_recno == nation_array.player_recno &&
		 sprite_recno == unit_array.selected_recno &&
		 power.command_id )
   {
		power.command_id = 0;
   }

	//---------- stop all action to attack this unit ------------//

	unit_array.stop_attack_unit(sprite_recno);

	//---- update nation_unit_count_array[] ----//

	unit_res[unit_id]->unit_change_nation(newNationRecno, nation_recno, rank_id);

	//------- if the nation has an AI action -------//

	stop2();       // clear the existing order

	//---------------- update vars ----------------//

	unit_group_id = unit_array.cur_group_id++;      // separate from the current group
	nation_recno  = newNationRecno;

	home_camp_firm_recno  = 0;					// reset it
	original_action_mode  = 0;

	if( race_id )
	{
		nation_contribution = 0;            // contribution to the nation
		total_reward        = 0;
	}

	//-------- if change to one of the existing nations ------//

	ai_unit = nation_recno && nation_array[nation_recno]->is_ai();

	//------------ update AI info --------------//

	if( oldAiUnit )
	{
		Nation* nationPtr = nation_array[oldNationRecno];

		if( rank_id == RANK_GENERAL || rank_id == RANK_KING )
			nationPtr->del_general_info(sprite_recno);

		else if( unit_res[unit_id]->unit_class == UNIT_CLASS_CARAVAN )
			nationPtr->del_caravan_info(sprite_recno);

		else if( unit_res[unit_id]->unit_class == UNIT_CLASS_SHIP )
			nationPtr->del_ship_info(sprite_recno);
	}

	if( ai_unit && nation_recno != 0 )
	{
		Nation* nationPtr = nation_array[nation_recno];

		if( rank_id == RANK_GENERAL || rank_id == RANK_KING )
			nationPtr->add_general_info(sprite_recno);

		else if( unit_res[unit_id]->unit_class == UNIT_CLASS_CARAVAN )
			nationPtr->add_caravan_info(sprite_recno);

		else if( unit_res[unit_id]->unit_class == UNIT_CLASS_SHIP )
			nationPtr->add_ship_info(sprite_recno);
	}

	//------ if this unit oversees a firm -----//

	if( unit_mode==UNIT_MODE_OVERSEE )
		firm_array[unit_mode_para]->change_nation(newNationRecno);

	//----- this unit was defending the town before it gets killed ----//

	else if( unit_mode==UNIT_MODE_DEFEND_TOWN )
	{
		if( !town_array.is_deleted(unit_mode_para) )
			town_array[unit_mode_para]->reduce_defender_count();

		set_mode(0);   // reset unit mode
	}

	//---- if the unit is no longer the same nation as the leader ----//

	if( leader_unit_recno )
	{
		Unit* leaderUnit = unit_array[leader_unit_recno];

		if( leaderUnit->nation_recno != nation_recno )
		{
			leaderUnit->del_team_member(sprite_recno);
			leader_unit_recno = 0;
			team_id = 0;
		}
	}

	//------ if it is currently selected -------//

	if( selected_flag )
		info.disp();
}
//----------- End of function Unit::change_nation -----------//


//--------- Begin of function Unit::pay_expense ---------//
//
void Unit::pay_expense()
{
   if( game.game_mode == GAME_TEST )      // no deduction in testing game
      return;

   if( !nation_recno )
      return;

   //--- if it's a mobile spy or the spy is in its own firm, no need to pay salary here as Spy::pay_expense() will do that ---//
   //
   // -If your spies are mobile:
   //  >your nation pays them 1 food and $5 dollars per month
   //
   // -If your spies are in an enemy's town or firm:
   //  >the enemy pays them 1 food and the normal salary of their jobs.
   //
   //  >your nation pays them $5 dollars per month. (your nation pays them no food)
   //
   // -If your spies are in your own town or firm:
   //  >your nation pays them 1 food and $5 dollars per month
   //
   //------------------------------------------------------//

   if( spy_recno )
   {
      if( is_visible() )      // the cost will be deducted in spy_array
         return;

      if( unit_mode == UNIT_MODE_OVERSEE &&
          firm_array[unit_mode_para]->nation_recno == true_nation_recno() )
      {
         return;
      }
   }

   //---------- if it's a human unit -----------//
   //
   // The unit is paid even during its training period in a town
   //
   //-------------------------------------------//

   Nation* nationPtr = nation_array[nation_recno];

   if( unit_res[unit_id]->race_id )
   {
      //---------- reduce cash -----------//

      if( nationPtr->cash > 0 )
      {
         if( rank_id == RANK_SOLDIER )
            nationPtr->add_expense( EXPENSE_MOBILE_UNIT, (float) SOLDIER_YEAR_SALARY / 365, 1 );

         if( rank_id == RANK_GENERAL )
            nationPtr->add_expense( EXPENSE_GENERAL, (float) GENERAL_YEAR_SALARY / 365, 1 );
      }
      else     // decrease loyalty if the nation cannot pay the unit
      {
         change_loyalty(-1);
      }

      //---------- reduce food -----------//

      if( unit_res[unit_id]->race_id )    // if it's a human unit
      {
         if( nationPtr->food > 0 )
				nationPtr->consume_food((float) PERSON_FOOD_YEAR_CONSUMPTION / 365);
			else
			{
				if( info.game_date%NO_FOOD_LOYALTY_DECREASE_INTERVAL == 0 )		// decrease 1 loyalty point every 2 days
					change_loyalty(-1);
         }
      }
   }
   else  //----- it's a non-human unit ------//
   {
      if( nationPtr->cash > 0 )
      {
         int expenseType;

			switch(unit_res[unit_id]->unit_class)
			{
				case UNIT_CLASS_WEAPON:
					expenseType = EXPENSE_WEAPON;
					break;

				case UNIT_CLASS_SHIP:
					expenseType = EXPENSE_SHIP;
					break;

				case UNIT_CLASS_CARAVAN:
					expenseType = EXPENSE_CARAVAN;
					break;

				default:
					expenseType = EXPENSE_MOBILE_UNIT;
			}

			nationPtr->add_expense( expenseType, (float) unit_res[unit_id]->year_cost / 365, 1 );
		}
		else     // decrease hit points if the nation cannot pay the unit
		{
			if( unit_res[unit_id]->unit_class != UNIT_CLASS_CARAVAN )		// Even when caravans are not paid, they still stay in your service.
			{
				if( hit_points > 0 )
				{
					hit_points--;

					if( hit_points < 0 )
						hit_points = (float) 0;

					//--- when the hit points drop to zero and the unit is destroyed ---//

					if( hit_points==0 )
					{
						if( nation_recno == nation_array.player_recno )
						{
							int unitClass = unit_res[unit_id]->unit_class;

							if( unitClass==UNIT_CLASS_WEAPON )
								news_array.weapon_ship_worn_out(unit_id, get_weapon_version());

							else if( unitClass==UNIT_CLASS_SHIP )
								news_array.weapon_ship_worn_out(unit_id, 0);
						}
					}
				}
			}
      }
   }
}
//----------- End of function Unit::pay_expense -----------//


//--------- Begin of function Unit::change_hit_points ---------//
//
void Unit::change_hit_points(float changePoints)
{
   hit_points += changePoints;

   if( hit_points < 0 )
      hit_points = (float) 0;

   if( hit_points > max_hit_points )
      hit_points = max_hit_points;
}
//-------- End of function Unit::change_hit_points -----------//


//--------- Begin of function Unit::change_loyalty ---------//
//
// <int> changeAmt - amount of loyalty to be changed.
//
void Unit::change_loyalty(int changeAmt)
{
   int newLoyalty = loyalty + changeAmt;

   newLoyalty = MAX(0, newLoyalty);

   loyalty = MIN(100, newLoyalty);
}
//----------- End of function Unit::change_loyalty -----------//


//------- Begin of function Unit::inc_minor_combat_level --------//
//
void Unit::inc_minor_combat_level(int incLevel)
{
   err_when( incLevel<0 || incLevel>100 );   // it cannot be larger than 100, because the current code can't handle it

   skill.combat_level_minor += incLevel;

   if( skill.combat_level_minor > 100 )
   {
      if( skill.combat_level < 100 )
         set_combat_level(skill.combat_level+1);

      skill.combat_level_minor -= 100;
   }
}
//-------- End of function Unit::inc_minor_combat_level ---------//


//------- Begin of function Unit::inc_minor_skill_level --------//
//
void Unit::inc_minor_skill_level(int incLevel)
{
   err_when( incLevel<0 || incLevel>100 );

   skill.skill_level_minor += incLevel;

   if( skill.skill_level_minor > 100 )
   {
      if( skill.skill_level < 100 )
         skill.skill_level++;

      skill.skill_level_minor -= 100;
   }
}
//-------- End of function Unit::inc_minor_skill_level ---------//


//--------- Begin of function Unit::set_combat_level ---------//
//
void Unit::set_combat_level(int combatLevel)
{
	err_when( combatLevel<=0 || combatLevel>100 );

   skill.combat_level = combatLevel;

   UnitInfo* unitInfo = unit_res[unit_id];

   int oldMaxHitPoints = max_hit_points;

   max_hit_points = unitInfo->hit_points * combatLevel / 100;

   hit_points = hit_points * max_hit_points / oldMaxHitPoints;

   hit_points = MIN(hit_points, max_hit_points);

   // --------- update can_guard_flag -------//

   if( combatLevel >= unitInfo->guard_combat_level)
   {
      can_guard_flag = sprite_info->can_guard_flag;
		if( unit_id == UNIT_ZULU )
			can_guard_flag |= 4;			// shield during attack delay
   }
   else
   {
      can_guard_flag = 0;
   }

   max_power = skill.combat_level + 50;
   cur_power = MIN(cur_power, max_power);
}
//-------- End of function Unit::set_combat_level -----------//


//--------- Begin of function Unit::set_rank ---------//
//
// Only if the unit has leadership skill, it can be a general or king.
//
void Unit::set_rank(int rankId)
{
	err_when( !race_id );

#ifdef DEBUG
	if( !is_visible() )
	{
		err_when( rank_id==RANK_GENERAL && rankId==RANK_SOLDIER );		
		err_when( rank_id==RANK_SOLDIER && rankId==RANK_GENERAL );
	}
#endif 

	if( rank_id == rankId )
      return;

   //------- promote --------//

   if( rankId > rank_id )
		change_loyalty(PROMOTE_LOYALTY_INCREASE);

	//------- demote -----------//

	else if( rankId < rank_id && rank_id != RANK_KING )      // no decrease in loyalty if a spy king hands his nation to his parent nation and become a general again
		change_loyalty(-DEMOTE_LOYALTY_DECREASE);

	//---- update nation_general_count_array[] ----//

	if( nation_recno )
	{
		UnitInfo* unitInfo = unit_res[unit_id];

		if( rank_id == RANK_GENERAL )    // if it was a general originally
			unitInfo->dec_nation_general_count(nation_recno);

		if( rankId == RANK_GENERAL )     // if the new rank is general
			unitInfo->inc_nation_general_count(nation_recno);

		//------ if demote a king to a unit ------//

		if( rank_id == RANK_KING && rankId != RANK_KING )
			unitInfo->inc_nation_unit_count(nation_recno);     // since kings are not included in nation_unit_count, when it is no longer a king, we need to re-increase it.

		//------ if promote a unit to a king ------//

		if( rank_id != RANK_KING && rankId == RANK_KING )
			unitInfo->dec_nation_unit_count(nation_recno);     // since kings are not included in nation_unit_count, we need to decrease it
	}

	//----- reset leader_unit_recno if demote a general to soldier ----//

	if( rank_id == RANK_GENERAL && rankId == RANK_SOLDIER )
	{
		//----- reset leader_unit_recno of the units he commands ----//

		for( int i=unit_array.size() ; i>0 ; i-- )
		{
			Unit* unitPtr = (Unit*) unit_array.get_ptr(i);		// don't use is_deleted() as it filters out units that are currently dying 

			if( unitPtr && unitPtr->leader_unit_recno == sprite_recno )
			{
				unitPtr->leader_unit_recno = 0;
				unitPtr->team_id = 0;
			}
		}

		//--------- deinit team_info ---------//

		err_when( !team_info );

		mem_del(team_info);
		team_info = NULL;
		team_id   = 0;
	}

	//----- if this is a soldier being promoted to a general -----//

	else if( rank_id == RANK_SOLDIER && rankId == RANK_GENERAL )
	{
		//-- if this soldier is formerly commanded by a general, detech it ---//

		if( leader_unit_recno )
		{
			if( !unit_array.is_deleted(leader_unit_recno) )    // the leader unit may have been killed at the same time
				unit_array[leader_unit_recno]->del_team_member(sprite_recno);

			leader_unit_recno = 0;
		}
	}

	//-------------- update AI info --------------//

	if( ai_unit )
	{
		if( rank_id == RANK_GENERAL || rank_id == RANK_KING )
			nation_array[nation_recno]->del_general_info(sprite_recno);

		rank_id = rankId;

		if( rank_id == RANK_GENERAL || rank_id == RANK_KING )
			nation_array[nation_recno]->add_general_info(sprite_recno);
	}
   else
   {
      rank_id = rankId;
   }

   //----- if this is a general/king ------//

   if( rank_id == RANK_GENERAL || rank_id == RANK_KING )
   {
      //--------- init team_info -------//

      if( !team_info )
      {
         team_info = (TeamInfo*) mem_add( sizeof(TeamInfo) );
         team_info->member_count = 0;
         team_info->ai_last_request_defense_date = 0;
      }

      //--- set leadership if this unit does not have any now ----//

      if( skill.skill_id != SKILL_LEADING )
      {
         skill.skill_id = SKILL_LEADING;
			skill.skill_level = 10 + misc.random(40);
      }
   }

   //------ refresh if the current unit is selected -----//

   if( unit_array.selected_recno == sprite_recno )
      info.disp();
}
//-------- End of function Unit::set_rank -----------//


//--------- Begin of function Unit::embark ---------//
//
// Order this unit to embark an vehicle
//
// <int> vehicleRecno - recno of the vehicle.
//
void Unit::embark(int vehicleRecno)
{
	err_here();			// this function is no longer functional

   Unit* vehiclePtr = unit_array[vehicleRecno];

   if( unit_res[unit_id]->vehicle_id == vehiclePtr->unit_id )   // not the right vehicle
      return;

   int vehicleUnitId    = unit_res[unit_id]->vehicle_unit_id;
   float vehicleHitPoints = vehiclePtr->hit_points;
   int xLoc = vehiclePtr->cur_x_loc();
   int yLoc = vehiclePtr->cur_y_loc();

   //------- delete the vehicle unit --------//

   unit_array.del(vehicleRecno);    // delete the vehicle (e.g. horse)

   //--------- add the combined unit -------//

   int newUnitRecno = unit_array.add_unit(vehicleUnitId, nation_recno, rank_id, loyalty, xLoc, yLoc);    // add the combined unit (e.g. cavalry)

   UnitVehicle* unitVehicle = (UnitVehicle*) unit_array[newUnitRecno];

   unitVehicle->skill = skill;
   unitVehicle->set_combat_level(skill.combat_level);

   unitVehicle->solider_hit_points = (int) hit_points;
   unitVehicle->vehicle_hit_points = (int) vehicleHitPoints;

   unitVehicle->hit_points = (float) unitVehicle->solider_hit_points +
                             unitVehicle->vehicle_hit_points;

   //-------- delete the solider unit ---------//

   unit_array.del(sprite_recno);    // delete the embarker (e.g. knight)
}
//-------- End of function Unit::embark -----------//



//--------- Begin of function Unit::reward ---------//
//
// <int> rewardNationRecno - the nation which does this reward.
//
void Unit::reward(int rewardNationRecno)
{
	// ###### patch begin Gilbert 24/9 ########//
	if( nation_array[rewardNationRecno]->cash < REWARD_COST )
		return;
	// ###### patch end Gilbert 24/9 ########//

   //--------- if this is a spy ---------//

   if( spy_recno && true_nation_recno() == rewardNationRecno )    // if the spy's owning nation rewards the spy
   {
      spy_array[spy_recno]->change_loyalty(REWARD_LOYALTY_INCREASE);
	}

	//--- if this spy's nation_recno & true_nation_recno() are both == rewardNationRecno, it's true loyalty and cloaked loyalty will both be increased ---//

	if( nation_recno == rewardNationRecno )
	{
		total_reward += REWARD_COST;

		change_loyalty(REWARD_LOYALTY_INCREASE);
	}

   nation_array[rewardNationRecno]->add_expense(EXPENSE_REWARD_UNIT, (float)REWARD_COST);
}
//----------- End of function Unit::reward -----------//


//------- Begin of function Unit::overseer_migrate ---------//
//
// Order the overseer migrate to a new town but still keeps
// working for the same firm.
//
// <int> destTownRecno - the recno of the town the worker should
//                       migrate to.
//
void Unit::overseer_migrate(int destTownRecno)
{
   err_when( unit_mode!=UNIT_MODE_OVERSEE );

   int curTownRecno = firm_array[unit_mode_para]->overseer_town_recno;

   //------- decrease the population of the unit's home town ------//

   town_array[curTownRecno]->dec_pop(race_id, 1);

   //--------- increase the population of the target town ------//

   town_array[destTownRecno]->inc_pop(race_id, 1, loyalty );
}
//-------- End of function Unit::overseer_migrate ---------//


//--------- Begin of function Unit::group_transform ---------//

void Unit::group_transform(char remoteAction, short *selectedArray, short selectedCount)
{
}
//----------- End of function Unit::group_transform -----------//


//--------- Begin of function Unit::transform ---------//
//
// Transform the unit into another unit type.
//
void Unit::transform()
{
/*
   UnitInfo* unitInfo = unit_res[unit_id];

   if( unitInfo->transform_unit_id==0 )
      return;

   UnitInfo* newUnitInfo = unit_res[unitInfo->transform_unit_id];

   //--- check if the unit has the required combat level ---//

   if( skill.combat_level < unitInfo->transform_combat_level )
      return;

   //------ free up space on the map for the new unit -----//

   int xLoc=next_x_loc(), yLoc=next_y_loc();

   stop();

   deinit_sprite(1);    // 1-keep the unit selected if it is currently selected

   //---- locate the space for the new unit as their size, mobile type may be different ----//

   SpriteInfo* spriteInfo = sprite_res[newUnitInfo->sprite_id];
   int         xLoc2 = xLoc + spriteInfo->loc_width-1;
   int         yLoc2 = yLoc + spriteInfo->loc_height-1;

   if( !world.check_unit_space(xLoc, yLoc, xLoc2, yLoc2, newUnitInfo->mobile_type) )      // first check if if is free to create unit on the same location
   {
      if( !world.locate_space( xLoc, yLoc, xLoc+spriteInfo->loc_width-1,
                               yLoc+spriteInfo->loc_height-1, spriteInfo->loc_width,
                               spriteInfo->loc_height, newUnitInfo->mobile_type) )
      {
         init_sprite( xLoc, yLoc );    // not able to find space for the new unit, transformation cancelled.
         return;
		}
	}

   //--------- transform now ------------//

   deinit_unit_id();

   init_unit_id(unitInfo->transform_unit_id);

   init_sprite( xLoc, yLoc );
*/
}
//----------- End of function Unit::transform -----------//


//--------- Begin of function Unit::spy_change_nation ---------//
//
// Change the deceiving nation recno of a spy unit which you control.
//
// <int>  newNationRecno - the new nation the spy changes its cloack to 
// <char> remoteAction   - remote action type 
// <int>  groupDefect    - if 1 this spy changed nation as part of a group defect called by Unit::betray
//                         when a General-spy changes colour and takes its subordinates along
//						   and the subordinate is actually an enemy spy. Supresses news notification and
//						   lets the caller handle the Unit nation change (so only handles spy nation change)
//
void Unit::spy_change_nation(int newNationRecno, char remoteAction, int groupDefect)
{
	if( newNationRecno == nation_recno )
		return;

	if( newNationRecno && nation_array.is_deleted(newNationRecno) )      // this can happen in a multiplayer message
		return;

	//------- if this is a remote action -------//

	if( !remoteAction && remote.is_enable() )
	{
		// packet structure <unit recno> <new nation Recno> <group defect>
		short *shortPtr = (short *)remote.new_send_queue_msg(MSG_UNIT_SPY_NATION, 3*sizeof(short) );
		*shortPtr = sprite_recno;
		shortPtr[1] = newNationRecno;
		shortPtr[2] = groupDefect;
		return;
	}

	//----- update the var in Spy ------//

	Spy* spyPtr = spy_array[spy_recno];

	//--- when a spy change cloak to another nation, he can't cloak as a general, he must become a soldier first ---//

	if( is_visible() &&					// if the spy is a commander in a camp, don't set its rank to soldier
		 rank_id == RANK_GENERAL &&
		 newNationRecno != spyPtr->true_nation_recno )
	{
		set_rank(RANK_SOLDIER);
	}

	//---------------------------------------------------//
	//
	// If this spy unit is a general or an overseer of the
	// cloaked nation, when he changes nation, that will
	// inevitably be noticed by the cloaked nation.
	//
	//---------------------------------------------------//

	if( spyPtr->true_nation_recno != nation_array.player_recno )	// only send news message if he is not the player's own spy
	{
		if( rank_id == RANK_GENERAL || unit_mode == UNIT_MODE_OVERSEE ||
			 (spyPtr->notify_cloaked_nation_flag && !groupDefect) )
		{
			//-- if this spy's cloaked nation is the player's nation, the player will be notified --//

			if( nation_recno == nation_array.player_recno )
				news_array.unit_betray(sprite_recno, newNationRecno);
		}

		//---- send news to the cloaked nation if notify flag is on ---//

		if( spyPtr->notify_cloaked_nation_flag && !groupDefect )
		{
			if( newNationRecno == nation_array.player_recno )    			// cloaked as the player's nation
				news_array.unit_betray(sprite_recno, newNationRecno);
		}
	}

	//--------- change nation recno now --------//

	spyPtr->cloaked_nation_recno = newNationRecno;

	if (!groupDefect)
		betray(newNationRecno);		// call the betray function to change natino. There is no difference between a spy changing nation and a unit truly betrays
}
//----------- End of function Unit::spy_change_nation -----------//


//--------- Begin of function Unit::can_spy_change_nation ---------//
//
// Whether the spy unit can change its spy cloak now or not.
//
// If there are enemy nearby, the unit cannot change its cloak.
//
int Unit::can_spy_change_nation()
{
	if( !spy_recno )
      return 0;

   //--------------------------------------------//

   int xLoc1=cur_x_loc()-SPY_ENEMY_RANGE, yLoc1=cur_y_loc()-SPY_ENEMY_RANGE;
   int xLoc2=cur_x_loc()+SPY_ENEMY_RANGE, yLoc2=cur_y_loc()+SPY_ENEMY_RANGE;

   xLoc1 = MAX(0, xLoc1);
   yLoc1 = MAX(0, yLoc1);
   xLoc2 = MIN(MAX_WORLD_X_LOC-1, xLoc2);
   yLoc2 = MIN(MAX_WORLD_Y_LOC-1, yLoc2);

   int       xLoc, yLoc;
   int       unitRecno, trueNationRecno = true_nation_recno();
   Location* locPtr;

   for( yLoc=yLoc1 ; yLoc<=yLoc2 ; yLoc++ )
   {
      locPtr = world.get_loc(xLoc1, yLoc);

      for( xLoc=xLoc1 ; xLoc<=xLoc2 ; xLoc++, locPtr++ )
      {
         if( locPtr->has_unit(UNIT_LAND) )
            unitRecno = locPtr->unit_recno(UNIT_LAND);

         else if( locPtr->has_unit(UNIT_SEA) )
            unitRecno = locPtr->unit_recno(UNIT_SEA);

         else if( locPtr->has_unit(UNIT_AIR) )
            unitRecno = locPtr->unit_recno(UNIT_AIR);

         else
            continue;

         if( unit_array.is_deleted(unitRecno) )    // the unit is dying, its recno is still in the location
            continue;

         if( unit_array[unitRecno]->true_nation_recno() != trueNationRecno )
            return 0;
      }
   }

   return 1;
}
//----------- End of function Unit::can_spy_change_nation -----------//


//--------- Begin of function Unit::resign ---------//
//
// Resign the unit.
//
void Unit::resign(int remoteAction)
{
   if( !remoteAction && remote.is_enable() )
   {
      // packet structure : <unit recno> <nation recno>
      short *shortPtr = (short *)remote.new_send_queue_msg(MSG_UNIT_RESIGN, 2*sizeof(short));
      *shortPtr = sprite_recno;
		shortPtr[1] = nation_array.player_recno;

      return;
   }

   //---- increase the wandering count when a unit is disbanded ----//

   if( race_id )
		town_array.race_wander_pop_array[race_id-1] += 2;     // disbanding one resulted in two wandering units to make the effect more significant

	//--- if the unit is visible, call stop2() so if it has an AI action queue, that will be reset ---//

	if( is_visible() )
		stop2();

	//--- if the spy is resigned by an enemy, display a message ---//

	if( spy_recno && true_nation_recno() != nation_recno )		// the spy is cloaked in an enemy nation when it is resigned
	{
		//------ decrease reputation ------//

		nation_array[true_nation_recno()]->change_reputation((float)-SPY_KILLED_REPUTATION_DECREASE);

		//------- add news message -------//

		if( true_nation_recno() == nation_array.player_recno ||		// display when the player's spy is revealed or the player has revealed an enemy spy
			 nation_recno == nation_array.player_recno )
		{
			//--- If a spy is caught, the spy's nation's reputation wil decrease ---//

			news_array.spy_killed(spy_recno);
		}
	}

	//----------------------------------------------//

	if( rank_id == RANK_GENERAL )			// if this is a general, news_array.general_die() will be called, set news_add_flag to 0 to suppress the display of thew news
		news_array.news_add_flag=0;

	unit_array.del( sprite_recno );

	news_array.news_add_flag=1;
}
//----------- End of function Unit::resign -----------//


//--------- Begin of function Unit::region_id ---------//
//
// Return the region id. of this unit.
//
uint8_t Unit::region_id()
{
   if( is_visible() )
   {
      return world.get_region_id( next_x_loc(), next_y_loc() );
	}
	else
	{
		if( unit_mode == UNIT_MODE_OVERSEE )
			return firm_array[unit_mode_para]->region_id;
	}

	return 0;
}
//----------- End of function Unit::region_id -----------//


//--------- Begin of function Unit::del_team_member ---------//
//
// Delete a specific member of the team led by this leader.
//
void Unit::del_team_member(int unitRecno)
{
	err_when( !team_info );

   for( int i=0 ; i<team_info->member_count ; i++ )
   {
      if( team_info->member_unit_array[i] == unitRecno )
		{
			err_when( team_info->member_count > MAX_TEAM_MEMBER );

			misc.del_array_rec( team_info->member_unit_array, team_info->member_count,
								  sizeof( team_info->member_unit_array[0] ), i+1 );

         team_info->member_count--;
         return;
      }
   }

   //-------------------------------------------------------//
   //
   // Note: for rebels and monsters, although they have
   //       leader_unit_recno, their team_info is not used.
   //       So del_team_member() won't be able to match the
   //       unit in its member_unit_array[].
   //
   //-------------------------------------------------------//
}
//----------- End of function Unit::del_team_member -----------//


//--------- Begin of function Unit::validate_team ---------//
//
// Validate the member in this commander's team. If there
// are any units with hit_points <= 0, delete them.
//
// Those unit may just be killed, so soon that the Unit's set_die()
// function hsa been called yet. validate_team() function must
// be called before all issunig any new team actions.
//
void Unit::validate_team()
{
	err_when( !team_info );

	int unitRecno;

	for( int i=team_info->member_count-1 ; i>=0 ; i-- )
	{
		unitRecno = team_info->member_unit_array[i];

		if( unit_array.is_deleted(unitRecno) )
		{
			err_when( team_info->member_count > MAX_TEAM_MEMBER );

			misc.del_array_rec( team_info->member_unit_array, team_info->member_count,
								  sizeof( team_info->member_unit_array[0] ), i+1 );

			team_info->member_count--;
		}
   }
}
//----------- End of function Unit::validate_team -----------//


//--------- Begin of function Unit::commanded_soldier_count ---------//
//
// Return the no. of soldiers commanded by this unit.
//
int Unit::commanded_soldier_count()
{
	if( rank_id != RANK_GENERAL && rank_id != RANK_KING )
		return 0;

	//--------------------------------------//

	err_when( !team_info );

   int soldierCount=0;

   if( is_visible() )
   {
      soldierCount = team_info->member_count-1;

      if( soldierCount < 0 )  // member_count can be 0
         soldierCount = 0;
   }
   else
   {
      if( unit_mode == UNIT_MODE_OVERSEE )
      {
         Firm* firmPtr = firm_array[unit_mode_para];

         if( firmPtr->firm_id == FIRM_CAMP )			// it can be an overseer of a seat of powre
				soldierCount = firmPtr->worker_count;
      }
   }

   return soldierCount;
}
//----------- End of function Unit::commanded_soldier_count -----------//


//----------- Begin of function Unit::fix_attack_info -----------//
void Unit::fix_attack_info()
{
	int techLevel;
	UnitInfo *unitInfo = unit_res[unit_id];

	attack_count = unitInfo->attack_count;

   if( attack_count > 0 && unitInfo->first_attack > 0)
      attack_info_array = unit_res.attack_info_array+unitInfo->first_attack-1;
   else
      attack_info_array = NULL;

   if( unitInfo->unit_class == UNIT_CLASS_WEAPON && 
		(techLevel=get_weapon_version()) > 0 )
   {
      switch( unit_id )
      {
      case UNIT_BALLISTA:
      case UNIT_F_BALLISTA:
         attack_count = 2;
         break;
      case UNIT_EXPLOSIVE_CART:
         attack_count = 0;
         break;
      default:
         attack_count = 1;
      }

      if( attack_count > 0)
      {
         attack_info_array += (techLevel-1) * attack_count;
      }
      else
      {
         // no attack like explosive cart
         attack_info_array = NULL;
      }
   }
}
//----------- End of function Unit::fix_attack_info -----------//


//----------- Begin of function Unit::return_camp -----------//
//
// Order this unit to return to the camp. For ordering many
// units to return to a camp, UnitArray::return_camp() should
// be called instead.
//
int Unit::return_camp()
{
	if( !home_camp_firm_recno )
		return 0;

	err_when( firm_array.is_deleted(home_camp_firm_recno) );

	Firm* firmPtr = firm_array[home_camp_firm_recno];

	if( firmPtr->region_id != region_id() )
		return 0;

	err_when( firmPtr->firm_id != FIRM_CAMP );
	err_when( firmPtr->nation_recno != nation_recno );

	//--------- assign now ---------//

	assign(firmPtr->loc_x1, firmPtr->loc_y1);

	force_move_flag = 1;

	return cur_action != SPRITE_IDLE;
}
//----------- End of function Unit::return_camp -----------//


//----------- Begin of function Unit::unit_power -----------//
//
// Return a power index of the weapon, this is for calculating
// the total combat level of a target.
//
int Unit::unit_power()
{
	UnitInfo* unitInfo = unit_res[unit_id];

	if( unitInfo->unit_class == UNIT_CLASS_WEAPON )
	{
		return (int) hit_points + (unitInfo->weapon_power + get_weapon_version() - 1) * 15;
	}
	else
	{
		return (int) hit_points;
	}
}
//----------- End of function Unit::unit_power -----------//


//----------- Begin of function Unit::get_cur_loc -----------//
//
// <short&> xLoc, yLoc - reference vars for returning the
//								 location of this unit
//
// return : 0 - if this unit is invisible
//				1 - if a location has been returned.
//
int Unit::get_cur_loc(short& xLoc, short& yLoc)
{
	if( is_visible() )
	{
		xLoc = next_x_loc();		// update location
		yLoc = next_y_loc();
	}
	else if( unit_mode == UNIT_MODE_OVERSEE ||
				unit_mode==UNIT_MODE_CONSTRUCT ||
				unit_mode == UNIT_MODE_IN_HARBOR )
	{
		Firm* firmPtr = firm_array[unit_mode_para];

		xLoc = firmPtr->center_x;
		yLoc = firmPtr->center_y;
	}
	else if( unit_mode == UNIT_MODE_ON_SHIP )
	{
		Unit* unitPtr = unit_array[unit_mode_para];

		//### begin alex 22/10 ###//
		//xLoc = unitPtr->next_x_loc();
		//yLoc = unitPtr->next_y_loc();
		if(unitPtr->is_visible())
		{
			xLoc = unitPtr->next_x_loc();
			yLoc = unitPtr->next_y_loc();
		}
		else
		{
			err_when(unitPtr->unit_mode!=UNIT_MODE_IN_HARBOR);
			Firm *firmPtr = firm_array[unitPtr->unit_mode_para];
			xLoc = firmPtr->center_x;
			yLoc = firmPtr->center_y;
		}
		//#### end alex 22/10 ####//
	}
	else
		return 0;

	return 1;
}
//----------- End of function Unit::get_cur_loc -----------//


//----------- Begin of function Unit::get_cur_loc2 -----------//
//
// <short&> xLoc, yLoc - reference vars for returning the
//								 location of this unit
//
// return : 0 - if this unit is invisible
//				1 - if a location has been returned.
//
int Unit::get_cur_loc2(short& xLoc, short& yLoc)
{
	if( is_visible() )
	{
		xLoc = cur_x_loc();
		yLoc = cur_y_loc();
	}
	else if( unit_mode == UNIT_MODE_OVERSEE ||
				unit_mode==UNIT_MODE_CONSTRUCT ||
				unit_mode == UNIT_MODE_IN_HARBOR )
	{
		Firm* firmPtr = firm_array[unit_mode_para];

		xLoc = firmPtr->center_x;
		yLoc = firmPtr->center_y;
	}
	else if( unit_mode == UNIT_MODE_ON_SHIP )
	{
		Unit* unitPtr = unit_array[unit_mode_para];

		if( unitPtr->is_visible() )
		{
			xLoc = unitPtr->cur_x_loc();
			yLoc = unitPtr->cur_y_loc();
		}
		else
		{
			err_when(unitPtr->unit_mode!=UNIT_MODE_IN_HARBOR);
			Firm *firmPtr = firm_array[unitPtr->unit_mode_para];
			xLoc = firmPtr->center_x;
			yLoc = firmPtr->center_y;
		}
	}
	else
		return 0;

	return 1;
}
//----------- End of function Unit::get_cur_loc2 -----------//


//----------- Begin of function Unit::is_leader_in_range -----------//
// If the leader is assigned and in range, return leader_unit_recno, otherwise
// return zero for indicating the leader is not in range.
//
short Unit::is_leader_in_range()
{
	if( !leader_unit_recno )
		return 0;

	if( unit_array.is_deleted(leader_unit_recno) )
	{
		leader_unit_recno = 0;
		return 0;
	}

	Unit* leaderUnit = unit_array[leader_unit_recno];

	if( !leaderUnit->is_visible() && leaderUnit->unit_mode == UNIT_MODE_CONSTRUCT )
		return 0;

	short leaderXLoc, leaderYLoc, xLoc, yLoc;
	leaderUnit->get_cur_loc2(leaderXLoc, leaderYLoc);
	get_cur_loc2(xLoc, yLoc);

	if( leaderXLoc >= 0 && misc.points_distance(xLoc, yLoc, leaderXLoc, leaderYLoc) <= EFFECTIVE_LEADING_DISTANCE )
		return leader_unit_recno;

	return 0;
}
//----------- End of function Unit::is_leader_in_range -----------//


//----------- Begin of function Unit::add_way_point -----------//
// Add the point to the way_point_array if it is not in the array.
// Otherwise, remove the point from the way_point_array.
// 
// <short>	 x	- x coordinate of the point
// <short>	 y - y coordinate of the point
//
void Unit::add_way_point(short x, short y)
{
	if(way_point_count>=100)
		return; // too many way point

	if(way_point_count>1) // don't allow to remove the 1st node, since the unit is moving there
	{
		ResultNode *nodePtr = way_point_array + 1;
		for(int i=1; i<way_point_count; ++i, nodePtr++)
		{
			if(nodePtr->node_x == x && nodePtr->node_y == y) // remove this node
			{
				misc.del_array_rec(way_point_array, way_point_count, sizeof(ResultNode), i+1); // remove 1st node
				way_point_count--;
				return; // there should be one and only one node with the same value
			}
		}
	}

	//-------------- add new node -----------------//
	if(way_point_count>=way_point_array_size) // buffer full
	{
		way_point_array_size += WAY_POINT_ADJUST_SIZE;
		
		if(way_point_count)
			way_point_array = (ResultNode*) mem_resize(way_point_array, way_point_array_size*sizeof(ResultNode));
		else
			way_point_array = (ResultNode*) mem_add(sizeof(ResultNode)*WAY_POINT_ADJUST_SIZE);
	}

	ResultNode *nodePtr = way_point_array + way_point_count;
	nodePtr->node_x = x;
	nodePtr->node_y = y;
	way_point_count++;
	
	if(way_point_count==1)
		move_to(x, y);
}
//----------- End of function Unit::add_way_point -----------//


//----------- Begin of function Unit::reset_way_point_array -----------//
void Unit::reset_way_point_array()
{
	//------------------------------------------------------------------------------------//
	// There are nly two conditions to reset the way_point_array
	// 1) action_mode2!=ACTION_MOVE in Unit::stop()
	// 2) dest? != node_? in the first node of way_point_array in calling Unit::move_to()
	//------------------------------------------------------------------------------------//
	if(way_point_array_size)
	{
		mem_del(way_point_array);
		way_point_array = NULL;
		way_point_array_size = 0;
		way_point_count = 0;
	}
}
//----------- End of function Unit::reset_way_point_array -----------//


//----------- Begin of function Unit::process_way_point -----------//
// move to the next way point and remove this point from the
// way_point_array
//
void Unit::process_way_point()
{
	err_when(action_mode!=ACTION_STOP || action_mode2!=ACTION_STOP || cur_action!=SPRITE_IDLE);

	int destX, destY;
	if(way_point_count>1)
	{
		ResultNode *nodePtr = way_point_array+1;
		destX = nodePtr->node_x;
		destY = nodePtr->node_y;
		misc.del_array_rec(way_point_array, way_point_count, sizeof(ResultNode), 1); // remove 1st node
		way_point_count--;
	}
	else // only one unprocessed node
	{
		ResultNode *nodePtr = way_point_array;
		destX = nodePtr->node_x;
		destY = nodePtr->node_y;
		//misc.del_array_rec(way_point_array, way_point_count, sizeof(ResultNode), 1); // remove 1st node
		//way_point_count--;
	}
	move_to(destX, destY);
}
//----------- End of function Unit::process_way_point -----------//


//----------- Begin of function TeamInfo::TeamInfo ---------//

TeamInfo::TeamInfo()
{
	memset( this, 0, sizeof(TeamInfo) );
}
//----------- End of function TeamInfo::TeamInfo ---------//

