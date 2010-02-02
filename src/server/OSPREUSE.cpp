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

//Filename		:	OSPREUSE.CPP
//Description	:	Object SeekPathReuse
//Owner			:	Alex

#include <OSPREUSE.h>
#include <OSPATH.h>
#include <ALL.h>
#include <OWORLD.h>
#include <OUNIT.h>
#include <OSYS.h>

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
#undef debug_reuse_check_sub_mode_node
#define debug_reuse_check_sub_mode_node(x, y)
#undef DEBUG
#endif

//--------------------------------------------------------//
// define static variables
//--------------------------------------------------------//
int	SeekPathReuse::max_node;
short SeekPathReuse::total_num_of_path;
short SeekPathReuse::cur_path_num;
short SeekPathReuse::unit_size;
DWORD SeekPathReuse::cur_group_id;
char	SeekPathReuse::mobile_type;
char	SeekPathReuse::unit_nation_recno;
short SeekPathReuse::move_scale;
short SeekPathReuse::search_mode;
short SeekPathReuse::reuse_mode;
short SeekPathReuse::reuse_path_status;
short SeekPathReuse::reuse_path_dist;
short *SeekPathReuse::reuse_node_matrix=NULL;

char SeekPathReuse::reuse_nation_passable[MAX_NATION+1] = {0};
char SeekPathReuse::reuse_search_sub_mode;

//----------------- backup leader path(reference path) ----------------------//
short	SeekPathReuse::leader_path_num;
ResultNode *SeekPathReuse::reuse_leader_path_backup;
int SeekPathReuse::reuse_leader_path_node_num;
int SeekPathReuse::leader_path_start_x;
int SeekPathReuse::leader_path_start_y;
int SeekPathReuse::leader_path_dest_x;
int SeekPathReuse::leader_path_dest_y;

//----------- decide which offset method is used ----------//
int SeekPathReuse::start_x_offset;
int SeekPathReuse::start_y_offset;
int SeekPathReuse::dest_x_offset;
int SeekPathReuse::dest_y_offset;
int SeekPathReuse::x_offset;
int SeekPathReuse::y_offset;
int SeekPathReuse::formation_x_offset;
int SeekPathReuse::formation_y_offset;
int SeekPathReuse::start_x;
int SeekPathReuse::start_y;
int SeekPathReuse::dest_x;
int SeekPathReuse::dest_y;
int SeekPathReuse::vir_dest_x;
int SeekPathReuse::vir_dest_y;

//---------- the current constructing result path --------//
ResultNode *SeekPathReuse::path_reuse_result_node_ptr;
int SeekPathReuse::num_of_result_node;
ResultNode *SeekPathReuse::cur_result_node_ptr;

short SeekPathReuse::result_node_array_def_size;
short SeekPathReuse::result_node_array_reset_amount;

//-- determine the current offset difference(leader path information) --//
ResultNode *SeekPathReuse::cur_leader_node_ptr;
int SeekPathReuse::cur_leader_node_num;

short SeekPathReuse::leader_vec_x;
short SeekPathReuse::leader_vec_y;

//----------- for smoothing the result path --------------//
short SeekPathReuse::vec_x;
short SeekPathReuse::vec_y;
short SeekPathReuse::new_vec_x;
short SeekPathReuse::new_vec_y;
int SeekPathReuse::vec_magn;
int SeekPathReuse::new_vec_magn;
int SeekPathReuse::result_vec_x;
int SeekPathReuse::result_vec_y;


