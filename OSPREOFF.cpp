#include <OSPREUSE.h>
#include <OUNIT.h>
#include <OWORLD.h>
#include <OMISC.h>

#ifdef NO_DEBUG_SEARCH
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
#undef debug_reuse_check_path
#define debug_reuse_check_path()
#undef DEBUG
#endif

#ifdef DEBUG
#include <OSYS.h>
#endif

//------------------- static function --------------------//
// check whether a location can be walked. This function is 
// similar to that in SeekPath.  However, only several search
// mode is useful here. i.e. search mode 1 and 2.
//
int SeekPathReuse::can_walk(int xLoc, int yLoc)
{
	if(xLoc>=MAX_WORLD_X_LOC || yLoc>=MAX_WORLD_Y_LOC)
		return 0;

	Location *locPtr = world.get_loc(xLoc, yLoc);
	short	recno = (mobile_type!=UNIT_AIR) ? locPtr->cargo_recno : locPtr->air_cargo_recno;
	Unit *unitPtr;
	UCHAR	unitCurAction;

	//------ check terrain id. -------//

	switch(mobile_type)
	{
		case UNIT_LAND:
			if(reuse_search_sub_mode==SEARCH_SUB_MODE_PASSABLE && locPtr->power_nation_recno &&
				!reuse_nation_passable[locPtr->power_nation_recno])
				return 0;

			if(!locPtr->walkable())
				return 0;
			
			if(!recno)
				return 1;

			unitPtr = unit_array[recno];
			if(search_mode==SEARCH_MODE_A_UNIT_IN_GROUP)
				return unitPtr->cur_action==SPRITE_MOVE;
			else
			{
				unitCurAction = unitPtr->cur_action;
				return (unitPtr->unit_group_id==cur_group_id && unitCurAction!=SPRITE_ATTACK) ||
						 (unitCurAction==SPRITE_MOVE && unitPtr->cur_x-unitPtr->next_x<=ZOOM_LOC_WIDTH/2 &&
						  unitPtr->cur_y-unitPtr->next_y<=ZOOM_LOC_HEIGHT/2) ||
						 (unitPtr->nation_recno==unit_nation_recno && unitCurAction==SPRITE_IDLE);
			}

			break;

		case UNIT_SEA:
			if(!locPtr->sailable())
				return 0;

			if(!recno)
				return 1;

			unitPtr = unit_array[recno];
			if(search_mode==SEARCH_MODE_A_UNIT_IN_GROUP)
				return unitPtr->cur_action==SPRITE_MOVE;
			else
			{
				unitCurAction = unitPtr->cur_action;
				return (unitPtr->unit_group_id==cur_group_id && unitCurAction!=SPRITE_ATTACK) ||
						 unitCurAction==SPRITE_MOVE ||
						 (unitPtr->nation_recno==unit_nation_recno && unitCurAction==SPRITE_IDLE);
			}

			break;

		case UNIT_AIR:
			if(!recno)
				return 1;

			unitPtr = unit_array[recno];
			if(search_mode==SEARCH_MODE_A_UNIT_IN_GROUP)
				return unitPtr->cur_action==SPRITE_MOVE;
			else
			{
				unitCurAction = unitPtr->cur_action;
				return (unitPtr->unit_group_id==cur_group_id && unitCurAction!=SPRITE_ATTACK) ||
						 unitCurAction==SPRITE_MOVE ||
						 (unitPtr->nation_recno==unit_nation_recno && unitCurAction==SPRITE_IDLE);
			}
			break;
	}
	return 0;
}
//------------ End of static function ----------------//


//------------------- static function --------------------//
int SeekPathReuse::can_walk_s2(int xLoc, int yLoc)
{
	if(xLoc>=MAX_WORLD_X_LOC-1 || yLoc>=MAX_WORLD_Y_LOC-1)
		return 0;

	if(can_walk(xLoc, yLoc) && can_walk(xLoc+1,yLoc) && can_walk(xLoc,yLoc+1) && can_walk(xLoc+1,yLoc+1))
		return 1;
	else
		return 0;
}
//------------ End of static function ----------------//


//------------------- static function --------------------//
void SeekPathReuse::sys_yield()
{
	//sys.yield();
}
//------------ End of static function ----------------//


