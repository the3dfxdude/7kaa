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

#include <OSPATH.h>

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

NodePriorityQueue SeekPath::open_node_list;
NodePriorityQueue SeekPath::closed_node_list;

//-------- Begin of function NodePriorityQueue::reset_priority_queue -------//
void NodePriorityQueue::reset_priority_queue()
{
	size = 0U;
	memset(elements, 0, sizeof(Node*)*(MAX_ARRAY_SIZE));
}
//-------- End of function NodePriorityQueue::reset_priority_queue ---------//


//-------- Begin of function NodePriorityQueue::insert_node -------//
void NodePriorityQueue::insert_node(Node *insertNode)
{
	unsigned int i = ++size;
	register int f=insertNode->node_f;
	Node **localElements = elements;
	
	while(i>1 && localElements[i/2]->node_f > f)
	{
		localElements[i] = localElements[i/2];
		i /= 2;
	}

	localElements[i] = insertNode;
}
//-------- End of function NodePriorityQueue::insert_node ---------//


//-------- Begin of function NodePriorityQueue::return_min -------//
Node* NodePriorityQueue::return_min()
{
	if(!size)
		return NULL;

	unsigned int i, child, doubleI;
	unsigned int localSize = size--;

	Node **localElements = elements;
	Node *minElement = localElements[1];
	Node *lastElement = localElements[localSize];
	int	lastF = lastElement->node_f;

	//--------- doubleI = i*2 ---------//
	for(i=1, doubleI=2; doubleI<=localSize; i=child, doubleI=i<<1)
	{
		child = doubleI;
		if(child!=localSize && localElements[child+1]->node_f < localElements[child]->node_f)
			child++;

		if(lastF > localElements[child]->node_f)
			localElements[i] = localElements[child];
		else
			break;
	}

	localElements[i] = lastElement;

	return minElement;
}
//-------- End of function NodePriorityQueue::return_min ---------//


//-------- Begin of function SeekPath::return_best_node -------//
// return : <Node*> the best node.
//						  NULL if no more node on the open node list. There is no
//						  possible path between the starting and destination points.
//
Node* SeekPath::return_best_node()
{
	Node *tempNode = open_node_list.return_min();
	
	if(tempNode)
		closed_node_list.insert_node(tempNode);

	return tempNode;
}
//-------- End of function SeekPath::return_best_node ---------//


//----- Begin of function SeekPath::return_closest_node -------//
// Return the node that is closest to the destination.
//
Node* SeekPath::return_closest_node()
{
	Node	*nodePtr;
	Node	*shortestNode;
	int	shortestDistance=0x7FFFFFFF;

	shortestNode = open_node_list.return_min();
	if(shortestNode)
		shortestDistance = shortestNode->node_h;
	
	nodePtr = closed_node_list.return_min();
	if(nodePtr && nodePtr->node_h<shortestDistance)
	{
		shortestNode = nodePtr;
		shortestDistance = nodePtr->node_h;
	}

	//--------------------------------------------------------------------------------//
	// there may be some nodes with the same shortest Distance from the destination node.
	// However, one should be closer to the starting node.  The following is used to find
	// out the closer node.
	//--------------------------------------------------------------------------------//
	if(!shortestNode)
		return NULL;

	nodePtr = shortestNode->parent_node;
	while(nodePtr)
	{
		if(nodePtr->node_h <= shortestDistance)
		{
			shortestDistance = nodePtr->node_h;
			shortestNode 	  = nodePtr;
		}
		else
			break;

		nodePtr=nodePtr->parent_node;
	}

	return shortestNode;
}
//----- End of function SeekPath::return_closest_node -------//