//-------- Begin of function SeekPathReuse::init ---------//
void SeekPathReuse::init(int maxNode)
{
	//------------------ initialize parameters ----------------//
	incomplete_search		= 0;
	max_node					= maxNode;

	total_num_of_path		= 0;
	cur_path_num			= 0;
	cur_group_id			= 0;
	mobile_type				= 0;
	search_mode				= 0;
	reuse_mode				= 0;
	reuse_path_status		= 0;

	reuse_leader_path_backup	= NULL;
	reuse_leader_path_node_num = 0;
	leader_path_start_x			= 0;
	leader_path_start_y			= 0;
	leader_path_dest_x			= 0;
	leader_path_dest_y			= 0;

	start_x_offset			= 0;
	start_y_offset			= 0;
	dest_x_offset			= 0;
	dest_y_offset			= 0;
	x_offset					= 0;
	y_offset					= 0;
	vir_dest_x				= 0;
	vir_dest_y				= 0;

	num_of_result_node					= 0;
	result_node_array_def_size			= 0;
	result_node_array_reset_amount	= 10;		// dafault value is 10
	cur_result_node_ptr					= NULL;
	path_reuse_result_node_ptr			= NULL;
	
	cur_leader_node_ptr	= NULL;
	cur_leader_node_num	= 0;
	leader_path_num		= -1;	// valid value is not less than 0
	leader_vec_x			= 0;
	leader_vec_y			= 0;
	
	init_reuse_node_matrix();	// initialize node matrix for setting offset path
}
//--------- End of function SeekPathReuse::init ---------//


//-------- Begin of function SeekPathReuse::deinit ---------//
void SeekPathReuse::deinit()
{
	deinit_reuse_search();			// destruct data structure

	if(reuse_node_matrix)
	{
		mem_del(reuse_node_matrix);
		reuse_node_matrix = NULL;
	}
}
//--------- End of function SeekPathReuse::deinit ---------//


//-------- Begin of function SeekPathReuse::init_reuse_node_matrix ---------//
void SeekPathReuse::init_reuse_node_matrix()
{
	reuse_node_matrix = (short*) mem_add(sizeof(short)*MAX_WORLD_X_LOC*MAX_WORLD_Y_LOC/4);
}
//--------- End of function SeekPathReuse::init_reuse_node_matrix ---------//


//-------- Begin of function SeekPathReuse::deinit_reuse_search ---------//
// destruct data structure
//
void SeekPathReuse::deinit_reuse_search()
{
	if(reuse_leader_path_backup!=NULL)
	{
		mem_del(reuse_leader_path_backup);
		reuse_leader_path_backup = NULL;
	}
	
	err_when(reuse_leader_path_backup!=NULL);
	reuse_leader_path_node_num = 0;
	leader_path_start_x = 0;
	leader_path_start_y = 0;
	leader_path_dest_x = 0;
	leader_path_dest_y = 0;
}
//--------- End of function SeekPathReuse::deinit_reuse_search ---------//


//-------- Begin of function SeekPathReuse::init_reuse_search ---------//
// re-construct data structure and re-initialzie parameters
//
void SeekPathReuse::init_reuse_search()
{
	incomplete_search				= 0;

	cur_path_num					= 0;
	result_node_array_def_size	= 0;
	leader_path_num				= -1;	// set to invalid value -1 after each initialization
	cur_result_node_ptr			= NULL;
	path_reuse_result_node_ptr	= NULL;
	num_of_result_node			= 0;
	
	deinit_reuse_search();
	
	reuse_leader_path_backup	= NULL;
	reuse_leader_path_node_num	= 0;
	leader_path_start_x			= 0;
	leader_path_start_y			= 0;
	leader_path_dest_x			= 0;
	leader_path_dest_y			= 0;
}
//--------- End of function SeekPathReuse::init_reuse_search ---------//


//-------- Begin of function SeekPathReuse::get_reuse_path_status ---------//
char SeekPathReuse::get_reuse_path_status()
{
	return (char) reuse_path_status;
}
//--------- End of function SeekPathReuse::get_reuse_path_status ---------//


//-------- Begin of function SeekPathReuse::bound_check_x ---------//
/*inline void SeekPathReuse::bound_check_x(int& x)
{
	if(x<0)
		x = 0;
	else if(x>=MAX_WORLD_X_LOC)
		x = MAX_WORLD_X_LOC-move_scale;
}*/
//--------- End of function SeekPathReuse::bound_check_x ---------//


