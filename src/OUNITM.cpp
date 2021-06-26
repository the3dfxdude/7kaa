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

//Filename    : OUNITM.CPP
//Description : Object Unit movement
//Owner		  : Alex

#include <ALL.h>
#include <OWORLD.h>
#include <OF_HARB.h>
#include <OGAME.h>
#include <ONATION.h>
#include <OU_MARI.h>
#include <OSPATH.h>
#include <OSPREUSE.h>
#include <OSERES.h>
#include <OLOG.h>
#include <OEFFECT.h>

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

//-*********** simulate aat ************-//
#ifdef DEBUG
#include <OSYS.h>
#endif
//-*********** simulate aat ************-//

//--- Define no. of pixels per direction move (N, NE, E, SE, S, SW, W, NW) ---//

static short move_x_pixel_array[] = { 0, ZOOM_LOC_WIDTH, ZOOM_LOC_WIDTH, ZOOM_LOC_WIDTH, 0, -ZOOM_LOC_WIDTH, -ZOOM_LOC_WIDTH, -ZOOM_LOC_WIDTH };
static short move_y_pixel_array[] = { -ZOOM_LOC_HEIGHT, -ZOOM_LOC_HEIGHT, 0, ZOOM_LOC_HEIGHT, ZOOM_LOC_HEIGHT, ZOOM_LOC_HEIGHT, 0, -ZOOM_LOC_HEIGHT };

static short cycle_wait_unit_index;
static short *cycle_wait_unit_array;
static short cycle_wait_unit_array_def_size;
static short cycle_wait_unit_array_multipler;


static char	 move_action_call_flag=0; // avoid calling move_to_my_loc() if this function is called from move_to() chain


//--------- Begin of function Unit::reset_action_para ---------//
// reset action parameters when action is finished or cancelled
// for action_mode
//
void Unit::reset_action_para()
{
	action_mode	= ACTION_STOP;
	action_x_loc = action_y_loc = -1;
	action_para = 0;
}
//----------- End of function Unit::reset_action_para -----------//


//--------- Begin of function Unit::reset_action_para2 ---------//
// reset action parameters when action is finished or cancelled
//
// <int>	keepMode	-	use to keep action
//
void Unit::reset_action_para2(int keepMode)
{
	if(keepMode!=KEEP_DEFENSE_MODE || !in_any_defense_mode())
	{
		action_mode2 = ACTION_STOP;
		action_para2 = 0;
		action_x_loc2 = action_y_loc2 = -1;	
	}
	else
	{
		switch(unit_mode)
		{
			case UNIT_MODE_DEFEND_TOWN:
					if(action_mode2!=ACTION_AUTO_DEFENSE_DETECT_TARGET)
						defend_town_detect_target();
					break;

			case UNIT_MODE_REBEL:
					if(action_mode2!=ACTION_DEFEND_TOWN_DETECT_TARGET)
						defense_detect_target();
					break;

			case UNIT_MODE_MONSTER:
					if(action_mode2!=ACTION_MONSTER_DEFEND_DETECT_TARGET)
						monster_defend_detect_target();
					break;
		}
	}
}
//----------- End of function Unit::reset_action_para2 -----------//


//--------- Begin of function Unit::reset_action_misc_para ---------//
// reset miscellaneous action parameters
//
void Unit::reset_action_misc_para()
{
	action_misc = ACTION_MISC_STOP;
	action_misc_para = 0;
}
//----------- End of function Unit::reset_action_misc_para -----------//


//--------- Begin of function Unit::stop ---------//
// stop unit action
//
// [int] preserveAction - preserve the current action or not.
//                        (default: 0)
//
void Unit::stop(int preserveAction)
{
	//------------- reset vars ------------//
	if(action_mode2!=ACTION_MOVE)
		reset_way_point_array();

   reset_path();
	err_when(result_node_array!=NULL);

	//-------------- keep action or not --------------//
	switch(preserveAction)
	{
		case 0: case KEEP_DEFENSE_MODE:
					reset_action_para();
					range_attack_x_loc = range_attack_y_loc = -1; // should set here or reset for attack
					break;
		
		case KEEP_PRESERVE_ACTION:
					break;

		/*case 3:	err_when(action_mode!=ACTION_ATTACK_UNIT);
					go_x = next_x;	// combine move_to_attack() into move_to()
					go_y = next_y;
					move_to_x_loc = next_x_loc();
					move_to_y_loc = next_y_loc();
					return;*/
	}
	
	waiting_term = 0; // for idle_detect_attack(), oscillate between 0 and 1
	err_when(cur_dir<0 || cur_dir>MAX_SPRITE_DIR_TYPE);
	err_when(final_dir<0 || final_dir>MAX_SPRITE_DIR_TYPE);
	UnitMarine *shipPtr;

	//----------------- update parameters ----------------//
	switch( cur_action )
   {
      //----- if the unit is moving right now, ask it to stop as soon as possible -----//

		case SPRITE_READY_TO_MOVE:
			set_idle();
			break;
	
		case SPRITE_TURN:
		case SPRITE_WAIT:
			go_x = next_x;
			go_y = next_y;
			move_to_x_loc = next_x_loc();
			move_to_y_loc = next_y_loc();
			final_dir = cur_dir;
			turn_delay = 0;
			set_idle();
			break;

		case SPRITE_SHIP_EXTRA_MOVE:
			shipPtr = (UnitMarine*) this;
			switch(shipPtr->extra_move_in_beach)
			{
				case NO_EXTRA_MOVE:
						if(cur_x==next_x && cur_y==next_y)
						{
							go_x = next_x;
							go_y = next_y;
							move_to_x_loc = next_x_loc();
							move_to_y_loc = next_y_loc();
							set_idle();
							return;
						}
						break;

				case EXTRA_MOVING_IN:
						if(cur_x==next_x && cur_y==next_y && (cur_x!=go_x || cur_y!=go_y))
						{
							shipPtr->extra_move_in_beach = NO_EXTRA_MOVE; // not yet move although location is chosed
							err_when(next_x_loc()%2 || next_y_loc()%2); // in even location
						}
						else
							err_when(next_x_loc()%2==0 && next_y_loc()%2==0); // in even location
						break;

				case EXTRA_MOVING_OUT:
						if(cur_x==next_x && cur_y==next_y && (cur_x!=go_x || cur_y!=go_y))
						{
							shipPtr->extra_move_in_beach = EXTRA_MOVE_FINISH; // not yet move although location is chosed
							err_when(next_x_loc()%2==0 && next_y_loc()%2==0); // not in even location
						}
						else
							err_when(next_x_loc()%2 || next_y_loc()%2); // in even location
						break;


				case EXTRA_MOVE_FINISH:
						break;
			}

			go_x = next_x;
         go_y = next_y;
			move_to_x_loc = next_x_loc();
			move_to_y_loc = next_y_loc();
			break;

      case SPRITE_MOVE:
			go_x = next_x;
         go_y = next_y;
			move_to_x_loc = next_x_loc();
			move_to_y_loc = next_y_loc();
			if(cur_x==next_x && cur_y==next_y)
				set_idle();
         break;
		
      //--- if its current action is SPRITE_ATTACK, stop immediately ---//

      case SPRITE_ATTACK:
			set_next(cur_x, cur_y, 0, 1);		//********** BUGHERE
			go_x = next_x;
			go_y = next_y;
			move_to_x_loc = next_x_loc();
			move_to_y_loc = next_y_loc();
			set_idle();

			#ifdef DEBUG
				char h, w, blocked=0;
				short x, y;

				for(h=0, y=next_y_loc(); h<sprite_info->loc_height&&!blocked; h++, y++)
				{
					for(w=0, x=next_x_loc(); w<sprite_info->loc_width&&!blocked; w++, x++)
						err_when(world.get_unit_recno(x, y, mobile_type) != sprite_recno);
				}
			#endif
			cur_frame  = 1;
			break;
   }
}
//----------- End of function Unit::stop -----------//


//--------- Begin of function Unit::stop2 ---------//
// stop unit action
//
// preserverAction can be
// 0, KEEP_PRESERVE_ACTION, KEEP_DEFENSE_MODE (default 0)
//
void Unit::stop2(int preserveAction)
{
	stop(preserveAction);
	reset_action_para2(preserveAction);

	//----------------------------------------------------------//
	// set the original location of the attacking target when
	// the attack() function is called, action_x_loc2 & action_y_loc2
	// will change when the unit move, but these two will not.
	//----------------------------------------------------------//

   //##### begin trevor 13/10 #####//

	force_move_flag = 0;
	ai_no_suitable_action = 0;

	if( preserveAction==0 )
	{
		original_action_mode = 0;
		ai_original_target_x_loc = -1;

		if( ai_action_id )
			nation_array[nation_recno]->action_failure(ai_action_id, sprite_recno);
	}

	//##### end trevor 13/10 #####//
}
//----------- End of function Unit::stop2 -----------//

//######## begin trevor 26/4 ###########//

//--------- Begin of function Unit::set_search_tries ---------//
// used to limit the number of nodes in searching
//
// <int> tries	-	number of nodes used in searhcing
//
void Unit::set_search_tries(int tries)
{
	unit_search_tries = tries;
	unit_search_tries_flag++;
}
//----------- End of function Unit::set_search_tries -----------//


//--------- Begin of function Unit::reset_search_tries ---------//
// reset the number of node to default value
//
void Unit::reset_search_tries()
{
	unit_search_tries = 0;		// 0 for reset
	unit_search_tries_flag = 0;
}
//----------- End of function Unit::reset_search_tries -----------//

//######## end trevor 26/4 ###########//

//-------- Begin of function Unit::abort_searching --------//
//
// <int> reuseSetNext	-	condition flag for updating parameters
//									of path_reuse
//
void	Unit::abort_searching(int reuseSetNext)
{
	if(reuseSetNext)	// to avoid error in path-reuse
		seek_path_reuse.set_next_cur_path_num();

	if(unit_search_tries_flag)
		reset_search_tries();
}
//-------- End of function Unit::abort_searching ---------//


//--------- Begin of function Unit::enable_force_move ---------//
void Unit::enable_force_move()
{
	force_move_flag = 1;
}
//-------- End of function Unit::enable_force_move ---------//


//--------- Begin of function Unit::disable_force_move ---------//
void Unit::disable_force_move()
{
	force_move_flag = 0;
}
//-------- End of function Unit::disable_force_move ---------//