//-------- Begin of function SeekPathReuse::seek_path_offset ---------//
void SeekPathReuse::seek_path_offset()
{
	if(!is_leader_path_valid())
		return;

	if(is_node_avail_empty())
	{
		copy_leader_path_offset();
		return;
	}

	//------------ construct data structure to store result node ----------------//
	result_node_array_def_size += result_node_array_reset_amount;
	path_reuse_result_node_ptr = (ResultNode*) mem_add(sizeof(ResultNode)* result_node_array_def_size);
	memset(path_reuse_result_node_ptr, 0, sizeof(ResultNode)* result_node_array_def_size);
	cur_result_node_ptr = path_reuse_result_node_ptr;
	num_of_result_node = 0;
	
	//---------------- set starting point ----------------------//
	add_result(start_x, start_y);
	
	//-----------------initialize offset path reuse ------------//
	cur_leader_node_ptr = reuse_leader_path_backup+1;
	cur_leader_node_num = 2;
	use_offset_method(reuse_leader_path_backup[0].node_x, reuse_leader_path_backup[0].node_y);	// using offset path method directly

	//----------------------------------------------------------------------//
	// checking for incomplete searching
	//----------------------------------------------------------------------//
	ResultNode *lastNode = path_reuse_result_node_ptr + num_of_result_node - 1;
	if((lastNode->node_x!=vir_dest_x || lastNode->node_y!=vir_dest_y) && is_node_avail_empty())
	{
		incomplete_search++;
		reuse_path_status = REUSE_PATH_INCOMPLETE_SEARCH;
	}
}
//--------- End of function SeekPathReuse::seek_path_offset ---------//


