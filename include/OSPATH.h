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

//Filename    : OSPATH.H
//Description : Header file of Object SeekPath
//Owner		  : Alex

#ifndef __OSPATH_H
#define __OSPATH_H

#ifndef __ALL_H
#include <ALL.h>
#endif

#ifndef __OVGA_H
#include <OVGA.h>
#endif

#ifndef __OWORLD_H
#include <OWORLD.h>
#endif

//---------- Define constants ------------//

enum { PATH_WAIT,				// Wait for path seeking orders
		 PATH_SEEKING,			// Seeking in process
		 PATH_FOUND,			// A path is found
		 PATH_NODE_USED_UP,	// All nodes have used, but still unable to find the destination
		 PATH_IMPOSSIBLE,		// impossible to move, no result at all
 		 PATH_REUSE_FOUND,	// a reuse path is found
	  };

//---------define the search mode------------//
enum	{	SEARCH_MODE_IN_A_GROUP = 1,		// general searching for a unit
			SEARCH_MODE_A_UNIT_IN_GROUP,		// general group searching
			SEARCH_MODE_TO_ATTACK,				// general attacking searching
			SEARCH_MODE_REUSE,					// reuse path searching
			SEARCH_MODE_BLOCKING,				// searching when blocking (mainly for 2x2 units)
			SEARCH_MODE_TO_VEHICLE,				// embarking searching
			
			//=================================================================//
			// for the following search mode, destination location may be not
			// walkable, group id together for speeding up chekcing in function
			// can_move_to()
			//-----------------------------------------------------------------//
			SEARCH_MODE_TO_FIRM,
			SEARCH_MODE_TO_TOWN,
			SEARCH_MODE_TO_WALL_FOR_GROUP,
			SEARCH_MODE_TO_WALL_FOR_UNIT,
			SEARCH_MODE_ATTACK_UNIT_BY_RANGE, // searching to attack by range attack (esp. for attacking target with different mobile_type from this unit)
			SEARCH_MODE_ATTACK_FIRM_BY_RANGE,
			SEARCH_MODE_ATTACK_TOWN_BY_RANGE,
			SEARCH_MODE_ATTACK_WALL_BY_RANGE,
			//=================================================================//
			
			SEARCH_MODE_TO_LAND_FOR_SHIP,		// for ship only
			SEARCH_MODE_LAST, //--- set to the last one
			MAX_SEARCH_MODE_TYPE = SEARCH_MODE_LAST-1,
		};

//--------------------- define sub-mode -----------------//
enum	{	SEARCH_SUB_MODE_NORMAL=0,
			SEARCH_SUB_MODE_PASSABLE,
		};

//----------- Define constants -----------//

#define MAX_BACKGROUND_NODE				2400		// the total no. of nodes can be used at a time
#define VALID_BACKGROUND_SEARCH_NODE	1600		// the search is considered unsuccessful if the node used > this value
#define MIN_BACKGROUND_NODE_USED_UP		400		// don't do any new search if the current available nodes is < this value

#define MAX_CHILD_NODE    8		// one for each direction
//#define MAX_STACK_NUM  2000		// maximum no. of stack entity in stack_aray, which is shared by all SeekPath objects. It is calculated based on: MAX possiblity: 500(MAX node) x 8(MAX child node) = 4000. Practical no. possiblities=2000. Memory occupied: 2000*4 = 8K
#define MAX_STACK_NUM  MAX_BACKGROUND_NODE	// maximum no. of stack entity in stack_aray, which is shared by all SeekPath objects. It is calculated based on: MAX possiblity: 500(MAX node) x 8(MAX child node) = 4000. Practical no. possiblities=2000. Memory occupied: 2000*4 = 8K

//---------- Define class Node -----------//

class Node
{
public:
	short node_x, node_y;
	int   node_f, node_h;// could be replaced by "unsigned short" to reduce memory, set all member vars to "unsigned short" for consistency
	short	node_g;
	char	node_type;// the type of the node, total 16 different type, 4 points in a 2x2 node, blocked/non-blocked, so there are 2^4 combinations
	char	enter_direction;
	// enter_direction -- 1-8 for eight directions, 0 for the starting node
	//
	//		8	7	6
	//		1	x	5		where x is the reference point
	//		2	3	4

	Node* parent_node;
	Node* child_node[MAX_CHILD_NODE];
	Node* next_node;

public:
	short generate_successors(short dir, short x , short y);
	short generate_succ(short x, short y, short direction, short cost);
	void	propagate_down();

