//Filename    : OUNITS.CPP
//Description : Object Unit(Ship) functions
//Owner       : Alex

#include <OU_MARI.H>
#include <OTERRAIN.H>
#include <ONATION.H>

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

//--------- Begin of function Unit::assign_to_ship ---------//
// assigns units to ship
//
// <int> destX, destY		- the location for the ship to load the unit
// <short> shipRecno			- the recno of the ship
// <int> miscNo				- used to cal the offset location (default: 0)
//
void Unit::assign_to_ship(int destX, int destY, short shipRecno, int miscNo)
{
	err_when(destX<0 || destX>=MAX_WORLD_X_LOC || destY<0 || destY>=MAX_WORLD_Y_LOC || shipRecno<=0);
	err_when(sprite_info->loc_width>1 || sprite_info->loc_height>1);

	//----------------------------------------------------------------//
	// return if the unit is dead
	//----------------------------------------------------------------//
	if(hit_points<=0 || action_mode==ACTION_DIE || cur_action==SPRITE_DIE)
		return;

	if(unit_array.is_deleted(shipRecno))
		return;
	
	#ifdef DEBUG
		int debugShipXLoc = unit_array[shipRecno]->next_x_loc();
		int debugShipYLoc = unit_array[shipRecno]->next_y_loc();
		err_when(terrain_res[world.get_loc(debugShipXLoc, debugShipYLoc)->terrain_id]->average_type!=TERRAIN_OCEAN);
		int debugCheckXLoc, debugCheckYLoc;
	#endif
		
	//----------------------------------------------------------------//
	// action_mode2: checking for equal action or idle action
	//----------------------------------------------------------------//
	if(action_mode2==ACTION_ASSIGN_TO_SHIP && action_para2==shipRecno && action_x_loc2==destX && action_y_loc2==destY)
	{
		if(cur_action!=SPRITE_IDLE)
			return;
	}
	else
	{
		//----------------------------------------------------------------//
		// action_mode2: store new order
		//----------------------------------------------------------------//
		action_mode2  = ACTION_ASSIGN_TO_SHIP;
		action_para2  = shipRecno;
		action_x_loc2 = destX;
		action_y_loc2 = destY;
	}

	//----- order the sprite to stop as soon as possible -----//
	stop();	// new order

	Unit *shipPtr = unit_array[shipRecno];
	int shipXLoc = shipPtr->next_x_loc();
	int shipYLoc = shipPtr->next_y_loc();
	int resultXLoc, resultYLoc;
	int xShift, yShift;
	if(!miscNo)
	{
		//-------- find a suitable location since no offset location is given ---------//
		if(abs(shipXLoc-action_x_loc2)<=1 && abs(shipYLoc-action_y_loc2)<=1)
		{
			int checkXLoc, checkYLoc;
			Location *locPtr = world.get_loc(next_x_loc(), next_y_loc());
			UCHAR regionId = locPtr->region_id;
			for(int i=2; i<=9; i++)
			{
				m.cal_move_around_a_point(i, 3, 3, xShift, yShift);
				checkXLoc = shipXLoc+xShift;
				checkYLoc = shipYLoc+yShift;
				if(checkXLoc<0 || checkXLoc>=MAX_WORLD_X_LOC || checkYLoc<0 || checkYLoc>=MAX_WORLD_Y_LOC)
					continue;

				locPtr = world.get_loc(checkXLoc, checkYLoc);
				if(locPtr->region_id!=regionId)
					continue;

				resultXLoc = checkXLoc;
				resultYLoc = checkYLoc;

				#ifdef DEBUG
					debugCheckXLoc = checkXLoc;
					debugCheckYLoc = checkYLoc;
				#endif
				break;
			}
		}
		else
		{
			resultXLoc = action_x_loc2;
			resultYLoc = action_y_loc2;
		}

		err_when(resultXLoc<0 || resultXLoc>=MAX_WORLD_X_LOC || resultYLoc<0 || resultYLoc>=MAX_WORLD_Y_LOC);
	}
	else
	{
		//------------ offset location is given, move there directly ----------//
		m.cal_move_around_a_point(miscNo, MAX_WORLD_X_LOC, MAX_WORLD_Y_LOC, xShift, yShift);
		resultXLoc = destX+xShift;
		resultYLoc = destY+yShift;
		err_when(resultXLoc<0 || resultXLoc>=MAX_WORLD_X_LOC || resultYLoc<0 || resultYLoc>=MAX_WORLD_Y_LOC);
	}

	//--------- start searching ----------//
	int curXLoc = next_x_loc();
	int curYLoc = next_y_loc();
	if((curXLoc!=destX || curYLoc!=destY) && (abs(shipXLoc-curXLoc)>1 || abs(shipYLoc-curYLoc)>1))
		search(resultXLoc, resultYLoc, 1);

	//-------- set action parameters ----------//
	action_mode = ACTION_ASSIGN_TO_SHIP;
	action_para = shipRecno;
	action_x_loc = destX;
	action_y_loc = destY;
}
//----------- End of function Unit::assign_to_ship -----------//


