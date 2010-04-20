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

//Filename    : OUNITAT3.CPP
//Description : Object Unit's decision making functions for attacking same or different type of target and reactivating
//					 idle unit that are ordered to attack
//Owner		  : Alex

#include <ALL.h>
#include <OWORLD.h>
#include <OUNIT.h>
#include <OGAME.h>

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

//--------- Begin of function Unit::move_try_to_range_attack ---------//
// return 1 if it is possible to reach a location to attack the target
// return 0 otherwise
//
// <Unit*>	targetUnit	- pointer to target unit
//
int Unit::move_try_to_range_attack(Unit* targetUnit)
{
	int curXLoc = next_x_loc();
	int curYLoc = next_y_loc();
	int targetXLoc = targetUnit->next_x_loc();
	int targetYLoc = targetUnit->next_y_loc();

	if(world.get_loc(curXLoc, curYLoc)->region_id==world.get_loc(targetXLoc, targetYLoc)->region_id)
	{
		//------------ for same region id, search now ---------------//
		if(search(targetXLoc, targetYLoc, 1, SEARCH_MODE_TO_ATTACK, action_para))
			return 1;
		else // search failure,
		{
			stop2(KEEP_DEFENSE_MODE);
			return 0;
		}
	}
	else
	{
		//--------------- different territory ------------------//
		int targetWidth = targetUnit->sprite_info->loc_width;
		int targetHeight = targetUnit->sprite_info->loc_height;
		int maxRange = max_attack_range();

		if(possible_place_for_range_attack(targetXLoc, targetYLoc, targetWidth, targetHeight, maxRange))
		{
			//---------------------------------------------------------------------------------//
			// space is found, attack target now
			//---------------------------------------------------------------------------------//
			if(move_to_range_attack(targetXLoc, targetYLoc, targetUnit->sprite_id, SEARCH_MODE_ATTACK_UNIT_BY_RANGE, maxRange))
				return 1;
			else
			{
				stop2(KEEP_DEFENSE_MODE);
				return 0;
			}
			return 1;
		}
		else
		{
			//---------------------------------------------------------------------------------//
			// unable to find location to attack the target, stop or move to the target
			//---------------------------------------------------------------------------------//
			if(action_mode2!=ACTION_AUTO_DEFENSE_ATTACK_TARGET && action_mode2!=ACTION_DEFEND_TOWN_ATTACK_TARGET &&
				action_mode2!=ACTION_MONSTER_DEFEND_ATTACK_TARGET)
				move_to(targetXLoc, targetYLoc, 1); // abort attacking, just call move_to() instead
			else
				stop2(KEEP_DEFENSE_MODE);
			return 0;
		}
	}

	return 0;
}
//----------- End of function Unit::move_try_to_range_attack -----------//


