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

//Filename    : OSPATH.CPP
//Description : Object SeekPath
//Owner		  :  

#include <math.h>
#include <stdlib.h>
#include <ALL.h>
#include <OWORLD.h>
#include <OSPATH.h>
#include <OFIRM.h>
#include <OTOWN.h>
#include <OUNIT.h>
#include <OSYS.h>
#include <ONATION.h>

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
#undef DEBUG
#endif

#define ZOOM_LOC_HALF_WIDTH	ZOOM_LOC_WIDTH/2
#define ZOOM_LOC_HALF_HEIGHT	ZOOM_LOC_HEIGHT/2

//----------- Define static variables -----------//

static Location*  world_loc_matrix;
static int		   cur_stack_pos=0;
static Node* 	   stack_array[MAX_STACK_NUM];
static DWORD	   group_id;
static short	   search_mode;
static char	  	   mobile_type;
static char			seek_nation_recno;
static int			attack_range;	// used in search_mode = SEARCH_MODE_ATTACK_UNIT_BY_RANGE
static short		target_recno;	// used in search_mode = SEARCH_MODE_TO_ATTACK or SEARCH_MODE_TO_VEHICLE, get from miscNo
static UCHAR		region_id;		// used in search_mode = SEARCH_MODE_TO_LAND_FOR_SHIP
static short		building_id;	// used in search_mode = SEARCH_MODE_TO_FIRM or SEARCH_MODE_TO_TOWN, get from miscNo
//======================================================================//
// 1) if search_mode = SEARCH_MODE_TO_FIRM or SEARCH_MODE_TO_TOWN
//		the building top_left to bottom_right positions
// 2) if search_mode = SEARCH_MODE_ATTACK_UNIT_BY_RANGE,
//		the effective attacking region top_left to bottom_right positions
//======================================================================//
static int			building_x1, building_y1, building_x2, building_y2;
static FirmInfo	*search_firm_info;

static int			max_node_num;
static short		*reuse_node_matrix_ptr;
static Node			*reuse_result_node_ptr;
static short		final_dest_x;	//	in search_mode SEARCH_MODE_REUSE, dest_x and dest_y may set to a different value.
static short		final_dest_y;	// i.e. the value used finally may not be the real dest_? given.

//------- aliasing class member vars for fast access ------//

static SeekPath* 	cur_seek_path;
static short  		cur_dest_x, cur_dest_y;
static short* 		cur_node_matrix;
static Node*  		cur_node_array;
static short 		cur_border_x1, cur_border_y1, cur_border_x2, cur_border_y2;

static char			nation_passable[MAX_NATION+1] = {0}; // Note: position 0 is not used for faster access
static char			search_sub_mode;

//----------- Define static functions -----------//

static void  stack_push(Node *nodePtr);
static Node* stack_pop();

//-***************************************************************************-//
//-*************************** for debuging **********************************-//
//-***************************************************************************-//
#ifdef DEBUG
	static int is_yielding = 0;
	static ResultNode		*debugNode1, *debugNode2;
	static int				dcount;
	static int				vX, vY;	 // for debug only

	//---------- function debug_check() ------------//
	void debug_check(ResultNode *nodeArray, int count)
	{
		debugNode1 = nodeArray;
		debugNode2 = nodeArray+1;
		
		for(dcount=1; dcount<count; dcount++, debugNode1++, debugNode2++)
		{
			err_when(debugNode1->node_x<0 || debugNode1->node_x>=MAX_WORLD_X_LOC ||
						debugNode1->node_y<0 || debugNode1->node_y>=MAX_WORLD_Y_LOC);
			vX = debugNode2->node_x - debugNode2->node_x;
			vY = debugNode2->node_y - debugNode2->node_y;
			err_when(vX!=0 && vY!=0 && (abs(vX)!=abs(vY)));
		}

		err_when(debugNode1->node_x<0 || debugNode1->node_x>=MAX_WORLD_X_LOC ||
					debugNode1->node_y<0 || debugNode1->node_y>=MAX_WORLD_Y_LOC);
	}

	//----------- function debug_check_smode_exclude_hostile() -------------//
	void debug_check_smode_exclude_hostile(ResultNode *nodeArray, int count)
	{
		ResultNode *curNode = nodeArray;
		ResultNode *nextNode = nodeArray+1;
		Location *locPtr;
		int checkXLoc, checkYLoc, vecX, vecY, magn;
		for(int ij=1; ij<count; ij++, curNode++, nextNode++)
		{
			checkXLoc = curNode->node_x;
			checkYLoc = curNode->node_y;
			vecX = nextNode->node_x-curNode->node_x;
			vecY = nextNode->node_y-curNode->node_y;
			magn = (abs(vecX)>=abs(vecY)) ? abs(vecX) : abs(vecY);
			if(vecX) vecX /= abs(vecX);
			if(vecY) vecY /= abs(vecY);
			for(int jk=0; jk<magn; jk++)
			{
				checkXLoc += vecX;
				checkYLoc += vecY;
				locPtr = world.get_loc(checkXLoc, checkYLoc);
				err_when(locPtr->power_nation_recno && !nation_passable[locPtr->power_nation_recno]);
			}
		}
	}

	//-------------- function debug_check_sailable_path() --------------//
	void debug_check_sailable_path(ResultNode *nodeArray, int count)
	{
		ResultNode *debugPtr1 = nodeArray;
		ResultNode *debugPtr2 = nodeArray+1;
		int di=1, dj, dvecX, dvecY, magn;
		while(di<count)
		{
			dvecX = debugPtr2->node_x-debugPtr1->node_x;
			dvecY = debugPtr2->node_y-debugPtr1->node_y;
			magn = (abs(dvecX) > abs(dvecY)) ? abs(dvecX) : abs(dvecY);
			dvecX /= magn;
			dvecY /= magn;
			for(dj=1; dj<=magn; dj++)
				err_when(!world.get_loc(debugPtr1->node_x+dvecX*dj, debugPtr1->node_y+dvecY*dj)->sailable());
			
			di++;
			debugPtr1++;
			debugPtr2++;
		}
	}

#endif

#ifdef DEBUG
#define debug_check_path(nodeArray, count)			debug_check((nodeArray), (count))
#define debug_check_sub_mode(nodeArray, count)		debug_check_smode_exclude_hostile((nodeArray), (count))
#define debug_check_sea_sailable(nodeArray, count)	debug_check_sailable_path((nodeArray), (count))
#else
#define debug_check_path(nodeArray, count)
#define debug_check_sub_mode(nodeArray, count)
#define debug_check_sea_sailable(nodeArray, count)
#endif
//-***************************************************************************-//
//-**************************** end debuging *********************************-//
//-***************************************************************************-//

//------- Begin of static function sys_yield ------//
static void sys_yield()
{
	#ifdef DEBUG
		is_yielding++;
	#endif
	
	//sys.yield();
	
	#ifdef DEBUG
		is_yielding = 0;
	#endif
}
//-------- End of static function sys_yield ------//


//------- Begin of static function can_move_to ------//
static int can_move_to(int xLoc, int yLoc)
{
	Location	*locPtr = world_loc_matrix+yLoc*MAX_WORLD_X_LOC+xLoc;
	Unit		*unitPtr;
	short		recno;
	char		powerNationRecno;
	UCHAR		unitCurAction;

	//------ check terrain id. -------//
	switch(mobile_type)
	{
		case UNIT_LAND:
			if(search_sub_mode==SEARCH_SUB_MODE_PASSABLE && (powerNationRecno=locPtr->power_nation_recno) &&
				!nation_passable[powerNationRecno])
				return 0;

			if(search_mode<SEARCH_MODE_TO_FIRM)  //------ be careful for the checking for search_mode>=SEARCH_MODE_TO_FIRM
			{
				//------------------------------------------------------------------------//
				if(!locPtr->walkable())
					return 0;
				
				recno = locPtr->cargo_recno;
				if(!recno)
					return 1;

				switch(search_mode)
				{
					case SEARCH_MODE_IN_A_GROUP:	// group move
					case SEARCH_MODE_REUSE:		// path-reuse
							break;

					case SEARCH_MODE_A_UNIT_IN_GROUP:	// a unit in a group
							unitPtr = unit_array[recno];
							return unitPtr->cur_action==SPRITE_MOVE && unitPtr->unit_id!=UNIT_CARAVAN;

					case SEARCH_MODE_TO_ATTACK:	// to attack target
					case SEARCH_MODE_TO_VEHICLE:	// move to a vehicle
							if(recno==target_recno)
								return 1;
							break;

					case SEARCH_MODE_BLOCKING:	// 2x2 unit blocking
							unitPtr = unit_array[recno];
							return unitPtr->unit_group_id==group_id && (unitPtr->cur_action==SPRITE_MOVE || unitPtr->cur_action==SPRITE_READY_TO_MOVE);
					
					default: err_here();
								break;
				}
			}
			else
			{
				//--------------------------------------------------------------------------------//
				// for the following search_mode, location may be treated as walkable although it is not.
				//--------------------------------------------------------------------------------//
				switch(search_mode)
				{
					case SEARCH_MODE_TO_FIRM:	// move to a firm, (location may be not walkable)
					case SEARCH_MODE_TO_TOWN:	// move to a town zone, (location may be not walkable)
							if(!locPtr->walkable())
								return (xLoc>=building_x1 && xLoc<=building_x2 && yLoc>=building_y1 && yLoc<=building_y2);
							break;

					case SEARCH_MODE_TO_WALL_FOR_GROUP:	// move to wall for a group, (location may be not walkable)
							if(!locPtr->walkable())
								return (xLoc==final_dest_x && yLoc==final_dest_y);
							break;

					case SEARCH_MODE_TO_WALL_FOR_UNIT:	// move to wall for a unit only, (location may be not walkable)
							return (locPtr->walkable() && locPtr->cargo_recno==0) || (xLoc==final_dest_x && yLoc==final_dest_y);

					case SEARCH_MODE_ATTACK_UNIT_BY_RANGE: // same as that used in SEARCH_MODE_TO_FIRM
					case SEARCH_MODE_ATTACK_FIRM_BY_RANGE:
					case SEARCH_MODE_ATTACK_TOWN_BY_RANGE:
					case SEARCH_MODE_ATTACK_WALL_BY_RANGE:
							if(!locPtr->walkable())
								return (xLoc>=building_x1 && xLoc<=building_x2 && yLoc>=building_y1 && yLoc<=building_y2);
							break;
							
					default: err_here();
								break;
				}

				recno = (mobile_type!=UNIT_AIR) ? locPtr->cargo_recno : locPtr->air_cargo_recno;
				if(!recno)
					return 1;
			}

			//------- checking for unit's group_id, cur_action, nation_recno and position --------//
			unitPtr = unit_array[recno];
			unitCurAction = unitPtr->cur_action;
			return (unitPtr->unit_group_id==group_id && unitCurAction!=SPRITE_ATTACK) ||
					 (unitCurAction==SPRITE_MOVE && unitPtr->cur_x-unitPtr->next_x<=ZOOM_LOC_HALF_WIDTH &&
					  unitPtr->cur_y-unitPtr->next_y<=ZOOM_LOC_HALF_HEIGHT) ||
					 (unitPtr->nation_recno==seek_nation_recno && unitCurAction==SPRITE_IDLE);
			break;

		case UNIT_SEA:
			if(search_mode<SEARCH_MODE_TO_FIRM) //--------- be careful for the search_mode>=SEARCH_MODE_TO_FIRM
			{
				if(!locPtr->sailable())
					return 0;

				recno = locPtr->cargo_recno;
				if(!recno)
					return 1;

				switch(search_mode)
				{
					case SEARCH_MODE_IN_A_GROUP:	// group move
					case SEARCH_MODE_REUSE:		// path-reuse
							break;

					case SEARCH_MODE_A_UNIT_IN_GROUP:	// a unit in a group
							return unit_array[recno]->cur_action==SPRITE_MOVE;

					case SEARCH_MODE_TO_ATTACK:
							if(recno==target_recno)
								return 1;
							break;

					default:	err_here();
								break;
				}
			}
			else
			{
				//--------------------------------------------------------------------------------//
				// for the following search_mode, location may be treated as sailable although it is not.
				//--------------------------------------------------------------------------------//
				switch(search_mode)
				{
					case SEARCH_MODE_TO_FIRM:	// move to a firm, (location may be not walkable)
					case SEARCH_MODE_TO_TOWN:	// move to a town zone, (location may be not walkable)
							if(!locPtr->sailable())
								return (xLoc>=building_x1 && xLoc<=building_x2 && yLoc>=building_y1 && yLoc<=building_y2);
							break;
					
					//case SEARCH_MODE_TO_WALL_FOR_GROUP:	// move to wall for a group, (location may be not walkable)
					//case SEARCH_MODE_TO_WALL_FOR_UNIT:	// move to wall for a unit only, (location may be not walkable)

					case SEARCH_MODE_ATTACK_UNIT_BY_RANGE: // same as that used in SEARCH_MODE_TO_FIRM
					case SEARCH_MODE_ATTACK_FIRM_BY_RANGE:
					case SEARCH_MODE_ATTACK_TOWN_BY_RANGE:
					case SEARCH_MODE_ATTACK_WALL_BY_RANGE:
							if(!locPtr->sailable())
								return (xLoc>=building_x1 && xLoc<=building_x2 && yLoc>=building_y1 && yLoc<=building_y2);
							break;

					case SEARCH_MODE_TO_LAND_FOR_SHIP:
							if(locPtr->sailable())
							{
								recno = locPtr->cargo_recno;
								if(!recno)
									return 1;

								unitPtr = unit_array[recno];
								unitCurAction = unitPtr->cur_action;
								return (unitPtr->unit_group_id==group_id && unitCurAction!=SPRITE_ATTACK &&
										  unitPtr->action_mode2!=ACTION_SHIP_TO_BEACH) || 
										 (unitPtr->unit_group_id!=group_id && unitCurAction==SPRITE_MOVE);
							}
							else if(locPtr->walkable() && locPtr->region_id==region_id)
								return 1;
							else
								return 0;

					default: err_here();
								break;
				}
				recno = locPtr->cargo_recno;
				if(!recno)
					return 1;
			}
			
			//------- checking for unit's group_id, cur_action, nation_recno and position --------//
			unitPtr = unit_array[recno];
			unitCurAction = unitPtr->cur_action;
			return (unitPtr->unit_group_id==group_id && unitCurAction!=SPRITE_ATTACK) ||
					 unitCurAction==SPRITE_MOVE ||
					 (unitPtr->nation_recno==seek_nation_recno && unitCurAction==SPRITE_IDLE);
			break;

		case UNIT_AIR:
			recno = locPtr->air_cargo_recno;
			if(!recno)
				return 1;
			switch(search_mode)
			{
				case SEARCH_MODE_IN_A_GROUP:
				case SEARCH_MODE_REUSE:
				case SEARCH_MODE_TO_ATTACK:
				case SEARCH_MODE_TO_FIRM:
				case SEARCH_MODE_TO_TOWN:
				case SEARCH_MODE_TO_WALL_FOR_GROUP:
				case SEARCH_MODE_TO_WALL_FOR_UNIT:
				case SEARCH_MODE_ATTACK_UNIT_BY_RANGE:
				case SEARCH_MODE_ATTACK_FIRM_BY_RANGE:
				case SEARCH_MODE_ATTACK_TOWN_BY_RANGE:
				case SEARCH_MODE_ATTACK_WALL_BY_RANGE:
						unitPtr = unit_array[recno];
						unitCurAction = unitPtr->cur_action;
						return (unitPtr->unit_group_id==group_id && unitCurAction!=SPRITE_ATTACK) ||
								 unitCurAction==SPRITE_MOVE ||
								 (unitPtr->nation_recno==seek_nation_recno && unitCurAction==SPRITE_IDLE);

				case SEARCH_MODE_A_UNIT_IN_GROUP: // a unit in a group
						return unit_array[recno]->cur_action==SPRITE_MOVE;

				default: err_here();
							break;
			}
			break;
	}

	return 0;
}
//-------- End of static function can_move_to ------//