//--------- Begin of function Unit::move_to ---------//
// Main function for action_mode = ACTION_MOVE
//
// Order the unit to move to a specific location following the shortest path.
//
// <int>		destX					-	x location of the destination
// <int>		destY					-	y location of the destination
// [int]		preserveAction		-	preserve the current action or not (default: 0)
// [short]	searchMode			-	the search mode used (default: SEARCH_MODE_IN_A_GROUP)
// [short]	miscNo				-	= target record no if search_mode=SEARCH_MODE_TO_ATTACK
//											= firm ID if search_mode=SEARCH_MODE_TO_FIRM (default: 0)
//	[short]	numOfPath			-	num of path, used in path reuse, (default: 1)
//	[short]	reuseMode			-	path reuse mode (default: GENERAL_GROUP_MOVEMENT)
//	[short]	pathReuseStatus	-	path reuse status (default: 0)
//
void Unit::move_to(int destX, int destY, int preserveAction, short searchMode, short miscNo, short numOfPath, short reuseMode, short pathReuseStatus)
{
	err_when(destX<0 || destX>=MAX_WORLD_X_LOC || destY<0 || destY>=MAX_WORLD_Y_LOC);

	if(!seek_path.total_node_avail)
	{
		//-------- insufficient nodes for searching, return now ----------//
		stop(KEEP_PRESERVE_ACTION);
		action_mode = action_mode2 = ACTION_MOVE;
		action_para = action_para2 = 0;
		action_x_loc = action_x_loc2 = destX;
		action_y_loc = action_y_loc2 = destY;
		return; // for later searching
	}

	//int useClosestNode = (seek_path.total_node_avail>=MIN_BACKGROUND_NODE_USED_UP);

	//---------- reset way point array since new action is assigned --------//
	if(way_point_count)
	{
		ResultNode *nodePtr = way_point_array;
		if(nodePtr->node_x!=destX || nodePtr->node_y!=destY)
			reset_way_point_array();
	}

	//----------------------------------------------------------------//
	// calculate new destination if trying to move to different territory
	//----------------------------------------------------------------//
	int curXLoc = next_x_loc();
	int curYLoc = next_y_loc();
	Location *locPtr = world.get_loc(curXLoc, curYLoc);
	Location *destLocPtr = world.get_loc(destX, destY);
	int destXLoc = destX;
	int destYLoc = destY;

	if(locPtr->region_id!=destLocPtr->region_id && mobile_type!=UNIT_AIR) // different territory
		different_territory_destination(destXLoc, destYLoc);
	
	//----------------------------------------------------------------//
	// for path_reuse initialization
	//----------------------------------------------------------------//
	if(numOfPath!=1 && pathReuseStatus==REUSE_PATH_INITIAL)	// for path-reuse only
	{
		search(destXLoc, destYLoc, preserveAction, searchMode, miscNo, numOfPath, reuseMode, pathReuseStatus);
		return;
	}

	//----------------------------------------------------------------//
	// return if the unit is dead
	//----------------------------------------------------------------//
	if(is_unit_dead())
	{
		abort_searching(searchMode==SEARCH_MODE_REUSE && numOfPath>1);
		return;
	}
	
	//-----------------------------------------------------------------------------------//
	// The codes here is used to check for equal action in movement.
	//
	// mainly checked by action_mode2. If previous action is ACTION_MOVE, action_mode2,
	// action_para2, action_x_loc2 and action_x_loc2 need to be kept for this checking.
	//
	// If calling from unit_array.move_to(), action_mode is set to ACTION_MOVE, action_para
	// is set to 0 while action_x_loc and action_y_loc are kept as original value for checking.
	// Meanwhile, action_mode2, action_para2, action_x_loc2 and action_y_loc2 are kept if
	// the condition is fulfilled (action_mode2==ACTION_MOVE)
	//-----------------------------------------------------------------------------------//
	if(action_mode2==ACTION_MOVE && action_mode==ACTION_MOVE)
	{
		//------ previous action is ACTION_MOVE -------//
		err_when(action_para2 || action_para);
		if(action_x_loc2==destXLoc && action_y_loc2==destYLoc)
		{
			//-------- equal order --------//
			action_x_loc = action_x_loc2;
			action_y_loc = action_y_loc2;

			if(cur_action!=SPRITE_IDLE)
			{
				//-------- the old order is processing --------//
				abort_searching(searchMode==SEARCH_MODE_REUSE && numOfPath>1);

				if(result_node_array==NULL) // cannot move
				{
					err_when(result_path_dist);
					if(unit_res[unit_id]->unit_class==UNIT_CLASS_SHIP)
					{
						if(cur_action!=SPRITE_SHIP_EXTRA_MOVE)
						{
							err_when(result_node_count || result_node_recno);
							if(cur_x!=next_x || cur_y!=next_y)
								set_move();
							else
								set_idle();
						}
						//else keep extra_moving
					}
					else
					{
						err_when(result_node_count || result_node_recno);
						if(cur_x!=next_x || cur_y!=next_y)
							set_move();
						else
							set_idle();
					}
				}

				err_when(action_mode==ACTION_MOVE && (action_x_loc==-1 || action_y_loc==-1));
				return;
			}//else action is hold due to some problems, re-activiate again
		}
	}//else, new order or searching is required
		
	move_action_call_flag++; // set flag to avoid calling move_to_my_loc()
	seek_path_reuse.set_status(PATH_WAIT);
	err_when(seek_path_reuse.get_reuse_path_status()==REUSE_PATH_INCOMPLETE_SEARCH);

	action_mode2 = ACTION_MOVE;
	action_para2 = 0;
	
	int enoughNode = search(destXLoc, destYLoc, preserveAction, searchMode, miscNo, numOfPath, reuseMode, pathReuseStatus);
	move_action_call_flag = 0; // clear the flag

	//----------------------------------------------------------------//
	// store new order in action parameters
	//----------------------------------------------------------------//
	action_mode = ACTION_MOVE;
	action_para = 0;
	
	if(!enoughNode || (searchMode==SEARCH_MODE_REUSE && seek_path_reuse.get_reuse_path_status()==REUSE_PATH_INCOMPLETE_SEARCH))
	{
		action_x_loc = action_x_loc2 = destXLoc;
		action_y_loc = action_y_loc2 = destYLoc;
	}
	else // enough node for search
	{
		action_x_loc = action_x_loc2 = move_to_x_loc;
		action_y_loc = action_y_loc2 = move_to_y_loc;
	}

	#ifdef DEBUG
		if(result_node_array && result_node_count)
		{
			ResultNode *debugPtr = result_node_array + result_node_count - 1;
			err_when(debugPtr->node_x != move_to_x_loc || debugPtr->node_y != move_to_y_loc);
		}
		else
		{
			UnitInfo* unitInfo = unit_res[unit_id];

			if( unitInfo->unit_class == UNIT_CLASS_SHIP )
			{
				UnitMarine *shipPtr = (UnitMarine*) this;
				if(shipPtr->extra_move_in_beach==NO_EXTRA_MOVE)
					err_when(curXLoc!=move_to_x_loc || curYLoc!=move_to_y_loc);
			}
			else
			{
				err_when(curXLoc!=move_to_x_loc || curYLoc!=move_to_y_loc);
			}
		}
	#endif
	err_when(action_mode==ACTION_MOVE && (action_x_loc==-1 || action_y_loc==-1));
	err_when(move_to_x_loc<0 || move_to_x_loc>=MAX_WORLD_X_LOC || move_to_y_loc<0 || move_to_y_loc>=MAX_WORLD_Y_LOC);
	err_when(cur_action==SPRITE_IDLE && (move_to_x_loc!=next_x_loc() || move_to_y_loc!=next_y_loc()));
	err_when(cur_action==SPRITE_IDLE && (cur_x!=next_x || cur_y!=next_y));
	err_when(action_mode2==ACTION_MOVE && (action_x_loc2==-1 || action_y_loc2==-1));
}
//----------- End of function Unit::move_to -----------//


//--------- Begin of function Unit::search ---------//
// This function is only used to find the shorest path for
// searching.  Action parameters should not be changed in
// this function.
//
//	<int>		destX					-	x coordinate of destination
//	<int>		destY					-	y coordinate of destination
//	<int>		preserveAction		-	used to keep action in calling stop()
//	<short>	searchMode			-	search mode being used
//	<short>	miscNo				-	see move_to()
// <short>	numOfPath			-	num of path
//	<short>	reuseMode			-	path reuse mode being used
//	<short>	pathReuseStatus	-	path reuse status
//
// return 1 for normal operation process
// return 0 otherwise or  there is not enough node for searching
//
int Unit::search(int destXLoc, int destYLoc, int preserveAction, short searchMode, short miscNo, short numOfPath, short reuseMode, short pathReuseStatus)
{
	#ifdef DEBUG
		err_when(destXLoc<0 || destXLoc>=MAX_WORLD_X_LOC || destYLoc<0 || destYLoc>=MAX_WORLD_Y_LOC);
		err_when(searchMode==SEARCH_MODE_TO_FIRM && miscNo!=FIRM_HARBOR && mobile_type!=UNIT_AIR &&
					world.get_loc(next_x_loc(), next_y_loc())->region_id!=world.get_loc(destXLoc, destYLoc)->region_id);
		err_when(hit_points<=0 || action_mode==ACTION_DIE || cur_action==SPRITE_DIE);
	#else
		if(destXLoc<0 || destXLoc>=MAX_WORLD_X_LOC || destYLoc<0 || destYLoc>=MAX_WORLD_Y_LOC || hit_points<=0 ||
			action_mode==ACTION_DIE || cur_action==SPRITE_DIE || searchMode<=0 || searchMode>MAX_SEARCH_MODE_TYPE)
		{
			stop2(KEEP_DEFENSE_MODE); //-********** BUGHERE, err_handling for retailed version
			return 1;
		}
	#endif
	
	//----------------------------------------------------------------//
	// for path_reuse initialization
	//----------------------------------------------------------------//
	if(numOfPath!=1 && pathReuseStatus==REUSE_PATH_INITIAL)	// for path-reuse only
	{
		seek_path_reuse.seek(next_x_loc(), next_y_loc(), move_to_x_loc, move_to_y_loc, sprite_info->loc_width,
									unit_group_id, mobile_type, searchMode, miscNo, numOfPath, reuseMode, pathReuseStatus);
		return 1;
	}
	
	int result=0;
	if(unit_res[unit_id]->unit_class==UNIT_CLASS_SHIP)
	{
		UnitMarine *shipPtr = (UnitMarine*) this;
		switch(shipPtr->extra_move_in_beach)
		{
			case NO_EXTRA_MOVE:
					result = searching(destXLoc, destYLoc, preserveAction, searchMode, miscNo, numOfPath, reuseMode, pathReuseStatus);
					break;

			case EXTRA_MOVING_IN:
					err_when(next_x_loc()%2==0 && next_y_loc()%2==0);
					if(pathReuseStatus==REUSE_PATH_SEARCH || pathReuseStatus==REUSE_PATH_FIRST_SEEK)
						seek_path_reuse.set_next_cur_path_num();
					return 0;

			case EXTRA_MOVING_OUT:
					err_when(next_x_loc()%2 || next_y_loc()%2);
					if(pathReuseStatus==REUSE_PATH_SEARCH || pathReuseStatus==REUSE_PATH_FIRST_SEEK)
						seek_path_reuse.set_next_cur_path_num();
					return 0;

			case EXTRA_MOVE_FINISH:
					err_when(next_x_loc()%2==0 && next_y_loc()%2==0);
					if(pathReuseStatus==REUSE_PATH_SEARCH || pathReuseStatus==REUSE_PATH_FIRST_SEEK)
						seek_path_reuse.set_next_cur_path_num();

					ship_leave_beach(next_x_loc(), next_y_loc());
					break;

			default: err_here();
						break;
		}
	}
	else
		result = searching(destXLoc, destYLoc, preserveAction, searchMode, miscNo, numOfPath, reuseMode, pathReuseStatus);
	
	if(way_point_count && !result_node_array) // can move no more
		reset_way_point_array();

	if(!result)
		return 0; // not enough node or extra_move_in_beach!=NO_EXTRA_MOVE
	else
		return 1;
}
//----------- End of function Unit::search -----------//


//--------- Begin of function Unit::select_search_sub_mode ---------//
// select searching sub_mode
//
//	<int>		sx				-	start location x
//	<int>		sy				-	start location y
// <int>		dx				-	x coordinate of destination
//	<int>		dy				-	y coordinate of destination
//	<short>	nationRecno	-	nation recno of the unit
//	<short>	searchMode	-	search mode being used
//
void Unit::select_search_sub_mode(int sx, int sy, int dx, int dy, short nationRecno, short searchMode)
{
	//seek_path.set_sub_mode(); // cancel the selection
	//return;

	err_when(mobile_type!=UNIT_LAND);

	if(!nation_recno || ignore_power_nation)
	{
		seek_path.set_sub_mode(); // always using normal mode for independent unit
		seek_path_reuse.set_sub_mode();
		return;
	}

	//--------------------------------------------------------------------------------//
	// Checking for starting location and destination to determine sub_mode used
	// N - not hostile, H - hostile
	// 1) N -> N, using normal mode
	// 2) N -> H, H -> N, H -> H, using sub_mode SEARCH_SUB_MODE_PASSABLE
	//--------------------------------------------------------------------------------//
	Location *startLocPtr = world.get_loc(sx, sy);
	Location *destLocPtr = world.get_loc(dx, dy);
	Nation *nationPtr = nation_array[nationRecno];
	int subModeOn = 1;
	
	if((startLocPtr->power_nation_recno && !nationPtr->get_relation_passable(startLocPtr->power_nation_recno)) ||
		(destLocPtr->power_nation_recno && !nationPtr->get_relation_passable(destLocPtr->power_nation_recno)))
		subModeOn = 0;

	if(subModeOn) // true only when both start and end locations are passable for this nation
	{
		seek_path.set_nation_passable(nationPtr->relation_passable_array);
		seek_path.set_sub_mode(SEARCH_SUB_MODE_PASSABLE);
	}
	else
		seek_path.set_sub_mode(); //----- normal sub mode, normal searching

	//----------- set sub_mode of path-reuse if more than one unit are selected -----------//
	if(subModeOn && searchMode==SEARCH_MODE_REUSE)
	{
		seek_path_reuse.set_nation_passable(nationPtr->relation_passable_array);
		seek_path_reuse.set_sub_mode(SEARCH_SUB_MODE_PASSABLE);
	}
	else
		seek_path_reuse.set_sub_mode();
}
//----------- End of function Unit::select_search_sub_mode -----------//