//--------- Begin of function Unit::move_to_range_attack ---------//
// search and attack target
//
// <int>		targetXLoc	-	target x location
// <int>		targetYLoc	-	target y location
// <short>	miscNo		-	is sprite_id if target is a unit
//									is firm_id if target is a firm
//									is 0 if target is a town or a wall
// <short>	searchMode	-	search mode being used
// <short>	maxRange		-	MAX attack range of this unit
//
int Unit::move_to_range_attack(int targetXLoc, int targetYLoc, short miscNo, short searchMode, short maxRange)
{
	//---------------------------------------------------------------------------------//
	// part 1, searching
	//---------------------------------------------------------------------------------//
	seek_path.set_attack_range_para(maxRange);
	search(targetXLoc, targetYLoc, 1, searchMode, miscNo);
	seek_path.reset_attack_range_para();
	//search(targetXLoc, targetYLoc, 1, searchMode, maxRange);

	if(result_node_array==NULL || result_node_count==0)
		return 0;

	//---------------------------------------------------------------------------------//
	// part 2, editing result path
	//---------------------------------------------------------------------------------//
	Location *locPtr = world.get_loc(next_x_loc(), next_y_loc());
	err_when(!locPtr);

	int regionId = locPtr->region_id;	// the region_id this unit in

	//----------------------------------------------------//
	err_when(result_node_count<2);
	ResultNode* editNode1 = result_node_array + result_node_count - 1;
	ResultNode* editNode2 = editNode1-1;
	int vecX = editNode1->node_x - editNode2->node_x;
	int vecY = editNode1->node_y - editNode2->node_y;

	if(vecX)
		vecX = ((vecX>0) ? 1 : -1)*move_step_magn();
	if(vecY)
		vecY = ((vecY>0) ? 1 : -1)*move_step_magn();

	int x = editNode1->node_x;
	int y = editNode1->node_y;
	int i, found=0, removedStep=0, preX, preY;

	for(i=result_node_count; i>1; i--)
	{
		while(x!=editNode2->node_x || y!=editNode2->node_y)
		{
			locPtr = world.get_loc(x, y);
			if(locPtr->region_id == regionId)
			{
				found = i;
				preX = x;
				preY = y;
				break;
			}
			
			x -= vecX;
			y -= vecY;
			removedStep++;
		}

		if(found)
			break;

		editNode1 = editNode2;
		editNode2--;
		
		vecX = editNode1->node_x - editNode2->node_x;
		vecY = editNode1->node_y - editNode2->node_y;
		if(vecX)
			vecX = ((vecX>0) ? 1 : -1)*move_step_magn();
		if(vecY)
			vecY = ((vecY>0) ? 1 : -1)*move_step_magn();
		
		x = editNode1->node_x;
		y = editNode1->node_y;
	}

	//---------------------------------------------------------------------------//
	// update unit parameters
	//---------------------------------------------------------------------------//
	if(found)
	{
		result_node_count = found;
		ResultNode* lastNode = result_node_array + result_node_count - 1;
		int goX = go_x>>ZOOM_X_SHIFT_COUNT;
		int goY = go_y>>ZOOM_Y_SHIFT_COUNT;

		//---------------------------------------------------------------------//
		// note: build?Loc-1, build?Loc+width, build?Loc+height may <0 or
		//			>MAX_WORLD_?_LOC.  To prevent errors from occuring, goX, goY
		//			must not be outside the map boundary
		//---------------------------------------------------------------------//
		if(goX==editNode1->node_x && goY==editNode1->node_y)
		{
			go_x = preX*ZOOM_LOC_WIDTH;
			go_y = preY*ZOOM_LOC_HEIGHT;
		}
		else if(result_node_count==2)
		{
			int magnCG = m.points_distance(cur_x, cur_y, go_x, go_y);
			int magnNG = m.points_distance(next_x, next_y, go_x, go_y);
			err_when(magnCG==0 && magnNG==0);

			if(magnCG && magnNG)
			{
				//---------- lie on the same line -----------//
				if( (go_x-cur_x)/magnCG==(go_x-next_x)/magnNG && (go_y-cur_y)/magnCG==(go_y-next_y)/magnNG )
				{
					go_x = preX*ZOOM_LOC_WIDTH;
					go_y = preY*ZOOM_LOC_HEIGHT;
				}
			}
		}

		lastNode->node_x = preX;
		lastNode->node_y = preY;
		move_to_x_loc = lastNode->node_x;
		move_to_y_loc = lastNode->node_y;
		
		result_path_dist -= (removedStep)*move_step_magn();

		err_when((cur_x!=next_x || cur_y!=next_y) &&	// is not blocked
					(check_unit_dir1=get_dir(cur_x, cur_y, next_x, next_y))!=(check_unit_dir2=get_dir(cur_x, cur_y, go_x, go_y)));
	}

	return found;
}
//----------- End of function Unit::move_to_range_attack -----------//


//--------- Begin of function Unit::can_attack_different_type_target ---------//
// return 1 if able to use range_attack
// return 0 otherwise
//
int Unit::can_attack_different_target_type()
{
	int maxRange = max_attack_range();
	if(mobile_type==UNIT_LAND && !maxRange)
		return 0; // unable to do range attack or cannot attack
	
	if(maxRange>1)
		return maxRange;
	else
		return 0;
}
//----------- End of function Unit::can_attack_different_type_target -----------//


