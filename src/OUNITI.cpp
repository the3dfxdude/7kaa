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

//Filename    : OUNITI.CPP
//Description : Object Unit idle processing
//Owner       : Alex

#include <ALL.h>
#include <OWORLD.h>
#include <OU_MARI.h>
#include <ONATION.h>
#include <OF_MONS.h>
#include <OTOWN.h>
#include <OSPY.h>
#include <OREBEL.h>
#include <OSYS.h>
#include <OU_GOD.h>

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

//-------------- define static variables -----------//
static char		idle_detect_has_unit;
static char		idle_detect_has_firm;
static char		idle_detect_has_town;
static char		idle_detect_has_wall;
static short	idle_detect_target_unit_recno;
static short	idle_detect_target_firm_recno;
static short	idle_detect_target_town_recno;
static short	idle_detect_target_wall_x1;
static short	idle_detect_target_wall_y1;

static int		idle_detect_default_mode;

static int		help_mode;
static short	help_attack_target_recno;

//--------- Begin of function Unit::process_idle ---------//
// process actions for idle units
//
void Unit::process_idle()
{
	err_when(result_path_dist || result_node_array);

   //---- if the unit is defending the town ----//

   switch( unit_mode )
   {
      case UNIT_MODE_REBEL:
         if(action_mode2==ACTION_STOP)
         {
            process_rebel(); // redirect to process_rebel for rebel units
            return;
         }
         break;

		case UNIT_MODE_MONSTER:
			if(action_mode2==ACTION_STOP)
			{
				if(unit_mode_para)
				{
					if(!firm_array.is_deleted(unit_mode_para))
					{
						//-------- return to monster firm -----------//
						FirmMonster *monsterFirmPtr = (FirmMonster*) firm_array[unit_mode_para];
						assign(monsterFirmPtr->loc_x1, monsterFirmPtr->loc_y1);
						return;
					}
					else
						unit_mode_para = 0;
				}
			}
			break;
   }

	//------------- process way point ------------//
	if(action_mode==ACTION_STOP && action_mode2==ACTION_STOP && way_point_count)
	{
		err_when(way_point_count < 0);
		if(way_point_count==1)
			reset_way_point_array();
		else
			process_way_point();
		return;
	}
	
   //-------- randomize direction --------//

   err_when(result_node_array!=NULL);

   //-------------------------------------------------------//
   // when the unit is idle, the following should be always true
   // move_to_x_loc == next_x_loc()
   // move_to_y_loc == next_y_loc()
   //-------------------------------------------------------//

   err_when(move_to_x_loc!=next_x_loc() || move_to_y_loc!=next_y_loc());
	//move_to_x_loc = next_x_loc();     //***************BUGHERE
   //move_to_y_loc = next_y_loc();
   err_when(next_x!=cur_x || next_y!=cur_y);

   if(match_dir())
   {
		if(!is_guarding() && race_id )     // only these units can move
		{
			if(!misc.random(150))             // change direction randomly
				set_dir(misc.random(8));
		}
	}
	else
		return;

	err_when(turn_delay);
	//------- call Sprite::process_idle() -------//

	Sprite::process_idle();

	//-*********************** simulate ship movment ***************************-//
	/*if(unit_res[unit_id]->unit_class==UNIT_CLASS_SHIP)
	{
		unit_group_id = 1;
		if(cur_action==SPRITE_IDLE)
		{
			if(misc.random(50))
				return;
		}
		else if(cur_action!=SPRITE_READY_TO_MOVE)
		{
			if(misc.random(30)==0)
				return;
		}

		int xOffset = misc.random(30)*(misc.random(2) ? 1 : -1);
		int yOffset = misc.random(30)*(misc.random(2) ? 1 : -1);
		int curXLoc = next_x_loc();
		int curYLoc = next_y_loc();
		int destXLoc = curXLoc+xOffset;
		int destYLoc = curYLoc+yOffset;

		if(destXLoc<0)	destXLoc = 0;
		else if(destXLoc>=MAX_WORLD_X_LOC)	destXLoc = MAX_WORLD_X_LOC-1;
		if(destYLoc<0) destYLoc = 0;
		else if(destYLoc>=MAX_WORLD_Y_LOC)	destYLoc = MAX_WORLD_Y_LOC-1;

		Location *locPtr = world.get_loc(destXLoc, destYLoc);
		int tempX, tempY;
		if(terrain_res[locPtr->terrain_id]->average_type==TERRAIN_OCEAN)
			move_to(destXLoc, destYLoc);
		else
			ship_to_beach(destXLoc, destYLoc, tempX, tempY);

		Nation *nationPtr = nation_array[nation_recno];
		if(nationPtr->cash<5000)
			nationPtr->cash += 10000;
		if(nationPtr->food<5000)
			nationPtr->food += 10000;
		if(hit_points<max_hit_points)
			hit_points = max_hit_points;
		return;
	}
	*/
	//-*********************** simulate ship movment ***************************-//

	//---------------------------------------------------------------------------//
	// reset idle blocked attacking unit.  If the previous condition is totally
	// blocked for attacking target, try again now
	// Note: reset blocked_edge is essentail for idle unit to reactivate attack
	// action
	//---------------------------------------------------------------------------//
	if(action_mode>=ACTION_ATTACK_UNIT && action_mode<=ACTION_ATTACK_WALL)
	{
		if(unit_array.idle_blocked_unit_reset_count && *(uint32_t*)blocked_edge)
		{
			unit_array.idle_blocked_unit_reset_count = 0;
			memset(blocked_edge, 0, sizeof(char)*4);
			err_when(blocked_edge[0] || blocked_edge[1] || blocked_edge[2] || blocked_edge[3]);
		}
	}

   err_when(action_mode==ACTION_STOP && cur_action==SPRITE_ATTACK);
   err_when(action_mode==ACTION_ATTACK_UNIT && action_para==0);

	//--------- reactivate action -----------//

	if(reactivate_idle_action())
   {
      err_when(action_mode==ACTION_STOP && cur_action==SPRITE_ATTACK);
      err_when(action_mode==ACTION_ATTACK_UNIT && action_para==0);
      return; // true if an action is reactivated
   }

	//-**************** simulate aat ********************-//
	#ifdef DEBUG
		if(debug_sim_game_type==2)
		{
			//int curXLoc = next_x_loc();
			//int curYLoc = next_y_loc();
			//int destXLoc, destYLoc;

			if(misc.random(30))
				move_to(misc.random(MAX_WORLD_X_LOC), misc.random(MAX_WORLD_Y_LOC));

			Nation *nationPtr = nation_array[nation_recno];
			if(nationPtr->cash<2000)
				nationPtr->cash += 8000;
			if(nationPtr->food<2000)
				nationPtr->food += 8000;
			return;
		}
	#endif
	//-**************** simulate aat ********************-//

   err_when(action_mode==ACTION_STOP && cur_action==SPRITE_ATTACK);
   err_when(action_mode==ACTION_ATTACK_UNIT && action_para==0);

	//----------- for ai unit idle -----------//

	if( action_mode != ACTION_STOP || action_mode2 != ACTION_STOP )		// only detect attack when the unit is really idle
		return;

	if( !can_attack() )
		return; // cannot attack

	err_when(!can_attack());
	err_when(action_mode==ACTION_STOP && cur_action==SPRITE_ATTACK);
	err_when(action_mode==ACTION_ATTACK_UNIT && action_para==0);

	//--- only detect attack if in aggressive mode or the unit is a monster ---//

	UnitInfo* unitInfo = unit_res[unit_id];

	if( unitInfo->unit_class == UNIT_CLASS_MONSTER || aggressive_mode  )
	{
		//----------- detect target to attack -----------//

		if( idle_detect_attack() )
		{
			err_when(action_mode==ACTION_STOP && cur_action==SPRITE_ATTACK);
			err_when(action_mode==ACTION_ATTACK_UNIT && action_para==0);
			return; // target detected
		}

		err_when(action_mode==ACTION_STOP && cur_action==SPRITE_ATTACK);
		err_when(action_mode==ACTION_ATTACK_UNIT && action_para==0);
	}

	//------------------------------------------------------------------//
	// wander around for monster
	//------------------------------------------------------------------//

	if( unitInfo->unit_class == UNIT_CLASS_MONSTER )
	{
		if(misc.random(500)==0)
		{
			#define WANDER_DIST 20

			int xOffset = misc.random(WANDER_DIST)-WANDER_DIST/2;
			int yOffset = misc.random(WANDER_DIST)-WANDER_DIST/2;
			int destX = next_x_loc()+xOffset;
			int destY = next_y_loc()+yOffset;

			if(destX<0) destX = 0;
			else if(destX>=MAX_WORLD_X_LOC) destX = MAX_WORLD_X_LOC-1;
			if(destY<0) destY = 0;
			else if(destY>=MAX_WORLD_Y_LOC) destY = MAX_WORLD_Y_LOC-1;
			move_to(destX, destY);
		}
	}
}
//----------- End of function Unit::process_idle -----------//