//-------- Begin of function SeekPath::bound_check_x ---------//
inline void SeekPath::bound_check_x(short &paraX)
{
	if(paraX<0)
		paraX = 0;
	else if(paraX>=MAX_WORLD_X_LOC-1)
		paraX = MAX_WORLD_X_LOC-1;
}
//-------- End of static function bound_check_x ------//


//-------- Begin of function SeekPath::bound_check_y ---------//
inline void SeekPath::bound_check_y(short &paraY)
{
	if(paraY<0)
		paraY = 0;
	else if(paraY>=MAX_WORLD_Y_LOC-1)
		paraY = MAX_WORLD_Y_LOC-1;
}
//-------- End of static function bound_check_y ------//


//-------- Begin of function SeekPath::result_node_distance ---------//
inline short SeekPath::result_node_distance(ResultNode *node1, ResultNode *node2)
{
	short xDist = abs(node1->node_x - node2->node_y);
	#ifdef DEBUG
	short yDist = abs(node1->node_y-node2->node_y);
	#endif

	if(xDist)
	{
		err_when(yDist && xDist!=yDist);
		return xDist;
	}
	else // xDist = 0;
	{
		err_when(yDist==0);
		return abs(node1->node_y-node2->node_y);
	}
}
//-------- End of static function result_node_distance ------//


//-------- Begin of function SeekPath::init ---------//
void SeekPath::init(int maxNode)
{
	max_node = maxNode;
	node_array = (Node*) mem_add( max_node * sizeof(Node) );
	node_matrix = (short*) mem_add(sizeof(short)*MAX_WORLD_X_LOC*MAX_WORLD_Y_LOC/4);

	path_status = PATH_WAIT;
	open_node_list.reset_priority_queue();
	closed_node_list.reset_priority_queue();

	reset_total_node_avail();
}
//--------- End of function SeekPath::init ---------//


//-------- Begin of function SeekPath::deinit ---------//
void SeekPath::deinit()
{
	if( node_array )
	{
		mem_del(node_array);
		node_array = NULL;
	}

	if( node_matrix )
	{
		mem_del(node_matrix);
		node_matrix = NULL;
	}
}
//--------- End of function SeekPath::deinit ---------//


//-------- Begin of function SeekPath::set_node_matrix ---------//
void SeekPath::set_node_matrix(short reuseNodeMatrix[])
{
	reuse_node_matrix_ptr = reuseNodeMatrix;
}
//--------- End of function SeekPath::set_node_matrix ---------//


//-------- Begin of function SeekPath::reset ---------//
void SeekPath::reset()
{
	path_status=PATH_WAIT;
	open_node_list.reset_priority_queue();
	closed_node_list.reset_priority_queue();
}
//--------- End of function SeekPath::reset ---------//


//-------- Begin of function SeekPath::reset_total_node_used ---------//
void SeekPath::reset_total_node_avail()
{
	total_node_avail = MAX_BACKGROUND_NODE;
}
//--------- End of function SeekPath::reset_total_node_used ---------//


//-------- Begin of function SeekPath::set_attack_range_para ---------//
void SeekPath::set_attack_range_para(int attackRange)
{
	attack_range = attackRange;
}
//--------- End of function SeekPath::set_attack_range_para ---------//


//-------- Begin of function SeekPath::reset_attack_range_para ---------//
void SeekPath::reset_attack_range_para()
{
	attack_range = 0;
}
//--------- End of function SeekPath::reset_attack_range_para ---------//


//-------- Begin of function SeekPath::set_nation_recno ---------//
// store the nation_recno of the unit calling searching
//
void SeekPath::set_nation_recno(char nationRecno)
{
	seek_nation_recno = nationRecno;
}
//--------- End of function SeekPath::set_nation_recno ---------//


//-------- Begin of function SeekPath::set_nation_passable ---------//
void SeekPath::set_nation_passable(char nationPassable[])
{
	memcpy(nation_passable+1, nationPassable, sizeof(char)*MAX_NATION);
}
//--------- End of function SeekPath::set_nation_passable ---------//


//-------- Begin of function SeekPath::set_sub_mode ---------//
void SeekPath::set_sub_mode(char subMode)
{
	search_sub_mode = subMode;
}
//--------- End of function SeekPath::set_sub_mode ---------//


//-------- Begin of function SeekPath::add_result_node ---------//
inline void SeekPath::add_result_node(int x, int y, ResultNode** curPtr, ResultNode** prePtr, int& count)
{
	(*curPtr)->node_x = x;
	(*curPtr)->node_y = y;

	err_when(count>1 && (abs((*curPtr)->node_x-(*prePtr)->node_x)>1 || abs((*curPtr)->node_y-(*prePtr)->node_y)>1));
	*prePtr = *curPtr;
	(*curPtr)++;
	count++;
}
//--------- End of function SeekPath::add_result_node ---------//


//-------- Begin of function SeekPath::seek ---------//
//
// <int> sx, sy      - the starting world location.
// <int> dx, dy      - the destination world location.
// <DWORD> groupId	- unit group id.
// <char> mobileType - mobile type, can be UNIT_AIR, UNIT_LAND or UNIT_SEA
//
//	[int] searchMode  -	1	SEARCH_MODE_IN_A_GROUP				for one group with an unique group id	(default)
//								2	SEARCH_MODE_A_UNIT_IN_GROUP		for one sprite in a group
//								3	SERACH_MODE_TO_ATTACK				for the searching that one sprite is ordered to attack its target
//								4	SEARCH_MODE_REUSE						for path-reuse
//								5	SEARCH_MODE_BLOCKING					for 2x2 unit blocking search
//								6	SEARCH_MODE_TO_FIRM					for moving to a firm
//								7	SEARCH_MODE_TO_TOWN					for moving to a town zone
//								8	SEARCH_MODE_TO_VEHICLE				for moving to a vehicle
//								9	SEARCH_MODE_TO_WALL_FOR_GROUP		for moving to a wall location 
//								10 SEARCH_MODE_TO_WALL_FOR_UNIT		for moving to a wall location 
//								11	SEARCH_MODE_ATTACK_UNIT_BY_RANGE	for range attacking target
//								12 SEARCH_MODE_ATTACK_FIRM_BY_RANGE ditto
//								13	SEARCH_MODE_ATTACK_TOWN_BY_RANGE ditto
//								14	SEARCH_MODE_ATTACK_WALL_BY_RANGE ditto
//								15	SEARCH_MODE_TO_LAND_FOR_SHIP		for ships to move to land
//								(default: 1)
//
//	[int] miscNo		-	if searchMode = SEARCH_MODE_TO_ATTACK, meaning target_recno
//							-	if searchMode = SEARCH_MODE_TO_FIRM, meaning firm_id
//							-	if searchMode = SEARCH_MODE_TO_LAND_FOR_SHIP, meaning the region_id of the land moving to
//							(default: 0)
//
// [int] numOfPath	- for group assign, group settle. It is used to generate more set of virtual
//							  destination in the firm/town for searching.
//
// [int] maxTries    - maximum no. of tries in the first seek action.
//						     this refer to the maximum no. of nodes created.
//						     (default : max_node)
//
// [int] borderX1, borderY1, - borders of the seek area in the world map
//			borderX2, borderY2	 (default: the whole map)
//
// Note: if maxTries==max_node, incremental seek (PATH_SEEKING) won't happen.
//
// return : <int> seekStatus - PATH_FOUND, PATH_SEEKING, PATH_NODE_USED_UP, or PATH_IMPOSSIBLE
//						if PATH_FOUND, or PATH_NODE_USED_UP, can call get_result() to retrieve the result.
//
int SeekPath::seek(int sx,int sy,int dx,int dy, DWORD groupId, char mobileType,
						 short searchMode, short miscNo, short numOfPath, int maxTries,
						 int borderX1,int borderY1,int borderX2,int borderY2)
{
	err_when(is_yielding);

	if(total_node_avail<=0)
		return PATH_FOUND; // checking

	border_x1 = short(borderX1/2);	// change to 2x2 node format
	border_y1 = short(borderY1/2);
	border_x2 = short(borderX2/2);
	border_y2 = short(borderY2/2);

	//-------- initialize vars --------------//
	current_search_node_used = 0; // count the number of nodes used in each searching
	path_status			= PATH_SEEKING;
	world_loc_matrix	= world.loc_matrix;
	open_node_list.reset_priority_queue();
	closed_node_list.reset_priority_queue();

	search_mode = searchMode;
	group_id 	= groupId;
	mobile_type = mobileType;
	err_when(search_mode!=searchMode || group_id!=groupId || mobile_type!=mobileType);

	//------------------------------------------------------------------------------//
	// using another searching for unit sea or unit air
	//------------------------------------------------------------------------------//
	if(mobile_type!=UNIT_LAND)
		return seek2(sx, sy, dx, dy, miscNo, numOfPath, maxTries);	// redirect entry of UNIT_SEA or UNIT_AIR

	//------------------------------------------------------------------------------//
	// extract informaton from the parameter "miscNo"
	//------------------------------------------------------------------------------//
	target_recno = building_id = 0;
	building_x1 = building_y1 = building_x2 = building_y2 = -1;

	switch(search_mode)
	{
		case SEARCH_MODE_TO_ATTACK:
		case SEARCH_MODE_TO_VEHICLE:
				target_recno = miscNo;
				break;

		case SEARCH_MODE_TO_FIRM:
				building_id = miscNo;
				building_x1 = dx; // upper left corner location
				building_y1 = dy;
				search_firm_info = firm_res[building_id];
				building_x2 = dx+search_firm_info->loc_width-1;
				building_y2 = dy+search_firm_info->loc_height-1;
				break;

		case SEARCH_MODE_TO_TOWN:
				building_id = miscNo;
				building_x1 = dx; // upper left corner location
				building_y1 = dy;
				if(miscNo != -1)
				{
					Location* buildPtr = world.get_loc(dx, dy);
					Town* targetTown = town_array[buildPtr->town_recno()];
					building_x2 = targetTown->loc_x2;
					building_y2 = targetTown->loc_y2;
				}
				else	// searching to settle. Detail explained in function set_move_to_surround()
				{
					building_x2 = building_x1+STD_TOWN_LOC_WIDTH-1;
					building_y2 = building_y1+STD_TOWN_LOC_HEIGHT-1;
				}
				break;

		case SEARCH_MODE_ATTACK_UNIT_BY_RANGE:
		case SEARCH_MODE_ATTACK_WALL_BY_RANGE:
				building_id = miscNo;
				building_x1 = MAX(dx-attack_range, 0);
				building_y1 = MAX(dy-attack_range, 0);
				building_x2 = MIN(dx+attack_range, MAX_WORLD_X_LOC-1);
				building_y2 = MIN(dy+attack_range, MAX_WORLD_Y_LOC-1);
				break;

		case SEARCH_MODE_ATTACK_FIRM_BY_RANGE:
				building_id = miscNo;
				building_x1 = MAX(dx-attack_range, 0);
				building_y1 = MAX(dy-attack_range, 0);
				search_firm_info = firm_res[building_id];
				building_x2 = MIN(dx+search_firm_info->loc_width-1+attack_range, MAX_WORLD_X_LOC-1);
				building_y2 = MIN(dy+search_firm_info->loc_height-1+attack_range, MAX_WORLD_Y_LOC-1);
				break;

		case SEARCH_MODE_ATTACK_TOWN_BY_RANGE:
				building_id = miscNo;
				building_x1 = MAX(dx-attack_range, 0);
				building_y1 = MAX(dy-attack_range, 0);
				building_x2 = MIN(dx+STD_TOWN_LOC_WIDTH-1+attack_range, MAX_WORLD_X_LOC-1);
				building_y2 = MIN(dy+STD_TOWN_LOC_HEIGHT-1+attack_range, MAX_WORLD_Y_LOC-1);
				break;
	}

	//------------------------------------------------------------------------------//
	// set start location and destination location
	//------------------------------------------------------------------------------//
	real_sour_x = sx;
	real_sour_y = sy;

	//---------- adjust destination for some kind of searching ------------//
	int xShift, yShift, area;
	short pathNum;
	switch(search_mode)
	{
		case SEARCH_MODE_TO_FIRM:
		case SEARCH_MODE_TO_TOWN:
				final_dest_x = (building_x1+building_x2)/2;	// the destination is set to be the middle of the building
				final_dest_y = (building_y1+building_y2)/2;

				//---------------------------------------------------------------------------------//
				// for group assign/settle, the final destination is adjusted by the value of numOfPath
				//---------------------------------------------------------------------------------//
				if(search_mode==SEARCH_MODE_TO_TOWN)
					area = STD_TOWN_LOC_WIDTH*STD_TOWN_LOC_HEIGHT;
				else // search_mode == SEARCH_MODE_TO_FIRM
					area = search_firm_info->loc_width*search_firm_info->loc_height;

				pathNum = (numOfPath>area) ? (numOfPath-1)%area + 1 : numOfPath;

				if(search_mode==SEARCH_MODE_TO_TOWN)
					m.cal_move_around_a_point(pathNum, STD_TOWN_LOC_WIDTH, STD_TOWN_LOC_HEIGHT, xShift, yShift);
				else
					m.cal_move_around_a_point(pathNum, search_firm_info->loc_width, search_firm_info->loc_height, xShift, yShift);

				final_dest_x += xShift;
				final_dest_y += yShift;

				bound_check_x(final_dest_x);
				bound_check_y(final_dest_y);
				break;

		case SEARCH_MODE_ATTACK_UNIT_BY_RANGE:
		case SEARCH_MODE_ATTACK_FIRM_BY_RANGE:
		case SEARCH_MODE_ATTACK_TOWN_BY_RANGE:
		case SEARCH_MODE_ATTACK_WALL_BY_RANGE:
				final_dest_x = (building_x1+building_x2)/2;	// the destination is set to be the middle of the building
				final_dest_y = (building_y1+building_y2)/2;
				break;
		
		default:
				final_dest_x = real_dest_x = dx;
				final_dest_y = real_dest_y = dy;
				break;
	}

	//--------------------------------------------------------------//
	// change to 2x2 node format
	//--------------------------------------------------------------//
	int sourceX	= short(sx/2);	// the upper left corner is used
	int sourceY	= short(sy/2);
	dest_x	= short(final_dest_x/2);
	dest_y	= short(final_dest_y/2);

	//-----------------------------------------//
	// reset node_matrix
	//-----------------------------------------//
	if(search_mode!=SEARCH_MODE_REUSE)
	{
		max_node_num = 0xFFFF;
		memset(node_matrix, 0, sizeof(short)*MAX_WORLD_X_LOC*MAX_WORLD_Y_LOC/4);
	}
	else
	{
		max_node_num = max_node;
		memcpy(node_matrix, reuse_node_matrix_ptr, sizeof(short)*MAX_WORLD_X_LOC*MAX_WORLD_Y_LOC/4);
	}

	//--------- create the first node ---------//
	node_count  = 0;
	result_node_ptr = NULL;

	Node *nodePtr = node_array + node_count++;
	memset(nodePtr, 0, sizeof(Node));

	//-------- store the upper left coordinate of the node ----------//
	upper_left_x = sourceX<<1;
	upper_left_y = sourceY<<1;

	//---------- calculate the node type -----------//
	nodePtr->node_type = 0;
	nodePtr->node_type = (can_move_to(upper_left_x,upper_left_y))+(can_move_to(upper_left_x+1,upper_left_y)<<1)+
							  (can_move_to(upper_left_x,upper_left_y+1)<<2)+(can_move_to(upper_left_x+1,upper_left_y+1)<<3);

	if(searchMode==SEARCH_MODE_A_UNIT_IN_GROUP || searchMode==SEARCH_MODE_TO_WALL_FOR_UNIT)	// plus the self-location
		nodePtr->node_type += (real_sour_x>upper_left_x)?2+(real_sour_y-upper_left_y)*6:1+(real_sour_y-upper_left_y)*3;
	else
	{
		/*if(searchMode==SEARCH_MODE_TO_FIRM || searchMode==SEARCH_MODE_TO_TOWN)	// plus the self-location
		{
			if(real_sour_x<building_x1 || real_sour_x>building_x2 ||
				real_sour_y<building_y1 || real_sour_y>building_y2)
				nodePtr->node_type += (real_sour_x>upper_left_x)?2+(real_sour_y-upper_left_y)*6:1+(real_sour_y-upper_left_y)*3;
		}*/
	}

	err_when(nodePtr->node_type>15 || nodePtr->node_type <0);

	int destUpperLeftX = dest_x<<1;
	int destUpperLeftY = dest_y<<1;
	is_dest_blocked = !((can_move_to(destUpperLeftX,destUpperLeftY))+(can_move_to(destUpperLeftX+1,destUpperLeftY)<<1)+
							(can_move_to(destUpperLeftX,destUpperLeftY+1)<<2)+(can_move_to(destUpperLeftX+1,destUpperLeftY+1)<<3));
	// whether the destination is blocked, if so, only search till the destination's neighbor locations

	nodePtr->node_g = 0;
	nodePtr->node_h = (sourceX-dest_x)*(sourceX-dest_x)+(sourceY-dest_y)*(sourceY-dest_y);  // should really use sqrt().
	nodePtr->node_f = nodePtr->node_g + nodePtr->node_h;
	nodePtr->node_x = sourceX;
	nodePtr->node_y = sourceY;
	nodePtr->enter_direction = 0;
	open_node_list.insert_node(nodePtr); // make Open List point to first node

	//--- if the destination is the current postion or next to it & the dest is occupied ---//
	if( sourceX==dest_x && sourceY==dest_y )
	{
		path_status 	 = PATH_FOUND;
		result_node_ptr = nodePtr;
		return path_status;
	}

	//------------ seek now ------------------//
	int maxNode = (!maxTries) ? max_node : maxTries;
	return continue_seek(maxNode, 1);	// 1-first seek session of the current seek order
}
//-------- End of function SeekPath::seek ---------//


