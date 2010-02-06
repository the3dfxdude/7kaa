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

#include <OSPREUSE.h>

#ifdef DEBUG
#include <OSYS.h>

static ResultNode*	debugSrePtr; // for debug only
static ResultNode*	debugSreNode1;
static ResultNode*	debugSreNode2;
static int				debugSreCount;
static int				debugSreVX, debugSreVY;	 // for debug only
	
//------------- function debug_check() --------------//
void SeekPathReuse::debug_check()
{
	debugSreNode1 = path_reuse_result_node_ptr;
	debugSreNode2 = path_reuse_result_node_ptr+1;

	for(debugSreCount=1; debugSreCount<num_of_result_node; debugSreCount++, debugSreNode1++, debugSreNode2++)
	{
		err_when(debugSreNode1->node_x<0 || debugSreNode1->node_x>=MAX_WORLD_X_LOC ||
					debugSreNode1->node_y<0 || debugSreNode1->node_y>=MAX_WORLD_Y_LOC);
		debugSreVX = debugSreNode2->node_x - debugSreNode1->node_x;
		debugSreVY = debugSreNode2->node_y - debugSreNode1->node_y;
		err_when(debugSreVX!=0 && debugSreVY!=0 && (abs(debugSreVX)!=abs(debugSreVY)));
	}

	err_when(debugSreNode1->node_x<0 || debugSreNode1->node_x>=MAX_WORLD_X_LOC ||
				debugSreNode1->node_y<0 || debugSreNode1->node_y>=MAX_WORLD_Y_LOC);
}

//-------------- function debug_check_magnitude() ---------------//
void SeekPathReuse::debug_check_magnitude(int x1, int y1, int x2, int y2)
{
	debugSreVX = x1-x2;
	debugSreVY = y1-y2;
	err_when(debugSreVX!=0 && debugSreVY!=0 && abs(debugSreVX)!=abs(debugSreVY));
}

//------------ function debug_check_smode_node() -------------//
void SeekPathReuse::debug_check_smode_node(int x, int y)
{
	if(reuse_search_sub_mode!=SEARCH_SUB_MODE_PASSABLE)
		return;

	Location *locPtr = world.get_loc(x, y);
	if(locPtr->power_nation_recno && !reuse_nation_passable[locPtr->power_nation_recno])
		err_here();
}

//------------- function debug_check_sub_mode_path() -------------//
void SeekPathReuse::debug_check_sub_mode_path(ResultNode *nodeArray, int count)
{
	ResultNode *debugSreNode1 = nodeArray;
	ResultNode *debugSreNode2 = debugSreNode1+1;
	int checkXLoc, checkYLoc, magn;
	for(int di=1; di<count; di++, debugSreNode1++, debugSreNode2++)
	{
		checkXLoc = debugSreNode1->node_x;
		checkYLoc = debugSreNode1->node_y;
		debugSreVX = debugSreNode2->node_x - debugSreNode1->node_x;
		debugSreVY = debugSreNode2->node_y - debugSreNode1->node_y;
		magn = (abs(debugSreVX) >= abs(debugSreVY)) ? abs(debugSreVX) : abs(debugSreVY);
		if(debugSreVX) debugSreVX /= abs(debugSreVX);
		if(debugSreVY) debugSreVY /= abs(debugSreVY);

		if(!magn)
			continue;

		for(int dj=0; dj<magn; dj++)
		{
			checkXLoc += debugSreVX;
			checkYLoc += debugSreVY;
			debug_check_smode_node(checkXLoc, checkYLoc);
		}
	}
}

#endif