//--------- Begin of function Unit::reactivate_idle_action --------//
// resume actions for idle units
//
// return 1 if an action is reactivated
// return 0 otherwise
//
int Unit::reactivate_idle_action()
{
   if(action_mode2==ACTION_STOP)
      return 0; // return for no idle action

   if(!is_dir_correct())
      return 1; // cheating for turning the direction

	//------------------- declare parameters ----------------------//
   Location    *locPtr;
   Firm        *firmPtr;
   FirmInfo    *firmInfo;
   Unit        *unitPtr;
   UnitMarine  *shipPtr;
   SpriteInfo  *spriteInfo;
   int         canMove = 1;

   int         returnFlag = 0;
   int         curXLoc = move_to_x_loc;
   int         curYLoc = move_to_y_loc;
   int         dummyX, dummyY;

	int			hasSearch = 0;
	int			validSearch = seek_path.is_valid_searching();

	seek_path.set_status(PATH_WAIT);
	err_when(seek_path.path_status==PATH_NODE_USED_UP);
   switch(action_mode2)
   {
      case ACTION_STOP:
      case ACTION_DIE:
               return 0; // do nothing

      case ACTION_ATTACK_UNIT:
               if(!action_para2 || unit_array.is_deleted(action_para2))
                  stop2();
               else
               {
                  unitPtr = unit_array[action_para2];
                  spriteInfo = unitPtr->sprite_info;

                  if(space_for_attack(action_x_loc2, action_y_loc2, unitPtr->mobile_type, spriteInfo->loc_width, spriteInfo->loc_height))
                  {
							//------ there should be place for this unit to attack the target, attempts to attack it ------//
                     attack_unit(action_para2, 0, 0, 0); // last 0 for not reset blocked_edge
							hasSearch++;
                     returnFlag = 1;
                     break;
                  }
               }
               break;

      case ACTION_ATTACK_FIRM:
               locPtr = world.get_loc(action_x_loc2, action_y_loc2);
               if(!action_para2 || !locPtr->is_firm())
                  stop2(); // stop since target is already destroyed
               else
               {
                  firmPtr = firm_array[action_para2];
                  firmInfo = firm_res[firmPtr->firm_id];

                  if(space_for_attack(action_x_loc2, action_y_loc2, UNIT_LAND, firmInfo->loc_width, firmInfo->loc_height))
                  {
							//-------- attack target since space is found for this unit to move to ---------//
                     attack_firm(action_x_loc2, action_y_loc2, 0, 0, 0); // last 0 for not reset blocked_edge
							hasSearch++;
                     returnFlag = 1;
                     break;
                  }
               }
               break;

      case ACTION_ATTACK_TOWN:
               locPtr = world.get_loc(action_x_loc2, action_y_loc2);
               if(!action_para2 || !locPtr->is_town())
                  stop2(); // stop since target is deleted
               else if(space_for_attack(action_x_loc2, action_y_loc2, UNIT_LAND, STD_TOWN_LOC_WIDTH, STD_TOWN_LOC_HEIGHT))
               {
						//---------- attack target --------//
                  attack_town(action_x_loc2, action_y_loc2, 0, 0, 0); // last 0 for not reset blocked_edge
						hasSearch++;
                  returnFlag = 1;
                  break;
               }
               break;

      case ACTION_ATTACK_WALL:
               locPtr = world.get_loc(action_x_loc2, action_y_loc2);
               if(!locPtr->is_wall())
                  stop2(); // stop since target doesn't exist
               else if(space_for_attack(action_x_loc2, action_y_loc2, UNIT_LAND, 1, 1))
               {
						//----------- attack target -----------//
                  attack_wall(action_x_loc2, action_y_loc2, 0, 0, 0); // last 0 for not reset blocked_edge
						hasSearch++;
                  returnFlag = 1;
                  break;
               }
               break;

      case ACTION_ASSIGN_TO_FIRM:
      case ACTION_ASSIGN_TO_TOWN:
      case ACTION_ASSIGN_TO_VEHICLE:
					//---------- resume assign actions -------------//
					err_when(action_x_loc2==-1 || action_y_loc2==-1 || !action_para2);
               assign(action_x_loc2, action_y_loc2);
					hasSearch++;
               waiting_term = 0;
               returnFlag = 1;
               break;

      case ACTION_ASSIGN_TO_SHIP:
					//------------ try to assign to marine ------------//
					err_when(action_x_loc2==-1 || action_y_loc2==-1 || !action_para2);
               assign_to_ship(action_x_loc2, action_y_loc2, action_para2);
					hasSearch++;
               waiting_term = 0;
               returnFlag = 1;
               break;

      case ACTION_BUILD_FIRM:
					//-------------- build again ----------------//
					err_when(action_x_loc2==-1 || action_y_loc2==-1 || !action_para2);
               build_firm(action_x_loc2, action_y_loc2, action_para2, COMMAND_AUTO);
					hasSearch++;
               waiting_term = 0;
               returnFlag = 1;
               break;

      case ACTION_SETTLE:
					//------------- try again to settle -----------//
					err_when(action_x_loc2==-1 || action_y_loc2==-1 || action_para2);
               settle(action_x_loc2, action_y_loc2);
					hasSearch++;
               waiting_term = 0;
               returnFlag = 1;
               break;

      case ACTION_BURN:
					//---------------- resume burn action -----------------//
					err_when(action_x_loc2==-1 || action_y_loc2==-1 || action_para2);
               burn(action_x_loc2, action_y_loc2, COMMAND_AUTO);
					hasSearch++;
               waiting_term = 0;
               returnFlag = 1;
               break;

      case ACTION_MOVE:
               //if(!avail_node_enough_for_search())
               //{
               // returnFlag = 1;
               // break;
               //}

               if(move_to_x_loc!=action_x_loc2 || move_to_y_loc!=action_y_loc2)
               {
						//------- move since the unit has not reached its destination --------//
                  move_to(action_x_loc2, action_y_loc2, 1);
						hasSearch++;
                  returnFlag = 1;
                  break;
               }

               waiting_term = 0;
               break;

      case ACTION_AUTO_DEFENSE_ATTACK_TARGET:
               if(unit_search_node_used<500) // limit the no. of nods to reactivate idle process
					{
						//---------- resume action -----------//
                  process_auto_defense_attack_target();
						hasSearch++;
					}

               returnFlag = 1;
               break;

      case ACTION_AUTO_DEFENSE_DETECT_TARGET:
               process_auto_defense_detect_target();
               returnFlag = 1;
               break;

      case ACTION_AUTO_DEFENSE_BACK_CAMP:
               process_auto_defense_back_camp();
					hasSearch++;
               returnFlag = 1;
               break;

      case ACTION_DEFEND_TOWN_ATTACK_TARGET:
               process_defend_town_attack_target();
					hasSearch++;
               returnFlag = 1;
               break;

      case ACTION_DEFEND_TOWN_DETECT_TARGET:
               process_defend_town_detect_target();
               returnFlag = 1;
               break;

      case ACTION_DEFEND_TOWN_BACK_TOWN:
               process_defend_town_back_town();
					hasSearch++;
               returnFlag = 1;
               break;

      case ACTION_MONSTER_DEFEND_ATTACK_TARGET:
               process_monster_defend_attack_target();
					hasSearch++;
               returnFlag = 1;
               break;

      case ACTION_MONSTER_DEFEND_DETECT_TARGET:
               process_monster_defend_detect_target();
               returnFlag = 1;
               break;

      case ACTION_MONSTER_DEFEND_BACK_FIRM:
               process_monster_defend_back_firm();
					hasSearch++;
               returnFlag = 1;
               break;

      case ACTION_SHIP_TO_BEACH:
               shipPtr = (UnitMarine*) this;
               if(!shipPtr->in_beach || shipPtr->extra_move_in_beach==EXTRA_MOVE_FINISH)
					{
						//----------- the ship has not reached inlet, so move again --------------//
                  ship_to_beach(action_x_loc2, action_y_loc2, dummyX, dummyY);
						hasSearch++;
					}

               returnFlag = 1;
               break;

		case ACTION_GO_CAST_POWER:
					go_cast_power(action_x_loc2, action_y_loc2, ((UnitGod*)this)->cast_power_type, COMMAND_AUTO);
               returnFlag = 1;
               break;

      default: err_here();
               break;
   }

	if(validSearch && hasSearch && seek_path.path_status==PATH_NODE_USED_UP &&
		next_x_loc()==move_to_x_loc && next_y_loc()==move_to_y_loc)
	{
		//-------------------------------------------------------------------------//
		// abort actions since the unit trys to move and move no more.
		//-------------------------------------------------------------------------//
		stop2(KEEP_DEFENSE_MODE);
		return 1;
	}

   int abort=0;
   if(returnFlag)
   {
      if(curXLoc==move_to_x_loc && curYLoc==move_to_y_loc && seek_path.path_status==PATH_NODE_USED_UP)
      {
			//---------------------------------------------------------------------------------//
			// insufficient nodes for searching
			//---------------------------------------------------------------------------------//
			if(action_mode2==ACTION_ASSIGN_TO_SHIP || action_mode2==ACTION_SHIP_TO_BEACH || in_any_defense_mode())
				return 1;

         //------- number of nodes is not enough to find the destination -------//
         if(action_misc!=ACTION_MISC_STOP)
         {
            if(action_misc==ACTION_MISC_PRE_SEARCH_NODE_USED_UP)
            {
               if(action_misc_para<20)
               {
                  action_misc_para++;
                  return 0;
               }
               else
                   action_misc_para++;
            }

            abort++; // assume destination unreachable, abort action
         }
         else
         {
            action_misc = ACTION_MISC_PRE_SEARCH_NODE_USED_UP;
            action_misc_para = 0;
         }
      }
      else // action resumed, return true
         return 1;
   }

   if(!returnFlag || abort)
   {
      stop2(KEEP_DEFENSE_MODE);
   }

   return 0;
}
//----------- End of function Unit::reactivate_idle_action -----------//