//-------- Begin of function SeekPathReuse::seek_path_join_offset ---------//
// The join-offset-path method.
//
void SeekPathReuse::seek_path_join_offset()
{
	if(!is_leader_path_valid())
		return;

	//----------------------------------------------------------------------//
	// checking for incomplete searching
	//----------------------------------------------------------------------//
	if(is_node_avail_empty())
	{
		incomplete_search++;
		reuse_path_status = REUSE_PATH_INCOMPLETE_SEARCH;
		return;
	}

	err_when(unit_size!=1);
	memset(reuse_node_matrix, 0, sizeof(short)*MAX_WORLD_X_LOC*MAX_WORLD_Y_LOC/4);
	path_reuse_result_node_ptr = NULL;

	//--------------------------------------------------------------//
	// initialization and declaring variables
	//--------------------------------------------------------------//
	int connectResultNodeNum;
	ResultNode* connectResultNodePtr=NULL;

	cur_leader_node_ptr = reuse_leader_path_backup;
	cur_leader_node_num = 1;

	int leaderNodeXLoc = cur_leader_node_ptr->node_x;
	int leaderNodeYLoc = cur_leader_node_ptr->node_y;

	#ifdef DEBUG
		int debugLoopCount = 0;
	#endif
	
	do
	{
		err_when(++debugLoopCount>10000);
		set_index_in_node_matrix(leaderNodeXLoc+x_offset, leaderNodeYLoc+y_offset);
	}while(get_next_offset_loc(leaderNodeXLoc, leaderNodeYLoc));
	
	//--------------------------------------------------------------//
	// copy the node_matrix to that node_matrix used in class SeekPath
	//--------------------------------------------------------------//
	err_when(unit_size!=1);
	seek_path.set_node_matrix(reuse_node_matrix);
	
	//--------------------------------------------------------------//
	// process shortest path searching to find a walkable point in 
	// the offset path for connection
	//--------------------------------------------------------------//
	//--- if the starting location is already in the offset path, process it immediately ---//
	err_when(unit_size!=1);
	short *locNode = reuse_node_matrix + MAX_WORLD_X_LOC/2*(start_y/2) + (start_x/2);

	if(*locNode > max_node && mobile_type==UNIT_LAND)	// starting point on the reuse offset path
	{
		connectResultNodePtr = (ResultNode*) mem_add(sizeof(ResultNode)*2);
		ResultNode* curNode = connectResultNodePtr;
		curNode->node_x = start_x;
		curNode->node_y = start_y;
		connectResultNodeNum = 1;
		curNode++;

		switch(*locNode-max_node)
		{
			case 1:	if(start_x%2 || start_y%2)
						{
							curNode->node_x = (start_x%2) ? start_x-1 : start_x;
							curNode->node_y = (start_y%2) ? start_y-1 : start_y;
							connectResultNodeNum++;
						}
						break;

			case 2:	if(!(start_x%2 && start_y%2==0))
						{
							curNode->node_x = (start_x%2) ? start_x : start_x+1;
							curNode->node_y = (start_y%2) ? start_y-1 : start_y;
							connectResultNodeNum++;
						}
						break;

			case 3:	if(!(start_x%2==0 && start_y%2))
						{
							curNode->node_x = (start_x%2) ? start_x-1 : start_x;
							curNode->node_y = (start_y%2) ? start_y : start_y+1;
							connectResultNodeNum++;
						}
						break;

			case 4:	if(start_x%2==0 || start_y%2==0)
						{
							curNode->node_x = (start_x%2) ? start_x : start_x+1;
							curNode->node_y = (start_y%2) ? start_y : start_y+1;
							connectResultNodeNum++;
						}
						break;
		}

		//********BUGHERE
		// unable to handle this blocked case now, abort path-reuse and seeking instead
		if(connectResultNodeNum>1 && !can_walk(curNode->node_x, curNode->node_y))	
		{
			path_reuse_result_node_ptr = call_seek(start_x, start_y, vir_dest_x, vir_dest_y, cur_group_id, mobile_type, SEARCH_MODE_IN_A_GROUP, num_of_result_node);
			mem_del(connectResultNodePtr);
			return;
		}
	}
	else
	{
		//------------- seek for connection point ------------//
		connectResultNodePtr = call_seek(start_x, start_y, vir_dest_x, vir_dest_y, cur_group_id, mobile_type,
													SEARCH_MODE_REUSE, connectResultNodeNum);
		err_when(connectResultNodePtr!=NULL && connectResultNodeNum<2);

		if(connectResultNodePtr==NULL || connectResultNodeNum==0)	// cannot reach the destination
		{
			if(connectResultNodePtr!=NULL)
				mem_del(connectResultNodePtr);
	
			return;
		}
	}

	//--------------------------------------------------------------//
	// constructing data structure
	//--------------------------------------------------------------//
	result_node_array_def_size += result_node_array_reset_amount;
	path_reuse_result_node_ptr = (ResultNode*) mem_add(sizeof(ResultNode)* result_node_array_def_size);
	memset(path_reuse_result_node_ptr, 0, sizeof(ResultNode)* result_node_array_def_size);
	cur_result_node_ptr = path_reuse_result_node_ptr;
	num_of_result_node = 0;

	//--------------------------------------------------------------//
	// add the result path
	//--------------------------------------------------------------//
	ResultNode* curResultNode = connectResultNodePtr;
	for(int i=0; i<connectResultNodeNum; i++)
	{
		add_result(curResultNode->node_x, curResultNode->node_y);
		curResultNode++;
	}

	//--------------------------------------------------------------//
	// determine the joining point in the offset path
	//--------------------------------------------------------------//
	cur_leader_node_ptr = reuse_leader_path_backup;
	cur_leader_node_num = 1;

	ResultNode *endNode = connectResultNodePtr + connectResultNodeNum - 1;
	short findConnectionPoint = 0;

	if(endNode->node_x==vir_dest_x && endNode->node_y==vir_dest_y)
	{
		if(connectResultNodePtr!=NULL);
			mem_del(connectResultNodePtr);
		return; // already the destination location
	}

	//------------------------------------------------------------------------------------//
	// find a offset-reference point in leader path
	//------------------------------------------------------------------------------------//
	leaderNodeXLoc = cur_leader_node_ptr->node_x;
	leaderNodeYLoc = cur_leader_node_ptr->node_y;
	short notEnd = 1;
	sys_yield(); // update cursor position

	int unitDestX, unitDestY; // for the current searching unit, using the leader path
	do
	{
		//---------- boundary checking -----------//
		bound_check_x((unitDestX = leaderNodeXLoc+x_offset));
		bound_check_y((unitDestY = leaderNodeYLoc+y_offset));

		if(endNode->node_x==unitDestX && endNode->node_y==unitDestY)
			findConnectionPoint = 1;	// ok, connected
		else
			notEnd = get_next_offset_loc(leaderNodeXLoc, leaderNodeYLoc);

		if(!notEnd)
			break;
	}while(!findConnectionPoint);

	sys_yield(); // update cursor position

	//-------------- clear the temporary path ------------//
	if(connectResultNodePtr!=NULL)
		mem_del(connectResultNodePtr);

	//-----------------------------------------------------------------------------//
	// return since cannot find a connection point
	//-----------------------------------------------------------------------------//
	if(!findConnectionPoint)
	{
		//----------------------------------------------------------------------//
		// checking for incomplete searching
		//----------------------------------------------------------------------//
		if(is_node_avail_empty())
		{
			incomplete_search++;
			reuse_path_status = REUSE_PATH_INCOMPLETE_SEARCH;
		}

		if(num_of_result_node==1)
		{
			mem_del(path_reuse_result_node_ptr);
			path_reuse_result_node_ptr = NULL;
			num_of_result_node = 0;
		}
		return;
	}

	//-----------------------------------------------------------------------------//
	// update pointer cur_leader_node_ptr if necessary
	//-----------------------------------------------------------------------------//
	if(leaderNodeXLoc==cur_leader_node_ptr->node_x && leaderNodeYLoc==cur_leader_node_ptr->node_y)	// at the turning point
	{
		if(cur_leader_node_num < reuse_leader_path_node_num)
		{
			cur_leader_node_num++;
			cur_leader_node_ptr++;
		}
		else	// join at the end of the offset path, search finished
			return;
	}

	//----------------------------------------------------------------------------------------//
	// at this moment, the searching unit has a offset path to the leader path.
	// There are not processed leader nodes. These nodes are pointed by cur_leader_node_ptr.
	//----------------------------------------------------------------------------------------//
	use_offset_method(leaderNodeXLoc, leaderNodeYLoc);	// changed to offset method for the rest
	
	//----------------------------------------------------------------------//
	// checking for incomplete searching
	//----------------------------------------------------------------------//
	ResultNode *lastNode = path_reuse_result_node_ptr + num_of_result_node - 1;
	if((lastNode->node_x!=vir_dest_x || lastNode->node_y!=vir_dest_y) && is_node_avail_empty())
	{
		incomplete_search++;
		reuse_path_status = REUSE_PATH_INCOMPLETE_SEARCH;
	}
}
//--------- End of function SeekPathReuse::seek_path_join_offset ---------//