//-------- Begin of function SeekPathReuse::bound_check_y ---------//
/*inline void SeekPathReuse::bound_check_y(int& y)
{
	if(y<0)
		y = 0;
	else if(y>=MAX_WORLD_X_LOC)
		y = MAX_WORLD_X_LOC-move_scale;
}*/
//--------- End of function SeekPathReuse::bound_check_y ---------//


//-------- Begin of function SeekPathReuse::call_seek ---------//
/*inline ResultNode* SeekPathReuse::call_seek(int sx, int sy, int dx, int dy, DWORD groupId, char mobileType,
														  short searchMode, int& nodeCount)
{
	err_when(unit_size!=1);
	seek_path.seek(sx, sy, dx, dy, groupId, mobileType, searchMode);
	return seek_path.get_result(nodeCount, reuse_path_dist);
}*/
//--------- End of function SeekPathReuse::call_seek ---------//


//-------- Begin of function SeekPathReuse::set_offset_condition ---------//
// store the starting location, destination, offset of each path
//
void SeekPathReuse::set_offset_condition(int startX, int startY, int destX, int destY)
{
	//-------------------------------------------------------------//
	// the offset is used to determine what the methods is used to
	// do the path-reuse
	//-------------------------------------------------------------//
	err_when(leader_path_num>=cur_path_num);

	if(cur_path_num>0)
	{
		//---- the following value is useless if no valid leader path -------//
		start_x_offset	= startX - leader_path_start_x;
		start_y_offset	= startY - leader_path_start_y;
		dest_x_offset	= destX - leader_path_dest_x;
		dest_y_offset	= destY - leader_path_dest_y;
	}
	else		// no leader path is available
	{
		err_when(reuse_path_status != REUSE_PATH_FIRST_SEEK);

		start_x_offset	= start_y_offset = dest_x_offset = dest_y_offset = 0;
		leader_path_start_x = startX;
		leader_path_start_y = startY;
		leader_path_dest_x = destX;
		leader_path_dest_y = destY;

		leader_path_num = cur_path_num;
	}
}
//--------- End of function SeekPathReuse::set_offset_condition ---------//


//-------- Begin of function SeekPathReuse::set_next_cur_path_num ---------//
//	This function may be called outside by Unit::move_to
//
void SeekPathReuse::set_next_cur_path_num()
{
	cur_path_num++;
}
//--------- End of function SeekPathReuse::set_next_cur_path_num ---------//


//-------- Begin of function SeekPathReuse::is_node_avail_empty ---------//
// return 1 if node available for search is zero
// return 0 otherwise
//
int SeekPathReuse::is_node_avail_empty()
{
	err_when(unit_size!=1);
	return seek_path.total_node_avail<MIN_BACKGROUND_NODE_USED_UP;
}
//--------- End of function SeekPathReuse::is_node_avail_empty ---------//


