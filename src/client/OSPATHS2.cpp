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

//Filename    : OSPATHS2.CPP
//Description : Object SeekPathS2
//Owner		  : Alex

#include <math.h>
#include <stdlib.h>
#include <ALL.h>
#include <OWORLD.h>
#include <OSPATHS2.h>
#include <OUNIT.h>
#include <OFIRM.h>
#include <OTOWN.h>
#include <OSYS.h>

//----------- Define static variables -----------//
static Location*	world_loc_matrix;
static int			cur_stack_pos_s2 = 0;
static NodeS2*		stack_array[MAX_STACK_NUM];
static DWORD		group_id_s2;
static short		search_mode_s2;
static char	  	   mobile_type_s2;
static short		target_recno_s2;
static short		building_id_s2;	// used in search_mode = SEARCH_MODE_TO_FIRM or SEARCH_MODE_TO_TOWN, get from miscNo
static int			building_x1_s2, building_y1_s2, building_x2_s2, building_y2_s2;
												// the building upper-left to lower-right positions
												// if search_mode = SEARCH_MODE_TO_FIRM or SEARCH_MODE_TO_TOWN
static FirmInfo	*searchFirmInfo_s2;

static short		x_shift_array[8] = {   1,  1,   0,  -1,  -1,  -1,   0,   1};
static short		y_shift_array[8] = {   0, -1,  -1,  -1,   0,   1,   1,   1};
//---- used in determining the return nodes in the starting node -----//
static char			source_blocked_exit_direction[8];	// indicate which exit direction of starting node is impossible
static char			source_locate_type;						// indicate the upper left position of the unit located in the 2x2 node
//---- used to decide which path is return in a node -------//
static short		prefer_upper_s2;			// if upper and lower paths are available, true if the upper path is preferred
static short		prefer_lower_s2;			//	if upper and lower paths are available, true if the lower path is preferred
static short		prefer_left_s2;			// if left and right paths are available, true if the left path is preferred
static short		prefer_right_s2;			// if left and right paths are available, true if the right path is preferred
//--- show whether shift-map method is used the direction shifted ---//
static short		use_shift_map_method_s2;// is 0 or 1, 1 for using this method
static short		shift_map_x_offset_s2;	// useful only as shift-map-method is applied
static short		shift_map_y_offset_s2;	// useful only as shift-map-method is applied
//static short		reverse_direction_result_node;
//-- store the orginal and the finalized starting and destination location --//
static short		source_x_s2;				// starting x location in 2x2 node format
static short		source_y_s2;				// starting y location in 2x2 node format
static int			vir_start_x_s2;			// store the start x loc after applying shift-map-method
static int			vir_start_y_s2;			// store the start y loc after applying shift-map-method
static int			final_dest_x_s2;			// store the finalize x loc after applying shift-map-method, path-reuse, and etc
static int			final_dest_y_s2;			// store the finalize y loc after applying shift-map-method, path-reuse, and etc
//------------------- used in path-reuse -----------------------//
static int			max_node_num_s2;
static short		*reuse_node_matrix_ptr_s2;
static NodeS2		*reuse_result_node_ptr_s2;
static int			final_dx;
static int			final_dy;

//------- aliasing class member vars for fast access ------//

static SeekPathS2* 	cur_seek_path_s2;
static short	  		cur_dest_x_s2, cur_dest_y_s2;
static short* 			cur_node_matrix_s2;
static NodeS2*			cur_node_array_s2;
static short	 		cur_border_x1_s2, cur_border_y1_s2, cur_border_x2_s2, cur_border_y2_s2;

#ifdef DEBUG
	static ResultNode		*debugPtr1, *debugPtr2;
	static short			debugVX, debugVY;
	static int				debugCount;

	void debug_checks2(ResultNode *nodeArray, int count)
	{
		debugPtr1 = nodeArray;
		debugPtr2 = nodeArray+1;
		
		for(debugCount=1; debugCount<count; debugCount++, debugPtr1++, debugPtr2++)
		{
			err_when(debugPtr1->node_x<0 || debugPtr1->node_x>=MAX_WORLD_X_LOC-1 ||
						debugPtr1->node_y<0 || debugPtr1->node_y>=MAX_WORLD_Y_LOC-1);
			debugVX = debugPtr1->node_x - debugPtr2->node_x;
			debugVY = debugPtr1->node_y - debugPtr2->node_y;
			err_when(debugVX!=0 && debugVY!=0 && (abs(debugVX)!=abs(debugVY)));
		}

		err_when(debugPtr1->node_x<0 || debugPtr1->node_x>=MAX_WORLD_X_LOC-1 ||
					debugPtr1->node_y<0 || debugPtr1->node_y>=MAX_WORLD_Y_LOC-1);
	}
#endif

#ifdef DEBUG
#define debug_check_paths2(nodeArray, count)		debug_checks2((nodeArray), (count))
#else
#define debug_check_paths2(nodeArray, count)
#endif

//----------- Define static functions -----------//

static void		stack_push(NodeS2 *nodePtr);
static NodeS2*	stack_pop();

//------- Begin of static function sys_yield ----------//
static void sys_yield()
{
	//sys.yield();
}
//------- End of static function sys_yield ----------//


//------- Begin of static function can_move_to ----------//
static int can_move_to(int xLoc, int yLoc) // xLoc, yLoc is location after applying shift-map-method
{
	//-------------------------------------------------------//
	// note : return value should be 0 (false) or 1 (true)
	//-------------------------------------------------------//
	int x, y;

	//-------------------- boundary checking -----------------//
	if(shift_map_x_offset_s2==0)
	{
		if(xLoc<0 || xLoc>=MAX_WORLD_X_LOC)
			return 0;
		else
			x = xLoc;
	}
	else	// shifted
	{
		if(xLoc<1 || xLoc>MAX_WORLD_X_LOC)
			return 0;
		else
			x = xLoc+shift_map_x_offset_s2;
	}

	if(shift_map_y_offset_s2==0)
	{
		if(yLoc<0 || yLoc>=MAX_WORLD_Y_LOC)
			return 0;
		else
			y = yLoc;
	}
	else	// shifted
	{
		if(yLoc<1 || yLoc>MAX_WORLD_Y_LOC)
			return 0;
		else
			y = yLoc+shift_map_y_offset_s2;
	}

	Location* locPtr = world_loc_matrix+y*MAX_WORLD_X_LOC+x;
	Unit* unitPtr;

	//---------- checking whether the location is walkable ----------//
	switch(mobile_type_s2)
	{
		case UNIT_LAND:
			if(!locPtr->walkable() && search_mode_s2<SEARCH_MODE_TO_FIRM) // be careful for the search_mode>=SEARCH_MODE_TO_FIRM
				return 0;
			
			switch(search_mode_s2)
			{
				case SEARCH_MODE_IN_A_GROUP:	// group move
				case SEARCH_MODE_REUSE:		// path-reuse
							if(!locPtr->cargo_recno)
								return 1;
							else
							{
								unitPtr = unit_array[locPtr->cargo_recno];
								return (unitPtr->unit_group_id==group_id_s2 && unitPtr->cur_action!=SPRITE_ATTACK) ||
										 unitPtr->cur_action==SPRITE_MOVE;
							}

				case SEARCH_MODE_A_UNIT_IN_GROUP:	// a unit in a group
							return locPtr->cargo_recno==0 || unit_array[locPtr->cargo_recno]->cur_action==SPRITE_MOVE;

				case SEARCH_MODE_TO_ATTACK:	// to attack target
							if(!locPtr->cargo_recno)
								return 1;
							else
							{
								unitPtr = unit_array[locPtr->cargo_recno];
								return (unitPtr->unit_group_id==group_id_s2 && unitPtr->cur_action!=SPRITE_ATTACK) ||
										 locPtr->cargo_recno==target_recno_s2 || unitPtr->cur_action==SPRITE_MOVE;
							}

				case SEARCH_MODE_BLOCKING:	// 2x2 unit blocking
							if(locPtr->cargo_recno)
								unitPtr = unit_array[locPtr->cargo_recno];
							return locPtr->cargo_recno==0 ||
									 (unitPtr->unit_group_id==group_id_s2 && (unitPtr->cur_action==SPRITE_MOVE || unitPtr->cur_action==SPRITE_READY_TO_MOVE));

				case SEARCH_MODE_TO_VEHICLE:
							err_here();	//** error
							return 0;

				//-----------------------------------------------------------------------//
				// for the following search_mode, location may be treated as walkable
				// although it is not.
				//-----------------------------------------------------------------------//
				case SEARCH_MODE_TO_FIRM:	// move to a firm, (location may be not walkable)
				case SEARCH_MODE_TO_TOWN:	// move to a town zone, (location may be not walkable)
							return (locPtr->walkable() && (locPtr->cargo_recno==0 ||
									  (unit_array[locPtr->cargo_recno]->unit_group_id==group_id_s2 &&
									   unit_array[locPtr->cargo_recno]->cur_action!=SPRITE_ATTACK)  )) ||
									 (xLoc>=building_x1_s2 && xLoc<=building_x2_s2 && yLoc>=building_y1_s2 && yLoc<=building_y2_s2);

				case SEARCH_MODE_TO_WALL_FOR_GROUP:	// move to wall for a group, (location may be not walkable)
							return (locPtr->walkable() && (locPtr->cargo_recno==0 ||
									  (unit_array[locPtr->cargo_recno]->unit_group_id==group_id_s2 &&
									   unit_array[locPtr->cargo_recno]->cur_action!=SPRITE_ATTACK)  )) ||
									 (xLoc==final_dest_x_s2 && yLoc==final_dest_y_s2);

				case SEARCH_MODE_TO_WALL_FOR_UNIT:	// move to wall for a unit only, (location may be not walkable)
							return (locPtr->walkable() && locPtr->cargo_recno==0) ||
									 (xLoc==final_dest_x_s2 && yLoc==final_dest_y_s2);
			}
			break;

		case UNIT_SEA:
			if( !locPtr->sailable() )
				return 0;

			if( search_mode_s2== SEARCH_MODE_A_UNIT_IN_GROUP)
				return locPtr->cargo_recno==0;
			else
				return locPtr->cargo_recno==0 || unit_array[locPtr->cargo_recno]->unit_group_id==group_id_s2 ||
						 (search_mode_s2==SEARCH_MODE_TO_ATTACK && locPtr->cargo_recno==target_recno_s2);
			break;

		case UNIT_AIR:
			if( search_mode_s2== SEARCH_MODE_A_UNIT_IN_GROUP)
				return locPtr->air_cargo_recno==0;
			else
				return locPtr->air_cargo_recno==0 || unit_array[locPtr->air_cargo_recno]->unit_group_id==group_id_s2 ||
						 (search_mode_s2==SEARCH_MODE_TO_ATTACK && locPtr->air_cargo_recno==target_recno_s2);
			break;

		default:
			err_here();
			break;		
	}
	return 0;
}
//------- End of static function can_move_to ----------//


//------- Begin of static function can_move_to_s2 ----------//
static int can_move_to_s2(int xLoc, int yLoc) // xLoc, yLoc is location after applying shift-map-method
{
	//------------------------------------------------//
	// note: return value should be 0 or 1
	//------------------------------------------------//
	if(can_move_to(xLoc,yLoc) && can_move_to(xLoc+1,yLoc) &&
		can_move_to(xLoc,yLoc+1) && can_move_to(xLoc+1,yLoc+1))
		return 1;	// is walkable
	else
		return 0;	// is not walkable as one point is not walkable
}
//------- End of static function can_move_to_s2 ----------//


//-------- Begin of function SeekPathS2::init ---------//
void SeekPathS2::init(int maxNode)
{
	max_node = maxNode;
	node_array = (NodeS2*) mem_add( max_node * sizeof(NodeS2) );
	node_matrix = (short*) mem_add(sizeof(short)*(MAX_WORLD_X_LOC/2+1)*(MAX_WORLD_Y_LOC/2+1));
	path_status = PATH_WAIT;

	open_node_list   = NULL;
	closed_node_list = NULL;
}
//--------- End of function SeekPathS2::init ---------//


//-------- Begin of function SeekPathS2::deinit ---------//
void SeekPathS2::deinit()
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
//--------- End of function SeekPathS2::deinit ---------//


//-------- Begin of function SeekPathS2::set_node_matrix ---------//
void SeekPathS2::set_node_matrix(short reuseNodeMatrix[])
{
	reuse_node_matrix_ptr_s2 = reuseNodeMatrix;
}
//--------- End of function SeekPathS2::set_node_matrix ---------//


//-------- Begin of function SeekPathS2::reset ---------//
void SeekPathS2::reset()
{
	path_status=PATH_WAIT; 
	open_node_list=NULL;
	closed_node_list=NULL; 
}
//--------- End of function SeekPathS2::reset ---------//


//-------- Begin of function SeekPathS2::add_result_node ---------//
inline void SeekPathS2::add_result_node(ResultNode** curPtr, ResultNode** prePtr, int& count)
{
	err_when(abs((*curPtr)->node_x-(*prePtr)->node_x)>1 || abs((*curPtr)->node_y-(*prePtr)->node_y)>1);
	*prePtr = *curPtr;
	(*curPtr)++;
	count++;
}
//--------- End of function SeekPathS2::add_result_node ---------//


//-------- Begin of function SeekPathS2::seek ---------//
//
// <int> sx, sy		- the starting world location.
// <int> dx, dy		- the destination world location.
// <DWORD> groupId	- unit group id.
// <char> mobileType - mobile type, can be UNIT_AIR, UNIT_LAND or UNIT_SEA
//
//	[int] searchMode	-	1, SEARCH_MODE_IN_A_GROUP			for one group with an unique group id	(default)
//								2, SEARCH_MODE_A_UNIT_IN_GROUP	for one sprite in a group
//								3, SEARCH_MODE_TO_ATTACK			for attacking
//								4, SEARCH_MODE_REUSE					for path reuse
//								5, SEARCH_MODE_BLOCKING				for 2x2 unit blocked searching
//								6, SEARCH_MODE_TO_FIRM				for moving to a firm
//								7, SEARCH_MODE_TO_TOWN				for moving to a town zone
//								8, SEARCH_MODE_TO_VEHICLE			for moving to a vehicle
//								(default : 1)
//
//	[int]	miscNo		-  target record no if searchMode=SEARCH_MODE_TO_ATTACK
//							-	firm ID if searchMode=SEARCH_MODE_TO_FIRM
//
// [int] maxTries - maximum no. of tries in the first seek action.
//						  this refer to the maximum no. of nodes created.
//						  (default : max_node)
//
// [int] borderX1, borderY1, - borders of the seek area in the world map
//			borderX2, borderY2	 (default: the whole map)
//
// Note: if maxTries==max_node, incremental seek (PATH_SEEKING) won't happen.
//
// return : <int> seekStatus - PATH_FOUND, PATH_SEEKING, PATH_NODE_USED_UP, or PATH_IMPOSSIBLE
//						if PATH_FOUND, or PATH_NODE_USED_UP, can call get_result() to retrieve the result.
//
int SeekPathS2::seek(int sx,int sy,int dx,int dy,DWORD groupId,char mobileType,short searchMode,short miscNo,
							int maxTries,int borderX1,int borderY1,int borderX2,int borderY2)
{
	//-------- initialize vars --------------//
	path_status			= PATH_SEEKING;
	world_loc_matrix	= world.loc_matrix;	// alias for fast access
	open_node_list		= NULL;
	closed_node_list	= NULL;

	search_mode_s2		= searchMode;
	group_id_s2			= groupId;
	mobile_type_s2		= mobileType;
	
	//------------------------------------------------------------------------------//
	// extract informaton from the parameter "miscNo"
	//------------------------------------------------------------------------------//
	target_recno_s2 = building_id_s2 = 0;
	building_x1_s2 = building_y1_s2 = building_x2_s2 = building_y2_s2 = -1;
	err_when(search_mode_s2==SEARCH_MODE_TO_VEHICLE);
	
	if(search_mode_s2==SEARCH_MODE_TO_ATTACK)
		target_recno_s2 = miscNo;
	else
	{
		building_id_s2 = miscNo;
		building_x1_s2 = dx; // upper left corner location
		building_y1_s2 = dy;

		//------- calculate the lower right corner location -------//
		if(search_mode_s2==SEARCH_MODE_TO_FIRM)
		{			
			searchFirmInfo_s2 = firm_res[building_id_s2];
			building_x2_s2 = dx+searchFirmInfo_s2->loc_width-1;
			building_y2_s2 = dy+searchFirmInfo_s2->loc_height-1;
		}
		else if(search_mode_s2==SEARCH_MODE_TO_TOWN)
		{
			if(miscNo != -1)
			{
				Location* buildPtr = world.get_loc(dx, dy);
				Town* targetTown = town_array[buildPtr->town_recno()];
				building_x2_s2 = targetTown->loc_x2;
				building_y2_s2 = targetTown->loc_y2;
			}
			else	// searching to settle. Detail explained in function set_move_to_surround()
			{
				building_x2_s2 = building_x1_s2+STD_TOWN_LOC_WIDTH-1;
				building_y2_s2 = building_y1_s2+STD_TOWN_LOC_HEIGHT-1;
			}
		}
	}

	//------------------------------------------------------------------------------//
	// set start location and destination location
	//------------------------------------------------------------------------------//
	real_sour_x = sx;
	real_sour_y = sy;

	//---------- adjust destination for some kind of searching ------------//
	if(search_mode_s2==SEARCH_MODE_TO_FIRM || search_mode_s2==SEARCH_MODE_TO_TOWN)
	{
		final_dx = (building_x1_s2+building_x2_s2)/2;	// the destination is set to be the middle of the building
		final_dy = (building_y1_s2+building_y2_s2)/2;
	}
	else
	{
		//---------- no adjustment --------//
		final_dx = real_dest_x = dx;
		final_dy = real_dest_y = dy;
	}

	//------------------------------------------------------------------------------//
	// check whether applying shift-map-method
	//------------------------------------------------------------------------------//
	if(final_dx%2)		// perform x-shift
	{
		use_shift_map_method_s2	= 1;
		shift_map_x_offset_s2	= -1;
		vir_start_x_s2				= sx+1;
		final_dest_x_s2			= final_dx+1;
	}
	else
	{
		use_shift_map_method_s2	= 0;
		shift_map_x_offset_s2	= 0;
		vir_start_x_s2				= sx;
		final_dest_x_s2			= final_dx;
	}

	if(final_dy%2)		// perform y-shift
	{
		use_shift_map_method_s2	= 1;
		shift_map_y_offset_s2	= -1;
		vir_start_y_s2				= sy+1;
		final_dest_y_s2			= final_dy+1;
	}
	else
	{
		//use_shift_map_method_s2	= 0;	// have been set to zero in the previous check
		shift_map_y_offset_s2	= 0;
		vir_start_y_s2				= sy;
		final_dest_y_s2			= final_dy;
	}

	//--------------------------------------------------------------//
	// change to 2x2 node format
	//--------------------------------------------------------------//
	border_x1 = short(borderX1/2);	// change to 2x2 node format
	border_y1 = short(borderY1/2);
	border_x2 = short(borderX2/2);
	border_y2 = short(borderY2/2);
	if(shift_map_x_offset_s2!=0)	// adjust if shift-map is used
		border_x2++;
	if(shift_map_y_offset_s2!=0)
		border_y2++;

	//-------------- initialize the array -----------------//
	memset(source_blocked_exit_direction, 0, sizeof(char)*8);

	//----- determine the blocked exit direction in starting point----------//
	// there are four cases the real source located in the 2x2 node
	//
	//		 -- --		the number in the 2x2 node representing where the
	//		| 1| 2|		upper_left location of the unit is
	//		 -- --
	//		| 3| 4|
	//		 -- --

	if(vir_start_y_s2%2 != 1)
	{	
		if(vir_start_x_s2%2 != 1)	// case 1
			source_locate_type = 1;
		else								// case 2
		{
			source_locate_type = 2;

			if(!can_move_to(vir_start_x_s2-1, vir_start_y_s2+1) && !can_move_to(vir_start_x_s2, vir_start_y_s2-1))
				source_blocked_exit_direction[7] = 1;	// direction 8

			if(!can_move_to(vir_start_x_s2-1, vir_start_y_s2) && !can_move_to(vir_start_x_s2, vir_start_y_s2+2))
				source_blocked_exit_direction[1] = 1;	// direction 2
			// other cases will not be checked as these cases do not crack the search
		}
	}
	else
	{
		if(vir_start_x_s2%2 != 1)	// case 3
		{
			source_locate_type = 3;

			if(!can_move_to(vir_start_x_s2, vir_start_y_s2-1) && !can_move_to(vir_start_x_s2+2, vir_start_y_s2))
				source_blocked_exit_direction[5] = 1;	// direction 6

			if(!can_move_to(vir_start_x_s2-1, vir_start_y_s2) && !can_move_to(vir_start_x_s2+1, vir_start_y_s2-1))
				source_blocked_exit_direction[7] = 1;	// direction 8
			// other cases will not be checked as these cases do not crack the search
		}
		else								// case 4
		{
			source_locate_type = 4;

			if(!can_move_to(vir_start_x_s2, vir_start_y_s2-1) || !can_move_to(vir_start_x_s2-1, vir_start_y_s2))
				source_blocked_exit_direction[7] = 1;	// direction 8

			if(!can_move_to(vir_start_x_s2-1, vir_start_y_s2+1) && !can_move_to(vir_start_x_s2, vir_start_y_s2-1))
				source_blocked_exit_direction[0] = 1;	// direction 1

			if(!can_move_to(vir_start_x_s2-1, vir_start_y_s2) && !can_move_to(vir_start_x_s2+1, vir_start_y_s2-1))
				source_blocked_exit_direction[6] = 1;	// direction 7
			// other cases will not be checked as these cases do not crack the search
		}
	}

	source_x_s2 = short(vir_start_x_s2/2);	// the upper left corner is used
	source_y_s2 = short(vir_start_y_s2/2);
	err_when(final_dest_x_s2%2 || final_dest_y_s2%2);
	dest_x = short(final_dest_x_s2/2);
	dest_y = short(final_dest_y_s2/2);

	if(search_mode_s2!=SEARCH_MODE_REUSE)
	{
		max_node_num_s2 = 0xFFFF;
		memset(node_matrix, 0, sizeof(short)*((MAX_WORLD_X_LOC/2+1)*(MAX_WORLD_Y_LOC/2+1)));
	}
	else
	{
		max_node_num_s2 = max_node;
		memcpy(node_matrix, reuse_node_matrix_ptr_s2, sizeof(short)*((MAX_WORLD_X_LOC/2+1)*(MAX_WORLD_Y_LOC/2+1)));
	}
	
	//--------- create the first node --------//
	node_count  = 0;
	result_node_ptr = NULL;

	NodeS2 *nodePtr = node_array + node_count++;
	memset(nodePtr, 0, sizeof(NodeS2));

	//-------- store the upper left coordinate of the node ----------//
	upper_left_x = source_x_s2<<1;
	upper_left_y = source_y_s2<<1;

	//---------- calculate the node type -----------//
	switch(source_locate_type)
	{
		case 1:	nodePtr->node_type =	15;
					break;

		case 2:	nodePtr->node_type = 10+can_move_to(vir_start_x_s2-1,vir_start_y_s2)+
												can_move_to(vir_start_x_s2-1,vir_start_y_s2+1)*4;
					break;

		case 3:	nodePtr->node_type = 12+can_move_to(vir_start_x_s2,vir_start_y_s2-1)+
												can_move_to(vir_start_x_s2+1,vir_start_y_s2-1)*2;
					break;

		case 4:	nodePtr->node_type = 8+can_move_to(vir_start_x_s2-1,vir_start_y_s2-1)+
												can_move_to(vir_start_x_s2,vir_start_y_s2-1)*2+
												can_move_to(vir_start_x_s2-1,vir_start_y_s2)*4;
					break;

		default:	err_here();
					break;
	}

	err_when(nodePtr->node_type>15 || nodePtr->node_type <0);
	is_dest_blocked = !can_move_to_s2(final_dest_x_s2,	final_dest_y_s2);
	// whether the destination is blocked, if so, only search till the destination's neighbor locations

	nodePtr->node_g = 0;
	nodePtr->node_h = (source_x_s2-dest_x)*(source_x_s2-dest_x)+(source_y_s2-dest_y)*(source_y_s2-dest_y);  // should really use sqrt().
	nodePtr->node_f = nodePtr->node_g + nodePtr->node_h;
	nodePtr->node_x = source_x_s2;
	nodePtr->node_y = source_y_s2;
	nodePtr->enter_direction = 0;

	open_node_list=nodePtr;        // make Open List point to first node

	//--- if the destination is the current postion or next to it & the dest is occupied ---//
	if(source_x_s2==dest_x && source_y_s2==dest_y)
	{
		path_status 	 = PATH_FOUND;
		result_node_ptr = nodePtr;
		return path_status;
	}

	//------------ seek now ------------------//
	if( !maxTries )
		maxTries = max_node;

	return continue_seek( maxTries, 1);	// 1-first seek session of the current seek order
}
//-------- End of function SeekPathS2::seek ---------//