//---- Begin of function SeekPath::continue_seek ---------//
//
// If the last seek operation does not find the whole path,
// continue the search.
//
// <int>  maxTries  - maximum path seeking tries
// [char] firstSeek - whether it's the first seek session of the seek order.
//							 (default: 0)
//
// return : <int> seekStatus - PATH_FOUND, PATH_SEEKING, PATH_NODE_USED_UP,
//										 or PATH_IMPOSSIBLE
//
// You can call get_result() to retrieve the result.
//
int SeekPath::continue_seek(int maxTries, char firstSeek)
{
	if( path_status != PATH_SEEKING )
		return path_status;

	//------- aliasing class member vars for fast access ------//
	cur_seek_path   = this;
	cur_dest_x	    = dest_x;
	cur_dest_y	  	 = dest_y;
	cur_node_matrix = node_matrix;
	cur_node_array  = node_array;

   cur_border_x1 	 = border_x1;
	cur_border_y1 	 = border_y1;
	cur_border_x2 	 = border_x2;
	cur_border_y2 	 = border_y2;

	//------ seek the path using the A star algorithm -----//
	int maxNode = (total_node_avail<maxTries) ? total_node_avail : maxTries;
	maxNode -= MAX_CHILD_NODE; // generate_successors() can generate a MAX of MAX_CHILD_NODE new nodes per call
	Node *bestNodePtr;

	int i;
	for(i=0; i<maxNode; i++)
	{
		bestNodePtr = return_best_node();

		//if(i%20==0)
		//	sys_yield(); // update cursor position

		//---- even if the path is impossible, get the closest path ----//
		if( !bestNodePtr )
		{
			path_status = PATH_IMPOSSIBLE;
			break;
		}

		//----- exceed the object's MAX's node limitation, return the closest path ----//
		if( node_count >= maxNode )
		{
			path_status = PATH_NODE_USED_UP;
			break;
		}

		//---------------------------------------------------------------//
		// If the path is found OR If the destination is blocked,
		// consider it done when we are next to it.
		//---------------------------------------------------------------//
		if( (bestNodePtr->node_x==dest_x && bestNodePtr->node_y==dest_y) ||
			 ( is_dest_blocked && abs(bestNodePtr->node_x-dest_x)<=0 && abs(bestNodePtr->node_y-dest_y)<=0 ) )
		{
			path_status 	 = PATH_FOUND;
			result_node_ptr = bestNodePtr;
			break;
		}

		//--------- generate successors -------//
		if(bestNodePtr->generate_successors(bestNodePtr->enter_direction, real_sour_x, real_sour_y))
		{
			path_status = PATH_REUSE_FOUND;
			result_node_ptr = reuse_result_node_ptr;
			break;
		}
	}

	err_when( cur_stack_pos!=0 );		// it should be zero all the times, all pushes should have been poped
	current_search_node_used = i+1;		// store the number of nodes used in this searching
	return path_status;
}
//------ End of function SeekPath::continue_seek ---------//


//---- Begin of function SeekPath::get_result ---------//
//
// Compile the seek result nodes using results processed by
// seek() and continue_seek() and store the results in
// an array of ResultNode.
//
// <int&>  resultNodeCount - a reference var for returning the no. of result nodes.
// <short&> pathDist			- a reference var for returning the total distance of the result path
//
// return : <ResultNode*> an array of ResultNode.
//								  the caller function is responsible for
//								  freeing the memory of the array.
//
ResultNode* SeekPath::get_result(int& resultNodeCount, short& pathDist)
{
	if(mobile_type!=UNIT_LAND)
		return get_result2(resultNodeCount, pathDist); // redirect entry point for UNIT_SEA or UNIT_AIR

	resultNodeCount = pathDist = 0;
	if(total_node_avail<=0)
		return NULL;
	
	total_node_avail -= current_search_node_used;
	short useClosestNode = 0; // indicate whether closest node is returned instead of the actual node

	if(!result_node_ptr)	// if PATH_IMPOSSIBLE or PATH_NODE_USED_UP, result_node_ptr is NULL, we need to call get_closest_node() to get the closest node.
	{
		result_node_ptr = return_closest_node();
		useClosestNode = 1;

		if(!result_node_ptr)
			return NULL;
	}

	//--------------------------------------------------//
	// Trace backwards to the starting node, and rationalize
	// nodes (i.e. group nodes of the same direction into
	// single nodes.)
	//--------------------------------------------------//
	Node* nodePtr		= result_node_ptr;		// the node current being processed
	Node* baseNodePtr = result_node_ptr;		// the first end node for connecting the other end node for the path in that direction.
	Node* parentNode  = nodePtr->parent_node;	 // the parent node of nodePtr
	Node* childNodePtr = nodePtr;					// it should point to the children node of nodePtr

	//------------------------------------------------------------------------
	// there are only one node, source & destination within the same 2x2 node
	//------------------------------------------------------------------------
	if(!parentNode) 		// parentNode==0 when the source location is the desination already
	{ 
		if((real_sour_x!=final_dest_x || real_sour_y!=final_dest_y) && abs(real_sour_x-final_dest_x)<=1 &&
			 abs(real_sour_y-final_dest_y)<=1 && can_move_to(final_dest_x, final_dest_y))
		{
			pathDist = 1;

			ResultNode* resultNodeArray1 = (ResultNode*) mem_add(sizeof(ResultNode)*2);
			ResultNode* resultNodePtr1 = resultNodeArray1;
			resultNodeCount=2;
			resultNodePtr1->node_x = real_sour_x;
			resultNodePtr1->node_y = real_sour_y;
			resultNodePtr1++;
			resultNodePtr1->node_x = final_dest_x;
			resultNodePtr1->node_y = final_dest_y;
			return resultNodeArray1;
		}
		else
			return NULL;
	}

	resultNodeCount=1;		// including the current node

	//===================================
	//	count the number of 2x2 node
	//===================================
	int numOfNode=0;
	Node* curPtr = result_node_ptr;

	while(curPtr != NULL)
	{
		curPtr = curPtr->parent_node;
		numOfNode++;
	}

	//sys_yield(); // update cursor position

	//---------------------------------
	// otherwise, there are more than one node
	//---------------------------------
	node_count=0;

	ResultNode* maxSizeResultNodeArray;	// store all the result node in the reverse order, the turning point will be extracted later
	int nodeAllocated = (numOfNode+1)<<1;//numOfNode*2+2;  // the additional 2 is for the starting node
	
	maxSizeResultNodeArray = (ResultNode*) mem_add(nodeAllocated*sizeof(ResultNode));
	max_size_result_node_ptr = maxSizeResultNodeArray;
	parent_result_node_ptr = maxSizeResultNodeArray;

	//----------------------------------
	// start from the destination point
	//----------------------------------
	memset(max_size_result_node_ptr, 0, sizeof(ResultNode)*nodeAllocated);
	int upper_left_x = nodePtr->node_x<<1;
	int upper_left_y = nodePtr->node_y<<1;
	short xCount, yCount;	// for counting

	if(!useClosestNode && (search_mode==SEARCH_MODE_TO_ATTACK || search_mode==SEARCH_MODE_TO_VEHICLE ||
		can_move_to(final_dest_x, final_dest_y)))
	{
		max_size_result_node_ptr->node_x = final_dest_x;
		max_size_result_node_ptr->node_y = final_dest_y;
	}
	else
	{	// use closest node

		max_size_result_node_ptr->node_x = MAX_WORLD_X_LOC;
		max_size_result_node_ptr->node_y = MAX_WORLD_Y_LOC;
		
		int newValue, xSquare, yDiff;
		int compareValue = 0x7FFFFFFF; // should > 199^2 + 199^2
		
		for(xCount=upper_left_x+1; xCount>=upper_left_x; xCount--)
		{
			xSquare = int(final_dest_x-xCount)*(final_dest_x-xCount);
			for(yCount=upper_left_y+1, yDiff=final_dest_y-yCount; yCount>=upper_left_y; yCount--, yDiff++)
			{
				if(can_move_to(xCount, yCount) && (newValue = xSquare + yDiff*yDiff)<compareValue)
				{
					max_size_result_node_ptr->node_x = xCount;
					max_size_result_node_ptr->node_y = yCount;
					compareValue = newValue;
				}
			}
		}

		err_when(max_size_result_node_ptr->node_x==MAX_WORLD_X_LOC &&
					max_size_result_node_ptr->node_y==MAX_WORLD_Y_LOC);
		
		final_dest_x = max_size_result_node_ptr->node_x;
		final_dest_y = max_size_result_node_ptr->node_y;
	}
	xCount = max_size_result_node_ptr->node_x;
	yCount = max_size_result_node_ptr->node_y;
	max_size_result_node_ptr++;	// note: parent_result_node_ptr don't move for this case
	node_count++;

	//---------------------------------------------------
	// process the end node if passing through two points
	//---------------------------------------------------
	int xLoc, yLoc;
	switch(nodePtr->enter_direction)
	{	
		case 1:						
			      if(upper_left_x < xCount)
					{	
						yLoc = can_move_to(upper_left_x, yCount) ? yCount : ((yCount>upper_left_y)?upper_left_y:upper_left_y+1);
						add_result_node(upper_left_x, yLoc, &max_size_result_node_ptr, &parent_result_node_ptr, node_count);
					}
					break;

		case 2:	if((upper_left_x!=xCount) || ((upper_left_y+1)!=yCount))
						add_result_node(upper_left_x, upper_left_y+1, &max_size_result_node_ptr, &parent_result_node_ptr, node_count);
					break;

		case 3:	
			      if(upper_left_y == yCount)
					{
						xLoc = can_move_to(xCount,upper_left_y+1) ? xCount : ((xCount>upper_left_x)?upper_left_x:(upper_left_x+1));
						add_result_node(xLoc, upper_left_y+1, &max_size_result_node_ptr, &parent_result_node_ptr, node_count);
					}
					break;

		case 4:	if(((upper_left_x+1)!=xCount) || ((upper_left_y+1)!=yCount))
						add_result_node(upper_left_x+1, upper_left_y+1, &max_size_result_node_ptr, &parent_result_node_ptr, node_count);
					break;

		case 5:	
					if(upper_left_x == final_dest_x)
					{
						yLoc = can_move_to(upper_left_x+1,yCount) ? yCount : ((yCount>upper_left_y)?upper_left_y:(upper_left_y+1));
						add_result_node(upper_left_x+1, yLoc, &max_size_result_node_ptr, &parent_result_node_ptr, node_count);
					}
					break;
		
		case 6:	if(((upper_left_x+1)!=xCount) || (upper_left_y!=yCount))
						add_result_node(upper_left_x+1, upper_left_y, &max_size_result_node_ptr, &parent_result_node_ptr, node_count);
					break;

		case 7:
					if(upper_left_y != yCount)
					{
						xLoc = can_move_to(xCount,upper_left_y) ? xCount : ((xCount>upper_left_x)?upper_left_x:(upper_left_x+1));
						add_result_node(xLoc, upper_left_y, &max_size_result_node_ptr, &parent_result_node_ptr, node_count);
			      }
					break;

		case 8:	if((upper_left_x!=xCount) || (upper_left_y!=yCount))
						add_result_node(upper_left_x, upper_left_y, &max_size_result_node_ptr, &parent_result_node_ptr, node_count);
					break;

		default:
					err_now("error in processing the end node");
					break;
	}
	nodePtr = nodePtr->parent_node;	// next 2x2 node
	
	//preNodeCount = node_count;
	err_when(node_count+nodePtr->node_g+2 > nodeAllocated);
		
	//--------------------------------------------------
	// get the actual path, process from the second node
	//--------------------------------------------------
	//int yieldCount = 0;
	while( (parentNode=nodePtr->parent_node) != NULL )
	{
		upper_left_x = nodePtr->node_x<<1;
		upper_left_y = nodePtr->node_y<<1;
		get_real_result_node(node_count,	nodePtr->enter_direction,(childNodePtr->enter_direction+3)%8+1,
									nodePtr->node_type, upper_left_x, upper_left_y);
		childNodePtr = nodePtr;
		nodePtr = parentNode;

		err_when(node_count+nodePtr->node_g+2 > nodeAllocated);
		//yieldCount++;
		//if(yieldCount%30==0)
		//	sys_yield();
	}

	//sys_yield(); // update cursor position
	
	//----------------------------------------------------
	// process the starting node
	// nodePtr points at the starting node now
	//----------------------------------------------------
	if(abs(parent_result_node_ptr->node_x-real_sour_x)>1 || abs(parent_result_node_ptr->node_y-real_sour_y)>1)
	{	// passing through two points
		upper_left_x = nodePtr->node_x<<1;
		upper_left_y = nodePtr->node_y<<1;
		switch(childNodePtr->enter_direction)
		{
			case 1:	
						max_size_result_node_ptr->node_x = upper_left_x+1;
						if((can_move_to(upper_left_x+1, real_sour_y)&&(real_sour_y==upper_left_y)) ||
							(!can_move_to(upper_left_x+1, real_sour_y)&&(real_sour_y>upper_left_y)))
							max_size_result_node_ptr->node_y = upper_left_y;
						else
							max_size_result_node_ptr->node_y = (upper_left_y+1);
						break;

			case 2:	
						max_size_result_node_ptr->node_x = upper_left_x+1;
						max_size_result_node_ptr->node_y = upper_left_y;
						break;

			case 3:	
						max_size_result_node_ptr->node_y = upper_left_y;
						if((can_move_to(real_sour_x, upper_left_y)&&(real_sour_x==upper_left_x)) ||
							(!can_move_to(real_sour_x, upper_left_y)&&(real_sour_x>upper_left_x)))
							max_size_result_node_ptr->node_x = upper_left_x;
						else
							max_size_result_node_ptr->node_x = upper_left_x+1;
						break;

			case 4:	
						max_size_result_node_ptr->node_x = upper_left_x;
						max_size_result_node_ptr->node_y = upper_left_y;
						break;

			case 5:	
						max_size_result_node_ptr->node_x = upper_left_x;
						if((can_move_to(upper_left_x, real_sour_y)&&(real_sour_y==upper_left_y)) ||
							(!can_move_to(upper_left_x, real_sour_y)&&(real_sour_y>upper_left_y)))
							max_size_result_node_ptr->node_y = upper_left_y;
						else
							max_size_result_node_ptr->node_y = upper_left_y+1;
						break;

			case 6:	
						max_size_result_node_ptr->node_x = upper_left_x;
						max_size_result_node_ptr->node_y = upper_left_y+1;
						break;

			case 7:	
						max_size_result_node_ptr->node_y = upper_left_y+1;
						if((can_move_to(real_sour_x, upper_left_y+1)&&(real_sour_x==upper_left_x)) ||
							(!can_move_to(real_sour_x, upper_left_y+1)&&(real_sour_x>upper_left_x)))
							max_size_result_node_ptr->node_x = upper_left_x;
						else
							max_size_result_node_ptr->node_x = upper_left_x+1;
						break;

			case 8:	
						max_size_result_node_ptr->node_x = upper_left_x+1;
						max_size_result_node_ptr->node_y = upper_left_y+1;
						break;

			default:
						err_now("error in processing the start node");
						break;
						

		}

		err_when((max_size_result_node_ptr->node_x==real_sour_x) && (max_size_result_node_ptr->node_y==real_sour_y));
		parent_result_node_ptr++;
		max_size_result_node_ptr++;
		node_count++;
	}
	max_size_result_node_ptr->node_x = real_sour_x;
	max_size_result_node_ptr->node_y = real_sour_y;
	node_count++;

	err_when(nodePtr->node_g || node_count>nodeAllocated);
	debug_check_path(maxSizeResultNodeArray, node_count); //*** debug only

	ResultNode* result_node_pointer;
	maxSizeResultNodeArray = smooth_the_path(maxSizeResultNodeArray, resultNodeCount);

	//sys_yield(); // update cursor position
	debug_check_path(maxSizeResultNodeArray, resultNodeCount); //*** debug only

	//-------------------------------------------------------------------//
	// After the above process, here we will have a link of rationalize
	// nodes. Retrieve them in the backwards order
	//-------------------------------------------------------------------//
 	ResultNode *resultNodeArray = (ResultNode*) mem_add( sizeof(ResultNode) * resultNodeCount );
	ResultNode *resultNodePtr = resultNodeArray+resultNodeCount-1;
	int 			processCount = 1;
	
	ResultNode *preNodePtr = maxSizeResultNodeArray;
	*resultNodePtr = *preNodePtr;
	resultNodePtr--;

	result_node_pointer = maxSizeResultNodeArray+1;
	err_when(pathDist!=0);

	int xDist, yDist;
	//yieldCount = 0;

	while(processCount++ < resultNodeCount)
	{
		err_when(result_node_pointer->node_x<0 || result_node_pointer->node_x>=MAX_WORLD_X_LOC || 
					result_node_pointer->node_y<0 || result_node_pointer->node_y>=MAX_WORLD_Y_LOC);

		*resultNodePtr = *result_node_pointer;
		resultNodePtr--;
		
		xDist = abs(result_node_pointer->node_x-preNodePtr->node_x);
		yDist = abs(result_node_pointer->node_y-preNodePtr->node_y);
		err_when((!xDist && !yDist) || (xDist && yDist && xDist!=yDist));
		pathDist += (xDist) ? xDist : yDist;

		preNodePtr++;
		result_node_pointer++;
		
		//yieldCount++;
		//if(yieldCount%35==0)
		//	sys_yield();
	}

	err_when(nodeAllocated<node_count);
	mem_del(maxSizeResultNodeArray);

	//======================================================================//
	#ifdef DEBUG
		if(search_sub_mode==SEARCH_SUB_MODE_PASSABLE && resultNodeCount>1)
		{
			err_when(mobile_type!=UNIT_LAND);
			debug_check_sub_mode(resultNodeArray, resultNodeCount);
		}
	#endif
	//======================================================================//

	return resultNodeArray;
}
//------ End of function SeekPath::get_result ---------//


