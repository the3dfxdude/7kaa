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
//Description : Object Unit misc. actions excluding attacking and moving functions
//Owner		  : Alex

#include <ALL.h>
#include <OWORLD.h>
#include <OFIRM.h>
#include <ONATION.h>
#include <ORACERES.h>
#include <OTOWN.h>
#include <OU_MARI.h>
#include <OF_CAMP.h>
#include <OREMOTE.h>
#include <OF_MONS.h>
#include <OU_GOD.h>
#include <OF_HARB.h>

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

//--------- Begin of function Unit::build_firm ---------//
// Build a firm.
//
// <int> buildXLoc, buildYLoc - the location to build
// <int> firmId               - id. of the firm to build
//
// [char] remoteAction
//
void Unit::build_firm(int buildXLoc, int buildYLoc, int firmId, char remoteAction)
{
	err_when(sprite_info->loc_width>1 || sprite_info->loc_height>1);

	if(!remoteAction && remote.is_enable() )
	{
		// packet structure : <unit recno> <xLoc> <yLoc> <firmId>
		short *shortPtr =(short *)remote.new_send_queue_msg(MSG_UNIT_BUILD_FIRM, 4*sizeof(short) );
		shortPtr[0] = sprite_recno;
		shortPtr[1] = buildXLoc;
		shortPtr[2] = buildYLoc;
		shortPtr[3] = firmId;
		return;
	}

	//----------------------------------------------------------------//
	// return if the unit is dead
	//----------------------------------------------------------------//
	if(hit_points<=0 || action_mode==ACTION_DIE || cur_action==SPRITE_DIE)
		return;

	//----------------------------------------------------------------//
	// location is blocked, cannot build. so move there instead
	//----------------------------------------------------------------//
	if(!world.can_build_firm(buildXLoc, buildYLoc, firmId, sprite_recno))
	{
		//reset_action_para2();
		move_to(buildXLoc, buildYLoc);
		return;
	}

	//----------------------------------------------------------------//
	// different territory
	//----------------------------------------------------------------//

	int harborDir = world.can_build_firm(buildXLoc, buildYLoc, firmId, sprite_recno);
	int goX = buildXLoc, goY = buildYLoc;
	if( firm_res[firmId]->tera_type == 4)
	{
		switch(harborDir)
		{
		case 1:		// north exit
			goX += 1;
			goY += 2;
			break;
		case 2:		// south exit
			goX += 1;
			break;
		case 4:		// west exit
			goX += 2;
			goY += 1;
			break;
		case 8:		// east exit
			goY += 1;
			break;
		default:
			err_here();
			move_to(buildXLoc, buildYLoc);
			return;
		}
		if(world.get_loc(next_x_loc(), next_y_loc())->region_id!=world.get_loc(goX,goY)->region_id)
		{
			move_to(buildXLoc, buildYLoc);
			return;
		}
	}
	else
	{
		if(world.get_loc(next_x_loc(), next_y_loc())->region_id!=world.get_loc(buildXLoc, buildYLoc)->region_id)
		{
			move_to(buildXLoc, buildYLoc);
			return;
		}
	}
	
	//----------------------------------------------------------------//
	// action_mode2: checking for equal action or idle action
	//----------------------------------------------------------------//
	if(action_mode2==ACTION_BUILD_FIRM && action_para2==firmId && action_x_loc2==buildXLoc && action_y_loc2==buildYLoc)
	{
		if(cur_action!=SPRITE_IDLE)
			return;
	}
	else
	{
		//----------------------------------------------------------------//
		// action_mode2: store new order
		//----------------------------------------------------------------//
		action_mode2  = ACTION_BUILD_FIRM;
		action_para2  = firmId;
		action_x_loc2 = buildXLoc;
		action_y_loc2 = buildYLoc;
	}

	//----- order the sprite to stop as soon as possible -----//
	stop();	// new order

	//---------------- define parameters -------------------//
	FirmInfo* firmInfo = firm_res[firmId];
	int firmWidth = firmInfo->loc_width;
	int firmHeight = firmInfo->loc_height;

	if(!is_in_surrounding(move_to_x_loc, move_to_y_loc, sprite_info->loc_width, buildXLoc, buildYLoc, firmWidth, firmHeight))
	{
		//----------- not in the firm surrounding ---------//
		set_move_to_surround(buildXLoc, buildYLoc, firmWidth, firmHeight, BUILDING_TYPE_FIRM_BUILD, firmId);
	}
	else
	{
		//------- the unit is in the firm surrounding -------//
		set_cur(next_x, next_y);
		set_dir(move_to_x_loc, move_to_y_loc, buildXLoc+firmWidth/2, buildYLoc+firmHeight/2);
	}

	//----------- set action to build the firm -----------//
	action_mode  = ACTION_BUILD_FIRM;
	action_para  = firmId;
	action_x_loc = buildXLoc;
	action_y_loc = buildYLoc;
}
//----------- End of function Unit::build_firm -----------//