//--------- Begin of function Unit::possible_place_for_range_attack ---------//
// check whether there is any place for this unit to attack the target
//
// <int> targetXLoc		- target x location
// <int> targetYLoc		- target y location
// <int> targetWidth		- target width
// <int> targetHeight	- target height
// <int> maxRange			- MAX attack range of this unit
//
// return 1 if place found
// return 0 otherwise
//
int Unit::possible_place_for_range_attack(int targetXLoc, int targetYLoc, int targetWidth, int targetHeight, int maxRange)
{
	err_when(targetXLoc<0 || targetXLoc>=MAX_WORLD_X_LOC || targetYLoc<0 || targetYLoc>=MAX_WORLD_Y_LOC);
	err_when(maxRange==0);

	if(mobile_type==UNIT_AIR)
		return 1; // air unit can reach any region
	
	int curXLoc = next_x_loc();
	int curYLoc = next_y_loc();

	if(abs(curXLoc-targetXLoc)<=maxRange && abs(curYLoc-targetYLoc)<=maxRange) // inside the attack range
		return 1;

	//----------------- init parameters -----------------//
	Location *locPtr = world.get_loc(curXLoc, curYLoc);
	int regionId = locPtr->region_id;
	int xLoc1 = MAX(targetXLoc-maxRange, 0);
	int yLoc1 = MAX(targetYLoc-maxRange, 0);
	int xLoc2 = MIN(targetXLoc+targetWidth-1+maxRange, MAX_WORLD_X_LOC-1);
	int yLoc2 = MIN(targetYLoc+targetHeight-1+maxRange, MAX_WORLD_Y_LOC-1);
	int checkXLoc, checkYLoc;

	//--------- do adjustment for UNIT_SEA and UNIT_AIR ---------//
	if(mobile_type!=UNIT_LAND)
	{
		if(xLoc1%2)
			xLoc1++;
		if(yLoc1%2)
			yLoc1++;
		if(xLoc2%2)
			xLoc2--;
		if(yLoc2%2)
			yLoc2--;
	}

	//-------- checking for surrounding location ----------//
	switch(mobile_type)
	{
		case UNIT_LAND:
				for(checkXLoc=xLoc1; checkXLoc<=xLoc2; checkXLoc++)
				{
					locPtr = world.get_loc(checkXLoc, yLoc1);
					if(locPtr->region_id==regionId && locPtr->is_accessible(mobile_type))
						return 1;

					locPtr = world.get_loc(checkXLoc, yLoc2);
					if(locPtr->region_id==regionId && locPtr->is_accessible(mobile_type))
						return 1;
				}

				for(checkYLoc=yLoc1+1; checkYLoc<yLoc2; checkYLoc++)
				{
					locPtr = world.get_loc(xLoc1, checkYLoc);
					if(locPtr->region_id==regionId && locPtr->is_accessible(mobile_type))
						return 1;

					locPtr = world.get_loc(xLoc2, checkYLoc);
					if(locPtr->region_id==regionId && locPtr->is_accessible(mobile_type))
						return 1;
				}
				break;

		case UNIT_SEA:
				for(checkXLoc=xLoc1; checkXLoc<=xLoc2; checkXLoc++)
				{
					if(checkXLoc%2==0 && yLoc1%2==0)
					{
						locPtr = world.get_loc(checkXLoc, yLoc1);
						if(locPtr->region_id==regionId && locPtr->is_accessible(mobile_type))
							return 1;
					}

					if(checkXLoc%2==0 && yLoc2%2==0)
					{
						locPtr = world.get_loc(checkXLoc, yLoc2);
						if(locPtr->region_id==regionId && locPtr->is_accessible(mobile_type))
							return 1;
					}
				}

				for(checkYLoc=yLoc1+1; checkYLoc<yLoc2; checkYLoc++)
				{
					if(xLoc1%2==0 && checkYLoc%2==0)
					{
						locPtr = world.get_loc(xLoc1, checkYLoc);
						if(locPtr->region_id==regionId && locPtr->is_accessible(mobile_type))
							return 1;
					}

					if(xLoc2%2==0 && checkYLoc%2==0)
					{
						locPtr = world.get_loc(xLoc2, checkYLoc);
						if(locPtr->region_id==regionId && locPtr->is_accessible(mobile_type))
							return 1;
					}
				}
				break;

		case UNIT_AIR:
				for(checkXLoc=xLoc1; checkXLoc<=xLoc2; checkXLoc++)
				{
					if(checkXLoc%2==0 && yLoc1%2==0)
					{
						locPtr = world.get_loc(checkXLoc, yLoc1);
						if(locPtr->is_accessible(mobile_type))
							return 1;
					}

					if(checkXLoc%2==0 && yLoc2%2==0)
					{
						locPtr = world.get_loc(checkXLoc, yLoc2);
						if(locPtr->is_accessible(mobile_type))
							return 1;
					}
				}

				for(checkYLoc=yLoc1+1; checkYLoc<yLoc2; checkYLoc++)
				{
					if(xLoc1%2==0 && checkYLoc%2==0)
					{
						locPtr = world.get_loc(xLoc1, checkYLoc);
						if(locPtr->is_accessible(mobile_type))
							return 1;
					}

					if(xLoc2%2==0 && checkYLoc%2==0)
					{
						locPtr = world.get_loc(xLoc2, checkYLoc);
						if(locPtr->is_accessible(mobile_type))
							return 1;
					}
				}
				break;

		default: err_here();
					break;
	}

	return 0;
}
//----------- End of function Unit::possible_place_for_range_attack -----------//