//--------- Begin of function Unit::process_assign_to_ship ---------//
// process unit action of assigning units to ship
//
void Unit::process_assign_to_ship()
{
	err_when(sprite_info->loc_width>1 || sprite_info->loc_height>1);
	
	//---------------------------------------------------------------------------//
	// clear unit's action if situation is changed
	//---------------------------------------------------------------------------//
	UnitMarine *shipPtr;
	if(unit_array.is_deleted(action_para2))
	{
		stop2();
		return; // stop the unit as the ship is deleted
	}
	else
	{
		shipPtr = (UnitMarine*) unit_array[action_para2];
		if(shipPtr->nation_recno != nation_recno)
		{
			stop2();
			return; // stop the unit as the ship's nation_recno != the unit's nation_recno
		}
	}

	if(shipPtr->action_mode2!=ACTION_SHIP_TO_BEACH)
	{
		stop2(); // the ship has changed its action
		return;
	}

	int curXLoc = next_x_loc();
	int curYLoc = next_y_loc();
	int shipXLoc = shipPtr->next_x_loc();
	int shipYLoc = shipPtr->next_y_loc();

	if(shipPtr->cur_x==shipPtr->next_x && shipPtr->cur_y==shipPtr->next_y &&
		abs(shipXLoc-curXLoc)<=1 && abs(shipYLoc-curYLoc)<=1)
	{
		//----------- assign the unit now -----------//
		if(abs(cur_x-next_x)<sprite_info->speed && abs(cur_y-next_y)<sprite_info->speed)
		{
			if(ai_action_id)
				nation_array[nation_recno]->action_finished(ai_action_id, sprite_recno);

			stop2();
			set_dir(curXLoc, curYLoc, shipXLoc, shipYLoc);
			shipPtr->load_unit(sprite_recno);
			return;
		}
	}
	else if(cur_action==SPRITE_IDLE)
		set_dir(curXLoc, curYLoc, shipPtr->move_to_x_loc, shipPtr->move_to_y_loc);

	//---------------------------------------------------------------------------//
	// update location to embark
	//---------------------------------------------------------------------------//
	int shipActionXLoc = shipPtr->action_x_loc2;
	int shipActionYLoc = shipPtr->action_y_loc2;
	if(abs(shipActionXLoc-action_x_loc2)>1 || abs(shipActionYLoc-action_y_loc2)>1)
	{
		if(shipActionXLoc!=action_x_loc2 || shipActionYLoc!=action_y_loc2)
		{
			Location *unitLocPtr = world.get_loc(curXLoc, curYLoc);
			Location *shipActionLocPtr = world.get_loc(shipActionXLoc, shipActionYLoc);
			if(unitLocPtr->region_id != shipActionLocPtr->region_id)
			{
				stop2();
				return;
			}

			assign_to_ship(shipActionXLoc, shipActionYLoc, action_para2);
			return;
		}
	}
}
//----------- End of function Unit::process_assign_to_ship -----------//