//--------- Begin of function Unit::searching ---------//
int Unit::searching(int destXLoc, int destYLoc, int preserveAction, short searchMode, short miscNo, short numOfPath, short reuseMode, short pathReuseStatus)
{
	stop(preserveAction); // stop the unit as soon as possible

	int startXLocLoc=next_x_loc();   // next location the sprite is moving towards
	int startYLocLoc=next_y_loc();
	int totalAvailableNode = seek_path.total_node_avail;

	if(!avail_node_enough_for_search(startXLocLoc, startYLocLoc, destXLoc, destYLoc))
	{
		abort_searching(searchMode==4 && numOfPath>1);
		return 0; // not enough node for searching
	}

	//stop(preserveAction); // stop the unit as soon as possible

	//---------------------------------------------------------------------------//
	// adjust the destination for unit size
	//---------------------------------------------------------------------------//
	/*err_when(sprite_info->loc_width!=sprite_info->loc_height);
	if(sprite_info->loc_width>1) // not size 1x1
	{
		destXLoc = move_to_x_loc = MIN(destXLoc, MAX_WORLD_X_LOC-sprite_info->loc_width);
		destYLoc = move_to_y_loc = MIN(destYLoc, MAX_WORLD_Y_LOC-sprite_info->loc_height);
	}
	else
	{*/
		move_to_x_loc = destXLoc;
		move_to_y_loc = destYLoc;
	//}

	//------------------------------------------------------------//
	// fast checking for destination == current location
	//------------------------------------------------------------//
	//if(startXLocLoc==move_to_x_loc && startYLocLoc==move_to_y_loc) // already here
	if(startXLocLoc==destXLoc && startYLocLoc==destYLoc) // already here
	{
		if(cur_x!=next_x || cur_y!=next_y)
			set_move();
		else
			set_idle();

		err_when(move_to_x_loc!=startXLocLoc || move_to_y_loc!=startYLocLoc);
		err_when(result_node_array!=NULL);

		abort_searching(searchMode==SEARCH_MODE_REUSE && numOfPath>1);
		return 1;
	}

	//------------------------ find the shortest path --------------------------//
	//
	// Note: seek() will never return PATH_SEEKING as the maxTries==max_node in
	//       calling seek()
	//
	// decide the searching to use according to the unit size
	// assume the unit size is always 1x1, 2x2, 3x3 and so on
	// i.e. sprite_info->loc_width == sprite_info->loc_height
	//--------------------------------------------------------------------------//

	result_node_recno = result_node_count = 0;
	err_when(result_node_array!=NULL);

	seek_path.set_nation_recno(nation_recno);

	int seekResult;
	#ifdef DEBUG
		unsigned long seekPathStartTime = misc.get_time();
	#endif
	/*switch(sprite_info->loc_width)
	{
		case 1:*/
					if(searchMode!=SEARCH_MODE_REUSE || numOfPath==1)	// no need to call path_reuse
					{
						if(mobile_type==UNIT_LAND)
							select_search_sub_mode(startXLocLoc, startYLocLoc, destXLoc, destYLoc, nation_recno, searchMode);
						seekResult = seek_path.seek(startXLocLoc, startYLocLoc, destXLoc, destYLoc, unit_group_id,
															mobile_type, searchMode, miscNo, numOfPath, unit_search_tries);

						result_node_array = seek_path.get_result(result_node_count, result_path_dist);
						seek_path.set_sub_mode(); // reset sub_mode searching
					}
					else	// use path_reuse
					{
						err_when(reuseMode!=GENERAL_GROUP_MOVEMENT);
						seekResult = seek_path_reuse.seek(startXLocLoc, startYLocLoc, destXLoc, destYLoc, 1, unit_group_id,
														mobile_type, searchMode, miscNo, numOfPath, reuseMode, pathReuseStatus);
						result_node_array = seek_path_reuse.get_result(result_node_count, result_path_dist);
					}
	#ifdef DEBUG
		seek_path_profile_time = misc.get_time() - seekPathStartTime;
	#endif
	/*				break;

		default: err_here();
					break;
	}*/

	if(seekResult==PATH_IMPOSSIBLE)
	{
		reset_path();
		err_when(result_node_array || result_node_count);
	}

	//####### begin trevor 15/10 ########//

	//-----------------------------------------------------------------------//
	// update ignore_power_nation,seek_path_fail_count
	//-----------------------------------------------------------------------//

	if(ai_unit)
	{
		//----- set seek_path_fail_count ------//

		if( seekResult==PATH_IMPOSSIBLE ||
			 (seekResult==PATH_NODE_USED_UP &&    // if all the nodes have been used up and the number of nodes original available is >= VALID_BACKGROUND_SEARCH_NODE
			  totalAvailableNode >= VALID_BACKGROUND_SEARCH_NODE) )
		{
			if( seek_path_fail_count < 100 )		// prevent numeric overflow
				seek_path_fail_count++;
		}
		else
			seek_path_fail_count=0;

		//------- set ignore_power_nation -------//

		if( seekResult==PATH_IMPOSSIBLE )
		{
			switch(ignore_power_nation)
			{
				case 0:	ignore_power_nation = 1;
							break;
				case 1:	ignore_power_nation = 2;
							break;
				case 2:	break;
				default:	err_here();
							break;
			}
		}
		else
		{
			if( ignore_power_nation==1 )
				ignore_power_nation = 0;
		}
	}

	//######## end trevor 15/10 ########//

	//-----------------------------------------------------------------------//
	// if closest node is returned, the destination should not be the real
	// location to go to.  Thus, move_to_?_loc should be adjusted
	//-----------------------------------------------------------------------//
	if(result_node_array && result_node_count)
	{
		ResultNode* lastNode = result_node_array + result_node_count - 1;
		move_to_x_loc = lastNode->node_x; // adjust move_to_?_loc
		move_to_y_loc = lastNode->node_y;

		result_node_recno = 1;        // skip the first node which is the current location
		if(cur_action != SPRITE_MOVE)     // check if the unit is moving right now, wait until it reaches the nearest complete tile.
		{
			err_when(cur_action!=SPRITE_SHIP_EXTRA_MOVE && (cur_x!=next_x || cur_y!=next_y));
			#ifdef DEBUG
				int moveToXLoc = move_to_x_loc;
				int moveToYLoc = move_to_y_loc;
			#endif
			
			ResultNode *nextNode = result_node_array + 1;
			//set_dir(next_x_loc(), next_y_loc(), nextNode->node_x, nextNode->node_y);
			set_dir(startXLocLoc, startYLocLoc, nextNode->node_x, nextNode->node_y);

			next_move();

			#ifdef DEBUG
			err_when(move_action_call_flag && (moveToXLoc!=move_to_x_loc || moveToYLoc!=move_to_y_loc));
			#endif
		}
	}
	else // stay in the current location
	{
		err_when(result_node_array!=NULL);
		
		move_to_x_loc = startXLocLoc; // adjust move_to_?_loc
		move_to_y_loc = startYLocLoc;
		err_when(move_to_x_loc!=startXLocLoc || move_to_y_loc!=startYLocLoc);

		if(cur_x!=next_x || cur_y!=next_y)
			set_move();
		else
			set_idle();
	}

	err_when(move_to_x_loc<0 || move_to_x_loc>=MAX_WORLD_X_LOC || move_to_y_loc<0 || move_to_y_loc>=MAX_WORLD_Y_LOC);
	err_when(cur_action==SPRITE_IDLE && (move_to_x_loc!=next_x_loc() || move_to_y_loc!=next_y_loc()));
	err_when(cur_action==SPRITE_IDLE && (cur_x!=next_x || cur_y!=next_y));

	//-------------------------------------------------------//
	// PATH_NODE_USED_UP happens when:
	// Exceed the object's MAX's node limitation, the closest path
	// is returned. Get to the closest path first and continue
	// to seek the path in the background.
	//-------------------------------------------------------//

	return 1;
}
//----------- End of function Unit::searching -----------//


//--------- Begin of function Unit::move_to_firm_surround ---------//
// order unit to move to firm surrounding
//
// <int> destXLoc, destYLoc	- destination
// <int> width						- unit width
// <int> height					- unit height
// <int> miscNo					- the firm id (default 0)
// <int>	readyDist				- similar to that in set_move_to_surround()
//										  (default 0)
//
// Note: the firm should exist in the location (destXloc, destYLoc)
//			this function can be called by unit with size 1x1, 2x2,
//
void Unit::move_to_firm_surround(int destXLoc, int destYLoc, int width, int height, int miscNo, int readyDist)
{
	err_when(destXLoc<0 || destXLoc>=MAX_WORLD_X_LOC || destYLoc<0 || destYLoc>=MAX_WORLD_Y_LOC);

	//----------------------------------------------------------------//
	// calculate new destination if trying to move to different territory
	//----------------------------------------------------------------//
	Location *locPtr = world.get_loc(destXLoc, destYLoc);
	if(unit_res[unit_id]->unit_class==UNIT_CLASS_SHIP && miscNo==FIRM_HARBOR)
	{
		err_when(!locPtr->is_firm());
		Firm *firmPtr = firm_array[locPtr->firm_recno()];
		FirmHarbor *harborPtr = (FirmHarbor*) firmPtr;
		if(world.get_loc(next_x_loc(), next_y_loc())->region_id!=harborPtr->sea_region_id)
		{
			move_to(destXLoc, destYLoc);
			return;
		}
	}
	else
	{
		if(world.get_loc(next_x_loc(), next_y_loc())->region_id!=locPtr->region_id)
		{
			move_to(destXLoc, destYLoc);
			return;
		}
	}

	//----------------------------------------------------------------//
	// return if the unit is dead
	//----------------------------------------------------------------//
	if(is_unit_dead())
		return;
	
	//----------------------------------------------------------------//
	// check for equal actions
	//----------------------------------------------------------------//
	if(action_mode2==ACTION_MOVE && action_mode==ACTION_MOVE)
	{
		//------ previous action is ACTION_MOVE -------//
		err_when(action_para2 || action_para);
		if(action_x_loc2==destXLoc && action_y_loc2==destYLoc)
		{
			//-------- equal order --------//
			action_x_loc = action_x_loc2;
			action_y_loc = action_y_loc2;

			if(cur_action!=SPRITE_IDLE)
			{
				//-------- the old order is processing --------//
				if(result_node_array==NULL) // cannot move
				{
					err_when(result_node_count || result_node_recno);
					set_idle();
				}

				err_when(action_mode==ACTION_MOVE && (action_x_loc==-1 || action_y_loc==-1));
				return;
			}//else action is hold due to some problems, re-activiate again
		}
	}//else, new order or searching is required
		
	int destX = MAX(0, ((width>1) ? destXLoc : destXLoc - width + 1));
	int destY = MAX(0, ((height>1) ? destYLoc : destYLoc - height + 1));
		
	FirmInfo *firmInfo = firm_res[miscNo];
	stop();
	set_move_to_surround(destX, destY, firmInfo->loc_width, firmInfo->loc_height, BUILDING_TYPE_FIRM_MOVE_TO, miscNo);

	//----------------------------------------------------------------//
	// store new order in action parameters
	//----------------------------------------------------------------//
	action_mode = action_mode2 = ACTION_MOVE;
	action_para = action_para2 = 0;
	action_x_loc = action_x_loc2 = move_to_x_loc;
	action_y_loc = action_y_loc2 = move_to_y_loc;

	err_when(action_mode==ACTION_MOVE && (action_x_loc==-1 || action_y_loc==-1));
	err_when(move_to_x_loc<0 || move_to_x_loc>=MAX_WORLD_X_LOC || move_to_y_loc<0 || move_to_y_loc>=MAX_WORLD_Y_LOC);
	err_when(cur_action==SPRITE_IDLE && (move_to_x_loc!=next_x_loc() || move_to_y_loc!=next_y_loc()));
	err_when(cur_action==SPRITE_IDLE && (cur_x!=next_x || cur_y!=next_y));

	#ifdef DEBUG
	if(unit_res[unit_id]->unit_class!=UNIT_CLASS_SHIP || ((UnitMarine*)this)->extra_move_in_beach==NO_EXTRA_MOVE)
		err_when(result_node_array==NULL && (next_x!=go_x || next_y!=go_y));
	#endif
}	
//----------- End of function Unit::move_to_firm_surround -----------//


//--------- Begin of function Unit::move_to_town_surround ---------//
// move to town surrounding
//
// <int> destXLoc, destYLoc	- destination
// <int> width						- unit width
// <int> height					- unit height
// <int> miscNo					- reserved (default 0)
// <int>	readyDist				- similar to that in set_move_to_surround()
//										  (default 0)
//
// Note: assume the town exists
//			this function can be called by unit with size 1x1, 2x2,
//
void Unit::move_to_town_surround(int destXLoc, int destYLoc, int width, int height, int miscNo, int readyDist)
{
	err_when(destXLoc<0 || destXLoc>=MAX_WORLD_X_LOC || destYLoc<0 || destYLoc>=MAX_WORLD_Y_LOC);

	//----------------------------------------------------------------//
	// calculate new destination if trying to move to different territory
	//----------------------------------------------------------------//
	Location *locPtr = world.get_loc(destXLoc, destYLoc);
	if(world.get_loc(next_x_loc(), next_y_loc())->region_id!=locPtr->region_id)
	{
		move_to(destXLoc, destYLoc);
		return;
	}

	//----------------------------------------------------------------//
	// return if the unit is dead
	//----------------------------------------------------------------//
	if(is_unit_dead())
		return;
	
	//----------------------------------------------------------------//
	// check for equal actions
	//----------------------------------------------------------------//
	if(action_mode2==ACTION_MOVE && action_mode==ACTION_MOVE)
	{
		//------ previous action is ACTION_MOVE -------//
		err_when(action_para2 || action_para);
		if(action_x_loc2==destXLoc && action_y_loc2==destYLoc)
		{
			//-------- equal order --------//
			action_x_loc = action_x_loc2;
			action_y_loc = action_y_loc2;

			if(cur_action!=SPRITE_IDLE)
			{
				//-------- the old order is processing --------//
				if(result_node_array==NULL) // cannot move
				{
					err_when(result_node_count || result_node_recno);
					set_idle();
				}

				err_when(action_mode==ACTION_MOVE && (action_x_loc==-1 || action_y_loc==-1));
				return;
			}//else action is hold due to some problems, re-activiate again
		}
	}//else, new order or searching is required
		
	int destX = MAX(0, ((width>1) ? destXLoc : destXLoc - width + 1));
	int destY = MAX(0, ((height>1) ? destYLoc : destYLoc - height + 1));
		
	stop();
	set_move_to_surround(destX, destY, STD_TOWN_LOC_WIDTH, STD_TOWN_LOC_HEIGHT, BUILDING_TYPE_TOWN_MOVE_TO);

	//----------------------------------------------------------------//
	// store new order in action parameters
	//----------------------------------------------------------------//
	action_mode = action_mode2 = ACTION_MOVE;
	action_para = action_para2 = 0;
	action_x_loc = action_x_loc2 = move_to_x_loc;
	action_y_loc = action_y_loc2 = move_to_y_loc;

	err_when(action_mode==ACTION_MOVE && (action_x_loc==-1 || action_y_loc==-1));
	err_when(move_to_x_loc<0 || move_to_x_loc>=MAX_WORLD_X_LOC || move_to_y_loc<0 || move_to_y_loc>=MAX_WORLD_Y_LOC);
	err_when(cur_action==SPRITE_IDLE && (move_to_x_loc!=next_x_loc() || move_to_y_loc!=next_y_loc()));
	err_when(cur_action==SPRITE_IDLE && (cur_x!=next_x || cur_y!=next_y));
	err_when(result_node_array==NULL && (next_x!=go_x || next_y!=go_y));
}
//----------- End of function Unit::move_to_town_surround -----------//


