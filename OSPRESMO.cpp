#include <OSPREUSE.h>
#include <OUNIT.h>

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
#undef debug_reuse_check_path
#undef debug_reuse_check_xy_magn
#define debug_reuse_check_path()
#define debug_reuse_check_xy_magn(x1, y1, x2, y2)
#undef DEBUG
#endif

//------ Begin of function SeekPathReuse::remove_duplicate_node ---------//
// this function is used to remove duplicate node
//
void SeekPathReuse::remove_duplicate_node(ResultNode resultList[], int& nodeCount)
{
	//------------------------------------------------------------//		
	// remove duplicate node 
	//------------------------------------------------------------//		
	ResultNode* curNodePtr = resultList+1;
	ResultNode* preNodePtr = resultList;
	ResultNode* writeNodePtr = resultList;
	int count = 0;

	err_when(mobile_type!=UNIT_LAND && (preNodePtr->node_x%2 || preNodePtr->node_y%2));
	for(int i=1; i<nodeCount; i++)
	{
		err_when(mobile_type!=UNIT_LAND && (curNodePtr->node_x%2 || curNodePtr->node_y%2));
		
		if(i%40==0)
			sys_yield(); // update cursor position
		
		if(curNodePtr->node_x!=preNodePtr->node_x || curNodePtr->node_y!=preNodePtr->node_y)
		{
			writeNodePtr->node_x = preNodePtr->node_x;
			writeNodePtr->node_y = preNodePtr->node_y;

			#ifdef DEBUG
				ResultNode *debugPtr;
				int vX, vY;
				if(i>1)
				{
					debugPtr = writeNodePtr;
					vX = debugPtr->node_x - writeNodePtr->node_x;
					vY = debugPtr->node_y - writeNodePtr->node_y;
				}
				err_when(i>1 && vX!=0 && vY!=0 && abs(vX)!=abs(vY));
			#endif

			writeNodePtr++;
			preNodePtr = curNodePtr;
			curNodePtr++;
			count++;

		}
		else
			curNodePtr++;
	}
	writeNodePtr->node_x = preNodePtr->node_x;
	writeNodePtr->node_y = preNodePtr->node_y;
	count++;
	nodeCount = count;
}
//-------- End of function SeekPathReuse::remove_duplicate_node ---------//


//-------- Begin of function SeekPathReuse::smooth_reuse_path ---------//
ResultNode* SeekPathReuse::smooth_reuse_path(ResultNode* resultPath, int& resultNodeNum)
{
	//------------------------------------------------------------//		
	// to handle all the unexpected case in turning direction or
	// reverse direction in reuse path. Finally, a well-shaped
	// path will be returned.
	//------------------------------------------------------------//

	//-----------------------------------------------------------//
	// remove duplicate node
	//-----------------------------------------------------------//
	remove_duplicate_node(resultPath, resultNodeNum);
	if(mobile_type==UNIT_LAND)
		resultPath = smooth_path(resultPath, resultNodeNum);
	else
		resultPath = smooth_path2(resultPath, resultNodeNum);

	return resultPath;
}
//--------- End of function SeekPathReuse::smooth_reuse_path ---------//