//---- Begin of function SeekPath::get_real_result_node ---------//
//
// called by get_result to extract the point in a 2x2 node that is 
// used in the shortest path
//
// <int&>  count - a reference var for counting number of node in 
//						 max_size_result_node_ptr
//
void SeekPath::get_real_result_node( int &count, short enterDirection, short exitDirection,
												short nodeType, short xCoord, short yCoord)
{
	short ma, mb, mc, md;	//	| a b | four elements of a 2x2 matrix
									// | c d | these values are either 0 or 1
	short mTmp;					// used in swapping
	short atEdge;				// at_edge = 1 if exitDirection is at the edge, otherwise 0 for corner
	short	rotateAngle;		// rotate angle clockwisely = its value*90 degree, value ranges from 0 to 3
	short furtherCheck;		//	indicate further checking is needed in finding a path in the node
	short rotatedEnterDirection;	// reference enter direction after rotation
	
	ma = nodeType&0x1;
	mb = (nodeType&0x2)>>1;
	mc = (nodeType&0x4)>>2;
	md = (nodeType&0x8)>>3;
	
	err_when((ma+(mb<<1)+(mc<<2)+(md<<3)) != nodeType);
	atEdge = exitDirection%2;
	rotateAngle = short((exitDirection-1)/2);
	furtherCheck = 0;	// may be used only if enterDirection and exitDirection are opposite to each other
		
	int exitArrowLeft		= 0;
	int exitArrowRight	= 0;
	int enterArrowLeft	= 0; 
	int enterArrowRight	= 0;
	if(atEdge)	// exit at the edge
	{
		//---------------------------------------------------------------------//
		//						 -------
		//	exitArrowRight	|1	 |2  |
		//			<--------|-------|
		//	exitArrowLeft	|3	 |4  |
		//						 -------
		// There are 2 possible choice for the previous node to select.  One is
		// the point left of 1, and the other is the point left of 3.  Since there
		// are four possible edge exiting cases, it call be represented by rotated
		// the above figure by 90 degree each.
		//
		// if the point left of 1 is chosen, exitArrowRight = 1
		// if the point left of 3 is chosen, exitArrowLeft = 1
		// the flag exitArrowLeft and exitArrowRight is used to generate a better
		// result path shape.
		//---------------------------------------------------------------------//

		switch(exitDirection)
		{
			case 1:	if(parent_result_node_ptr->node_y%2)
							exitArrowLeft = 1;
						else
							exitArrowRight = 1;
						break;

			case 3:	if(parent_result_node_ptr->node_x%2)
							exitArrowLeft = 1;
						else
							exitArrowRight = 1;
						break;

			case 5:	if(parent_result_node_ptr->node_y%2)
							exitArrowRight = 1;
						else
							exitArrowLeft = 1;
						break;

			case 7:	if(parent_result_node_ptr->node_x%2)
							exitArrowRight = 1;
						else
							exitArrowLeft = 1;
						break;

			default:	err_here();
						break;
		}
	}
	else	// exit at edge
	{
		if(enterDirection%2) // enter at the edge
		{
			//---------------------------------------------------------------------//
			//						 -------
			//	enterArrowLeft |1	 |2  |
			//			-------->|-------|
			//	enterArrowRight|3	 |4  |
			//						 -------
			// There are 2 possible choice for the next node to select.  One is
			// the point left of 1, and the other is the point left of 3.  Since there
			// are four possible edge entering cases, it call be represented by rotated
			// the above figure by 90 degree each time.
			//
			// if the point left of 1 is chosen, enterArrowLeft = 1
			// if the point left of 3 is chosen, enterArrowRight = 1
			// the flag enterArrowLeft and enterArrowRight is used to generate a better
			//	result path shape.
			//---------------------------------------------------------------------//
		
			switch(enterDirection)
			{
				case 1:	if(can_move_to(xCoord-1, yCoord))
								enterArrowLeft = 1;
							if(can_move_to(xCoord-1, yCoord+1))
								enterArrowRight = 1;
							break;

				case 3:	if(can_move_to(xCoord, yCoord+2))
								enterArrowLeft = 1;
							if(can_move_to(xCoord+1, yCoord+2))
								enterArrowRight = 1;
							break;

				case 5:	if(can_move_to(xCoord+2, yCoord+1))
								enterArrowLeft = 1;
							if(can_move_to(xCoord+2, yCoord))
								enterArrowRight = 1;
							break;

				case 7:	if(can_move_to(xCoord+1, yCoord-1))
								enterArrowLeft = 1;
							if(can_move_to(xCoord, yCoord-1))
								enterArrowRight = 1;
							break;

				default:	err_here();
							break;
			}
		}
	}

	//----------------------------------------------------------------
	// perform rotation such that the exit direction is either 1 or 2
	//----------------------------------------------------------------
	switch(exitDirection)
	{	case 1: case 2:
					break;

		case 3: case 4: // rotate clockwise 90 degree
					//(((mTmp = mb), mb = ma), ma = mc), mc = md;
					mTmp	= mb;
					mb		= ma;
					ma		= mc;
					mc		= md;
					md		= mTmp;
					break;

		case 5: case 6: // rotate clockwise 180 degree
					mTmp = md;
					md		= ma;
					ma		= mTmp;
					mTmp	= mb;
					mb		= mc;
					mc		= mTmp;
					break;

		case 7: case 8: // rotate clockwise 270 degree
					mTmp	= ma;
					ma		= mb;
					mb		= md;
					md		= mc;
					mc		= mTmp;
					break;
		default:
					err_now("exitDirection error");
					break;
	}
	
	rotatedEnterDirection = (enterDirection-(rotateAngle<<1)+7)%8+1; // store angle rotated for reverse rotation later
	err_when(rotatedEnterDirection>8 || rotatedEnterDirection <1);

	//-----------------------------------------
	// set the value of the matrix element to
	// 1 for the possible answer, the rest is 0
	//-----------------------------------------
	if(atEdge) // the case for the exit direction at 1
	{	//------------------- at edge ------------------------//
		switch(rotatedEnterDirection)
		{
			case 1:
						err_now("unexpected case at edge");
						break;

			case 2: 
						err_when(!mc);
						mc = 1;		// must be
						ma = mb = md = 0;
						break;

			case 3:	
						// there are two possible paths
						if(mc)	// check for the perfered path
							ma = mb = md = 0;
						else
						{	
							err_when((!ma) || (!md));
							mb = 0;
						}
							// ma, md should be 1
						break;

			case 4:
						err_when(!md);	// md should be 1
						mb = 0;
						err_when((!mc) && (!ma));

						if(exitArrowLeft)
							ma = 1-mc;	// either one is used, prefer mc	
						else	// should be exitArrowRight
							mc = 1-ma;	// either one is used, prefer ma
						break;

			case 5:
						if(ma&&mb)
						{	// one path (bar state) exists
							if(mc&&md)
								furtherCheck = 1;// both paths exist
							else
								mc = md = 0; // choose ma, mb
						}
						else if(mc&&md)
							ma = mb = 0; // choose mc, md
						else
							err_when(((!ma)&&(!mc)) && ((!mb)&&(!md)));
							// else, only one path exists, do nothing
						break;

			case 6:	// similar to case 4
						err_when(!mb);	// mb should be 1
						md = 0;
						err_when((!ma)&&(!mc));

						if(exitArrowLeft)
							ma = 1-mc;	// either one is used, prefer mc	
						else	// should be exitArrowRight
							mc = 1-ma;	// either one is used, prefer ma
						break;

			case 7:	// similar to case 3
						if(ma)
							mb = mc = md = 0;
						else
						{
							err_when((!mb)||(!mc));
							md = 0; // mb, mc should be 1
						}
						break;

			case 8:	
						err_when(!ma);
						ma = 1;	// must be
						mb = mc = md = 0;
						break;

			default:
						err_now("at edge error");
						break;
		}
	}
	else
	{	//---------------------------- at corner-------------------------//
		// the case for the exit direction at 2
		switch(rotatedEnterDirection)
		{
			case 1: case 3:
						err_when(!mc);
						mc = 1; // must be
						ma = mb = md = 0;
						break;

			case 2:	
						err_now("unexpected case at corner");
						break;

			case 4: 
						err_when((!mc)||(!md));
						mc = md = 1; // must be
						ma = mb = 0;
						break;

			case 5:
						err_when(!mc);
						mc = 1; // must be
						ma = 0;
						err_when((!mb)&&(!md));

						if(!enterArrowLeft)	// the enter arrow left location canot be walked
							md = 1-mb;	// either one is used, prefer mb
						else
							mb = 1-md;	// either one is used, prefer md
						break;

			case 6:
						err_when((!mb)||(!mc));
						mb = mc = 1;
						ma = md = 0;
						break;

			case 7: 
						err_when(!mc);
						mc = 1; // must be
						md = 0;
						err_when((!ma)&&(!mb));

						if(!enterArrowRight)
							ma = 1-mb;	// either one is used, prefer mb
						else
							mb = 1-ma;	// either one is used, prefer ma
						break;

			case 8: 
						err_when((!ma)||(!mc));
						ma = mc = 1; // must be
						mb = md = 0;
						break;

			default:
						err_now("at corner error");
						break;
		}

	}

	//--------------------------- reverse rotation ----------------------------//
	switch(exitDirection)
	{	case 1: case 2:
					break;

		case 3: case 4: // rotate clockwise 270 degree
					mTmp	= ma;
					ma		= mb;
					mb		= md;
					md		= mc;
					mc		= mTmp;
					break;

		case 5: case 6: // rotate clockwise 180 degree
					mTmp = md;
					md		= ma;
					ma		= mTmp;
					mTmp	= mb;
					mb		= mc;
					mc		= mTmp;
					break;

		case 7: case 8: // rotate clockwise 90 degree
					mTmp	= mb;
					mb		= ma;
					ma		= mc;
					mc		= md;
					md		= mTmp;
					break;

		default:
					err_now("exitDirection error");
					break;
	}

	//------------------- get the answer ----------------------//
	switch(exitDirection)
	{
		case 1:
					if(!furtherCheck)
					{
						add_result_node(xCoord, yCoord+1-ma, &max_size_result_node_ptr, &parent_result_node_ptr, count);
						err_when(mb && md);
						if(mb||md) // at most one is 1
							add_result_node(xCoord+1, yCoord+1-mb, &max_size_result_node_ptr, &parent_result_node_ptr, count);
					}
					else
					{	
						if(parent_result_node_ptr->node_y == yCoord)
						{	// use upper path
							add_result_node(xCoord, yCoord, &max_size_result_node_ptr, &parent_result_node_ptr, count);
							add_result_node(xCoord+1, yCoord, &max_size_result_node_ptr, &parent_result_node_ptr, count);
						}
						else
						{	// use lower path
							add_result_node(xCoord, yCoord+1, &max_size_result_node_ptr, &parent_result_node_ptr, count);
							add_result_node(xCoord+1, yCoord+1, &max_size_result_node_ptr, &parent_result_node_ptr, count);
						}
					}
					break;
	
		case 2:
					err_when(!mc);
					if(mc)
						add_result_node(xCoord, yCoord+1, &max_size_result_node_ptr, &parent_result_node_ptr, count);
					// else, error

					err_when(ma+mb+md>1);  //(ma&&mb) || (ma&&md) || (mb&&md))
					if(ma||mb||md) // at most one is 1
						add_result_node(xCoord+1-ma, yCoord+md, &max_size_result_node_ptr, &parent_result_node_ptr, count);
					break;
		
		case 3:
					if(!furtherCheck)
					{
						add_result_node(xCoord+1-mc, yCoord+1, &max_size_result_node_ptr, &parent_result_node_ptr, count);

						err_when(ma&&mb);
						if(ma||mb) // at most one is 1
							add_result_node(xCoord+1-ma, yCoord, &max_size_result_node_ptr, &parent_result_node_ptr, count);
					}
					else
					{	
						if(parent_result_node_ptr->node_x == xCoord)
						{	// use left path
							add_result_node(xCoord, yCoord+1, &max_size_result_node_ptr, &parent_result_node_ptr, count);
							add_result_node(xCoord, yCoord, &max_size_result_node_ptr, &parent_result_node_ptr, count);
						}
						else
						{	// use right path
							add_result_node(xCoord+1, yCoord+1, &max_size_result_node_ptr, &parent_result_node_ptr, count);
							add_result_node(xCoord+1, yCoord, &max_size_result_node_ptr, &parent_result_node_ptr, count);
						}
					}
					break;

		case 4:
					err_when(!md);
					if(md)
						add_result_node(xCoord+1, yCoord+1, &max_size_result_node_ptr, &parent_result_node_ptr, count);
					// else, error

					err_when(ma+mb+mc>1); //(ma&&mb) || (ma&&mc) || (mb&&mc))
					if(ma||mb||mc) // at most one is 1
						add_result_node(xCoord+mb, yCoord+mc, &max_size_result_node_ptr, &parent_result_node_ptr, count);
					break;

		case 5:
					if(!furtherCheck)
					{
						add_result_node(xCoord+1, yCoord+1-mb, &max_size_result_node_ptr, &parent_result_node_ptr, count);
						err_when(ma&&mc);
						if(ma||mc) // at most one is 1
							add_result_node(xCoord, yCoord+1-ma, &max_size_result_node_ptr, &parent_result_node_ptr, count);
					}
					else
					{	
						if(parent_result_node_ptr->node_y == yCoord)
						{	// use upper path
							add_result_node(xCoord+1, yCoord, &max_size_result_node_ptr, &parent_result_node_ptr, count);
							add_result_node(xCoord, yCoord, &max_size_result_node_ptr, &parent_result_node_ptr, count);
						}
						else
						{	// use lower path
							add_result_node(xCoord+1, yCoord+1, &max_size_result_node_ptr, &parent_result_node_ptr, count);
							add_result_node(xCoord, yCoord+1, &max_size_result_node_ptr, &parent_result_node_ptr, count);
						}
					}
					break;
		
		case 6:
					err_when(!mb);
					if(mb)
						add_result_node(xCoord+1, yCoord, &max_size_result_node_ptr, &parent_result_node_ptr, count);
					// else, error

					err_when(ma+mc+md>1);//(ma&&mc) || (ma&&md) || (mc&&md))
					if(ma||mc||md) // at most one is 1
						add_result_node(xCoord+md, yCoord+1-ma, &max_size_result_node_ptr, &parent_result_node_ptr, count);
					break;

		case 7:
					if(!furtherCheck)
					{
						add_result_node(xCoord+1-ma, yCoord, &max_size_result_node_ptr, &parent_result_node_ptr, count);
						err_when(mc&&md);
						if(mc||md) // at most one is 1
							add_result_node(xCoord+1-mc, yCoord+1, &max_size_result_node_ptr, &parent_result_node_ptr, count);
					}
					else
					{	
						if(parent_result_node_ptr->node_x == xCoord)
						{	// use left path
							add_result_node(xCoord, yCoord, &max_size_result_node_ptr, &parent_result_node_ptr, count);
							add_result_node(xCoord, yCoord+1, &max_size_result_node_ptr, &parent_result_node_ptr, count);
						}
						else
						{	// use right path
							add_result_node(xCoord+1, yCoord, &max_size_result_node_ptr, &parent_result_node_ptr, count);
							add_result_node(xCoord+1, yCoord+1, &max_size_result_node_ptr, &parent_result_node_ptr, count);
						}
					}
					break;
		
		case 8:
					err_when(!ma);
					if(ma)
						add_result_node(xCoord, yCoord, &max_size_result_node_ptr, &parent_result_node_ptr, count);
					// else, error

					err_when(mb+mc+md>1);//(mb&&mc) || (mb&&md) || (mc&&md))
					if(mb||mc||md) // at most one is 1
						add_result_node(xCoord+1-mc, yCoord+1-mb, &max_size_result_node_ptr, &parent_result_node_ptr, count);
					break;

		default:
					err_now("error in extracting answer");
					break;
	}
}
//------ End of function SeekPath::get_real_result_node ---------//