//--------- Begin of function Unit::idle_detect_attack --------//
// detect target for idle units
//
// [int] startLoc       -  (default = 0), used to select region of the square
//                         for checking
// [int] dimensionInput -  (default = 0), the detected size is calculated as
//                         2*dimensionInput+1. this unit is located in
//                         the center of the square.
// [char] defenseMode   -  true for defensive mode on.
//
// return 1 if any target is detected
// return 0 otherwise
//
int Unit::idle_detect_attack(int startLoc, int dimensionInput, char defenseMode)
{
   err_when(attack_count==0);

	//---------------------------------------------------//
	// Set detectDelay.
	//
	// The larger its value, the less CPU time it will takes,
	// but it will also take longer to detect enemies.
	//---------------------------------------------------//

	int detectDelay = 1+unit_array.packed_size()/10;

	Location    *locPtr;
	Unit			*unitPtr;
	//Unit        *unitPtr, *targetUnitPtr;
	//Firm        *firmPtr, *targetFirmPtr;
	//Town        *townPtr, *targetTownPtr;
	//int         targetWallXLoc, targetWallYLoc;
	//char        hasUnit=0, hasFirm=0, hasTown=0, hasWall=0;
	char        targetMobileType;
	int         dimension, countLimit;
	short       targetRecno, i;
	int         xOffset, yOffset, checkXLoc, checkYLoc;
	idle_detect_default_mode = (!startLoc && !dimensionInput && !defenseMode); //----- true when all zero
	idle_detect_has_unit = idle_detect_has_firm = idle_detect_has_town = idle_detect_has_wall = 0;
	help_mode = HELP_NOTHING;
	
	err_when(idle_detect_default_mode!=0 && idle_detect_default_mode!=1);

	//-----------------------------------------------------------------------------------------------//
	// adjust waiting_term for default_mode
	//-----------------------------------------------------------------------------------------------//
	++waiting_term;
	waiting_term = MAX(waiting_term,0); //**BUGHERE
	int lowestBit = waiting_term%detectDelay;

	if(action_mode2==ACTION_STOP)
	{
		err_when(action_mode!=ACTION_STOP);
		waiting_term = lowestBit;
	}

	(dimension = (dimensionInput ? dimensionInput : ATTACK_DETECT_DISTANCE)<<1)++;
	countLimit = dimension*dimension;
	i = startLoc ? startLoc : 1+lowestBit;
	int incAmount = (idle_detect_default_mode) ? detectDelay : 1;

   //-----------------------------------------------------------------------------------------------//
   // check the location around the unit
   //
   // The priority to choose target is (value of targetType)
   // 1) Unit, 2) firm, 3) wall
   //-----------------------------------------------------------------------------------------------//
   err_when(defenseMode && action_mode2!=ACTION_AUTO_DEFENSE_DETECT_TARGET &&
				action_mode2!=ACTION_DEFEND_TOWN_DETECT_TARGET && action_mode2!=ACTION_MONSTER_DEFEND_DETECT_TARGET);

	err_when(incAmount<1 || incAmount>100000);
   for(; i<=countLimit; i+=incAmount) // 1 is the self location
   {
      misc.cal_move_around_a_point(i, dimension, dimension, xOffset, yOffset);
      checkXLoc = move_to_x_loc+xOffset;
      checkYLoc = move_to_y_loc+yOffset;
      if(checkXLoc<0 || checkXLoc>=MAX_WORLD_X_LOC || checkYLoc<0 || checkYLoc>=MAX_WORLD_Y_LOC)
         continue;

		//------------------ verify location ---------------//
      locPtr = world.get_loc(checkXLoc, checkYLoc);
      if(defenseMode && action_mode2!=ACTION_DEFEND_TOWN_DETECT_TARGET)
      {
         if(action_mode2==ACTION_AUTO_DEFENSE_DETECT_TARGET)
            if(locPtr->power_nation_recno!=nation_recno && locPtr->power_nation_recno)
               continue; // skip this location because it is not neutral nation or our nation
      }

      //----------------------------------------------------------------------------//
      // checking the target type
      //----------------------------------------------------------------------------//
      if((targetMobileType=locPtr->has_any_unit(i==1 ? mobile_type : UNIT_LAND)) &&
         (targetRecno=locPtr->unit_recno(targetMobileType)) && !unit_array.is_deleted(targetRecno))
      {
         //=================== is unit ======================//
         if(idle_detect_has_unit || (action_para==targetRecno && action_mode==ACTION_ATTACK_UNIT &&
            checkXLoc==action_x_loc && checkYLoc==action_y_loc))
            continue; // same target as before

			unitPtr = unit_array[targetRecno];
			if(nation_recno && unitPtr->nation_recno==nation_recno && help_mode!=HELP_ATTACK_UNIT)
				idle_detect_helper_attack(targetRecno); // help our troop
			else if((help_mode==HELP_ATTACK_UNIT && help_attack_target_recno==targetRecno) ||
					  (unitPtr->nation_recno!=nation_recno && idle_detect_unit_checking(targetRecno)))
			{
				idle_detect_target_unit_recno = targetRecno;
				idle_detect_has_unit++;
				break; // break with highest priority
			}
      }
      else if(locPtr->is_firm() && (targetRecno = locPtr->firm_recno()))
      {
         //=============== is firm ===============//
         if(idle_detect_has_firm || (action_para==targetRecno && action_mode==ACTION_ATTACK_FIRM &&
            action_x_loc==checkXLoc && action_y_loc==checkYLoc))
				continue; // same target as before

         if(idle_detect_firm_checking(targetRecno))
         {
				idle_detect_target_firm_recno = targetRecno;
				idle_detect_has_firm++;
         }
      }
      /*else if(locPtr->is_town() && (targetRecno = locPtr->town_recno()))
      {
         //=============== is town ===========//
         if(idle_detect_has_town || (action_para==targetRecno && action_mode==ACTION_ATTACK_TOWN &&
            action_x_loc==checkXLoc && action_y_loc==checkYLoc))
            continue; // same target as before

         if(idle_detect_town_checking(targetRecno))
         {
				idle_detect_target_town_recno = targetRecno;
				idle_detect_has_town++;
         }
      }
      else if(locPtr->is_wall())
      {
         //================ is wall ==============//
         if(idle_detect_has_wall || (action_mode==ACTION_ATTACK_WALL && action_para==targetRecno &&
            action_x_loc==checkXLoc && action_y_loc==checkYLoc))
            continue; // same target as before

         if(idle_detect_wall_checking(checkXLoc, checkYLoc))
         {
				idle_detect_target_wall_x1 = checkXLoc;
				idle_detect_target_wall_y1 = checkYLoc;
				idle_detect_has_wall++;
         }
      }*/

      //if(hasUnit && hasFirm && hasTown && hasWall)
      //if(hasUnit && hasFirm && hasWall)
      //  break; // there is target for attacking
   }

	return idle_detect_choose_target(defenseMode);
}
//----------- End of function Unit::idle_detect_attack -----------//