//-------- Begin of function SeekPathReuse::smooth_path ---------//
ResultNode*	SeekPathReuse::smooth_path(ResultNode* resultPath, int& resultNodeNum)
{
	err_when(mobile_type!=UNIT_LAND);
	#define UNKNOWN_FACTOR	5

 	//----------------------------------------------------------------------------//
 	// to handle all the unexpected case in turning direction or reverse direction
	// in reuse path. Finally, a well-shaped path will be returned.
 	//----------------------------------------------------------------------------//
 	ResultNode	*preNodePtr;
 	ResultNode	*writeNodePtr;
 	ResultNode	*curNodePtr;
 	ResultNode	*oldPreNodePtr;
	ResultNode	*wellShapedNodePtr;
 	short			changed = 1;
 	int			count = 0, i;
	//int			resultVectorX, resultVectorY;
	int			tempXLoc, tempYLoc;
	int			necessaryPoint, type;
 
 	//------------------------------------------------------------//
 	// smoothing the path 
 	//------------------------------------------------------------//
 	while(changed)
 	{
 		oldPreNodePtr = resultPath;	//	may point to writeNodePtr or tempReuseResultPtr list
 		preNodePtr = resultPath+1;		//	may point to writeNodePtr or tempReuseResultPtr list
 		curNodePtr = resultPath+2;		// always point to the tempReuseResultPtr list
 		count = 0;
 		changed = 0;
 
 		if(resultNodeNum>2)
 		{
 			wellShapedNodePtr = (ResultNode*)mem_add(resultNodeNum*UNKNOWN_FACTOR*sizeof(ResultNode*)); //********BUGHERE
 			memset(wellShapedNodePtr, 0, resultNodeNum*UNKNOWN_FACTOR*sizeof(ResultNode*)); // num of ResultNode may not be enough for use
 				
 			writeNodePtr = wellShapedNodePtr; //---------- write the beginning node ---------//
			*writeNodePtr = *oldPreNodePtr;
 			count++;
 
 			i = 2;
 			do
 			{
 				//-------------------------------------------------//
 				// calculate the vector offset and vector magnitude
 				//-------------------------------------------------//
 				vec_x = preNodePtr->node_x-oldPreNodePtr->node_x;
 				vec_y = preNodePtr->node_y-oldPreNodePtr->node_y;
 				err_when(vec_x==0 && vec_y==0);
 				vec_magn = (vec_x!=0) ? abs(vec_x) : abs(vec_y);
 				vec_x /= vec_magn;
 				vec_y /= vec_magn;
 				
 				new_vec_x = curNodePtr->node_x - preNodePtr->node_x;
 				new_vec_y = curNodePtr->node_y - preNodePtr->node_y;
 				err_when(new_vec_x==0 && new_vec_y==0);
 				new_vec_magn = (new_vec_x!=0)?abs(new_vec_x):abs(new_vec_y);
 				new_vec_x /= new_vec_magn;
 				new_vec_y /= new_vec_magn;
 				
 				//----------------------------------------------//
 				// (1) same direction 
 				//----------------------------------------------//
 				if(vec_x==new_vec_x && vec_y==new_vec_y)
 				{
 					preNodePtr = curNodePtr;
 					curNodePtr++;
 
 					err_when(oldPreNodePtr->node_x==preNodePtr->node_x && oldPreNodePtr->node_y==preNodePtr->node_y);
 					continue;
 				}
 
 				//----------------------------------------------//
 				// (2) reverse direction
 				//----------------------------------------------//
 				if(vec_x==-new_vec_x && vec_y==-new_vec_y) 
				{
 					//--- old vector magnitude != new vector magnitude ----//
 					if(vec_magn!=new_vec_magn)
 					{
 						preNodePtr = curNodePtr;
 						curNodePtr++;
 						err_when(i+1!=resultNodeNum && preNodePtr->node_x==curNodePtr->node_x && preNodePtr->node_y==curNodePtr->node_y);
 					}
 					else if(i+2<resultNodeNum)	//------ not the end of the array ------//
 					{
 						//------------ both magnitudes are equal ----------//
						oldPreNodePtr = curNodePtr;
						preNodePtr = oldPreNodePtr+1;
 						curNodePtr = preNodePtr+1;
 						changed++;
 						i++;
 					}
 					else if(i+1<resultNodeNum)
 					{
 						preNodePtr = curNodePtr+1;	// for writing the last node
 						changed++;
 						i++;
 					}
 					else
 						break;
 					err_when(oldPreNodePtr->node_x==preNodePtr->node_x && oldPreNodePtr->node_y==preNodePtr->node_y);
 					continue;
 				}
 
				result_vec_x = vec_x+new_vec_x;
 				result_vec_y = vec_y+new_vec_y;
				necessaryPoint = 0;
 				//----------------------------------------------//
 				// (3) turn 45 degree
 				//----------------------------------------------//
				if(abs(result_vec_x)<=move_scale && abs(result_vec_y)<=move_scale && (result_vec_x==0 || result_vec_y==0))
				{
					err_when(result_vec_x>=2*move_scale || result_vec_y>=2*move_scale);
					if((vec_x==0 || vec_y==0) && vec_magn>move_scale)
					{
						//--------- choose the farther location if possible --------------//
						tempXLoc = preNodePtr->node_x - 2*vec_x;
						tempYLoc = preNodePtr->node_y - 2*vec_y;
						if(vec_magn>2*move_scale) // add a new point
						{
							oldPreNodePtr->node_x = tempXLoc;
							oldPreNodePtr->node_y = tempYLoc;
							debug_reuse_check_xy_magn(oldPreNodePtr->node_x, oldPreNodePtr->node_y, writeNodePtr->node_x, writeNodePtr->node_y);//**** debug checking
 							writeNodePtr++;
 							writeNodePtr->node_x = oldPreNodePtr->node_x;
 							writeNodePtr->node_y = oldPreNodePtr->node_y;
 							count++;
						}//else vec_magn==2*move_scale, do nothing
					}
					else
					{
						//------------------------------------------------------------------------------------//
						// 1) the vector direction is not horizontal or vertical (only one step is checked), or
						// 2) the direction is matched but vec_magn==move_scale
						//------------------------------------------------------------------------------------//
						if(vec_magn>move_scale)
						{
							oldPreNodePtr->node_x = preNodePtr->node_x-vec_x;
							oldPreNodePtr->node_y = preNodePtr->node_y-vec_y;
							debug_reuse_check_xy_magn(oldPreNodePtr->node_x, oldPreNodePtr->node_y, writeNodePtr->node_x, writeNodePtr->node_y);//**** debug checking
 							writeNodePtr++;
 							writeNodePtr->node_x = oldPreNodePtr->node_x;
 							writeNodePtr->node_y = oldPreNodePtr->node_y;
 							count++;
						}
					}

					if(new_vec_magn>move_scale)
					{
						preNodePtr->node_x += new_vec_x;
						preNodePtr->node_y += new_vec_y;
 						debug_reuse_check_xy_magn(preNodePtr->node_x, preNodePtr->node_y, writeNodePtr->node_x, writeNodePtr->node_y);//**** debug checking
 						writeNodePtr++;
 						writeNodePtr->node_x = preNodePtr->node_x;
 						writeNodePtr->node_y = preNodePtr->node_y;
 						count++;
						oldPreNodePtr = preNodePtr;
					}

					preNodePtr = curNodePtr;
					curNodePtr++;
					changed++;
					continue;
				}

				//----------------------------------------------//
 				// (4) turn 90 degree
 				//----------------------------------------------//
				if(vec_x*new_vec_x+vec_y*new_vec_y==0)
				{
					type = (vec_x==0 || vec_y==0) ? 0 : 1; // 0 for + case, 1 for x case

					if(type==1) // x case
					{
						err_when(new_vec_x-vec_x!=0 && new_vec_y-vec_y!=0);
						tempXLoc = preNodePtr->node_x+(new_vec_x-vec_x)/2;
						tempYLoc = preNodePtr->node_y+(new_vec_y-vec_y)/2;
						if(can_walk(tempXLoc, tempYLoc))
						{
							if(vec_magn>move_scale)
							{
 								oldPreNodePtr->node_x = preNodePtr->node_x-vec_x;
 								oldPreNodePtr->node_y = preNodePtr->node_y-vec_y;
 								debug_reuse_check_xy_magn(oldPreNodePtr->node_x, oldPreNodePtr->node_y, writeNodePtr->node_x, writeNodePtr->node_y);//**** debug checking
 								writeNodePtr++;
 								writeNodePtr->node_x = oldPreNodePtr->node_x;
 								writeNodePtr->node_y = oldPreNodePtr->node_y;
 								count++;
							}

							if(new_vec_magn>move_scale)
							{
 								oldPreNodePtr->node_x = preNodePtr->node_x+new_vec_x;
 								oldPreNodePtr->node_y = preNodePtr->node_y+new_vec_y;
 								err_when(oldPreNodePtr->node_x==curNodePtr->node_x && oldPreNodePtr->node_y==curNodePtr->node_y);
 								err_when(oldPreNodePtr->node_x==preNodePtr->node_x && oldPreNodePtr->node_y==preNodePtr->node_y);
 								debug_reuse_check_xy_magn(oldPreNodePtr->node_x, oldPreNodePtr->node_y, writeNodePtr->node_x, writeNodePtr->node_y);//**** debug checking
  								writeNodePtr++;
 								writeNodePtr->node_x = oldPreNodePtr->node_x;
 								writeNodePtr->node_y = oldPreNodePtr->node_y;
 								count++;
							}

							preNodePtr = curNodePtr;
							curNodePtr++;
						}
						else
							necessaryPoint = 1;
					}
					else // + case
					{
						if(vec_magn>move_scale && new_vec_magn>move_scale)
						{
							tempXLoc = preNodePtr->node_x+new_vec_x-vec_x;
							tempYLoc = preNodePtr->node_y+new_vec_y-vec_y;
							if(can_walk(tempXLoc, tempYLoc))
							{
								if(vec_magn>2*move_scale)
								{
									oldPreNodePtr->node_x = preNodePtr->node_x-2*vec_x;
									oldPreNodePtr->node_y = preNodePtr->node_y-2*vec_y;
 									debug_reuse_check_xy_magn(oldPreNodePtr->node_x, oldPreNodePtr->node_y, writeNodePtr->node_x, writeNodePtr->node_y);//**** debug checking
 									writeNodePtr++;
 									writeNodePtr->node_x = oldPreNodePtr->node_x;
 									writeNodePtr->node_y = oldPreNodePtr->node_y;
 									count++;
								}

								if(new_vec_magn>2*move_scale)
								{
									oldPreNodePtr->node_x = preNodePtr->node_x+2*new_vec_x;
									oldPreNodePtr->node_y = preNodePtr->node_y+2*new_vec_y;
 									debug_reuse_check_xy_magn(oldPreNodePtr->node_x, oldPreNodePtr->node_y, writeNodePtr->node_x, writeNodePtr->node_y);//**** debug checking
 									writeNodePtr++;
 									writeNodePtr->node_x = oldPreNodePtr->node_x;
 									writeNodePtr->node_y = oldPreNodePtr->node_y;
 									count++;
								}

								preNodePtr = curNodePtr;
								curNodePtr++;
							}
							else
								necessaryPoint = 1;
						}
						else // not both vector magnitude>move_scale
						{
							if(vec_magn>move_scale)
							{
								oldPreNodePtr->node_x = preNodePtr->node_x-vec_x;
								oldPreNodePtr->node_y = preNodePtr->node_y-vec_y;
 								debug_reuse_check_xy_magn(oldPreNodePtr->node_x, oldPreNodePtr->node_y, writeNodePtr->node_x, writeNodePtr->node_y);//**** debug checking
 								writeNodePtr++;
 								writeNodePtr->node_x = oldPreNodePtr->node_x;
 								writeNodePtr->node_y = oldPreNodePtr->node_y;
 								count++;
							}

							if(new_vec_magn>move_scale)
							{
								preNodePtr->node_x += new_vec_x;
								preNodePtr->node_y += new_vec_y;
 								debug_reuse_check_xy_magn(preNodePtr->node_x, preNodePtr->node_y, writeNodePtr->node_x, writeNodePtr->node_y);//**** debug checking
 								writeNodePtr++;
 								writeNodePtr->node_x = preNodePtr->node_x;
 								writeNodePtr->node_y = preNodePtr->node_y;
 								count++;
								oldPreNodePtr = preNodePtr;
							}

							preNodePtr = curNodePtr;
							curNodePtr++;
						}
					}

					if(!necessaryPoint)
					{
						changed++;
						continue;
					}
				}
				
 				//--------------------------------------------------//
 				// (5) this point is necessary for the reuse-path
 				//--------------------------------------------------//
 				debug_reuse_check_xy_magn(preNodePtr->node_x, preNodePtr->node_y, writeNodePtr->node_x, writeNodePtr->node_y);//**** debug checking
 				writeNodePtr++;
 				writeNodePtr->node_x = preNodePtr->node_x;
 				writeNodePtr->node_y = preNodePtr->node_y;
 				count++;
 				oldPreNodePtr = preNodePtr;
 				preNodePtr = curNodePtr;
 				curNodePtr++;
 				err_when(oldPreNodePtr->node_x==preNodePtr->node_x && oldPreNodePtr->node_y==preNodePtr->node_y);
 			}while(++i<resultNodeNum);
 			
 			debug_reuse_check_xy_magn(preNodePtr->node_x, preNodePtr->node_y, writeNodePtr->node_x, writeNodePtr->node_y);//**** debug checking
 			
 			writeNodePtr++; //------- write the end node -----------//
 			writeNodePtr->node_x = preNodePtr->node_x;
 			writeNodePtr->node_y = preNodePtr->node_y;
 			count++;
 			resultNodeNum = count;
 			
 			mem_del(resultPath);
 			resultPath = wellShapedNodePtr;
 			wellShapedNodePtr = NULL;
 		}
	}

	return resultPath;
}
//--------- End of function SeekPathReuse::smooth_path ---------//