//--------- Begin of function Unit::move_to_wall_surround ---------//
// move to wall surrounding
//
// <int> destXLoc, destYLoc	- destination
// <int> width						- unit width
// <int> height					- unit height
// <int> miscNo					- reserved (default 0)
// <int>	readyDist				- similar to that in set_move_to_surround()
//										  (default 0)
//
// Note: assume the wall exists
//			this function can be called by unit with size 1x1, 2x2,
//
void Unit::move_to_wall_surround(int destXLoc, int destYLoc, int width, int height, int miscNo, int readyDist)
{
	err_when(destXLoc<0 || destXLoc>=MAX_WORLD_X_LOC || destYLoc<0 || destYLoc>=MAX_WORLD_Y_LOC);

	//----------------------------------------------------------------//
	// calculate new destination if trying to move to different territory
	//----------------------------------------------------------------//
	Location *locPtr = world.get_loc(destXLoc, destYLoc);
	if(world.get_loc(next_x_loc(), next_y_loc())->region_id!=locPtr->region_id)
	{
		move_to(destXLoc, destYLoc);
		return;
	}

	//----------------------------------------------------------------//
	// return if the unit is dead
	//----------------------------------------------------------------//
	if(is_unit_dead())
		return;
	
	//----------------------------------------------------------------//
	// check for equal actions
	//----------------------------------------------------------------//
	if(action_mode2==ACTION_MOVE && action_mode==ACTION_MOVE)
	{
		//------ previous action is ACTION_MOVE -------//
		err_when(action_para2 || action_para);
		if(action_x_loc2==destXLoc && action_y_loc2==destYLoc)
		{
			//-------- equal order --------//
			action_x_loc = action_x_loc2;
			action_y_loc = action_y_loc2;

			if(cur_action!=SPRITE_IDLE)
			{
				//-------- the old order is processing --------//
				if(result_node_array==NULL) // cannot move
				{
					err_when(result_node_count || result_node_recno);
					set_idle();
				}

				err_when(action_mode==ACTION_MOVE && (action_x_loc==-1 || action_y_loc==-1));
				return;
			}//else action is hold due to some problems, re-activiate again
		}
	}//else, new order or searching is required
		
	int destX = MAX(0, ((width>1) ? destXLoc : destXLoc - width + 1));
	int destY = MAX(0, ((height>1) ? destYLoc : destYLoc - height + 1));
		
	stop();
	set_move_to_surround(destX, destY, 1, 1, BUILDING_TYPE_WALL);

	//----------------------------------------------------------------//
	// store new order in action parameters
	//----------------------------------------------------------------//
	action_mode = action_mode2 = ACTION_MOVE;
	action_para = action_para2 = 0;
	action_x_loc = action_x_loc2 = move_to_x_loc;
	action_y_loc = action_y_loc2 = move_to_y_loc;

	err_when(action_mode==ACTION_MOVE && (action_x_loc==-1 || action_y_loc==-1));
	err_when(move_to_x_loc<0 || move_to_x_loc>=MAX_WORLD_X_LOC || move_to_y_loc<0 || move_to_y_loc>=MAX_WORLD_Y_LOC);
	err_when(cur_action==SPRITE_IDLE && (move_to_x_loc!=next_x_loc() || move_to_y_loc!=next_y_loc()));
	err_when(cur_action==SPRITE_IDLE && (cur_x!=next_x || cur_y!=next_y));
	err_when(result_node_array==NULL && (next_x!=go_x || next_y!=go_y));
}
//----------- End of function Unit::move_to_wall_surround -----------//


//--------- Begin of function Unit::move_to_unit_surround ---------//
// move to unit surrounding
//
// <int> destXLoc, destYLoc	- destination
// <int> width						- unit width
// <int> height					- unit height
// <int> miscNo					- vehicle unit_recno (default 0)
// <int>	readyDist				- similar to that in set_move_to_surround()
//										  (default 0)
//
// Note: assume the vehicle exists
//			this function can be called by unit with size 1x1, 2x2,
//
void Unit::move_to_unit_surround(int destXLoc, int destYLoc, int width, int height, int miscNo, int readyDist)
{
	err_when(destXLoc<0 || destXLoc>=MAX_WORLD_X_LOC || destYLoc<0 || destYLoc>=MAX_WORLD_Y_LOC);

	//----------------------------------------------------------------//
	// calculate new destination if trying to move to different territory
	//----------------------------------------------------------------//
	Location *locPtr = world.get_loc(destXLoc, destYLoc);
	if(world.get_loc(next_x_loc(), next_y_loc())->region_id!=locPtr->region_id)
	{
		move_to(destXLoc, destYLoc);
		return;
	}

	//----------------------------------------------------------------//
	// return if the unit is dead
	//----------------------------------------------------------------//
	if(is_unit_dead())
		return;
	
	//----------------------------------------------------------------//
	// check for equal actions
	//----------------------------------------------------------------//
	if(action_mode2==ACTION_MOVE && action_mode==ACTION_MOVE)
	{
		//------ previous action is ACTION_MOVE -------//
		err_when(action_para2 || action_para);
		if(action_x_loc2==destXLoc && action_y_loc2==destYLoc)
		{
			//-------- equal order --------//
			action_x_loc = action_x_loc2;
			action_y_loc = action_y_loc2;

			if(cur_action!=SPRITE_IDLE)
			{
				//-------- the old order is processing --------//
				if(result_node_array==NULL) // cannot move
				{
					err_when(result_node_count || result_node_recno);
					set_idle();
				}

				err_when(action_mode==ACTION_MOVE && (action_x_loc==-1 || action_y_loc==-1));
				return;
			}//else action is hold due to some problems, re-activiate again
		}
	}//else, new order or searching is required
		
	int destX = MAX(0, ((width>1) ? destXLoc : destXLoc - width + 1));
	int destY = MAX(0, ((height>1) ? destYLoc : destYLoc - height + 1));
		
	err_when(unit_array.is_deleted(miscNo));
	Unit *unitPtr = unit_array[miscNo];
	SpriteInfo *spriteInfo = unitPtr->sprite_info;
	stop();
	set_move_to_surround(destX, destY, spriteInfo->loc_width, spriteInfo->loc_height, BUILDING_TYPE_VEHICLE);

	//----------------------------------------------------------------//
	// store new order in action parameters
	//----------------------------------------------------------------//
	action_mode = action_mode2 = ACTION_MOVE;
	action_para = action_para2 = 0;
	action_x_loc = action_x_loc2 = move_to_x_loc;
	action_y_loc = action_y_loc2 = move_to_y_loc;

	err_when(action_mode==ACTION_MOVE && (action_x_loc==-1 || action_y_loc==-1));
	err_when(move_to_x_loc<0 || move_to_x_loc>=MAX_WORLD_X_LOC || move_to_y_loc<0 || move_to_y_loc>=MAX_WORLD_Y_LOC);
	err_when(cur_action==SPRITE_IDLE && (move_to_x_loc!=next_x_loc() || move_to_y_loc!=next_y_loc()));
	err_when(cur_action==SPRITE_IDLE && (cur_x!=next_x || cur_y!=next_y));
	err_when(result_node_array==NULL && (next_x!=go_x || next_y!=go_y));
}
//----------- End of function Unit::move_to_unit_surround -----------//


//--------- Begin of function Unit::different_territory_destination ---------//
// calculate a reachable destination if the unit is ordered to move to unreachable
// location on different territory
// 
// <int&> destX, destY	- reference to return destination
//
void Unit::different_territory_destination(int& destX, int& destY)
{
	int curXLoc = next_x_loc();
	int curYLoc = next_y_loc();
	
	Location *locPtr = world.get_loc(curXLoc, curYLoc);
	int regionId = locPtr->region_id;
	int xStep = destX-curXLoc;
	int yStep = destY-curYLoc;
	int absXStep = abs(xStep);
	int absYStep = abs(yStep);
	int count = (absXStep>=absYStep) ? absXStep : absYStep;
	int x, y;
	long int sameTerr = 0;

	//------------------------------------------------------------------------------//
	// draw a line from the unit location to the destination, find the last location
	// with the same region id.
	//------------------------------------------------------------------------------//
	for(long int i=1; i<=count; i++)
	{
		x = curXLoc + int((i*xStep)/count);
		y = curYLoc + int((i*yStep)/count);

		locPtr = world.get_loc(x, y);
		if(locPtr->region_id==regionId)
			sameTerr = i;
	}

	if(sameTerr)
	{
		destX = curXLoc + int((sameTerr*xStep)/count);
		destY = curYLoc + int((sameTerr*yStep)/count);
	}
	else
	{
		destX = curXLoc;
		destY = curYLoc;
	}
}
//----------- End of function Unit::different_territory_destination -----------//


//--------- Begin of function Unit::next_move ---------//
//
//	If there is unprocessed node(s) in the result_node_array,
// then next unprocessed node will be set to be the next location
// to move to. (i.e. go_? = location of the unprocessed node)
//
void Unit::next_move()
{
	if(result_node_array == NULL || !result_node_count || !result_node_recno)
      return;

   if( ++result_node_recno > result_node_count )
   {
		//------------ all nodes are visited --------------//
		err_when(cur_x!=next_x || cur_y!=next_y);
		err_when(next_x_loc()!=move_to_x_loc || next_y_loc()!=move_to_y_loc);

      mem_del(result_node_array);
      result_node_array = NULL;
		set_idle();
		
		if(action_mode2==ACTION_MOVE) //--------- used to terminate action_mode==ACTION_MOVE
		{
			force_move_flag = 0;

			//------- reset ACTION_MOVE parameters ------//
			reset_action_para();
			if(move_to_x_loc==action_x_loc2 && move_to_y_loc==action_y_loc2)
				reset_action_para2();
		}
      return;
   }

   //---- order the unit to move to the next checkpoint following the path ----//
	
   ResultNode* resultNode = result_node_array+result_node_recno-1;
	#ifdef DEBUG
		err_when(cur_x==move_to_x_loc*ZOOM_LOC_WIDTH && cur_y==move_to_y_loc*ZOOM_LOC_HEIGHT);
		err_when(!resultNode);
	#endif

	sprite_move( resultNode->node_x*ZOOM_LOC_WIDTH, resultNode->node_y*ZOOM_LOC_HEIGHT );

	err_when(cur_x==go_x && cur_y==go_y && (cur_x!=next_x || cur_y!=next_y));
}
//----------- End of function Unit::next_move -----------//


//--------- Begin of function Unit::reset_path ---------//
// Cancel all movement.
//
void Unit::reset_path()
{
	if( result_node_array )
	{
		mem_del(result_node_array);
      result_node_array = NULL;
	}

	result_path_dist = result_node_count = result_node_recno = 0;
}
//----------- End of function Unit::reset_path -----------//


//--------- Begin of function Unit::pre_process ---------//
// process unit's action
//
void Unit::pre_process()
{
	//-*********** simulate aat ************-//
	#ifdef DEBUG
		if(debug_sim_game_type==2)
		{
			if(hit_points!=max_hit_points)
				hit_points = max_hit_points;

			Nation *nationPtr = nation_array[nation_recno];
			if(nationPtr->cash<4000)
				nationPtr->cash += 10000;

			if(nationPtr->food<4000)
				nationPtr->food += 10000;
		}
	#endif
	//-*********** simulate aat ************-//

	#if defined(DEBUG) && defined(ENABLE_LOG)
		String logStr;
		logStr = " begin unit ";
		logStr += sprite_recno;
		// ########## begin Gilbert 6/9 #######//
		logStr += " nation=";
		logStr += nation_recno;
		logStr += "(";
		logStr += true_nation_recno();
		logStr += ")";
		// ########## end Gilbert 6/9 #######//
		logStr += " pre_process(), action_mode=";
		logStr += action_mode;
		logStr += " action_mode2=";
		logStr += action_mode2;
		LOG_MSG(logStr);

		logStr = "action_para=";
		logStr += action_para;
		logStr += " action_para2=";
		logStr += action_para2;
		logStr += " cur_action=";
		logStr += cur_action;
		logStr += " cur_x/y=";
		logStr += cur_x;
		logStr += "/";
		logStr += cur_y;
		logStr += " next_x/y=";
		logStr += next_x;
		logStr += "/";
		logStr += next_y;

		LOG_MSG(logStr);
	#endif

	//------ if all the hit points are lost, die now ------//
	if(hit_points <= 0 && action_mode != ACTION_DIE)
	{
		set_die();

		if(ai_action_id)
			nation_array[nation_recno]->action_failure(ai_action_id, sprite_recno);

		return;
	}

	#ifdef DEBUG
		int debugActionMode = action_mode;
		int debugActionPatra = action_para;
		int debugCurAction = cur_action;
	#endif

	if( config.fog_of_war )
	{
		if( is_own() ||
			 (nation_recno && nation_array[nation_recno]->is_allied_with_player) )
		{
			world.visit(next_x_loc(), next_y_loc(), next_x_loc()+sprite_info->loc_width-1,
				next_y_loc()+sprite_info->loc_height-1, unit_res[unit_id]->visual_range,
				unit_res[unit_id]->visual_extend);
		}
	}

	//--------- process action corresponding to action_mode ----------//

	#ifdef DEBUG
		unsigned long startTime;
	#endif

	switch(action_mode)
	{
		case ACTION_ATTACK_UNIT:

			#ifdef DEBUG
				startTime = misc.get_time();
			#endif

			//------------------------------------------------------------------//
			// if unit is in defense mode, check situation to follow the target
			// or return back to camp
			//------------------------------------------------------------------//
			if(action_mode!=action_mode2)
			{
				if(action_mode2==ACTION_AUTO_DEFENSE_ATTACK_TARGET)
				{
					if(!defense_follow_target()) // false if abort attacking
						break; // cancel attack and go back to military camp
				}
				else if(action_mode2==ACTION_DEFEND_TOWN_ATTACK_TARGET)
				{
					if(!defend_town_follow_target())
						break;
				}
				else if(action_mode2==ACTION_MONSTER_DEFEND_ATTACK_TARGET)
				{
					if(!monster_defend_follow_target())
						break;
				}
				else
					err_here();
			}

			process_attack_unit();

			#ifdef DEBUG
				unit_attack_profile_time += misc.get_time() - startTime;
			#endif
			break;

		case ACTION_ATTACK_FIRM:
			process_attack_firm();
			break;

		case ACTION_ATTACK_TOWN:
			process_attack_town();
			break;

		case ACTION_ATTACK_WALL:
			process_attack_wall();
			break;

		case ACTION_ASSIGN_TO_FIRM:
		case ACTION_ASSIGN_TO_TOWN:
		case ACTION_ASSIGN_TO_VEHICLE:
			#ifdef DEBUG
				startTime = misc.get_time();
			#endif
			process_assign();
			#ifdef DEBUG
				unit_assign_profile_time += misc.get_time() - startTime;
			#endif
			break;

		case ACTION_ASSIGN_TO_SHIP:
			process_assign_to_ship();
			break;

		case ACTION_BUILD_FIRM:
			process_build_firm();
			break;

		case ACTION_BURN:
			process_burn();
			break;

		case ACTION_SETTLE:
			process_settle();
			break;

		case ACTION_SHIP_TO_BEACH:
			process_ship_to_beach();
			break;

		case ACTION_GO_CAST_POWER:
			process_go_cast_power();
			break;
	}

	//-****** don't add code here, the unit may be removed after the above function call*******-//

	// ###### begin Gilbert 20/6 ########//
	#ifdef DEBUG
		// do not read data member
		LOG_MSG( " end Unit::pre_process()" );
		LOG_MSG( misc.get_random_seed() );
	#endif
	// ###### end Gilbert 20/6 ########//

}
//----------- End of function Unit::pre_process -----------//