//--------- Begin of function Unit::idle_detect_unit_checking --------//
// check whether to attack the unit with recno = targetRecno
//
// <short>	targetRecno	-	recno of unit being checked
//
// return 1 if situation is suitable for attacking
// return 0 otherwise
//
int Unit::idle_detect_unit_checking(short targetRecno)
{
	Unit *targetUnitPtr = unit_array[targetRecno];

	if(targetUnitPtr->unit_id == UNIT_CARAVAN)
		return 0;

	//###### trevor 15/10 #######//

	//-------------------------------------------//
	// If the target is moving, don't attack it.
	// Only attack when the unit stands still or
	// is attacking.
	//-------------------------------------------//

	if( targetUnitPtr->cur_action != SPRITE_ATTACK &&
		 targetUnitPtr->cur_action != SPRITE_IDLE )
	{
		return 0;
	}

	//-------------------------------------------//
	// If the target is a spy of our own and the
	// notification flag is set to 0, then don't
	// attack.
	//-------------------------------------------//

	if( targetUnitPtr->spy_recno )		// if the target unit is our spy, don't attack 
	{
		Spy* spyPtr = spy_array[targetUnitPtr->spy_recno];

		if( spyPtr->true_nation_recno == nation_recno &&
			 spyPtr->notify_cloaked_nation_flag == 0 )
		{
			return 0;
		}
	}

	if( spy_recno )			// if this unit is our spy, don't attack own units
	{
		Spy* spyPtr = spy_array[spy_recno];

		if( spyPtr->true_nation_recno == targetUnitPtr->nation_recno &&
			 spyPtr->notify_cloaked_nation_flag == 0 )
		{
			return 0;
		}
	}

	//###### trevor 15/10 #######//

	SpriteInfo *spriteInfo = targetUnitPtr->sprite_info;
	Nation   *nationPtr = nation_recno ? nation_array[nation_recno] : NULL;
	short    targetNationRecno = targetUnitPtr->nation_recno;

	//-------------------------------------------------------------------//
	// checking nation relationship
	//-------------------------------------------------------------------//

	if(nation_recno)
	{
		if(targetNationRecno)
		{
			//------- don't attack own units and non-hostile units -------//
			err_when(targetNationRecno==nation_recno);

			//--------------------------------------------------------------//
			// if the unit is hostile, only attack if should_attack flag to
			// that nation is true or the unit is attacking somebody or something.
			//--------------------------------------------------------------//
			NationRelation* nationRelation = nationPtr->get_relation(targetNationRecno);

			if( nationRelation->status != NATION_HOSTILE || !nationRelation->should_attack )
				return 0;
		}
		else if(!targetUnitPtr->independent_nation_can_attack(nation_recno))
			return 0;
	}
	else if(!independent_nation_can_attack(targetNationRecno)) // independent unit
		return 0;

	//---------------------------------------------//
	if(space_for_attack(targetUnitPtr->next_x_loc(), targetUnitPtr->next_y_loc(), targetUnitPtr->mobile_type,
							  spriteInfo->loc_width, spriteInfo->loc_height))
		return 1;
	else
		return 0;
}
//----------- End of function Unit::idle_detect_unit_checking -----------//