//--------- Begin of function Unit::ship_to_beach ---------//
//	<int>	destX, destY				-	the location in land planned for LAND_UNIT to embark
// <int&> finalDestX, finalDestY -	reference to final location used to embark
//
// This function is only for ship
// move the ship to the coast
//
//
void Unit::ship_to_beach(int destX, int destY, int& finalDestX, int& finalDestY)
{
	err_when(unit_res[unit_id]->unit_class!=UNIT_CLASS_SHIP);
	err_when(destX<0 || destX>=MAX_WORLD_X_LOC || destY<0 || destY>=MAX_WORLD_Y_LOC);
	err_when(terrain_res[world.get_loc(destX, destY)->terrain_id]->average_type == TERRAIN_OCEAN);

	//----------------------------------------------------------------//
	// return if the unit is dead
	//----------------------------------------------------------------//
	if(hit_points<=0 || action_mode==ACTION_DIE || cur_action==SPRITE_DIE)
		return;

	//----------------------------------------------------------------//
	// change to move_to if the ship cannot carry units
	//----------------------------------------------------------------//
	if(unit_res[unit_id]->carry_unit_capacity<=0)
	{
		move_to(destX, destY, 1);
		finalDestX = finalDestY = -1;
		return;
	}

	//----------------------------------------------------------------//
	// calculate new destination
	//----------------------------------------------------------------//
	#ifdef DEBUG
		UnitMarine *debugShipPtr = (UnitMarine*) this; //****** for testing
	#endif

	int curXLoc = next_x_loc();
	int curYLoc = next_y_loc();
	int resultXLoc, resultYLoc;
	
	stop();
	err_when(cur_action==SPRITE_MOVE && cur_x==move_to_x_loc*ZOOM_LOC_WIDTH && cur_y==move_to_y_loc*ZOOM_LOC_HEIGHT);

	if(abs(destX-curXLoc)>1 || abs(destY-curYLoc)>1)
	{
		//-----------------------------------------------------------------------------//
		// get a suitable location in the territory as a reference location
		//-----------------------------------------------------------------------------//
		Location *locPtr = world.get_loc(destX, destY);
		UCHAR regionId = locPtr->region_id;
		int xStep = curXLoc-destX;
		int yStep = curYLoc-destY;
		int absXStep = abs(xStep);
		int absYStep = abs(yStep);
		int count = (absXStep>=absYStep) ? absXStep : absYStep;
		int x, y;
		long int sameTerr = 0;

		for(long int i=1; i<=count; i++)
		{
			x = destX + int((i*xStep)/count);
			y = destY + int((i*yStep)/count);

			locPtr = world.get_loc(x, y);
			if(locPtr->region_id==regionId)
			{
				if(locPtr->walkable())
					sameTerr = i;
			}
		}

		if(sameTerr)
		{
			resultXLoc = destX + int((sameTerr*xStep)/count);
			resultYLoc = destY + int((sameTerr*yStep)/count);
		}
		else
		{
			resultXLoc = destX;
			resultYLoc = destY;
		}
		
		//------------------------------------------------------------------------------//
		// find the path from the ship location in the ocean to the reference location
		// in the territory
		//------------------------------------------------------------------------------//
		if(!ship_to_beach_path_edit(resultXLoc, resultYLoc, regionId))
		{
			finalDestX = finalDestY = -1;
			return; // calling move_to() instead
		}
	}
	else
	{
		resultXLoc = destX;
		resultYLoc = destY;
	}

	err_when(cur_action==SPRITE_MOVE && cur_x==next_x && cur_y==next_y &&
				cur_x==move_to_x_loc*ZOOM_LOC_WIDTH && cur_y==move_to_y_loc*ZOOM_LOC_HEIGHT);
	err_when(result_node_array==NULL && result_path_dist);

	action_mode = action_mode2 = ACTION_SHIP_TO_BEACH;
	action_para = action_para2 = 0;
	finalDestX = action_x_loc = action_x_loc2 = resultXLoc;
	finalDestY = action_y_loc = action_y_loc2 = resultYLoc;
}
//----------- End of function Unit::ship_to_beach -----------//