//-------- Begin of function SeekPath::smooth_the_path ---------//
ResultNode* SeekPath::smooth_the_path(ResultNode* nodeArray, int& nodeCount)
{
	//------------------------------------------------------------//
	// smoothing the path and get the turning point
	//------------------------------------------------------------//
	//---------- declare variables ---------------//
	int i, j, checkedNodeCount, curNodeCount;
	short vectorX, vectorY, newVectorX, newVectorY, newVectorX2, newVectorY2;
	short changed, caseNum, processed;
	ResultNode *curUsefulNodePtr = nodeArray+1;
	ResultNode *preCheckingNodePtr = nodeArray;
	ResultNode *ptr1;
	ResultNode *ptr2;
	ResultNode *ptr3;
	ResultNode *ptr4;

	parent_result_node_ptr = nodeArray;
	max_size_result_node_ptr = nodeArray+1;

	//--------------------------------------------------//
	// to remove the duplicated or useless node near the 
	// destination node
	//
	// note: preCheckingNodePtr points at the previous node of
	//			max_size_result_node_ptr pointed at
	//--------------------------------------------------//
	i = 1;
	while(abs(parent_result_node_ptr->node_x-max_size_result_node_ptr->node_x)<=1 &&
			abs(parent_result_node_ptr->node_y-max_size_result_node_ptr->node_y)<=1)
	{
		max_size_result_node_ptr++;
		preCheckingNodePtr++;
		i++;
		if(i>=node_count)
			break;

		//if(i%40==0)
		//	sys_yield();
	}
	max_size_result_node_ptr = preCheckingNodePtr;
	
	//--------------------------------------------------//
	// to smooth the path
	//--------------------------------------------------//
	changed = 1;
	checkedNodeCount = 1;
	curNodeCount = node_count-i+2;
	ptr1 = parent_result_node_ptr;
	ptr2 = max_size_result_node_ptr;
	ptr3 = max_size_result_node_ptr+1;

#ifdef DEBUG
	int countEnterWhile = 0;
#endif

	//int yieldCount = 0;

	while(changed && curNodeCount>=3)
	{
#ifdef DEBUG
		countEnterWhile++;
#endif
		
		//yieldCount++;
		//if(yieldCount%30==0)
		//	sys_yield();

		vectorX = ptr2->node_x - ptr1->node_x;
		vectorY = ptr2->node_y - ptr1->node_y;
		changed = 0;
		curUsefulNodePtr = nodeArray+1;
		checkedNodeCount = 1;
		
		for(j=1; j<curNodeCount-1; j++)
		{
			processed = 0;
			newVectorX= ptr3->node_x - ptr2->node_x;
			newVectorY= ptr3->node_y - ptr2->node_y;
					
			//------ turning at 90 degree clockwise / anti-clockwise --------//
			if((vectorX==0 && vectorY!=0 && newVectorX!=0 && newVectorY==0) ||	// + case
				(vectorX!=0 && vectorY==0 && newVectorX==0 && newVectorY!=0))
			{
				ptr2++;
				err_when(abs(ptr1->node_x-ptr2->node_x)>1 || abs(ptr1->node_y-ptr2->node_y)>1);
				ptr3++;
				err_when(j<curNodeCount-2 && ptr3->node_x && ptr3->node_y &&
							(abs(ptr3->node_x-ptr2->node_x)>1 || abs(ptr3->node_y-ptr2->node_y)>1));
				vectorX = ptr2->node_x - ptr1->node_x;
				vectorY = ptr2->node_y - ptr1->node_y;
				processed = 1;
				continue;
			}

			if((vectorX!=0 && vectorY!=0 && newVectorX!=0 && newVectorY!=0) &&		//	x case
				((vectorX==newVectorX && vectorY==-newVectorY) || (vectorX==-newVectorX && vectorY==newVectorY)) &&
				can_move_to((ptr1->node_x+ptr3->node_x)/2, (ptr1->node_y+ptr3->node_y)/2))
			{
				err_when(ptr1->node_x==ptr3->node_x && ptr1->node_y==ptr3->node_y);
				ptr2->node_x = (ptr1->node_x+ptr3->node_x)/2;
				ptr2->node_y = (ptr1->node_y+ptr3->node_y)/2;
				*curUsefulNodePtr = *ptr2;
				err_when(abs(ptr1->node_x-curUsefulNodePtr->node_x)>1 || abs(ptr1->node_y-curUsefulNodePtr->node_y)>1);
				curUsefulNodePtr++;
				checkedNodeCount++;
				ptr1 = ptr2;
				ptr2 = ptr3;
				err_when(abs(ptr1->node_x-ptr2->node_x)>1 || abs(ptr1->node_y-ptr2->node_y)>1);
				ptr3++;
				err_when(j<curNodeCount-2 && ptr3->node_x && ptr3->node_y &&
							(abs(ptr3->node_x-ptr2->node_x)>1 || abs(ptr3->node_y-ptr2->node_y)>1));
				vectorX = ptr2->node_x - ptr1->node_x;
				vectorY = ptr2->node_y - ptr1->node_y;
				processed = 1;
				continue;
			}

			if(j<curNodeCount-2)	//	check for the 4-point case
			{
				ptr4 = ptr3+1;
				newVectorX2 = ptr4->node_x - ptr3->node_x;
				newVectorY2	= ptr4->node_y - ptr3->node_y;
				caseNum = 0;

				if(vectorX==-1 && vectorY==0 && newVectorX==-1 && newVectorX2==0)
				{
					if(newVectorY==1 && newVectorY2==1 && can_move_to(ptr2->node_x, ptr3->node_y))
						caseNum = 1; //-------- (0,0), (-1,0), (-2,1), (-2,2) --------//
					else if(newVectorY==-1 && newVectorY2==-1 && can_move_to(ptr2->node_x, ptr3->node_y))
						caseNum = 5; //-------- (0,0), (-1,0), (-2,-1), (-2,-2) --------//
				}
				else if(vectorX==1 && vectorY==0 && newVectorX==1 && newVectorX2==0)
				{	
					if(newVectorY==1 && newVectorY2==1 && can_move_to(ptr2->node_x, ptr3->node_y))
						caseNum = 3; //-------- (0,0), (1,0), (2,1), (2,2) --------//
					else if(newVectorY==-1 && newVectorY2==-1 && can_move_to(ptr2->node_x, ptr3->node_y))
						caseNum = 7; //-------- (0,0), (1,0), (2,-1), (2,-2) --------//
				}
				else if(vectorX==0 && vectorY==-1 && newVectorY==-1 && newVectorY2==0)
				{
					if(newVectorX==1 && newVectorX2==1 && can_move_to(ptr3->node_x, ptr2->node_y))
						caseNum = 2; //-------- (0,0), (0,-1), (1,-2), (2,-2) --------//
					else if(newVectorX==-1 && newVectorX2==-1 && can_move_to(ptr3->node_x, ptr2->node_y))
						caseNum = 4; //-------- (0,0), (0,-1), (-1,-2), (-2,-2) --------//
				}
				else if(vectorX==0 && vectorY==1 && newVectorY==1 && newVectorY2==0)
				{
					if(newVectorX==1 && newVectorX2==1 && can_move_to(ptr3->node_x, ptr2->node_y))
						caseNum = 6; //-------- (0,0), (0,1), (1,2), (2,2) --------//
					else if(newVectorX==-1 && newVectorX2==-1 && can_move_to(ptr3->node_x, ptr2->node_y))
						caseNum = 8; //-------- (0,0), (0,1), (-1,2), (-2,2) --------//
				}

				if(caseNum)
				{
					if(caseNum%2) // case 1, 3, 5 7
						ptr2->node_y = ptr3->node_y;
					else // case 2, 4, 6, 8
						ptr2->node_x = ptr3->node_x;

					ptr3++; //ptr3 = ptr4;
					*curUsefulNodePtr = *ptr2;
					err_when(abs(ptr1->node_x-curUsefulNodePtr->node_x)>1 || abs(ptr1->node_y-curUsefulNodePtr->node_y)>1);
					curUsefulNodePtr++;
					checkedNodeCount++;
					ptr1 = ptr2;
					ptr2 = ptr3;
					err_when(abs(ptr1->node_x-ptr2->node_x)>1 || abs(ptr1->node_y-ptr2->node_y)>1);
					ptr3++;
					err_when(j<curNodeCount-2 && ptr3->node_x && ptr3->node_y &&
								(abs(ptr3->node_x-ptr2->node_x)>1 || abs(ptr3->node_y-ptr2->node_y)>1));
					j++;
					vectorX = ptr2->node_x - ptr1->node_x;
					vectorY = ptr2->node_y - ptr1->node_y;
					processed = 1;
					continue;
				}
			}

			//------ none of the above case ---------//
			if(!processed)
			{
				*curUsefulNodePtr = *ptr2;
				err_when(abs(ptr1->node_x-curUsefulNodePtr->node_x)>1 || abs(ptr1->node_y-curUsefulNodePtr->node_y)>1);
				curUsefulNodePtr++;
				checkedNodeCount++;
			}
			else 
				changed = 1;

			ptr1 = ptr2;
			ptr2 = ptr3;
			err_when(abs(ptr1->node_x-ptr2->node_x)>1 || abs(ptr1->node_y-ptr2->node_y)>1);
			ptr3++;
			err_when(j<curNodeCount-2 && ptr3->node_x && ptr3->node_y &&
						(abs(ptr3->node_x-ptr2->node_x)>1 || abs(ptr3->node_y-ptr2->node_y)>1));
			vectorX = ptr2->node_x - ptr1->node_x;
			vectorY = ptr2->node_y - ptr1->node_y;
		}
		//---- end checking and then reset parameters----//

		if(processed)
			changed = 1;
		
		*curUsefulNodePtr = *ptr2;
		err_when(abs(ptr1->node_x-curUsefulNodePtr->node_x)>1 || abs(ptr1->node_y-curUsefulNodePtr->node_y)>1);
		curUsefulNodePtr++;
		checkedNodeCount++;
		
		curNodeCount = checkedNodeCount;
		checkedNodeCount = 1;
		ptr1 = parent_result_node_ptr;
		ptr2 = ptr1+1;
		err_when(abs(ptr1->node_x-ptr2->node_x)>1 || abs(ptr1->node_y-ptr2->node_y)>1);
		ptr3 = ptr2+1;
		err_when(j<curNodeCount-2 && ptr3->node_x && ptr3->node_y &&
					(abs(ptr3->node_x-ptr2->node_x)>1 || abs(ptr3->node_y-ptr2->node_y)>1));

		debug_check_path(nodeArray, nodeCount); //*** debug only
	}
	node_count = curNodeCount;

	//--------------------------------------------------//
	// to remove the duplicated destination node
	//--------------------------------------------------//
	parent_result_node_ptr = nodeArray;
	max_size_result_node_ptr = nodeArray+1;
	ResultNode* result_node_pointer = max_size_result_node_ptr;

	//----------------------------------
	// get the turning point 
	//----------------------------------
	vectorX = max_size_result_node_ptr->node_x - parent_result_node_ptr->node_x;
	vectorY = max_size_result_node_ptr->node_y - parent_result_node_ptr->node_y;

	for(i=1; i<node_count-1; i++)
	{
		parent_result_node_ptr = max_size_result_node_ptr; // don't use parent_result_node_ptr++, if the above code of removing duplication is used.
		max_size_result_node_ptr++;
		newVectorX=(max_size_result_node_ptr->node_x-parent_result_node_ptr->node_x);
		newVectorY=(max_size_result_node_ptr->node_y-parent_result_node_ptr->node_y);
		err_when(newVectorY!=0 && newVectorX!=0 && abs(newVectorX)!=abs(newVectorY));

		//------ turning to another direction at this point ------//
		if(vectorX!=newVectorX || vectorY!=newVectorY)
		{	
			err_when(abs(newVectorX)>1 || abs(newVectorY)>1);// || (newVectorX==0 && newVectorY==0))
			if(newVectorX!=0 || newVectorY!=0)
			{
				*result_node_pointer = *parent_result_node_ptr;
				result_node_pointer++;
				nodeCount++;

				vectorX = newVectorX;
				vectorY = newVectorY;
			}
		}

		//if(i%30==0)
		//	sys_yield();
	}

	result_node_pointer->node_x = real_sour_x;
	result_node_pointer->node_y = real_sour_y;
	result_node_pointer++;
	nodeCount++;

	return nodeArray;
}	
//------- End of function SeekPath::smooth_the_path -------//