//--------- Begin of function Unit::burn ---------//
//
// Burn a locaiton
//
// <int> burnXLoc, burnYLoc - the location to burn
//
void Unit::burn(int burnXLoc, int burnYLoc, char remoteAction)
{
	err_when(sprite_info->loc_width>1 || sprite_info->loc_height>1);

	if( !remoteAction && remote.is_enable() )
	{
		// packet structure : <unit recno> <xLoc> <yLoc>
		short *shortPtr = (short *)remote.new_send_queue_msg(MSG_UNIT_BURN, 3*sizeof(short) );
		shortPtr[0] = sprite_recno;
		shortPtr[1] = burnXLoc;
		shortPtr[2] = burnYLoc;
		return;
	}

	if(move_to_x_loc==burnXLoc && move_to_y_loc==burnYLoc)
		return;	// should not burn the unit itself

	//----------------------------------------------------------------//
	// return if the unit is dead
	//----------------------------------------------------------------//
	if(hit_points<=0 || action_mode==ACTION_DIE || cur_action==SPRITE_DIE)
		return;

	//----------------------------------------------------------------//
	// move there instead if ordering to different territory
	//----------------------------------------------------------------//
	if(world.get_loc(next_x_loc(), next_y_loc())->region_id!=world.get_loc(burnXLoc, burnYLoc)->region_id)
	{
		move_to(burnXLoc, burnYLoc);
		return;
	}

	//----------------------------------------------------------------//
	// action_mode2: checking for equal action or idle action
	//----------------------------------------------------------------//
	if(action_mode2==ACTION_BURN && action_x_loc2==burnXLoc && action_y_loc2==burnYLoc)
	{
		if(cur_action!=SPRITE_IDLE)
			return;
	}
	else
	{
		//----------------------------------------------------------------//
		// action_mode2: store new order
		//----------------------------------------------------------------//
		action_mode2  = ACTION_BURN;
		action_para2  = 0;
		action_x_loc2 = burnXLoc;
		action_y_loc2 = burnYLoc;
	}

	//----- order the sprite to stop as soon as possible -----//
	stop();	// new order
	
	if(abs(burnXLoc-next_x_loc())>1 || abs(burnYLoc-next_y_loc())>1)
	{
		//--- if the unit is not in the burning surrounding location, move there first ---//
		search(burnXLoc, burnYLoc, 1, SEARCH_MODE_A_UNIT_IN_GROUP);

		if(move_to_x_loc != burnXLoc || move_to_y_loc != burnYLoc)	// cannot reach the destination
		{
			action_mode  = ACTION_BURN;
			action_para	 = 0;
			action_x_loc = burnXLoc;
			action_y_loc = burnYLoc;
			return;	// just move to the closest location returned by shortest path searching
		}
	}
	else
	{
		if(cur_x==next_x && cur_y==next_y)
			set_dir(next_x_loc(), next_y_loc(), burnXLoc, burnYLoc);

		err_when((cur_x!=next_x || cur_y!=next_y) && (check_unit_dir1=get_dir(cur_x,cur_y,next_x,next_y))!=final_dir);
		err_when(result_node_array || result_node_count || result_path_dist);
	}

	//--------------------------------------------------------//
	// edit the result path such that the unit can reach the
	// burning location surrounding
	//--------------------------------------------------------//
	if(result_node_array && result_node_count)
	{
		//--------------------------------------------------------//
		// there should be at least two nodes, and should take at
		// least two steps to the destination
		//--------------------------------------------------------//
		err_when(move_to_x_loc!=burnXLoc || move_to_y_loc!=burnYLoc);
		err_when(result_node_count<2);

		ResultNode* lastNode1 = result_node_array+result_node_count-1;	// the last node
		ResultNode* lastNode2 = result_node_array+result_node_count-2;	// the node before the last node

		int vX = lastNode1->node_x-lastNode2->node_x;	// get the vectors direction
		int vY = lastNode1->node_y-lastNode2->node_y;
		int vDirX = (vX) ? vX/abs(vX) : 0;
		int vDirY = (vY) ? vY/abs(vY) : 0;

		if(result_node_count>2)	// go_? should not be the burning location 
		{
			err_when(go_x>>ZOOM_X_SHIFT_COUNT==burnXLoc && go_y>>ZOOM_Y_SHIFT_COUNT==burnYLoc);
			err_when(vX!=0 && vY!=0 && abs(vX)!=abs(vY));

			if(abs(vX)>1 || abs(vY)>1)
			{
				lastNode1->node_x -= vDirX;
				lastNode1->node_y -= vDirY;

				move_to_x_loc = lastNode1->node_x;
				move_to_y_loc = lastNode1->node_y;
			}
			else	// move only one step
			{
				result_node_count--;	// remove a node
				move_to_x_loc = lastNode2->node_x;
				move_to_y_loc = lastNode2->node_y;
			}
		}
		else	// go_? may be the burning location
		{
			err_when(result_node_count!=2);

			lastNode1->node_x -= vDirX;
			lastNode1->node_y -= vDirY;

			if(go_x>>ZOOM_X_SHIFT_COUNT==burnXLoc && go_y>>ZOOM_Y_SHIFT_COUNT==burnYLoc)	// go_? is the burning location
			{
				//--- edit parameters such that only moving to the nearby location to do the action ---//
				err_when(abs(vX)<=1 && abs(vY)<=1);	// this case should be handled before
				
				go_x = lastNode1->node_x * ZOOM_LOC_WIDTH;
				go_y = lastNode1->node_y * ZOOM_LOC_HEIGHT;
			}
			//else the unit is still doing sthg else, no action here

			move_to_x_loc = lastNode1->node_x;
			move_to_y_loc = lastNode1->node_y;
		}

		//--------------------------------------------------------------//
		// reduce the result_path_dist by 1
		//--------------------------------------------------------------//
		result_path_dist--;
		
		#ifdef DEBUG
			ResultNode *preNode = result_node_array;
			ResultNode *curNode = result_node_array+1;
			int debugCount=1;
			int debugDist = 0;
			int xDist = abs(next_x_loc()-preNode->node_x);
			int yDist = abs(next_y_loc()-preNode->node_y);
			debugDist -= (xDist) ? xDist : yDist;

			while(debugCount++ < result_node_count)
			{
				err_when(debugCount>1000);
				xDist = abs(preNode->node_x - curNode->node_x);
				yDist = abs(preNode->node_y - curNode->node_y);
				debugDist += (xDist) ? xDist : yDist;
				preNode++;
				curNode++;
			}

			err_when(result_path_dist!=debugDist);
		#endif

		err_when((cur_x!=next_x || cur_y!=next_y) &&	// is not blocked
					(check_unit_dir1=get_dir(cur_x, cur_y, next_x, next_y))!=(check_unit_dir2=get_dir(cur_x, cur_y, go_x, go_y)));
	}

	err_when(!is_in_surrounding(move_to_x_loc, move_to_y_loc, sprite_info->loc_width, burnXLoc, burnYLoc, 1, 1));

	//-- set action if the burning location can be reached, otherwise just move nearby --//
	action_mode  = ACTION_BURN;
	action_para	 = 0;
	action_x_loc = burnXLoc;
	action_y_loc = burnYLoc;
}
//----------- End of function Unit::burn -----------//


//--------- Begin of function Unit::settle ---------//
//
// settle to a town
//
// <int> settleXLoc, settleYLoc - the location to settle
// [short] curSettleUnitNum - the number to call this function by
//										a group of unit. (default: 1)
//
void Unit::settle(int settleXLoc, int settleYLoc, short curSettleUnitNum)
{
	err_when(sprite_info->loc_width>1 || sprite_info->loc_height>1);

	//----------------------------------------------------------------//
	// return if the unit is dead
	//----------------------------------------------------------------//
	if(hit_points<=0 || action_mode==ACTION_DIE || cur_action==SPRITE_DIE)
		return;

	//---------- no settle for non-human -----------//
	if(unit_res[unit_id]->unit_class!=UNIT_CLASS_HUMAN)
		return;

	//----------------------------------------------------------------//
	// move there if cannot settle
	//----------------------------------------------------------------//
	if(!world.can_build_town(settleXLoc, settleYLoc, sprite_recno))
	{
		Location *locPtr = world.get_loc(settleXLoc, settleYLoc);
		if(locPtr->is_town() && town_array[locPtr->town_recno()]->nation_recno==nation_recno)
			assign(settleXLoc, settleYLoc);
		else
			move_to(settleXLoc, settleYLoc);
		return;
	}

	//----------------------------------------------------------------//
	// move there if location is in different territory
	//----------------------------------------------------------------//
	if(world.get_loc(next_x_loc(), next_y_loc())->region_id!=world.get_loc(settleXLoc, settleYLoc)->region_id)
	{
		move_to(settleXLoc, settleYLoc);
		return;
	}

	//----------------------------------------------------------------//
	// action_mode2: checking for equal action or idle action
	//----------------------------------------------------------------//
	if(action_mode2==ACTION_SETTLE && action_x_loc2==settleXLoc && action_y_loc2==settleYLoc)
	{
		if(cur_action!=SPRITE_IDLE)
			return;
	}
	else
	{
		//----------------------------------------------------------------//
		// action_mode2: store new order
		//----------------------------------------------------------------//
		action_mode2  = ACTION_SETTLE;
		action_para2  = 0;
		action_x_loc2 = settleXLoc;
		action_y_loc2 = settleYLoc;
	}

	//----- order the sprite to stop as soon as possible -----//
	stop();	// new order
	
	if(!is_in_surrounding(move_to_x_loc, move_to_y_loc, sprite_info->loc_width, settleXLoc, settleYLoc,
		STD_TOWN_LOC_WIDTH, STD_TOWN_LOC_HEIGHT))
	{
		//------------ not in the town surrounding ------------//
		set_move_to_surround(settleXLoc, settleYLoc, STD_TOWN_LOC_WIDTH, STD_TOWN_LOC_HEIGHT, BUILDING_TYPE_SETTLE, 0, 0, curSettleUnitNum);
	}
	else
	{
		//------- the unit is within the settle location -------//
		set_cur(next_x, next_y);
		set_dir(move_to_x_loc, move_to_y_loc, settleXLoc+STD_TOWN_LOC_WIDTH/2, settleYLoc+STD_TOWN_LOC_HEIGHT/2);
	}

	//----------- set action to settle -----------//
	action_mode  = ACTION_SETTLE;
	action_para  = 0;
	action_x_loc = settleXLoc;
	action_y_loc = settleYLoc;
}
//----------- End of function Unit::settle -----------//