//--------- Begin of function Unit::idle_detect_firm_checking --------//
// check whether to attack the firm
//
// <short>	targetRecno	-	recno of the firm being checked
//
// return 1 if situation is suitable
// return 0 otherwise
//
int Unit::idle_detect_firm_checking(short targetRecno)
{
   Firm *firmPtr = firm_array[targetRecno];

	//------------ code to select firm for attacking -----------//
	switch(firmPtr->firm_id)
	{
		case FIRM_CAMP:case FIRM_BASE: case FIRM_WAR_FACTORY:
				break;
		
		default: return 0;
	}

   Nation   *nationPtr = nation_recno ? nation_array[nation_recno] : NULL;
   short    targetNationRecno = firmPtr->nation_recno;
   char     targetMobileType = mobile_type==UNIT_SEA ? UNIT_SEA : UNIT_LAND;

   //-------------------------------------------------------------------------------//
   // checking nation relationship
   //-------------------------------------------------------------------------------//
   if(nation_recno)
   {
      if(targetNationRecno)
      {
			//------- don't attack own units and non-hostile units -------//

			if( targetNationRecno==nation_recno )
				return 0;

			//--------------------------------------------------------------//
			// if the unit is hostile, only attack if should_attack flag to
			// that nation is true or the unit is attacking somebody or something.
			//--------------------------------------------------------------//

			NationRelation* nationRelation = nationPtr->get_relation(targetNationRecno);

			if( nationRelation->status != NATION_HOSTILE || !nationRelation->should_attack )
				return 0;
      }
      else // independent firm
      {
         FirmMonster *monsterFirmPtr = (FirmMonster*) firm_array[targetRecno];

			if(!monsterFirmPtr->is_hostile_nation(nation_recno))
            return 0;
      }
   }
   else if(!independent_nation_can_attack(targetNationRecno)) // independent town
      return 0;

   FirmInfo *firmInfo = firm_res[firmPtr->firm_id];
   if(space_for_attack(firmPtr->loc_x1, firmPtr->loc_y1, UNIT_LAND, firmInfo->loc_width, firmInfo->loc_height))
      return 1;
   else
      return 0;
}
//----------- End of function Unit::idle_detect_firm_checking -----------//