//--------- Begin of function Unit::ship_to_beach_path_edit ---------//
// find a path to the beach
//
// <int&>	resultXLoc	-	reference to return final x location the ship move to
// <int&>	resultYLoc	-	reference to return final y location the ship move to
// <UCHAR>	regionId		-	region id of the destination location
//
// return 1 if normal execution
// return 0 if calling move_to() instead
//
int Unit::ship_to_beach_path_edit(int& resultXLoc, int& resultYLoc, UCHAR regionId)
{
	int curXLoc = next_x_loc();
	int curYLoc = next_y_loc();
	if(abs(curXLoc-resultXLoc)<=1 && abs(curYLoc-resultYLoc)<=1)
		return 1;

	//--------------- find a path to land area -------------------//
	UnitMarine *shipPtr = (UnitMarine*) this;
	int result = search(resultXLoc, resultYLoc, 1, SEARCH_MODE_TO_LAND_FOR_SHIP, regionId);
	if(!result)
		return 1;

	//----------- update cur location --------//
	curXLoc = next_x_loc();
	curYLoc = next_y_loc();
	err_when(shipPtr->extra_move_in_beach!=NO_EXTRA_MOVE);
		
	//------------------------------------------------------------------------------//
	// edit the result path to get a location for embarking
	//------------------------------------------------------------------------------//
	err_when(result_node_array==NULL && result_node_count);

	if(result_node_array && result_node_count)
	{
		err_when(result_node_count<2);
		ResultNode *curNodePtr = result_node_array;
		ResultNode *nextNodePtr = curNodePtr + 1;
		int moveScale = move_step_magn();
		int nodeCount = result_node_count;
		Location *locPtr;
		int i, j, found, pathDist;
		
		int preXLoc, preYLoc;
		int checkXLoc = curXLoc;
		int checkYLoc = curYLoc;
		int hasMoveStep = 0;
		if(checkXLoc!=curNodePtr->node_x || checkYLoc!=curNodePtr->node_y)
		{
			err_when(abs(checkXLoc-curNodePtr->node_x)>moveScale || abs(checkYLoc-curNodePtr->node_y)>moveScale);
			hasMoveStep += moveScale;
			checkXLoc = curNodePtr->node_x;
			checkYLoc = curNodePtr->node_y;
		}
		
		//-----------------------------------------------------------------//
		// find the pair of points that one is in ocean and one in land
		//-----------------------------------------------------------------//
		err_when(terrain_res[world.get_loc(curNodePtr->node_x, curNodePtr->node_y)->terrain_id]->average_type!=TERRAIN_OCEAN);
		int vecX, vecY, xMagn, yMagn, magn;

		#ifdef DEBUG
			int debugLoop1=0, debugLoop2=0;
		#endif
		for(pathDist=0, found=0, i=1; i<nodeCount; ++i, curNodePtr++, nextNodePtr++)
		{
			#ifdef DEBUG
			err_when(++debugLoop1>10000);
			#endif
			
			vecX = nextNodePtr->node_x - curNodePtr->node_x;
			vecY = nextNodePtr->node_y - curNodePtr->node_y;
			magn = ((xMagn=abs(vecX)) > (yMagn=abs(vecY))) ? xMagn : yMagn;
			if(xMagn) {	vecX /= xMagn;	vecX *= moveScale; }
			if(yMagn) {	vecY /= yMagn;	vecY *= moveScale; }
			err_when(abs(vecX)>moveScale && abs(vecY)>moveScale);

			//------------- check each location bewteen editNode1 and editNode2 -------------//
			for(j=0; j<magn; j+=moveScale)
			{
				#ifdef DEBUG
				err_when(++debugLoop2>10000);
				#endif

				preXLoc = checkXLoc;
				preYLoc = checkYLoc;
				checkXLoc += vecX;
				checkYLoc += vecY;

				locPtr =  world.get_loc(checkXLoc, checkYLoc);
				if(terrain_res[locPtr->terrain_id]->average_type!=TERRAIN_OCEAN) // found
				{
					found++;
					break;
				}
			}

			if(found)
			{
				//------------ a soln is found ---------------//
				if(!j) // end node should be curNodePtr pointed at
				{
					pathDist -= hasMoveStep;
					result_node_count = i;
					result_path_dist = pathDist;
				}
				else
				{
					nextNodePtr->node_x = checkXLoc;
					nextNodePtr->node_y = checkYLoc;

					if(i==1) // first editing
					{
						ResultNode *firstNodePtr = result_node_array;
						if(cur_x==firstNodePtr->node_x*ZOOM_LOC_WIDTH && cur_y==firstNodePtr->node_y*ZOOM_LOC_HEIGHT)
						{
							go_x = checkXLoc * ZOOM_LOC_WIDTH;
							go_y = checkYLoc * ZOOM_LOC_HEIGHT;
						}
					}

					pathDist += (j+moveScale);
					pathDist -= hasMoveStep;
					result_node_count = i+1;
					result_path_dist = pathDist;
				}

				move_to_x_loc = preXLoc;
				move_to_y_loc = preYLoc;
				locPtr = world.get_loc((preXLoc+checkXLoc)/2, (preYLoc+checkYLoc)/2);
				if(terrain_res[locPtr->terrain_id]->average_type!=TERRAIN_OCEAN)
				{
					resultXLoc = (preXLoc+checkXLoc)/2;
					resultYLoc = (preYLoc+checkYLoc)/2;
				}
				else
				{
					resultXLoc = checkXLoc;
					resultYLoc = checkYLoc;
				}
				break;
			}
			else
				pathDist += magn;
		}

		if(!found)
		{
			ResultNode *endNodePtr = result_node_array + result_node_count - 1;
			if(abs(endNodePtr->node_x-resultXLoc)>1 || abs(endNodePtr->node_y-resultYLoc)>1)
			{
				move_to(resultXLoc, resultYLoc, -1);
				return 0;
			}
		}
	}
	else
	{
		//------------- scan for the surrounding for a land location -----------//
		int xShift, yShift, checkXLoc, checkYLoc;
		Location *locPtr;
		for(int i=2; i<=9; ++i)
		{
			m.cal_move_around_a_point(i, 3, 3, xShift, yShift);
			checkXLoc = curXLoc + xShift;
			checkYLoc = curYLoc + yShift;
			if(checkXLoc<0 || checkXLoc>=MAX_WORLD_X_LOC || checkYLoc<0 || checkYLoc>=MAX_WORLD_Y_LOC)
				continue;

			locPtr = world.get_loc(checkXLoc, checkYLoc);
			if(locPtr->region_id!=regionId)
				continue;

			if(terrain_res[locPtr->terrain_id]->average_type!=TERRAIN_OCEAN && locPtr->can_move(UNIT_LAND))
			{
				resultXLoc = checkXLoc;
				resultYLoc = checkYLoc;
				return 1;
			}
		}

		return 0;
	}
	
	return 1;
}
//----------- End of function Unit::ship_to_beach_path_edit -----------//