//--------- Begin of function Unit::assign ---------//
//
// Assign an unit to :
//
// - a firm as an overseer, as a worker
// - a town as a citizen
// - a vehicle
//
// <int> buildXLoc, buildYLoc - the location to build
//
void Unit::assign(int assignXLoc, int assignYLoc, short curAssignUnitNum)
{
	err_when(sprite_info->loc_width>1 || sprite_info->loc_height>1);

	//----------------------------------------------------------------//
	// return if the unit is dead
	//----------------------------------------------------------------//
	if(hit_points<=0 || action_mode==ACTION_DIE || cur_action==SPRITE_DIE)
		return;

	//----------- BUGHERE : cannot assign when on a ship -----------//
	err_when(!is_visible())
	if(!is_visible())
		return;

	//----------- cannot assign for caravan -----------//
	if(unit_id==UNIT_CARAVAN)
		return;

	//----------------------------------------------------------------//
	// move there if the destination in other territory
	//----------------------------------------------------------------//
	Location	*locPtr = world.get_loc(assignXLoc, assignYLoc);
	uint8_t unitRegionId = world.get_loc(next_x_loc(), next_y_loc())->region_id;
	if(locPtr->is_firm())
	{
		Firm *firmPtr = firm_array[locPtr->firm_recno()];
		int quit = 0;
		
		if(firmPtr->firm_id==FIRM_HARBOR)
		{
			FirmHarbor *harborPtr = (FirmHarbor*) firmPtr;
			switch(unit_res[unit_id]->unit_class)
			{
				case UNIT_CLASS_HUMAN:
					if(unitRegionId != harborPtr->land_region_id)
						quit = 1;
					break;

				case UNIT_CLASS_SHIP:
					if(unitRegionId != harborPtr->sea_region_id)
						quit = 1;
					break;

				default: err_here();
							break;
			}
		}
		else if(unitRegionId!=locPtr->region_id)
			quit = 1;

		if(quit)
		{
			move_to_firm_surround(assignXLoc, assignYLoc, sprite_info->loc_width, sprite_info->loc_height, firmPtr->firm_id);
			return;
		}
	}
	else if(unitRegionId!=locPtr->region_id)
	{
		if(locPtr->is_town())
			move_to_town_surround(assignXLoc, assignYLoc, sprite_info->loc_width, sprite_info->loc_height);
		/*else if(locPtr->has_unit(UNIT_LAND))
		{
			Unit *unitPtr = unit_array[locPtr->unit_recno(UNIT_LAND)];
			move_to_unit_surround(assignXLoc, assignYLoc, sprite_info->loc_width, sprite_info->loc_height, unitPtr->sprite_recno);
		}*/
		
		return;
	}

	//---------------- define parameters --------------------//
	int		width, height;
	int		buildingType=0; // 1 for Firm, 2 for TownZone
	short		recno;
	int		firmNeedUnit=1;
	
	if(locPtr->is_firm())
	{
		//-------------------------------------------------------//
		// the location is firm
		//-------------------------------------------------------//
		recno = locPtr->firm_recno();

		//----------------------------------------------------------------//
		// action_mode2: checking for equal action or idle action
		//----------------------------------------------------------------//
		if(action_mode2==ACTION_ASSIGN_TO_FIRM && action_para2==recno && action_x_loc2==assignXLoc && action_y_loc2==assignYLoc)
		{
			if(cur_action!=SPRITE_IDLE)
				return;
		}
		else
		{
			//----------------------------------------------------------------//
			// action_mode2: store new order
			//----------------------------------------------------------------//
			action_mode2  = ACTION_ASSIGN_TO_FIRM;
			action_para2  = recno;
			action_x_loc2 = assignXLoc;
			action_y_loc2 = assignYLoc;
		}
		
		Firm		*firmPtr = firm_array[recno];
		FirmInfo *firmInfo = firm_res[firmPtr->firm_id];

		if(!firm_can_assign(recno))
		{
			//firmNeedUnit = 0; // move to the surrounding of the firm
			move_to_firm_surround(assignXLoc, assignYLoc, sprite_info->loc_width, sprite_info->loc_height, firmPtr->firm_id);
			return;
		}

		width  = firmInfo->loc_width;
		height = firmInfo->loc_height;
		buildingType = BUILDING_TYPE_FIRM_MOVE_TO;
	}
	else if(locPtr->is_town())	// there is town
	{
		if(unit_res[unit_id]->unit_class != UNIT_CLASS_HUMAN)
			return;

		//-------------------------------------------------------//
		// the location is town
		//-------------------------------------------------------//
		recno = locPtr->town_recno();
		
		//----------------------------------------------------------------//
		// action_mode2: checking for equal action or idle action
		//----------------------------------------------------------------//
		if(action_mode2==ACTION_ASSIGN_TO_TOWN && action_para2==recno && action_x_loc2==assignXLoc && action_y_loc2==assignYLoc)
		{
			if(cur_action!=SPRITE_IDLE)
				return;
		}
		else
		{
			//----------------------------------------------------------------//
			// action_mode2: store new order
			//----------------------------------------------------------------//
			action_mode2  = ACTION_ASSIGN_TO_TOWN;
			action_para2  = recno;
			action_x_loc2 = assignXLoc;
			action_y_loc2 = assignYLoc;
		}

		Town *targetTown = town_array[recno];
		if(town_array[recno]->nation_recno != nation_recno)
		{
			move_to_town_surround(assignXLoc, assignYLoc, sprite_info->loc_width, sprite_info->loc_height);
			return;
		}

		width = targetTown->loc_width();
		height = targetTown->loc_height();
		buildingType = BUILDING_TYPE_TOWN_MOVE_TO;
	}
	/*else if(locPtr->has_unit(UNIT_LAND)) // there is vehicle
	{
		//-------------------------------------------------------//
		// the location is vehicle
		//-------------------------------------------------------//
		Unit* vehicleUnit = unit_array[locPtr->unit_recno(UNIT_LAND)];
		if( vehicleUnit->unit_id!=unit_res[unit_id]->vehicle_id )
			return;

		recno = vehicleUnit->sprite_recno;

		//----------------------------------------------------------------//
		// action_mode2: checking for equal action or idle action
		//----------------------------------------------------------------//
		if(action_mode2==ACTION_ASSIGN_TO_VEHICLE && action_para2==recno && action_x_loc2==assignXLoc && action_y_loc2==assignYLoc)
		{
			if(cur_action!=SPRITE_IDLE)
				return;
		}
		else
		{
			//----------------------------------------------------------------//
			// action_mode2: store new order
			//----------------------------------------------------------------//
			action_mode2  = ACTION_ASSIGN_TO_VEHICLE;
			action_para2  = recno;
			action_x_loc2 = assignXLoc;
			action_y_loc2 = assignYLoc;
		}

		SpriteInfo* spriteInfo = vehicleUnit->sprite_info;
		width	 = spriteInfo->loc_width;
		height = spriteInfo->loc_height;
		buildingType = BUILDING_TYPE_VEHICLE;
	}*/
	else
	{
		stop2(KEEP_DEFENSE_MODE);
		return;
	}

	//-----------------------------------------------------------------//
	// order the sprite to stop as soon as possible (new order)
	//-----------------------------------------------------------------//
	stop();
	set_move_to_surround(assignXLoc, assignYLoc, width, height, buildingType, 0, 0, curAssignUnitNum);

	//-----------------------------------------------------------------//
	// able to reach building surrounding, set action parameters
	//-----------------------------------------------------------------//
	action_para  = recno;
	action_x_loc = assignXLoc;
	action_y_loc = assignYLoc;

	switch(buildingType)
	{
		case BUILDING_TYPE_FIRM_MOVE_TO:
				action_mode = ACTION_ASSIGN_TO_FIRM;
				break;

		case BUILDING_TYPE_TOWN_MOVE_TO:
				action_mode = ACTION_ASSIGN_TO_TOWN;
				break;

		case BUILDING_TYPE_VEHICLE:
				action_mode = ACTION_ASSIGN_TO_VEHICLE;
				break;
	}

	//##### begin trevor 9/10 #######//

//	force_move_flag = 1;		// don't stop and fight back on an assign mission

	//##### end trevor 9/10 #######//

	//-----------------------------------------------------------------//
	// edit parameters for those firms don't need unit
	//-----------------------------------------------------------------//
	/*if(!firmNeedUnit)
	{
		action_mode2 = action_mode = ACTION_MOVE;
		action_para2 = action_para = 0;
		action_x_loc2 = action_x_loc = move_to_x_loc;
		action_y_loc2 = action_y_loc = move_to_y_loc;
	}*/
}
//----------- End of function Unit::assign -----------//