//-------- Begin of function SeekPathReuse::is_leader_path_valid ---------//
// return 1 leader path is valid
// return 0 otherwise
//
int SeekPathReuse::is_leader_path_valid()
{
	if(leader_path_num<0 || reuse_leader_path_node_num<2)
	{
		//---------------------------------------------------------------//
		// if no leader path of if the number of node in the leader path < 2.
		// A new one will be chosed to be the leader-path.
		//---------------------------------------------------------------//
		if(reuse_leader_path_backup!=NULL)
		{
			mem_del(reuse_leader_path_backup);
			reuse_leader_path_backup = NULL;
		}

		err_when(reuse_leader_path_backup!=NULL);

		//start_x_offset	= start_y_offset = dest_x_offset = dest_y_offset = 0;
		leader_path_start_x = start_x;
		leader_path_start_y = start_y;
		leader_path_dest_x = dest_x;
		leader_path_dest_y = dest_y;

		leader_path_num = cur_path_num;	// set leader path number
		if(is_node_avail_empty())
		{
			incomplete_search++;
			reuse_path_status = REUSE_PATH_INCOMPLETE_SEARCH;
			return 0;
		}

		path_reuse_result_node_ptr = call_seek(start_x, start_y, vir_dest_x, vir_dest_y, cur_group_id, mobile_type,
															SEARCH_MODE_IN_A_GROUP, num_of_result_node);

		//------------------------------------------------------------------------//
		// checking for incomplete searching
		//------------------------------------------------------------------------//
		if(!path_reuse_result_node_ptr || !num_of_result_node)
		{
			incomplete_search = 1;
			reuse_path_status = REUSE_PATH_INCOMPLETE_SEARCH;
			return 0;
		}

		err_when(!path_reuse_result_node_ptr || !num_of_result_node);
		ResultNode *lastNodePtr = path_reuse_result_node_ptr + num_of_result_node - 1;
		if(lastNodePtr->node_x!=vir_dest_x || lastNodePtr->node_y!=vir_dest_y)
		{
			err_when(unit_size!=1);
			if(seek_path.path_status==PATH_NODE_USED_UP)
			{
				incomplete_search++;
				reuse_path_status = REUSE_PATH_INCOMPLETE_SEARCH;
			}
		}

		return 0;
	}

	return 1;
}
//--------- End of function SeekPathReuse::is_leader_path_valid ---------//


//-------- Begin of function SeekPathReuse::set_nation_passable ---------//
void SeekPathReuse::set_nation_passable(char nationPassable[])
{
	memcpy(reuse_nation_passable+1, nationPassable, sizeof(char)*MAX_NATION);
}
//--------- End of function SeekPathReuse::set_nation_passable ---------//


//-------- Begin of function SeekPathReuse::set_sub_mode ---------//
void SeekPathReuse::set_sub_mode(char subMode)
{
	reuse_search_sub_mode = subMode;
}
//--------- End of function SeekPathReuse::set_sub_mode ---------//


//-------- Begin of function SeekPathReuse::set_status ---------//
void SeekPathReuse::set_status(char newStatus)
{
	reuse_path_status = newStatus;
}
//--------- End of function SeekPathReuse::set_status ---------//


//-------- Begin of function SeekPathReuse::count_path_dist ---------//
short SeekPathReuse::count_path_dist(ResultNode* nodeArray, int nodeNum)
 {
 	if(!nodeArray || !nodeNum)
 		return 0;
 
 	err_when(nodeNum<2);
 
 	ResultNode *preNode = nodeArray;
	ResultNode *curNode = preNode+1;
	int processed = 1;
	int xDist, yDist;
 	short pathDist = 0;
 
 	while(processed++ < nodeNum)
	{
		xDist = abs(preNode->node_x - curNode->node_x);
		yDist = abs((preNode++)->node_y - (curNode++)->node_y);
		err_when((!xDist && !yDist) || (xDist && yDist && xDist!=yDist));

		pathDist += (xDist) ? xDist : yDist;
	}

	err_when(mobile_type!=UNIT_LAND && pathDist%2);
	return pathDist;
}
//--------- End of function SeekPathReuse::count_path_dist ---------//


//-------- Begin of function SeekPathReuse::add_result ---------//
// add the result node in the reuse result node array and resize
// the array size if default size is not large enough to hold the
// data
// 
void SeekPathReuse::add_result(int x, int y)
{
	debug_reuse_check_sub_mode_node(x, y);

	if(num_of_result_node>=result_node_array_def_size)	// the array is not enough to hold the data
	{
		result_node_array_def_size += result_node_array_reset_amount;
		path_reuse_result_node_ptr = (ResultNode*) mem_resize(path_reuse_result_node_ptr, sizeof(ResultNode)* result_node_array_def_size);
		cur_result_node_ptr = path_reuse_result_node_ptr+num_of_result_node;
	}

	cur_result_node_ptr->node_x = x;
	cur_result_node_ptr->node_y = y;
	cur_result_node_ptr++;
	num_of_result_node++;
}
//--------- End of function SeekPathReuse::add_result ---------//


