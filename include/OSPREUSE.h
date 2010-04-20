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

//Filename    : OSPREUSE.H
//Description : Header file of Object SeekPathReuse
//Owner		  : Alex

#ifndef __OSPREUSE_H
#define __OSPREUSE_H

#ifndef	__OSPATH_H
#include <OSPATH.h>
#endif

#ifndef __ALL_H
#include <ALL.h>
#endif

#ifndef __OWORLD_H
#define <OWORLD.H>
#endif

//---------- Define constants ------------//
enum{	REUSE_PATH_INITIAL=1,			// 0 is used for other searching call with no path reuse, or for error checking
		REUSE_PATH_FIRST_SEEK,
		REUSE_PATH_SEARCH,
		//REUSE_PATH_FOUND,
		//REUSE_PATH_IMPOSSIBLE,
		REUSE_PATH_INCOMPLETE_SEARCH,
	 };

enum{	GENERAL_GROUP_MOVEMENT = 1,	// 0 is used for error checking
		FORMATION_MOVEMENT,
	 };

#define RNA_RESET_AMOUNT	10

//---------- Define class SeekPathReuse -----------//
class SeekPathReuse
{
	public:
		char			incomplete_search;

		static int				max_node;				// MAX number of node used in searching
		static short			total_num_of_path;	// equal to number of unit to process path-reuse
		static short			cur_path_num;			// which unit in the group is processing path-reuse
		static short			unit_size;				// the size of the current unit
		static DWORD			cur_group_id;			// group id used to pass into SeekPath for searching
		static char				mobile_type;			// mobile type used to pass into SeekPath for searching
		static char				unit_nation_recno;	// the nation recno of this unit
		static short			move_scale;				// 1 for UNIT_LAND, 2 for others
		static short			search_mode;			// search mode to check whether path-reuse is used
		static short			reuse_mode;				// reuse mode, general or formation movement of path-reuse
		static short			reuse_path_status;	// initial, seek first or reuse
		static short			reuse_path_dist;		// count the reuse-path distance
		static short			*reuse_node_matrix;	// point to the node matrix that store offset path for reusing

		static char				reuse_nation_passable[MAX_NATION+1]; // Note: position 0 is not used for faster access
		static char				reuse_search_sub_mode;

		//----------------- backup leader path(reference path) ----------------------//
		static short			leader_path_num;	//------------------------------------------------------------------------------------------------//
															// usually the unit calling with pathReuseStatus==REUSE_PATH_FIRST_SEEK should be the leader.
															// However, in some condition, e.g. leader_num_of_node<2, another unit will be selected
															// to be the leader. This variable stores which path number is selected to be the new leader path.
															//------------------------------------------------------------------------------------------------//
		static ResultNode		*reuse_leader_path_backup;
		static int				reuse_leader_path_node_num;
		static int				leader_path_start_x;
		static int				leader_path_start_y;
		static int				leader_path_dest_x;
		static int				leader_path_dest_y;

		//----------- decide which offset method is used ----------//
		// some will be removed later
		static int				start_x_offset;		// x offset in the starting location refer to that of leader unit
		static int				start_y_offset;		// y offset in the starting location refer to that of leader unit
		static int				dest_x_offset;			// x offset in the destination refer to that of leader unit
		static int				dest_y_offset;			// y offset in the destination refer to that of leader unit
		static int				x_offset;				// the x offset used in generating the offset path 
		static int				y_offset;				// the y offset used in generating the offset path 
		static int				formation_x_offset;	// formation x offset
		static int				formation_y_offset;	// formation y offset
		static int				start_x;					// the x location of the unit starting point
		static int				start_y;					// the y location of the unit starting point
		static int				dest_x;					// the x location of the unit destination
		static int				dest_y;					// the y location of the unit destination
		static int				vir_dest_x;
		static int				vir_dest_y;

		//---------- the current constructing result path --------//
		static ResultNode		*path_reuse_result_node_ptr;		// store the reuse path result node
		static int				num_of_result_node;					// store the number of result node of the reuse path
		static ResultNode		*cur_result_node_ptr;				// point to the current result node used in the result node array

		static short			result_node_array_def_size;		// the current node of node in the result node array
		static short			result_node_array_reset_amount;	// the default size to adjust in the result node array each time 

		//-- determine the current offset difference(leader path information) --//
		static ResultNode		*cur_leader_node_ptr;	// point to the leader result node backup array
		static int				cur_leader_node_num;		// current number of node used in the leader backup node array

		static short			leader_vec_x;			// the current unit vector in x-direction in the leader path
		static short			leader_vec_y;			// the current unit vector in y-direction in teh leader path

		//----------- for smoothing the result path --------------//
		static short			vec_x, vec_y;
		static short			new_vec_x, new_vec_y;
		static int				vec_magn, new_vec_magn;
		static int				result_vec_x, result_vec_y;

	public:

		void			init(int maxNode);
		void			deinit();
		void			init_reuse_search();		// init data structure
		void			deinit_reuse_search();	// deinit data structure
		void			init_reuse_node_matrix();
		void			set_index_in_node_matrix(int xLoc, int yLoc);

		int			seek(int sx,int sy,int dx,int dy,short unitSize, DWORD groupId,char mobileType, short searchMode=4, short miscNo=0,
							 short numOfPath=1, short pathReuseStatus=REUSE_PATH_INITIAL, short reuseMode=GENERAL_GROUP_MOVEMENT,
							 int maxTries=0,int borderX1=0, int borderY1=0, int borderX2=MAX_WORLD_X_LOC-1, int borderY2=MAX_WORLD_Y_LOC-1);
		ResultNode* get_result(int& resultNodeCount, short& pathDist);
		inline ResultNode* call_seek(int sx, int sy, int dx, int dy, DWORD groupId, char mobileType, short searchMode, int& nodeCount)
						{	seek_path.seek(sx, sy, dx, dy, groupId, mobileType, searchMode);
							return seek_path.get_result(nodeCount, reuse_path_dist);
						}
		inline ResultNode* call_seek2(int sx, int sy, int dx, int dy, DWORD groupId, char mobileType, short searchMode, int& nodeCount, int& returnValue);
		short			count_path_dist(ResultNode* nodeArray, int nodeNum);

		void			extend_result_path(short addSize=RNA_RESET_AMOUNT);
		void			add_result(int x, int y);	// record the result node in the node array
		void			add_result_path(ResultNode *pathPtr, int nodeNum);
		void			set_offset_condition(int startXOffset=0, int startYOffset=0, int destXOffset=0, int destYOffset=0);
		int			get_next_offset_loc(int& nextXLoc, int& nextYLoc);
		int			get_next_nonblocked_offset_loc(int& nextXLoc, int&nextYLoc);
		void			set_next_cur_path_num();

		void			seek_path_offset();	// process the offset method to get the shortest path
		void			seek_path_join_offset();
		void			use_offset_method(int xLoc, int yLoc);

		inline void	bound_check_x(int& x)
						{ if(x<0) x = 0; else if(x>=MAX_WORLD_X_LOC)	x = MAX_WORLD_X_LOC-move_scale; }
		inline void	bound_check_y(int& y)
						{ if(y<0) y = 0; else if(y>=MAX_WORLD_X_LOC)	y = MAX_WORLD_X_LOC-move_scale; }
		char			get_reuse_path_status();

		void			set_nation_passable(char nationPassable[]);
		void			set_sub_mode(char subMode=SEARCH_SUB_MODE_NORMAL);
		void			set_status(char newStatus);

		//-------------- for optimize the result path ----------------//
		void			remove_duplicate_node(ResultNode* resultList, int& nodeCount);	// remove duplicate node in the node array
		ResultNode*	smooth_reuse_path(ResultNode* resultPath, int& resultNodeNum);	// smoothing the reuse path
		ResultNode*	smooth_path(ResultNode* resultPath, int& resultNodeNum);			// smooth any path, subset of smooth_reuse_path()
		ResultNode*	smooth_path2(ResultNode* resultPath, int& resultNodeNum);			// version for UNIT_SEA, UNIT_AIR

		//-------------- for node limitation -----------------//
		int			is_node_avail_empty();
		int			is_leader_path_valid();
		void			copy_leader_path_offset();
		void			move_within_map(int preX, int preY, int curX, int curY);
		void			move_inside_map(int preX, int preY, int curX, int curY);
		void			move_outside_map(int preX, int preY, int curX, int curY);
		void			move_beyond_map(int preX, int preY, int curX, int curY);

	public:
		static int	can_walk(int xLoc, int yLoc);
		static int	can_walk_s2(int xLoc, int yLoc);
		static void	sys_yield();

		#ifdef DEBUG
		void			debug_check();
		void			debug_check_magnitude(int x1, int y1, int x2, int y2);
		void			debug_check_smode_node(int x, int y);
		void			debug_check_sub_mode_path(ResultNode *nodeArray, int count);
		#endif

		#ifdef DEBUG
		#define debug_reuse_check_path()								debug_check()
		#define debug_reuse_check_xy_magn(x1, y1, x2, y2)		debug_check_magnitude((x1), (y1), (x2), (y2))
		#define debug_reuse_check_sub_mode_node(x, y)			debug_check_smode_node((x), (y))
		#define debug_reuse_check_sub_mode(nodeArray, count)	debug_check_sub_mode_path((nodeArray), (count))
		#else
		#define debug_reuse_check_path()
		#define debug_reuse_check_xy_magn(x1, y1, x2, y2)
		#define debug_reuse_check_sub_mode_node(x, y)
		#define debug_reuse_check_sub_mode(nodeArray, count)
		#endif

	protected:
	private:
};

extern SeekPathReuse seek_path_reuse;

#endif