//---- Begin of function SeekPathS2::continue_seek ---------//
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
int SeekPathS2::continue_seek(int maxTries, char firstSeek)
{
	if( path_status != PATH_SEEKING )
		return path_status;

	//------- aliasing class member vars for fast access ------//

	cur_seek_path_s2   = this;
	cur_dest_x_s2	    = dest_x;
	cur_dest_y_s2	  	 = dest_y;
	cur_node_matrix_s2 = node_matrix;
	cur_node_array_s2  = node_array;

   cur_border_x1_s2 	 = border_x1;
	cur_border_y1_s2 	 = border_y1;
	cur_border_x2_s2 	 = border_x2;
	cur_border_y2_s2 	 = border_y2;

	//------ seek the path using the A star algorithm -----//

	NodeS2 *bestNodePtr;
	int  maxNode = max_node-MAX_CHILD_NODE;	// generate_successors() can generate a MAX of MAX_CHILD_NODE new nodes per call

	if(maxTries < maxNode)
		maxNode = maxTries-MAX_CHILD_NODE;  // limit the nodes used in shortest path searching

	for(int i=0; i<maxTries ; i++ )
	{
		bestNodePtr = return_best_node();
		
		if(i%20==0)
			sys_yield(); // update cursor position

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

		//------------------------------------------//
		// If the path is found OR
		//
		// If the destination is blocked,
		// consider it done when we are next to it.
		//------------------------------------------//
		if((bestNodePtr->node_x==dest_x && bestNodePtr->node_y==dest_y) ||
			(is_dest_blocked && abs(bestNodePtr->node_x-dest_x)<=0 && abs(bestNodePtr->node_y-dest_y)<=0))
		{
			path_status 	 = PATH_FOUND;
			result_node_ptr = bestNodePtr;
			break;
		}

		//--------- generate successors -------//
		if(bestNodePtr->generate_successors(bestNodePtr->enter_direction))
		{
			path_status = PATH_REUSE_FOUND;
			result_node_ptr = reuse_result_node_ptr_s2;
			return path_status;
		}
	}

	err_when( cur_stack_pos_s2!=0 );		// it should be zero all the times, all pushes should have been poped
	return path_status;
}
//------ End of function SeekPathS2::continue_seek ---------//


//---- Begin of function SeekPathS2::get_result ---------//
//
// Compile the seek result nodes using results processed by
// seek() and continue_seek() and store the results in
// an array of ResultNode.
//
// <int&>  resultNodeCount - a reference var for returning the no. of result nodes.
//
// return : <ResultNode*> an array of ResultNode.
//								  the caller function is responsible for
//								  freeing the memory of the array.
//
ResultNode* SeekPathS2::get_result(int& resultNodeCount, short& pathDist)
{
	resultNodeCount = pathDist = 0;
	short useClosestNode = 0;// indicate whether closest node is return instead of the actual node

	if( !result_node_ptr )	// if PATH_IMPOSSIBLE or PATH_NODE_USED_UP, result_node_ptr is NULL, we need to call get_closest_node() to get the closest node.
	{
		result_node_ptr = return_closest_node();
		useClosestNode = 1;

		if( !result_node_ptr )
			return NULL;
	}

	//--------------------------------------------------//
	// Trace backwards to the starting node, and rationalize
	// nodes (i.e. group nodes of the same direction into
	// single nodes.)
	//--------------------------------------------------//
	NodeS2* nodePtr		= result_node_ptr;		// the node current being processed
	NodeS2* baseNodePtr	= result_node_ptr;		// the first end node for connecting the other end node for the path in that direction.
	NodeS2* parentNode	= nodePtr->parent_node;	// the parent node of nodePtr
	NodeS2* childNodePtr = nodePtr;					// it should point to the children node of nodePtr

	//------------------------------------------------------------------------
	// there are only one node, source & destination within the same 2x2 node
	//------------------------------------------------------------------------
	if( !parentNode ) 		// parentNode==0 when the source location is the desination already
	{ 
		if( (vir_start_x_s2!=final_dest_x_s2 || vir_start_y_s2!=final_dest_y_s2) &&
			  abs(vir_start_x_s2-final_dest_x_s2)<=1 && abs(vir_start_y_s2-final_dest_y_s2)<=1 &&
			  can_move_to_s2(vir_start_x_s2,vir_start_y_s2) && // for source
			  can_move_to_s2(final_dest_x_s2,final_dest_y_s2)) // for destination
		{
			pathDist = 1;

			ResultNode* resultNodeArray1 = (ResultNode*) mem_add(sizeof(ResultNode)*2);
			ResultNode* resultNodePtr1 = resultNodeArray1;
			resultNodeCount=2;
			resultNodePtr1->node_x = vir_start_x_s2 + shift_map_x_offset_s2;
			resultNodePtr1->node_y = vir_start_y_s2 + shift_map_y_offset_s2;
			resultNodePtr1++;
			resultNodePtr1->node_x = final_dest_x_s2 + shift_map_x_offset_s2;
			resultNodePtr1->node_y = final_dest_y_s2 + shift_map_y_offset_s2;

			#ifdef DEBUG
				debugPtr1 = resultNodeArray1;
				debugPtr2 = resultNodeArray1+1;
				err_when(debugPtr1->node_x==debugPtr2->node_x && debugPtr1->node_y==debugPtr2->node_y);
			#endif

			return resultNodeArray1;
	   }
		else 
			return NULL;
	}

	resultNodeCount = 1;

	//-----------------------------------
	//	count the number of 2x2 node
	//-----------------------------------
	int numOfNode=0;
	NodeS2* curPtr = result_node_ptr;
	while(curPtr != NULL)
	{
		curPtr = curPtr->parent_node;
		numOfNode++;
	}

	sys_yield(); // update cursor position

	//-----------------------------------------
	// otherwise, there are more than one node
	//-----------------------------------------
	ResultNode* maxSizeResultNodeArray;	// store all the result node in the reverse order, the turning point will be extracted later
	int nodeAllocated, nodeCount=0;
	int preNodeCount = nodeCount;

	nodeAllocated = numOfNode*2+2;
	maxSizeResultNodeArray = (ResultNode*) mem_add(nodeAllocated*sizeof(ResultNode));
	max_size_result_node_ptr = maxSizeResultNodeArray;
	parent_result_node_ptr = maxSizeResultNodeArray;

	//----------------------------------
	// start from the destination
	//----------------------------------
	memset(max_size_result_node_ptr, 0, sizeof(ResultNode)*nodeAllocated);
	int upper_left_x = nodePtr->node_x<<1;
	int upper_left_y = nodePtr->node_y<<1;
	
	if(!useClosestNode && (search_mode_s2==SEARCH_MODE_TO_ATTACK || can_move_to_s2(final_dest_x_s2, final_dest_y_s2)))
	{
		max_size_result_node_ptr->node_x = final_dest_x_s2;
		max_size_result_node_ptr->node_y = final_dest_y_s2;

		max_size_result_node_ptr++;		// note: parent_result_node_ptr don't change here
		nodeCount++;
		//---------------------------------------------------
		// process the end node, may pass through two points
		//---------------------------------------------------
		if(search_mode_s2==SEARCH_MODE_REUSE)
			process_end_node(upper_left_x, upper_left_y, nodePtr->node_type, nodePtr->enter_direction, nodeCount);
	}
	else	// closest node is used
	{
		//------------------------------------------------------------------------//
		// may generate duplicated node for the destination if closet node is used.
		//------------------------------------------------------------------------//
		switch(nodePtr->enter_direction)
		{
			case 1:	max_size_result_node_ptr->node_x = upper_left_x-1;
						max_size_result_node_ptr->node_y = upper_left_y;
						break;

			case 2:	max_size_result_node_ptr->node_x = upper_left_x-1;
						max_size_result_node_ptr->node_y = upper_left_y+1;
						break;

			case 3:	max_size_result_node_ptr->node_x = upper_left_x;
						max_size_result_node_ptr->node_y = upper_left_y+1;
						break;

			case 4:	max_size_result_node_ptr->node_x = upper_left_x+1;
						max_size_result_node_ptr->node_y = upper_left_y+1;
						break;

			case 5:	max_size_result_node_ptr->node_x = upper_left_x+1;
						max_size_result_node_ptr->node_y = upper_left_y;
						break;

			case 6:	max_size_result_node_ptr->node_x = upper_left_x+1;
						max_size_result_node_ptr->node_y = upper_left_y-1;
						break;

			case 7:	max_size_result_node_ptr->node_x = upper_left_x;
						max_size_result_node_ptr->node_y = upper_left_y-1;
						break;

			case 8:	max_size_result_node_ptr->node_x = upper_left_x-1;
						max_size_result_node_ptr->node_y = upper_left_y-1;
						break;

			default:	err_here();
						break;
		}
		max_size_result_node_ptr++;		// note: parent_result_node_ptr don't change here
		nodeCount++;
	}
	nodePtr = nodePtr->parent_node;	// next 2x2 node

	//--------------------------------------------------
	// get the actual path, process from the second node
	//--------------------------------------------------
	int yieldCount = 0;
	while( (parentNode=nodePtr->parent_node) != NULL )
	{
		upper_left_x = nodePtr->node_x<<1;
		upper_left_y = nodePtr->node_y<<1;
		err_when(nodePtr->enter_direction==(childNodePtr->enter_direction+3)%8+1);
		get_real_result_node(nodeCount, nodePtr->enter_direction, (childNodePtr->enter_direction+3)%8+1,
									nodePtr->node_type, upper_left_x, upper_left_y);
		childNodePtr = nodePtr;
		nodePtr = parentNode;
		//err_when(nodeCount+nodePtr->node_g+2 > nodeAllocated);

		yieldCount++;
		if(yieldCount%20==0)
			sys_yield(); // update cursor position
	}

	sys_yield(); // update cursor position
	debug_check_paths2(maxSizeResultNodeArray, nodeCount);
/*#ifdef DEBUG
	debugPtr1 = maxSizeResultNodeArray;
	debugPtr2 = maxSizeResultNodeArray+1;
	for(debugCount=1; debugCount<nodeCount-1; debugCount++)
	{
		debugVX = debugPtr2->node_x - debugPtr1->node_x;
		debugVY = debugPtr2->node_y - debugPtr1->node_y;
		err_when(debugVX!=0 && debugVY!=0 && abs(debugVX)!=abs(debugVY));
		debugPtr1++;
		debugPtr2++;
	}
#endif*/

	//----------------------------------------------------
	// process the starting node
	// nodePtr points at the starting node now
	//----------------------------------------------------
	err_when(nodePtr->enter_direction);	//	it should be zero
	upper_left_x = nodePtr->node_x<<1;
	upper_left_y = nodePtr->node_y<<1;

	//------------------------------------------------------------------------------------------//
	// process the starting node
	//------------------------------------------------------------------------------------------//
	process_start_node(upper_left_x, upper_left_y, nodePtr->node_type, (childNodePtr->enter_direction+3)%8+1, nodeCount);

	max_size_result_node_ptr->node_x = vir_start_x_s2;
	max_size_result_node_ptr->node_y = vir_start_y_s2;
	err_when((abs(max_size_result_node_ptr->node_x-parent_result_node_ptr->node_x)>1) ||
				(abs(max_size_result_node_ptr->node_y-parent_result_node_ptr->node_y)>1));
	nodeCount++;
	sys_yield(); // update cursor position
	debug_check_paths2(maxSizeResultNodeArray, nodeCount);

/*#ifdef DEBUG
	debugPtr1 = maxSizeResultNodeArray;
	debugPtr2 = maxSizeResultNodeArray+1;
	
	for(debugCount=1; debugCount<nodeCount; debugCount++)
	{
		debugVX = debugPtr2->node_x - debugPtr1->node_x;
		debugVY = debugPtr2->node_y - debugPtr1->node_y;
		
		err_when(debugVX!=0 && debugVY!=0 && (abs(debugVX)!=abs(debugVY)));
		debugPtr1++;
		debugPtr2++;
	}
#endif*/
	
	//--------------------------------------------------//
	// smooth the path
	//--------------------------------------------------//
	maxSizeResultNodeArray = smooth_the_path(maxSizeResultNodeArray, nodeCount);
	sys_yield(); // update cursor position

	parent_result_node_ptr = maxSizeResultNodeArray;
	max_size_result_node_ptr = maxSizeResultNodeArray+1;
	ResultNode* result_node_pointer = max_size_result_node_ptr;

	//----------------------------------
	// get the turning point 
	//----------------------------------
	short vectorX = max_size_result_node_ptr->node_x - parent_result_node_ptr->node_x;
	short vectorY = max_size_result_node_ptr->node_y - parent_result_node_ptr->node_y;
	short newVectorX, newVectorY;

	for(int i=1; i<nodeCount-1; i++)
	{
		parent_result_node_ptr = max_size_result_node_ptr; // don't use parent_result_node_ptr++, if the above code of removing duplication is used.
		max_size_result_node_ptr++;
		newVectorX=(max_size_result_node_ptr->node_x-parent_result_node_ptr->node_x);
		newVectorY=(max_size_result_node_ptr->node_y-parent_result_node_ptr->node_y);
		err_when(newVectorY!=0 && newVectorX!=0 && abs(newVectorX)!=abs(newVectorY));

		//------ turning to another direction at this point ------//
		if(vectorX!=newVectorX || vectorY!=newVectorY)
		{	
			if(newVectorX!=0 || newVectorY!=0)
			{
				result_node_pointer->node_x = parent_result_node_ptr->node_x;
				result_node_pointer->node_y = parent_result_node_ptr->node_y;
				result_node_pointer++;
				resultNodeCount++;

				vectorX = newVectorX;
				vectorY = newVectorY;
			}
		}
		
		if(i%20==0)
			sys_yield(); // update cursor position
	}

	result_node_pointer->node_x = vir_start_x_s2;
	result_node_pointer->node_y = vir_start_y_s2;
	result_node_pointer++;
	resultNodeCount++;

	//------------------------------------------------//
	// After the above process, here we will have a
	// link of rationalize nodes. Retrieve them in the
	// backwards order
	//------------------------------------------------//
	ResultNode *resultNodeArray = (ResultNode*) mem_add( sizeof(ResultNode) * resultNodeCount );
	ResultNode *resultNodePtr = resultNodeArray+resultNodeCount-1;
	int 			processCount = 1;
	
	ResultNode *preNodePtr = maxSizeResultNodeArray;
	resultNodePtr->node_x = preNodePtr->node_x;
	resultNodePtr->node_y = preNodePtr->node_y;
	resultNodePtr--;

	result_node_pointer = maxSizeResultNodeArray+1;
	err_when(pathDist!=0);

	int xDist, yDist;
	while(processCount++ < resultNodeCount)
	{
		err_when(result_node_pointer->node_x<0 || result_node_pointer->node_x>=MAX_WORLD_X_LOC || 
					result_node_pointer->node_y<0 || result_node_pointer->node_y>=MAX_WORLD_Y_LOC);

		resultNodePtr->node_x = result_node_pointer->node_x;
		resultNodePtr->node_y = result_node_pointer->node_y;
		resultNodePtr--;
		
		xDist = abs(result_node_pointer->node_x-preNodePtr->node_x);
		yDist = abs(result_node_pointer->node_y-preNodePtr->node_y);
		err_when((!xDist && !yDist) || (xDist && yDist && xDist!=yDist));
		pathDist += (xDist) ? xDist : yDist;

		preNodePtr = result_node_pointer;
		result_node_pointer++;
		if(processCount%20==0)
			sys_yield(); // update cursor position
	}

	err_when(nodeAllocated<nodeCount);
	mem_del(maxSizeResultNodeArray);

	//-------- adjust/shift the path if shift-map-method is use --------//
	ResultNode* adjustNode = resultNodeArray;
	if(use_shift_map_method_s2)
	{
		for(int ii=0; ii<resultNodeCount; ii++)
		{
			adjustNode->node_x += shift_map_x_offset_s2;
			adjustNode->node_y += shift_map_y_offset_s2;
			adjustNode++;
		}
	}

	debug_check_paths2(resultNodeArray, resultNodeCount);
/*#ifdef DEBUG
	debugPtr1 = resultNodeArray;
	debugPtr2 = resultNodeArray+1;
	
	for(debugCount=1; debugCount<resultNodeCount; debugCount++)
	{
		debugVX = debugPtr2->node_x - debugPtr1->node_x;
		debugVY = debugPtr2->node_y - debugPtr1->node_y;
		
		err_when(debugVX!=0 && debugVY!=0 && (abs(debugVX)!=abs(debugVY)));
		debugPtr1++;
		debugPtr2++;
	}
#endif*/

	//--------------------------------------------------//
	// return NULL if there are only two nodes and these
	// nodes are equal
	//--------------------------------------------------//
	if(resultNodeCount == 2)
	{
		ResultNode *curTestNode1 = resultNodeArray;
		ResultNode *curTestNode2 = resultNodeArray+1;
		if(curTestNode1->node_x==curTestNode2->node_x &&
			curTestNode1->node_y==curTestNode2->node_y)
		{
			mem_del(resultNodeArray);
			resultNodeCount = 0;
			pathDist = 0;
			return NULL;
		}
	}

#ifdef DEBUG
	if(resultNodeCount==2)
	{
		debugPtr1 = resultNodeArray;
		debugPtr2 = resultNodeArray+1;
		err_when(debugPtr1->node_x==debugPtr2->node_x && debugPtr1->node_y==debugPtr2->node_y);
	}
#endif

	sys_yield(); // update cursor position
	return resultNodeArray;
}
//------ End of function SeekPathS2::get_result ---------//


//---- Begin of function SeekPathS2::process_end_node ---------//
void SeekPathS2::process_end_node(int xLoc, int yLoc, char nodeType, char enterDirection, int &nodeCount)
{
	short usePathSuccess = 0;	// if failure, use the last path in the checking
	short destType = 1 + (final_dest_x_s2%2) + 2*(final_dest_y_s2%2);
	
	switch(destType)
	{
		case 1:
					break;

		case 2:
					switch(enterDirection)
					{
						case 1:
									max_size_result_node_ptr->node_x = xLoc;
									max_size_result_node_ptr->node_y = yLoc;
									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, nodeCount);
									break;

						case 2:
									err_when(!(nodeType&0x1) && !can_move_to(xLoc+1,yLoc+2));
									max_size_result_node_ptr->node_x = xLoc;
									max_size_result_node_ptr->node_y = yLoc+!(nodeType&0x1);
									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, nodeCount);
									break;

						case 3: case 4: case 6: case 7:
									break;

						case 5:
									break;

						case 8:
									err_when(!(nodeType&0x4) && !can_move_to(xLoc+1,yLoc-1));
									max_size_result_node_ptr->node_x = xLoc;
									max_size_result_node_ptr->node_y = yLoc-!(nodeType&0x4);
									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, nodeCount);
									break;

						default:	err_here();
									break;
					}
					break;

		case 3:
					switch(enterDirection)
					{
						case 1: case 2: case 4: case 5:
									break;

						case 3:
									break;

						case 6:
									err_when(!(nodeType&0x1) && !can_move_to(xLoc+2,yLoc+1));
									max_size_result_node_ptr->node_x = xLoc+!(nodeType&0x1);
									max_size_result_node_ptr->node_y = yLoc;
									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, nodeCount);
									break;

						case 7:
									max_size_result_node_ptr->node_x = xLoc;
									max_size_result_node_ptr->node_y = yLoc;
									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, nodeCount);
									break;

						case 8:
									err_when(!(nodeType&0x2) && !can_move_to(xLoc-1,yLoc+1));
									max_size_result_node_ptr->node_x = xLoc-!(nodeType&0x2);
									max_size_result_node_ptr->node_y = yLoc;
									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, nodeCount);
									break;

						default:	err_here();
									break;
					}
					break;

		case 4:
					switch(enterDirection)
					{
						case 1:
									err_when(!(nodeType&0x2) && !can_move_to(xLoc,yLoc+2));
									max_size_result_node_ptr->node_x = xLoc;
									max_size_result_node_ptr->node_y = yLoc+can_move_to(xLoc,yLoc+2);
									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, nodeCount);
									break;

						case 2:
									max_size_result_node_ptr->node_x = xLoc;
									max_size_result_node_ptr->node_y = yLoc+1;
									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, nodeCount);
									break;

						case 3: case 4: case 5:
									break;

						case 6:	
									max_size_result_node_ptr->node_x = xLoc+1;
									max_size_result_node_ptr->node_y = yLoc;
									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, nodeCount);
									break;

						case 7:
									err_when(!(nodeType&0x4) && !can_move_to(xLoc+2,yLoc));
									max_size_result_node_ptr->node_x = xLoc+can_move_to(xLoc+2,yLoc);
									max_size_result_node_ptr->node_y = yLoc;
									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, nodeCount);
									break;

						case 8:
									max_size_result_node_ptr->node_x = xLoc;
									max_size_result_node_ptr->node_y = yLoc;
									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, nodeCount);
									break;

						default:	err_here();
									break;
					}
					break;

		default:	err_here();
					break;
	}
}
//------ End of function SeekPathS2::process_end_node ---------//