//-------- Begin of function SeekPathReuse::set_index_in_node_matrix ---------//
// Generally speaking, the value of the node in node matrix is less than max_node.
// In order to set a offset path in the node matrix and not conflict the shortest
// path searching, value > max_node is chosed.
//
// In this method, 4 value(max_node+k, where k is 1,..,4) is used for 2x2 node.
// The value k indicate which point in the node will be chosen to be the connection
// point to the offset path.
//
void SeekPathReuse::set_index_in_node_matrix(int xLoc, int yLoc)
{
	short *curLocInNodeMatrix;
	int x = xLoc;
	int y = yLoc;

	//-------------------- boundary checking ---------------//
	bound_check_x(x);
	bound_check_y(y);

	err_when(unit_size!=1);
	curLocInNodeMatrix = reuse_node_matrix + MAX_WORLD_X_LOC/2*(y/2) + (x/2);
	//-------------------------------------------------------------------------//
	// the location is changed into 2x2 node format.
	//		 -- --		In fact, the location is in one of the 4 points in the node.
	//		|1	|2	|		The value k is shown in the figure.  The value in the
	//		 -- --		node_matrix is set to max_node+k.
	//		|3	|4	|		
	//		 -- --		The value will be set again if two points is available in the
	//						node and the last one will be chosen.
	//-------------------------------------------------------------------------//
	if(mobile_type==UNIT_LAND)
		*curLocInNodeMatrix = max_node + 1 + (x%2) + 2*(y%2);
	else
		*curLocInNodeMatrix = max_node + 1 + (x%4!=0) + 2*(y%4!=0);
}
//--------- End of function SeekPathReuse::set_index_in_node_matrix ---------//


//-------- Begin of function SeekPathReuse::move_within_map ---------//
// locations (preX, preY) and (curX, curY) both are inside the map
//
// add one point
//
void SeekPathReuse::move_within_map(int preX, int preY, int curX, int curY)
{
	err_when(preX<0 || preX>=MAX_WORLD_X_LOC || preY<0 || preY>=MAX_WORLD_Y_LOC);
	err_when(curX<0 || curX>=MAX_WORLD_X_LOC || curY<0 || curY>=MAX_WORLD_Y_LOC);
	add_result(curX, curY);
}
//--------- End of function SeekPathReuse::move_within_map ---------//


//-------- Begin of function SeekPathReuse::move_outside_map ---------//
// location (preX, preY) is inside the map while location (curX, curY)
// is outside the map
//
// add one/two points
//
void SeekPathReuse::move_outside_map(int preX, int preY, int curX, int curY)
{
	err_when(preX<0 || preX>=MAX_WORLD_X_LOC || preY<0 || preY>=MAX_WORLD_Y_LOC);
	err_when(curX>=0 && curX<MAX_WORLD_X_LOC && curY>=0 && curY<MAX_WORLD_Y_LOC);

	int vecX = curX-preX;
	int vecY = curY-preY;
	if(vecX!=0) vecX /= abs(vecX);
	if(vecY!=0) vecY /= abs(vecY);

	int vertical=0;		// 1 for upper edge, 2 for lower edge
	int horizontal=0;		// 1 for left edge, 2 for right edge
	int xStep, yStep;
	if(curX<0)
	{
		xStep = preX;
		horizontal = 1;
	}
	else if(curX>=MAX_WORLD_X_LOC)
	{
		xStep = MAX_WORLD_X_LOC-preX-1;
		horizontal = 2;
	}
	else
		err_here();

	if(curY<0)
	{
		yStep = preY;
		vertical = 1;
	}
	else if(curY>=MAX_WORLD_Y_LOC)
	{
		yStep = MAX_WORLD_Y_LOC-preY-1;
		vertical = 2;
	}
	else
		err_here();

	err_when(xStep!=yStep);
	int addXLoc = preX+xStep*vecX;
	int addYLoc = preY+yStep*vecY;
	add_result(addXLoc, addYLoc); // add the first point

	//-*************** codes here ***************-//
	//---------------------------------------------------------------//
	// may add the second point if exit at the edge of the map
	//---------------------------------------------------------------//
	/*if((addXLoc==0 && addYLoc==0) ||
		(addXLoc==0 && addYLoc==MAX_WORLD_Y_LOC-1) ||
		(addXLoc==MAX_WORLD_X_LOC-1 && addYLoc==0) ||
		(addXLoc==MAX_WORLD_X_LOC-1 && addYLoc==MAX_WORLD_Y_LOC-1))
	{
		err_when(!vertical || !horizontal);
		return; // exit at corner
	}*/
}
//--------- End of function SeekPathReuse::move_outside_map ---------//