	//------------- for scale 2 --------------//
	short generate_successors2(short x, short y);
	short generate_succ2(short x, short y, short cost=1);
	void	propagate_down2();
};

//------- Define struct ResultNode --------//

#pragma pack(1)
struct ResultNode
{
public:
	short node_x, node_y;
};
#pragma pack()

//---------- Define class Stack ----------//

struct Stack
{
	Node  *node_ptr;
	Stack *next_stack_ptr;
};

//---------- define struct NodePriorityQueue --------//

struct NodePriorityQueue
{
	#define MAX_ARRAY_SIZE	MAX_BACKGROUND_NODE+1
	public:
		UINT	size;
		Node	*elements[MAX_ARRAY_SIZE];

	public:
		void	reset_priority_queue();
		void	insert_node(Node *insertNode);
		Node*	return_min();
};

//--------- Define class SeekPath --------//

class SeekPath
{
friend class Node;

public:
	char	path_status;
	short real_sour_x, real_sour_y;	// the actual coordinate of the starting point
	short real_dest_x, real_dest_y;	// the actual coordinate of the destination point
	short	dest_x, dest_y;				// the coordinate of the destination represented in 2x2 node form
	char	is_dest_blocked;
	short	current_search_node_used;	// count the number of nodes used in the current searching

   short border_x1, border_y1, border_x2, border_y2;

	static NodePriorityQueue	open_node_list;
	static NodePriorityQueue	closed_node_list;

	short* node_matrix;
	Node*  node_array;

	int	max_node;
	int 	node_count;

	Node*	result_node_ptr;

	short	total_node_avail;
	void	reset_total_node_avail();

private:
	ResultNode* max_size_result_node_ptr;	// point to the temprory result node list
	ResultNode* parent_result_node_ptr;		// the parent node of the currently node pointed by max_size_result_node_ptr
	int	upper_left_x;	// x coord. of upper left corner of the 2x2 node
	int	upper_left_y;	// y coord. of upper left corner of the 2x2 node

public:
	SeekPath() 		{ node_array=NULL; node_matrix=NULL; }
	~SeekPath()		{ deinit(); }

	void  init(int maxNode);
	void  deinit();

	void	set_node_matrix(short reuseNodeMatrix[]);
	void	set_status(char newStatus) { path_status = newStatus; }
	int	is_valid_searching()	{ return total_node_avail>VALID_BACKGROUND_SEARCH_NODE; }

	void	reset();
	inline void add_result_node(int x, int y, ResultNode** curPtr, ResultNode** prePtr, int& count);
	int   seek(int sx,int sy,int dx,int dy,DWORD groupId,char mobileType, short searchMode=SEARCH_MODE_IN_A_GROUP, short miscNo=0, short numOfPath=1, int maxTries=0,int borderX1=0, int borderY1=0, int borderX2=MAX_WORLD_X_LOC-1, int borderY2=MAX_WORLD_Y_LOC-1);
	int   continue_seek(int,char=0);

	ResultNode* get_result(int& resultNodeCount, short& pathDist);
	Node* 		return_closest_node();
	ResultNode* smooth_the_path(ResultNode* nodeArray, int& nodeCount); // smoothing the path

	//------------ for scale 2 -------------//
	int	seek2(int sx,int sy,int dx,int dy,short miscNo,short numOfPath,int maxTries);
	int   continue_seek2(int,char=0);
	ResultNode* get_result2(int& resultNodeCount, short& pathDist);

	void	set_attack_range_para(int attackRange);
	void	reset_attack_range_para();
	void	set_nation_recno(char nationRecno);
	void	set_nation_passable(char nationPassable[]);
	void	set_sub_mode(char subMode=SEARCH_SUB_MODE_NORMAL);

   int   write_file(File* filePtr);
   int   read_file(File* filePtr);

private:
	static Node* return_best_node();

	void	get_real_result_node(int &count, short enterDirection, short exitDirection, short nodeType, short xCoord, short yCoord);
	// function used to get the actual shortest path out of the 2x2 node path

	inline void		bound_check_x(short &paraX);
	inline void		bound_check_y(short &paraY);
	inline short	result_node_distance(ResultNode *node1, ResultNode *node2);
};

extern SeekPath seek_path;
//### begin alex 29/9 ###//
#ifdef DEBUG
extern unsigned long seek_path_profile_time;
extern unsigned long last_seek_path_profile_time;
#endif
//#### end alex 29/9 ####//

//-----------------------------------------//

#endif