//---- Begin of function SeekPathS2::process_start_node ---------//
void SeekPathS2::process_start_node(int xLoc, int yLoc, char nodeType, char exitDirection, int &nodeCount)
{
	short usePathSuccess = 0;	// if failure, use the last path in the checking
	switch(source_locate_type)
	{
		case 1:
					switch(exitDirection)
					{
						case 1:	// 3 possible paths
									if(parent_result_node_ptr->node_y<yLoc && can_move_to(xLoc-1,yLoc-1) &&
										can_move_to(xLoc,yLoc-1))
									{
										max_size_result_node_ptr->node_x = xLoc-1;
										max_size_result_node_ptr->node_y = yLoc-1;
										usePathSuccess++;
									}

									if(!usePathSuccess && parent_result_node_ptr->node_y>yLoc && 
										can_move_to(xLoc-1,yLoc+2) && can_move_to(xLoc,yLoc+2))
									{	
										max_size_result_node_ptr->node_x = xLoc-1;
										max_size_result_node_ptr->node_y = yLoc+1;
										usePathSuccess++;
									}

									if(!usePathSuccess)
									{
										max_size_result_node_ptr->node_x = xLoc-1;
										max_size_result_node_ptr->node_y = yLoc;
									}

									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, nodeCount);
									break;

						case 2:	// 3 possible paths
									if(parent_result_node_ptr->node_x==xLoc && //can_move_to(xLoc+1,yLoc+1) &&
										can_move_to(xLoc+1,yLoc+2))
									{
										max_size_result_node_ptr->node_x = xLoc;
										max_size_result_node_ptr->node_y = yLoc+1;
										usePathSuccess++;
									}

									if(!usePathSuccess && parent_result_node_ptr->node_y==yLoc &&
										can_move_to(xLoc-1,yLoc))
									{
										max_size_result_node_ptr->node_x = xLoc-1;
										max_size_result_node_ptr->node_y = yLoc;
										usePathSuccess++;
									}

									if(!usePathSuccess)
									{
										max_size_result_node_ptr->node_x = xLoc-1;
										max_size_result_node_ptr->node_y = yLoc+1;
									}

									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, nodeCount);
									break;

						case 3:
									if(parent_result_node_ptr->node_x>xLoc && can_move_to(xLoc+2,yLoc+1) &&
										can_move_to(xLoc+2,yLoc+2))
									{
										max_size_result_node_ptr->node_x = xLoc+1;
										max_size_result_node_ptr->node_y = yLoc+1;
										usePathSuccess++;
									}

									if(!usePathSuccess && parent_result_node_ptr->node_x<xLoc &&
										can_move_to(xLoc-1,yLoc+1) && can_move_to(xLoc-1,yLoc+2))
									{
										max_size_result_node_ptr->node_x = xLoc-1;
										max_size_result_node_ptr->node_y = yLoc+1;
										usePathSuccess++;
									}

									if(!usePathSuccess)
									{
										max_size_result_node_ptr->node_x = xLoc;
										max_size_result_node_ptr->node_y = yLoc+1;
									}

									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, nodeCount);
									break;

						case 4:
									if(parent_result_node_ptr->node_x==xLoc && //can_move_to(xLoc,yLoc+1) &&
										can_move_to(xLoc,yLoc+2))
									{
										max_size_result_node_ptr->node_x = xLoc;
										max_size_result_node_ptr->node_y = yLoc+1;
										usePathSuccess++;
									}

									if(!usePathSuccess && parent_result_node_ptr->node_y==yLoc && //can_move_to(xLoc+1,yLoc) &&
										can_move_to(xLoc+2,yLoc))
									{
										max_size_result_node_ptr->node_x = xLoc+1;
										max_size_result_node_ptr->node_y = yLoc;
										usePathSuccess++;
									}

									if(!usePathSuccess)
									{
										max_size_result_node_ptr->node_x = xLoc+1;
										max_size_result_node_ptr->node_y = yLoc+1;
									}

									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, nodeCount);
									break;

						case 5:
									if(parent_result_node_ptr->node_y<yLoc && can_move_to(xLoc+1,yLoc-1) &&
										can_move_to(xLoc+2,yLoc-1))
									{
										max_size_result_node_ptr->node_x = xLoc+1;
										max_size_result_node_ptr->node_y = yLoc-1;
										usePathSuccess++;
									}

									if(!usePathSuccess && parent_result_node_ptr->node_y>yLoc &&
										can_move_to(xLoc+1,yLoc+2) && can_move_to(xLoc+2,yLoc+2))
									{
										max_size_result_node_ptr->node_x = xLoc+1;
										max_size_result_node_ptr->node_y = yLoc+1;
										usePathSuccess++;
									}

									if(!usePathSuccess)
									{
										max_size_result_node_ptr->node_x = xLoc+1;
										max_size_result_node_ptr->node_y = yLoc;
									}

									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, nodeCount);
									break;

						case 6:
									if(parent_result_node_ptr->node_y==yLoc && //can_move_to(xLoc+1,yLoc+1) &&
										can_move_to(xLoc+2,yLoc+1))
									{
										max_size_result_node_ptr->node_x = xLoc+1;
										max_size_result_node_ptr->node_y = yLoc;
										usePathSuccess++;
									}

									if(!usePathSuccess && parent_result_node_ptr->node_x==xLoc && can_move_to(xLoc,yLoc-1))
									{
										max_size_result_node_ptr->node_x = xLoc;
										max_size_result_node_ptr->node_y = yLoc-1;
										usePathSuccess++;
									}

									if(!usePathSuccess)
									{
										max_size_result_node_ptr->node_x = xLoc+1;
										max_size_result_node_ptr->node_y = yLoc-1;
									}

									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, nodeCount);
									break;

						case 7:
									if(parent_result_node_ptr->node_x>xLoc && can_move_to(xLoc+2,yLoc-1) &&
										can_move_to(xLoc+2,yLoc))
									{
										max_size_result_node_ptr->node_x = xLoc+1;
										max_size_result_node_ptr->node_y = yLoc-1;
										usePathSuccess++;
									}

									if(!usePathSuccess && parent_result_node_ptr->node_x<xLoc &&
										can_move_to(xLoc-1,yLoc-1) && can_move_to(xLoc-1,yLoc))
									{
										max_size_result_node_ptr->node_x = xLoc-1;
										max_size_result_node_ptr->node_y = yLoc-1;
										usePathSuccess++;
									}

									if(!usePathSuccess)
									{
										max_size_result_node_ptr->node_x = xLoc;
										max_size_result_node_ptr->node_y = yLoc-1;
									}

									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, nodeCount);
									break;

						case 8:
									if(parent_result_node_ptr->node_x==xLoc && can_move_to(xLoc+1,yLoc-1))
									{
										max_size_result_node_ptr->node_x = xLoc;
										max_size_result_node_ptr->node_y = yLoc-1;
										usePathSuccess++;
									}

									if(!usePathSuccess && parent_result_node_ptr->node_y==yLoc && can_move_to(xLoc-1,yLoc+1))
									{
										max_size_result_node_ptr->node_x = xLoc-1;
										max_size_result_node_ptr->node_y = yLoc;
										usePathSuccess++;
									}

									if(!usePathSuccess)
									{
										max_size_result_node_ptr->node_x = xLoc-1;
										max_size_result_node_ptr->node_y = yLoc-1;
									}
									
									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, nodeCount);
									break;

						default:	err_here();
									break;
					}
					break;

		case 2:
					switch(exitDirection)
					{
						case 1:
									if(parent_result_node_ptr->node_y<yLoc && can_move_to(xLoc-1,yLoc-1) &&
										can_move_to(xLoc,yLoc-1) && can_move_to(xLoc+1,yLoc-1))
									{
										max_size_result_node_ptr->node_x = xLoc-1;
										max_size_result_node_ptr->node_y = yLoc-1;
										add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, nodeCount);

										max_size_result_node_ptr->node_x = xLoc;
										max_size_result_node_ptr->node_y = yLoc-1;
										usePathSuccess++;
									}

									if(!usePathSuccess && parent_result_node_ptr->node_y>yLoc && can_move_to(xLoc-1,yLoc+2) &&
										can_move_to(xLoc,yLoc+2) && can_move_to(xLoc+1,yLoc+2))
									{	
										max_size_result_node_ptr->node_x = xLoc-1;
										max_size_result_node_ptr->node_y = yLoc+1;
										add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, nodeCount);

										max_size_result_node_ptr->node_x = xLoc;
										max_size_result_node_ptr->node_y = yLoc+1;
										usePathSuccess++;
									}

									if(!usePathSuccess)
									{
										max_size_result_node_ptr->node_x = xLoc-1;
										max_size_result_node_ptr->node_y = yLoc;
										add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, nodeCount);

										max_size_result_node_ptr->node_x = xLoc;
										max_size_result_node_ptr->node_y = yLoc;
									}

									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, nodeCount);
									break;

						case 2:
									if(parent_result_node_ptr->node_y==yLoc+1 && can_move_to(xLoc+1,yLoc+2))
									{
										max_size_result_node_ptr->node_x = xLoc-1;
										max_size_result_node_ptr->node_y = yLoc+1;
										add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, nodeCount);

										max_size_result_node_ptr->node_x = xLoc;
										max_size_result_node_ptr->node_y = yLoc+1;
										usePathSuccess++;
									}
							
									if(!usePathSuccess && can_move_to(xLoc+1,yLoc+2) &&
										 ( (vir_start_x_s2-parent_result_node_ptr->node_x==parent_result_node_ptr->node_y-vir_start_y_s2) ||
											(parent_result_node_ptr->node_x==xLoc) ) )
									{
										max_size_result_node_ptr->node_x = xLoc;
										max_size_result_node_ptr->node_y = yLoc+1;
										usePathSuccess++;
									}
							
									if(!usePathSuccess && parent_result_node_ptr->node_y==yLoc &&
										can_move_to(xLoc-1,yLoc) && can_move_to(xLoc,yLoc))
									{
										max_size_result_node_ptr->node_x = xLoc-1;
										max_size_result_node_ptr->node_y = yLoc;
										add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, nodeCount);

										max_size_result_node_ptr->node_x = xLoc;
										max_size_result_node_ptr->node_y = yLoc;
										usePathSuccess++;
									}

									if(!usePathSuccess)
									{
										max_size_result_node_ptr->node_x = xLoc-1;
										max_size_result_node_ptr->node_y = yLoc+1;
										add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, nodeCount);

										err_when(!(nodeType&0x1) && !can_move_to(xLoc+1,yLoc+2));
										max_size_result_node_ptr->node_x = xLoc;
										max_size_result_node_ptr->node_y = yLoc+(!(nodeType&0x1));
									}

									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, nodeCount);
									break;

						case 3:
									if(parent_result_node_ptr->node_x>xLoc &&	//can_move_to(xLoc+2,yLoc+1) &&
										can_move_to(xLoc+2,yLoc+2))
									{
										max_size_result_node_ptr->node_x = xLoc+1;
										max_size_result_node_ptr->node_y = yLoc+1;
										usePathSuccess++;
									}

									if(!usePathSuccess)
									{
										max_size_result_node_ptr->node_x = xLoc;
										max_size_result_node_ptr->node_y = yLoc+1;
									}

									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, nodeCount);
									break;

						case 4:
									if(parent_result_node_ptr->node_x==xLoc && can_move_to(xLoc,yLoc+1) &&
										can_move_to(xLoc,yLoc+2))
									{
										max_size_result_node_ptr->node_x = xLoc;
										max_size_result_node_ptr->node_y = yLoc+1;
										usePathSuccess++;
									}

									if(!usePathSuccess)
									{
										if(abs(parent_result_node_ptr->node_x-vir_start_x_s2)>1 ||
										   abs(parent_result_node_ptr->node_y-vir_start_y_s2)>1)
										{
											max_size_result_node_ptr->node_x = xLoc+1;
											max_size_result_node_ptr->node_y = yLoc+1;
										}
										else
											break;
									}

									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, nodeCount);
									break;

						case 5:
									break;

						case 6:
									if(parent_result_node_ptr->node_x==xLoc && can_move_to(xLoc,yLoc-1) &&
										can_move_to(xLoc,yLoc))
									{
										max_size_result_node_ptr->node_x = xLoc;
										max_size_result_node_ptr->node_y = yLoc-1;
										usePathSuccess++;
									}

									if(!usePathSuccess)
									{
										if(abs(parent_result_node_ptr->node_x-vir_start_x_s2)>1 ||
										   abs(parent_result_node_ptr->node_y-vir_start_y_s2)>1)
										{
											max_size_result_node_ptr->node_x = xLoc+1;
											max_size_result_node_ptr->node_y = yLoc-1;
										}
										else
											break;
									}

									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, nodeCount);
									break;

						case 7:
									if(parent_result_node_ptr->node_x>xLoc &&	can_move_to(xLoc+2,yLoc-1) &&
										can_move_to(xLoc+2,yLoc))
									{
										max_size_result_node_ptr->node_x = xLoc+1;
										max_size_result_node_ptr->node_y = yLoc-1;
										usePathSuccess++;
									}

									if(!usePathSuccess)
									{
										max_size_result_node_ptr->node_x = xLoc;
										max_size_result_node_ptr->node_y = yLoc-1;
									}

									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, nodeCount);
									break;

						case 8:
									if(parent_result_node_ptr->node_y==yLoc-1 && can_move_to(xLoc+1,yLoc-1))
									{
										max_size_result_node_ptr->node_x = xLoc-1;
										max_size_result_node_ptr->node_y = yLoc-1;
										add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, nodeCount);

										max_size_result_node_ptr->node_x = xLoc;
										max_size_result_node_ptr->node_y = yLoc-1;
										usePathSuccess++;
									}
							
									if(!usePathSuccess && can_move_to(xLoc+1,yLoc-1) &&
										 ( (vir_start_x_s2-parent_result_node_ptr->node_x==vir_start_y_s2-parent_result_node_ptr->node_y) ||
											(parent_result_node_ptr->node_x==xLoc) ) )
									{
										max_size_result_node_ptr->node_x = xLoc;
										max_size_result_node_ptr->node_y = yLoc-1;
										usePathSuccess++;
									}
							
									if(!usePathSuccess && parent_result_node_ptr->node_y==yLoc &&
										can_move_to(xLoc-1,yLoc+1) && can_move_to(xLoc,yLoc+1))
									{
										max_size_result_node_ptr->node_x = xLoc-1;
										max_size_result_node_ptr->node_y = yLoc;
										add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, nodeCount);

										max_size_result_node_ptr->node_x = xLoc;
										max_size_result_node_ptr->node_y = yLoc;
										usePathSuccess++;
									}

									if(!usePathSuccess)
									{
										max_size_result_node_ptr->node_x = xLoc-1;
										max_size_result_node_ptr->node_y = yLoc-1;
										add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, nodeCount);

										err_when(!(nodeType&0x4) && !can_move_to(xLoc+1,yLoc-1));
										max_size_result_node_ptr->node_x = xLoc;
										max_size_result_node_ptr->node_y = yLoc-(!(nodeType&0x4));
									}

									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, nodeCount);
									break;

						default:	err_here();
									break;
					}
					break;

		case 3:
					switch(exitDirection)
					{
						case 1:
									if(parent_result_node_ptr->node_y>yLoc && can_move_to(xLoc-1,yLoc+2))
									{
										max_size_result_node_ptr->node_x = xLoc-1;
										max_size_result_node_ptr->node_y = yLoc+1;
										usePathSuccess++;
									}
							
									if(!usePathSuccess)
									{
										max_size_result_node_ptr->node_x = xLoc-1;
										max_size_result_node_ptr->node_y = yLoc;
									}

									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, nodeCount);
									break;

						case 2:	
									if(abs(parent_result_node_ptr->node_x-vir_start_x_s2)>1 ||
										abs(parent_result_node_ptr->node_y-vir_start_y_s2)>1)
									{
										max_size_result_node_ptr->node_x = xLoc-1;
										max_size_result_node_ptr->node_y = yLoc+1;
										add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, nodeCount);
									}
									break;

						case 3:
									break;

						case 4:	
									if(abs(parent_result_node_ptr->node_x-vir_start_x_s2)>1 ||
										abs(parent_result_node_ptr->node_y-vir_start_y_s2)>1)
									{
										max_size_result_node_ptr->node_x = xLoc+1;
										max_size_result_node_ptr->node_y = yLoc+1;
										add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, nodeCount);
									}
									break;

						case 5:	
									if(parent_result_node_ptr->node_y>yLoc && can_move_to(xLoc+2,yLoc+2))
									{
										max_size_result_node_ptr->node_x = xLoc+1;
										max_size_result_node_ptr->node_y = yLoc+1;
										usePathSuccess++;
									}

									if(!usePathSuccess)
									{
										max_size_result_node_ptr->node_x = xLoc+1;
										max_size_result_node_ptr->node_y = yLoc;
									}

									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, nodeCount);
									break;

						case 6:
									if(parent_result_node_ptr->node_y==yLoc && can_move_to(xLoc+2,yLoc+1))
									{
										max_size_result_node_ptr->node_x = xLoc+1;
										max_size_result_node_ptr->node_y = yLoc;
										usePathSuccess++;
									}

									if(!usePathSuccess && parent_result_node_ptr->node_x==xLoc &&
										can_move_to(xLoc,yLoc-1) && can_move_to(xLoc,yLoc))
									{								
										max_size_result_node_ptr->node_x = xLoc;
										max_size_result_node_ptr->node_y = yLoc-1;
										add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, nodeCount);

										max_size_result_node_ptr->node_x = xLoc;
										max_size_result_node_ptr->node_y = yLoc;
										usePathSuccess++;
									}

									if(!usePathSuccess)
									{
										max_size_result_node_ptr->node_x = xLoc+1;
										max_size_result_node_ptr->node_y = yLoc-1;
										add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, nodeCount);

										err_when(!(nodeType&0x1) && !can_move_to(xLoc+2, yLoc+1));
										max_size_result_node_ptr->node_x = xLoc+!(nodeType&0x1);
										max_size_result_node_ptr->node_y = yLoc;
									}

									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, nodeCount);
									break;

						case 7:
									if(parent_result_node_ptr->node_x>xLoc && can_move_to(xLoc+2,yLoc-1) &&
										can_move_to(xLoc+2,yLoc))
									{
										max_size_result_node_ptr->node_x = xLoc+1;
										max_size_result_node_ptr->node_y = yLoc-1;
										add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, nodeCount);

										max_size_result_node_ptr->node_x = xLoc+1;
										max_size_result_node_ptr->node_y = yLoc;
										usePathSuccess++;
									}

									if(!usePathSuccess && parent_result_node_ptr->node_x<xLoc &&
										can_move_to(xLoc-1,yLoc-1) && can_move_to(xLoc-1,yLoc))
									{
										max_size_result_node_ptr->node_x = xLoc-1;
										max_size_result_node_ptr->node_y = yLoc-1;
										add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, nodeCount);

										max_size_result_node_ptr->node_x = xLoc-1;
										max_size_result_node_ptr->node_y = yLoc;
										usePathSuccess++;
									}

									if(!usePathSuccess)
									{
										max_size_result_node_ptr->node_x = xLoc;
										max_size_result_node_ptr->node_y = yLoc-1;
										add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, nodeCount);

										max_size_result_node_ptr->node_x = xLoc;
										max_size_result_node_ptr->node_y = yLoc;
									}

									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, nodeCount);
									break;

						case 8:
									if(can_move_to(xLoc-1,yLoc+1) &&
										( (vir_start_x_s2-parent_result_node_ptr->node_x==vir_start_y_s2-parent_result_node_ptr->node_y)||
										  (parent_result_node_ptr->node_y==yLoc) ) )
									{
										max_size_result_node_ptr->node_x = xLoc-1;
										max_size_result_node_ptr->node_y = yLoc;
										usePathSuccess++;
									}

									if(!usePathSuccess && parent_result_node_ptr->node_x==xLoc &&
										can_move_to(xLoc+1,yLoc-1) && can_move_to(xLoc+1,yLoc))
									{
										max_size_result_node_ptr->node_x = xLoc;
										max_size_result_node_ptr->node_y = yLoc-1;
										add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, nodeCount);

										max_size_result_node_ptr->node_x = xLoc;
										max_size_result_node_ptr->node_y = yLoc;
										usePathSuccess++;
									}

									if(!usePathSuccess)
									{
										max_size_result_node_ptr->node_x = xLoc-1;
										max_size_result_node_ptr->node_y = yLoc-1;
										add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, nodeCount);
										
										err_when(!(nodeType&0x2) && !can_move_to(xLoc-1, yLoc+1));
										max_size_result_node_ptr->node_x = xLoc-!(nodeType&0x2);
										max_size_result_node_ptr->node_y = yLoc;
									}

									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, nodeCount);
									break;

						default:	err_here();
									break;
					}
					break;

		case 4:
					switch(exitDirection)
					{
						case 1:
									if(parent_result_node_ptr->node_y>yLoc && can_move_to(xLoc-1,yLoc+2) &&
										can_move_to(xLoc,yLoc+2))
									{
										max_size_result_node_ptr->node_x = xLoc-1;
										max_size_result_node_ptr->node_y = yLoc+1;
										add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, nodeCount);

										max_size_result_node_ptr->node_x = xLoc;
										max_size_result_node_ptr->node_y = yLoc+1;
										usePathSuccess++;
									}

									if(!usePathSuccess && can_move_to(xLoc,yLoc+2) && parent_result_node_ptr->node_y==yLoc-1)
									{
										max_size_result_node_ptr->node_x = xLoc-1;
										max_size_result_node_ptr->node_y = yLoc;
										add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, nodeCount);

										max_size_result_node_ptr->node_x = xLoc;
										max_size_result_node_ptr->node_y = yLoc+1;
										usePathSuccess++;
									}

									if(!usePathSuccess)
									{
										max_size_result_node_ptr->node_x = xLoc-1;
										max_size_result_node_ptr->node_y = yLoc;
										add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, nodeCount);

										err_when(!(nodeType&0x2) && !can_move_to(xLoc, yLoc+2));
										max_size_result_node_ptr->node_x = xLoc;
										max_size_result_node_ptr->node_y = yLoc+!(nodeType&0x2);
									}

									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, nodeCount);
									break;

						case 2:
									if(vir_start_x_s2-parent_result_node_ptr->node_x==parent_result_node_ptr->node_y+1-vir_start_y_s2)
									{
										max_size_result_node_ptr->node_x = xLoc;
										max_size_result_node_ptr->node_y = yLoc+1;
										usePathSuccess++;
									}

									if(!usePathSuccess)
									{
										if(abs(parent_result_node_ptr->node_x-vir_start_x_s2)>1 ||
										   abs(parent_result_node_ptr->node_y-vir_start_y_s2)>1)
										{
											max_size_result_node_ptr->node_x = xLoc-1;
											max_size_result_node_ptr->node_y = yLoc+1;
											add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, nodeCount);

											max_size_result_node_ptr->node_x = xLoc;
											max_size_result_node_ptr->node_y = yLoc+1;
										}
										else
											break;
									}

									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, nodeCount);
									break;

						case 3:
									if(abs(parent_result_node_ptr->node_x-vir_start_x_s2)>1 ||
										abs(parent_result_node_ptr->node_y-vir_start_y_s2)>1)
									{
										max_size_result_node_ptr->node_x = xLoc;
										max_size_result_node_ptr->node_y = yLoc+1;
										add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, nodeCount);
									}
									break;

						case 4:
									break;

						case 5:
									if(abs(parent_result_node_ptr->node_x-vir_start_x_s2)>1 ||
										abs(parent_result_node_ptr->node_y-vir_start_y_s2)>1)
									{
										max_size_result_node_ptr->node_x = xLoc+1;
										max_size_result_node_ptr->node_y = yLoc;
										add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, nodeCount);
									}
									break;

						case 6:	
									if(parent_result_node_ptr->node_x+1-vir_start_x_s2==vir_start_y_s2-parent_result_node_ptr->node_y)
									{
										max_size_result_node_ptr->node_x = xLoc+1;
										max_size_result_node_ptr->node_y = yLoc;
										usePathSuccess++;
									}

									if(!usePathSuccess)
									{
										if(abs(parent_result_node_ptr->node_x-vir_start_x_s2)>1 ||
										   abs(parent_result_node_ptr->node_y-vir_start_y_s2)>1)
										{
											max_size_result_node_ptr->node_x = xLoc+1;
											max_size_result_node_ptr->node_y = yLoc-1;
											add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, nodeCount);

											max_size_result_node_ptr->node_x = xLoc+1;
											max_size_result_node_ptr->node_y = yLoc;
										}
										else
											break;
									}

									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, nodeCount);
									break;

						case 7:
									if(parent_result_node_ptr->node_x>xLoc && can_move_to(xLoc+2,yLoc-1) &&
										can_move_to(xLoc+2,yLoc))
									{
										max_size_result_node_ptr->node_x = xLoc+1;
										max_size_result_node_ptr->node_y = yLoc-1;
										add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, nodeCount);

										max_size_result_node_ptr->node_x = xLoc+1;
										max_size_result_node_ptr->node_y = yLoc;
										usePathSuccess++;
									}

									if(!usePathSuccess && parent_result_node_ptr->node_x<xLoc &&
										can_move_to(xLoc+2,yLoc))
									{
										max_size_result_node_ptr->node_x = xLoc;
										max_size_result_node_ptr->node_y = yLoc-1;
										add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, nodeCount);

										max_size_result_node_ptr->node_x = xLoc+1;
										max_size_result_node_ptr->node_y = yLoc;
										usePathSuccess++;
									}

									if(!usePathSuccess)
									{
										max_size_result_node_ptr->node_x = xLoc;
										max_size_result_node_ptr->node_y = yLoc-1;
										add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, nodeCount);

										err_when(!(nodeType&0x4) && !can_move_to(xLoc+2, yLoc));
										max_size_result_node_ptr->node_x = xLoc+!(nodeType&0x4);
										max_size_result_node_ptr->node_y = yLoc;
									}

									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, nodeCount);
									break;

						case 8:
									if(parent_result_node_ptr->node_x==xLoc && can_move_to(xLoc+1,yLoc-1))
									{
										max_size_result_node_ptr->node_x = xLoc;
										max_size_result_node_ptr->node_y = yLoc-1;
										usePathSuccess++;
									}

									if(!usePathSuccess && parent_result_node_ptr->node_y==yLoc && can_move_to(xLoc-1,yLoc+1))
									{
										max_size_result_node_ptr->node_x = xLoc-1;
										max_size_result_node_ptr->node_y = yLoc;
										usePathSuccess++;
									}

									if(!usePathSuccess)
									{
										max_size_result_node_ptr->node_x = xLoc-1;
										max_size_result_node_ptr->node_y = yLoc-1;
									}

									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, nodeCount);
									err_when(!can_move_to(xLoc+1, yLoc) || !can_move_to(xLoc, yLoc+1));
									max_size_result_node_ptr->node_x = xLoc;
									max_size_result_node_ptr->node_y = yLoc;
									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, nodeCount);
									break;

						default:	err_here();
									break;
					}
					break;

		default:	err_here();
					break;
	}
}
//------ End of function SeekPathS2::process_start_node ---------//