//-------- Begin of function SeekPathReuse::use_offset_method ---------//
void SeekPathReuse::use_offset_method(int xLoc, int yLoc)
{
	//----------------------------------------------------------------------//
	// checking for incomplete searching
	//----------------------------------------------------------------------//
	if(is_node_avail_empty())
	{
		incomplete_search++;
		reuse_path_status = REUSE_PATH_INCOMPLETE_SEARCH;
		return;
	}

	//-----------------------------------------------------//
	int leaderNodeXLoc = xLoc; // hold the currently referred leader node
	int leaderNodeYLoc = yLoc;

	leader_vec_x = cur_leader_node_ptr->node_x - leaderNodeXLoc;
	leader_vec_y = cur_leader_node_ptr->node_y - leaderNodeYLoc;
	if(leader_vec_x!=0)
		leader_vec_x /= abs(leader_vec_x);
	if(leader_vec_y!=0)
		leader_vec_y /= abs(leader_vec_y);

	ResultNode *partOfResultNodePtr, *curNodePtr;
	int partOfResultNodeNum, restNode;

	partOfResultNodePtr = NULL;
	partOfResultNodeNum = 0;

	//-----------------------------------------------------//
	int unitNodeXLoc, unitNodeYLoc;
	int preNonblockedXLoc, preNonblockedYLoc;
	int nextNonblockedLeaderXLoc, nextNonblockedLeaderYLoc;
	int preLeaderVecX, preLeaderVecY;
	int virDestX, virDestY;
	int pathSeekResult, canReach;

	//-----------------------------------------------------//
	// start walking along the leader path
	//-----------------------------------------------------//
	#ifdef DEBUG
		int debugPreLeaderNodeXLoc, debugPreLeaderNodeYLoc;
		while(debugPreLeaderNodeXLoc=leaderNodeXLoc, debugPreLeaderNodeYLoc=leaderNodeYLoc,
				get_next_offset_loc(leaderNodeXLoc, leaderNodeYLoc)) // get next location in the leader path
	#else
		while(get_next_offset_loc(leaderNodeXLoc, leaderNodeYLoc)) // get next location in the leader path
	#endif
	{
		err_when(leaderNodeXLoc<0 || leaderNodeXLoc>=MAX_WORLD_X_LOC);
		err_when(leaderNodeYLoc<0 || leaderNodeYLoc>=MAX_WORLD_Y_LOC);

		err_when(partOfResultNodePtr!=NULL);
		sys_yield(); // update cursor position

		unitNodeXLoc = leaderNodeXLoc+x_offset;	// calculate the corresponding location in the offset path.
		unitNodeYLoc = leaderNodeYLoc+y_offset;

		if(unitNodeXLoc>=0 && unitNodeXLoc<MAX_WORLD_X_LOC && unitNodeYLoc>=0 && unitNodeYLoc<MAX_WORLD_Y_LOC &&
			((move_scale==1 && can_walk(unitNodeXLoc, unitNodeYLoc)) ||
		 	 (move_scale==2 && can_walk(unitNodeXLoc, unitNodeYLoc) &&
			  can_walk(unitNodeXLoc-leader_vec_x, unitNodeYLoc-leader_vec_y)) ))
		{
			//----------------------------------------------------------------------------//
			// offset node exists, so add it to the result path
			//----------------------------------------------------------------------------//
			add_result(unitNodeXLoc, unitNodeYLoc); // add result if location can be reached
			if(unitNodeXLoc==vir_dest_x && unitNodeYLoc==vir_dest_y)
				break;
		}
		else
		{
			//----------------------------------------------------------------------------//
			// get the next non-blocked location calculated by offset
			//----------------------------------------------------------------------------//
			nextNonblockedLeaderXLoc = leaderNodeXLoc; // used as reference
			nextNonblockedLeaderYLoc = leaderNodeYLoc;
			preLeaderVecX = leader_vec_x;
			preLeaderVecY = leader_vec_y;

			//========================================================================//
			//========================================================================//
			if(get_next_nonblocked_offset_loc(nextNonblockedLeaderXLoc, nextNonblockedLeaderYLoc))	// get the next nonblocked location for joining
			{
				if(is_node_avail_empty())
				{
					incomplete_search++;
					reuse_path_status = REUSE_PATH_INCOMPLETE_SEARCH;
					return;
				}

				//--- seek from the current location to the next non-blocked location ---//
				err_when(num_of_result_node<1);
				preNonblockedXLoc = (cur_result_node_ptr-1)->node_x;
				preNonblockedYLoc = (cur_result_node_ptr-1)->node_y;
				//bound_check_x((preNonblockedXLoc = unitNodeXLoc-preLeaderVecX*move_scale));
				//bound_check_y((preNonblockedYLoc = unitNodeYLoc-preLeaderVecY*move_scale));
				err_when(preNonblockedXLoc<0 || preNonblockedXLoc>=MAX_WORLD_X_LOC);
				err_when(preNonblockedYLoc<0 || preNonblockedYLoc>=MAX_WORLD_Y_LOC);
				
				//-------- find a path to the next non-blocked location ----------//
				bound_check_x((virDestX = nextNonblockedLeaderXLoc+x_offset));
				bound_check_y((virDestY = nextNonblockedLeaderYLoc+y_offset));
				err_when(partOfResultNodePtr!=NULL);

				err_when(unit_size!=1);
				pathSeekResult = seek_path.seek(preNonblockedXLoc, preNonblockedYLoc, virDestX, virDestY,
															cur_group_id, mobile_type, SEARCH_MODE_IN_A_GROUP);
				partOfResultNodePtr = seek_path.get_result(partOfResultNodeNum, reuse_path_dist);

				//--------------------------------------------------------------------------//
				// go to destination directly if cannot reach the next nonblocked lcoation
				//--------------------------------------------------------------------------//
				if(partOfResultNodePtr==NULL || partOfResultNodeNum==0)
					canReach = 0;
				else
				{
					#ifdef DEBUG
						int ddX = abs((cur_result_node_ptr-1)->node_x-partOfResultNodePtr[0].node_x);
						int ddY = abs((cur_result_node_ptr-1)->node_y-partOfResultNodePtr[0].node_y);
						err_when(ddX && ddY && ddX!=ddY);
					#endif

					ResultNode *curNode = partOfResultNodePtr + partOfResultNodeNum-1;
					if(curNode->node_x==virDestX && curNode->node_y==virDestY)
						canReach = 1;
					else
					{
						canReach = 0;
						mem_del(partOfResultNodePtr);
						partOfResultNodePtr = NULL;
					}
				}

				if(canReach==0)	//	 unable to reach the location specified
				{
					if(is_node_avail_empty())
					{
						incomplete_search++;
						reuse_path_status = REUSE_PATH_INCOMPLETE_SEARCH;
						return;
					}

					bound_check_x((virDestX = dest_x));
					bound_check_y((virDestY = dest_y));
					err_when(partOfResultNodePtr!=NULL);

					err_when(unit_size!=1);
					int pathSeekResult= seek_path.seek(preNonblockedXLoc, preNonblockedYLoc, virDestX, virDestY, cur_group_id, mobile_type, SEARCH_MODE_IN_A_GROUP);
					partOfResultNodePtr = seek_path.get_result(partOfResultNodeNum, reuse_path_dist);

					#ifdef DEBUG
						if(partOfResultNodePtr!=NULL)
						{
							int ddX = abs((cur_result_node_ptr-1)->node_x-partOfResultNodePtr[0].node_x);
							int ddY = abs((cur_result_node_ptr-1)->node_y-partOfResultNodePtr[0].node_y);
							err_when(ddX && ddY && ddX!=ddY);
						}
					#endif
				}
				
				//---------------------- connect the two paths ----------------------//
				if(partOfResultNodePtr!=NULL)
				{
					restNode = partOfResultNodeNum;
					curNodePtr = partOfResultNodePtr;

					err_when(preNonblockedXLoc!=partOfResultNodePtr->node_x || preNonblockedYLoc!=partOfResultNodePtr->node_y);
					
					restNode--;
					curNodePtr++;
					while(restNode)
					{
						add_result(curNodePtr->node_x, curNodePtr->node_y);
						curNodePtr++;
						restNode--;
					}

					debug_reuse_check_path(); //-************** debug checking 
					
					mem_del(partOfResultNodePtr);
					partOfResultNodePtr = NULL;
				}

				err_when(partOfResultNodePtr!=NULL);

				if(canReach)
				{
					leaderNodeXLoc = nextNonblockedLeaderXLoc;
					leaderNodeYLoc = nextNonblockedLeaderYLoc;
				}
				else
				{
					//------------------------------------------------//
					// incomplete search
					//------------------------------------------------//
					if(is_node_avail_empty())
					{
						incomplete_search++;
						reuse_path_status = REUSE_PATH_INCOMPLETE_SEARCH;
						return;
					}
					break;
				}

				if(pathSeekResult!=PATH_FOUND && pathSeekResult!=PATH_REUSE_FOUND)
					break;
			}
			//========================================================================//
			//========================================================================//
			else	// no next non-blocked offset location, searching directly to the destinaton
			{
				//-------------------------------------------------------------//				
				// move directly to the destination from the current location
				// 
				// This case occurs when the destination cannot be reached.
				//-------------------------------------------------------------//
				
				//--- seek from the current location to the destination ---//
				//bound_check_x((preNonblockedXLoc = unitNodeXLoc-preLeaderVecX*move_scale));
				//bound_check_y((preNonblockedYLoc = unitNodeYLoc-preLeaderVecY*move_scale));
				err_when(num_of_result_node<1);
				preNonblockedXLoc = (cur_result_node_ptr-1)->node_x;
				preNonblockedYLoc = (cur_result_node_ptr-1)->node_y;
				err_when(preNonblockedXLoc<0 || preNonblockedXLoc>=MAX_WORLD_X_LOC);
				err_when(preNonblockedYLoc<0 || preNonblockedYLoc>=MAX_WORLD_Y_LOC);

				bound_check_x((virDestX = nextNonblockedLeaderXLoc+x_offset));
				bound_check_y((virDestY = nextNonblockedLeaderYLoc+y_offset));
				err_when(partOfResultNodePtr!=NULL);

				//------------------------------------------------//
				// set to incomplete_search for later searching
				//------------------------------------------------//
				incomplete_search++;
				reuse_path_status = REUSE_PATH_INCOMPLETE_SEARCH;

				#ifdef DEBUG
					int ddX1 = abs((cur_result_node_ptr-1)->node_x-preNonblockedXLoc);
					int ddY1 = abs((cur_result_node_ptr-1)->node_y-preNonblockedYLoc);
					err_when(ddX1 && ddY1 && ddX1!=ddY1);
				#endif

				err_when(unit_size!=1);
				seek_path.seek(preNonblockedXLoc, preNonblockedYLoc, virDestX, virDestY, cur_group_id, mobile_type, 1);
				partOfResultNodePtr = seek_path.get_result(partOfResultNodeNum, reuse_path_dist);

				//---------------- connect the two paths together ----------------//
				if(partOfResultNodePtr!=NULL)
				{
					#ifdef DEBUG
						int ddX = abs((cur_result_node_ptr-1)->node_x-partOfResultNodePtr[0].node_x);
						int ddY = abs((cur_result_node_ptr-1)->node_y-partOfResultNodePtr[0].node_y);
						err_when(ddX && ddY && ddX!=ddY);
					#endif

					err_when(preNonblockedXLoc!=partOfResultNodePtr->node_x || preNonblockedYLoc!=partOfResultNodePtr->node_y);

					restNode = partOfResultNodeNum-1;
					curNodePtr = partOfResultNodePtr+1;
					
					while(restNode)
					{
						add_result(curNodePtr->node_x, curNodePtr->node_y);
						curNodePtr++;
						restNode--;
					}

					debug_reuse_check_path(); //-************** debug checking 
	
					mem_del(partOfResultNodePtr);
					partOfResultNodePtr = NULL;
				}
			}//end if get_next_nonblocked_offset_loc()

			//-------------------------------------------------------------------------------------------------//
			// update leaderNode?Loc since it may be changed after calling get_next_nonblocked_offset_loc()
			//-------------------------------------------------------------------------------------------------//
			leaderNodeXLoc = nextNonblockedLeaderXLoc;
			leaderNodeYLoc = nextNonblockedLeaderYLoc;
		}

		debug_reuse_check_path(); //-************** debug checking 
		err_when(partOfResultNodePtr!=NULL);
	}	// end while

	err_when(partOfResultNodePtr!=NULL);
	debug_reuse_check_path(); //-************** debug checking 
}
//--------- End of function SeekPathReuse::use_offset_method ---------//