//--------- Begin of function Unit::process_ship_to_beach ---------//
// process unit action SHIP_TO_BEACH
//
void Unit::process_ship_to_beach()
{
	//----- action_mode never clear, in_beach to skip idle checking
	if(cur_action==SPRITE_IDLE)
	{
		int shipXLoc = next_x_loc();
		int shipYLoc = next_y_loc();
		if(shipXLoc==move_to_x_loc && shipYLoc==move_to_y_loc)
		{
			if(abs(move_to_x_loc-action_x_loc2)<=2 && abs(move_to_y_loc-action_y_loc2)<=2)
			{
				UnitMarine *shipPtr = (UnitMarine*) this;
				//------------------------------------------------------------------------------//
				// determine whether extra_move is required
				//------------------------------------------------------------------------------//
				switch(shipPtr->extra_move_in_beach)
				{
					case NO_EXTRA_MOVE:
							if(abs(shipXLoc-action_x_loc2)>1 || abs(shipYLoc-action_y_loc2)>1)
							{
								err_when(abs(shipXLoc-action_x_loc2)>2 || abs(shipYLoc-action_y_loc2)>2);
								err_when(result_node_array);

								shipPtr->extra_move();
							}
							else
							{
								//shipPtr->in_beach = 1;
								shipPtr->extra_move_in_beach = NO_EXTRA_MOVE;
								
							//#### trevor 23/10 #####//
								if(ai_action_id)
									nation_array[nation_recno]->action_finished(ai_action_id, sprite_recno);
							//#### trevor 23/10 #####//
							}
							break;

					case EXTRA_MOVING_IN:
					case EXTRA_MOVING_OUT:
							//err_when(cur_action!=SPRITE_SHIP_EXTRA_MOVE);
							break;

							//#### trevor 23/10 #####//

					case EXTRA_MOVE_FINISH:
							if(ai_action_id)
								nation_array[nation_recno]->action_finished(ai_action_id, sprite_recno);
							break;

							//#### trevor 23/10 #####//

					default: err_here();
								break;
				}
			}
		}			
		else
			reset_action_para();
	}
	else if(cur_action==SPRITE_TURN && is_dir_correct())
		set_move();
}
//----------- End of function Unit::process_ship_to_beach -----------//


