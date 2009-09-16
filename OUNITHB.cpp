//Filename    : OUNITATM.CPP
//Description : Object Unit attack functions to handle blocked movement and group attacking
//Owner		  : Alex

#include <OWORLD.h>
#include <OFIRM.h>
#include <OTOWN.h>
#include <OUNIT.h>
#include <OTERRAIN.h>

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

//--------- Begin of function Unit::search_or_stop ---------//
// process searching or stop the units
//
// <int>		destX				-	x location to move to
// <int>		destY				-	y location to move to
// <int>		preserveAction	-	preserve action when calling to stop
// <short>	searchMode		-	search mode for searching
// <short>	miscNo			-	miscenllaneous information
//
void Unit::search_or_stop(int destX, int destY, int preserveAction, short searchMode, short miscNo)
{
	Location *locPtr = world.get_loc(destX, destY);
	if(!locPtr->can_move(mobile_type))
	{
		stop(KEEP_PRESERVE_ACTION); // let reactivate..() call searching later
		//waiting_term = MAX_SEARCH_OT_STOP_WAIT_TERM;
	}
	else
	{
		search(destX, destY, preserveAction, searchMode, miscNo);
		/*if(mobile_type==UNIT_LAND)
			search(destX, destY, preserveAction, searchMode, miscNo);
		else
			waiting_term = 0;*/
	}
}
//----------- End of function Unit::search_or_stop -----------//


//------ Begin of function Unit::search_or_wait ---------//
// call searching or wait
//
void Unit::search_or_wait()
{
	#define	SQUARE1	9
	#define	SQUARE2	25
	#define	SQUARE3	49
	#define	DIMENSION 7

	int curXLoc = next_x_loc(), curYLoc = next_y_loc();
	short surrArray[SQUARE3];
	int xShift, yShift, checkXLoc, checkYLoc, hasFree, i, shouldWait;
	short unitRecno;
	Unit *unitPtr;
	Location *locPtr;

	//----------------------------------------------------------------------------//
	// wait if the unit is totally blocked.  Otherwise, call searching
	//----------------------------------------------------------------------------//
	memset(surrArray, 0, sizeof(short)*SQUARE3);
	for(shouldWait=0, hasFree=0, i=2; i<=SQUARE3; i++)
	{
		if(i==SQUARE1 || i==SQUARE2 || i==SQUARE3)
		{
			if(!hasFree)
			{	
				shouldWait++;
				break;
			}
			else
				hasFree = 0;
		}

		m.cal_move_around_a_point(i, DIMENSION, DIMENSION, xShift, yShift);
		checkXLoc = curXLoc+xShift;
		checkYLoc = curYLoc+yShift;
		if(checkXLoc<0 || checkXLoc>=MAX_WORLD_X_LOC || checkYLoc<0 || checkYLoc>=MAX_WORLD_Y_LOC)
			continue;

		locPtr = world.get_loc(checkXLoc, checkYLoc);
		if(!locPtr->has_unit(mobile_type))
		{
			hasFree++;
			continue;
		}

		unitRecno = locPtr->unit_recno(mobile_type);
		err_when(!unitRecno);
		if(unit_array.is_deleted(unitRecno))
			continue;

		unitPtr = unit_array[unitRecno];
		if(unitPtr->nation_recno==nation_recno && unitPtr->unit_group_id==unit_group_id &&
			((unitPtr->cur_action==SPRITE_WAIT && unitPtr->waiting_term>1) || unitPtr->cur_action==SPRITE_TURN ||
			  unitPtr->cur_action==SPRITE_MOVE))
		{
			surrArray[i-2] = unitRecno;
			unitPtr->unit_group_id++;
		}
	}

	//------------------- call searching if should not wait --------------------//
	if(!shouldWait)
		search(move_to_x_loc, move_to_y_loc, 1, SEARCH_MODE_IN_A_GROUP);
		//search_or_stop(move_to_x_loc, move_to_y_loc, 1, SEARCH_MODE_IN_A_GROUP);

	for(i=0; i<SQUARE3; i++)
	{
		if(surrArray[i])
		{
			err_when(unit_array.is_deleted(surrArray[i]));
			unitPtr = unit_array[surrArray[i]];
			unitPtr->unit_group_id--;
		}
	}

	if(shouldWait)
		set_wait();
}
//-------- End of function Unit::search_or_wait ---------//