//-------- Begin of function Node::generate_successors ---------//
// Note: In fact, the cost of the starting node should be 0 or 1
//			and the parentEnterDirection is 0.	Now the cost in this
//			case is set to 2.	The difference can be ignored as it will
//			not affect the search after generating the second level
//			children.
//			The advantage to ignore this case is that less comparsion
//			effort in checking parentEnterDirection.
//
short Node::generate_successors(short parentEnterDirection, short realSourX, short realSourY)
{
	int hasLeft  = node_x > cur_border_x1;
	int hasRight = node_x < cur_border_x2;
	int hasUp	 = node_y > cur_border_y1;
	int hasDown  = node_y < cur_border_y2;
	int upperLeftX,upperLeftY;
	short cost;

	upperLeftX = node_x<<1;
	upperLeftY = node_y<<1;

	//-------------------------------------------
	// enter_direction = (exit_direction+3)%8+1
	//-------------------------------------------

	if( hasLeft )
	{
		//--------- Left, exit_direction=1 --------//
		if ((node_type&0x05) && (can_move_to(upperLeftX-1,upperLeftY) || can_move_to(upperLeftX-1,upperLeftY+1)))
		{	
			if(parentEnterDirection==2 || parentEnterDirection==8 ||
			   (node_type&0x1 && parentEnterDirection==7) || (node_type&0x4 && parentEnterDirection==3) ||
				(!parentEnterDirection && realSourX == upperLeftX))
				cost = 1;
			else
				cost = 2;

			if(generate_succ(node_x-1,node_y,5,cost))
				return 1;
		}

		if( hasUp )
		{
			//------- Upper-Left, exit_direction=8 ---------//
			if ((node_type&0x1) && can_move_to(upperLeftX-1,upperLeftY-1))
			{
				if(parentEnterDirection==7 || parentEnterDirection==1 ||
					(!parentEnterDirection && realSourX==upperLeftX && realSourY==upperLeftY))
				  cost = 1;
				else cost = 2;

				if(generate_succ(node_x-1,node_y-1,4,cost))
					return 1;
			}
		}

		if( hasDown )
		{
			//--------- Lower-Left, exit_direction=2 ----------//
			if ((node_type&0x4) && can_move_to(upperLeftX-1,upperLeftY+2))
			{	if(parentEnterDirection==1 || parentEnterDirection==3 ||
					(!parentEnterDirection && realSourX==upperLeftX && realSourY==(upperLeftY+1)))
				  cost = 1;
				else cost = 2;

				if(generate_succ(node_x-1,node_y+1,6,cost))
					return 1;
			}
		}
	}

	if( hasRight )
	{
		//----------- Right, exit_direction=5 -----------//
		if ((node_type&0xA) && (can_move_to(upperLeftX+2,upperLeftY) || can_move_to(upperLeftX+2,upperLeftY+1)))
		{	
			if(parentEnterDirection==4 || parentEnterDirection==6 ||
				(node_type&0x02 && parentEnterDirection==7) || (node_type&0x08 && parentEnterDirection==3) ||
				(!parentEnterDirection && realSourX==(upperLeftX+1)))
				cost = 1;
			else cost = 2;

			if(generate_succ(node_x+1,node_y,1,cost))
				return 1;
		}

		if( hasUp )
		{
			//-------- Upper-Right, exit_direction=6 ---------//

			if ((node_type&0x2)&&can_move_to(upperLeftX+2,upperLeftY-1))
			{  
				if(parentEnterDirection==5 || parentEnterDirection==7 ||
					(!parentEnterDirection && realSourX==(upperLeftX+1) && realSourY==upperLeftY))
				  cost = 1;
				else cost = 2;

				if(generate_succ(node_x+1,node_y-1,2,cost))
					return 1;
			}
		}

		if( hasDown )
		{
			//--------- Lower-Right, exit_direction=4 ---------//

			if ((node_type&0x8)&&can_move_to(upperLeftX+2,upperLeftY+2))
			{  
				if(parentEnterDirection==3 || parentEnterDirection==5 ||
					(!parentEnterDirection && realSourX==(upperLeftX+1) && realSourY==(upperLeftY+1)))
				  cost = 1;
				else cost = 2;

				if(generate_succ(node_x+1,node_y+1,8,cost))
					return 1;
			}
		}
	}

	if( hasUp )
	{  
		//---------- Upper, exit_direction=7 -----------//
		if ((node_type&0x03) && (can_move_to(upperLeftX,upperLeftY-1) || can_move_to(upperLeftX+1,upperLeftY-1)))
		{	
			if(parentEnterDirection==6 || parentEnterDirection==8 ||
			   (node_type&0x01 && parentEnterDirection==1) || (node_type&0x02 && parentEnterDirection==5) ||
				(!parentEnterDirection && realSourY==upperLeftY))
				cost = 1;
			else cost = 2;

			if(generate_succ(node_x,node_y-1,3,cost))
				return 1;
		}
	}
	
	if( hasDown )
	{	
		//---------- Lower, exit_direction=3 -----------// 
		if ((node_type&0xC) && ( can_move_to(upperLeftX,upperLeftY+2) || can_move_to(upperLeftX+1,upperLeftY+2) ) )
		{	
			if(parentEnterDirection==2 || parentEnterDirection==4 ||
			   (node_type&0x4 && parentEnterDirection==1) || (node_type&0x8 && parentEnterDirection==5) ||
				(!parentEnterDirection && realSourY==(upperLeftY+1)))
				cost = 1;
			else cost = 2;

			if(generate_succ(node_x,node_y+1,7,cost))
				return 1;
		}
	}
	return 0;
}
//------- End of function Node::generate_successors -------//


//-------- Begin of function Node::generate_succ ---------//
short Node::generate_succ(short x, short y, short enter_direct,short cost)
{
	//----- if it points back to this node's parent ----//
	if( parent_node )
	{
		if( parent_node->node_x==x && parent_node->node_y==y )
			return 0;
	}

	//----- if there is an existing node at the given position ----//
	int upperLeftX, upperLeftY;
	//int cost;
	short c, g = node_g+cost;	    	 // g(Successor)=g(BestNode)+cost of getting from BestNode to Successor
	short nodeRecno;

	if( (nodeRecno=cur_node_matrix[y*MAX_WORLD_X_LOC/2+x]) > 0 &&
		 nodeRecno<max_node_num)
	{
		Node* oldNode = cur_node_array+nodeRecno-1;

		//------ Add oldNode to the list of BestNode's child_noderen (or Successors).
		for(c=0 ; c<MAX_CHILD_NODE && child_node[c] ; c++);
		child_node[c]=oldNode;

		//---- if our new g value is < oldNode's then reset oldNode's parent to point to BestNode
		if(g < oldNode->node_g)
		{
			oldNode->parent_node = this;
			oldNode->node_g 	   = g;
			oldNode->node_f	 	= g+oldNode->node_h;
			oldNode->enter_direction = (char)enter_direct;

			//-------- if it's a closed node ---------//
			if(oldNode->child_node[0] )
			{
				//-------------------------------------------------//
				// Since we changed the g value of oldNode, we need
				// to propagate this new value downwards, i.e.
				// do a Depth-First traversal of the tree!
				//-------------------------------------------------//
				oldNode->propagate_down();
				//sys_yield();
			}
		}
	}
	else //------------ add a new node -----------//
	{
		Node* succNode = cur_node_array + cur_seek_path->node_count++;

		memset(succNode, 0, sizeof(Node));

		succNode->parent_node = this;
		succNode->node_g = g;
		succNode->node_h = (x-cur_dest_x)*(x-cur_dest_x)+(y-cur_dest_y)*(y-cur_dest_y); // should do sqrt(), but since we don't really
		succNode->node_f = g+succNode->node_h;     		// care about the distance but just which branch looks
		succNode->node_x = x;                  			// better this should suffice. Anyayz it's faster.
		succNode->node_y = y;
		succNode->enter_direction = (char)enter_direct;
		upperLeftX = x<<1;
		upperLeftY = y<<1;
		succNode->node_type = can_move_to(upperLeftX,upperLeftY)+(can_move_to(upperLeftX+1,upperLeftY)<<1)+
									(can_move_to(upperLeftX,upperLeftY+1)<<2)+(can_move_to(upperLeftX+1,upperLeftY+1)<<3);

		if(search_mode==SEARCH_MODE_REUSE && nodeRecno>max_node_num)	// path-reuse node found, but checking of can_walk(final_dest_?) is requested
		{
			int destIndex = nodeRecno - max_node_num;
			switch(destIndex)
			{
				case 1:	final_dest_x = x<<1;
							final_dest_y = y<<1;
							break;

				case 2:	final_dest_x = (x<<1) + 1;
							final_dest_y = y<<1;
							break;

				case 3:	final_dest_x = x<<1;
							final_dest_y = (y<<1) + 1;
							break;

				case 4:	final_dest_x = (x<<1) + 1;
							final_dest_y = (y<<1) + 1;
							break;

				default: err_here();
							break;
			}

			if(can_move_to(final_dest_x, final_dest_y))	// can_walk the connection point, accept this case
			{
				reuse_result_node_ptr = succNode;
				return 1;
			}	// else continue until reuse node is found and connection point can be walked
		}

		cur_node_matrix[y*MAX_WORLD_X_LOC/2+x] = cur_seek_path->node_count;
		cur_seek_path->open_node_list.insert_node(succNode);
		for(c=0 ; c<MAX_CHILD_NODE && child_node[c] ; c++);   // Add oldNode to the list of BestNode's child_noderen (or succNodes).
		child_node[c]=succNode;
	}

	return 0;
}
//------- End of function Node::generate_succ -------//