//-------- Begin of function SeekPathReuse::smooth_path2 ---------//
// another version of smooth_path().  This version is for unit's
// mobile_type == UNIT_SEA or UNIT_AIR
//
ResultNode*	SeekPathReuse::smooth_path2(ResultNode* resultPath, int& resultNodeNum)
{
	err_when(mobile_type!=UNIT_SEA && mobile_type!=UNIT_AIR);
	#define UNKNOWN_FACTOR	5

	#ifdef DEBUG
		//------------------------------------------------------------------------//
		// used to debug for the path of UNIT_SEA
		//------------------------------------------------------------------------//
		if(mobile_type==UNIT_SEA && resultNodeNum>1)
		{
			ResultNode *debugPtr1 = resultPath;
			ResultNode *debugPtr2 = resultPath+1;
			int di=1, dj, dvecX, dvecY, magn;
			while(di<resultNodeNum)
			{
				dvecX = debugPtr2->node_x-debugPtr1->node_x;
				dvecY = debugPtr2->node_y-debugPtr1->node_y;
				magn = (abs(dvecX) > abs(dvecY)) ? abs(dvecX) : abs(dvecY);
				dvecX /= magn;
				dvecY /= magn;
				for(dj=1; dj<=magn; dj++)
					if(mobile_type==UNIT_SEA)
						err_when(!world.get_loc(debugPtr1->node_x+dvecX*dj, debugPtr1->node_y+dvecY*dj)->sailable());
				
				di++;
				debugPtr1++;
				debugPtr2++;
			}
		}
	#endif
	
	//----------------------------------------------------------------------------//
 	// to handle all the unexpected case in turning direction or reverse direction
	// in reuse path. Finally, a well-shaped path will be returned.
 	//----------------------------------------------------------------------------//
 	ResultNode	*preNodePtr;
 	ResultNode	*writeNodePtr;
 	ResultNode	*curNodePtr;
 	ResultNode	*oldPreNodePtr;
	ResultNode	*wellShapedNodePtr;
 	short			changed = 1;
 	int			count = 0, i;
	int			tempXLoc, tempYLoc, tempVectorX, tempVectorY;
	int			necessaryPoint, type;
 
 	//------------------------------------------------------------//		
 	// smoothing the path 
 	//------------------------------------------------------------//		
 	while(changed)
 	{
 		oldPreNodePtr = resultPath;	//	may point to writeNodePtr or tempReuseResultPtr list
 		preNodePtr = resultPath+1;		//	may point to writeNodePtr or tempReuseResultPtr list
 		curNodePtr = resultPath+2;		// always point to the tempReuseResultPtr list
 		count = 0;
 		changed = 0;

 		if(resultNodeNum>2)
 		{
 			wellShapedNodePtr = (ResultNode*)mem_add(resultNodeNum*UNKNOWN_FACTOR*sizeof(ResultNode*)); //********BUGHERE
 			memset(wellShapedNodePtr, 0, resultNodeNum*UNKNOWN_FACTOR*sizeof(ResultNode*)); // num of ResultNode may not be enough for use
 				
 			writeNodePtr = wellShapedNodePtr; //---------- write the beginning node ---------//
 			writeNodePtr->node_x = oldPreNodePtr->node_x;
 			writeNodePtr->node_y = oldPreNodePtr->node_y;
 			count++;
 
 			i = 2;
 			do
 			{
 				//-------------------------------------------------//
 				// calculate the vector offset and vector magnitude
 				//-------------------------------------------------//
 				vec_x = preNodePtr->node_x-oldPreNodePtr->node_x;
 				vec_y = preNodePtr->node_y-oldPreNodePtr->node_y;
 				err_when(vec_x==0 && vec_y==0);
 				vec_magn = (vec_x!=0) ? abs(vec_x) : abs(vec_y);
 				(vec_x /= vec_magn) *= move_scale;
 				(vec_y /= vec_magn) *= move_scale;
 				
 				new_vec_x = curNodePtr->node_x-preNodePtr->node_x;
 				new_vec_y = curNodePtr->node_y-preNodePtr->node_y;
 				err_when(new_vec_x==0 && new_vec_y==0);
 				new_vec_magn = (new_vec_x!=0) ? abs(new_vec_x) : abs(new_vec_y);
 				(new_vec_x /= new_vec_magn) *= move_scale;
 				(new_vec_y /= new_vec_magn) *= move_scale;
 				
 				//----------------------------------------------//
 				// (1) same direction 
 				//----------------------------------------------//
 				if(vec_x==new_vec_x && vec_y==new_vec_y)
 				{
 					preNodePtr = curNodePtr;
 					curNodePtr++;
 
 					err_when(oldPreNodePtr->node_x==preNodePtr->node_x && oldPreNodePtr->node_y==preNodePtr->node_y);
 					continue;
 				}
 
 				//----------------------------------------------//
 				// (2) reverse direction
 				//----------------------------------------------//
 				if(vec_x==-new_vec_x && vec_y==-new_vec_y) 
 				{
 					//--- old vector magnitude != new vector magnitude ----//
 					if(vec_magn!=new_vec_magn)
 					{
 						preNodePtr = curNodePtr;
 						curNodePtr++;
 						err_when(i+1!=resultNodeNum && preNodePtr->node_x==curNodePtr->node_x && preNodePtr->node_y==curNodePtr->node_y);
 					}
 					else if(i+2<resultNodeNum)	//------ not the end of the array ------//
 					{
 						//------------ both magnitudes are equal ----------//
						oldPreNodePtr = curNodePtr;
						preNodePtr = oldPreNodePtr+1;
 						curNodePtr = preNodePtr+1;
 						changed++;
 						i++;
 					}
 					else if(i+1<resultNodeNum)
 					{
 						preNodePtr = curNodePtr+1;	// for writing the last node
 						changed++;
 						i++;
 					}
 					else
 						break;
 					err_when(oldPreNodePtr->node_x==preNodePtr->node_x && oldPreNodePtr->node_y==preNodePtr->node_y);
 					continue;
 				}
 
 				result_vec_x = vec_x+new_vec_x;
 				result_vec_y = vec_y+new_vec_y;
				err_when(result_vec_x%2 || result_vec_y%2);
				necessaryPoint = 0;
 				//----------------------------------------------//
 				// (3) turn 45 degree
 				//----------------------------------------------//
				if(abs(result_vec_x)<=move_scale && abs(result_vec_y)<=move_scale && (result_vec_x==0 || result_vec_y==0))
				{
					err_when(result_vec_x>=2*move_scale || result_vec_y>=2*move_scale);
					if((vec_x==0 || vec_y==0) && vec_magn>move_scale)
					{
						//-------------------------------------------------------------------------------------------//
						// check one more point if the entering vector direction is horizontal or vertical
						//-------------------------------------------------------------------------------------------//
						tempXLoc = preNodePtr->node_x - 2*vec_x;
						tempYLoc = preNodePtr->node_y - 2*vec_y;
						tempVectorX = vec_x + result_vec_x;
						tempVectorY = vec_y + result_vec_y;
						err_when((tempXLoc+tempVectorX/2)%2==0 && (tempYLoc+tempVectorY/2)%2==0);
						if(can_walk(tempXLoc+tempVectorX/2, tempYLoc+tempVectorY/2))
						{
							//------- checking for the farther point -------//
							if(vec_magn>2*move_scale) // add a new point
							{
								oldPreNodePtr->node_x = tempXLoc;
								oldPreNodePtr->node_y = tempYLoc;
								debug_reuse_check_xy_magn(oldPreNodePtr->node_x, oldPreNodePtr->node_y, writeNodePtr->node_x, writeNodePtr->node_y);//**** debug checking
 								writeNodePtr++;
 								writeNodePtr->node_x = oldPreNodePtr->node_x;
 								writeNodePtr->node_y = oldPreNodePtr->node_y;
 								count++;
							}
						}
						else if(can_walk(preNodePtr->node_x+(new_vec_x-vec_x)/2, preNodePtr->node_y+(new_vec_y-vec_y)/2))
						{
							//-------------------------------------------------------------------------------------------//
							// 1) the vector entering direction is not horizontal or vetical (only one can be processed), or
							// 2) the vector entering direction is matched, but vec_magn==move_scale
							//-------------------------------------------------------------------------------------------//
							if(vec_magn>move_scale)
							{
								oldPreNodePtr->node_x = preNodePtr->node_x-vec_x;
								oldPreNodePtr->node_y = preNodePtr->node_y-vec_y;
								debug_reuse_check_xy_magn(oldPreNodePtr->node_x, oldPreNodePtr->node_y, writeNodePtr->node_x, writeNodePtr->node_y);//**** debug checking
 								writeNodePtr++;
 								writeNodePtr->node_x = oldPreNodePtr->node_x;
 								writeNodePtr->node_y = oldPreNodePtr->node_y;
 								count++;
							}
						}
						else
							necessaryPoint = 1; //--- failure checking in the near point ----//
					}
					else
					{
						//------------------------------------------------------------------------------------//
						// 1) the vector direction is not horizontal or vertical (only one step is checked), or
						// 2) the direction is matched but vec_magn==move_scale
						//------------------------------------------------------------------------------------//
						if(can_walk(preNodePtr->node_x+(new_vec_x-vec_x)/2, preNodePtr->node_y+(new_vec_y-vec_y)/2))
						{
							if(vec_magn>move_scale)
							{
								oldPreNodePtr->node_x = preNodePtr->node_x-vec_x;
								oldPreNodePtr->node_y = preNodePtr->node_y-vec_y;
								debug_reuse_check_xy_magn(oldPreNodePtr->node_x, oldPreNodePtr->node_y, writeNodePtr->node_x, writeNodePtr->node_y);//**** debug checking
 								writeNodePtr++;
 								writeNodePtr->node_x = oldPreNodePtr->node_x;
 								writeNodePtr->node_y = oldPreNodePtr->node_y;
 								count++;
							}
						}
						else
							necessaryPoint = 1;
					}

					if(!necessaryPoint)
					{
						if(new_vec_magn>move_scale)
						{
							preNodePtr->node_x += new_vec_x;
							preNodePtr->node_y += new_vec_y;
 							debug_reuse_check_xy_magn(preNodePtr->node_x, preNodePtr->node_y, writeNodePtr->node_x, writeNodePtr->node_y);//**** debug checking
 							writeNodePtr++;
 							writeNodePtr->node_x = preNodePtr->node_x;
 							writeNodePtr->node_y = preNodePtr->node_y;
 							count++;
							oldPreNodePtr = preNodePtr;
						}

						preNodePtr = curNodePtr;
						curNodePtr++;
						changed++;
						continue;
					}
				}

 				//----------------------------------------------//
 				// (4) turn 90 degree
 				//----------------------------------------------//
				if(!necessaryPoint && vec_x*new_vec_x+vec_y*new_vec_y==0)
				{
					type = (vec_x==0 || vec_y==0) ? 0 : 1; // 0 for + case, 1 for x case

					if(type==1) // x case
					{
						tempXLoc = preNodePtr->node_x+(new_vec_x-vec_x)/2;
						tempYLoc = preNodePtr->node_y+(new_vec_y-vec_y)/2;
						
						if(can_walk(tempXLoc, tempYLoc) && can_walk(tempXLoc-result_vec_x/4, tempYLoc-result_vec_y/4) &&
							can_walk(tempXLoc+result_vec_x/4, tempYLoc+result_vec_y/4))
						{
							if(vec_magn>move_scale)
							{
 								oldPreNodePtr->node_x = preNodePtr->node_x-vec_x;
 								oldPreNodePtr->node_y = preNodePtr->node_y-vec_y;
 								debug_reuse_check_xy_magn(oldPreNodePtr->node_x, oldPreNodePtr->node_y, writeNodePtr->node_x, writeNodePtr->node_y);//**** debug checking
 								writeNodePtr++;
 								writeNodePtr->node_x = oldPreNodePtr->node_x;
 								writeNodePtr->node_y = oldPreNodePtr->node_y;
 								count++;
							}

							if(new_vec_magn>move_scale)
							{
 								oldPreNodePtr->node_x = preNodePtr->node_x+new_vec_x;
 								oldPreNodePtr->node_y = preNodePtr->node_y+new_vec_y;
 								err_when(oldPreNodePtr->node_x==curNodePtr->node_x && oldPreNodePtr->node_y==curNodePtr->node_y);
 								err_when(oldPreNodePtr->node_x==preNodePtr->node_x && oldPreNodePtr->node_y==preNodePtr->node_y);
 								debug_reuse_check_xy_magn(oldPreNodePtr->node_x, oldPreNodePtr->node_y, writeNodePtr->node_x, writeNodePtr->node_y);//**** debug checking
  								writeNodePtr++;
 								writeNodePtr->node_x = oldPreNodePtr->node_x;
 								writeNodePtr->node_y = oldPreNodePtr->node_y;
 								count++;
							}

							preNodePtr = curNodePtr;
							curNodePtr++;
						}
						else
							necessaryPoint = 1;
					}
					else // + case
					{
						tempVectorX = new_vec_x-vec_x;
						tempVectorY = new_vec_y-vec_y;
						tempXLoc = preNodePtr->node_x+tempVectorX/2;
						tempYLoc = preNodePtr->node_y+tempVectorY/2;
						err_when(tempXLoc%2==0 || tempYLoc%2==0);
						if(can_walk(tempXLoc, tempYLoc))
						{
							if(vec_magn>move_scale)
							{
								oldPreNodePtr->node_x = preNodePtr->node_x-vec_x;
								oldPreNodePtr->node_y = preNodePtr->node_y-vec_y;
 								debug_reuse_check_xy_magn(oldPreNodePtr->node_x, oldPreNodePtr->node_y, writeNodePtr->node_x, writeNodePtr->node_y);//**** debug checking
 								writeNodePtr++;
 								writeNodePtr->node_x = oldPreNodePtr->node_x;
 								writeNodePtr->node_y = oldPreNodePtr->node_y;
 								count++;
							}

							if(new_vec_magn>move_scale)
							{
								oldPreNodePtr->node_x = preNodePtr->node_x+new_vec_x;
								oldPreNodePtr->node_y = preNodePtr->node_y+new_vec_y;
 								debug_reuse_check_xy_magn(oldPreNodePtr->node_x, oldPreNodePtr->node_y, writeNodePtr->node_x, writeNodePtr->node_y);//**** debug checking
 								writeNodePtr++;
 								writeNodePtr->node_x = oldPreNodePtr->node_x;
 								writeNodePtr->node_y = oldPreNodePtr->node_y;
 								count++;
							}

							preNodePtr = curNodePtr;
							curNodePtr++;
						}
						else
							necessaryPoint = 1;
					}

					if(!necessaryPoint)
					{	
						changed++;
						continue;
					}
				}
				
 				//--------------------------------------------------//
 				// (5) this point is necessary for the reuse-path
 				//--------------------------------------------------//
 				debug_reuse_check_xy_magn(preNodePtr->node_x, preNodePtr->node_y, writeNodePtr->node_x, writeNodePtr->node_y);//**** debug checking
 				writeNodePtr++;
 				writeNodePtr->node_x = preNodePtr->node_x;
 				writeNodePtr->node_y = preNodePtr->node_y;
 				count++;
 				oldPreNodePtr = preNodePtr;
 				preNodePtr = curNodePtr;
 				curNodePtr++;
 				err_when(oldPreNodePtr->node_x==preNodePtr->node_x && oldPreNodePtr->node_y==preNodePtr->node_y);
 			}while(++i<resultNodeNum);
 			
 			debug_reuse_check_xy_magn(preNodePtr->node_x, preNodePtr->node_y, writeNodePtr->node_x, writeNodePtr->node_y);//**** debug checking
 			writeNodePtr++; //------- write the end node -----------//
 			writeNodePtr->node_x = preNodePtr->node_x;
 			writeNodePtr->node_y = preNodePtr->node_y;
 			count++;
 			resultNodeNum = count;
 			
 			mem_del(resultPath);
 			resultPath = wellShapedNodePtr;
 			wellShapedNodePtr = NULL;
 		}
	}

	#ifdef DEBUG
		//------------------------------------------------------------------------//
		// used to debug for the path of UNIT_SEA
		//------------------------------------------------------------------------//
		if(mobile_type==UNIT_SEA && resultNodeNum>1)
		{
			ResultNode *debugPtr1 = resultPath;
			ResultNode *debugPtr2 = resultPath+1;
			int di=1, dj, dvecX, dvecY, magn;
			while(di<resultNodeNum)
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
	
	return resultPath;
}
//--------- End of function SeekPathReuse::smooth_path2 ---------//