//------ Begin of function Unit::handle_blocked_move_s11 -------//
// both blocked and blocking are size 1
//
void Unit::handle_blocked_move_s11(Unit *unitPtr)
{
	err_when( world.get_unit_recno(next_x_loc(), next_y_loc(), mobile_type) != sprite_recno );
	err_when( world.get_unit_recno(unitPtr->next_x_loc(), unitPtr->next_y_loc(), unitPtr->mobile_type) != unitPtr->sprite_recno );
	err_when(cur_x!=next_x || cur_y!=next_y);

	int waitTerm;
	int moveStep = move_step_magn();

	switch(unitPtr->cur_action)
	{
		//------------------------------------------------------------------------------------//
		// handle blocked for units belonging to the same nation.  For those belonging to other
		// nations, wait for it moving to other locations or search for another path.
		//------------------------------------------------------------------------------------//
		case SPRITE_WAIT: // the blocking unit is waiting
				err_when(unitPtr->go_x==unitPtr->cur_x && unitPtr->go_y==unitPtr->cur_y);
		
		case SPRITE_TURN:
				if(unitPtr->nation_recno==nation_recno)
					handle_blocked_wait(unitPtr); // check for cycle wait for our nation
				else if(waiting_term>=MAX_WAITING_TERM_DIFF)
				{
					search_or_stop(move_to_x_loc, move_to_y_loc, 1); // recall searching
					waiting_term = 0;
				}
				else // wait
					set_wait();
				return;

		//------------------------------------------------------------------------------------//
		// We know from the cur_action of the blocking unit it is moving to another locations,
		// the blocked unit wait for a number of terms or search again.
		//------------------------------------------------------------------------------------//
		case SPRITE_MOVE:
		case SPRITE_READY_TO_MOVE:
		case SPRITE_SHIP_EXTRA_MOVE:
				if(unit_id!=UNIT_CARAVAN && unitPtr->unit_id==UNIT_CARAVAN) // don't wait for caravans, and caravans don't wait for other units
				{
					search(move_to_x_loc, move_to_y_loc, 1, SEARCH_MODE_A_UNIT_IN_GROUP);
				}
				else
				{
					waitTerm = (nation_recno==unitPtr->nation_recno) ? MAX_WAITING_TERM_SAME : MAX_WAITING_TERM_DIFF;
					if(waiting_term>=waitTerm)
					{
						search_or_wait();
						waiting_term = 0;
					}
					else
						set_wait();
				}
				return;

		//------------------------------------------------------------------------------------//
		// handling blocked for idle unit
		//------------------------------------------------------------------------------------//
		case SPRITE_IDLE:
				err_when(unitPtr->result_node_array!=NULL);
				if(unitPtr->action_mode==ACTION_SHIP_TO_BEACH)
				{
					//----------------------------------------------------------------------//
					// the blocking unit is trying to move to beach, so wait a number of terms,
					// or call searching again
					//----------------------------------------------------------------------//
					if(abs(unitPtr->next_x_loc()-unitPtr->action_x_loc2)<=moveStep &&
						abs(unitPtr->next_y_loc()-unitPtr->action_y_loc2)<=moveStep &&
						terrain_res[world.get_loc(unitPtr->action_x_loc2, unitPtr->action_y_loc2)->terrain_id]->average_type
						!=TERRAIN_OCEAN)
					{
						if(action_mode2==ACTION_SHIP_TO_BEACH && action_x_loc2==unitPtr->action_x_loc2 &&
							action_y_loc2==unitPtr->action_y_loc2)
						{
							int tempX, tempY;
							ship_to_beach(action_x_loc2, action_y_loc2, tempX, tempY);
						}
						else
						{
							waitTerm = (nation_recno==unitPtr->nation_recno) ? MAX_WAITING_TERM_SAME : MAX_WAITING_TERM_DIFF;
							if(waiting_term>=waitTerm)
								stop2();
							else
								set_wait();
						}
						return;
					}
				}

				if(unitPtr->nation_recno==nation_recno) //-------- same nation
				{
					//------------------------------------------------------------------------------------//
					// units from our nation
					//------------------------------------------------------------------------------------//
					if(unitPtr->unit_group_id==unit_group_id)
					{
						//--------------- from the same group -----------------//
						if(way_point_count && !unitPtr->way_point_count)
						{
							//------------ reset way point --------------//
							stop2();
							reset_way_point_array();
						}
						else if((unitPtr->next_x_loc() != move_to_x_loc || unitPtr->next_y_loc() != move_to_y_loc) &&
							(unitPtr->cur_action==SPRITE_IDLE && unitPtr->action_mode2==ACTION_STOP))
							move_to_my_loc(unitPtr); // push the blocking unit and exchange their destination
						else if(unitPtr->action_mode == ACTION_SETTLE)
							set_wait(); // wait for the settler
						else if(waiting_term>MAX_WAITING_TERM_SAME)
						{
							//---------- stop if wait too long ----------//
							terminate_move();
							waiting_term = 0;
						}
						else
							set_wait();
					}
					else if(unitPtr->action_mode2==ACTION_STOP)
						handle_blocked_by_idle_unit(unitPtr);
					else if(way_point_count && !unitPtr->way_point_count)
					{
						stop2();
						reset_way_point_array();
					}
					else
						search_or_stop(move_to_x_loc, move_to_y_loc, 1); // recall A* algorithm by default mode
				}
				else // different nation
				{
					//------------------------------------------------------------------------------------//
					// units from other nations
					//------------------------------------------------------------------------------------//
					if(unitPtr->next_x_loc() == move_to_x_loc && unitPtr->next_y_loc() == move_to_y_loc)
					{  
						terminate_move(); // destination occupied by other unit

						if(action_mode==ACTION_ATTACK_UNIT && unitPtr->nation_recno!=nation_recno && unitPtr->sprite_recno==action_para)
						{
							err_when(action_x_loc!=unitPtr->next_x_loc() || action_y_loc!=unitPtr->next_y_loc());
							err_when(action_mode2!=ACTION_ATTACK_UNIT && action_mode2!=ACTION_AUTO_DEFENSE_ATTACK_TARGET &&
										action_mode2!=ACTION_DEFEND_TOWN_ATTACK_TARGET && action_mode2!=ACTION_MONSTER_DEFEND_ATTACK_TARGET);
							err_when(action_para2!=action_para);
							err_when(action_x_loc!=action_x_loc2 || action_y_loc!=action_y_loc2);
							
							set_dir(next_x, next_y, unitPtr->next_x, unitPtr->next_y);
							if(is_dir_correct())
								attack_unit(action_para);
							else
								set_turn();
							cur_frame  = 1;
						}
					}
					else
						search_or_stop(move_to_x_loc, move_to_y_loc, 1); // recall A* algorithm by default mode
				}
				return;

		//------------------------------------------------------------------------------------//
		// don't wait for attackers from other nations, search for another path.
		//------------------------------------------------------------------------------------//
		case SPRITE_ATTACK:
				//----------------------------------------------------------------//
				// don't wait for other nation unit, call searching again
				//----------------------------------------------------------------//
				if(nation_recno!=unitPtr->nation_recno)
				{
					search_or_stop(move_to_x_loc, move_to_y_loc, 1);
					return;
				}

				//------------------------------------------------------------------------------------//
				// for attackers owned by our commander, handled blocked case by case as follows.
				//------------------------------------------------------------------------------------//
				switch(unitPtr->action_mode)
				{
					case ACTION_ATTACK_UNIT:
							if(action_para && !unit_array.is_deleted(action_para))
							{
								Unit *targetPtr = unit_array[action_para];
								handle_blocked_attack_unit(unitPtr, targetPtr);
							}
							else
								search_or_stop(move_to_x_loc, move_to_y_loc, 1, SEARCH_MODE_A_UNIT_IN_GROUP);
							break;

					case ACTION_ATTACK_FIRM:
							if(!unitPtr->action_para || firm_array.is_deleted(unitPtr->action_para))
								set_wait();
							else
								handle_blocked_attack_firm(unitPtr);
							break;

					case ACTION_ATTACK_TOWN:
							if(!unitPtr->action_para || town_array.is_deleted(unitPtr->action_para))
								set_wait();
							else
								handle_blocked_attack_town(unitPtr);
							break;

					case ACTION_ATTACK_WALL:
							if(unitPtr->action_para)
								set_wait();
							else
								handle_blocked_attack_wall(unitPtr);
							break;

					case ACTION_GO_CAST_POWER:
							set_wait();
							break;

					default: err_here();
								break;
				}
				return;

		//------------------------------------------------------------------------------------//
		// the blocked unit can pass after the blocking unit disappears in air.
		//------------------------------------------------------------------------------------//
		case SPRITE_DIE:
				set_wait();	// assume this unit will not wait too long
				return;

		default:
				err_here();
				break;
	}
	
	err_when(cur_x==go_x && cur_y==go_y && (cur_x!=next_x || cur_y!=next_y));
}
//------- End of function Unit::handle_blocked_move_s11 --------//