//--------- Begin of function Unit::idle_detect_town_checking --------//
// check town to attack
//
// <short>	targetRecno	-	recno of town
//
// return 1 if situation is suitable for attacking
// return 0 otherwise
//
int Unit::idle_detect_town_checking(short targetRecno)
{
   Town     *townPtr = town_array[targetRecno];
   Nation   *nationPtr = nation_recno ? nation_array[nation_recno] : NULL;
   short    targetNationRecno = townPtr->nation_recno;

   //-------------------------------------------------------------------------------//
   // checking nation relationship
   //-------------------------------------------------------------------------------//
   if(nation_recno)
   {
      if(targetNationRecno)
      {
			//------- don't attack own units and non-hostile units -------//

			if( targetNationRecno==nation_recno )
				return 0;

			//--------------------------------------------------------------//
			// if the unit is hostile, only attack if should_attack flag to
			// that nation is true or the unit is attacking somebody or something.
			//--------------------------------------------------------------//

			NationRelation* nationRelation = nationPtr->get_relation(targetNationRecno);

			if( nationRelation->status != NATION_HOSTILE || !nationRelation->should_attack )
				return 0;
      }
		else if(!townPtr->is_hostile_nation(nation_recno))
			return 0; // false if the indepentent unit don't want to attack us
   }
   else if(!independent_nation_can_attack(targetNationRecno)) // independent town
      return 0;

   if(space_for_attack(townPtr->loc_x1, townPtr->loc_y1, UNIT_LAND, STD_TOWN_LOC_WIDTH, STD_TOWN_LOC_HEIGHT))
      return 1;
   else
      return 0;
}
//----------- End of function Unit::idle_detect_town_checking -----------//