//--------- Begin of function Unit::process_die ---------//
// process unit die
// 
// return 1 if die frame is counting
// return 0 otherwise
//
int Unit::process_die()
{
	//-*********** simulate aat ************-//
	#ifdef DEBUG
		if(debug_sim_game_type)
		{
			cur_action = SPRITE_IDLE;
			hit_points = max_hit_points;
			stop2();
			return 0;
		}
	#endif
	//-*********** simulate aat ************-//

	//--------- voice ------------//
	se_res.sound(cur_x_loc(), cur_y_loc(), cur_frame, 'S',sprite_id,"DIE");

	// ####### begin Gilbert 14/7 ###########//
	//------------- add die effect on first frame --------- //
	if( cur_frame == 1 && unit_res[unit_id]->die_effect_id)
	{
		Effect::create(unit_res[unit_id]->die_effect_id, cur_x, cur_y, 
			SPRITE_DIE, cur_dir, mobile_type == UNIT_AIR ? 8 : 2, 0);
	}
	// ####### end Gilbert 14/7 ###########//

	//--------- next frame ---------//
	if( ++cur_frame > sprite_info->die.frame_count )
		return 1;

	return 0;
}
//----------- End of function Unit::process_die -----------//


//--------- Begin of function Unit::process_rebel ---------//
//
// Unit::process_rebel() is in OREBEL.CPP
//
//----------- End of function Unit::process_rebel ---------//


//--------- Begin of function Unit::avail_node_enough_for_search --------//
// decide whether the available number of nodes enough for a valid path searching
//
// <short>	x1, y1	-	start location
// <short>	x2, y2	-	end location
//
// return 1 if num of nodes is enough
// return 0 otherwise
//
int Unit::avail_node_enough_for_search(short x1, short y1, short x2, short y2)
{
	short dispX = abs(x1-x2);
	short dispY = abs(y1-y2);

	short majDist = dispX>dispY ? dispX : dispY;
	short minDist = abs(dispX-dispY);

	int nodeRequire = MIN(VALID_BACKGROUND_SEARCH_NODE, majDist<<5); // *32
	int totalNode = seek_path.total_node_avail;
	if(totalNode < nodeRequire)
	{
		if(totalNode>=MIN_BACKGROUND_NODE_USED_UP)
			seek_path.total_node_avail = MIN_BACKGROUND_NODE_USED_UP-1;

		return 0;
	}

	return 1;
}
//----------- End of function Unit::avail_node_enough_for_search -----------//


//--------- Begin of function Unit::process_move --------//
// process unit movement
//
void Unit::process_move()
{
	//----- if the sprite has reach the destintion ----//

	//--------------------------------------------------------//
	// if the unit reach its destination, then
	// cur_? == next_? == go_?
	//--------------------------------------------------------//
	err_when(cur_x==go_x && cur_y==go_y && (cur_x!=next_x || cur_y!=next_y));
	
	if(cur_x==go_x && cur_y==go_y)
	{
		if(result_node_array)
		{
			next_move();
			if( cur_action != SPRITE_MOVE )     // if next_move() is not successful, the movement has been stopped
				return;

			//---------------------------------------------------------------------------//
			// If (1) the unit is blocked at cur_? == go_? and go_? != destination and 
			//		(2) a new path is generated if calling the previous next_move(),
			//	then cur_? still equal to go_?.
			//
			// The following Sprite::process_move() call will set the unit to SPRITE_IDLE 
			// since cur_? == go_?. Thus, the unit terminates its move although it has not
			// reached its destination.
			//
			// (note: if it has reached its destination, cur_? == go_? and cur_action =
			//			 SPRITE_IDLE)
			//
			// if the unit is still moving and cur_? == go_?, call next_move() again to reset
			// the go_?.
			//---------------------------------------------------------------------------//
			if(cur_action==SPRITE_MOVE && cur_x==go_x && cur_y==go_y)
				next_move();
		}
	}

	err_when(result_node_array && result_node_count==result_node_recno &&
				(result_node_array[result_node_count-1].node_x!=go_x>>ZOOM_X_SHIFT_COUNT ||
				result_node_array[result_node_count-1].node_y!=go_y>>ZOOM_Y_SHIFT_COUNT));

	//--------- process the move, update sprite position ---------//
	//--------------------------------------------------------//
	// if the unit is moving, cur_?!=go_? and
	// if next_? != cur_?, the direction from cur_? to next_?
	// should equal to that from cur_? to go_?
	//--------------------------------------------------------//
	err_when((cur_x%ZOOM_LOC_WIDTH==0 && cur_y%ZOOM_LOC_HEIGHT==0) && (cur_x!=next_x || cur_y!=next_y) &&
				((check_unit_dir1=get_dir(cur_x, cur_y, go_x, go_y))!=(check_unit_dir2=get_dir(cur_x, cur_y, next_x, next_y))));

	#ifdef DEBUG
		int debugCurX = cur_x;
		int debugCurY = cur_y;
		int debugNextX = next_x;
		int debugNextY = next_y;
		int debugGoX = go_x;
		int debugGoY = go_y;
	#endif
	err_when(cur_x-next_x!=0 && cur_y-next_y!=0 && abs(next_x-cur_x)!=abs(next_y-cur_y));
	err_when(result_path_dist && result_node_array==NULL);

	Sprite::process_move();
	
	err_when( cur_x < 0 || cur_y < 0 || cur_x >= ZOOM_X_PIXELS || cur_y >= ZOOM_Y_PIXELS );
	if(cur_x==go_x && cur_y==go_y && cur_action==SPRITE_IDLE)	// the sprite has reached its destination
	{
		move_to_x_loc = next_x_loc();
		move_to_y_loc = next_y_loc();
	}

	//--------------------------------------------------------//
	// after Sprite::process_move(), if the unit is blocked, its
	// cur_action is set to SPRITE_WAIT. Otherwise, its cur_action
	// is still SPRITE_MOVE.  Then cur_? != next_? if the unit
	// has not reached its destination.
	//--------------------------------------------------------//
	err_when(cur_action==SPRITE_MOVE && (cur_x!=go_x || cur_y!=go_y) && (cur_x==next_x && cur_y==next_y));
}
//---------- End of function Unit::process_move ----------//


//--------- Begin of function Unit::process_wait ---------//
// process unit's waiting
//
void Unit::process_wait()
{
	err_when((check_unit_dir1=get_dir(cur_x, cur_y, go_x, go_y))!=final_dir);
	if(!match_dir())
		return;
	
	//-----------------------------------------------------//
	// If the unit is moving to the destination and was
	// blocked by something. If it is now no longer blocked,
	// continue the movement.
	//-----------------------------------------------------//
   //
	// When this funciton is called:
   //
	// (next_x, next_y)==(cur_x, cur_y), it's the location of the sprite.
   //
   //-----------------------------------------------------//

   //--- find out the next location which the sprite should be moving towards ---//

	//-----------------------------------------------------//
	// If the unit is waiting,
	//		go_? != cur_?
	//		go_? != next_?
	//
	// If the unit is not under swapping, the next_?_loc()
	//	is always the move_to_?_loc. Thus, the unit is ordered
	//	to stop.
	//-----------------------------------------------------//
	err_when( (go_x>>ZOOM_X_SHIFT_COUNT!=move_to_x_loc || go_y>>ZOOM_Y_SHIFT_COUNT!=move_to_y_loc) &&
				 ( (cur_x==go_x && cur_y==go_y) || (cur_x!=next_x || cur_y!=next_y) ) );

	if(next_x_loc()==move_to_x_loc && next_y_loc()==move_to_y_loc && !swapping)
	{
		terminate_move();
		return; // terminate since already in destination
	}

   int stepMagn = move_step_magn();
	int nextX = cur_x+stepMagn*move_x_pixel_array[final_dir];
   int nextY = cur_y+stepMagn*move_y_pixel_array[final_dir];

	/*short w, h, blocked=0;
	short x, y, blockedX, blockedY;
	Location* locPtr;

	//---------- check whether the unit is blocked -----------//
	for(h=0, y=nextY>>ZOOM_Y_SHIFT_COUNT; h<sprite_info->loc_height && !blocked; h++, y++)
	{
		for(w=0, x=nextX>>ZOOM_X_SHIFT_COUNT; w<sprite_info->loc_width && !blocked; w++, x++)
		{
			locPtr = world.get_loc(x, y);
			blocked = ( (!locPtr->is_accessible(mobile_type)) || (locPtr->has_unit(mobile_type) &&
							locPtr->unit_recno(mobile_type)!=sprite_recno) );

			if(blocked)
			{
				blockedX = x;
				blockedY = y;
			}
		}
	}*/
	short x = nextX>>ZOOM_X_SHIFT_COUNT;
	short y = nextY>>ZOOM_Y_SHIFT_COUNT; 
	Location *locPtr = world.get_loc(x, y);
	short blocked = ( (!locPtr->is_accessible(mobile_type)) || (locPtr->has_unit(mobile_type) &&
							locPtr->unit_recno(mobile_type)!=sprite_recno) );

	if(!blocked || move_action_call_flag)
	{
		//--------- not blocked, continue to move --------//
		waiting_term = 0;
		set_move();
		cur_frame  = 1;
		set_next(nextX, nextY, -stepMagn, 1);
	}
	else
	{
		//------- blocked, call handle_blocked_move() ------//
		//locPtr = world.get_loc(blockedX, blockedY);
		err_when(cur_x!=next_x || cur_y!=next_y);
      handle_blocked_move(locPtr);
	}

	err_when(cur_action==SPRITE_MOVE && (cur_x!=go_x || cur_y!=go_y) && (cur_x==next_x && cur_y==next_y));
	err_when(cur_x==go_x && cur_y==go_y && (cur_x!=next_x || cur_y!=next_y));
}
//----------- End of function Unit::process_wait -----------//


//--------- Begin of function Unit::set_next --------//
//	set the next coordinates to move to
//
// <int> newNextX, newNextY	- next coordinate to move to
// <int> para						- used to count the result_path_dist
// <int> blockedChecked			- whether the next location specified is checked to be
//											non-blocked.  1 checked for non-blocked, 0 for not checked
//
void Unit::set_next(int newNextX, int newNextY, int para, int blockedChecked)
{
	err_when(!is_visible());

	int curNextXLoc = next_x_loc();
   int curNextYLoc = next_y_loc();
	int newNextXLoc = newNextX >> ZOOM_X_SHIFT_COUNT;
	int newNextYLoc = newNextY >> ZOOM_Y_SHIFT_COUNT;

	#ifdef DEBUG
		int debugStepMagn = move_step_magn();
		err_when(abs(curNextXLoc-newNextXLoc)>debugStepMagn || abs(curNextYLoc-newNextYLoc)>debugStepMagn);
	#endif

	if(curNextXLoc!=newNextXLoc || curNextYLoc!=newNextYLoc)
	{
		if(!match_dir())
		{
			set_wait();
			return;
		}
	}

	//short w, h, blocked=0;
	//short x, y, blockedX, blockedY;
	short w, h, blocked=0;
	short x, y;

	#ifdef DEBUG	
		for(h=0, y=curNextYLoc; h<sprite_info->loc_height; h++, y++)
			for(w=0, x=curNextXLoc; w<sprite_info->loc_width; w++, x++)
				err_when( world.get_unit_recno(x, y, mobile_type) != sprite_recno ); // it must be 0 to put the sprite in this location
	#endif

	if( curNextXLoc != newNextXLoc || curNextYLoc != newNextYLoc )
	{
		//------- if the next location is blocked ----------//

		Location* locPtr;
		if(!blockedChecked)
		{
			/*for(h=0, y=newNextYLoc; h<sprite_info->loc_height && !blocked; h++, y++)
			{
				for(w=0, x=newNextXLoc; w<sprite_info->loc_width && !blocked; w++, x++)
				{
					locPtr = world.get_loc(x, y);
					blocked = ( (!locPtr->is_accessible(mobile_type)) || (locPtr->has_unit(mobile_type) &&
									locPtr->unit_recno(mobile_type)!=sprite_recno) );

					if(blocked)
					{
						blockedX = x;
						blockedY = y;
					}
				}
			}*/
			x = newNextXLoc;
			y = newNextYLoc;
			locPtr = world.get_loc(x, y);
			blocked = ( (!locPtr->is_accessible(mobile_type)) || (locPtr->has_unit(mobile_type) &&
							locPtr->unit_recno(mobile_type)!=sprite_recno) );
		}//else, then blockedChecked = 0

		//--- no change to next_x & next_y if the new next location is blocked ---//

		if(blocked)
		{
			set_cur(next_x, next_y);   // align the sprite to 32x32 location when it stops
  
			//------ avoid infinitely looping in calling handle_blocked_move() ------//
			if(blocked_by_member || move_action_call_flag)
			{
				set_wait();
				blocked_by_member = 0;
			}
			else
			{
				locPtr = world.get_loc(x, y);
				handle_blocked_move(locPtr);
			}

		#ifdef DEBUG
			for(h=0, y=next_y_loc(); h<sprite_info->loc_height; h++, y++)
			{
				for(w=0, x=next_x_loc(); w<sprite_info->loc_width; w++, x++)
					err_when( world.get_unit_recno(x, y, mobile_type) != sprite_recno );    // it must be 0 to put the sprite in this location
			}
		#endif
		}
		else
		{
			err_when(mobile_type!=UNIT_LAND && (abs(para)!=2 || result_path_dist%2));
			if(para)
			{
				#ifdef DEBUG
					int count=0, ii;
					int dist;
					int curXLoc = next_x_loc();
					int curYLoc = next_y_loc();
					ResultNode *curNode = result_node_array + result_node_recno -1;
					ResultNode *nextNode;

					dist = misc.points_distance(curXLoc, curYLoc, curNode->node_x, curNode->node_y);
					if(result_node_recno>1)
						count += dist;
					else
						count -= dist;

					for(ii=result_node_recno, nextNode=curNode+1; ii<result_node_count; ii++, curNode++, nextNode++)
					{
						dist = misc.points_distance(nextNode->node_x, nextNode->node_y, curNode->node_x, curNode->node_y);
						count += dist;
					}

					err_when(result_path_dist!=count);
				#endif

				//----------------------------------------------------------------------------//
				// calculate the result_path_dist as the unit move from one tile to another
				//----------------------------------------------------------------------------//
				result_path_dist += para;
			}

			next_x = newNextX;
         next_y = newNextY;

			swapping = blocked_by_member = 0;
			
			//---- move sprite_recno to the next location ------//

			for(h=0, y=curNextYLoc; h<sprite_info->loc_height; h++, y++)
			{
				for(w=0, x=curNextXLoc; w<sprite_info->loc_width; w++, x++)
					world.set_unit_recno(x, y, mobile_type, 0);
			}

			for(h=0, y=next_y_loc(); h<sprite_info->loc_height; h++, y++)
			{
				for(w=0, x=next_x_loc(); w<sprite_info->loc_width; w++, x++)
					world.set_unit_recno(x, y, mobile_type, sprite_recno);
			}

			//--------- explore land ----------//

			// ###### begin Gilbert 24/5 ######//
			if( !config.explore_whole_map && is_own() )
			// ###### end Gilbert 24/5 ######//
			{
				int xLoc1 = MAX(0,newNextXLoc-EXPLORE_RANGE);
				int yLoc1 = MAX(0,newNextYLoc-EXPLORE_RANGE);
				int xLoc2 = MIN(MAX_WORLD_X_LOC-1, newNextXLoc+EXPLORE_RANGE);
				int yLoc2 = MIN(MAX_WORLD_Y_LOC-1, newNextYLoc+EXPLORE_RANGE);
				int exploreWidth = move_step_magn()-1;

				if( newNextYLoc < curNextYLoc )			// if move upwards, explore upper area
					world.explore(xLoc1, yLoc1, xLoc2, yLoc1+exploreWidth);

				else if( newNextYLoc > curNextYLoc )	// if move downwards, explore lower area
					world.explore(xLoc1, yLoc2-exploreWidth, xLoc2, yLoc2);

				if( newNextXLoc < curNextXLoc )        // if move towards left, explore left area
					world.explore(xLoc1, yLoc1, xLoc1+exploreWidth, yLoc2);

				else if( newNextXLoc > curNextXLoc )   // if move towards right, explore right area
					world.explore(xLoc2-exploreWidth, yLoc1, xLoc2, yLoc2);
			}
		}
	}

	err_when(cur_x!=next_x && cur_y!=next_y &&	// is not blocked
				(check_unit_dir1=get_dir(cur_x, cur_y, next_x, next_y))!=(check_unit_dir2=get_dir(cur_x, cur_y, go_x, go_y)));
}
//---------- End of function Unit::set_next ----------//