//------ Begin of function Unit::handle_blocked_same_target_attack ---------//
// the target of the blocked unit and this unit are same
//
// <Unit*>	unitPtr		-	pointer to blocking uit
// <Unit*>	targetPtr	-	pointer to target unit
//
void Unit::handle_blocked_same_target_attack(Unit* unitPtr, Unit* targetPtr)
{
	//----------------------------------------------------------//
	// this unit is now waiting and the unit pointed by unitPtr
	// is attacking the unit pointed by targetPtr
	//----------------------------------------------------------//
	err_when(cur_x%ZOOM_LOC_WIDTH || cur_y%ZOOM_LOC_HEIGHT);
	err_when(cur_x!=next_x || cur_y!=next_y);
	err_when(cur_x==go_x && cur_y==go_y);

	if(space_for_attack(action_x_loc, action_y_loc, targetPtr->mobile_type, targetPtr->sprite_info->loc_width, targetPtr->sprite_info->loc_height))
	{
		err_when(action_x_loc!=action_x_loc2 || action_y_loc!=action_y_loc2);
		search_or_stop(move_to_x_loc, move_to_y_loc, 1, SEARCH_MODE_TO_ATTACK, targetPtr->sprite_recno);
		//search(move_to_x_loc, move_to_y_loc, 1, SEARCH_MODE_TO_ATTACK, targetPtr->sprite_recno);
	}
	else if(in_any_defense_mode())
	{
		err_when(action_mode2!=ACTION_AUTO_DEFENSE_ATTACK_TARGET && action_mode2!=ACTION_DEFEND_TOWN_ATTACK_TARGET &&
					action_mode2!=ACTION_MONSTER_DEFEND_ATTACK_TARGET);

		general_defend_mode_detect_target();
	}
	else if(m.points_distance(next_x_loc(), next_y_loc(), action_x_loc, action_y_loc)<ATTACK_DETECT_DISTANCE)
	{
		//------------------------------------------------------------------------//
		// if the target is within the detect range, stop the unit's action to detect
		// another target if any exist. In case, there is no other target, the unit
		// will still attack the original target since it is the only target in the
		// detect range
		//------------------------------------------------------------------------//
		stop2(KEEP_DEFENSE_MODE);
	}
	else
		set_wait(); // set wait to stop the movement
}
//------- End of function Unit::handle_blocked_same_target_attack --------//