//-------- Begin of function SeekPathReuse::get_next_nonblocked_offset_loc ---------//
// find the next nonblocked offset location if the inputed location is blocked
// return 1 if found, 0 for none
//
int SeekPathReuse::get_next_nonblocked_offset_loc(int& nextXLoc, int&nextYLoc)
{
	int found = 0;
	int unitDestX, unitDestY;
	
	/*#ifdef DEBUG
		int debugLoopCount = 0;
	#endif

	while(!found)
	{
		err_when(++debugLoopCount>10000);
		if(!get_next_offset_loc(nextXLoc, nextYLoc))
			break; // all nodes visited

		unitDestX = nextXLoc+x_offset;
		unitDestY = nextYLoc+y_offset;

		err_when(unit_size!=1);
		if(unitDestX>=0 && unitDestX<MAX_WORLD_X_LOC && unitDestY>=0 && unitDestY<MAX_WORLD_Y_LOC &&
			can_walk(unitDestX, unitDestY))
			found = 1;
	}

	return found;*/
	while(!found)
	{
		if(nextXLoc != cur_leader_node_ptr->node_x || nextYLoc != cur_leader_node_ptr->node_y)
		{
			nextXLoc += leader_vec_x;
			nextYLoc += leader_vec_y;

			unitDestX = nextXLoc+x_offset;
			unitDestY = nextYLoc+y_offset;

			err_when(unit_size!=1);
			if(unitDestX>=0 && unitDestX<MAX_WORLD_X_LOC && unitDestY>=0 && unitDestY<MAX_WORLD_Y_LOC &&
				can_walk(unitDestX, unitDestY))
				found = 1;
		}
		else
		{
			if(cur_leader_node_num < reuse_leader_path_node_num)
			{
				cur_leader_node_num++;
				cur_leader_node_ptr++;
				leader_vec_x = cur_leader_node_ptr->node_x - nextXLoc;
				leader_vec_y = cur_leader_node_ptr->node_y - nextYLoc;
				if(leader_vec_x!=0)
				{
					leader_vec_x /= abs(leader_vec_x);
					nextXLoc += leader_vec_x;
				}
				if(leader_vec_y!=0)
				{
					leader_vec_y /= abs(leader_vec_y);
					nextYLoc += leader_vec_y;
				}

				unitDestX = nextXLoc+x_offset;
				unitDestY = nextYLoc+y_offset;

				err_when(unit_size!=1);
				if(unitDestX>=0 && unitDestX<MAX_WORLD_X_LOC && unitDestY>=0 && unitDestY<MAX_WORLD_Y_LOC &&
					can_walk(unitDestX, unitDestY))
					found = 1;
			}
			else
				break;	// or return 0;
		}

		sys_yield(); // update cursor position
	}

	return found;
}
//--------- End of function SeekPathReuse::get_next_nonblocked_offset_loc ---------//