//--------- Begin of function Unit::ship_leave_beach ---------//
void Unit::ship_leave_beach(int shipOldXLoc, int shipOldYLoc)
{
	err_when(cur_x!=next_x || cur_y!=next_y);

	UnitMarine *shipPtr = (UnitMarine*) this;
	err_when(!shipPtr->in_beach && shipPtr->extra_move_in_beach==EXTRA_MOVE_FINISH);

	//--------------------------------------------------------------------------------//
	// scan for location to leave the beach
	//--------------------------------------------------------------------------------//
	int curXLoc = next_x_loc();
	int curYLoc = next_y_loc();
	int xShift, yShift, checkXLoc, checkYLoc, found=0;
	Location *locPtr;

	//------------- find a location to leave the beach ------------//
	for(int i=2; i<=9; i++)
	{
		m.cal_move_around_a_point(i, 3, 3, xShift, yShift);
		checkXLoc = curXLoc + xShift;
		checkYLoc = curYLoc + yShift;

		if(checkXLoc<0 || checkXLoc>=MAX_WORLD_X_LOC || checkYLoc<0 || checkYLoc>=MAX_WORLD_Y_LOC)
			continue;
		
		if(checkXLoc%2 || checkYLoc%2)
			continue;

		locPtr = world.get_loc(checkXLoc, checkYLoc);
		if(terrain_res[locPtr->terrain_id]->average_type==TERRAIN_OCEAN &&
			locPtr->can_move(mobile_type))
		{
			found++;
			break;
		}
	}

	if(!found)
		return; // no suitable location, wait until finding suitable location

	//---------------- leave now --------------------//
	set_dir(shipOldXLoc, shipOldYLoc, checkXLoc, checkYLoc);
	set_ship_extra_move();
	go_x = checkXLoc*ZOOM_LOC_WIDTH;
	go_y = checkYLoc*ZOOM_LOC_HEIGHT;
	err_when(cur_x==go_x && cur_y==go_y);
}
//----------- End of function Unit::ship_leave_beach -----------//