//---- Begin of function SeekPathS2::get_real_result_node ---------//
//
// called by get_result to extract the point in a 2x2 node that is 
// used in the shortest path
//
// <int&>  count - a reference var for counting number of node in 
//						 max_size_result_node_ptr
//
void SeekPathS2::get_real_result_node(int &count, short enterDirection, short exitDirection,
												  short nodeType, short xCoord, short yCoord)
{	
	prefer_upper_s2 = prefer_lower_s2 = prefer_left_s2 = prefer_right_s2 = 0;

	switch(enterDirection)
	{
		case 0:	err_here();
					break;
	
		case 1:	
					switch(exitDirection)
					{
						case 2:	// 2 possible paths
									if(parent_result_node_ptr->node_x<xCoord)
										prefer_left_s2 = 1;
									else
									{
										if(!can_move_to(xCoord+1,yCoord+1) || !can_move_to(xCoord+1,yCoord+2))
											prefer_left_s2 = 1;
										else
											prefer_right_s2 = 1;
									}
									max_size_result_node_ptr->node_x = xCoord-prefer_left_s2;
									max_size_result_node_ptr->node_y = yCoord+1;
									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);
									break;

						case 3:	// 2 possible paths 
									if(parent_result_node_ptr->node_x>=xCoord)
										prefer_right_s2 = 1;
									else
									{
										if(can_move_to(xCoord-1,yCoord+2))
											prefer_left_s2 = 1;
										else
											prefer_right_s2 = 1;
									}
									max_size_result_node_ptr->node_x = xCoord-prefer_left_s2;
									max_size_result_node_ptr->node_y = yCoord+1;
									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);
									break;

						case 4:	// 2 points, 2 paths
									max_size_result_node_ptr->node_x = xCoord+1;
									max_size_result_node_ptr->node_y = yCoord+1;
									//----------- process path selection ---------------//
									if( (parent_result_node_ptr->node_x-max_size_result_node_ptr->node_x==0) ||
										 (parent_result_node_ptr->node_y-max_size_result_node_ptr->node_y==0) )
										prefer_lower_s2 = 1;
									else
										prefer_upper_s2 = 1;

									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);
									err_when(!(nodeType&0x2) && !can_move_to(xCoord, yCoord+2));
									
									max_size_result_node_ptr->node_x = xCoord;
									if(prefer_upper_s2)
										max_size_result_node_ptr->node_y = yCoord + !(nodeType&0x2);
									else // prefer_lower_s2
										max_size_result_node_ptr->node_y = yCoord + can_move_to(xCoord, yCoord+2);

									err_when(max_size_result_node_ptr->node_y!=yCoord && max_size_result_node_ptr->node_y!=yCoord+1);
									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);
									break;

						case 5: // 2 points, 2 paths
									//----------- process path selection ---------------//
									if(parent_result_node_ptr->node_y<=yCoord)
										prefer_upper_s2 = 1;
									else
									{
										if(!can_move_to(xCoord,yCoord+2) || !can_move_to(xCoord+1,yCoord+2) ||
											!can_move_to(xCoord+2,yCoord+2))
											prefer_upper_s2 = 1;
										else
											prefer_lower_s2 = 1;
									}
							
									max_size_result_node_ptr->node_x = xCoord+1;
									max_size_result_node_ptr->node_y = yCoord+prefer_lower_s2;
									err_when(max_size_result_node_ptr->node_y!=yCoord && max_size_result_node_ptr->node_y!=yCoord+1);
									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);

									max_size_result_node_ptr->node_x = xCoord;
									max_size_result_node_ptr->node_y = yCoord+prefer_lower_s2;
									err_when(max_size_result_node_ptr->node_y!=yCoord && max_size_result_node_ptr->node_y!=yCoord+1);
									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);
									break;

						case 6: // 2 points, 2 paths
									max_size_result_node_ptr->node_x = xCoord+1;
									max_size_result_node_ptr->node_y = yCoord-1;
									//----------- process path selection ---------------//
									if( (parent_result_node_ptr->node_x-max_size_result_node_ptr->node_x==0) ||
										 (parent_result_node_ptr->node_y-max_size_result_node_ptr->node_y==0) )
										prefer_upper_s2 = 1;
									else
										prefer_lower_s2 = 1;

									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);
									
									err_when(!(nodeType&0x8) && !can_move_to(xCoord, yCoord-1));
									max_size_result_node_ptr->node_x = xCoord;
									if(prefer_upper_s2)
										max_size_result_node_ptr->node_y = yCoord-can_move_to(xCoord, yCoord-1);
									else	// prefer_lower_s2
										max_size_result_node_ptr->node_y = yCoord-!(nodeType&0x8);

									err_when(max_size_result_node_ptr->node_y!=yCoord && max_size_result_node_ptr->node_y!=yCoord-1);
									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);
									break;

						case 7:	// 2 possible paths
									if(parent_result_node_ptr->node_x>=xCoord)
										prefer_right_s2 = 1;
									else
									{
										if(can_move_to(xCoord-1,yCoord-1))
											prefer_left_s2 = 1;
										else
											prefer_right_s2 = 1;
									}
									max_size_result_node_ptr->node_x = xCoord-prefer_left_s2;
									max_size_result_node_ptr->node_y = yCoord-1;
									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);
									break;

						case 8:	// 2 possible paths
									if(parent_result_node_ptr->node_x<xCoord)
										prefer_left_s2 = 1;
									else
									{
										if(!can_move_to(xCoord+1,yCoord-1) || !can_move_to(xCoord+1,yCoord))
											prefer_left_s2 = 1;
										else
											prefer_right_s2 = 1;
									}
									max_size_result_node_ptr->node_x = xCoord-prefer_left_s2;
									max_size_result_node_ptr->node_y = yCoord-1;
									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);
									break;

						default:	err_here();
									break;
					}
					break;

		case 2:	
				switch(exitDirection)
				{
						case 1:
									max_size_result_node_ptr->node_x = xCoord-1;
									max_size_result_node_ptr->node_y = yCoord;
									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);
									break;

						case 3:
									max_size_result_node_ptr->node_x = xCoord;
									max_size_result_node_ptr->node_y = yCoord+1;
									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);
									break;

						case 4: // 2 points, 2 possible paths
									if(parent_result_node_ptr->node_y<=yCoord+1)
										prefer_upper_s2 = 1;
									else
									{
										if(!can_move_to(xCoord, yCoord+3) || !can_move_to(xCoord+1, yCoord+3) ||
											!can_move_to(xCoord+2, yCoord+3))
											prefer_upper_s2 = 1;
										else
											prefer_lower_s2 = 1;
									}
									max_size_result_node_ptr->node_x = xCoord+1;
									max_size_result_node_ptr->node_y = yCoord+1+prefer_lower_s2;
									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);

									max_size_result_node_ptr->node_x = xCoord;
									max_size_result_node_ptr->node_y = yCoord+1+prefer_lower_s2;
									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);
									break;

						case 5: // 2 points, 2 paths
									if(parent_result_node_ptr->node_y<=yCoord)
										prefer_upper_s2 = 1;
									else
									{
										if(!can_move_to(xCoord+2,yCoord+2) || !can_move_to(xCoord+1,yCoord+2))
											prefer_upper_s2 = 1;
										else
											prefer_lower_s2 = 1;
									}
									max_size_result_node_ptr->node_x = xCoord+1;
									max_size_result_node_ptr->node_y = yCoord+prefer_lower_s2;

									err_when(max_size_result_node_ptr->node_y!=yCoord && max_size_result_node_ptr->node_y!=yCoord+1);
									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);

									err_when(!(nodeType&0x1) && !can_move_to(xCoord+1, yCoord+2));
									max_size_result_node_ptr->node_x = xCoord;
									max_size_result_node_ptr->node_y = yCoord+(prefer_lower_s2 || !(nodeType&0x1));
									err_when(max_size_result_node_ptr->node_y!=yCoord && max_size_result_node_ptr->node_y!=yCoord+1);
									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);
									break;

						case 6: // 2 points
									max_size_result_node_ptr->node_x = xCoord+1;
									max_size_result_node_ptr->node_y = yCoord-1;
									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);

									err_when(nodeType != 15);
									max_size_result_node_ptr->node_x = xCoord;
									max_size_result_node_ptr->node_y = yCoord;
									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);
									break;

						case 7: // 2 points, 2 paths
									max_size_result_node_ptr->node_x = xCoord;
									max_size_result_node_ptr->node_y = yCoord-1;
									//----------- process path selection ---------------//
									if( (parent_result_node_ptr->node_x-max_size_result_node_ptr->node_x==0) ||
										 (parent_result_node_ptr->node_y-max_size_result_node_ptr->node_y==0) )
										prefer_right_s2 = 1;
									else
										prefer_left_s2 = 1;

									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);

									err_when(!can_move_to(xCoord-1,yCoord) && !(nodeType&0x8));
									max_size_result_node_ptr->node_y = yCoord;
									if(prefer_left_s2)
										max_size_result_node_ptr->node_x = xCoord-can_move_to(xCoord-1,yCoord);
									else
										max_size_result_node_ptr->node_x = xCoord-!(nodeType&0x8);
									
									err_when(max_size_result_node_ptr->node_x!=xCoord && max_size_result_node_ptr->node_x!=xCoord-1);
									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);
									break;

						case 8: // 2 points, 2 possible paths
									if(parent_result_node_ptr->node_x>=xCoord-1)
										prefer_right_s2 = 1;
									else
									{
										if(!can_move_to(xCoord-2,yCoord-1) || !can_move_to(xCoord-2,yCoord) ||
											!can_move_to(xCoord-2,yCoord+1))	// must check for all 3 points to avoid problems in dummy node generated in path-reuse
											prefer_right_s2 = 1;
										else
											prefer_left_s2 = 1;
									}
									max_size_result_node_ptr->node_x = xCoord-1-prefer_left_s2;
									max_size_result_node_ptr->node_y = yCoord-1;
									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);

									max_size_result_node_ptr->node_x = xCoord-1-prefer_left_s2;
									max_size_result_node_ptr->node_y = yCoord;
									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);
									break;

						default:	err_here();
									break;
					}
					break;
					
		case 3:	switch(exitDirection)
					{
						case 1:	// 2 possible paths
									if(parent_result_node_ptr->node_y<=yCoord)
										prefer_upper_s2 = 1;
									else
									{
										if(can_move_to(xCoord-1,yCoord+2))
											prefer_lower_s2 = 1;
										else
											prefer_upper_s2 = 1;
									}
									max_size_result_node_ptr->node_x = xCoord-1;
									max_size_result_node_ptr->node_y = yCoord+prefer_lower_s2;
									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);
									break;

						case 2:	// 2 possible paths
									if(parent_result_node_ptr->node_y>yCoord)
										prefer_lower_s2 = 1;
									else
									{
										if(!can_move_to(xCoord, yCoord) || !can_move_to(xCoord-1,yCoord))
											prefer_lower_s2 = 1;
										else
											prefer_upper_s2 = 1;
									}
									max_size_result_node_ptr->node_x = xCoord-1;
									max_size_result_node_ptr->node_y = yCoord+prefer_lower_s2;
									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);
									break;

						case 4:	// 2 possible paths
									if(parent_result_node_ptr->node_y>yCoord)
										prefer_lower_s2 = 1;
									else
									{
										if(!can_move_to(xCoord+1,yCoord) || !can_move_to(xCoord+2,yCoord))
											prefer_lower_s2 = 1;
										else
											prefer_upper_s2 = 1;
									}
									max_size_result_node_ptr->node_x = xCoord+1;
									max_size_result_node_ptr->node_y = yCoord+prefer_lower_s2;
									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);
									break;

						case 5:	// 2 possible paths
									if(parent_result_node_ptr->node_y<=yCoord)
										prefer_upper_s2 = 1;
									else
									{
										if(can_move_to(xCoord+2,yCoord+2))
											prefer_lower_s2 = 1;
										else
											prefer_upper_s2 = 1;
									}
									max_size_result_node_ptr->node_x = xCoord+1;
									max_size_result_node_ptr->node_y = yCoord+prefer_lower_s2;
									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);
									break;

						case 6: // 2 points. 2 paths
									max_size_result_node_ptr->node_x = xCoord+1;
									max_size_result_node_ptr->node_y = yCoord-1;
									//----------- process path selection ---------------//
									if( (parent_result_node_ptr->node_x-max_size_result_node_ptr->node_x==0) ||
										 (parent_result_node_ptr->node_y-max_size_result_node_ptr->node_y==0) )
										prefer_right_s2 = 1;
									else
										prefer_left_s2 = 1;

									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);

									err_when(!(nodeType&0x1) && !can_move_to(xCoord+2, yCoord+1));
									max_size_result_node_ptr->node_y = yCoord;
									if(prefer_left_s2)
										max_size_result_node_ptr->node_x = xCoord + !(nodeType&0x1);
									else	// prefer_right_s2
										max_size_result_node_ptr->node_x = xCoord + can_move_to(xCoord+2, yCoord+1);

									err_when(max_size_result_node_ptr->node_x!=xCoord && max_size_result_node_ptr->node_x!=xCoord+1);
									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);
									break;

						case 7: // 2 points, 2 paths
									max_size_result_node_ptr->node_y = yCoord-1;
									//----------- process path selection ---------------//
									if(parent_result_node_ptr->node_x<=xCoord)
										prefer_left_s2 = 1;
									else
									{
										if(!can_move_to(xCoord+2,yCoord-1) || !can_move_to(xCoord+2,yCoord) ||
											!can_move_to(xCoord+2,yCoord+1))
											prefer_left_s2 = 1;
										else
											prefer_right_s2 = 1;
									}
									max_size_result_node_ptr->node_x = xCoord+prefer_right_s2;

									err_when(max_size_result_node_ptr->node_x!=xCoord && max_size_result_node_ptr->node_x!=xCoord+1);
									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);

									max_size_result_node_ptr->node_x = xCoord+prefer_right_s2;
									max_size_result_node_ptr->node_y = yCoord;
									err_when(max_size_result_node_ptr->node_x!=xCoord && max_size_result_node_ptr->node_x!=xCoord+1);
									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);
									break;

						case 8: // 2 points, 2 paths
									max_size_result_node_ptr->node_x = xCoord-1;
									max_size_result_node_ptr->node_y = yCoord-1;
									//----------- process path selection ---------------//
									if( (parent_result_node_ptr->node_x-max_size_result_node_ptr->node_x==0) ||
										 (parent_result_node_ptr->node_y-max_size_result_node_ptr->node_y==0) )
										prefer_left_s2 = 1;
									else
										prefer_right_s2 = 1;

									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);

									err_when(!(nodeType&0x2) && !can_move_to(xCoord-1, yCoord+1));
									max_size_result_node_ptr->node_y = yCoord;
									if(prefer_left_s2)
										max_size_result_node_ptr->node_x = xCoord - can_move_to(xCoord-1, yCoord+1);
									else	// prefer_right_s2
										max_size_result_node_ptr->node_x = xCoord - !(nodeType&0x2);
									
									err_when(max_size_result_node_ptr->node_x!=xCoord && max_size_result_node_ptr->node_x!=xCoord-1);
									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);
									break;

						default:	err_here();
									break;
					}
					break;

		case 4:	switch(exitDirection)
					{
						case 1: // 2 points, 2 paths
									max_size_result_node_ptr->node_x = xCoord-1;
									//----------- process path selection ---------------//
									if(parent_result_node_ptr->node_y<=yCoord)
										prefer_upper_s2 = 1;
									else
									{
										if(!can_move_to(xCoord-1,yCoord+2) || !can_move_to(xCoord,yCoord+2))
											prefer_upper_s2 = 1;
										else
											prefer_lower_s2 = 1;
									}
									max_size_result_node_ptr->node_y = yCoord+prefer_lower_s2;

									err_when(max_size_result_node_ptr->node_y!=yCoord && max_size_result_node_ptr->node_y!=yCoord+1);
									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);

									err_when(!(nodeType&0x2) && !can_move_to(xCoord, yCoord+2));
									max_size_result_node_ptr->node_x = xCoord;
									max_size_result_node_ptr->node_y = yCoord+(prefer_lower_s2 || !(nodeType&0x2));
									err_when(max_size_result_node_ptr->node_y!=yCoord && max_size_result_node_ptr->node_y!=yCoord+1);
									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);
									break;

						case 2: // 2 points, 2 possible paths
									if(parent_result_node_ptr->node_y<=yCoord+1)
										prefer_upper_s2 = 1;
									else
									{
										if(!can_move_to(xCoord-1, yCoord+3) || !can_move_to(xCoord, yCoord+3) ||
											!can_move_to(xCoord+1, yCoord+3))
											prefer_upper_s2 = 1;
										else
											prefer_lower_s2 = 1;
									}
									max_size_result_node_ptr->node_x = xCoord-1;
									max_size_result_node_ptr->node_y = yCoord+1+prefer_lower_s2;
									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);

									max_size_result_node_ptr->node_x = xCoord;
									max_size_result_node_ptr->node_y = yCoord+1+prefer_lower_s2;
									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);
									break;

						case 3:
									max_size_result_node_ptr->node_x = xCoord;
									max_size_result_node_ptr->node_y = yCoord+1;
									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);
									break;

						case 5:
									max_size_result_node_ptr->node_x = xCoord+1;
									max_size_result_node_ptr->node_y = yCoord;
									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);
									break;

						case 6: // 2 points, 2 possible paths
									if(parent_result_node_ptr->node_x<=xCoord+1)
										prefer_left_s2 = 1;
									else
									{
										if(!can_move_to(xCoord+3,yCoord-1) || !can_move_to(xCoord+3,yCoord) ||
											!can_move_to(xCoord+3,yCoord+1))
											prefer_left_s2 = 1;
										else
											prefer_right_s2 = 1;
									}
									max_size_result_node_ptr->node_x = xCoord+1+prefer_right_s2;
									max_size_result_node_ptr->node_y = yCoord-1;
									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);

									max_size_result_node_ptr->node_x = xCoord+1+prefer_right_s2;
									max_size_result_node_ptr->node_y = yCoord;
									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);
									break;

						case 7: // 2 points, 2 paths
									max_size_result_node_ptr->node_y = yCoord-1;
									//----------- process path selection ---------------//
									if(parent_result_node_ptr->node_x<=xCoord)
										prefer_left_s2 = 1;
									else
									{
										if(!can_move_to(xCoord+2,yCoord-1) || !can_move_to(xCoord+2,yCoord))
											prefer_left_s2 = 1;
										else
											prefer_right_s2 = 1;
									}
									max_size_result_node_ptr->node_x = xCoord+prefer_right_s2;

									err_when(max_size_result_node_ptr->node_x!=xCoord && max_size_result_node_ptr->node_x!=xCoord+1);
									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);

									err_when(!(nodeType&0x4) && !can_move_to(xCoord+2, yCoord));
									max_size_result_node_ptr->node_x = xCoord+(prefer_right_s2 || !(nodeType&0x4));
									max_size_result_node_ptr->node_y = yCoord;
									err_when(max_size_result_node_ptr->node_x!=xCoord && max_size_result_node_ptr->node_x!=xCoord+1);
									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);
									break;

						case 8: // 2 points
									max_size_result_node_ptr->node_x = xCoord-1;
									max_size_result_node_ptr->node_y = yCoord-1;
									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);

									err_when(nodeType!=15);
									max_size_result_node_ptr->node_x = xCoord;
									max_size_result_node_ptr->node_y = yCoord;
									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);
									break;

						default:	err_here();
									break;
					}
					break;

		case 5:	switch(exitDirection)
					{
						case 1: // 2 points, 2 paths
									max_size_result_node_ptr->node_x = xCoord-1;
									//----------- process path selection ---------------//
									if(parent_result_node_ptr->node_y<=yCoord)
										prefer_upper_s2 = 1;
									else
									{
										if(!can_move_to(xCoord-1,yCoord+2) || !can_move_to(xCoord,yCoord+2) ||
											!can_move_to(xCoord+1,yCoord+2))
											prefer_upper_s2 = 1;
										else
											prefer_lower_s2 = 1;
									}
									max_size_result_node_ptr->node_y = yCoord+prefer_lower_s2;

									err_when(max_size_result_node_ptr->node_y!=yCoord && max_size_result_node_ptr->node_y!=yCoord+1);
									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);

									max_size_result_node_ptr->node_x = xCoord;
									max_size_result_node_ptr->node_y = yCoord+prefer_lower_s2;

									err_when(max_size_result_node_ptr->node_y!=yCoord && max_size_result_node_ptr->node_y!=yCoord+1);
									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);
									break;

						case 2: // 2 points, 2 paths
									max_size_result_node_ptr->node_x = xCoord-1;
									max_size_result_node_ptr->node_y = yCoord+1;
									//----------- process path selection ---------------//
									if( (parent_result_node_ptr->node_x-max_size_result_node_ptr->node_x==0) ||
										 (parent_result_node_ptr->node_y-max_size_result_node_ptr->node_y==0) )
										prefer_lower_s2 = 1;
									else
										prefer_upper_s2 = 1;

									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);

									err_when(!(nodeType&0x1) && !can_move_to(xCoord+1, yCoord+2));
									max_size_result_node_ptr->node_x = xCoord;
									if(prefer_upper_s2)
										max_size_result_node_ptr->node_y = yCoord + !(nodeType&0x1);
									else	// prefer_lower_s2
										max_size_result_node_ptr->node_y = yCoord + can_move_to(xCoord+1, yCoord+2);

									err_when(max_size_result_node_ptr->node_y!=yCoord && max_size_result_node_ptr->node_y!=yCoord+1);
									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);
									break;

						case 3:	// 2 possible paths
									if(parent_result_node_ptr->node_x<=xCoord)
										prefer_left_s2 = 1;
									else
									{
										if(can_move_to(xCoord+2,yCoord+2))
											prefer_right_s2 = 1;
										else
											prefer_left_s2 = 1;
									}
									max_size_result_node_ptr->node_x = xCoord+prefer_right_s2;
									max_size_result_node_ptr->node_y = yCoord+1;
									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);
									break;

						case 4:	// 2 possible paths 
									if(parent_result_node_ptr->node_x>xCoord)
										prefer_right_s2 = 1;
									else
									{
										if(!can_move_to(xCoord,yCoord+1) || !can_move_to(xCoord,yCoord+2))
											prefer_right_s2 = 1;
										else
											prefer_left_s2 = 1;
									}

									max_size_result_node_ptr->node_x = xCoord+prefer_right_s2;
									max_size_result_node_ptr->node_y = yCoord+1;
									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);
									break;

						case 6:	// 2 possible paths
									if(parent_result_node_ptr->node_x>xCoord)
										prefer_right_s2 = 1;
									else
									{
										if(!can_move_to(xCoord,yCoord-1) || !can_move_to(xCoord,yCoord))
											prefer_right_s2 = 1;
										else
											prefer_left_s2 = 1;
									}
									max_size_result_node_ptr->node_x = xCoord+prefer_right_s2;
									max_size_result_node_ptr->node_y = yCoord-1;
									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);
									break;

						case 7:	// 2 possible paths
									if(parent_result_node_ptr->node_x<=xCoord)
										prefer_left_s2 = 1;
									else
									{
										if(can_move_to(xCoord+2,yCoord-1))
											prefer_right_s2 = 1;
										else
											prefer_left_s2 = 1;
									}
									max_size_result_node_ptr->node_x = xCoord+prefer_right_s2;
									max_size_result_node_ptr->node_y = yCoord-1;
									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);
									break;

						case 8: // 2 points
									max_size_result_node_ptr->node_x = xCoord-1;
									max_size_result_node_ptr->node_y = yCoord-1;
									//----------- process path selection ---------------//
									if( (parent_result_node_ptr->node_x-max_size_result_node_ptr->node_x==0) ||
										 (parent_result_node_ptr->node_y-max_size_result_node_ptr->node_y==0) )
										prefer_upper_s2 = 1;
									else
										prefer_lower_s2 = 1;

									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);

									err_when(!(nodeType&0x4) && !can_move_to(xCoord+1, yCoord-1));
									max_size_result_node_ptr->node_x = xCoord;
									if(prefer_upper_s2)
										max_size_result_node_ptr->node_y = yCoord - can_move_to(xCoord+1, yCoord-1);
									else	// prefer_lower_s2
										max_size_result_node_ptr->node_y = yCoord - !(nodeType&0x4);

									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);
									break;

						default:	err_here();
									break;
					}
					break;

		case 6:	switch(exitDirection)
					{
						case 1: // 2 points, 2 paths
									max_size_result_node_ptr->node_x = xCoord-1;
									max_size_result_node_ptr->node_y = yCoord;
									//----------- process path selection ---------------//
									if( (parent_result_node_ptr->node_x-max_size_result_node_ptr->node_x==0) ||
										 (parent_result_node_ptr->node_y-max_size_result_node_ptr->node_y==0) )
										prefer_lower_s2 = 1;
									else
										prefer_upper_s2 = 1;

									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);

									err_when(!(nodeType&0x8) && !can_move_to(xCoord, yCoord-1));
									max_size_result_node_ptr->node_x = xCoord;
									if(prefer_upper_s2)
										max_size_result_node_ptr->node_y = yCoord - can_move_to(xCoord, yCoord-1);
									else	// prefer_lower_s2
										max_size_result_node_ptr->node_y = yCoord - !(nodeType&0x8);

									err_when(max_size_result_node_ptr->node_y!=yCoord && max_size_result_node_ptr->node_y!=yCoord-1);
									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);
									break;

						case 2: // 2 points
									max_size_result_node_ptr->node_x = xCoord-1;
									max_size_result_node_ptr->node_y = yCoord+1;
									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);

									err_when(nodeType != 15);
									max_size_result_node_ptr->node_x = xCoord;
									max_size_result_node_ptr->node_y = yCoord;
									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);
									break;

						case 3: // 2 points, 2 paths
									max_size_result_node_ptr->node_x = xCoord;
									max_size_result_node_ptr->node_y = yCoord+1;
									//----------- process path selection ---------------//
									if( (parent_result_node_ptr->node_x-max_size_result_node_ptr->node_x==0) ||
										 (parent_result_node_ptr->node_y-max_size_result_node_ptr->node_y==0) )
										prefer_left_s2 = 1;
									else
										prefer_right_s2 = 1;

									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);

									err_when(!(nodeType&0x1) && !can_move_to(xCoord+2, yCoord+1));
									max_size_result_node_ptr->node_y = yCoord;
									if(prefer_left_s2)
										max_size_result_node_ptr->node_x = xCoord + !(nodeType&0x1);
									else	// prefer_right_s2
										max_size_result_node_ptr->node_x = xCoord + can_move_to(xCoord+2, yCoord+1);

									err_when(max_size_result_node_ptr->node_y!=yCoord && max_size_result_node_ptr->node_y!=yCoord-1);
									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);
									break;

						case 4:	// 2 points, 2 possible paths
									if(parent_result_node_ptr->node_x<=xCoord+1)
										prefer_left_s2 = 1;
									else
									{
										if(!can_move_to(xCoord+3,yCoord) || !can_move_to(xCoord+3,yCoord+1) ||
											!can_move_to(xCoord+3,yCoord+2))
											prefer_left_s2 = 1;
										else
											prefer_right_s2 = 1;
									}
									max_size_result_node_ptr->node_x = xCoord+1+prefer_right_s2;
									max_size_result_node_ptr->node_y = yCoord+1;
									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);

									max_size_result_node_ptr->node_x = xCoord+1+prefer_right_s2;
									max_size_result_node_ptr->node_y = yCoord;
									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);
									break;

						case 5: 
									max_size_result_node_ptr->node_x = xCoord+1;
									max_size_result_node_ptr->node_y = yCoord;
									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);
									break;

						case 7:
									max_size_result_node_ptr->node_x = xCoord;
									max_size_result_node_ptr->node_y = yCoord-1;
									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);
									break;

						case 8: // 2 points, 2 possible paths
									if(parent_result_node_ptr->node_y>=yCoord-1)
										prefer_lower_s2 = 1;
									else
									{
										if(!can_move_to(xCoord-1,yCoord-2) || !can_move_to(xCoord,yCoord-2) ||
											!can_move_to(xCoord+1,yCoord-2)) // must check for all 3 points to avoid problems in dummy node generated in path-reuse
											prefer_lower_s2 = 1;
										else
											prefer_upper_s2 = 1;
									}
									max_size_result_node_ptr->node_x = xCoord-1;
									max_size_result_node_ptr->node_y = yCoord-1-prefer_upper_s2;
									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);

									max_size_result_node_ptr->node_x = xCoord;
									max_size_result_node_ptr->node_y = yCoord-1-prefer_upper_s2;
									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);
									break;
					
						default:	err_here();
									break;
					}
					break;

		case 7:	switch(exitDirection)
					{
						case 1:	// 2 possible paths
									if(parent_result_node_ptr->node_y>=yCoord)
										prefer_lower_s2 = 1;
									else
									{
										if(can_move_to(xCoord-1,yCoord-1))
											prefer_upper_s2 = 1;
										else
											prefer_lower_s2 = 1;
									}
									max_size_result_node_ptr->node_x = xCoord-1;
									max_size_result_node_ptr->node_y = yCoord-prefer_upper_s2;
									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);
									break;

						case 2: // 2 points, 2 paths
									max_size_result_node_ptr->node_x = xCoord-1;
									max_size_result_node_ptr->node_y = yCoord+1;
									//----------- process path selection ---------------//
									if( (parent_result_node_ptr->node_x-max_size_result_node_ptr->node_x==0) ||
										 (parent_result_node_ptr->node_y-max_size_result_node_ptr->node_y==0) )
										prefer_left_s2 = 1;
									else
										prefer_right_s2 = 1;

									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);

									err_when(!(nodeType&0x8) && !can_move_to(xCoord-1, yCoord));
									max_size_result_node_ptr->node_y = yCoord;
									if(prefer_left_s2)
										max_size_result_node_ptr->node_x = xCoord - can_move_to(xCoord-1, yCoord);
									else	// prefer_right_s2
										max_size_result_node_ptr->node_x = xCoord - !(nodeType&0x8);

									err_when(max_size_result_node_ptr->node_x!=xCoord && max_size_result_node_ptr->node_x!=xCoord-1);
									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);
									break;

						case 3: // 2 points, 2 paths
									max_size_result_node_ptr->node_y = yCoord+1;
									//----------- process path selection ---------------//
									if(parent_result_node_ptr->node_x<=xCoord)
										prefer_left_s2 = 1;
									else
									{
										if(!can_move_to(xCoord+2,yCoord) || !can_move_to(xCoord+2,yCoord+1) ||
											!can_move_to(xCoord+2,yCoord+2))
											prefer_left_s2 = 1;
										else
											prefer_right_s2 = 1;
									}
									max_size_result_node_ptr->node_x = xCoord+prefer_right_s2;

									err_when(max_size_result_node_ptr->node_x!=xCoord && max_size_result_node_ptr->node_x!=xCoord+1);
									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);

									max_size_result_node_ptr->node_x = xCoord+prefer_right_s2;
									max_size_result_node_ptr->node_y = yCoord;
									err_when(max_size_result_node_ptr->node_x!=xCoord && max_size_result_node_ptr->node_x!=xCoord+1);
									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);
									break;

						case 4: // 2 points, 2 paths
									max_size_result_node_ptr->node_x = xCoord+1;
									max_size_result_node_ptr->node_y = yCoord+1;
									//----------- process path selection ---------------//
									if( (parent_result_node_ptr->node_x-max_size_result_node_ptr->node_x==0) ||
										 (parent_result_node_ptr->node_y-max_size_result_node_ptr->node_y==0) )
										prefer_right_s2 = 1;
									else
										prefer_left_s2 = 1;

									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);

									err_when(!(nodeType&0x4) && !can_move_to(xCoord+2, yCoord));
									max_size_result_node_ptr->node_y = yCoord;
									if(prefer_left_s2)
										max_size_result_node_ptr->node_x = xCoord + !(nodeType&0x4);
									else	// prefer_right_s2
										max_size_result_node_ptr->node_x = xCoord + can_move_to(xCoord+2, yCoord);

									err_when(max_size_result_node_ptr->node_x!=xCoord && max_size_result_node_ptr->node_x!=xCoord+1);
									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);
									break;

						case 5:	// 2 possible paths
									if(parent_result_node_ptr->node_y>=yCoord)
										prefer_lower_s2 = 1;
									else
									{
										if(can_move_to(xCoord+1,yCoord-1))
											prefer_upper_s2 = 1;
										else
											prefer_lower_s2 = 1;
									}
									max_size_result_node_ptr->node_x = xCoord+1;
									max_size_result_node_ptr->node_y = yCoord-prefer_upper_s2;
									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);
									break;

						case 6:	// 2 possible paths
									if(parent_result_node_ptr->node_y<yCoord)
										prefer_upper_s2 = 1;
									else
									{
										if(!can_move_to(xCoord+1,yCoord+1) || !can_move_to(xCoord+2,yCoord+1))
											prefer_upper_s2 = 1;
										else
											prefer_lower_s2 = 1;
									}
									max_size_result_node_ptr->node_x = xCoord+1;
									max_size_result_node_ptr->node_y = yCoord-prefer_upper_s2;
									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);
									break;

						case 8:	// 2 possible paths
									if(parent_result_node_ptr->node_y<yCoord)
										prefer_upper_s2 = 1;
									else
									{
										if(!can_move_to(xCoord-1,yCoord+1) || !can_move_to(xCoord,yCoord+1))
											prefer_lower_s2 = 1;
										else
											prefer_upper_s2 = 1;
									}
									max_size_result_node_ptr->node_x = xCoord-1;
									max_size_result_node_ptr->node_y = yCoord-prefer_upper_s2;
									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);
									break;

						default:	err_here();
									break;
					}
					break;

		case 8:	switch(exitDirection)
					{
						case 1:
									max_size_result_node_ptr->node_x = xCoord-1;
									max_size_result_node_ptr->node_y = yCoord;
									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);
									break;

						case 2: // 2 points, 2 possible paths
									if(parent_result_node_ptr->node_x>=xCoord-1)
										prefer_right_s2 = 1;
									else
									{
										if(!can_move_to(xCoord-2,yCoord) || !can_move_to(xCoord-2,yCoord+1) ||
											!can_move_to(xCoord-2,yCoord+2))
											prefer_right_s2 = 1;
										else
											prefer_left_s2 = 1;
									}
									max_size_result_node_ptr->node_x = xCoord-1-prefer_left_s2;
									max_size_result_node_ptr->node_y = yCoord+1;
									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);

									max_size_result_node_ptr->node_x = xCoord-1-prefer_left_s2;
									max_size_result_node_ptr->node_y = yCoord;
									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);
									break;

						case 3: // 2 points, 2 paths
									max_size_result_node_ptr->node_x = xCoord;
									max_size_result_node_ptr->node_y = yCoord+1;
									//----------- process path selection ---------------//
									if( (parent_result_node_ptr->node_x-max_size_result_node_ptr->node_x==0) ||
										 (parent_result_node_ptr->node_y-max_size_result_node_ptr->node_y==0) )
										prefer_right_s2 = 1;
									else
										prefer_left_s2 = 1;

									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);

									err_when(!(nodeType&0x2) && !can_move_to(xCoord-1, yCoord+1));
									max_size_result_node_ptr->node_y = yCoord;
									if(prefer_left_s2)
										max_size_result_node_ptr->node_x = xCoord - can_move_to(xCoord-1, yCoord+1);
									else	// prefer_right_s2
										max_size_result_node_ptr->node_x = xCoord - !(nodeType&0x2);

									err_when(max_size_result_node_ptr->node_x!=xCoord && max_size_result_node_ptr->node_x!=xCoord-1);
									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);
									break;

						case 4: // 2 points
									max_size_result_node_ptr->node_x = xCoord+1;
									max_size_result_node_ptr->node_y = yCoord+1;
									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);

									err_when(nodeType!=15);
									max_size_result_node_ptr->node_x = xCoord;
									max_size_result_node_ptr->node_y = yCoord;
									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);
									break;

						case 5: // 2 points, 2 paths
									max_size_result_node_ptr->node_x = xCoord+1;
									max_size_result_node_ptr->node_y = yCoord;
									//----------- process path selection ---------------//
									if( (parent_result_node_ptr->node_x-max_size_result_node_ptr->node_x==0) ||
										 (parent_result_node_ptr->node_y-max_size_result_node_ptr->node_y==0) )
										prefer_lower_s2 = 1;
									else
										prefer_upper_s2 = 1;

									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);

									err_when(!(nodeType&0x4) && !can_move_to(xCoord+1, yCoord-1));
									max_size_result_node_ptr->node_x = xCoord;
									if(prefer_upper_s2)
										max_size_result_node_ptr->node_y = yCoord - can_move_to(xCoord+1, yCoord-1);
									else	// prefer_lower_s2
										max_size_result_node_ptr->node_y = yCoord - !(nodeType&0x4);

									err_when(max_size_result_node_ptr->node_y!=yCoord && max_size_result_node_ptr->node_y!=yCoord-1);
									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);
									break;

						case 6: // 2 points, 2 possible paths
									if(parent_result_node_ptr->node_y>=yCoord-1)
										prefer_lower_s2 = 1;
									else
									{
										if(!can_move_to(xCoord,yCoord-2) || !can_move_to(xCoord+1,yCoord-2) ||
											!can_move_to(xCoord+2,yCoord-2))
											prefer_lower_s2 = 1;
										else
											prefer_upper_s2 = 1;
									}
									max_size_result_node_ptr->node_x = xCoord+1;
									max_size_result_node_ptr->node_y = yCoord-1-prefer_upper_s2;
									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);

									max_size_result_node_ptr->node_x = xCoord;
									max_size_result_node_ptr->node_y = yCoord-1-prefer_upper_s2;
									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);
									break;

						case 7:
									max_size_result_node_ptr->node_x = xCoord;
									max_size_result_node_ptr->node_y = yCoord-1;
									add_result_node(&max_size_result_node_ptr, &parent_result_node_ptr, count);
									break;

						default:	err_here();
									break;
					}
					break;

		default:	err_here();
					break;
	}
}
//------ End of function SeekPathS2::get_real_result_node ---------//