//-------- Begin of function SeekPathReuse::move_inside_map ---------//
// location (preX, preY) is outside the map and location (curX, curY)
// is insode the map
//
// add one/two points
//
void SeekPathReuse::move_inside_map(int preX, int preY, int curX, int curY)
{
	err_when(preX>=0 && preX<MAX_WORLD_X_LOC && preY>=0 && preY<MAX_WORLD_Y_LOC);
	err_when(curX<0 || curX>=MAX_WORLD_X_LOC || curY<0 || curY>=MAX_WORLD_Y_LOC);
	//-*************** codes here ***************-//
}
//--------- End of function SeekPathReuse::move_inside_map ---------//


//-------- Begin of function SeekPathReuse::move_beyond_map ---------//
// locations (preX, preY) and (curX, curY) are both outside the map
//
// add one/two points
void SeekPathReuse::move_beyond_map(int preX, int preY, int curX, int curY)
{
	err_when(preX>=0 && preX<MAX_WORLD_X_LOC && preY>=0 && preY<MAX_WORLD_Y_LOC);
	err_when(curX>=0 && curX<MAX_WORLD_X_LOC && curY>=0 && curY<MAX_WORLD_Y_LOC);
	//-*************** codes here ***************-//
}
//--------- End of function SeekPathReuse::move_beyond_map ---------//