//=====================================================================================//
//--------- Begin of function Unit::space_for_attack ---------//
// check whether there is any place for the unit to attack target.
//
// <int>		targetXLoc			-	target x location
// <int>		targetYLoc			-	target y location
// <char>	targetMobileType	-	target mobile type
// <int>		targetWidth			-	target width
// <int>		targetHeight		-	target height
//
int Unit::space_for_attack(int targetXLoc, int targetYLoc, char targetMobileType, int targetWidth, int targetHeight)
{
	if(mobile_type==UNIT_LAND && targetMobileType==UNIT_LAND)
		return space_around_target(targetXLoc, targetYLoc, targetWidth, targetHeight);

	if((mobile_type==UNIT_SEA && targetMobileType==UNIT_SEA) ||
		(mobile_type==UNIT_AIR && targetMobileType==UNIT_AIR))
		return space_around_target_ver2(targetXLoc, targetYLoc, targetWidth, targetHeight);

	//-------------------------------------------------------------------------//
	// mobile_type is differet from that of target unit
	//-------------------------------------------------------------------------//
	Location *locPtr = world.get_loc(next_x_loc(), next_y_loc());
	if(mobile_type==UNIT_LAND && targetMobileType==UNIT_SEA &&
		!can_attack_different_target_type() &&
		ship_surr_has_free_land(targetXLoc, targetYLoc, locPtr->region_id))
		return 1;

	int maxRange = max_attack_range();
	if(maxRange==1)
		return 0;

	if(free_space_for_range_attack(targetXLoc, targetYLoc, targetWidth, targetHeight, targetMobileType, maxRange))
		return 1;

	return 0;
}
//----------- End of function Unit::space_for_attack -----------//