//--------- Begin of function Unit::firm_can_assign ---------//
// return 1 for true, i.e. unit can assign to the firm
// return 2 for assigning leader
// return 3 for repair
// return 4 for spy assigning to inn
// return 5 for capture	(not used)
// return 0 otherwise
//
int Unit::firm_can_assign(short firmRecno)
{
	Firm *firmPtr = firm_array[firmRecno];
	FirmInfo *firmInfo = firm_res[firmPtr->firm_id];

	switch( unit_res[unit_id]->unit_class )
	{
	case UNIT_CLASS_HUMAN:
		if( nation_recno == firmPtr->nation_recno )
		{
			if( skill.skill_id == SKILL_CONSTRUCTION && firmPtr->firm_id != FIRM_MONSTER)
			{
				return 3;
			}

			// ###### begin Gilbert 22/10 #######//
			//----------------------------------------//
			// If this is a spy, then he can only be
			// assigned to an enemy firm when there is
			// space for the unit.
			//----------------------------------------//

			//if( spy_recno && true_nation_recno() != firmPtr->nation_recno )
			//{
			//	if( rank_id == RANK_GENERAL )
			//	{
			//		if( firmPtr->overseer_recno )
			//			return 0;
			//	}
			//	else
			//	{
			//		if( firmPtr->worker_count == MAX_WORKER )
			//			return 0;
			//	}
			//}
			//--------------------------------------//
			// ###### end Gilbert 22/10 #######//

			switch( firmPtr->firm_id )
			{
			case FIRM_CAMP:
				return rank_id == RANK_SOLDIER ? 1 : 2;

			case FIRM_BASE:
				if(race_id == firmPtr->race_id)
				{
					if( !skill.skill_id || skill.skill_id==SKILL_PRAYING)	// non-skilled worker
						return 1;
					if( rank_id != RANK_SOLDIER )
						return 2;
				}
				break;

			//case FIRM_INN:
				// shealthed soldier spy can assign to inn
			//	return rank_id == RANK_SOLDIER && nation_recno != true_nation_recno() ? 4 : 0;

			default:
				return rank_id == RANK_SOLDIER && firmInfo->need_unit() ? 1 : 0;
			}
		}
		break;

	case UNIT_CLASS_WEAPON:
		if(firmPtr->firm_id == FIRM_CAMP && nation_recno == firmPtr->nation_recno)
			return 1;
		break;

	case UNIT_CLASS_SHIP:
		if(firmPtr->firm_id == FIRM_HARBOR && nation_recno == firmPtr->nation_recno)
			return 1;
		break;

	case UNIT_CLASS_MONSTER:
		if(firmPtr->firm_id == FIRM_MONSTER && mobile_type == UNIT_LAND)
		{ 
			// BUGHERE : suppose only land monster can assign
			return rank_id == RANK_SOLDIER ? 1 : 2;
		}
		break;

	case UNIT_CLASS_GOD:
	case UNIT_CLASS_CARAVAN:
		break;
	default:
		err_here();		// undefined unit class
	}

	return 0;
}
//----------- End of function Unit::firm_can_assign -----------//