//------ Begin of function Unit::blocked_move_new_handle -------//
/*int Unit::blocked_move_new_handle()
{
	//------------------------------------------------------------------//
	// new handling for blocked move
	//------------------------------------------------------------------//
	static int counter = 0;
	int checkXLoc1, checkYLoc1, checkXLoc2, checkYLoc2;
	int curXLoc = next_x_loc(), curYLoc = next_y_loc();
	Location *locPtr;

	counter++;
	switch(final_dir)
	{
		case DIR_N:
				checkXLoc1 = MIN(curXLoc+1, MAX_WORLD_X_LOC-1);
				checkYLoc1 = checkYLoc2 = MAX(curYLoc-1, 0);
				checkXLoc2 = MAX(curXLoc-1, 0);
				break;

		case DIR_NE:
				checkXLoc1 = MIN(curXLoc+1, MAX_WORLD_X_LOC-1);
				checkYLoc1 = curYLoc;
				checkXLoc2 = curXLoc;
				checkYLoc2 = MAX(curYLoc-1, 0);
				break;

		case DIR_E:
				checkXLoc1 = checkXLoc2 = MIN(curXLoc+1, MAX_WORLD_X_LOC-1);
				checkYLoc1 = MIN(curYLoc+1, MAX_WORLD_Y_LOC-1);
				checkYLoc2 = MAX(curYLoc-1, 0);
				break;

		case DIR_SE:
				checkXLoc1 = curXLoc;
				checkYLoc1 = MIN(curYLoc+1, MAX_WORLD_Y_LOC-1);
				checkXLoc2 = MIN(curXLoc+1, MAX_WORLD_X_LOC-1);
				checkYLoc2 = curYLoc;
				break;

		case DIR_S:
				checkXLoc1 = MAX(curXLoc-1, 0);
				checkYLoc1 = checkYLoc2 = MIN(curYLoc+1, MAX_WORLD_Y_LOC-1);
				checkXLoc2 = MIN(curXLoc+1, MAX_WORLD_X_LOC-1);
				break;

		case DIR_SW:
				checkXLoc1 = MAX(curXLoc-1, 0);
				checkYLoc1 = curYLoc;
				checkXLoc2 = curXLoc;
				checkYLoc2 = MIN(curYLoc, MAX_WORLD_Y_LOC-1);
				break;

		case DIR_W:
				checkXLoc1 = checkXLoc2 = MAX(curXLoc-1, 0);
				checkYLoc1 = MAX(curYLoc-1, 0);
				checkYLoc2 = MIN(curYLoc+1, MAX_WORLD_Y_LOC-1);
				break;

		case DIR_NW:
				checkXLoc1 = curXLoc;
				checkYLoc1 = MAX(curYLoc-1, 0);
				checkXLoc2 = MAX(curXLoc-1, 0);
				checkYLoc2 = curYLoc;
				break;
	}

	locPtr = world.get_loc(checkXLoc1, checkYLoc1);
	if(locPtr->can_move(mobile_type))
		set_path_to(checkXLoc1, checkYLoc1);
	else
	{
		locPtr = world.get_loc(checkXLoc2, checkYLoc2);
		if(locPtr->can_move(mobile_type))
			set_path_to(checkXLoc2, checkYLoc2);
		else
			return 0;
	}

	return 1;
}*/
//------- End of function Unit::blocked_move_new_handle --------//


//------ Begin of function Unit::set_path_to -------//
/*void Unit::set_path_to(int destXLoc, int destYLoc)
{
	//reset_path();
	terminate_move();
	result_node_array = (ResultNode*) mem_add(2*sizeof(ResultNode));
	ResultNode *curNodePtr = result_node_array;
	curNodePtr->node_x = next_x_loc();
	curNodePtr->node_y = next_y_loc();
	curNodePtr++;
	curNodePtr->node_x = destXLoc;
	curNodePtr->node_y = destYLoc;

	result_node_count = 2;
	result_node_recno = 1;
	result_path_dist = 1;
	move_to_x_loc = destXLoc;
	move_to_y_loc = destYLoc;
	go_x = destXLoc*ZOOM_LOC_WIDTH;
	go_y = destYLoc*ZOOM_LOC_HEIGHT;

	//set_dir(cur_x, cur_y, go_x, go_y);
	//set_wait();
	next_move();
}*/
//------- End of function Unit::set_path_to --------//


//------ Begin of function Unit::handle_blocked_move -------//
// Note: it assumes the given location is blocked, it makes no
//       further attempt to verify it.
//
// <Location*>	blockedLoc	-	blocked location
//
// return : <int> 1 - handled successfully
//                0 - cannot be handled, the movement must be terminated
//
void Unit::handle_blocked_move(Location* blockedLoc)
{
	//--- check if the tile we are moving at is blocked by a building ---//
	if(blockedLoc->is_firm() || blockedLoc->is_town() || blockedLoc->is_wall())
   {
		//------------------------------------------------//
		// firm/town/wall is on the blocked location
		//------------------------------------------------//
		reset_path();
		search_or_stop(move_to_x_loc, move_to_y_loc, 1);
		//search(move_to_x_loc, move_to_y_loc, 1);
      return;
   }

	if(next_x_loc()==move_to_x_loc && next_y_loc()==move_to_y_loc && !swapping)
	{
		terminate_move(); // terminate since already reaching destination
		return;
	}
	
	if(!blockedLoc->is_accessible(mobile_type))
	{
		terminate_move();  // the location is not accessible
		err_when(cur_x==go_x && cur_y==go_y && (cur_x!=next_x || cur_y!=next_y));
		return;
	}
	
	//-----------------------------------------------------------------------------------//
	// there is another sprite on the move_to location, check the combination of both sizes
	//-----------------------------------------------------------------------------------//
	blocked_by_member = 1;
	err_when(!blockedLoc->unit_recno(mobile_type));
	
	Unit* unitPtr = unit_array[blockedLoc->unit_recno(mobile_type)];
	//if(unitPtr->sprite_info->loc_width>1 || sprite_info->loc_width>1)
	//{
	//	set_wait();
	//	return;
	//}
	//else
	handle_blocked_move_s11(unitPtr); //------ both units size 1x1

	err_when(cur_x==go_x && cur_y==go_y && (cur_x!=next_x || cur_y!=next_y));
	return;
}
//------- End of function Unit::handle_blocked_move --------//


//------ Begin of function Unit::handle_blocked_by_idle_unit ---------//
// handle the case blocked by idle unit
//
// <Unit*>	unitPtr -	the unit blocking this unit
//
void Unit::handle_blocked_by_idle_unit(Unit *unitPtr)
{
	#define TEST_DIMENSION	10
	#define TEST_LIMIT		TEST_DIMENSION*TEST_DIMENSION

	char notLandUnit = (mobile_type!=UNIT_LAND);
	int unitXLoc = unitPtr->next_x_loc();
	int unitYLoc = unitPtr->next_y_loc();
	int xShift, yShift;
	int checkXLoc, checkYLoc;
	Location *locPtr;

	int xSign = misc.random(2) ? 1 : -1;
	int ySign = misc.random(2) ? 1 : -1;
	
	for(int i=2; i<=TEST_LIMIT; i++)
	{
		misc.cal_move_around_a_point(i, TEST_DIMENSION, TEST_DIMENSION, xShift, yShift);
		xShift *= xSign;
		yShift *= ySign;
		
		if(notLandUnit)
		{
			checkXLoc = unitXLoc + xShift*2;
			checkYLoc = unitYLoc + yShift*2;
		}
		else
		{
			checkXLoc = unitXLoc + xShift;
			checkYLoc = unitYLoc + yShift;
		}

		if(checkXLoc<0 || checkXLoc>=MAX_WORLD_X_LOC || checkYLoc<0 || checkYLoc>=MAX_WORLD_Y_LOC)
			continue;

		locPtr = world.get_loc(checkXLoc , checkYLoc);
		if(!locPtr->can_move(unitPtr->mobile_type))
			continue;

		if(on_my_path(checkXLoc, checkYLoc))
			continue;

		unitPtr->move_to(checkXLoc, checkYLoc);
		set_wait();
		return;
	}

	stop(KEEP_DEFENSE_MODE);

	//--------------------------------------------------------------------------------//
	// improved version!!!
	//--------------------------------------------------------------------------------//
	/*int testResult = 0, worstCase=0;
	int worstXLoc=-1, worstYLoc=-1;
	int startCount, endCount;
	int i, j;
	
	for(j=0; j<2; j++)
	{
		//----------- set the startCount and endCount ------------//
		if(j==0)
		{
			startCount = 2;
			endCount = 9;
		}
		else
		{
			startCount = 10;
			endCount = TEST_LIMIT;
		}

		for(i=startCount; i<=endCount; i++)
		{
			misc.cal_move_around_a_point(i, TEST_DIMENSION, TEST_DIMENSION, xShift, yShift);
			if(notLandUnit)
			{
				checkXLoc = unitXLoc + xShift*2;
				checkYLoc = unitYLoc + yShift*2;
			}
			else
			{
				checkXLoc = unitXLoc + xShift;
				checkYLoc = unitYLoc + yShift;
			}
		
			if(checkXLoc<0 || checkXLoc>=MAX_WORLD_X_LOC || checkYLoc<0 || checkYLoc>=MAX_WORLD_Y_LOC)
				continue;

			locPtr = world.get_loc(checkXLoc , checkYLoc);
			if(!locPtr->can_move(unitPtr->mobile_type))
				continue;

			//-------------------------------------------------------------------//
			// a possible location
			//-------------------------------------------------------------------//
			testResult = on_my_path(checkXLoc, checkYLoc);
			if(testResult)
			{
				if(j==0 && !worstCase)
				{
					worstCase++;
					worstXLoc = checkXLoc;
					worstYLoc = checkYLoc;
				}
				continue;
			}

			unitPtr->move_to(checkXLoc, checkYLoc):
			set_wait();
			return;
		}
	}

	//-------------------------------------------------------------------//
	if(worstCase)
	{
		unitPtr->move_to(worstXLoc, worstYLoc);
		set_wait();
	}
	else
		stop(KEEP_DEFENSE_MODE);*/
}
//------- End of function Unit::handle_blocked_by_idle_unit --------//


//------ Begin of function Unit::on_my_path ---------//
// This function is used to check whether a location
// (checkXLoc, checkYLoc) is on the unit path, result_node_array.
//
// return 1 if true
// return 0 otherwise
//
int Unit::on_my_path(short checkXLoc, short checkYLoc)
{
	err_when(result_node_count<2);
	ResultNode* curNodePtr = result_node_array+result_node_recno-2;
	ResultNode* nextNodePtr = curNodePtr+1;

	for(int i=result_node_recno-1; i<result_node_count; i++, curNodePtr++, nextNodePtr++)
	{
		if((curNodePtr->node_x-checkXLoc)*(checkYLoc-nextNodePtr->node_y)==
			(curNodePtr->node_y-checkYLoc)*(checkXLoc-nextNodePtr->node_x)) // point of division
			return 1;
	}

	return 0;
}
//------- End of function Unit::on_my_path --------//