//-------- Begin of function Node::propagate_down ---------//
void Node::propagate_down()
{
	Node *childNode, *fatherNode;
	int 	c, g=node_g;            // alias.
	short	cost;
	char	xShift, yShift;	// the x, y difference between parent and child nodes

	char childEnterDirection;
	char exitDirection;
	char testResult;
	
	for(c=0;c<8;c++)
	{
		if ((childNode=child_node[c])==NULL)   // create alias for faster access.
			break;

		cost = 2; // in fact, may be 1 or 2
		if (g+cost <= childNode->node_g) // first checking
		{
			xShift = childNode->node_x-node_x;
			yShift = childNode->node_y-node_y;
			err_when(abs(xShift)>1 || abs(yShift)>1);
			//---------- calulate the new enter direction ------------//
			switch(yShift)
			{	
				case -1:
							childEnterDirection = char(3-xShift);	
							break;

				case 0:
							childEnterDirection = char(3-(xShift<<1));
							break;

				case 1:
							childEnterDirection = char(7+xShift);
							break;
			}

			exitDirection = (childEnterDirection+3)%8+1;
			
			if(enter_direction > exitDirection)
			{
				if((enter_direction==8 && (exitDirection==1 || exitDirection==2)) ||
					(enter_direction==7 && exitDirection==1))
					testResult = exitDirection + 8 - enter_direction;
				else
					testResult = enter_direction - exitDirection;
			}
			else
			{
				if((exitDirection==8 && (enter_direction==1 || enter_direction==2)) ||
					(exitDirection==7 && enter_direction==1))
					testResult = enter_direction + 8 - exitDirection;
				else
					testResult = exitDirection - enter_direction;
			}

			if(exitDirection%2 && testResult==2)
			{
				int upperLeftX = 2*node_x;
				int upperLeftY = 2*node_y;
				// this case only occurs at the edge
				switch(childEnterDirection)
				{
					case 1:	if((exitDirection==3 && can_move_to(upperLeftX, upperLeftY+1)) ||
									(exitDirection==7 && can_move_to(upperLeftX, upperLeftY)))
									cost = 1;
								break;

					case 3:	if((exitDirection==1 && can_move_to(upperLeftX, upperLeftY+1)) ||
									(exitDirection==5 && can_move_to(upperLeftX+1, upperLeftY+1)))
									cost = 1;
								break;

					case 5:	if((exitDirection==3 && can_move_to(upperLeftX+1, upperLeftY+1)) ||
									(exitDirection==7 && can_move_to(upperLeftX+1, upperLeftY)))
									cost = 1;
								break;
								
					case 7:	if((exitDirection==1 && can_move_to(upperLeftX, upperLeftY)) ||
									(exitDirection==5 && can_move_to(upperLeftX+1, upperLeftY)))
									cost = 1;
								break;

					default:	err_here();
								break;
				}
			}
			else
				cost = 2 - (testResult <= 1); 	//if(testResult <= 1) cost = 1;
			
			err_when(cost>2 || cost<1);
			if(g+cost < childNode->node_g) // second checking, mainly for cost = 2;
			{
				childNode->node_g 	  = g+cost;
				childNode->node_f 	  = childNode->node_g+childNode->node_h;
				childNode->parent_node = this;// reset parent to new path.
				childNode->enter_direction = childEnterDirection;
				stack_push(childNode);			// Now the childNode's branch need to be checked out. Remember the new cost must be propagated down.
			}
		}     
	}

	while(cur_stack_pos>0)
	{
		fatherNode=stack_pop();
		g = fatherNode->node_g;

		for(c=0;c<8;c++)
		{
			if((childNode=fatherNode->child_node[c])==NULL)       // we may stop the propagation 2 ways: either
				break;

			cost = 2; // in fact, may be 1 or 2
			if(g+cost <= childNode->node_g)	// first checking
				// there are no children, or that the g value of the child is equal or better than the cost we're propagating
			{	
				xShift = childNode->node_x-fatherNode->node_x;
				yShift = childNode->node_y-fatherNode->node_y;
				err_when(abs(xShift)>1 || abs(yShift)>1);
				//---------- calulate the new enter direction ------------//
				switch(yShift)
				{	
					case -1:
								childEnterDirection = char(3-xShift);	
								break;

					case 0:
								childEnterDirection = char(3-(xShift<<1));
								break;

					case 1:
								childEnterDirection = char(7+xShift);
								break;
				}

				exitDirection = (childEnterDirection+3)%8+1;
			
				char fatherEnterDirection = fatherNode->enter_direction;
				if(fatherEnterDirection > exitDirection)
				{
					if((fatherEnterDirection==8 && (exitDirection==1 || exitDirection==2)) ||
						(fatherEnterDirection==7 && exitDirection==1))
						testResult = exitDirection + 8 - fatherEnterDirection;
					else
						testResult = fatherEnterDirection - exitDirection;
				}
				else
				{
					if((exitDirection==8 && (fatherEnterDirection==1 || fatherEnterDirection==2)) ||
						(exitDirection==7 && fatherEnterDirection==1))
						testResult = fatherEnterDirection + 8 - exitDirection;
					else
						testResult = exitDirection - fatherEnterDirection;
				}

				if(exitDirection%2 && testResult==2)
				{
					int upperLeftX = 2*fatherNode->node_x;
					int upperLeftY = 2*fatherNode->node_y;
					// this case only occurs at the edge
					switch(childEnterDirection)
					{
						case 1:	if((exitDirection==3 && can_move_to(upperLeftX, upperLeftY+1)) ||
										(exitDirection==7 && can_move_to(upperLeftX, upperLeftY)))
										cost = 1;
									break;

						case 3:	if((exitDirection==1 && can_move_to(upperLeftX, upperLeftY+1)) ||
										(exitDirection==5 && can_move_to(upperLeftX+1, upperLeftY+1)))
										cost = 1;
									break;

						case 5:	if((exitDirection==3 && can_move_to(upperLeftX+1, upperLeftY+1)) ||
										(exitDirection==7 && can_move_to(upperLeftX+1, upperLeftY)))
										cost = 1;
									break;
								
						case 7:	if((exitDirection==1 && can_move_to(upperLeftX, upperLeftY)) ||
										(exitDirection==5 && can_move_to(upperLeftX+1, upperLeftY)))
										cost = 1;
									break;

						default:	err_here();
									break;
					}
				}
				else
					cost = 2 - (testResult <= 1); 	//if(testResult <= 1) cost = 1;

				err_when(cost>2 || cost<1);
				if(g+cost < childNode->node_g) // second checking, mainly for cost = 2;
				{
					childNode->node_g 	  = g+cost;
					childNode->node_f 	  = childNode->node_g+childNode->node_h;
					childNode->parent_node = fatherNode;
					childNode->enter_direction = childEnterDirection;
					stack_push(childNode);
				}
			}
		}
	}
}
//------- End of function Node::propagate_down -------//


//-------- Begin of static function stack_push ---------//
static void stack_push(Node *nodePtr)
{
	stack_array[cur_stack_pos++] = nodePtr;

	err_when( cur_stack_pos >= MAX_STACK_NUM );
}
//--------- End of static function stack_push ---------//


//-------- Begin of static function stack_pop ---------//
static Node* stack_pop()
{
	return stack_array[--cur_stack_pos];
}
//--------- End of static function stack_pop ---------//

//-********************************************************************************-//
// for UNIT_SEA and UNIT_AIR, the scale used is double that of UNIT_LAND
//-********************************************************************************-//

//-------- Begin of static function SeekPath::seek2 ---------//
int SeekPath::seek2(int sx, int sy, int dx, int dy, short miscNo, short numOfPath, int maxTries)
{
	err_when(mobile_type==UNIT_LAND);

	//---------------------------------------------------------------------------//
	// target_recno, building_id is not implemented for this version of seek2()
	//---------------------------------------------------------------------------//
	target_recno = building_id = 0;
	building_x1 = building_y1 = building_x2 = building_y2 = -1;

	switch(search_mode)
	{
		case SEARCH_MODE_TO_FIRM:
				building_id = miscNo;
				building_x1 = dx; // upper left corner location
				building_y1 = dy;
				search_firm_info = firm_res[building_id];
				building_x2 = dx+search_firm_info->loc_width-1;
				building_y2 = dy+search_firm_info->loc_height-1;
				break;

		case SEARCH_MODE_TO_TOWN:
				building_id = miscNo;
				building_x1 = dx; // upper left corner location
				building_y1 = dy;
				if(miscNo != -1)
				{
					Location* buildPtr = world.get_loc(dx, dy);
					Town* targetTown = town_array[buildPtr->town_recno()];
					building_x2 = targetTown->loc_x2;
					building_y2 = targetTown->loc_y2;
				}
				else	// searching to settle. Detail explained in function set_move_to_surround()
				{
					building_x2 = building_x1+STD_TOWN_LOC_WIDTH-1;
					building_y2 = building_y1+STD_TOWN_LOC_HEIGHT-1;
				}
				break;

		case SEARCH_MODE_ATTACK_UNIT_BY_RANGE:
		case SEARCH_MODE_ATTACK_WALL_BY_RANGE:
				err_when(attack_range==0);
				building_id = miscNo;
				building_x1 = MAX(dx-attack_range, 0);
				building_y1 = MAX(dy-attack_range, 0);
				building_x2 = MIN(dx+attack_range, MAX_WORLD_X_LOC-1);
				building_y2 = MIN(dy+attack_range, MAX_WORLD_Y_LOC-1);
				break;

		case SEARCH_MODE_ATTACK_FIRM_BY_RANGE:
				building_id = miscNo;
				building_x1 = MAX(dx-attack_range, 0);
				building_y1 = MAX(dy-attack_range, 0);
				search_firm_info = firm_res[building_id];
				building_x2 = MIN(dx+search_firm_info->loc_width-1+attack_range, MAX_WORLD_X_LOC-1);
				building_y2 = MIN(dy+search_firm_info->loc_height-1+attack_range, MAX_WORLD_Y_LOC-1);
				break;

		case SEARCH_MODE_ATTACK_TOWN_BY_RANGE:
				building_id = miscNo;
				building_x1 = MAX(dx-attack_range, 0);
				building_y1 = MAX(dy-attack_range, 0);
				building_x2 = MIN(dx+STD_TOWN_LOC_WIDTH-1+attack_range, MAX_WORLD_X_LOC-1);
				building_y2 = MIN(dy+STD_TOWN_LOC_HEIGHT-1+attack_range, MAX_WORLD_Y_LOC-1);
				break;
	
		case SEARCH_MODE_TO_LAND_FOR_SHIP:
				region_id = (UCHAR)miscNo;
				break;
	}
	
	//------------------------------------------------------------------------------//
	// set start location and destination location
	//------------------------------------------------------------------------------//
	real_sour_x = sx;
	real_sour_y = sy;
	//final_dest_x = real_dest_x = dx;
	//final_dest_y = real_dest_y = dy;
	real_dest_x = dx;
	real_dest_y = dy;
	
	int xShift, yShift, area;
	short pathNum;
	switch(search_mode)
	{
		case SEARCH_MODE_TO_FIRM:
		case SEARCH_MODE_TO_TOWN:
				final_dest_x = (building_x1+building_x2)/2;
				final_dest_y = (building_y1+building_y2)/2;

				//---------------------------------------------------------------------------------//
				// for group assign, the final destination is adjusted by the value of numOfPath
				//---------------------------------------------------------------------------------//
				if(search_mode==SEARCH_MODE_TO_TOWN)
					area = STD_TOWN_LOC_WIDTH*STD_TOWN_LOC_HEIGHT;
				else // search_mode == SEARCH_MODE_TO_FIRM
					area = search_firm_info->loc_width*search_firm_info->loc_height;

				pathNum = (numOfPath>area) ? (numOfPath-1)%area + 1 : numOfPath;
				if(search_mode==SEARCH_MODE_TO_TOWN)
					m.cal_move_around_a_point(pathNum, STD_TOWN_LOC_WIDTH, STD_TOWN_LOC_HEIGHT, xShift, yShift);
				else
					m.cal_move_around_a_point(pathNum, search_firm_info->loc_width, search_firm_info->loc_height, xShift, yShift);

				final_dest_x += xShift;
				final_dest_y += yShift;

				if(final_dest_x<0)
					final_dest_x = 0;
				else if(final_dest_x>=MAX_WORLD_X_LOC)
					final_dest_x = MAX_WORLD_X_LOC-1;

				if(final_dest_y<0)
					final_dest_y = 0;
				else if(final_dest_y>=MAX_WORLD_Y_LOC)
					final_dest_y = MAX_WORLD_Y_LOC-1;
				break;

		case SEARCH_MODE_ATTACK_UNIT_BY_RANGE:
		case SEARCH_MODE_ATTACK_FIRM_BY_RANGE:
		case SEARCH_MODE_ATTACK_TOWN_BY_RANGE:
		case SEARCH_MODE_ATTACK_WALL_BY_RANGE:
				final_dest_x = (building_x1+building_x2)/2;
				final_dest_y = (building_y1+building_y2)/2;
				break;
		
		default:
				final_dest_x = real_dest_x;
				final_dest_y = real_dest_y;
				break;
	}

	//--------------------------------------------------------------//
	// change to 2x2 node format
	//--------------------------------------------------------------//
	int sourceX	= short(sx/2);	// the upper left corner is used
	int sourceY	= short(sy/2);
	dest_x	= short(final_dest_x/2);
	dest_y	= short(final_dest_y/2);

	//-----------------------------------------//
	// reset node_matrix
	//-----------------------------------------//
	if(search_mode!=SEARCH_MODE_REUSE)
	{
		max_node_num = 0xFFFF;
		memset(node_matrix, 0, sizeof(short)*MAX_WORLD_X_LOC*MAX_WORLD_Y_LOC/4);
	}
	else
	{
		max_node_num = max_node;
		memcpy(node_matrix, reuse_node_matrix_ptr, sizeof(short)*MAX_WORLD_X_LOC*MAX_WORLD_Y_LOC/4);
	}

	//--------- create the first node ---------//
	node_count  = 0;
	result_node_ptr = NULL;

	Node *nodePtr = node_array + node_count++;
	memset(nodePtr, 0, sizeof(Node));

	//-------- store the upper left coordinate of the node ----------//
	int destUpperLeftX = dest_x<<1;
	int destUpperLeftY = dest_y<<1;
	is_dest_blocked = !can_move_to(destUpperLeftX, destUpperLeftY);
	// whether the destination is blocked, if so, only search till the destination's neighbor locations

	//### begin alex 25/9 ###//
	//------------ do some adjustment for ship to beach if destination is blocked ----------------//
	if(is_dest_blocked && search_mode==SEARCH_MODE_TO_LAND_FOR_SHIP &&
		(final_dest_x%2 || final_dest_y%2)) // either is odd location
	{
		if(final_dest_x%2 && final_dest_y%2==0)
		{
			if(destUpperLeftX+2<MAX_WORLD_X_LOC-1 && can_move_to(destUpperLeftX+2, destUpperLeftY))
			{
				is_dest_blocked = 0;
				destUpperLeftX += 2;
				dest_x++;
			}
		}
		else if(final_dest_x%2==0 && final_dest_y%2)
		{
			if(destUpperLeftY+2<MAX_WORLD_Y_LOC-1 && can_move_to(destUpperLeftX, destUpperLeftY+2))
			{
				is_dest_blocked = 0;
				destUpperLeftY += 2;
				dest_y++;
			}
		}
		else
		{
			if(destUpperLeftX+2<MAX_WORLD_X_LOC-1 && can_move_to(destUpperLeftX+2, destUpperLeftY))
			{
				is_dest_blocked = 0;
				destUpperLeftX += 2;
				dest_x++;
			}
			else if(destUpperLeftX+2<MAX_WORLD_X_LOC-1 && destUpperLeftY+2<MAX_WORLD_Y_LOC-1 &&
					  can_move_to(destUpperLeftX+2, destUpperLeftY+2))
			{
				is_dest_blocked = 0;
				destUpperLeftX += 2;
				destUpperLeftY += 2;
				dest_x++;
				dest_y++;
			}
			else if(destUpperLeftY+2<MAX_WORLD_Y_LOC-1 && can_move_to(destUpperLeftX, destUpperLeftY+2))
			{
				is_dest_blocked = 0;
				destUpperLeftY += 2;
				dest_y++;
			}
		}
	}
	//#### end alex 25/9 ####//

	nodePtr->node_g = 0;
	nodePtr->node_h = (sourceX-dest_x)*(sourceX-dest_x)+(sourceY-dest_y)*(sourceY-dest_y);  // should really use sqrt().
	nodePtr->node_f = nodePtr->node_g + nodePtr->node_h;
	nodePtr->node_x = sourceX;
	nodePtr->node_y = sourceY;

	open_node_list.insert_node(nodePtr); // make Open List point to first node

	//--- if the destination is the current postion or next to it & the dest is occupied ---//
	if(sourceX==dest_x && sourceY==dest_y)
	{
		path_status 	 = PATH_FOUND;
		result_node_ptr = NULL;
		return path_status;
	}

	//------------ seek now ------------------//
	int maxNode = (!maxTries) ? max_node : maxTries;
	return continue_seek2(maxNode, 1);	// 1-first seek session of the current seek order
}
//--------- End of static function SeekPath::seek2 ---------//