//--------- Begin of function Unit::space_around_target ---------//
// check the surroundung location around a square, and the result is
// stored in the blocked_edge[] by bit
//
// <int>	squareXLoc	-	upper left x location of target
// <int>	squareYLoc	-	upper left y location of target
// <int> width			-	target width
// <int> height		-	target height
//
// return 1 if the surrounding location that can_move is not equal to
//				the result in the blocked_edge stored previously.
// return 0 otherwise (i.e. all location situation is same as before)
//
int Unit::space_around_target(int squareXLoc, int squareYLoc, int width, int height)
{
	err_when(width<=0 || height<=0);
	//				edge 1
	//				1 1 4
	// edge 2	2 x 4		edge 4
	//				2 3 3
	//				edge3

	Location *locPtr;
	Unit *unitPtr;
	char sum, locWeight;
	int testXLoc, testYLoc, i, equal=1;

	//------------------ top edge ---------------//
	sum = 0;
	if((testYLoc=squareYLoc-1) >= 0)
	{
		if(squareXLoc>=1) // have upper left corner
		{
			i=-1;
			locWeight = 1;
		}
		else
		{
			i = 0;
			locWeight = 2;
		}
		
		for(; i<width; i++, locWeight<<=1)
		{
			locPtr = world.get_loc(squareXLoc+i, testYLoc);
			if(locPtr->can_move(mobile_type))
				sum ^= locWeight;
			else if(locPtr->has_unit(mobile_type))
			{
				unitPtr = unit_array[locPtr->unit_recno(mobile_type)];
				if(unitPtr->cur_action!=SPRITE_ATTACK)
					sum ^= locWeight;
			}
		}
	}

	if(blocked_edge[0]!=sum)
	{
		blocked_edge[0] = sum;
		equal = 0;
	}

	//----------------- left edge -----------------//
	sum = 0;
	if((testXLoc=squareXLoc-1) >= 0)
	{
		if(squareYLoc+height<=MAX_WORLD_Y_LOC-1) // have lower left corner
		{
			i = height;
			locWeight = 1;
		}
		else
		{
			i = height - 1;
			locWeight = 2;
		}

		for(; i>=0; i--, locWeight<<=1)
		{
			locPtr = world.get_loc(testXLoc, squareYLoc+i);
			if(locPtr->can_move(mobile_type))
				sum ^= locWeight;
			else if(locPtr->has_unit(mobile_type))
			{
				unitPtr = unit_array[locPtr->unit_recno(mobile_type)];
				if(unitPtr->cur_action!=SPRITE_ATTACK)
					sum ^= locWeight;
			}
		}
	}

	if(blocked_edge[1]!=sum)
	{
		blocked_edge[1] = sum;
		equal = 0;
	}

	//------------------- bottom edge ------------------//
	sum = 0;
	if((testYLoc=squareYLoc+height) <= MAX_WORLD_Y_LOC-1)
	{
		if(squareXLoc+width<=MAX_WORLD_X_LOC-1) // have lower right corner
		{
			i = width;
			locWeight = 1;
		}
		else
		{
			i = width - 1;
			locWeight = 2;
		}

		for(; i>=0; i--, locWeight<<=1)
		{
			locPtr = world.get_loc(squareXLoc+i, testYLoc);
			if(locPtr->can_move(mobile_type))
				sum ^= locWeight;
			else if(locPtr->has_unit(mobile_type))
			{
				unitPtr = unit_array[locPtr->unit_recno(mobile_type)];
				if(unitPtr->cur_action!=SPRITE_ATTACK)
					sum ^= locWeight;
			}
		}
	}

	if(blocked_edge[2]!=sum)
	{
		blocked_edge[2] = sum;
		equal = 0;
	}

	//---------------------- right edge ----------------------//
	sum = 0;
	if((testXLoc=squareXLoc+width) <= MAX_WORLD_X_LOC-1)
	{
		if(squareYLoc>=1) // have upper right corner
		{
			i = -1;
			locWeight = 1;
		}
		else
		{
			i = 0;
			locWeight = 2;
		}

		for(; i<height; i++, locWeight<<=1)
		{
			locPtr = world.get_loc(testXLoc, squareYLoc+i);
			if(locPtr->can_move(mobile_type))
				sum ^= locWeight;
			else if(locPtr->has_unit(mobile_type))
			{
				unitPtr = unit_array[locPtr->unit_recno(mobile_type)];
				if(unitPtr->cur_action!=SPRITE_ATTACK)
					sum ^= locWeight;
			}
		}
	}

	if(blocked_edge[3]!=sum)
	{
		blocked_edge[3] = sum;
		equal = 0;
	}

	return !equal;
}
//----------- End of function Unit::space_around_target -----------//