//------ Begin of function Unit::handle_blocked_wait ---------//
//
//	this function is worked for unit size 1x1 only to handle case that
// blocked by waiting unit
//
// <Unit*>	unitPtr	-	unit blocking this unit
//
void Unit::handle_blocked_wait(Unit* unitPtr)
{
	err_when(sprite_info->loc_width>1 || unitPtr->sprite_info->loc_width>1);

	int			stepMagn = move_step_magn();
	short			cycleWait = 0;
	Location		*locPtr;

	if(is_dir_correct())
	{
		Unit			*blockedUnitPtr = unitPtr;
		SpriteInfo	*unitSpriteInfo = unitPtr->sprite_info;
		int			nextX, nextY, loop = 0, i;
		short			blocked=0;

		//---------------------------------------------------------------//
		// construct a cycle_waiting array to store the sprite_recno of
		// those units in cycle_waiting in order to prevent forever looping
		// in the checking
		//---------------------------------------------------------------//
		int arraySize = 20;
		cycle_wait_unit_array_def_size = arraySize;
		cycle_wait_unit_index = 0;
		cycle_wait_unit_array_multipler = 1;
		cycle_wait_unit_array = (short*)mem_add(sizeof(short)*cycle_wait_unit_array_def_size);
		memset(cycle_wait_unit_array, 0, sizeof(short)*cycle_wait_unit_array_def_size);

		//---------------------------------------------------------------//
		// don't handle the case blocked by size 2x2 unit in this moment
		//---------------------------------------------------------------//
		while(!cycleWait && blockedUnitPtr->cur_action==SPRITE_WAIT)
		{
			if(unitSpriteInfo->loc_width>1)
				break; // don't handle unit size > 1

			if(!blockedUnitPtr->is_dir_correct())
				break;

			//----------------------------------------------------------------------------------------//
			// cur_x, cur_y of unit pointed by blockedUnitPtr should be exactly inside a tile
			//----------------------------------------------------------------------------------------//
			nextX = blockedUnitPtr->cur_x+stepMagn*move_x_pixel_array[blockedUnitPtr->final_dir];
			nextY = blockedUnitPtr->cur_y+stepMagn*move_y_pixel_array[blockedUnitPtr->final_dir];

			//---------- calculate location blocked unit attempts to move to ---------//
			nextX >>= ZOOM_X_SHIFT_COUNT;
			nextY >>= ZOOM_Y_SHIFT_COUNT;

			locPtr = world.get_loc(nextX, nextY);
			blocked = locPtr->has_unit(mobile_type);

			//---------------- the unit is also waiting ---------------//
			if(blocked && (blockedUnitPtr->move_to_x_loc!=blockedUnitPtr->cur_x_loc() ||
				blockedUnitPtr->move_to_y_loc!=blockedUnitPtr->cur_y_loc()))
			{
				if(locPtr->unit_recno(mobile_type) == sprite_recno)
					cycleWait = 1;
				else
				{
					for(i=0; i<cycle_wait_unit_index; i++)
					{
						//---------- checking for forever loop ----------------//
						if(cycle_wait_unit_array[i] == blockedUnitPtr->sprite_recno)
						{
							loop = 1;
							break;
						}
					}

					if(loop)
						break;

					//------------------------------------------------------//
					// resize array if required size is larger than arraySize
					//------------------------------------------------------//
					if(cycle_wait_unit_index >= arraySize)
					{
						cycle_wait_unit_array_multipler++;
						arraySize = cycle_wait_unit_array_def_size*cycle_wait_unit_array_multipler;
						cycle_wait_unit_array = (short*) mem_resize(cycle_wait_unit_array, sizeof(short)*arraySize);
					}
					else
					{
						//-------- store recno of next blocked unit ----------//
						cycle_wait_unit_array[cycle_wait_unit_index++] = blockedUnitPtr->sprite_recno;
						locPtr = world.get_loc(nextX, nextY);
						err_when(blockedUnitPtr->mobile_type!=mobile_type);
						blockedUnitPtr = unit_array[locPtr->unit_recno(mobile_type)];
						unitSpriteInfo = blockedUnitPtr->sprite_info;
					}
				}
			}
			else
				break;
		}

		//---------- deinit data structure -------//
		mem_del(cycle_wait_unit_array);
		cycle_wait_unit_array = NULL;
	}

	if(cycleWait)
	{
		//----------------------------------------------------------------------//
		// shift the recno of all the unit in the cycle
		//----------------------------------------------------------------------//
		err_when(!is_dir_correct());
		
		short backupSpriteRecno;
		world.set_unit_recno(cur_x_loc(), cur_y_loc(), mobile_type, 0); // empty the firt node in the cycle
		cycle_wait_shift_recno(this, unitPtr);	// shift all the unit in the cycle
		backupSpriteRecno = world.get_unit_recno(cur_x_loc(), cur_y_loc(), mobile_type);
		world.set_unit_recno(cur_x_loc(), cur_y_loc(), mobile_type, sprite_recno);
		set_next(unitPtr->cur_x, unitPtr->cur_y, -stepMagn, 1);
		set_move();
		world.set_unit_recno(unitPtr->cur_x_loc(), unitPtr->cur_y_loc(), mobile_type, sprite_recno);
		world.set_unit_recno(cur_x_loc(), cur_y_loc(), mobile_type, backupSpriteRecno);
		swapping = 1;
	}
	else // not in a cycle
	{
		set_wait();

		//if(waiting_term>=MAX_WAITING_TERM_SAME)
		if(waiting_term>=MAX_WAITING_TERM_SAME*move_step_magn())
		{
			//-----------------------------------------------------------------//
			// codes used to speed up frame rate
			//-----------------------------------------------------------------//
			locPtr = world.get_loc(move_to_x_loc, move_to_y_loc);
			if(!locPtr->can_move(mobile_type) && action_mode2!=ACTION_MOVE)
				stop(KEEP_PRESERVE_ACTION); // let reactivate..() call searching later
			else
				search_or_wait();

			waiting_term = 0;
		}
	}
}
//-------- End of function Unit::handle_blocked_wait ---------//


//------ Begin of function Unit::cycle_wait_shift_recno ---------//
// copy recno of curUnit to location where nextUnit on 
//
// <Unit*> curUnit	-	
// <Unit*> nextUnit	-	
//
void Unit::cycle_wait_shift_recno(Unit* curUnit, Unit* nextUnit)
{
	err_when(!curUnit->is_dir_correct() || !nextUnit->is_dir_correct());
	
	int			stepMagn = move_step_magn();
	Unit			*blockedUnitPtr;
	Location		*locPtr;

	//----------- find the next location ------------//
	int nextX = nextUnit->cur_x+stepMagn*move_x_pixel_array[nextUnit->final_dir];
	int nextY = nextUnit->cur_y+stepMagn*move_y_pixel_array[nextUnit->final_dir];

	nextX >>= ZOOM_X_SHIFT_COUNT;
	nextY >>= ZOOM_Y_SHIFT_COUNT;

	if(nextX != cur_x_loc() || nextY != cur_y_loc())
	{
		locPtr = world.get_loc(nextX, nextY);
		blockedUnitPtr = unit_array[locPtr->unit_recno(nextUnit->mobile_type)];
	}
	else
		blockedUnitPtr = this;

	err_when(!blockedUnitPtr);

	if(blockedUnitPtr != this)
	{
		cycle_wait_shift_recno(nextUnit, blockedUnitPtr);
		nextUnit->set_next(blockedUnitPtr->cur_x, blockedUnitPtr->cur_y, -stepMagn, 1);
		nextUnit->set_move();
		world.set_unit_recno(blockedUnitPtr->cur_x_loc(), blockedUnitPtr->cur_y_loc(), nextUnit->mobile_type, nextUnit->sprite_recno);
		world.set_unit_recno(nextUnit->cur_x_loc(), nextUnit->cur_y_loc(), nextUnit->mobile_type, 0);
		nextUnit->swapping = 1;
	}
	else // the cycle shift is ended
	{
		err_when(blockedUnitPtr != this);
		err_when(nextUnit->cur_x!=nextUnit->next_x || nextUnit->cur_y!=nextUnit->next_y);
		err_when(nextUnit->cur_action != SPRITE_WAIT);

		nextUnit->set_next(cur_x, cur_y, -stepMagn, 1);
		nextUnit->set_move();
		world.set_unit_recno(cur_x_loc(), cur_y_loc(), nextUnit->mobile_type, nextUnit->sprite_recno);
		world.set_unit_recno(nextUnit->cur_x_loc(), nextUnit->cur_y_loc(), nextUnit->mobile_type, 0);

		nextUnit->swapping = 1;
	}
}
//-------- End of function Unit::cycle_wait_shift_recno ---------//


//------ Begin of function Unit::opposite_direction_blocked ---------//
// the two units are oppositely blocked, handle swapping
//
void Unit::opposite_direction_blocked(short vecX, short vecY, short unitPtrVecX, short unitPtrVecY, Unit* unitPtr)
{
	//---------------------------------------------------------------------------//
	// processing swapping only when both units are exactly in the tiles
	//---------------------------------------------------------------------------//
	if(unitPtr->cur_action!=SPRITE_IDLE)	
	{
		if(unitPtr->move_to_x_loc!=move_to_x_loc || unitPtr->move_to_y_loc!=move_to_y_loc)
		{
			int stepMagn = move_step_magn();

			world.set_unit_recno(unitPtr->cur_x_loc(), unitPtr->cur_y_loc(), mobile_type, 0);
			err_when(!is_dir_correct());
			set_next(unitPtr->cur_x, unitPtr->cur_y, -stepMagn, -1);

			world.set_unit_recno(unitPtr->cur_x_loc(), unitPtr->cur_y_loc(), mobile_type, unitPtr->sprite_recno);
			world.set_unit_recno(cur_x_loc(), cur_y_loc(), unitPtr->mobile_type, 0);
			err_when(!unitPtr->is_dir_correct());
			unitPtr->set_next(cur_x, cur_y, -stepMagn, 1);

			world.set_unit_recno(unitPtr->cur_x_loc(), unitPtr->cur_y_loc(), mobile_type, sprite_recno);
			world.set_unit_recno(cur_x_loc(), cur_y_loc(), unitPtr->mobile_type, unitPtr->sprite_recno);

			set_move();
			unitPtr->set_move();

			swapping = 1;
			unitPtr->swapping = 1;

		#ifdef DEBUG
			ResultNode* curNode;
			curNode = result_node_array + result_node_count - 1;
			err_when(curNode->node_x!=move_to_x_loc || curNode->node_y!=move_to_y_loc);
			curNode = unitPtr->result_node_array + unitPtr->result_node_count - 1;
			err_when(curNode->node_x!=unitPtr->move_to_x_loc || curNode->node_y!=unitPtr->move_to_y_loc);
		#endif
		}
		else
			terminate_move();
	}
	else
	{
		//----------------------------------------------------------------------//
		// If the unit pointed by unitPtr (unit B) has the same unit_id, rank_id
		//	and both	are in the same group, this unit will order the other unit to
		// move to its location and this unit will occupy the location of the unit B.
		//
		// If the above condition is not fulfilled, swapping is processed.
		//----------------------------------------------------------------------//
		if(unit_id!=unitPtr->unit_id || rank_id!=unitPtr->rank_id || unit_group_id!=unitPtr->unit_group_id)
		{
			if(unitPtr->move_to_x_loc!=move_to_x_loc || unitPtr->move_to_y_loc!=move_to_y_loc)
			{
				//----------------- process swapping ---------------//
				set_wait();
				unitPtr->set_dir(unitPtr->next_x, unitPtr->next_y, next_x, next_y);
				set_dir(next_x, next_y, unitPtr->next_x, unitPtr->next_y);
				
				err_when(unitPtr->result_node_array!=NULL);
					
				unitPtr->result_node_array = (ResultNode*)mem_add(sizeof(ResultNode)*2);
				ResultNode* nodePtr = unitPtr->result_node_array;

				err_when(next_x_loc()!= cur_x_loc() || next_y_loc()!=cur_y_loc());
				nodePtr->node_x = next_x_loc();
				nodePtr->node_y = next_y_loc();
				nodePtr++;
				nodePtr->node_x = unitPtr->next_x_loc();
				nodePtr->node_y = unitPtr->next_y_loc();
				unitPtr->result_node_count = 2;
				unitPtr->result_node_recno = 1;

				unitPtr->set_wait();
				unitPtr->go_x = next_x;
				unitPtr->go_y = next_y;

				unitPtr->result_path_dist = 2;

				err_when((check_unit_dir1=get_dir(cur_x, cur_y, go_x, go_y))!=final_dir);
				swapping = 1;
				unitPtr->swapping = 1;
			}
			else
				terminate_move();
		}
		else
		{
			//------------ process move_to_my_loc or terminate the movement -----------//
			if(unitPtr->move_to_x_loc!=move_to_x_loc || unitPtr->move_to_y_loc!=move_to_y_loc)
				move_to_my_loc(unitPtr);
			else
				terminate_move();
		}

		#ifdef DEBUG
			ResultNode* curNode;
			if(result_node_array!=NULL)
			{
				curNode = result_node_array + result_node_count - 1;
				err_when(curNode->node_x!=move_to_x_loc || curNode->node_y!=move_to_y_loc);
			}
			if(unitPtr->result_node_array!=NULL)
			{
				curNode = unitPtr->result_node_array + unitPtr->result_node_count - 1;
				err_when(curNode->node_x!=unitPtr->move_to_x_loc || curNode->node_y!=unitPtr->move_to_y_loc);
			}
		#endif
	}
}
//-------- End of function Unit::opposite_direction_blocked ---------//