//-------- Begin of static function SeekPath::continue_seek2 ---------//
int SeekPath::continue_seek2(int maxTries, char firstSeek)
{
	if( path_status != PATH_SEEKING )
		return path_status;

	//------- aliasing class member vars for fast access ------//
	cur_seek_path   = this;
	cur_dest_x	    = dest_x;
	cur_dest_y	  	 = dest_y;
	cur_node_matrix = node_matrix;
	cur_node_array  = node_array;

   cur_border_x1 	 = border_x1;
	cur_border_y1 	 = border_y1;
	cur_border_x2 	 = border_x2;
	cur_border_y2 	 = border_y2;

	//------ seek the path using the A star algorithm -----//
	int maxNode = (total_node_avail<maxTries) ? total_node_avail : maxTries;
	maxNode -= MAX_CHILD_NODE; // generate_successors() can generate a MAX of MAX_CHILD_NODE new nodes per call
	Node *bestNodePtr;

	int i;
	for(i=0; i<maxNode; i++)
	{
		bestNodePtr = return_best_node();

		//if(i%20==0)
		//	sys_yield(); // update cursor position

		//---- even if the path is impossible, get the closest path ----//
		if( !bestNodePtr )
		{
			path_status = PATH_IMPOSSIBLE;
			break;
		}

		//----- exceed the object's MAX's node limitation, return the closest path ----//
		if( node_count >= maxNode )
		//if( i >= maxNode )
		{
			path_status = PATH_NODE_USED_UP;
			break;
		}

		//------------------------------------------//
		// If the path is found OR
		//
		// If the destination is blocked,
		// consider it done when we are next to it.
		//------------------------------------------//
		if( (bestNodePtr->node_x==dest_x && bestNodePtr->node_y==dest_y) ||
			 ( is_dest_blocked && abs(bestNodePtr->node_x-dest_x)<=1 && abs(bestNodePtr->node_y-dest_y)<=1 ) )
		{
			path_status 	 = PATH_FOUND;
			result_node_ptr = bestNodePtr;
			break;
		}

		//--------- generate successors -------//
		if(bestNodePtr->generate_successors2(real_sour_x, real_sour_y))
		{
			path_status = PATH_REUSE_FOUND;
			result_node_ptr = reuse_result_node_ptr;
			break;
		}
	}

	err_when( cur_stack_pos!=0 );		// it should be zero all the times, all pushes should have been poped
	current_search_node_used = i+1;		// store the number of nodes used in this searching
	return path_status;
}
//--------- End of static function SeekPath::continue_seek2 ---------//


//---- Begin of function SeekPath::get_result2 ---------//
ResultNode* SeekPath::get_result2(int& resultNodeCount, short& pathDist)
{
	resultNodeCount = pathDist = 0;
	if(total_node_avail<=0)
		return NULL;

	total_node_avail -= current_search_node_used;
	short useClosestNode = 0; // indicate whether closest node is returned instead of the actual node

	if(!result_node_ptr || !result_node_ptr->parent_node)	// if PATH_IMPOSSIBLE or PATH_NODE_USED_UP, result_node_ptr is NULL, we need to call get_closest_node() to get the closest node.
	{
		if(path_status==PATH_FOUND)
			return NULL; // the current location is already the destination
		
		err_when(path_status==PATH_FOUND);
		result_node_ptr = return_closest_node();
		useClosestNode = 1;

		if(!result_node_ptr)
			return NULL;
	}

	//--------------------------------------------------//
	// Trace backwards to the starting node, and rationalize
	// nodes (i.e. group nodes of the same direction into
	// single nodes.)
	//--------------------------------------------------//
	Node* nodePtr		= result_node_ptr;		// the node current being processed
	Node* baseNodePtr = result_node_ptr;		// the first end node for connecting the other end node for the path in that direction.
	Node* parentNode  = nodePtr->parent_node;	 // the parent node of nodePtr

	if(!parentNode)
		return NULL;
	
	resultNodeCount=1;		// the current node
	err_when( !parentNode );		// the desination node must have a parent node

	//--- if the following nodes are going at the same direction, rationalize them ---//
	short vectorX = parentNode->node_x - nodePtr->node_x;	// the vector at which the sprite is going
	short vectorY = parentNode->node_y - nodePtr->node_y;
	short newVectorX, newVectorY;
	nodePtr = parentNode;
	//sys_yield(); // update cursor position

	err_when(vectorX && vectorY && abs(vectorX)!=abs(vectorY));
	while((parentNode=nodePtr->parent_node) != NULL)
	{
		err_when(abs(vectorX)>1 || abs(vectorY)>1);

		//------ turning to another direction at this point ------//
		newVectorX = parentNode->node_x-nodePtr->node_x;
		newVectorY = parentNode->node_y-nodePtr->node_y;
		if(vectorX != newVectorX || vectorY != newVectorY)
		{
			err_when(abs(newVectorX)>1 || abs(newVectorY)>1);
			err_when(newVectorX && newVectorY && abs(newVectorX)!=abs(newVectorY));
			err_when(baseNodePtr->node_x-nodePtr->node_x && baseNodePtr->node_y-nodePtr->node_y &&
						abs(baseNodePtr->node_x-nodePtr->node_x)!=abs(baseNodePtr->node_y-nodePtr->node_y));
			baseNodePtr->parent_node = nodePtr;	  // skip the in-between ones which are in the same direction
			baseNodePtr = nodePtr;    				  // prepare for the next line.
			resultNodeCount++;

			vectorX = newVectorX;
			vectorY = newVectorY;
		}

		nodePtr = parentNode;
	}

	baseNodePtr->parent_node = nodePtr;	  // finish off the last one
	resultNodeCount++;

	//----------------------------------------------------------------------------//
	// After the above process, here we will have a link of rationalize nodes.
	// Retrieve them in the backwards order
	//----------------------------------------------------------------------------//
	//sys_yield(); // update cursor position
	ResultNode	*resultNodeArray = (ResultNode*) mem_add( sizeof(ResultNode) * resultNodeCount );
	ResultNode	*resultNodePtr = resultNodeArray+resultNodeCount-1;
	int 			processCount = 1;

	Node			*preNodePtr = result_node_ptr;
	resultNodePtr->node_x = preNodePtr->node_x;
	resultNodePtr->node_y = preNodePtr->node_y;
	resultNodePtr--;
	Node			*curNodePtr = result_node_ptr->parent_node;
	err_when(pathDist!=0);

	int xDist, yDist;
	while(processCount++ < resultNodeCount)
	{
		err_when(curNodePtr->node_x<0 || curNodePtr->node_x>=MAX_WORLD_X_LOC || 
					curNodePtr->node_y<0 || curNodePtr->node_y>=MAX_WORLD_Y_LOC);

		resultNodePtr->node_x = curNodePtr->node_x;
		resultNodePtr->node_y = curNodePtr->node_y;
		resultNodePtr--;

		xDist = abs(curNodePtr->node_x-preNodePtr->node_x);
		yDist = abs(curNodePtr->node_y-preNodePtr->node_y);
		err_when((!xDist && !yDist) || (xDist && yDist && xDist!=yDist));
		pathDist += (xDist) ? xDist : yDist;

		preNodePtr = curNodePtr;
		curNodePtr = curNodePtr->parent_node;
	}

	//sys_yield(); // update cursor position

	//------------------------------------------------//
	// multipy all the node by scale 2
	//------------------------------------------------//
	for(processCount=0, resultNodePtr=resultNodeArray; processCount<resultNodeCount; processCount++, resultNodePtr++)
	{
		resultNodePtr->node_x <<= 1;
		resultNodePtr->node_y <<= 1;

		err_when(resultNodePtr->node_x<0 || resultNodePtr->node_x>=MAX_WORLD_X_LOC ||
					resultNodePtr->node_y<0 || resultNodePtr->node_y>=MAX_WORLD_Y_LOC);
	}
	pathDist <<= 1;

	#ifdef DEBUG
		//------------------------------------------------------------------------//
		// used to debug for the path of UNIT_SEA
		//------------------------------------------------------------------------//
		if(search_mode==SEARCH_MODE_IN_A_GROUP || search_mode==SEARCH_MODE_REUSE || search_mode==SEARCH_MODE_A_UNIT_IN_GROUP)
			if(mobile_type==UNIT_SEA && resultNodeCount>1 && search_mode!=SEARCH_MODE_TO_LAND_FOR_SHIP)
				debug_check_sea_sailable(resultNodeArray, resultNodeCount);
	#endif

	return resultNodeArray;
}
//------- End of function SeekPath::get_result2 -------//


//-------- Begin of function Node::generate_successors2 ---------//
short Node::generate_successors2(short realSourX, short realSourY)
{
	int	hasLeft  = node_x > cur_border_x1;
	int	hasRight = node_x < cur_border_x2;
	int	hasUp		= node_y > cur_border_y1;
	int	hasDown  = node_y < cur_border_y2;
	int	upperLeftX, upperLeftY;
	short	cost = 2;

	upperLeftX = node_x<<1;
	upperLeftY = node_y<<1;

	if(hasLeft)
	{
		//--------- Left --------//
		if(can_move_to(upperLeftX-2, upperLeftY) && can_move_to(upperLeftX-1, upperLeftY))
		{
			if(generate_succ2(node_x-1, node_y))
				return 1;
		}

		//------- Upper-Left ---------//
		if(hasUp)
		{
			if(can_move_to(upperLeftX-2, upperLeftY-2) && can_move_to(upperLeftX-1, upperLeftY-1))	// can pass through the tile
			{
				if(generate_succ2(node_x-1, node_y-1))
					return 1;
			}
		}

		//--------- Lower-Left ----------//
		if(hasDown)
		{
			if(can_move_to(upperLeftX-2, upperLeftY+2) && can_move_to(upperLeftX-1, upperLeftY+1))
			{
				if(generate_succ2(node_x-1, node_y+1))
					return 1;
			}
		}
	}

	if(hasRight)
	{
		//----------- Right -----------//
		if(can_move_to(upperLeftX+2, upperLeftY) && can_move_to(upperLeftX+1, upperLeftY))
		{
			if(generate_succ2(node_x+1, node_y))
				return 1;
		}

		//-------- Upper-Right ---------//
		if(hasUp)
		{
			if(can_move_to(upperLeftX+2, upperLeftY-2) && can_move_to(upperLeftX+1, upperLeftY-1))
			{
				if(generate_succ2(node_x+1, node_y-1))
					return 1;
			}
		}

		//--------- Lower-Right ---------//
		if(hasDown)
		{
			if(can_move_to(upperLeftX+2, upperLeftY+2) && can_move_to(upperLeftX+1, upperLeftY+1))
			{
				if(generate_succ2(node_x+1, node_y+1))
					return 1;
			}
		}
	}

	//---------- Upper -----------//
	if(hasUp)
	{
		if(can_move_to(upperLeftX, upperLeftY-2) && can_move_to(upperLeftX, upperLeftY-1))
		{
			if(generate_succ2(node_x, node_y-1))
				return 1;
		}
	}

	//---------- Lower -----------//
	if(hasDown)
	{
		if(can_move_to(upperLeftX, upperLeftY+2) && can_move_to(upperLeftX, upperLeftY+1))
		{
			if(generate_succ2(node_x, node_y+1))
				return 1;
		}
	}

	return 0;
}
//------- End of function Node::generate_successors2 -------//


//-------- Begin of function Node::generate_succ2 ---------//
short Node::generate_succ2(short x, short y, short cost)
{
	//----- if it points back to this node's parent ----//
	if(parent_node)
	{
		if(parent_node->node_x==x && parent_node->node_y==y)
			return 0;
	}

	//----- if there is an existing node at the given position ----//
	//int upperLeftX, upperLeftY;
	short c, g = node_g+1;	    	 // g(Successor)=g(BestNode)+cost of getting from BestNode to Successor
	short nodeRecno;

	if((nodeRecno=cur_node_matrix[y*MAX_WORLD_X_LOC/2+x]) > 0 && nodeRecno<max_node_num)
	{
		Node* oldNode = cur_node_array+nodeRecno-1;

		//------ Add oldNode to the list of BestNode's child_noderen (or Successors).
		for(c=0 ; c<MAX_CHILD_NODE && child_node[c] ; c++);
		child_node[c]=oldNode;

		//---- if our new g value is < oldNode's then reset oldNode's parent to point to BestNode
		if(g < oldNode->node_g)
		{
			oldNode->parent_node = this;
			oldNode->node_g 	   = g;
			oldNode->node_f	 	= g+oldNode->node_h;

			//-------- if it's a closed node ---------//
			if(oldNode->child_node[0] )
			{
				 //-------------------------------------------------//
				 // Since we changed the g value of oldNode, we need
				 // to propagate this new value downwards, i.e.
				 // do a Depth-First traversal of the tree!
				 //-------------------------------------------------//
				 oldNode->propagate_down();
				 //sys_yield();
			}
		}
	}
	else //------------ add a new node -----------//
	{
		Node* succNode = cur_node_array + cur_seek_path->node_count++;

		memset(succNode, 0, sizeof(Node));

		succNode->parent_node = this;
		succNode->node_g = g;
		succNode->node_h = (x-cur_dest_x)*(x-cur_dest_x)+(y-cur_dest_y)*(y-cur_dest_y); // should do sqrt(), but since we don't really
		succNode->node_f = g+succNode->node_h;     		// care about the distance but just which branch looks
		succNode->node_x = x;                  			// better this should suffice. Anyayz it's faster.
		succNode->node_y = y;

		if(search_mode==SEARCH_MODE_REUSE && nodeRecno>max_node_num)	// path-reuse node found, but checking of can_walk(final_dest_?) is requested
		{
			final_dest_x = x<<1;
			final_dest_y = y<<1;

			if(can_move_to(final_dest_x, final_dest_y))	// can_walk the connection point, accept this case
			{
				reuse_result_node_ptr = succNode;
				return 1;
			}	// else continue until reuse node is found and connection point can be walked
		}

		cur_node_matrix[y*MAX_WORLD_X_LOC/2+x] = cur_seek_path->node_count;
		cur_seek_path->open_node_list.insert_node(succNode);
		for(c=0 ; c<MAX_CHILD_NODE && child_node[c] ; c++);   // Add oldNode to the list of BestNode's child_noderen (or succNodes).
		child_node[c]=succNode;
	}

	return 0;
}
//------- End of function Node::generate_succ2 -------//


//-------- Begin of function Node::propagate_down2 --------//
void Node::propagate_down2()
{
	Node* childNode, *fatherNode;
	int 	c, g=node_g;            // alias.
	int	cost = 1;
	#ifdef DEBUG
	int	middleXLoc, middleYLoc;
	#endif

	for(c=0;c<8;c++)
	{
		if((childNode=child_node[c])==NULL)   // create alias for faster access.
			break;

		if(g+cost < childNode->node_g)
		{
			#ifdef DEBUG
			middleXLoc = (node_x + childNode->node_x); // calculate real coordinate
			middleYLoc = (node_y + childNode->node_y);
			if(can_move_to(middleXLoc, middleYLoc))
			#else
			if(can_move_to(node_x+childNode->node_x, node_y+childNode->node_y))
			#endif
			{
				childNode->node_g 	  = g+cost;
				childNode->node_f 	  = childNode->node_g+childNode->node_h;
				childNode->parent_node = this;     		// reset parent to new path.

				stack_push(childNode);                 		// Now the childNode's branch need to be
			}
		}     // checked out. Remember the new cost must be propagated down.
	}

	while(cur_stack_pos>0)
	{
		fatherNode=stack_pop();
		g = fatherNode->node_g;

		for(c=0;c<8;c++)
		{
			if ((childNode=fatherNode->child_node[c])==NULL)       // we may stop the propagation 2 ways: either
				break;

			if(g+cost < childNode->node_g) // there are no children, or that the g value of
			{                           			// the child is equal or better than the cost we're propagating
				#ifdef DEBUG
				middleXLoc = (node_x + childNode->node_x); // calculate real coordinate
				middleYLoc = (node_y + childNode->node_y);
				if(can_move_to(middleXLoc, middleYLoc))
				#else
				if(can_move_to(node_x+childNode->node_x, node_y+childNode->node_y))
				#endif
				{
					childNode->node_g 	  = g+cost;
					childNode->node_f 	  = childNode->node_g+childNode->node_h;
					childNode->parent_node = fatherNode;
					stack_push(childNode);
				}
			}
		}
	}
}
//------- End of function Node::propagate_down2 -------//