//-------- Begin of function SeekPathS2::return_best_node -------//
//
// return : <NodeS2*> the best node.
//						  NULL if no more node on the open node list. There is no
//						  possible path between the starting and destination points.
//
NodeS2* SeekPathS2::return_best_node()
{
	NodeS2 *tempNode;

	if( !open_node_list )
		return NULL;

	//------------------------------------------------------------------//
	// Pick Node with lowest f, in this case it's the first node in list
	// Because we sort the open_node_list list wrt lowest f. Call it BESTNode.
	//------------------------------------------------------------------//

	tempNode=open_node_list;   				 // point to first node on open_node_list
	open_node_list=tempNode->next_node;     // Make open_node_list point to nextnode or NULL.

	//------------------------------------------------------------------//
	// Next take BESTNode (or temp in this case) and put it on closed_node_list
	//------------------------------------------------------------------//

	tempNode->next_node=closed_node_list;
	closed_node_list=tempNode;

	return tempNode;
}
//-------- End of function SeekPathS2::return_best_node ---------//


//----- Begin of function SeekPathS2::return_closest_node -------//
//
// Return the node that is closest to the destination.
//
NodeS2* SeekPathS2::return_closest_node()
{
	NodeS2*  nodePtr;
	NodeS2*  shortestNode=NULL;
	int		nodeDistance, shortestDistance=0x7FFF;
	int		disX, disY;

	//------------ search open_node_list ----------//

	nodePtr = open_node_list;

	int yieldCount = 0;

	while(nodePtr)
	{
		//--- the distance between this node and the destination node ----//

		nodeDistance = (disX=cur_dest_x_s2-nodePtr->node_x)*disX + (disY=cur_dest_y_s2-nodePtr->node_y)*disY;

		//--- if the distance is shorter than the current shortest distance ---//

		if( nodeDistance < shortestDistance )
		{
			shortestDistance = nodeDistance;
			shortestNode 	  = nodePtr;
		}

		nodePtr=nodePtr->next_node;

		yieldCount++;
		if(yieldCount%25==0)
			sys_yield();
	}

	//------------ search closed_node_list ----------//

	nodePtr = closed_node_list;

	yieldCount = 0;
	while(nodePtr)
	{
		//--- the distance between this node and the destination node ----//

		nodeDistance = (disX=cur_dest_x_s2-nodePtr->node_x)*disX + (disY=cur_dest_y_s2-nodePtr->node_y)*disY;

		//--- if the distance is shorter than the current shortest distance ---//

		if( nodeDistance < shortestDistance )
		{
			shortestDistance = nodeDistance;
			shortestNode 	  = nodePtr;
		}

		nodePtr=nodePtr->next_node;

		yieldCount++;
		if(yieldCount%25==0)
			sys_yield();
	}

	//------------------------------------------------------//
	// there may be some nodes with the same shortest Distance
	// from the destination node. However, one should be closer
	// to the starting node.  The following is used to find
	// out the closer node.
	//------------------------------------------------------//
	nodePtr = shortestNode->parent_node;

	yieldCount = 0;
	while(nodePtr)
	{
		//--- the distance between the parent node and the destination node ----//

		nodeDistance = (disX=cur_dest_x_s2-nodePtr->node_x)*disX + (disY=cur_dest_y_s2-nodePtr->node_y)*disY;

		//--- if the distance is shorter than the current shortest distance ---//

		if( nodeDistance <= shortestDistance )
		{
			shortestDistance = nodeDistance;
			shortestNode 	  = nodePtr;
		}
		else
			break;

		nodePtr=nodePtr->parent_node;

		yieldCount++;
		if(yieldCount%25==0)
			sys_yield();
	}

	return shortestNode;
}
//----- End of function SeekPathS2::return_closest_node -------//