//--------- Begin of function Unit::space_around_target_ver2 ---------//
// similar function as space_around_target()
// This version is for sea unit and air unit only
//
int Unit::space_around_target_ver2(int targetXLoc, int targetYLoc, int targetWidth, int targetHeight)
{
	err_when(targetWidth<=0 || targetHeight<=0);

	Location *locPtr;
	Unit *unitPtr;
	char sum, locWeight;
	int xLoc1, yLoc1, xLoc2, yLoc2;
	int i, equal=1;
	//int testXLoc, testYLoc, 

	xLoc1 = targetXLoc%2 ? targetXLoc-1 : targetXLoc-2;
	yLoc1 = targetYLoc%2 ? targetYLoc-1 : targetYLoc-2;
	xLoc2 = (targetXLoc+targetWidth-1)%2 ? targetXLoc+targetWidth : targetXLoc+targetWidth+1;
	yLoc2 = (targetYLoc+targetHeight-1)%2 ? targetYLoc+targetHeight : targetYLoc+targetHeight+1;

	//------------------------ top edge ------------------------//
	sum = 0;
	if(yLoc1>=0)
	{
		if(xLoc1>=0)
		{
			i = xLoc1;
			locWeight = 1;
		}
		else
		{
			i = xLoc1 + 2;
			err_when(i<0);
			locWeight = 2;
		}

		for(; i<=xLoc2; i+=2, locWeight<<=1)
		{
			locPtr = world.get_loc(i, yLoc1);
			if(locPtr->can_move(mobile_type))
				sum ^= locWeight;
			else if(locPtr->has_unit(mobile_type))
			{
				unitPtr = unit_array[locPtr->unit_recno(mobile_type)];
				if(unitPtr->cur_action!=SPRITE_ATTACK)
					sum ^= locWeight;
			}
		}
	}
	
	if(blocked_edge[0]!=sum)
	{
		blocked_edge[0] = sum;
		equal = 0;
	}

	//---------------------- left edge -----------------------//
	sum = 0;
	if(xLoc1>=0)
	{
		if(yLoc2<=MAX_WORLD_Y_LOC-1)
		{
			i = yLoc2;
			locWeight = 1;
		}
		else
		{
			i = yLoc2-2;
			err_when(i>=MAX_WORLD_Y_LOC);
			locWeight = 2;
		}

		for(; i>yLoc1; i-=2, locWeight<<=1)
		{
			locPtr = world.get_loc(xLoc1, i);
			if(locPtr->can_move(mobile_type))
				sum ^= locWeight;
			else if(locPtr->has_unit(mobile_type))
			{
				unitPtr = unit_array[locPtr->unit_recno(mobile_type)];
				if(unitPtr->cur_action!=SPRITE_ATTACK)
					sum ^= locWeight;
			}
		}
	}

	if(blocked_edge[1]!=sum)
	{
		blocked_edge[1] = sum;
		equal = 0;
	}

	//----------------------- bottom edge ---------------------------//
	sum = 0;
	if(yLoc2<=MAX_WORLD_Y_LOC-1)
	{
		if(xLoc2<=MAX_WORLD_X_LOC-1)
		{
			i = xLoc2;
			locWeight = 1;
		}
		else
		{
			i = xLoc2-2;
			err_when(i>=MAX_WORLD_X_LOC);
			locWeight = 2;
		}

		for(; i>xLoc1; i-=2, locWeight<<=1)
		{
			locPtr = world.get_loc(i, yLoc2);
			if(locPtr->can_move(mobile_type))
				sum ^= locWeight;
			else if(locPtr->has_unit(mobile_type))
			{
				unitPtr = unit_array[locPtr->unit_recno(mobile_type)];
				if(unitPtr->cur_action!=SPRITE_ATTACK)
					sum ^= locWeight;
			}
		}
	}

	if(blocked_edge[2]!=sum)
	{
		blocked_edge[2] = sum;
		equal = 0;
	}

	//---------------------- right edge ------------------------//
	sum = 0;
	if(xLoc2<=MAX_WORLD_X_LOC-1)
	{
		if(yLoc1>=0)
		{
			i = yLoc1;
			locWeight = 1;
		}
		else
		{
			i = yLoc1+2;
			err_when(i<0);
			locWeight = 2;
		}

		for(; i<yLoc2; i+=2, locWeight<<=1)
		{
			locPtr = world.get_loc(xLoc2, i);
			if(locPtr->can_move(mobile_type))
				sum ^= locWeight;
			else if(locPtr->has_unit(mobile_type))
			{
				unitPtr = unit_array[locPtr->unit_recno(mobile_type)];
				if(unitPtr->cur_action!=SPRITE_ATTACK)
					sum ^= locWeight;
			}
		}
	}

	if(blocked_edge[3]!=sum)
	{
		blocked_edge[3] = sum;
		equal = 0;
	}

	return !equal;
}
//----------- End of function Unit::space_around_target_ver2 -----------//