//------ Begin of function Unit::terminate_move -------//
//
// When the sprite has finished moving the next tile in
// the path. If the following tile is blocked and the whole
// movement need to be terminated, this function is called.
//
// When SPRITE_IDLE:
//
// (next_x, next_y) must be == (cur_x, cur_y), it's the location of the sprite.
//
void Unit::terminate_move()
{
	#ifdef DEBUG	
		err_when( next_x != cur_x || next_y != cur_y );
		short h, y;
		for(h=0, y=cur_y_loc(); h<sprite_info->loc_height; h++, y++)
			for(short w=0, x=cur_x_loc(); w<sprite_info->loc_width; w++, x++)
				err_when( world.get_unit_recno(x, y, mobile_type) != sprite_recno );
	#endif

   go_x = next_x;
   go_y = next_y;

	move_to_x_loc = next_x_loc();
	move_to_y_loc = next_y_loc();

	#ifdef DEBUG
		char blocked=0;
		for(h=0, y=next_y_loc(); h<sprite_info->loc_height&&!blocked; h++, y++)
			for(short		w=0, x=next_x_loc(); w<sprite_info->loc_width&&!blocked; w++, x++)
				blocked = world.get_unit_recno(x,y,mobile_type) != sprite_recno;
		err_when(blocked);
	#endif

	cur_frame  = 1;

   reset_path();
	set_idle();

	err_when(result_node_array!=NULL);
}
//------- End of function Unit::terminate_move --------//


//------------- Begin of function Unit::move_to_my_loc --------------//
//
// This function is used as this unit Unit A is blocked by another
// IDLE unit Unit B. Then Unit A will move to the location of Unit B
// and Unit B will move to the location of Unit A want to move to.
//
// Note: action_?_loc2 of both units can be different from their
//			move_to_?_loc
//
void Unit::move_to_my_loc(Unit* unitPtr)
{
	int unitDestX, unitDestY;
	if(unitPtr->action_mode2==ACTION_MOVE)
	{
		unitDestX = unitPtr->action_x_loc2;
		unitDestY = unitPtr->action_y_loc2;
		err_when(unitDestX==-1 || unitDestY==-1);
	}
	else
	{
		unitDestX = unitPtr->move_to_x_loc;
		unitDestY = unitPtr->move_to_y_loc;
	}

	//--------------- init parameters ---------------//
	int unitCurX = unitPtr->next_x_loc();
	int unitCurY = unitPtr->next_y_loc();
	int destX = action_x_loc2;
	int destY = action_y_loc2;
	int curX = next_x_loc();
	int curY = next_y_loc();
	int moveScale = move_step_magn();
	err_when(curX != cur_x_loc() || curY != cur_y_loc());
	err_when(mobile_type!=unitPtr->mobile_type);
	
	//------------------------------------------------------------------//
	// setting for unit pointed by unitPtr
	//------------------------------------------------------------------//
	if(result_node_array==NULL)	//************BUGHERE
	{
		err_when(unitPtr->cur_action!=SPRITE_IDLE);
		unitPtr->move_to(destX, destY, 1); // unit pointed by unitPtr is idle before calling searching
		err_when(unitPtr->cur_action!=SPRITE_DIE && unitPtr->action_mode2!=ACTION_MOVE);
	}
	else	
	{
		err_when(result_node_recno<1 || unitPtr->result_node_array!=NULL);
		ResultNode* resultNode = result_node_array+result_node_recno-1;
		if(go_x == unitPtr->next_x && go_y == unitPtr->next_y)
		{
			//------ Unit B is in one of the node of the result_node_array ---//
			unitPtr->result_node_count = result_node_count-result_node_recno+1;	// at least there are 2 nodes
			unitPtr->result_node_array = (ResultNode*)mem_add(sizeof(ResultNode)*(unitPtr->result_node_count));
			memcpy(unitPtr->result_node_array, resultNode, sizeof(ResultNode)*(unitPtr->result_node_count));
		}
		else
		{
			//----- Unit B is in the middle of two nodes in the result_node_array -----//
			unitPtr->result_node_count = result_node_count-result_node_recno+2;
			unitPtr->result_node_array = (ResultNode*)mem_add(sizeof(ResultNode)*(unitPtr->result_node_count));
			ResultNode* curNode = unitPtr->result_node_array;
			curNode->node_x = unitCurX;
			curNode->node_y = unitCurY;
			curNode++;
			memcpy(curNode, resultNode, sizeof(ResultNode)*(unitPtr->result_node_count-1));
		}
		err_when(unitPtr->result_node_count<2);

		//--------------- set unit action ---------------//
		if(unitPtr->action_mode2==ACTION_STOP || unitPtr->action_mode2==ACTION_MOVE) // unitPtr is idle now
		{
			//---------- activate unit pointed by unitPtr now ------------//
			unitPtr->action_mode	 = unitPtr->action_mode2 = ACTION_MOVE;
			unitPtr->action_para  = unitPtr->action_para2 = 0;
			if(destX!=-1 && destY!=-1)
			{
				unitPtr->action_x_loc = unitPtr->action_x_loc2 = destX;
				unitPtr->action_y_loc = unitPtr->action_y_loc2 = destY;
			}
			else
			{
				ResultNode *lastNodePtr = unitPtr->result_node_array + unitPtr->result_node_count - 1;
				unitPtr->action_x_loc = unitPtr->action_x_loc2 = lastNodePtr->node_x;
				unitPtr->action_y_loc = unitPtr->action_y_loc2 = lastNodePtr->node_y;
			}
		}

		//----------------- set unit movement parameters -----------------//
		unitPtr->result_node_recno = 1;
		unitPtr->result_path_dist = result_path_dist-moveScale;
		unitPtr->move_to_x_loc = move_to_x_loc;
		unitPtr->move_to_y_loc = move_to_y_loc;
		err_when( next_x != cur_x || next_y != cur_y );
		unitPtr->next_move();

		#ifdef DEBUG
			if(unitPtr->result_node_array!=NULL)
			{
				ResultNode* curNode1 = unitPtr->result_node_array + unitPtr->result_node_recno - 1;
				err_when(curNode1->node_x!=unitPtr->go_x>>ZOOM_X_SHIFT_COUNT || curNode1->node_y!=unitPtr->go_y>>ZOOM_Y_SHIFT_COUNT);
			}
		#endif
	}

	//------------------------------------------------------------------//
	// setting for this unit
	//------------------------------------------------------------------//
	int shouldWait = 0;
	if(next_x==unitPtr->cur_x && next_y==unitPtr->cur_y)
	{
		reset_path();
		result_path_dist = 0;
	}
	else
	{
		terminate_move();
		shouldWait++;
		result_path_dist = moveScale;
	}

	go_x = unitPtr->cur_x;
	go_y = unitPtr->cur_y;
	move_to_x_loc = unitCurX;
	move_to_y_loc = unitCurY;

	if(action_mode2==ACTION_MOVE)
	{
		action_x_loc = action_x_loc2 = unitDestX;
		action_y_loc = action_y_loc2 = unitDestY;
	}

	//---------- note: the cur_dir is already the correct direction ---------------//
	err_when(result_node_array!=NULL);
	result_node_array = (ResultNode*)mem_add(sizeof(ResultNode)*2);
	ResultNode* nodePtr = result_node_array;
	nodePtr->node_x = curX;
	nodePtr->node_y = curY;
	nodePtr++;
	nodePtr->node_x = unitCurX;
	nodePtr->node_y = unitCurY;
	result_node_count = 2;
	result_node_recno = 2;
	if(shouldWait)
		set_wait();	// wait for the blocking unit to move first
	
	#ifdef DEBUG
		if(result_node_array!=NULL)
		{
			ResultNode* curNode2 = result_node_array + result_node_recno - 1;
			err_when(curNode2->node_x!=go_x>>ZOOM_X_SHIFT_COUNT || curNode2->node_y!=go_y>>ZOOM_Y_SHIFT_COUNT);
		}
	#endif

	err_when(cur_action==SPRITE_IDLE && (move_to_x_loc!=next_x_loc() || move_to_y_loc!=next_y_loc()));
	err_when(cur_action==SPRITE_IDLE && (cur_x!=next_x || cur_y!=next_y));
	err_when(unitPtr->cur_action==SPRITE_IDLE && (unitPtr->move_to_x_loc!=unitPtr->next_x_loc() ||
				unitPtr->move_to_y_loc!=unitPtr->next_y_loc()));
}
//----------------- End of function Unit::move_to_my_loc ----------------//

//###### begin trevor 25/6 #######//

/*
//------------- Begin of function Unit::change_relation --------------//
void Unit::change_relation(short nation1, short nation2, int relationType)
{
	if(!nation1 || !nation2 || nation1==nation2)
		return; // return if either is neutral nation or same nation

	nation_array[nation1]->set_relation_status(nation2, relationType);
}
//----------------- End of function Unit::change_relation ----------------//
*/

//####### end trevor 25/6 ########//


//------------- Begin of function Unit::set_idle --------------//
// set parameters for unit idle
//
void Unit::set_idle()
{
	/*err_when(unit_res[unit_id]->unit_class==UNIT_CLASS_SHIP &&
				(((UnitMarine*)this)->extra_move_in_beach==EXTRA_MOVING_IN ||
				((UnitMarine*)this)->extra_move_in_beach==EXTRA_MOVING_OUT));*/
	err_when(cur_x<0);
	err_when(move_to_x_loc!=next_x_loc() || move_to_y_loc!=next_y_loc());
	err_when(cur_x!=next_x || cur_y!=next_y || next_x!=go_x || next_y!=go_y);
	err_when(cur_x%ZOOM_LOC_WIDTH || cur_y%ZOOM_LOC_HEIGHT);

	err_when(result_path_dist || result_node_array);
	err_when(cur_dir<0 || cur_dir>MAX_SPRITE_DIR_TYPE);
	final_dir = cur_dir;
	turn_delay = 0;
	cur_action = SPRITE_IDLE;
}
//----------------- End of function Unit::set_idle ----------------//


//------------- Begin of function Unit::set_ready --------------//
// set parameters for unit ready to move
//
void Unit::set_ready()
{
	err_when(cur_x<0);
	err_when(move_to_x_loc!=next_x_loc() || move_to_y_loc!=next_y_loc());
	err_when(cur_x!=next_x || cur_y!=next_y || next_x!=go_x || next_y!=go_y);
	err_when(cur_x%ZOOM_LOC_WIDTH || cur_y%ZOOM_LOC_HEIGHT);
	
	err_when(cur_dir<0 || cur_dir>MAX_SPRITE_DIR_TYPE);
	final_dir = cur_dir;
	turn_delay = 0;
	cur_action = SPRITE_READY_TO_MOVE;
}
//----------------- End of function Unit::set_ready ----------------//


//------------- Begin of function Unit::set_move --------------//
// set parameters for unit movement
//
void Unit::set_move()
{
	err_when(cur_x<0);
	err_when(cur_dir!=final_dir);
	cur_action = SPRITE_MOVE;
}
//----------------- End of function Unit::set_move ----------------//


//------------- Begin of function Unit::set_wait --------------//
// Set the unit to waiting status.
// When SPRITE_WAIT:
// (next_x, next_y) must be == (cur_x, cur_y), it's the location of the sprite.
//
void Unit::set_wait()
{
	err_when(cur_x<0);

	#ifdef DEBUG	
		err_when(go_x==cur_x && go_y==cur_y);
		err_when( next_x != cur_x || next_y != cur_y );
	
		short w, h, x, y;
		for(h=0, y=cur_y_loc(); h<sprite_info->loc_height; h++, y++)
			for(w=0, x=cur_x_loc(); w<sprite_info->loc_width; w++, x++)
				err_when( world.get_unit_recno(x, y, mobile_type) != sprite_recno );
		err_when((check_unit_dir1=get_dir(cur_x, cur_y, go_x, go_y))!=final_dir);
	#endif

	cur_action = SPRITE_WAIT;
   cur_frame  = 1;
	waiting_term++;
}
//----------------- End of function Unit::set_wait ----------------//


//------------- Begin of function Unit::set_attack --------------//
// set parameters for unit attack
//
void Unit::set_attack()
{
	err_when(cur_x<0);
	err_when(next_x!=cur_x || next_y!=cur_y);
	err_when(cur_dir<0 || cur_dir>=MAX_SPRITE_DIR_TYPE || turn_delay);
	final_dir = cur_dir;
	turn_delay = 0;
	cur_action = SPRITE_ATTACK;
}
//----------------- End of function Unit::set_attack ----------------//


//------------- Begin of function Unit::set_turn --------------//
// set parameters for unit turning
//
void Unit::set_turn()
{
	err_when(cur_x<0);
	err_when(next_x!=cur_x || next_y!=cur_y);
	cur_action = SPRITE_TURN;
}
//----------------- End of function Unit::set_turn ----------------//


//------------- Begin of function Unit::set_ship_extra_move --------------//
// set parameters for ship extra moving in inlet
//
void Unit::set_ship_extra_move()
{
	err_when(cur_x<0);
	cur_action = SPRITE_SHIP_EXTRA_MOVE;
}
//----------------- End of function Unit::set_ship_extra_move ----------------//


//------------- Begin of function Unit::set_die --------------//
// set parameters for unit die
//
void Unit::set_die()
{
	if( action_mode == ACTION_DIE )
		return;

	err_when(hit_points>0);

	action_mode = ACTION_DIE;
	cur_action  = SPRITE_DIE;
	cur_frame   = 1;

   //##### begin trevor 19/7 #####//

	//---- if this unit is led by a leader, only mobile units has leader_unit_recno assigned to a leader -----//

	if( leader_unit_recno && !unit_array.is_deleted(leader_unit_recno) )		// the leader unit may have been killed at the same time
	{
		unit_array[leader_unit_recno]->del_team_member(sprite_recno);
		leader_unit_recno = 0;
	}

	//##### end trevor 19/7 #####//
}
//----------------- End of function Unit::set_die ----------------//