//----- Begin of function SeekPathS2::insert_open_node -------//

void SeekPathS2::insert_open_node(NodeS2 *succNode)
{
	if (open_node_list == NULL)
	{
		open_node_list=succNode;
		return;
	}

	//---- insert into open_node_list successor wrt f ----//

	register int f=succNode->node_f;

	if( open_node_list && open_node_list->node_f < f )
	{
		NodeS2 *tempNode1, *tempNode2;

		tempNode1=open_node_list;
		tempNode2=tempNode1->next_node;

		int yieldCount = 0;

		while( tempNode2 && tempNode2->node_f < f )
		{
			tempNode1=tempNode2;
			tempNode2=tempNode2->next_node;

			yieldCount++;
			if(yieldCount%200==0)
				sys_yield();
		}

		err_when(succNode!=NULL && tempNode2!=NULL && succNode->node_x==tempNode2->node_x &&
					succNode->node_y==tempNode2->node_y); // avoid pointer pointing back to itself
		succNode->next_node  = tempNode2;
		tempNode1->next_node = succNode;
	}
	else
	{
		succNode->next_node = open_node_list;
		open_node_list      = succNode;
	}
}
//----- End of function SeekPathS2::insert_open_node -------//


//-------- Begin of function SeekPath::smooth_the_path ---------//
ResultNode* SeekPathS2::smooth_the_path(ResultNode* nodeArray, int& nodeCount)
{
	//--------------------------------------------------//
	// to remove duplicate or useless node
	//--------------------------------------------------//
	int i, j, curNodeCount = 1;
	parent_result_node_ptr = nodeArray;
	max_size_result_node_ptr = nodeArray+1;
	
	for(i=0; i<nodeCount-1; i++)
	{
		if(parent_result_node_ptr->node_x!=max_size_result_node_ptr->node_x ||
			parent_result_node_ptr->node_y!=max_size_result_node_ptr->node_y)
		{
			parent_result_node_ptr++;
			parent_result_node_ptr->node_x = max_size_result_node_ptr->node_x;
			parent_result_node_ptr->node_y = max_size_result_node_ptr->node_y;
			curNodeCount++;
		}
		max_size_result_node_ptr++;

		if(i%50==0)
			sys_yield();
	}
	nodeCount = curNodeCount;
	//return nodeArray;
	
	//---------- declare variables ---------------//
	int checkedNodeCount;
	short vectorX, vectorY, newVectorX, newVectorY, newVectorX2, newVectorY2;
	short changed, caseNum, processed;
	ResultNode *curUsefulNodePtr = nodeArray+1;
	ResultNode *ptr1;
	ResultNode *ptr2;
	ResultNode *ptr3;
	ResultNode *ptr4;
	
	//--------------------------------------------------//
	// smooth the path
	//--------------------------------------------------//
	parent_result_node_ptr = nodeArray;
	max_size_result_node_ptr = nodeArray+1;
	changed = 1;
	checkedNodeCount = 1;
	curNodeCount = nodeCount;
	ptr1 = parent_result_node_ptr;
	ptr2 = max_size_result_node_ptr;
	ptr3 = max_size_result_node_ptr+1;

	int yieldCount = 1;

	while(changed && curNodeCount>=3)
	{
		yieldCount++;
		if(yieldCount%20==0)
			sys_yield();

		vectorX = ptr2->node_x - ptr1->node_x;
		vectorY = ptr2->node_y - ptr1->node_y;
		changed = 0;
		
		for(j=1; j<curNodeCount-1; j++)
		{
			processed = 0;
			newVectorX= ptr3->node_x - ptr2->node_x;
			newVectorY= ptr3->node_y - ptr2->node_y;
		
			//----------------------------------------------------------//
			// reverse direction
			//----------------------------------------------------------//
			if(vectorX==-newVectorX && vectorY==-newVectorY)
			{
				if(j+2<curNodeCount)	//------ not the end of the array ------//
				{
					ptr2 = ptr3+1;
					ptr3 = ptr2+1;
					vectorX = ptr2->node_x - ptr1->node_x;
					vectorY = ptr2->node_y - ptr1->node_y;
					processed = 1;
					j++;
					continue;
				}
				else	//-------- already the end of the array ----------//
				{
					ptr3++;
					changed++;
					break;	// break the for loop
				}
			}

			//----------------------------------------------------------//
			// turning 45 degree angle
			//----------------------------------------------------------//
			if(abs(vectorX+newVectorX)<=1 && abs(vectorY+newVectorY)<=1 &&	// if so, turning 45 or 90 degree
				vectorX*newVectorX+vectorY*newVectorY!=0)							// reject turning 90 degree
			{
				ptr2=ptr3;
				ptr3++;
				vectorX = ptr2->node_x - ptr1->node_x;
				vectorY = ptr2->node_y - ptr1->node_y;
				processed = 1;
				continue;
			}

			//----------------------------------------------------------//
			// turning at 90 degree clockwise / anti-clockwise
			//----------------------------------------------------------//
			if((vectorX==0 && vectorY!=0 && newVectorX!=0 && newVectorY==0) ||	// + case
				(vectorX!=0 && vectorY==0 && newVectorX==0 && newVectorY!=0))
			{
				ptr2=ptr3;
				ptr3++;
				vectorX = ptr2->node_x - ptr1->node_x;
				vectorY = ptr2->node_y - ptr1->node_y;
				processed = 1;
				continue;
			}

			if((vectorX!=0 && vectorY!=0 && newVectorX!=0 && newVectorY!=0) &&		//	x case
				((vectorX==newVectorX && vectorY==-newVectorY) || (vectorX==-newVectorX && vectorY==newVectorY)))
			{
				ptr2->node_x = (ptr1->node_x+ptr3->node_x)/2;
				ptr2->node_y = (ptr1->node_y+ptr3->node_y)/2;
				curUsefulNodePtr->node_x = ptr2->node_x;
				curUsefulNodePtr->node_y = ptr2->node_y;
				curUsefulNodePtr++;
				checkedNodeCount++;
				ptr1 = ptr2;
				ptr2 = ptr3;
				ptr3++;
				vectorX = ptr2->node_x - ptr1->node_x;
				vectorY = ptr2->node_y - ptr1->node_y;
				processed = 1;
				continue;
			}

			//----------------------------------------------------------//
			//	check for the 4-point case
			//----------------------------------------------------------//
			if(j<curNodeCount-2)
			{
				ptr4 = ptr3+1;
				newVectorX2 = ptr4->node_x - ptr3->node_x;
				newVectorY2	= ptr4->node_y - ptr3->node_y;
				caseNum = 0;
				if(!caseNum && vectorX==-1 && vectorY==0 && newVectorX==-1 && newVectorX2==0)
				{
					if(newVectorY==1 && newVectorY2==1 && can_move_to(ptr1->node_x, ptr4->node_y))
						caseNum = 1;

					if(!caseNum && newVectorY==-1 && newVectorY2==-1 && can_move_to(ptr1->node_x, ptr4->node_y-1))
						caseNum = 5;
				}

				if(!caseNum && vectorX==1 && vectorY==0 && newVectorX==1 && newVectorX2==0)
				{	
					if(newVectorY==1 && newVectorY2==1 && can_move_to(ptr1->node_x+1, ptr4->node_y))
						caseNum = 3;

					if(!caseNum && newVectorY==-1 && newVectorY2==-1 && can_move_to(ptr1->node_x+1, ptr4->node_y-1))
						caseNum = 7;
				}

				if(!caseNum && vectorX==0 && vectorY==-1 && newVectorY==-1 && newVectorY2==0)
				{
					if(newVectorX==1 && newVectorX2==1 && can_move_to(ptr4->node_x, ptr1->node_y))
						caseNum = 2;

					if(!caseNum && newVectorX==-1 && newVectorX2==-1 && can_move_to(ptr4->node_x+1, ptr1->node_y))
						caseNum = 4;
				}

				if(!caseNum && vectorX==0 && vectorY==1 && newVectorY==1 && newVectorY2==0)
				{
					if(newVectorX==1 && newVectorX2==1 && can_move_to(ptr4->node_x, ptr1->node_y+1))
						caseNum = 6;

					if(!caseNum && newVectorX==-1 && newVectorX2==-1 && can_move_to(ptr4->node_x+1, ptr1->node_y+1))
						caseNum = 8;
				}

				if(caseNum)
				{
					switch(caseNum)
					{
						case 1:	
									if(can_move_to(ptr1->node_x, ptr4->node_y))
									{
										//ptr2->node_x = ptr2->node_x;
										ptr2->node_y++;	//ptr2->node_y = ptr3->node_y;
										processed = 1;
									}
									break;

						case 2:
									if(can_move_to(ptr4->node_x, ptr1->node_y))
									{
										ptr2->node_x++;	//ptr2->node_x = ptr3->node_x;
										//ptr2->node_y = ptr2->node_y;
										processed = 1;
									}
									break;

						case 3:
									if(can_move_to(ptr2->node_x, ptr4->node_y))
									{
										//ptr2->node_x = ptr2->node_x;
										ptr2->node_y++;	//ptr2->node_y = ptr3->node_y;
										processed = 1;
									}
									break;

						case 4:
									if(can_move_to(ptr3->node_x, ptr1->node_y))
									{
										ptr2->node_x--;	//ptr2->node_x = ptr3->node_x;
										//ptr2->node_y = ptr2->node_y;
										processed = 1;
									}
									break;

						case 5:
									if(can_move_to(ptr1->node_x, ptr3->node_y))
									{
										//ptr2->node_x = ptr2->node_x;
										ptr2->node_y--;	//ptr2->node_y = ptr3->node_y;
										processed = 1;
									}
									break;

						case 6:
									if(can_move_to(ptr4->node_x, ptr2->node_y))
									{
										ptr2->node_x++;	//ptr2->node_x = ptr3->node_x;
										//ptr2->node_y = ptr2->node_y;
										processed = 1;
									}
									break;

						case 7:
									if(can_move_to(ptr2->node_x, ptr3->node_y))
									{
										//ptr2->node_x = ptr2->node_x;
										ptr2->node_y--;	//ptr2->node_y = ptr3->node_y;
										processed = 1;
									}
									break;

						case 8:
									if(can_move_to(ptr3->node_x, ptr2->node_y))
									{
										ptr2->node_x--;	//ptr2->node_x = ptr3->node_x;
										//ptr2->node_y = ptr2->node_y;
										processed = 1;
									}
									break;

						default:
									err_here();
									break;
					}

					if(processed)
					{
						ptr3 = ptr4;
						curUsefulNodePtr->node_x = ptr2->node_x;
						curUsefulNodePtr->node_y = ptr2->node_y;
						curUsefulNodePtr++;
						checkedNodeCount++;
						ptr1 = ptr2;
						ptr2 = ptr3;
						ptr3++;
						j++;
						vectorX = ptr2->node_x - ptr1->node_x;
						vectorY = ptr2->node_y - ptr1->node_y;
						continue;
					}
				}
			}

			//------ none of the above case ---------//
			if(!processed)
			{
				curUsefulNodePtr->node_x = ptr2->node_x;
				curUsefulNodePtr->node_y = ptr2->node_y;
				curUsefulNodePtr++;
				checkedNodeCount++;
			}
			else 
				changed = 1;

			ptr1 = ptr2;
			ptr2 = ptr3;
			ptr3++;
			vectorX = ptr2->node_x - ptr1->node_x;
			vectorY = ptr2->node_y - ptr1->node_y;
		}

		//---- end checking and then reset parameters----//
		curUsefulNodePtr->node_x = ptr2->node_x;
		curUsefulNodePtr->node_y = ptr2->node_y;
		curUsefulNodePtr++;
		checkedNodeCount++;
		
		curNodeCount = checkedNodeCount;
		checkedNodeCount = 1;
		ptr1 = parent_result_node_ptr;
		ptr2 = ptr1+1;
		ptr3 = ptr2+1;
	}
	nodeCount = curNodeCount;

	return nodeArray;
}	
//------- End of function SeekPathS2::smooth_the_path -------//


//-------- Begin of function NodeS2::generate_successors ---------//
// Note: In fact, the cost of the starting node should be 0 or 1
//			and the parentEnterDirection is 0.	Now the cost in this
//			case is set to 2.	The difference can be ignored as it will
//			not affect the search after generating the second level
//			children.
//			The advantage to ignore this case is that less comparsion
//			effort in checking parentEnterDirection.
//
short NodeS2::generate_successors(short parentEnterDirection)
{
	int hasLeft  = node_x > cur_border_x1_s2;
	int hasRight = node_x < cur_border_x2_s2;
	int hasUp	 = node_y > cur_border_y1_s2;
	int hasDown  = node_y < cur_border_y2_s2;
	int upperLeftX,upperLeftY;
	short cost;

	upperLeftX = node_x<<1;
	upperLeftY = node_y<<1;
	err_when(upperLeftX != node_x+node_x || upperLeftY != node_y+node_y);

	//-------------------------------------------
	// enter_direction = (exit_direction+3)%8+1
	//-------------------------------------------
	if( hasLeft )
	{
		//--------- Left, exit_direction=1 --------//
		if(can_move_to(upperLeftX-1, upperLeftY) && node_type&0x1 &&
			can_move_to(upperLeftX-1, upperLeftY+1) && node_type&0x4 )
		{
			if(parentEnterDirection==2 || parentEnterDirection==3 || parentEnterDirection==7 ||	parentEnterDirection==8)
				cost = 1;
			else
			{	
				if((parentEnterDirection==4 && (node_type&0x2 || can_move_to(upperLeftX,upperLeftY+2))) ||
					parentEnterDirection==5 ||
					(parentEnterDirection==6 && (node_type&0x8 || can_move_to(upperLeftX,upperLeftY-1))))
					cost = 2;
				else
				{
					if(parentEnterDirection==0 &&	!source_blocked_exit_direction[0]) // direction 1
					{	
						if(source_locate_type==1 || source_locate_type==3)
							cost = 1;
						else
							cost = 2;
					}
					else
						cost = 0;
				}
			}

			if(cost)
			{
				err_when(parentEnterDirection==1);
				if(generate_succ(node_x-1,node_y,5,cost))
					return 1;
			}
		}
		
		if( hasUp )
		{
			//------- Upper-Left, exit_direction=8 ---------//
			if(can_move_to(upperLeftX-1, upperLeftY-1) && can_move_to(upperLeftX, upperLeftY-1) &&
				can_move_to(upperLeftX-1, upperLeftY) && node_type&0x1)
			{
				if(parentEnterDirection==1 || parentEnterDirection==2 || parentEnterDirection==6 || parentEnterDirection==7)
					cost = 1;
				else
				{	
					if((parentEnterDirection==3 && (node_type&0x2 || can_move_to(upperLeftX-1, upperLeftY+1))) ||
						(parentEnterDirection==4 && node_type==15) ||
						(parentEnterDirection==5 && (node_type&0x4 || can_move_to(upperLeftX+1, upperLeftY-1))))
						cost = 2;
					else
					{
						if(parentEnterDirection==0 && !source_blocked_exit_direction[7])	// direction 8
						{
							if(source_locate_type==1)
								cost = 1;
							else
								cost = 2;
						}
						else
							cost = 0;
					}
				}

				if(cost)
				{
					err_when(parentEnterDirection==8);
					if(generate_succ(node_x-1,node_y-1,4,cost))
						return 1;
				}
			}
		}

		if( hasDown )
		{
			//--------- Lower-Left, exit_direction=2 ----------//
			if(can_move_to(upperLeftX-1,upperLeftY+1) && node_type&0x4 &&
				can_move_to(upperLeftX-1,upperLeftY+2) && can_move_to(upperLeftX,upperLeftY+2))
			{
				if(parentEnterDirection==1 || parentEnterDirection==3 || parentEnterDirection==4 || parentEnterDirection==8)
					cost = 1;
				else
				{
					if((parentEnterDirection==5 && (node_type&0x1 || can_move_to(upperLeftX+1,upperLeftY+2))) ||
						(parentEnterDirection==6 && node_type==15) || 
						(parentEnterDirection==7 && (node_type&0x8 || can_move_to(upperLeftX-1,upperLeftY))))
						cost = 2;
					else
					{
						if(parentEnterDirection==0 && !source_blocked_exit_direction[1])	// direction 2
						{
							if(source_locate_type==1||source_locate_type==3)
								cost = 1;
							else
								cost = 2;
						}
						else
							cost = 0;
					}
				}

				if(cost || (parentEnterDirection==0 && source_locate_type==3 && !source_blocked_exit_direction[1]))
				{
					err_when(parentEnterDirection==2);
					if(generate_succ(node_x-1,node_y+1,6,cost))
						return 1;
				}
			}
		}
	}

	if( hasRight )
	{
		//----------- Right, exit_direction=5 -----------//
		if(node_type&0x2 && can_move_to(upperLeftX+2,upperLeftY) &&
			node_type&0x8 && can_move_to(upperLeftX+2,upperLeftY+1))
		{
			if(parentEnterDirection==3 || parentEnterDirection==4 || parentEnterDirection==6 ||	parentEnterDirection==7)
				cost = 1;
			else
			{
				if(parentEnterDirection==1 ||
					(parentEnterDirection==2 && (node_type&0x1 || can_move_to(upperLeftX+1,upperLeftY+2))) ||
					(parentEnterDirection==8 && (node_type&0x4 || can_move_to(upperLeftX+1,upperLeftY-1))))
					cost = 2;
				else
				{
					if(parentEnterDirection==0 && !source_blocked_exit_direction[4] && //direction 5
						(source_locate_type==1||source_locate_type==3))
						cost = 1;
					else
						cost = 0;
				}
			}	

			if(cost||(parentEnterDirection==0&&!source_blocked_exit_direction[4]&&(source_locate_type==2||source_locate_type==4)))
			{
				err_when(parentEnterDirection==5);
				if(generate_succ(node_x+1,node_y,1,cost))
					return 1;
			}
		}
		
		if( hasUp )
		{
			//-------- Upper-Right, exit_direction=6 ---------//
			if(can_move_to(upperLeftX+1,upperLeftY-1) && can_move_to(upperLeftX+2,upperLeftY-1) &&
				node_type&0x2 && can_move_to(upperLeftX+2,upperLeftY))
			{
				if(parentEnterDirection==4 || parentEnterDirection==5 || parentEnterDirection==7 ||	parentEnterDirection==8)
					cost = 1;
				else
				{
					if((parentEnterDirection==1 && (node_type&0x8 || can_move_to(upperLeftX,upperLeftY-1))) ||
						(parentEnterDirection==2 && node_type==15) || 
						(parentEnterDirection==3 && (node_type&0x1 || can_move_to(upperLeftX+2,upperLeftY+1))))
						cost = 2;
					else
					{
						if(parentEnterDirection==0 && !source_blocked_exit_direction[5])	// direction 6
						{
							if(source_locate_type==1||source_locate_type==2)
								cost = 1;
							else
								cost = 2;
						}
						else
							cost = 0;
					}
				}

				if(cost)
				{
					err_when(parentEnterDirection==6);
					if(generate_succ(node_x+1,node_y-1,2,cost))
						return 1;
				}
			}
		}

		if( hasDown )
		{
			//--------- Lower-Right, exit_direction=4 ---------//
			if(node_type&0x8 && can_move_to(upperLeftX+2,upperLeftY+1) &&
				can_move_to(upperLeftX+1,upperLeftY+2) && can_move_to(upperLeftX+2,upperLeftY+2))
			{
				if(parentEnterDirection==2 || parentEnterDirection==3 || parentEnterDirection==5 ||	parentEnterDirection==6)
					cost = 1;
				else
				{	
					if((parentEnterDirection==1 && (node_type&0x2 || can_move_to(upperLeftX,upperLeftY+2))) ||
						(parentEnterDirection==8 && node_type==15) || 
						(parentEnterDirection==7 && (node_type&0x4 || can_move_to(upperLeftX+2,upperLeftY))))
						cost = 2;
					else
					{
						if(parentEnterDirection==0&&source_locate_type!=4&&!source_blocked_exit_direction[3])	// direction 4
							cost = 1;
						else
							cost = 0;
					}
				}

				if(cost||(parentEnterDirection==0&&source_locate_type==4&&!source_blocked_exit_direction[3]))
				{
					err_when(parentEnterDirection==4);
					if(generate_succ(node_x+1,node_y+1,8,cost))
						return 1;
				}
			}
		}
	}

	if( hasUp )
	{  
		//---------- Upper, exit_direction=7 -----------//
		if(can_move_to(upperLeftX,upperLeftY-1) && can_move_to(upperLeftX+1,upperLeftY-1) &&
			node_type&0x1 && node_type&0x2)
		{
			if(parentEnterDirection==1 || parentEnterDirection==5 || parentEnterDirection==6 ||	parentEnterDirection==8)
				cost = 1;
			else
			{
				if((parentEnterDirection==2 && (node_type&0x8 || can_move_to(upperLeftX-1,upperLeftY))) ||
					parentEnterDirection==3 ||
					(parentEnterDirection==4 && (node_type&0x4 || can_move_to(upperLeftX+2,upperLeftY))))
					cost = 2;
				else
				{
					if(parentEnterDirection==0 && !source_blocked_exit_direction[6])	// direction 7
					{
						if(source_locate_type==1||source_locate_type==2)
							cost = 1;
						else
							cost = 2;
					}
					else
						cost = 0;
				}
			}

			if(cost)
			{
				err_when(parentEnterDirection==7);
				if(generate_succ(node_x,node_y-1,3,cost))
					return 1;
			}
		}
	}
	
	if( hasDown )
	{	
		//---------- Lower, exit_direction=3 -----------// 
		if(node_type&0x4 && node_type&0x8 &&
			can_move_to(upperLeftX,upperLeftY+2) && can_move_to(upperLeftX+1,upperLeftY+2))
		{
			if(parentEnterDirection==1 || parentEnterDirection==2 || parentEnterDirection==4 || parentEnterDirection==5)
				cost = 1;
			else
			{
				if((parentEnterDirection==6 && (node_type&0x1 || can_move_to(upperLeftX+2,upperLeftY+1))) ||
					parentEnterDirection==7 ||
					(parentEnterDirection==8 && (node_type&0x2 || can_move_to(upperLeftX-1,upperLeftY+1))))
					cost = 2;
				else
				{
					if(parentEnterDirection==0&&(source_locate_type==1||source_locate_type==2)&&!source_blocked_exit_direction[2])	// direction 3
						cost = 1;
					else
						cost = 0;
				}
			}

			if(cost||(parentEnterDirection==0&&(source_locate_type==3||source_locate_type==4)&&!source_blocked_exit_direction[2]))
			{
				err_when(parentEnterDirection==3);
				if(generate_succ(node_x,node_y+1,7,cost))
					return 1;
			}
		}
	}
	return 0;
}
//------- End of function NodeS2::generate_successors -------//