//--------- Begin of function Unit::set_move_to_surround ---------//
// Mode 1:
//		return 1 if the unit can reach the surrounding of the firm
//		return 0 otherwise.
//
// Mode 2:
//		In this mode, the unit calling this function is ordered to move to
// location a little bit away from the surrounding of the object.
//
// return 1 if the unit can move to there
// return 0 otherwise
//
// <int> buildXLoc				- upper left x location of the building (firm/town)
// <int> buildYLoc				- upper left y location of the building
// <int> width						- width of the building
// <int> height					- height of the building
// <int> buildingType			- used to determine how searching is processed
// [int] miscNo					- firm_id of firm if buildingType==BUILDING_TYPE_FIRM_BUILD
//										- 0 for group search  (for wall, firm)
//										- 1 for a unit search
// [short] readyDist				- the extra distance the unit stands from the building
//											(default 0, i.e. the surrounding of the building)
// [short] curProcessUnitNum	- the cur unit no. in a group calling this function
//											(default 1)
//
//============================================================================//
// Note:	This funcion should not change any action parameters
//============================================================================//
//
int Unit::set_move_to_surround(int buildXLoc, int buildYLoc, int width, int height, int buildingType, int miscNo, int readyDist, short curProcessUnitNum)
{
	err_when(hit_points<=0 || action_mode==ACTION_DIE || cur_action==SPRITE_DIE);

	//--------------------------------------------------------------//
	// calculate the distance from the object
	//--------------------------------------------------------------//
	int found=0, foundAgain=0;
	int distance = cal_distance(buildXLoc, buildYLoc, width, height); // 0 for inside, 1 for surrounding, >1 for the rest
	
	//--------------------------------------------------------------//
	// inside the building
	//--------------------------------------------------------------//
	if(!distance)
	{
		reset_path();
		if(cur_x==next_x && cur_y==next_y)
			set_idle();

		return 1;
	}

	if(distance>1)
	{
		//--------------------------------------------------------------//
		// the searching is divided into 2 parts.
		//
		// part 1 using the firm_type and firm_id to find a shortest path.
		// 
		// part 2
		//	if the width and height is the actual width and height of the
		// firm, the unit move to the surrounding of the firm.
		//
		// if the width and height > the actual width and height of the
		// firm, the unit move to a location far away from the surrounding
		// of the firm.
		//--------------------------------------------------------------//

		//====================================================================//
		// part 1
		//====================================================================//
		
		Location	*locPtr = world.get_loc( buildXLoc, buildYLoc );
		Firm		*firmPtr = NULL;
		Town		*targetTown = NULL;
		int		searchResult;

		switch(buildingType)
		{
			case BUILDING_TYPE_FIRM_MOVE_TO:	// (assign) firm is on the location
						firmPtr = firm_array[locPtr->firm_recno()];
						searchResult = search(buildXLoc, buildYLoc, 1, SEARCH_MODE_TO_FIRM, firmPtr->firm_id, curProcessUnitNum);
						break;

			case BUILDING_TYPE_FIRM_BUILD:	// (build firm) no firm on the location
						err_when(sprite_info->loc_width>1);
						searchResult = search(buildXLoc, buildYLoc, 1, SEARCH_MODE_TO_FIRM, miscNo);
						break;

			case BUILDING_TYPE_TOWN_MOVE_TO:	// (assign) town is on the location
						targetTown = town_array[locPtr->town_recno()];
						searchResult = search(buildXLoc, buildYLoc, 1, SEARCH_MODE_TO_TOWN, targetTown->town_recno, curProcessUnitNum);
						break;

			case BUILDING_TYPE_SETTLE:			// (settle, first unit) no town on the location
						//---------------------------------------------------------------------//
						// the record number sent to the searching algorithm is used to determine
						// the width and the height of the building.  However, the standard
						// dimension for settling is used and the building built is a type of
						// town.  Thus, passing -1 as the recno. to show that "settle" is
						// processed
						//---------------------------------------------------------------------//
						err_when(sprite_info->loc_width>1);
						searchResult = search(buildXLoc, buildYLoc, 1, SEARCH_MODE_TO_TOWN, -1, curProcessUnitNum);
						break;

			case BUILDING_TYPE_VEHICLE:
						err_when(sprite_info->loc_width>1);
						searchResult = search(buildXLoc, buildYLoc, 1, SEARCH_MODE_TO_VEHICLE, (short)world.get_loc(buildXLoc, buildYLoc)->cargo_recno);
						break;

			case BUILDING_TYPE_WALL:			// wall is on the location
						err_when(miscNo!=0 && miscNo!=1);
						searchResult = search(buildXLoc, buildYLoc, 1, miscNo?SEARCH_MODE_TO_WALL_FOR_UNIT:SEARCH_MODE_TO_WALL_FOR_GROUP);
						break;

			default:	err_here();
						break;
		}

		if(!searchResult)
			return 0; // incomplete searching
		
		//====================================================================//
		// part 2
		//====================================================================//
		if(result_node_array && result_node_count)
			return edit_path_to_surround(buildXLoc, buildYLoc, buildXLoc+width-1, buildYLoc+height-1, readyDist);
		else
			return 0;
	}
	else	// in the surrounding, no need to move
	{
		reset_path();
		err_when(distance!=1);

		if(cur_x==next_x && cur_y==next_y)
		{
			move_to_x_loc = next_x_loc();
			move_to_y_loc = next_y_loc();
			go_x = cur_x;
			go_y = cur_y;
			set_idle();
			set_dir(move_to_x_loc, move_to_y_loc, buildXLoc + width/2, buildYLoc + height/2);

			err_when(result_node_array!=NULL);
		}

		err_when(cur_action==SPRITE_IDLE && (move_to_x_loc!=next_x_loc() || move_to_y_loc!=next_y_loc()));
		err_when(cur_action==SPRITE_IDLE && (cur_x!=next_x || cur_y!=next_y));
		return 1;
	}
}
//----------- End of function Unit::set_move_to_surround -----------//