//--------- Begin of function Unit::handle_blocked_attack_unit ---------//
// the blocking unit is attacking against other unit
//
// <Unit*>	unitPtr		-	pointer to blocking uit
// <Unit*>	targetPtr	-	pointer to target unit
//
void Unit::handle_blocked_attack_unit(Unit* unitPtr, Unit* targetPtr)
{
	if(action_para==targetPtr->sprite_recno && unitPtr->action_para==targetPtr->sprite_recno &&
		action_mode==unitPtr->action_mode)
	{  //----------------- both attack the same target --------------------//
		err_when(unit_array.is_deleted(action_para));
		handle_blocked_same_target_attack(unitPtr, targetPtr);
	}
	else
		search_or_stop(move_to_x_loc, move_to_y_loc, 1, SEARCH_MODE_A_UNIT_IN_GROUP); // recall A* algorithm
		//search(move_to_x_loc, move_to_y_loc, 1, SEARCH_MODE_A_UNIT_IN_GROUP); // recall A* algorithm
}
//----------- End of function Unit::handle_blocked_attack_unit -----------//


//--------- Begin of function Unit::handle_blocked_attack_firm ---------//
// handle the case that the way of this unit to the target firm is blocked by
// another unit
//
// <Unit*> unitPtr	- the blocking unit
//
void Unit::handle_blocked_attack_firm(Unit *unitPtr)
{
	if(action_x_loc==unitPtr->action_x_loc && action_y_loc==unitPtr->action_y_loc && action_para==unitPtr->action_para &&
		action_mode==unitPtr->action_mode)
	{
		//------------- both attacks the same firm ------------//
		Location *locPtr = world.get_loc(action_x_loc, action_y_loc);
		if(!locPtr->is_firm())
			stop2(KEEP_DEFENSE_MODE);	// stop since firm is deleted
		else
		{
			Firm *firmPtr = firm_array[action_para];
			FirmInfo *firmInfo = firm_res[firmPtr->firm_id];

			if(space_for_attack(action_x_loc, action_y_loc, UNIT_LAND, firmInfo->loc_width, firmInfo->loc_height))
			{
				//------------ found surrounding place to attack the firm -------------//
				if(mobile_type==UNIT_LAND)
					set_move_to_surround(firmPtr->loc_x1, firmPtr->loc_y1, firmInfo->loc_width, firmInfo->loc_height, BUILDING_TYPE_FIRM_MOVE_TO);
				else
					attack_firm(firmPtr->loc_x1, firmPtr->loc_y1);
			}
			else // no surrounding place found, stop now
				stop(KEEP_PRESERVE_ACTION);
		}
	}
	else // let process_idle() handle it
		stop();
}
//----------- End of function Unit::handle_blocked_attack_firm -----------//