//-------- Begin of function NodeS2::generate_succ ---------//
short NodeS2::generate_succ(short x, short y, short enter_direct,short cost)
{
	int upperLeftX, upperLeftY;
	upperLeftX = x<<1;
	upperLeftY = y<<1;

	//----- if it points back to this node's parent ----//

	if( parent_node )
	{
		if( parent_node->node_x==x && parent_node->node_y==y )
			return 0;
	}

	//----- if there is an existing node at the given position ----//

	short c, g = node_g+cost;	    	 // g(Successor)=g(BestNode)+cost of getting from BestNode to Successor
	short nodeRecno;

	//if( (nodeRecno=cur_node_matrix_s2[y*(MAX_WORLD_X_LOC/2+1)+x]) > 0 )
	if( (nodeRecno=cur_node_matrix_s2[y*(MAX_WORLD_X_LOC/2+1)+x]) > 0 &&
		 nodeRecno<max_node_num_s2)
	{
		NodeS2* oldNode = cur_node_array_s2+nodeRecno-1;

		//------ Add oldNode to the list of BestNode's child_noderen (or Successors).

		for(c=0 ; c<MAX_CHILD_NODE && child_node[c] ; c++);

		child_node[c]=oldNode;

		//---- if our new g value is < oldNode's then reset oldNode's parent to point to BestNode

		if (g < oldNode->node_g)
		{
			//-------------check whether the oldNode can process propagate down -----------//
			char canPropagateDown=1;

			if(oldNode->node_type!=15 && enter_direct%2==0)
			{
				NodeS2*	oldChildNode;
				short xShift1 = x_shift_array[enter_direct-1];
				short	yShift1 = y_shift_array[enter_direct-1];
				short xShift2, yShift2, xShift3, yShift3;
				if(enter_direct>1)
				{
					xShift2 = x_shift_array[enter_direct-2];
					yShift2 = y_shift_array[enter_direct-2];
				}
				else
				{
					xShift2 = x_shift_array[7];
					yShift2 = y_shift_array[7];
				}

				if(enter_direct<8)
				{
					xShift3 = x_shift_array[enter_direct];
					yShift3 = y_shift_array[enter_direct];
				}
				else
				{
					xShift3 = x_shift_array[0];
					yShift3 = y_shift_array[0];
				}

				for(c=0; c<MAX_CHILD_NODE && oldNode->child_node[c]; c++) // for each child of oldNode
				{
					oldChildNode = oldNode->child_node[c];
					if(oldNode->parent_node != oldChildNode)
					{	
						switch(enter_direct)	// for entering at corner
						{
							case 2:	
										if(oldChildNode->enter_direction==enter_direct && 
											oldChildNode->node_x-oldNode->node_x==xShift1 &&
											oldChildNode->node_y-oldNode->node_y==yShift1 &&
											(!can_move_to(upperLeftX, upperLeftY) ||
											 !can_move_to(upperLeftX+1, upperLeftY+1)))
											canPropagateDown = 0;

										if(oldChildNode->enter_direction==enter_direct-1 &&
											oldChildNode->node_x-oldNode->node_x==xShift2 &&
											oldChildNode->node_x-oldNode->node_y==yShift2 &&
											!can_move_to(upperLeftX,upperLeftY) &&
											!can_move_to(upperLeftX+1,upperLeftY+2))
											canPropagateDown = 0;

										if(oldChildNode->enter_direction==enter_direct+1 &&
											oldChildNode->node_x-oldNode->node_x==xShift2 &&
											oldChildNode->node_x-oldNode->node_y==yShift2 &&
											!can_move_to(upperLeftX+1,upperLeftY+1) &&
											!can_move_to(upperLeftX-1,upperLeftY))
											canPropagateDown = 0;
										break;

							case 4:
										if(oldChildNode->enter_direction==enter_direct && 
											oldChildNode->node_x-oldNode->node_x==xShift1 &&
											oldChildNode->node_y-oldNode->node_y==yShift1 &&
											(!can_move_to(upperLeftX+1, upperLeftY) ||
											 !can_move_to(upperLeftX, upperLeftY+1)))
											canPropagateDown = 0;

										if(oldChildNode->enter_direction==enter_direct-1 &&
											oldChildNode->node_x-oldNode->node_x==xShift2 &&
											oldChildNode->node_x-oldNode->node_y==yShift2 &&
											!can_move_to(upperLeftX,upperLeftY+1)&&
											!can_move_to(upperLeftX+2,upperLeftY))
											canPropagateDown = 0;

										if(oldChildNode->enter_direction==enter_direct+1 &&
											oldChildNode->node_x-oldNode->node_x==xShift2 &&
											oldChildNode->node_x-oldNode->node_y==yShift2 &&
											!can_move_to(upperLeftX,upperLeftY+2) &&
											!can_move_to(upperLeftX+1,upperLeftY))
											canPropagateDown = 0;
										break;

							case 6:
										if(oldChildNode->enter_direction==enter_direct && 
											oldChildNode->node_x-oldNode->node_x==xShift1 &&
											oldChildNode->node_y-oldNode->node_y==yShift1 &&
											(!can_move_to(upperLeftX, upperLeftY) ||
											 !can_move_to(upperLeftX+1, upperLeftY+1)))
											canPropagateDown = 0;

										if(oldChildNode->enter_direction==enter_direct-1 &&
											oldChildNode->node_x-oldNode->node_x==xShift2 &&
											oldChildNode->node_x-oldNode->node_y==yShift2 &&
											!can_move_to(upperLeftX,upperLeftY-1)&&
											!can_move_to(upperLeftX+1,upperLeftY+1))
											canPropagateDown = 0;

										if(oldChildNode->enter_direction==enter_direct+1 &&
											oldChildNode->node_x-oldNode->node_x==xShift2 &&
											oldChildNode->node_x-oldNode->node_y==yShift2 &&
											!can_move_to(upperLeftX,upperLeftY) &&
											!can_move_to(upperLeftX+2,upperLeftY+1))
											canPropagateDown = 0;
										break;

							case 8:
										if(oldChildNode->enter_direction==enter_direct && 
											oldChildNode->node_x-oldNode->node_x==xShift1 &&
											oldChildNode->node_y-oldNode->node_y==yShift1 &&
											(!can_move_to(upperLeftX+1, upperLeftY) ||
											 !can_move_to(upperLeftX, upperLeftY+1)))
											canPropagateDown = 0;

										if(oldChildNode->enter_direction==enter_direct-1 &&
											oldChildNode->node_x-oldNode->node_x==xShift2 &&
											oldChildNode->node_x-oldNode->node_y==yShift2 &&
											!can_move_to(upperLeftX-1,upperLeftY+1)&&
											!can_move_to(upperLeftX+1,upperLeftY))
											canPropagateDown = 0;

										if(oldChildNode->enter_direction==(enter_direct%8+1) &&
											oldChildNode->node_x-oldNode->node_x==xShift2 &&
											oldChildNode->node_x-oldNode->node_y==yShift2 &&
											!can_move_to(upperLeftX+1,upperLeftY-1) &&
											!can_move_to(upperLeftX,upperLeftY+1))
											canPropagateDown = 0;
										break;

							default:	err_here();
										break;
						}
					}
				}
			}
			
			if(canPropagateDown)
			{
				oldNode->parent_node = this;
				oldNode->node_g 	   = g;
				oldNode->node_f	 	= g+oldNode->node_h;
				oldNode->enter_direction = (char)enter_direct;

				//-------- if it's a closed node ---------//
				if( oldNode->child_node[0] )
				{
					//-------------------------------------------------//
					 // Since we changed the g value of oldNode, we need
					// to propagate this new value downwards, i.e.
					// do a Depth-First traversal of the tree!
					//-------------------------------------------------//
					oldNode->propagate_down();
					sys_yield();
				}
			}
		}
	}
	else //------------ add a new node -----------//
	{
		NodeS2* succNode = cur_node_array_s2 + cur_seek_path_s2->node_count++;

		memset(succNode, 0, sizeof(NodeS2));

		succNode->parent_node = this;
		succNode->node_g = g;
		succNode->node_h = (x-cur_dest_x_s2)*(x-cur_dest_x_s2)+(y-cur_dest_y_s2)*(y-cur_dest_y_s2); // should do sqrt(), but since we don't really
		succNode->node_f = g+succNode->node_h;     		// care about the distance but just which branch looks
		succNode->node_x = x;                  			// better this should suffice. Anyayz it's faster.
		succNode->node_y = y;
		succNode->enter_direction = (char)enter_direct;
		succNode->node_type =can_move_to(upperLeftX,upperLeftY)+(can_move_to(upperLeftX+1,upperLeftY)<<1)+
									(can_move_to(upperLeftX,upperLeftY+1)<<2)+(can_move_to(upperLeftX+1,upperLeftY+1)<<3);

		/*if(search_mode_s2==4 && nodeRecno>max_node_num_s2)
		{
			reuse_connect_node_exit_direction = nodeRecno - max_node_num_s2;
			reuse_result_node_ptr_s2 = succNode;
			return 1;
		}*/
		
		if(search_mode_s2==SEARCH_MODE_REUSE && nodeRecno>max_node_num_s2)	// path-reuse node found, but checking of can_walk(final_dest_?) is requested
		{
			int destIndex = nodeRecno - max_node_num_s2;
			int addNode = 1;
			switch(destIndex)
			{
				case 1:
							final_dest_x_s2 = x<<1;
							final_dest_y_s2 = y<<1;
							break;

				case 2:
							if(enter_direct==2 && (!succNode->node_type&0x1 || !can_move_to((x<<1)+1,(y<<1)+2) ) )
								addNode = 0;

							if(addNode && enter_direct==8 && (!succNode->node_type&0x4 || !can_move_to((x<<1)+1,(y<<1)-1) ) )
								addNode = 0;

							if(addNode)	// not the two blocked cases
							{
								final_dest_x_s2 = (x<<1) + 1;
								final_dest_y_s2 = y<<1;
							}
							break;

				case 3:
							if(enter_direct==6 && (!succNode->node_type&0x1 || !can_move_to((x<<1)+2,(y<<1)+1) ) )
								addNode = 0;

							if(addNode && enter_direct==8 && (!succNode->node_type&0x2 || !can_move_to((x<<1)-1,(y<<1)+1) ) )
								addNode = 0;

							if(addNode)	// not the two blocked cases
							{
								final_dest_x_s2 = x<<1;
								final_dest_y_s2 = (y<<1)+1;
							}
							break;

				case 4:
							if(enter_direct==1 && (!succNode->node_type&0x2 || !can_move_to(x<<1,(y<<1)+2) ) )
								addNode = 0;

							if(addNode && enter_direct==7 && (!succNode->node_type&0x4 || !can_move_to((x<<1)+2,y<<1) ) )
								addNode = 0;

							if(addNode && enter_direct==8 && (!succNode->node_type&0x2 || !succNode->node_type&0x4) )
								addNode = 0;

							if(addNode)	// not the two blocked cases
							{
								final_dest_x_s2 = (x<<1)+1;
								final_dest_y_s2 = (y<<1)+1;
							}
							break;

				default: err_here();
							break;
			}

			if(addNode)
			{
				reuse_result_node_ptr_s2 = succNode;
				err_when(!can_move_to_s2(final_dest_x_s2, final_dest_y_s2));
				return 1;	// PATH_REUSE_FOUND
			}
			else	// add nothing
			{
				memset(succNode, 0, sizeof(NodeS2));
				cur_seek_path_s2->node_count--;
				return 0;
			}
		}
				
		cur_node_matrix_s2[y*(MAX_WORLD_X_LOC/2+1)+x] = cur_seek_path_s2->node_count;
		cur_seek_path_s2->insert_open_node(succNode);     // insert succNode on open_node_list list wrt f

		for(c=0 ; c<MAX_CHILD_NODE && child_node[c] ; c++);   // Add oldNode to the list of BestNode's child_noderen (or succNodes).

		child_node[c]=succNode;
	}
	return 0;
}
//------- End of function NodeS2::generate_succ -------//