//--------- Begin of function Unit::edit_path_to_surround ---------//
// edit the path such that the unit only move to the surrounding of the firm
//
// <int> objectXLoc1, objectYLoc1 - object top_left position
// <int> objectXLoc2, objectYLoc2 - object bottom_right position
// <short> readyDist				- the extra distance the unit stands from the building
//											(default 0, i.e. the surrounding of the building)
//
// return 1 if able to reach object surrounding
// return 0 otherwise
//
int Unit::edit_path_to_surround(int objectXLoc1, int objectYLoc1, int objectXLoc2, int objectYLoc2, int readyDist)
{
	err_when(!result_node_array || !result_node_count);
	if(result_node_count<2)
		return 0;

	//----------------------------------------------------------------------------//
	// At this moment, the unit generally has a path to the location inside the object,
	// walk through it and extract a path to the surrounding of the object.
	//----------------------------------------------------------------------------//

	//------- calculate the surrounding top-left and bottom-right points ------//
	int moveScale = move_step_magn();
	int xLoc1 = objectXLoc1 - readyDist - 1;
	int yLoc1 = objectYLoc1 - readyDist - 1;
	int xLoc2 = objectXLoc2 + readyDist + 1;
	int yLoc2 = objectYLoc2 + readyDist + 1;

	//------------------- boundary checking -------------------//
	if(xLoc1<0) xLoc1 = 0;
	if(yLoc1<0) yLoc1 = 0;
	if(xLoc2>=MAX_WORLD_X_LOC) yLoc1 = MAX_WORLD_X_LOC - moveScale;
	if(yLoc2>=MAX_WORLD_Y_LOC) xLoc2 = MAX_WORLD_Y_LOC - moveScale;

	//--------------- adjust for air and sea units -----------------//
	if(mobile_type!=UNIT_LAND)
	{
		//------ assume even x, y coordinate is used for UNIT_SEA and UNIT_AIR -------//
		if(xLoc1%2) xLoc1--;
		if(yLoc1%2) yLoc1--;
		if(xLoc2%2) xLoc2++;
		if(yLoc2%2) yLoc2++;

		if(xLoc2>MAX_WORLD_X_LOC-moveScale)
			xLoc2 = MAX_WORLD_X_LOC-moveScale;
		if(yLoc2>MAX_WORLD_Y_LOC-moveScale)
			yLoc2 = MAX_WORLD_Y_LOC-moveScale;

		err_when(xLoc1<0 || yLoc1<0);
		err_when(xLoc2>MAX_WORLD_X_LOC-moveScale || yLoc2>MAX_WORLD_Y_LOC-moveScale);
	}

	int checkXLoc = next_x_loc();
	int checkYLoc = next_y_loc();
	ResultNode *editNode1 = result_node_array;		// alias the unit's result_node_array
	ResultNode *editNode2 = result_node_array + 1;	// ditto


	int hasMoveStep = 0;
	if(checkXLoc!=editNode1->node_x || checkYLoc!=editNode1->node_y)
	{
		err_when(abs(checkXLoc-editNode1->node_x)>moveScale || abs(checkYLoc-editNode1->node_y)>moveScale);
		hasMoveStep += moveScale;
		checkXLoc = editNode1->node_x;
		checkYLoc = editNode1->node_y;
	}
	
	int i, j;
	int pathDist=0, found=0; // pathDist - counts the disitance of the generated path, found - whether a path to the surrounding is found
	int vecX, vecY, xMagn, yMagn, magn;

	#ifdef DEBUG
		int debugLoop1 = 0;
		int debugLoop2 = 0;
	#endif
	
	//------- find the first node that is on the surrounding of the object -------//
	for(i=1; i<result_node_count; ++i, editNode1++, editNode2++)
	{
		#ifdef DEBUG
		err_when(++debugLoop1>10000);
		#endif

		//------------ calculate parameters for checking ------------//
		vecX = editNode2->node_x - editNode1->node_x;
		vecY = editNode2->node_y - editNode1->node_y;
		err_when(vecX==0 && vecY==0);

		magn = ((xMagn=abs(vecX)) > (yMagn=abs(vecY))) ? xMagn : yMagn;
		if(xMagn)
		{
			vecX /= xMagn;
			vecX *= moveScale;
		}
		if(yMagn)
		{
			vecY /= yMagn;
			vecY *= moveScale;
		}
		err_when(abs(vecX)>moveScale && abs(vecY)>moveScale);

		#ifdef DEBUG
			debugLoop2 = 0;
		#endif
		//------------- check each location bewteen editNode1 and editNode2 -------------//
		for(j=0; j<magn; j+=moveScale)
		{
			#ifdef DEBUG
			err_when(++debugLoop2>10000);
			#endif

			checkXLoc += vecX;
			checkYLoc += vecY;

			if(checkXLoc>=xLoc1 && checkXLoc<=xLoc2 && checkYLoc>=yLoc1 && checkYLoc<=yLoc2)
			{
				found++;
				break;
			}
		}

		//-------------------------------------------------------------------------------//
		// a path is found, then set unit's parameters for its movement
		//-------------------------------------------------------------------------------//
		if(found)
		{
			editNode2->node_x = checkXLoc;
			editNode2->node_y = checkYLoc;
			
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
			move_to_x_loc = checkXLoc;
			move_to_y_loc = checkYLoc;
			break;
		}
		else
			pathDist += magn;
	}

	return found;
}
//----------- End of function Unit::edit_path_to_surround -----------//


//--------- Begin of function Unit::is_in_surrounding ---------//
// Test whether the location (checkXLoc, checkYLoc) is in the surrounding of an object
//
// <int> checkXLoc, checkYLoc		- location to check
// <int> width							- the width of the caller
// <int> ObjectXLoc, objectYLoc	- object location
// <int> objectWidth					- object width
// <int>	objectHeight				- object height
//
// Note: assume the width and the height of the caller is equal
// return 1 if in object surrounding
// return 0 otherwise
//
int Unit::is_in_surrounding(int checkXLoc, int checkYLoc, int width, int objectXLoc, int objectYLoc,
									 int objectWidth, int objectHeight)
{
	switch(move_step_magn())
	{
		case 1:
			if(checkXLoc>=objectXLoc-width && checkXLoc<=objectXLoc+objectWidth &&
				checkYLoc>=objectYLoc-width && checkYLoc<=objectYLoc+objectHeight)
				return 1;
			break;

		case 2:
			if(checkXLoc>=objectXLoc-width-1 && checkXLoc<=objectXLoc+objectWidth+1 &&
				checkYLoc>=objectYLoc-width-1 && checkYLoc<=objectYLoc+objectHeight+1)
				return 1;
			break;

		default: err_here();
					break;
	}

	return 0;
}
//----------- End of function Unit::is_in_surrounding -----------//


//--------- Begin of function Unit::process_build_firm ---------//
// process action of building firms
//
void Unit::process_build_firm()
{
	err_when(sprite_info->loc_width>1 || sprite_info->loc_height>1);
	err_when(action_x_loc<0 || action_x_loc>=MAX_WORLD_X_LOC || action_y_loc<0 || action_y_loc>=MAX_WORLD_Y_LOC);

	err_when(action_x_loc<0 || action_x_loc>=MAX_WORLD_X_LOC);
	err_when(action_y_loc<0 || action_y_loc>=MAX_WORLD_Y_LOC);

	if( cur_action == SPRITE_IDLE )  // the unit is at the build location now
	{
		// **BUGHERE, the unit shouldn't be hidden when building structures
		// otherwise, it's cargo_recno will be conflict with the structure's
		// cargo_recno

		int succeedFlag=0;
		int shouldProceed = 1;

		if( cur_x_loc()==move_to_x_loc && cur_y_loc()==move_to_y_loc )
		{
			FirmInfo *firmInfo = firm_res[action_para];
			int width = firmInfo->loc_width;
			int height = firmInfo->loc_height;

			//---------------------------------------------------------//
			// check whether the unit in the building surrounding
			//---------------------------------------------------------//
			if(!is_in_surrounding(move_to_x_loc, move_to_y_loc, sprite_info->loc_width, action_x_loc, action_y_loc, width, height))
			{
				//---------- not in the building surrounding ---------//
				return;
			}

			//---------------------------------------------------------//
			// the unit in the firm surrounding
			//---------------------------------------------------------//
			Nation* nationPtr;

			if(nation_recno)
			{
				nationPtr = nation_array[nation_recno];

				if(nationPtr->cash < firmInfo->setup_cost)
					shouldProceed = 0; // out of cash
			}
			else
				nationPtr = NULL;

			//---------------------------------------------------------//
			// check whether the firm can be built in the specified location
			//---------------------------------------------------------//
			if( shouldProceed && world.can_build_firm(action_x_loc, action_y_loc, action_para, sprite_recno) &&
				 firm_res[action_para]->can_build(sprite_recno) )
			{
				int aiUnit			= ai_unit;
				int actionXLoc		= action_x_loc;
				int actionYLoc		= action_y_loc;
				short unitRecno	= sprite_recno;

				//---------------------------------------------------------------------------//
				// if unit inside the firm location, deinit the unit to free the space for
				// building firm
				//---------------------------------------------------------------------------//
				if(move_to_x_loc>=action_x_loc && move_to_x_loc<action_x_loc+width &&
					move_to_y_loc>=action_y_loc && move_to_y_loc<action_y_loc+height)
					deinit_sprite(0); // 0-if the unit is currently selected, deactivate it.

				if( firm_array.build_firm(action_x_loc, action_y_loc, nation_recno,
												action_para, sprite_info->sprite_code, sprite_recno) ) // action_para = firm id.
				{
					//--------- able to build the firm --------//

					reset_action_para2();
					succeedFlag = 1;
				}
			}
		}

		//----- call action finished/failure -----//

		if( ai_action_id && nation_recno )
		{
			if( succeedFlag )
				nation_array[nation_recno]->action_finished(ai_action_id, sprite_recno);
			else
				nation_array[nation_recno]->action_failure(ai_action_id, sprite_recno);
		}

		//---------------------------------------//

		reset_action_para();
	}
}
//----------- End of function Unit::process_build_firm -----------//