//--------- Begin of function Unit::ship_surr_has_free_land ---------//
// check surrounding place for close attack by land units
//
// <int>		targetXLoc	- target x loc
// <int>		targetYLoc	- target y loc
// <UCHAR>	regionId		- region id
//
// return 1 if there is space for the land unit to move to ship surrounding for close attack
// return 0 otherwise
//
int Unit::ship_surr_has_free_land(int targetXLoc, int targetYLoc, UCHAR regionId)
{
	err_when(mobile_type!=UNIT_LAND);
	Location *locPtr;
	int xShift, yShift, checkXLoc, checkYLoc;

	for(int i=2; i<9; i++)
	{
		m.cal_move_around_a_point(i, 3, 3, xShift, yShift);
		checkXLoc = targetXLoc+xShift;
		checkYLoc = targetYLoc+yShift;

		if(checkXLoc<0 || checkXLoc>=MAX_WORLD_X_LOC || checkYLoc<0 || checkYLoc>=MAX_WORLD_Y_LOC)
			continue;

		locPtr = world.get_loc(checkXLoc, checkYLoc);
		if(locPtr->region_id==regionId && locPtr->can_move(mobile_type))
			return 1;
	}

	return 0;
}
//----------- End of function Unit::ship_surr_has_free_land -----------//