//--------- Begin of function Unit::idle_detect_wall_checking --------//
// check wall to attack
//
// <int>	targetXLoc	-	wall x location
//	<int>	targetYLoc	-	wall y location
//
// return 1 if situation is suitable for attacking
// return 0 otherwise
//
int Unit::idle_detect_wall_checking(int targetXLoc, int targetYLoc)
{
   Location *locPtr = world.get_loc(targetXLoc, targetYLoc);
   Nation   *nationPtr = nation_recno ? nation_array[nation_recno] : NULL;
   short    targetNationRecno = locPtr->wall_nation_recno();

   //-------------------------------------------------------------------------------//
   // checking nation relationship
   //-------------------------------------------------------------------------------//
   if(nation_recno)
   {
      if(targetNationRecno)
      {
			//------- don't attack own units and non-hostile units -------//

			if( targetNationRecno==nation_recno )
				return 0;

			//--------------------------------------------------------------//
			// if the unit is hostile, only attack if should_attack flag to
			// that nation is true or the unit is attacking somebody or something.
			//--------------------------------------------------------------//

			NationRelation* nationRelation = nationPtr->get_relation(targetNationRecno);

			if( nationRelation->status != NATION_HOSTILE || !nationRelation->should_attack )
				return 0;
      }
      else
         return 0;
   }
   else if(!independent_nation_can_attack(targetNationRecno)) // independent town
      return 0;

   if(space_for_attack(targetXLoc, targetYLoc, UNIT_LAND, 1, 1))
      return 1;
   else
      return 0;
}
//----------- End of function Unit::idle_detect_wall_checking -----------//