//-------- Begin of function SeekPathReuse::seek ---------//
//	searchMode	should be 4	(i.e. path reuse searching mode)
//	reuseMode	GENERAL_GROUP_MOVEMENT for general group movement
//					FORMATION_MOVEMENT for formation movement
//
//	miscNo == target record no if searchMode==SEARCH_MODE_TO_ATTACK
//			 == firm ID if searchMode==SEARCH_MODE_TO_FIRM
//
int SeekPathReuse::seek(int sx,int sy,int dx,int dy,short unitSize, DWORD groupId, char mobileType,
								short searchMode, short miscID,
								short numOfPath, short reuseMode, short pathReuseStatus,
								int maxTries,int borderX1, int borderY1, int borderX2, int borderY2)
{
	//---------------------- error checking ---------------------//
	err_when(numOfPath<0 || searchMode!=4 || numOfPath<1);
	err_when(pathReuseStatus!=REUSE_PATH_INITIAL && numOfPath!=total_num_of_path);

	//-------------------------------------------------------------//
	// initialize parameters
	//-------------------------------------------------------------//
	total_num_of_path		= numOfPath;
	reuse_path_status		= pathReuseStatus;

	if(reuse_path_status == REUSE_PATH_INITIAL)
	{
		//-------- initialize data structure and then return ---------//
		init_reuse_search();
		return 1;
	}

	start_x					= sx;
	start_y					= sy;
	dest_x					= dx;
	dest_y					= dy;
	vir_dest_x				= dest_x;
	vir_dest_y				= dest_y;

	unit_size				= unitSize;
	cur_group_id			= groupId;
	mobile_type				= mobileType;
	search_mode				= searchMode;
	reuse_mode				= reuseMode;
	move_scale				= (mobile_type==UNIT_LAND) ? 1 : 2;

	unit_nation_recno		= unit_array[world.get_unit_recno(start_x, start_y, mobile_type)]->nation_recno;
	
	result_node_array_def_size	= 0;
	num_of_result_node			= 0;
	cur_result_node_ptr			= NULL;
	path_reuse_result_node_ptr	= NULL;

	//-------------------------------------------------------------//
	set_offset_condition(sx, sy, dx, dy);

	//-------------------------------------------------------------//
	if(reuse_path_status == REUSE_PATH_FIRST_SEEK)	// first seeking, usually choose to be leader path
	{
		err_when(cur_path_num != 0);
		path_reuse_result_node_ptr = call_seek(sx, sy, dx, dy, cur_group_id, mobileType, SEARCH_MODE_IN_A_GROUP, num_of_result_node);
	}
	else	// it should be REUSE_PATH_SEARCH
	{
		//-------------------------------------------------------------//
		// reuse_mode = GENERAL_GROUP_MOVEMENT
		//-------------------------------------------------------------//
		if(start_x_offset==dest_x_offset && start_y_offset==dest_y_offset)
		{
			//----------------------------------------------------------//
			// optimal case, starting offset==ending offset, thus, using
			// offset method to find the shortest path directly
			//----------------------------------------------------------//
			x_offset = start_x_offset;
			y_offset = start_y_offset;
			
			err_when(cur_path_num==leader_path_num);
			seek_path_offset();
		}
		else
		{
			//-------------------------------------------------------------//
			// the starting location is not the correct offset location,
			// using the join-offset-path method
			//-------------------------------------------------------------//
			x_offset = dest_x_offset;
			y_offset = dest_y_offset;
			seek_path_join_offset();
		}
	}
	
	//-------------------------------------------------------------//
	// backup the ResultNode of this path
	//-------------------------------------------------------------//
	if(num_of_result_node && path_reuse_result_node_ptr)
	{
		#ifdef DEBUG
			if(mobile_type==UNIT_LAND && reuse_search_sub_mode==SEARCH_SUB_MODE_PASSABLE && num_of_result_node>1)
			{
				err_when(mobile_type!=UNIT_LAND);
				debug_reuse_check_sub_mode(path_reuse_result_node_ptr, num_of_result_node);
			}
		#endif

		path_reuse_result_node_ptr = smooth_reuse_path(path_reuse_result_node_ptr, num_of_result_node);

		#ifdef DEBUG
			if(mobile_type==UNIT_LAND && reuse_search_sub_mode==SEARCH_SUB_MODE_PASSABLE && num_of_result_node>1)
			{
				err_when(mobile_type!=UNIT_LAND);
				debug_reuse_check_sub_mode(path_reuse_result_node_ptr, num_of_result_node);
			}
		#endif

		if(leader_path_num==cur_path_num)
		{
			err_when(num_of_result_node>0 && path_reuse_result_node_ptr==NULL);
			reuse_leader_path_backup = (ResultNode*) mem_add(sizeof(ResultNode)*num_of_result_node);
			memcpy(reuse_leader_path_backup, path_reuse_result_node_ptr, sizeof(ResultNode)* num_of_result_node);
			reuse_leader_path_node_num = num_of_result_node;
		}
	}
	else
	{
		path_reuse_result_node_ptr = NULL;
		num_of_result_node = 0;

		if(leader_path_num==cur_path_num)
		{
			reuse_leader_path_backup = NULL;
			reuse_leader_path_node_num = 0;
		}
	}
	
	set_next_cur_path_num();
	err_when(cur_path_num>total_num_of_path+3);

	return 1;
}
//--------- End of function SeekPathReuse::seek ---------//