//-------- Begin of function SeekPathReuse::get_next_offset_loc ---------//
//	get the next location along the main path (reuse reference path)
// return 1 if found, 0 for end of the path
//
int SeekPathReuse::get_next_offset_loc(int& nextXLoc, int& nextYLoc)
{
	if(nextXLoc != cur_leader_node_ptr->node_x || nextYLoc != cur_leader_node_ptr->node_y)
	{
		//--------- point in a the middle of two nodes ----------//
		nextXLoc += leader_vec_x*move_scale;
		nextYLoc += leader_vec_y*move_scale;
		return 1;
	}
	else if(cur_leader_node_num < reuse_leader_path_node_num)
	{
		//------------- the end of a node ----------//
		cur_leader_node_num++;
		cur_leader_node_ptr++;
		leader_vec_x = cur_leader_node_ptr->node_x - nextXLoc;
		leader_vec_y = cur_leader_node_ptr->node_y - nextYLoc;
		err_when(leader_vec_x!=0 && leader_vec_y!=0 && abs(leader_vec_x)!=abs(leader_vec_y));

		if(leader_vec_x!=0)
		{
			leader_vec_x /= abs(leader_vec_x);
			nextXLoc += leader_vec_x*move_scale;
		}
		if(leader_vec_y!=0)
		{
			leader_vec_y /= abs(leader_vec_y);
			nextYLoc += leader_vec_y*move_scale;
		}

		err_when(nextXLoc<0 || nextXLoc>=MAX_WORLD_X_LOC);
		err_when(nextYLoc<0 || nextYLoc>=MAX_WORLD_Y_LOC);
		return 1;
	}
	else
		return 0;
}
//--------- End of function SeekPathReuse::get_next_offset_loc ---------//


