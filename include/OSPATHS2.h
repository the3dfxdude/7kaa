//Filename    : OSPATHS2.H
//Description : Header file of Object SeekPathS2
//Owner		  : Alex

#ifndef __OSPATHS2_H
#define __OSPATHS2_H

#ifndef __OSPATH_H
#include <OSPATH.h>
#endif

class NodeS2
{
public:
	short node_x, node_y;
	int   node_f, node_h;// could be replaced by "unsigned short" to reduce memory, set all member vars to "unsigned short" for consistency
	short node_g;
	char node_type;	// the type of the node, total 16 different type, 4 points in a 2x2 node, blocked/non-blocked, so there are 2^4 combinations
	char enter_direction;
	// enter_direction -- 1-8 for eight directions, 0 for the starting node
	//
	//		8	7	6
	//		1	x	5		where x is the reference point
	//		2	3	4

	NodeS2* parent_node;
	NodeS2* child_node[MAX_CHILD_NODE];
	NodeS2* next_node;

public:
	short generate_successors(short);//, short, short);
	short generate_succ(short x, short y, short direction, short cost);
	void propagate_down();
};

//------- Define struct ResultNode --------//

/*struct ResultNode
{
public:
	short node_x, node_y;
};*/

//---------- Define class StackS2 ----------//

struct StackS2
{
	NodeS2  *old_parent_node_ptr;
	NodeS2  *node_ptr;
	StackS2 *next_stack_ptr;
};

//--------- Define class SeekPathS2 --------//

class SeekPathS2
{
friend class NodeS2;

public:
	char			path_status;
	short			real_sour_x, real_sour_y;	// the actual coordinate of the starting point
	short			real_dest_x, real_dest_y;	// the actual coordinate of the destination point
	short			dest_x, dest_y;				// the coordinate of the destination represented in 2x2 node form
	char			is_dest_blocked;

	//char		source_locate_type;
	//char		dest_locate_type;

   short			border_x1, border_y1, border_x2, border_y2;
	//short			vir_border_x2, vir_border_y2;

	NodeS2*		open_node_list;
	NodeS2*		closed_node_list;

	short*		node_matrix;
	NodeS2*		node_array;

	int			max_node;
	int 			node_count;

	NodeS2*		result_node_ptr;

private:
	ResultNode* max_size_result_node_ptr;	// point to the temprory result node list
	ResultNode* parent_result_node_ptr;		// the parent node of the currently node pointed by max_size_result_node_ptr
	int			upper_left_x;	// x coord. of upper left corner of the 2x2 node
	int			upper_left_y;	// y coord. of upper left corner of the 2x2 node

public:
	SeekPathS2() 		{ node_array=NULL; }
	~SeekPathS2()		{ deinit(); }

	void			init(int maxNode);
	void			deinit();
	void			set_node_matrix(short reuseNodeMatrix[]);

	void			reset();
	inline void add_result_node(ResultNode** curPtr, ResultNode** prePtr, int& count);

	int			seek(int sx,int sy,int dx,int dy,DWORD groupId, char mobileType,short searchMode=1, short miscNo=0,int maxTries=0,int borderX1=0, int borderY1=0, int borderX2=MAX_WORLD_X_LOC-1, int borderY2=MAX_WORLD_Y_LOC-1);
	int			continue_seek(int,char=0);

	ResultNode* get_result(int& resultNodeCount, short& pathDist);
	NodeS2* 		return_closest_node();
	ResultNode* smooth_the_path(ResultNode* nodeArray, int& nodeCount); // smoothing the path

private:
	NodeS2*		return_best_node();
	void 			insert_open_node(NodeS2 *succNode);

	void			process_start_node(int xLoc, int yLoc, char nodeType, char exitDirection, int &nodeCount);
	void			process_end_node(int xLoc, int yLoc, char nodeType, char enterDirection, int &nodeCount);
	void			get_real_result_node(int &count, short enterDirection, short exitDirection, short nodeType, short xCoord, short yCoord);
	// function used to get the actual shortest path out of the 2x2 node path
};

extern SeekPathS2 seek_path_s2, seek_path_continue_s2;

//-----------------------------------------//

#endif