//--------- Begin of function Unit::process_assign ---------//
// process action of assigning
//
void Unit::process_assign()
{
	err_when(sprite_info->loc_width>1 || sprite_info->loc_height>1);

	if(cur_action!=SPRITE_IDLE)
	{
		//------------------------------------------------------------------//
		// change units' action if the firm/town/unit assign to has been deleted
		// or has changed its nation
		//------------------------------------------------------------------//
		switch(action_mode2)
		{
			case ACTION_ASSIGN_TO_FIRM:
			case ACTION_AUTO_DEFENSE_BACK_CAMP:
			case ACTION_MONSTER_DEFEND_BACK_FIRM:
					if(firm_array.is_deleted(action_para))
					{
						stop2();
						return;
					}
					else
					{
						Firm *firmPtr = firm_array[action_para];
                  //###### begin trevor 21/6 #######//
						if(firmPtr->nation_recno!=nation_recno && !firmPtr->can_assign_capture())
						{
							stop2();
							return;
						}
						//###### end trevor 21/6 #######//
					}
					break;

			case ACTION_ASSIGN_TO_TOWN:
			case ACTION_DEFEND_TOWN_BACK_TOWN:
					if(town_array.is_deleted(action_para))
					{
						stop2();
						return;
					}
					else if(town_array[action_para]->nation_recno!=nation_recno)
					{
						stop2();
						return;
					}
					break;

			case ACTION_ASSIGN_TO_VEHICLE:
					if(unit_array.is_deleted(action_para))
					{
						stop2();
						return;
					}
					else if(unit_array[action_para]->nation_recno!=nation_recno)
					{
						stop2();
						return;
					}
					break;

			default:	err_here();
						break;
		}
	}
	else //--------------- unit is idle -----------------//
	{
		if( cur_x_loc()==move_to_x_loc && cur_y_loc()==move_to_y_loc )
		{
			//----- first check if there is firm in the given location ------//
			Location* locPtr = world.get_loc( action_x_loc, action_y_loc );

			if( locPtr->is_firm() && locPtr->firm_recno() == action_para )
			{
				//---------------- a firm on the location -----------------//
				Firm* firmPtr = firm_array[action_para];
				FirmInfo *firmInfo = firm_res[firmPtr->firm_id];

				//---------- resume action if the unit has not reached the firm surrounding ----------//
				if(!is_in_surrounding(move_to_x_loc, move_to_y_loc, sprite_info->loc_width, action_x_loc, action_y_loc,
					firmInfo->loc_width, firmInfo->loc_height))
				{
					//------------ not in the surrounding -----------//
					if(action_mode!=action_mode2) // for defense mode
						set_move_to_surround(action_x_loc, action_y_loc, firmInfo->loc_width, firmInfo->loc_height, BUILDING_TYPE_FIRM_MOVE_TO);
					return;
				}

				//------------ in the firm surrounding ------------//
				if(!firmPtr->under_construction)
				{
					//-------------------------------------------------------//
					// if in defense mode, update parameters in military camp
					//-------------------------------------------------------//
					if(action_mode2==ACTION_AUTO_DEFENSE_BACK_CAMP)
					{
						FirmCamp *campPtr = firmPtr->cast_to_FirmCamp();
						campPtr->update_defense_unit(sprite_recno);
					}
					
					//---------------------------------------------------------------//
					// remainder useful parameters to do reaction to Nation.
					// These parameters will be destroyed after calling assign_unit()
					//---------------------------------------------------------------//
					int	nationRecno = nation_recno;
					short	unitRecno = sprite_recno;
					int	actionXLoc = action_x_loc;
					int	actionYLoc = action_y_loc;
					uint16_t	aiActionId = ai_action_id;
					char	aiUnit = ai_unit;

					reset_action_para2();

					firmPtr->assign_unit(sprite_recno);

					//----------------------------------------------------------//
					// firm_array[]->assign_unit() must be done first.  Then a
					// town will be created and the reaction to build other firms
					// requires the location of the town.
					//----------------------------------------------------------//

					if(aiActionId)
						nation_array[nationRecno]->action_finished(aiActionId, unitRecno);

					if( unit_array.is_deleted(unitRecno) )
						return;

					//--- else the firm is full, the unit's skill level is lower than those in firm, or no space to create town ---//
				}
				else
				{
					//---------- change the builder ------------//
					if(ai_unit && firmPtr->under_construction)
						return; // not allow AI to change firm builder

					reset_action_para2();
					if(skill.get_skill(SKILL_CONSTRUCTION) || skill.get_skill(firmPtr->firm_skill_id))
						firmPtr->set_builder(sprite_recno);
				}

				//------------ update unit_array's selected parameters ------------//
				reset_action_para();
				if(selected_flag)
				{
					selected_flag = 0;
					unit_array.selected_count--;
				}
			}
			else if( locPtr->is_town() && locPtr->town_recno() == action_para )
			{
				//---------------- a town on the location -----------------//
				if(!is_in_surrounding(move_to_x_loc, move_to_y_loc, sprite_info->loc_width, action_x_loc, action_y_loc,
					STD_TOWN_LOC_WIDTH, STD_TOWN_LOC_HEIGHT))
				{
					//----------- not in the surrounding ------------//
					return;
				}
				
				short actionPara = action_para;
				short spriteRecno = sprite_recno;

				if(ai_action_id)
					nation_array[nation_recno]->action_finished(ai_action_id, sprite_recno);

				//------------ update unit_array's selected parameters ------------//
				reset_action_para2();
				reset_action_para();
				if(selected_flag)
				{
					selected_flag = 0;
					unit_array.selected_count--;
				}

				//-------------- assign the unit to the town -----------------//
				town_array[actionPara]->assign_unit(spriteRecno);
			}

			//####### begin trevor 18/8 #########// the following code was called wrongly and causing bug
/*
         //------ embarking a ground vehicle/animal ------//

			else if(locPtr->has_unit(UNIT_LAND) && locPtr->unit_recno(UNIT_LAND) == action_para)
			{
				reset_action_para2();
				reset_action_para();
				if(selected_flag)
				{
					selected_flag = 0;
					unit_array.selected_count--;
				}

				embark(action_para);
			}
*/
			//####### end trevor 18/8 #########//

			//------ embarking a sea vehicle/animal ------//

			else if(locPtr->has_unit(UNIT_SEA) && locPtr->unit_recno(UNIT_SEA) == action_para)
			{
				//------------ update unit_array's selected parameters ------------//
				reset_action_para2();
				reset_action_para();
				if(selected_flag)
				{
					selected_flag = 0;
					unit_array.selected_count--;
				}

				//----------------- load the unit to the marine -----------------//
				((UnitMarine*)unit_array[action_para])->load_unit(sprite_recno);
			}
			else
			{
				//------------------------------------------------------------------//
				// abort actions for ai_unit since the target location has nothing
				//------------------------------------------------------------------//

				if( ai_action_id )
					nation_array[nation_recno]->action_failure(ai_action_id, sprite_recno);
			}
		}

		//-***** don't place codes here as unit may be removed above *****-//
		//reset_action_para();
		//selected_flag = 0;
	}
}
//----------- End of function Unit::process_assign -----------//