//--------- Begin of function Unit::idle_detect_choose_target --------//
//
// <char>	defenseMode -	indicate whether defensive mode is on
//
int Unit::idle_detect_choose_target(char defenseMode)
{
   //-----------------------------------------------------------------------------------------------//
   // Decision making for choosing target to attack
	//-----------------------------------------------------------------------------------------------//
   if(defenseMode)
   {
      if(action_mode2==ACTION_AUTO_DEFENSE_DETECT_TARGET)
      {
			//----------- defense units allow to attack units and firms -----------//
         err_when(!in_auto_defense_mode());

         if(idle_detect_has_unit)
            defense_attack_unit(idle_detect_target_unit_recno);
         else if(idle_detect_has_firm)
			{
				Firm *targetFirmPtr = firm_array[idle_detect_target_firm_recno];
            defense_attack_firm(targetFirmPtr->loc_x1, targetFirmPtr->loc_y1);
			}
         /*else if(idle_detect_has_town)
			{
				TownPtr *targetTownPtr = town_array[idle_detect_target_town_recno];
            defense_attack_town(targetTownPtr->loc_x1, targetTownPtr->loc_y1);
			}
         else if(idle_detect_has_wall)
				defense_attack_wall(idle_detect_target_wall_x1, idle_detect_target_wall_y1);*/
         else
            return 0;

         return 1;
      }
      else if(action_mode2==ACTION_DEFEND_TOWN_DETECT_TARGET)
      {
			//----------- town units only attack units ------------//
         err_when(!in_defend_town_mode());

         if(idle_detect_has_unit)
            defend_town_attack_unit(idle_detect_target_unit_recno);
         else
            return 0;

         return 1;
      }
      else if(action_mode2==ACTION_MONSTER_DEFEND_DETECT_TARGET)
      {
			//---------- monsters can attack units and firms -----------//
			err_when(!in_monster_defend_mode());

         if(idle_detect_has_unit)
            monster_defend_attack_unit(idle_detect_target_unit_recno);
         else if(idle_detect_has_firm)
			{
				Firm *targetFirmPtr = firm_array[idle_detect_target_firm_recno];
            monster_defend_attack_firm(targetFirmPtr->loc_x1, targetFirmPtr->loc_y1);
			}
         /*else if(idle_detect_has_town)
			{
				Town *targetTownPtr = town_array[idle_detect_target_town_recno];
            monster_defend_attack_town(targetTownPtr->loc_x1, targetTownPtr->loc_y1);
			}
         else if(idle_detect_has_wall)
            monster_defend_attack_wall(idle_detect_target_wall_x1, idle_detect_target_wall_y1);*/
         else
            return 0;

			return 1;
		}
		else
			err_here();
	}
	else // default mode
	{
		//#### begin trevor 9/10 ####//

		int rc = 0;

		if(idle_detect_has_unit)
		{
			attack_unit(idle_detect_target_unit_recno);

			//--- set the original position of the target, so the unit won't chase too far away ---//

			Unit* unitPtr = unit_array[idle_detect_target_unit_recno];

			original_target_x_loc = unitPtr->next_x_loc();
			original_target_y_loc = unitPtr->next_y_loc();

			rc = 1;
		}

		else if(help_mode==HELP_ATTACK_UNIT)
		{
			attack_unit(help_attack_target_recno);

			//--- set the original position of the target, so the unit won't chase too far away ---//

			Unit* unitPtr = unit_array[help_attack_target_recno];

			original_target_x_loc = unitPtr->next_x_loc();
			original_target_y_loc = unitPtr->next_y_loc();

			rc = 1;
		}
		else if(idle_detect_has_firm)
		{
			Firm *targetFirmPtr = firm_array[idle_detect_target_firm_recno];
			attack_firm(targetFirmPtr->loc_x1, targetFirmPtr->loc_y1);
		}
		/*else if(idle_detect_has_town)
		{
			Town *targetTownPtr = town_array[idle_detect_target_town_recno];
			attack_town(targetTownPtr->loc_x1, targetTownPtr->loc_y1);
		}
		else if(idle_detect_has_wall)
			attack_wall(idle_detect_target_wall_x1, idle_detect_target_wall_y1);*/
		else
			return 0;

		//---- set original action vars ----//

		if( rc && original_action_mode==0 )
		{
			original_action_mode  = ACTION_MOVE;
			original_action_para  = 0;
			original_action_x_loc = next_x_loc();
			original_action_y_loc = next_y_loc();
		}

		return 1;

		//#### end trevor 9/10 ####//
	}

	return 0;
}
//----------- End of function Unit::idle_detect_choose_target -----------//


//--------- Begin of function Unit::idle_detect_helper_attack --------//
// check the action_mode of the unit being checked, which has same nation
// recno as this unit. If the unit attacks other unit, this unit help to
// attack the same target.
//
// <short>	unitRecno	-	recno of the unit of nation recno same as this unit
//
void Unit::idle_detect_helper_attack(short unitRecno)
{
	#define HELP_DISTANCE	15

   Unit *unitPtr = unit_array[unitRecno];
   if(unitPtr->unit_id == UNIT_CARAVAN)
      return;

	//char	actionMode;
	short	actionPara;
	//short actionXLoc, actionYLoc;
	char	isUnit = 0;

	//------------- is the unit attacking other unit ------------//
	switch(unitPtr->action_mode2)
	{
		case ACTION_ATTACK_UNIT:
				actionPara = unitPtr->action_para2;
				isUnit++;
				break;

		default:
				switch(unitPtr->action_mode)
				{
					case ACTION_ATTACK_UNIT:
							actionPara = unitPtr->action_para;
							isUnit++;
							break;
				}
	}

	if(isUnit && !unit_array.is_deleted(actionPara))
	{
		Unit *targetUnit = unit_array[actionPara];

		if(targetUnit->nation_recno==nation_recno)
			return;

		// the targetUnit this unitPtr is attacking may have entered a
		// building by now due to processing order -- skip this one
		if(!targetUnit->is_visible())
			return;

		if(misc.points_distance(next_x_loc(), next_y_loc(), targetUnit->next_x_loc(), targetUnit->next_y_loc())<HELP_DISTANCE)
		{
			if(idle_detect_unit_checking(actionPara))
			{
				help_attack_target_recno = actionPara;
				help_mode = HELP_ATTACK_UNIT;
			}

			memset(blocked_edge, 0, sizeof(blocked_edge));
		}
	}
}
//----------- End of function Unit::idle_detect_helper_attack -----------//