//-------- Begin of function SeekPathReuse::get_result ---------//
// return the final result node path
//ResultNode* SeekPathReuse::get_result(int& resultNodeCount)
//
ResultNode* SeekPathReuse::get_result(int& resultNodeCount, short& pathDist)
{
	if(reuse_path_status == REUSE_PATH_FIRST_SEEK)
	{
		resultNodeCount = num_of_result_node;
 		reuse_path_dist = count_path_dist(path_reuse_result_node_ptr, num_of_result_node);
 		pathDist = reuse_path_dist;
		return path_reuse_result_node_ptr;
	}
	else
	{
		err_when(reuse_path_status!=REUSE_PATH_SEARCH && reuse_path_status!=REUSE_PATH_INCOMPLETE_SEARCH);

		if(path_reuse_result_node_ptr!=NULL && num_of_result_node>0)
		{
			sys_yield(); // update cursor position
			if(num_of_result_node<2)
			{
				mem_del(path_reuse_result_node_ptr);
				path_reuse_result_node_ptr = NULL;
				num_of_result_node = 0;
			}

			sys_yield(); // update cursor position

			resultNodeCount = num_of_result_node;
 			reuse_path_dist = count_path_dist(path_reuse_result_node_ptr, num_of_result_node);
 			pathDist = reuse_path_dist;
		}
		else
		{
			num_of_result_node = 0;
			if(path_reuse_result_node_ptr!=NULL)
			{
				mem_del(path_reuse_result_node_ptr);
				path_reuse_result_node_ptr = NULL;
			}

			err_when(num_of_result_node!=0 || path_reuse_result_node_ptr!=NULL);
		}

		//******************* debug **********************//
		#ifdef DEBUG
			err_when(mobile_type!=UNIT_LAND && pathDist%2);
			if(!path_reuse_result_node_ptr && num_of_result_node>0)
			{
				//---------------------------------------------------------//
				// final checking, error free for the result_path
				//---------------------------------------------------------//
				ResultNode*	debugResultPtr = path_reuse_result_node_ptr;
				ResultNode*	debugStartNode = path_reuse_result_node_ptr;
				ResultNode*	debugEndNode = debugStartNode + 1;
				int			debugCount = num_of_result_node;
				int			dvX, dvY;	// vector direction
				int			dXLoc, dYLoc;

				err_when(mobile_type!=UNIT_LAND && (debugStartNode->node_x%2 || debugStartNode->node_y%2));
				for(int d=1; d<debugCount; d++)
				{
					//------- check x, y vector magnitude ---------//
					debugStartNode = path_reuse_result_node_ptr + d-1;
					debugEndNode = debugStartNode + d;
					err_when(mobile_type!=UNIT_LAND && (debugEndNode->node_x%2 || debugEndNode->node_y%2));
					dvX = debugEndNode->node_x - debugStartNode->node_x;
					dvY = debugEndNode->node_y - debugStartNode->node_y;

					err_when(dvX!=0 && dvY!=0 && abs(dvX)!=abs(dvY));
					if(dvX)	dvX /= abs(dvX);
					if(dvY)	dvY /= abs(dvY);
					dXLoc = debugStartNode->node_x;
					dYLoc = debugStartNode->node_y;
					
					//-------- check accessible ---------//
					while(dXLoc!=debugEndNode->node_x || dYLoc!=debugEndNode->node_y)
					{
						dXLoc += dvX;
						dYLoc += dvY;
						err_when(unit_size==1 && !can_walk(dXLoc, dYLoc));
						err_when(unit_size==2 && !can_walk_s2(dXLoc, dYLoc));
					}
				}
			}
		#endif
		//******************* debug **********************//

		return path_reuse_result_node_ptr;
	}
}
//--------- End of function SeekPathReuse::get_result ---------//