//--------- Begin of function Unit::process_burn ---------//
// process action of burning to a specified location
//
void Unit::process_burn()
{
	err_when(sprite_info->loc_width>1 || sprite_info->loc_height>1);

	if( cur_action == SPRITE_IDLE )  // the unit is at the build location now
	{
		if( next_x_loc()==action_x_loc && next_y_loc()==action_y_loc )
		{
			reset_action_para2();
			set_dir(move_to_x_loc, move_to_y_loc, action_x_loc, action_y_loc);
			world.setup_fire( action_x_loc, action_y_loc );
		}

		reset_action_para();
	}
}
//----------- End of function Unit::process_burn -----------//


//--------- Begin of function Unit::process_settle ---------//
// process action of settling to an existing town
//
void Unit::process_settle()
{
	err_when(sprite_info->loc_width>1 || sprite_info->loc_height>1);

	if( cur_action == SPRITE_IDLE )  // the unit is at the build location now
	{
		reset_path();

		if( cur_x_loc()==move_to_x_loc && cur_y_loc()==move_to_y_loc )
		{
			if(!is_in_surrounding(move_to_x_loc, move_to_y_loc, sprite_info->loc_width, action_x_loc, action_y_loc,
				STD_TOWN_LOC_WIDTH, STD_TOWN_LOC_HEIGHT))
				return;

			Location* locPtr = world.get_loc(action_x_loc, action_y_loc);
			if(!locPtr->is_town())
			{
				//##### begin trevor 1/9 ########//
				int unitRecno = sprite_recno;

				reset_action_para2();
				//------------ settle the unit now -------------//
				town_array.settle(sprite_recno, action_x_loc, action_y_loc);

				if( unit_array.is_deleted(unitRecno) )
					return;

				reset_action_para();
				//##### end trevor 1/9 ########//
			}
			else if(town_array[locPtr->town_recno()]->nation_recno == nation_recno)
			{
				//---------- a town zone already exists ---------//
				assign(action_x_loc, action_y_loc);
				return;
			}
		}
		else
			reset_action_para();
	}
}
//----------- End of function Unit::process_settle -----------//


//--------- Begin of function Unit::go_cast_power ---------//
// do action of god casting
//
void Unit::go_cast_power(int castXLoc, int castYLoc, char castPowerType, char remoteAction)
{
	err_when(mobile_type!=UNIT_AIR);

	//----------------------------------------------------------------//
	// return if the unit is dead
	//----------------------------------------------------------------//
	if(hit_points<=0 || action_mode==ACTION_DIE || cur_action==SPRITE_DIE)
		return;

	if(!remoteAction && remote.is_enable() )
	{
		//------------ process multiplayer calling ---------------//
		// packet structure : <unit recno> <xLoc> <yLoc> <power type>
		short *shortPtr =(short *)remote.new_send_queue_msg(MSG_U_GOD_CAST, 4*sizeof(short) );
		shortPtr[0] = sprite_recno;
		shortPtr[1] = castXLoc;
		shortPtr[2] = castYLoc;
		shortPtr[3] = castPowerType;
		return;
	}

	UnitGod *unitGod = (UnitGod *)this;

	//----------------------------------------------------------------//
	// action_mode2: checking for equal action or idle action
	//----------------------------------------------------------------//
	if(action_mode2==ACTION_GO_CAST_POWER && action_para2==0 && action_x_loc2==castXLoc && action_y_loc2==castYLoc
		&& unitGod->cast_power_type==castPowerType)
	{
		if(cur_action!=SPRITE_IDLE)
			return;
	}
	else
	{
		//----------------------------------------------------------------//
		// action_mode2: store new order
		//----------------------------------------------------------------//
		action_mode2  = ACTION_GO_CAST_POWER;
		action_para2  = 0;
		action_x_loc2 = castXLoc;
		action_y_loc2 = castYLoc;
	}

	//----- order the sprite to stop as soon as possible -----//
	stop();	// new order

	//------------- do searching if neccessary -------------//
	if(misc.points_distance(next_x_loc(), next_y_loc(), castXLoc, castYLoc)>DO_CAST_POWER_RANGE)
		search(castXLoc, castYLoc, 1);

	//----------- set action to build the firm -----------//
	action_mode  = ACTION_GO_CAST_POWER;
	action_para  = 0;
	action_x_loc = castXLoc;
	action_y_loc = castYLoc;

	unitGod->cast_power_type = castPowerType;
	unitGod->cast_origin_x = next_x_loc();
	unitGod->cast_origin_y = next_y_loc();
	unitGod->cast_target_x = castXLoc;
	unitGod->cast_target_y = castYLoc;
}
//----------- End of function Unit::go_cast_power -----------//


//--------- Begin of function Unit::process_go_cast_power ---------//
// process action of god casting
//
void Unit::process_go_cast_power()
{
	err_when(mobile_type!=UNIT_AIR);
	err_when(action_x_loc2<0 || action_x_loc2>=MAX_WORLD_X_LOC);
	err_when(action_y_loc2<0 || action_y_loc2>=MAX_WORLD_Y_LOC);

	UnitGod *unitGod = (UnitGod *)this;
	if(cur_action==SPRITE_IDLE)
	{
		//----------------------------------------------------------------------------------------//
		// Checking condition to do casting power, Or resume action
		//----------------------------------------------------------------------------------------//
		if(misc.points_distance(cur_x_loc(), cur_y_loc(), action_x_loc2, action_y_loc2)<=DO_CAST_POWER_RANGE)
		{
			if( next_x_loc() != action_x_loc2 ||
				next_y_loc() != action_y_loc2 )
			{
				set_dir(next_x_loc(), next_y_loc(), action_x_loc2, action_y_loc2);
			}
			set_attack(); // set cur_action=sprite_attack to cast power 
			cur_frame = 1;
		}
		else
		{
			err_when(action_mode2!=ACTION_GO_CAST_POWER || action_para2);
			go_cast_power(action_x_loc2, action_y_loc2, unitGod->cast_power_type, COMMAND_AUTO);
		}
	}
	else if(cur_action==SPRITE_ATTACK)
	{
		//----------------------------------------------------------------------------------------//
		// do casting power now
		//----------------------------------------------------------------------------------------//
		AttackInfo *attackInfo = attack_info_array + cur_attack;
		if(cur_frame == attackInfo->bullet_out_frame)
		{
			// add effect
			add_close_attack_effect();

			unitGod->cast_power(action_x_loc2, action_y_loc2);
			set_remain_attack_delay();
			// stop2();
		}

		if( cur_frame == 1 && remain_attack_delay == 0 )                // last frame of delaying
			stop2();
	}
}
//----------- End of function Unit::process_go_cast_power -----------//