//--------- Begin of function Unit::handle_blocked_attack_town ---------//
// handle the case that the way of this unit to the target town is blocked by
// another unit
//
// <Unit*> unitPtr	- the blocking unit
//
void Unit::handle_blocked_attack_town(Unit *unitPtr)
{
	if(action_x_loc==unitPtr->action_x_loc && action_y_loc==unitPtr->action_y_loc && action_para==unitPtr->action_para &&
		action_mode==unitPtr->action_mode)
	{
		//---------------- both attacks the same town ----------------------//
		Location *locPtr = world.get_loc(action_x_loc, action_y_loc);
		if(!locPtr->is_town())
			stop2(KEEP_DEFENSE_MODE);	// stop since town is deleted
		else if(space_for_attack(action_x_loc, action_y_loc, UNIT_LAND, STD_TOWN_LOC_WIDTH, STD_TOWN_LOC_HEIGHT))
		{
			//------------ found surrounding place to attack the town -------------//
			Town *townPtr = town_array[action_para];
			{
				if(mobile_type==UNIT_LAND)
					set_move_to_surround(townPtr->loc_x1, townPtr->loc_y1, STD_TOWN_LOC_WIDTH, STD_TOWN_LOC_HEIGHT, BUILDING_TYPE_TOWN_MOVE_TO);
				else
					attack_town(townPtr->loc_x1, townPtr->loc_y1);
			}
		}
		else // no surrounding place found, stop now
			stop(KEEP_PRESERVE_ACTION);
	}
	else
		stop();
}
//----------- End of function Unit::handle_blocked_attack_town -----------//


//--------- Begin of function Unit::handle_blocked_attack_wall ---------//
// handle the case that the way of this unit to the target wall is blocked by
// another unit
//
// <Unit*> unitPtr	- the blocking unit
//
void Unit::handle_blocked_attack_wall(Unit *unitPtr)
{
	if(action_x_loc==unitPtr->action_x_loc && action_y_loc==unitPtr->action_y_loc && action_mode==unitPtr->action_mode)
	{
		//------------- both attacks the same wall ------------//
		Location *locPtr = world.get_loc(action_x_loc, action_y_loc);
		if(!locPtr->is_wall())
			stop2(KEEP_DEFENSE_MODE);	// stop since wall is deleted
		else if(space_for_attack(action_x_loc, action_y_loc, UNIT_LAND, 1, 1))
		{
			//------------ found surrounding place to attack the wall -------------//
			if(mobile_type==UNIT_LAND)
				set_move_to_surround(action_x_loc, action_y_loc, 1, 1, BUILDING_TYPE_WALL); // search for a unit only, not for a group
			else
				attack_wall(action_x_loc, action_y_loc);
		}
		else // no surrounding place found, stop now
			stop(KEEP_PRESERVE_ACTION); // no space available, so stop to wait for space to attack the wall
	}
	else
	{
		if(action_x_loc==-1 || action_y_loc==-1)
			stop();
		else
			set_wait();
	}
}
//----------- End of function Unit::handle_blocked_attack_wall -----------//