//-------- Begin of function NodeS2::propagate_down ---------//
void NodeS2::propagate_down()
{
	NodeS2	*childNode, *fatherNode;
	int 		c, g=node_g;      // alias.
	short		cost;
	char		xShift, yShift;	// the x, y difference between parent and child nodes

	char		childEnterDirection;
	char		exitDirection;
	char		testResult;
	
	for(c=0;c<8;c++)
	{
		if ((childNode=child_node[c])==NULL)   // create alias for faster access.
			break;

		if(childNode->parent_node==this)
			break;

		cost = 2; // in fact, may be 1 or 2
		if (g+cost <= childNode->node_g) // first checking
		{
			char canMove = 0;
			int childUpperLeftX = childNode->node_x<<1;
			int childUpperLeftY = childNode->node_y<<1;

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

			//---------- decide can move or not and use special case or not --------//
			switch(childEnterDirection)
			{
				case 1:
							err_when(node_x!=childNode->node_x-1 || node_y!=childNode->node_y);
							if(can_move_to(childUpperLeftX-1,childUpperLeftY) && childNode->node_type&0x1 &&
								can_move_to(childUpperLeftX-1,childUpperLeftY+1) && childNode->node_type&0x4)	//if the entering location is walkable
							{
								switch(enter_direction)
								{
									case 5:	canMove = 0;	//reverse direction
												break;
								
									case 1:case 3:case 4:case 6:case 7:	// obviously
												canMove = 1;
												break;

									case 2:	if(node_type&0x1 || can_move_to(childUpperLeftX-1, childUpperLeftY+2))
													canMove = 1;
												break;

									case 8:	if(node_type&0x4 || can_move_to(childUpperLeftX-1, childUpperLeftY-1))
													canMove = 1;
												break;

									default:	err_here();
												break;
								}
							}
							break;

				case 2:
							err_when(node_x!=childNode->node_x-1 || node_y!=childNode->node_y+1);
							if(can_move_to(childUpperLeftX-1,childUpperLeftY+1) && childNode->node_type&0x4 &&
								can_move_to(childUpperLeftX-1,childUpperLeftY+2) && can_move_to(childUpperLeftX,childUpperLeftY+2))
							{
								switch(enter_direction)
								{
									case 6:	canMove = 0;	//reverse direction
												break;

									case 4:case 5:case 7:case 8:
												canMove = 1;
												break;

									case 1:	if(node_type&0x8 || can_move_to(childUpperLeftX-2, childUpperLeftY+1))
													canMove = 1;
												break;

									case 2:	if(node_type==15)
													canMove = 1;
												else
													canMove = 0;
												break;

									case 3:	if(node_type&0x1 || can_move_to(childUpperLeftX, childUpperLeftY+3))
													canMove = 1;
												break;

									default:	err_here();
												break;
								}
							}
							break;

				case 3:
							err_when(node_x!=childNode->node_x || node_y!=childNode->node_y+1);
							if(childNode->node_type&0x4 && childNode->node_type&0x8 &&
								can_move_to(childUpperLeftX,childUpperLeftY+2) && can_move_to(childUpperLeftX+1,childUpperLeftY+2))
							{
								switch(enter_direction)
								{
									case 7:	canMove = 0;	//reverse direction
												break;

									case 1:case 3:case 5:case 6:case 8:
												canMove = 1;
												break;

									case 2:	if(node_type&0x8 || can_move_to(childUpperLeftX-1, childUpperLeftY+2))
													canMove = 1;
												break;

									case 4:	if(node_type&0x4 || can_move_to(childUpperLeftX+2, childUpperLeftY+2))
													canMove = 1;
												break;

									default:	err_here();
												break;
								}
							}
							break;

				case 4:
							err_when(node_x!=childNode->node_x+1 || node_y!=childNode->node_y+1);
							if(childNode->node_type&0x8 && can_move_to(childUpperLeftX+2,childUpperLeftY+1) &&
								can_move_to(childUpperLeftX+1,childUpperLeftY+2) && can_move_to(childUpperLeftX+2,childUpperLeftY+2))
							{
								switch(enter_direction)
								{
									case 8:	canMove = 0;	//reverse direction
												break;

									case 1:case 2:case 6:case 7:
												canMove = 1;
												break;

									case 3:	if(node_type&0x2 || can_move_to(childUpperLeftX+1, childUpperLeftY+3))
													canMove = 1;
												break;

									case 4:	if(node_type==15)
													canMove = 1;
												else
													canMove = 0;
												break;

									case 5:	if(node_type&0x4 || can_move_to(childUpperLeftX+3, childUpperLeftY+1))
													canMove = 1;
												break;

									default:	err_here();
												break;

								}
							}
							break;

				case 5:
							err_when(node_x!=childNode->node_x+1 || node_y!=childNode->node_y);
							if(childNode->node_type&0x2 && can_move_to(childUpperLeftX+2,childUpperLeftY) &&
								childNode->node_type&0x8 && can_move_to(childUpperLeftX+2,childUpperLeftY+1))
							{
								switch(enter_direction)
								{
									case 1:	canMove = 0;	//reverse direction
												break;

									case 2:case 3:case 5:case 7:case 8:
												canMove = 1;
												break;

									case 4:	if(node_type&0x2 || can_move_to(childUpperLeftX+2, childUpperLeftY+2))
													canMove = 1;
												break;

									case 6:	if(node_type&0x8 || can_move_to(childUpperLeftX+2, childUpperLeftY-1))
													canMove = 1;
												break;

									default:	err_here();
												break;
								}
							}
							break;

				case 6:
							err_when(node_x!=childNode->node_x+1 || node_y!=childNode->node_y-1);
							if(can_move_to(childUpperLeftX+1,childUpperLeftY-1) && can_move_to(childUpperLeftX+2,childUpperLeftY-1) &&
								childNode->node_type&0x2 && can_move_to(childUpperLeftX+2,childUpperLeftY))
							{
								switch(enter_direction)
								{
									case 2:	canMove = 0;	//reverse direction
												break;
									case 1:case 3:case 4:case 8:
												canMove = 1;
												break;

									case 5:	if(node_type&0x1 || can_move_to(childUpperLeftX+3, childUpperLeftY))
													canMove = 1;
												break;

									case 6:	if(node_type==15)
													canMove = 1;
												else
													canMove = 0;
												break;

									case 7:	if(node_type&0x8 || can_move_to(childUpperLeftX+1, childUpperLeftY-2))
													canMove = 1;
												break;

									default:	err_here();
												break;
								}
							}
							break;

				case 7:
							err_when(node_x!=childNode->node_x || node_y!=childNode->node_y-1);
							if(can_move_to(childUpperLeftX,childUpperLeftY-1) && can_move_to(childUpperLeftX+1,childUpperLeftY-1) &&
								childNode->node_type&0x1 && childNode->node_type&0x2)
							{
								switch(enter_direction)
								{
									case 3:	canMove = 0;	//reverse direction
												break;

									case 1:case 2:case 4:case 5:case 7:
												canMove = 1;
												break;

									case 6:	if(node_type&0x1 || can_move_to(childUpperLeftX+2, childUpperLeftY-1))
													canMove = 1;
												break;

									case 8:	if(node_type&0x2 || can_move_to(childUpperLeftX-1, childUpperLeftY-1))
													canMove = 1;
												break;

									default:	err_here();
												break;
								}				
							}
							break;

				case 8:
							err_when(node_x!=childNode->node_x-1 || node_y!=childNode->node_y-1);
							if(can_move_to(childUpperLeftX-1,childUpperLeftY-1) && can_move_to(childUpperLeftX,childUpperLeftY-1) &&
								can_move_to(childUpperLeftX-1,childUpperLeftY) && childNode->node_type&0x1)//can_move_to(childUpperLeftX,childUpperLeftY))
							{
								switch(enter_direction)
								{
									case 4:	canMove = 0;	//reverse direction
												break;

									case 2:case 3:case 5:case 6:
												canMove = 1;
												break;

									case 1:	if(node_type&0x2 || can_move_to(childUpperLeftX-2, childUpperLeftY))
													canMove = 1;
												break;

									case 7:	if(node_type&0x4 || can_move_to(childUpperLeftX, childUpperLeftY-2))
													canMove = 1;
												break;

									case 8:	if(node_type==15)
													canMove = 1;
												else
													canMove = 0;
												break;

									default:	err_here();
												break;
								}
							}
							break;

				default:	err_here();
							break;
			}

			if(canMove)	// can move in the child node
			{
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

				if((exitDirection%2 && testResult<=2) || (!exitDirection%2 && testResult<=1))
					cost = 1;
				else
					cost = 2;
				
				err_when(cost>2 || cost<1);
				if(g+cost < childNode->node_g) // second checking, mainly for cost = 2;
				{
					char canPropagateDown=1;

					if(childNode->node_type!=15 && childEnterDirection%2==0)
					{
						NodeS2*	oldChildNode;
						short xShift1 = x_shift_array[childEnterDirection-1];
						short	yShift1 = y_shift_array[childEnterDirection-1];
						short xShift2, yShift2, xShift3, yShift3;
						if(childEnterDirection>1)
						{
							xShift2 = x_shift_array[childEnterDirection-2];
							yShift2 = y_shift_array[childEnterDirection-2];
						}
						else
						{
							xShift2 = x_shift_array[7];
							yShift2 = y_shift_array[7];
						}

						if(childEnterDirection<8)
						{
							xShift3 = x_shift_array[childEnterDirection];
							yShift3 = y_shift_array[childEnterDirection];
						}
						else
						{
							xShift3 = x_shift_array[0];
							yShift3 = y_shift_array[0];
						}

						for(c=0; c<MAX_CHILD_NODE && childNode->child_node[c]; c++) // for each child of oldNode
						{
							oldChildNode = childNode->child_node[c];
							if(childNode->parent_node != oldChildNode)
							{
								switch(childEnterDirection)
								{	
									case 2:	
												if(oldChildNode->enter_direction==childEnterDirection && 
													oldChildNode->node_x-childNode->node_x==xShift1 &&
													oldChildNode->node_y-childNode->node_y==yShift1)
													canPropagateDown = 0;

												if(oldChildNode->enter_direction==childEnterDirection-1 &&
													oldChildNode->node_x-childNode->node_x==xShift2 &&
													oldChildNode->node_x-childNode->node_y==yShift2 &&
													!can_move_to(childUpperLeftX,childUpperLeftY)&&
													!can_move_to(childUpperLeftX+1,childUpperLeftY+2))
													canPropagateDown = 0;
	
												if(oldChildNode->enter_direction==childEnterDirection+1 &&
													oldChildNode->node_x-childNode->node_x==xShift2 &&
													oldChildNode->node_x-childNode->node_y==yShift2 &&
													!can_move_to(childUpperLeftX+1,childUpperLeftY+1) &&
													!can_move_to(childUpperLeftX-1,childUpperLeftY))
													canPropagateDown = 0;
												break;

									case 4:
												if(oldChildNode->enter_direction==childEnterDirection && 
													oldChildNode->node_x-childNode->node_x==xShift1 &&
													oldChildNode->node_y-childNode->node_y==yShift1)
													canPropagateDown = 0;

												if(oldChildNode->enter_direction==childEnterDirection-1 &&
													oldChildNode->node_x-childNode->node_x==xShift2 &&
													oldChildNode->node_x-childNode->node_y==yShift2 &&
													!can_move_to(childUpperLeftX,childUpperLeftY+1)&&
													!can_move_to(childUpperLeftX+2,childUpperLeftY))
													canPropagateDown = 0;

												if(oldChildNode->enter_direction==childEnterDirection+1 &&
													oldChildNode->node_x-childNode->node_x==xShift2 &&
													oldChildNode->node_x-childNode->node_y==yShift2 &&
													!can_move_to(childUpperLeftX,childUpperLeftY+2) &&
													!can_move_to(childUpperLeftX+1,childUpperLeftY))
													canPropagateDown = 0;
												break;

									case 6:
												if(oldChildNode->enter_direction==childEnterDirection && 
													oldChildNode->node_x-childNode->node_x==xShift1 &&
													oldChildNode->node_y-childNode->node_y==yShift1)
													canPropagateDown = 0;
		
												if(oldChildNode->enter_direction==childEnterDirection-1 &&
													oldChildNode->node_x-childNode->node_x==xShift2 &&
													oldChildNode->node_x-childNode->node_y==yShift2 &&
													!can_move_to(childUpperLeftX,childUpperLeftY-1)&&
													!can_move_to(childUpperLeftX+1,childUpperLeftY+1))
													canPropagateDown = 0;

												if(oldChildNode->enter_direction==childEnterDirection+1 &&
													oldChildNode->node_x-childNode->node_x==xShift2 &&
													oldChildNode->node_x-childNode->node_y==yShift2 &&
													!can_move_to(childUpperLeftX,childUpperLeftY) &&
													!can_move_to(childUpperLeftX+2,childUpperLeftY+1))
													canPropagateDown = 0;
												break;

									case 8:
												if(oldChildNode->enter_direction==childEnterDirection && 
													oldChildNode->node_x-childNode->node_x==xShift1 &&
													oldChildNode->node_y-childNode->node_y==yShift1)
													canPropagateDown = 0;

												if(oldChildNode->enter_direction==childEnterDirection-1 &&
													oldChildNode->node_x-childNode->node_x==xShift2 &&
													oldChildNode->node_x-childNode->node_y==yShift2 &&
													!can_move_to(childUpperLeftX-1,childUpperLeftY+1)&&
													!can_move_to(childUpperLeftX+1,childUpperLeftY))
													canPropagateDown = 0;

												if(oldChildNode->enter_direction==childEnterDirection+1 &&
													oldChildNode->node_x-childNode->node_x==xShift2 &&
													oldChildNode->node_x-childNode->node_y==yShift2 &&
													!can_move_to(childUpperLeftX+1,childUpperLeftY-1) &&
													!can_move_to(childUpperLeftX,childUpperLeftY+1))
													canPropagateDown = 0;
												break;

									default:	err_here();
												break;
								}
							}	
						}
					}
					
					if(canPropagateDown)
					{
						childNode->node_g 	  = g+cost;
						childNode->node_f 	  = childNode->node_g+childNode->node_h;
						childNode->parent_node = this;// reset parent to new path.
						childNode->enter_direction = childEnterDirection;
						stack_push(childNode);			// Now the childNode's branch need to be checked out. Remember the new cost must be propagated down.
					}
				}
			}
		}     
	}

	while( cur_stack_pos_s2>0 )
	{
		fatherNode=stack_pop();
		g = fatherNode->node_g;

		for(c=0;c<8;c++)
		{
			if ((childNode=fatherNode->child_node[c])==NULL)       // we may stop the propagation 2 ways: either
				break;

			if(childNode->parent_node==fatherNode)
				break;

			cost = 2; // in fact, may be 1 or 2
			if (g+cost <= childNode->node_g)	// first checking
				// there are no children, or that the g value of the child is equal or better than the cost we're propagating
			{	
				char canMove = 0;
				int childUpperLeftX = childNode->node_x<<1;
				int childUpperLeftY = childNode->node_y<<1;
				
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

				switch(childEnterDirection)
				{
					case 1:
								err_when(fatherNode->node_x!=childNode->node_x-1 || fatherNode->node_y!=childNode->node_y);
								if(can_move_to(childUpperLeftX-1,childUpperLeftY) && childNode->node_type&0x1 && //can_move_to(childUpperLeftX,childUpperLeftY) &&
									can_move_to(childUpperLeftX-1,childUpperLeftY+1) && childNode->node_type&0x4)//can_move_to(childUpperLeftX,childUpperLeftY+1))
								{
									switch(fatherNode->enter_direction)
									{
										case 5:	canMove = 0;	//reverse direction
													break;

										case 1:case 3:case 4:case 6:case 7:
													canMove = 1;
													break;

										case 2:	if(fatherNode->node_type&0x1 || can_move_to(childUpperLeftX-1, childUpperLeftY+2))
														canMove = 1;
													break;

										case 8:	if(fatherNode->node_type&0x4 || can_move_to(childUpperLeftX-1, childUpperLeftY-1))
														canMove = 1;
													break;

										default:	err_here();
													break;
									}
								}
								break;

					case 2:
								err_when(fatherNode->node_x!=childNode->node_x-1 || fatherNode->node_y!=childNode->node_y+1);
								if(can_move_to(childUpperLeftX-1,childUpperLeftY+1) && childNode->node_type&0x4 &&//can_move_to(childUpperLeftX,childUpperLeftY+1) &&
									can_move_to(childUpperLeftX-1,childUpperLeftY+2) && can_move_to(childUpperLeftX,childUpperLeftY+2))
								{
									switch(fatherNode->enter_direction)
									{
										case 6:	canMove = 0;	//reverse direction
													break;

										case 4:case 5:case 7:case 8:
													canMove = 1;
													break;

										case 1:	if(fatherNode->node_type&0x8 || can_move_to(childUpperLeftX-2,childUpperLeftY+1))
														canMove = 1;
													break;

										case 2:	if(fatherNode->node_type==15)
														canMove = 1;
													else
														canMove = 0;
													break;

										case 3:	if(fatherNode->node_type&0x1 || can_move_to(childUpperLeftX,childUpperLeftY+3))
														canMove = 1;
													break;

										default:	err_here();
													break;
									}
									canMove = 1;
								}
								break;

					case 3:
								err_when(fatherNode->node_x!=childNode->node_x || fatherNode->node_y!=childNode->node_y+1);
								if(childNode->node_type&0x4 && childNode->node_type&0x8 &&
									can_move_to(childUpperLeftX,childUpperLeftY+2) && can_move_to(childUpperLeftX+1,childUpperLeftY+2))
								{
									switch(fatherNode->enter_direction)
									{
										case 7:	canMove = 0;	//reverse direction
													break;

										case 1:case 3:case 5:case 6:case 8:
													canMove = 1;
													break;

										case 2:	if(fatherNode->node_type&0x8 || can_move_to(childUpperLeftX-1,childUpperLeftY+2))
														canMove = 1;
													break;

										case 4:	if(fatherNode->node_type&0x4 || can_move_to(childUpperLeftX+2,childUpperLeftY+2))
														canMove = 1;
													break;

										default:	err_here();
													break;
									}
								}
								break;

					case 4:
								err_when(fatherNode->node_x!=childNode->node_x+1 || fatherNode->node_y!=childNode->node_y+1);
								if(childNode->node_type&0x8 && can_move_to(childUpperLeftX+2,childUpperLeftY+1) &&
									can_move_to(childUpperLeftX+1,childUpperLeftY+2) && can_move_to(childUpperLeftX+2,childUpperLeftY+2))
								{
									switch(fatherNode->enter_direction)
									{
										case 8:	canMove = 0;	//reverse direction
													break;

										case 1:case 2:case 6:case 7:
													canMove = 1;
													break;

										case 3:	if(fatherNode->node_type&0x2 || can_move_to(childUpperLeftX+1,childUpperLeftY+3))
														canMove = 1;
													break;

										case 4:	if(fatherNode->node_type==15)
														canMove = 1;
													else 
														canMove = 0;
													break;

										case 5:	if(fatherNode->node_type&0x4 && can_move_to(childUpperLeftX+3,childUpperLeftY+1))
														canMove = 1;
													break;

										default:	err_here();
													break;
									}
								}
								break;

					case 5:
								err_when(fatherNode->node_x!=childNode->node_x+1 || fatherNode->node_y!=childNode->node_y);
								if(childNode->node_type&0x2 && can_move_to(childUpperLeftX+2,childUpperLeftY) &&
									childNode->node_type&0x8 && can_move_to(childUpperLeftX+2,childUpperLeftY+1))
								{
									switch(fatherNode->enter_direction)
									{
										case 1:	canMove = 0;	//reverse direction
													break;
									
										case 2:case 3:case 5:case 7:case 8:
													canMove = 1;
													break;

										case 4:	if(fatherNode->node_type&0x2 || can_move_to(childUpperLeftX+2,childUpperLeftY+2))
														canMove = 1;
													break;

										case 6:	if(fatherNode->node_type&0x8 || can_move_to(childUpperLeftX+2,childUpperLeftY-1))
														canMove = 1;
													break;

										default:	err_here();
													break;
									}
								}
								break;
	
					case 6:
								err_when(fatherNode->node_x!=childNode->node_x+1 || fatherNode->node_y!=childNode->node_y-1);
								if(can_move_to(childUpperLeftX+1,childUpperLeftY-1) && can_move_to(childUpperLeftX+2,childUpperLeftY-1) &&
									childNode->node_type&0x2 && can_move_to(childUpperLeftX+2,childUpperLeftY))
								{
									switch(fatherNode->enter_direction)
									{
										case 2:	canMove = 0;	//reverse direction
													break;
									
										case 1:case 3:case 4:case 8:
													canMove = 1;
													break;

										case 5:	if(fatherNode->node_type&0x1 || can_move_to(childUpperLeftX+3,childUpperLeftY))
														canMove = 1;
													break;

										case 6:	if(fatherNode->node_type==15)
														canMove = 1;
													else
														canMove = 0;
													break;

										case 7:	if(fatherNode->node_type&0x8 || can_move_to(childUpperLeftX+1,childUpperLeftY-2))
														canMove = 1;
													break;

										default:	err_here();
													break;
									}
								}
								break;

					case 7:
								err_when(fatherNode->node_x!=childNode->node_x || fatherNode->node_y!=childNode->node_y-1);
								if(can_move_to(childUpperLeftX,childUpperLeftY-1) && can_move_to(childUpperLeftX+1,childUpperLeftY-1) &&
									childNode->node_type&0x1 && childNode->node_type&0x2)
								{
									switch(fatherNode->enter_direction)
									{
										case 3:	canMove = 0;	//reverse direction
													break;
									
										case 1:case 2:case 4:case 5:case 7:
													canMove = 1;
													break;

										case 6:	if(fatherNode->node_type&0x1 || can_move_to(childUpperLeftX+2,childUpperLeftY-1))
														canMove = 1;
													break;

										case 8:	if(fatherNode->node_type&0x2 || can_move_to(childUpperLeftX-1,childUpperLeftY-1))
														canMove = 1;
													break;

										default:	err_here();
													break;
									}
								}
								break;

					case 8:
								err_when(fatherNode->node_x!=childNode->node_x-1 || fatherNode->node_y!=childNode->node_y-1);
								if(can_move_to(childUpperLeftX-1,childUpperLeftY-1) && can_move_to(childUpperLeftX,childUpperLeftY-1) &&
									can_move_to(childUpperLeftX-1,childUpperLeftY) && childNode->node_type&0x1)//can_move_to(childUpperLeftX,childUpperLeftY))
								{
									switch(fatherNode->enter_direction)
									{
										case 4:	canMove = 0;	//reverse direction
													break;

										case 2:case 3:case 5:case 6:
													canMove = 1;
													break;

										case 1:	if(fatherNode->node_type&0x2 || can_move_to(childUpperLeftX-2,childUpperLeftY))
														canMove = 1;
													break;

										case 8:	if(fatherNode->node_type==15)
														canMove = 1;
													else
														canMove = 0;
													break;

										case 7:	if(fatherNode->node_type&0x4 || can_move_to(childUpperLeftX,childUpperLeftY-2))
														canMove = 1;
													break;

										default:	err_here();
													break;
									}
								}
								break;
	
					default:	err_here();
								break;
				}

				if(canMove)
				{	
					exitDirection = (childEnterDirection+3)%8+1;
					if(fatherNode->enter_direction > exitDirection)
					{
						if((fatherNode->enter_direction==8 && (exitDirection==1 || exitDirection==2)) ||
							(fatherNode->enter_direction==7 && exitDirection==1))
							testResult = exitDirection + 8 - fatherNode->enter_direction;
						else
							testResult = fatherNode->enter_direction - exitDirection;
					}
					else
					{
						if((exitDirection==8 && (fatherNode->enter_direction==1 || fatherNode->enter_direction==2)) ||
							(exitDirection==7 && fatherNode->enter_direction==1))
							testResult = fatherNode->enter_direction + 8 - exitDirection;
						else
							testResult = exitDirection - fatherNode->enter_direction;
					}

					if((exitDirection%2 && testResult<=2) || (!exitDirection%2 && testResult<=1))
						cost = 1;
					else
						cost = 2;
	
					err_when(cost>2 || cost<1);
					if(g+cost < childNode->node_g) // second checking, mainly for cost = 2;
					{
						char canPropagateDown=1;
						if(childNode->node_type!=15 && childEnterDirection%2==0)
						{
							NodeS2*	oldChildNode;
							short xShift1 = x_shift_array[childEnterDirection-1];
							short	yShift1 = y_shift_array[childEnterDirection-1];
							short xShift2, yShift2, xShift3, yShift3;
							if(childEnterDirection>1)
							{
								xShift2 = x_shift_array[childEnterDirection-2];
								yShift2 = y_shift_array[childEnterDirection-2];
							}
							else
							{
								xShift2 = x_shift_array[7];
								yShift2 = y_shift_array[7];
							}

							if(childEnterDirection<8)
							{
								xShift3 = x_shift_array[childEnterDirection];
								yShift3 = y_shift_array[childEnterDirection];
							}
							else
							{
								xShift3 = x_shift_array[0];
								yShift3 = y_shift_array[0];
							}

							for(c=0; c<MAX_CHILD_NODE && childNode->child_node[c]; c++) // for each child of oldNode
							{
								oldChildNode = childNode->child_node[c];
								if(childNode->parent_node != oldChildNode)
								{
									switch(childEnterDirection)
									{	
										case 2:	
													if(oldChildNode->enter_direction==childEnterDirection && 
														oldChildNode->node_x-childNode->node_x==xShift1 &&
														oldChildNode->node_y-childNode->node_y==yShift1)
														canPropagateDown = 0;

													if(oldChildNode->enter_direction==childEnterDirection-1 &&
														oldChildNode->node_x-childNode->node_x==xShift2 &&
														oldChildNode->node_x-childNode->node_y==yShift2 &&
														!can_move_to(childUpperLeftX,childUpperLeftY)&&
														!can_move_to(childUpperLeftX+1,childUpperLeftY+2))
														canPropagateDown = 0;
	
													if(oldChildNode->enter_direction==childEnterDirection+1 &&
														oldChildNode->node_x-childNode->node_x==xShift2 &&
														oldChildNode->node_x-childNode->node_y==yShift2 &&
														!can_move_to(childUpperLeftX+1,childUpperLeftY+1) &&
														!can_move_to(childUpperLeftX-1,childUpperLeftY))
														canPropagateDown = 0;
													break;

										case 4:
													if(oldChildNode->enter_direction==childEnterDirection && 
														oldChildNode->node_x-childNode->node_x==xShift1 &&
														oldChildNode->node_y-childNode->node_y==yShift1)
														canPropagateDown = 0;
	
													if(oldChildNode->enter_direction==childEnterDirection-1 &&
														oldChildNode->node_x-childNode->node_x==xShift2 &&
														oldChildNode->node_x-childNode->node_y==yShift2 &&
														!can_move_to(childUpperLeftX,childUpperLeftY+1)&&
														!can_move_to(childUpperLeftX+2,childUpperLeftY))
														canPropagateDown = 0;

													if(oldChildNode->enter_direction==childEnterDirection+1 &&
														oldChildNode->node_x-childNode->node_x==xShift2 &&
														oldChildNode->node_x-childNode->node_y==yShift2 &&
														!can_move_to(childUpperLeftX,childUpperLeftY+2) &&
														!can_move_to(childUpperLeftX+1,childUpperLeftY))
														canPropagateDown = 0;
													break;

										case 6:
													if(oldChildNode->enter_direction==childEnterDirection && 
														oldChildNode->node_x-childNode->node_x==xShift1 &&
														oldChildNode->node_y-childNode->node_y==yShift1)
														canPropagateDown = 0;
			
													if(oldChildNode->enter_direction==childEnterDirection-1 &&
														oldChildNode->node_x-childNode->node_x==xShift2 &&
														oldChildNode->node_x-childNode->node_y==yShift2 &&
														!can_move_to(childUpperLeftX,childUpperLeftY-1)&&
														!can_move_to(childUpperLeftX+1,childUpperLeftY+1))
														canPropagateDown = 0;

													if(oldChildNode->enter_direction==childEnterDirection+1 &&
														oldChildNode->node_x-childNode->node_x==xShift2 &&
														oldChildNode->node_x-childNode->node_y==yShift2 &&
														!can_move_to(childUpperLeftX,childUpperLeftY) &&
														!can_move_to(childUpperLeftX+2,childUpperLeftY+1))
														canPropagateDown = 0;
													break;

										case 8:
													if(oldChildNode->enter_direction==childEnterDirection && 
														oldChildNode->node_x-childNode->node_x==xShift1 &&
														oldChildNode->node_y-childNode->node_y==yShift1)
														canPropagateDown = 0;
	
													if(oldChildNode->enter_direction==childEnterDirection-1 &&
														oldChildNode->node_x-childNode->node_x==xShift2 &&
														oldChildNode->node_x-childNode->node_y==yShift2 &&
														!can_move_to(childUpperLeftX-1,childUpperLeftY+1)&&
														!can_move_to(childUpperLeftX+1,childUpperLeftY))
														canPropagateDown = 0;

													if(oldChildNode->enter_direction==childEnterDirection+1 &&
														oldChildNode->node_x-childNode->node_x==xShift2 &&
														oldChildNode->node_x-childNode->node_y==yShift2 &&
														!can_move_to(childUpperLeftX+1,childUpperLeftY-1) &&
														!can_move_to(childUpperLeftX,childUpperLeftY+1))
														canPropagateDown = 0;
													break;

										default:	err_here();
													break;
									}
								}
							}	
						}
						
						if(canPropagateDown)
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
	}
}
//------- End of function NodeS2::propagate_down -------//


//-------- Begin of static function stack_push ---------//

static void stack_push(NodeS2 *nodePtr)
{
	stack_array[cur_stack_pos_s2++] = nodePtr;

	err_when( cur_stack_pos_s2 >= MAX_STACK_NUM );
}
//--------- End of static function stack_push ---------//


//-------- Begin of static function stack_pop ---------//

static NodeS2* stack_pop()
{
	return stack_array[--cur_stack_pos_s2];
}
//--------- End of static function stack_pop ---------//