//--------- Begin of function Unit::free_space_for_range_attack ---------//
// similar to possible_place_for_range_attack() but checking can_move() rather than is_accessible()
//
int Unit::free_space_for_range_attack(int targetXLoc, int targetYLoc, int targetWidth, int targetHeight, int targetMobileType, int maxRange)
{
	err_when(targetXLoc<0 || targetXLoc>=MAX_WORLD_X_LOC || targetYLoc<0 || targetYLoc>=MAX_WORLD_Y_LOC);
	err_when(maxRange==0);

	//if(mobile_type==UNIT_AIR)
	//	return 1; // air unit can reach any region
	
	int curXLoc = next_x_loc();
	int curYLoc = next_y_loc();

	if(abs(curXLoc-targetXLoc)<=maxRange && abs(curYLoc-targetYLoc)<=maxRange) // inside the attack range
		return 1;

	Location *locPtr = world.get_loc(curXLoc, curYLoc);
	int regionId = locPtr->region_id;
	int xLoc1 = MAX(targetXLoc-maxRange, 0);
	int yLoc1 = MAX(targetYLoc-maxRange, 0);
	int xLoc2 = MIN(targetXLoc+targetWidth-1+maxRange, MAX_WORLD_X_LOC-1);
	int yLoc2 = MIN(targetYLoc+targetHeight-1+maxRange, MAX_WORLD_Y_LOC-1);
	int checkXLoc, checkYLoc;

	//--------- do adjustment for UNIT_SEA and UNIT_AIR ---------//
	if(mobile_type!=UNIT_LAND)
	{
		if(xLoc1%2)
			xLoc1++;
		if(yLoc1%2)
			yLoc1++;
		if(xLoc2%2)
			xLoc2--;
		if(yLoc2%2)
			yLoc2--;
	}

	//-------- checking for surrounding location ----------//
	switch(mobile_type)
	{
		case UNIT_LAND:
				for(checkXLoc=xLoc1; checkXLoc<=xLoc2; checkXLoc++)
				{
					locPtr = world.get_loc(checkXLoc, yLoc1);
					if(locPtr->region_id==regionId && locPtr->can_move(mobile_type))
						return 1;

					locPtr = world.get_loc(checkXLoc, yLoc2);
					if(locPtr->region_id==regionId && locPtr->can_move(mobile_type))
						return 1;
				}

				for(checkYLoc=yLoc1+1; checkYLoc<yLoc2; checkYLoc++)
				{
					locPtr = world.get_loc(xLoc1, checkYLoc);
					if(locPtr->region_id==regionId && locPtr->can_move(mobile_type))
						return 1;

					locPtr = world.get_loc(xLoc2, checkYLoc);
					if(locPtr->region_id==regionId && locPtr->can_move(mobile_type))
						return 1;
				}
				break;

		case UNIT_SEA:
				for(checkXLoc=xLoc1; checkXLoc<=xLoc2; checkXLoc++)
				{
					if(checkXLoc%2==0 && yLoc1%2==0)
					{
						locPtr = world.get_loc(checkXLoc, yLoc1);
						if(locPtr->region_id==regionId && locPtr->can_move(mobile_type))
							return 1;
					}

					if(checkXLoc%2==0 && yLoc2%2==0)
					{
						locPtr = world.get_loc(checkXLoc, yLoc2);
						if(locPtr->region_id==regionId && locPtr->can_move(mobile_type))
							return 1;
					}
				}

				for(checkYLoc=yLoc1+1; checkYLoc<yLoc2; checkYLoc++)
				{
					if(xLoc1%2==0 && checkYLoc%2==0)
					{
						locPtr = world.get_loc(xLoc1, checkYLoc);
						if(locPtr->region_id==regionId && locPtr->can_move(mobile_type))
							return 1;
					}

					if(xLoc2%2==0 && checkYLoc%2==0)
					{
						locPtr = world.get_loc(xLoc2, checkYLoc);
						if(locPtr->region_id==regionId && locPtr->can_move(mobile_type))
							return 1;
					}
				}
				break;

		case UNIT_AIR:
				for(checkXLoc=xLoc1; checkXLoc<=xLoc2; checkXLoc++)
				{
					if(checkXLoc%2==0 && yLoc1%2==0)
					{
						locPtr = world.get_loc(checkXLoc, yLoc1);
						if(locPtr->can_move(mobile_type))
							return 1;
					}

					if(checkXLoc%2==0 && yLoc2%2==0)
					{
						locPtr = world.get_loc(checkXLoc, yLoc2);
						if(locPtr->can_move(mobile_type))
							return 1;
					}
				}

				for(checkYLoc=yLoc1+1; checkYLoc<yLoc2; checkYLoc++)
				{
					if(xLoc1%2==0 && checkYLoc%2==0)
					{
						locPtr = world.get_loc(xLoc1, checkYLoc);
						if(locPtr->can_move(mobile_type))
							return 1;
					}

					if(xLoc2%2==0 && checkYLoc%2==0)
					{
						locPtr = world.get_loc(xLoc2, checkYLoc);
						if(locPtr->can_move(mobile_type))
							return 1;
					}
				}
				break;

		default:	err_here();
					break;
	}

	return 0;
}
//----------- End of function Unit::free_space_for_range_attack -----------//