//-------- Begin of function SeekPathReuse::copy_leader_path_offset ---------//
void SeekPathReuse::copy_leader_path_offset()
{
	if(!is_leader_path_valid())
	{
		incomplete_search++;
		reuse_path_status = REUSE_PATH_INCOMPLETE_SEARCH;
		return;
	}

	err_when(leader_path_num<0 || leader_path_num>total_num_of_path);
	cur_leader_node_ptr = reuse_leader_path_backup;
	cur_leader_node_num = 1;

	ResultNode *curNodePtr = cur_leader_node_ptr;
	err_when(curNodePtr->node_x+x_offset<0 || curNodePtr->node_x+x_offset>=MAX_WORLD_X_LOC ||
				curNodePtr->node_y+y_offset<0 || curNodePtr->node_y+y_offset>=MAX_WORLD_Y_LOC);

	int preXLoc = curNodePtr->node_x+x_offset;
	int preYLoc = curNodePtr->node_y+y_offset;
	add_result(preXLoc, preYLoc);
	
	int curXLoc, curYLoc;
	curNodePtr++;

	int status = 0; // 0 for current position inside map, 1 for outside map
	int checkXLoc, checkYLoc;
	int vecX, vecY, magnX, magnY, magn;
	int i, quitLoop;
	Location *locPtr;

	while(cur_leader_node_num++<reuse_leader_path_node_num)
	{
		curXLoc = curNodePtr->node_x+x_offset;
		curYLoc = curNodePtr->node_y+y_offset;
		
		//----------------------------------------------------------------------//
		// offset method is terminated when the path leaves the map region for
		// this version.
		//----------------------------------------------------------------------//
		if(curXLoc>=0 && curXLoc<MAX_WORLD_X_LOC && curYLoc>=0 && curYLoc<MAX_WORLD_Y_LOC) // inside map
		{
			//----------------------------------------------------------------------//
			// checking passable condition
			//----------------------------------------------------------------------//
			if(reuse_search_sub_mode==SEARCH_SUB_MODE_PASSABLE)
			{
				vecX = curXLoc-preXLoc;
				vecY = curYLoc-preYLoc;
				magn = ((magnX=abs(vecX)) > (magnY=abs(vecY))) ? magnX : magnY;
				if(magnX) vecX /= magnX;
				if(magnY) vecY /= magnY;
				checkXLoc = preXLoc;
				checkYLoc = preYLoc;
				quitLoop = 0;

				for(i=0; i<magn; ++i)
				{
					checkXLoc += vecX;
					checkYLoc += vecY;
					locPtr = world.get_loc(checkXLoc, checkYLoc);
					if(locPtr->power_nation_recno && !reuse_nation_passable[locPtr->power_nation_recno])
					{
						quitLoop = 1;
						break; // can't handle this case: not passable and copying leader path
					}
				}

				if(quitLoop)
					break;
			}

			status = 0;
			if(!status) // inside
				move_within_map(preXLoc, preYLoc, curXLoc, curYLoc);
			else // outside
				//move_inside_map(preXLoc, preYLoc, curXLoc, curYLoc);
				break;
		}
		else // outside
		{
			//----------------------------------------------------------------------//
			// checking passable condition
			//----------------------------------------------------------------------//
			if(reuse_search_sub_mode==SEARCH_SUB_MODE_PASSABLE)
				break;

			status = 1;
			if(!status) // inside
			{
				move_outside_map(preXLoc, preYLoc, curXLoc, curYLoc);
				break;
			}
			else // outside
				//move_beyond_map(preXLoc, preYLoc, curXLoc, curYLoc);
				break;
		}

		debug_reuse_check_path(); //-************** debug checking 
		preXLoc = curXLoc;
		preYLoc = curYLoc;
		curNodePtr++;
	}

	debug_reuse_check_path(); //-************** debug checking 
}
//--------- End of function SeekPathReuse::copy_leader_path_offset ---------//
