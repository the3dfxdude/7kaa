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

//Filename    : OUNITAAT.CPP
//Description : Object UnitArray - part 2

#include <ALL.h>
#include <OVGA.h>
#include <OSYS.h>
#include <OSTR.h>
#include <OREMOTE.h>
#include <OWORLD.h>
#include <ONATION.h>
#include <OFIRM.h>
#include <OTOWN.h>
#include <OTERRAIN.h>
#include <OU_MARI.h>

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

//------ static variables for attacking ------//
// ATTACK_DIR is defined in OUNIT.H
#define MAX_TARGET_SIZE				4
#define MAX_UNIT_SURROUND_SIZE	MAX_TARGET_SIZE+2	// handle the case up to target size 4x4
#define SHIFT_ADJUST					1						// since the upper left offset position is (-1, -1)
static short *unit_processed_array;						// store those processed unit sprite_recno
static short unit_processed_count;						// count the number processed units
static short *dir_array_ptr[ATTACK_DIR];				// store units' sprite_recno in each direction
static short dir_array_count[ATTACK_DIR];				// num of unit in each direction
static char unreachable_table[MAX_UNIT_SURROUND_SIZE][MAX_UNIT_SURROUND_SIZE]; // table shared for all attackers

//************* debug ***************//
#ifdef DEBUG
	//--------------------------------------------------------------------------------//
	// <int> resultNum -	the num of free space around the target
	// <int>	width		 - width of target
	// <int> height	 - height of target
	//--------------------------------------------------------------------------------//
	static void debug_analyse_result_check(int resultNum, int width, int height)
	{
		int count=0, i, j;
		for(i=0; i<width+2; i++)
		{
			if(unreachable_table[i][0]==0)
				count++;
		}

		i = width+1;
		for(j=1; j<height+2; j++)
		{
			if(unreachable_table[i][j]==0)
				count++;
		}

		for(j=1; j<height+2; j++)
		{
			if(unreachable_table[0][j]==0)
				count++;
		}

		j = height+1;
		for(i=1; i<width+1; i++)
		{
			if(unreachable_table[i][j]==0)
				count++;
		}

		err_when(count!=resultNum);
	}
#endif

#ifdef DEBUG
#define debug_result_check(resultNum, width, height)		debug_analyse_result_check(resultNum, width, height)
#else
#define debug_result_check(resultNum, width, height)
#endif
//************* debug ***************//


//--------- Begin of function UnitArray::get_target_surround_loc ---------//
// <int>	targetWidth		- target width
// <int>	targetHeight	- target height
//
char UnitArray::get_target_surround_loc(int targetWidth, int targetHeight)
{
	static char surround_loc[MAX_TARGET_SIZE][MAX_TARGET_SIZE] // width, height
	=	{	{ 8, 10, 12, 14}, {10, 12, 14, 16}, {12, 14, 16, 18}, {14, 16, 18, 20}};

	return surround_loc[targetWidth-1][targetHeight-1];
}
//----------- End of function UnitArray::get_target_surround_loc -----------//


//--------- Begin of function UnitArray::update_unreachable_table ---------//
// check target surroundig to drop those unreachable location
//
// <int>		targetXLoc		- target x location
//	<int>		targetYLoc		- target y location
//	<int>		targetWidth		- target width
// <int>		targetHeight	- target height
//	<char>	mobileType		- mobile type of the attacker
//	<int&>	analyseResult	- reference for returning num of reachable location
//
void UnitArray::update_unreachable_table(int targetXLoc, int targetYLoc, int targetWidth, int targetHeight, 
													  char mobileType, int &analyseResult)
{
	int xLoc1, yLoc1, xLoc2, yLoc2, i;

	//----------------------------------------------------------------------//
	// checking for left & right edges, calculate location and counter
	//----------------------------------------------------------------------//
	xLoc1 = targetXLoc-1;
	xLoc2 = targetXLoc+targetWidth;
	
	if(targetYLoc==0)
	{
		i = targetHeight+1;
		yLoc1=targetYLoc;
	}
	else
	{
		i = targetHeight+2;
		yLoc1=targetYLoc-1;
	}

	if(targetYLoc+targetHeight>=MAX_WORLD_Y_LOC)
		i--;

	for(yLoc2=yLoc1-targetYLoc+SHIFT_ADJUST; i>0; i--, yLoc1++, yLoc2++)
	{
		//---- left edge ----//
		if(xLoc1>=0 && !unreachable_table[0][yLoc2] && !world.get_loc(xLoc1, yLoc1)->can_move(mobileType))
		{
			unreachable_table[0][yLoc2] = 1;
			analyseResult--;
		}

		//----- right edge -----//
		if(xLoc2<MAX_WORLD_X_LOC && !unreachable_table[targetWidth+1][yLoc2] && 
			!world.get_loc(xLoc2, yLoc1)->can_move(mobileType))
		{
			unreachable_table[targetWidth+1][yLoc2] = 1;
			analyseResult--;
		}

		if(!analyseResult)
			return;
	}

	//----------------------------------------------------------------------//
	// checking for the top and bottom edges
	//----------------------------------------------------------------------//
	yLoc1 = targetYLoc-1;
	yLoc2 = targetYLoc+targetHeight;
	for(i=0, xLoc1=targetXLoc, xLoc2=xLoc1-targetXLoc+SHIFT_ADJUST; i<targetWidth; i++, xLoc1++, xLoc2++)
	{
		//---- top edge ----//
		if(yLoc1>=0 && !unreachable_table[xLoc2][0] && !world.get_loc(xLoc1, yLoc1)->can_move(mobileType))
		{
			unreachable_table[xLoc2][0] = 1;
			analyseResult--;
		}

		//----- bottom edge -----//
		if(yLoc2<MAX_WORLD_Y_LOC && !unreachable_table[xLoc2][targetHeight+1] &&
			!world.get_loc(xLoc1, yLoc2)->can_move(mobileType))
		{
			unreachable_table[xLoc2][targetHeight+1] = 1;
			analyseResult--;
		}

		if(!analyseResult)
			return;
	}

	debug_result_check(analyseResult, targetWidth, targetHeight);
	err_when(analyseResult<0);
}
//----------- End of function UnitArray::update_unreachable_table -----------//


//--------- Begin of function UnitArray::attack ---------//
//
// Order the unit to attack a specific location.
//
// <int> 	targetXLoc, targetYLoc - target location
// <int>		divided					  - whether the units are divided
// <short*> selectedUnitArray   	  - an array of recno of selected units.
// <int>    selectedCount 		 	  - no. of selected units.
// [char]   remoteAction 	    	  - whether this is an action carried out by a remote machine or not.
//												 (default: 0)
// <int>    targetUnitRecno        - if it is unit, pass the unit recno, otherwise 0
//
void UnitArray::attack(int targetXLoc, int targetYLoc, int divided, short* selectedUnitArray, int selectedCount, char remoteAction, int targetUnitRecno)
{
	err_when(targetXLoc<0 || targetYLoc<0 || targetXLoc>=MAX_WORLD_X_LOC || targetYLoc>=MAX_WORLD_Y_LOC);

	// ######## patch begin Gilbert 5/8 #########//

	int targetNationRecno = 0;

	if( targetUnitRecno == 0 )
	{
		// unit determined from the location

		int tmpMobileType;
		Location* locPtr = world.get_loc(targetXLoc, targetYLoc);
		targetUnitRecno = locPtr->get_any_unit(tmpMobileType) ;
	}
	else
	{
		// location determined by the unit
		Unit *unitPtr;
		if( !unit_array.is_deleted(targetUnitRecno)
			&& (unitPtr = unit_array[targetUnitRecno]) 
			&& unitPtr->is_visible() )
		{
			targetXLoc = unitPtr->next_x_loc();
			targetYLoc = unitPtr->next_y_loc();
			if( unitPtr->unit_id != UNIT_EXPLOSIVE_CART )	// attacking own porcupine is allowed
				targetNationRecno = unitPtr->nation_recno;
		}
		else
			targetUnitRecno = 0;
	}
	
	if( targetUnitRecno == 0 )
	{
		//--- set the target coordination to the top left position of the town/firm ---//

		Location* locPtr = world.get_loc(targetXLoc, targetYLoc);

		//---- if there is a firm on this location -----//

		if( locPtr->is_firm() )
		{
			Firm* firmPtr = firm_array[locPtr->firm_recno()];

			targetXLoc = firmPtr->loc_x1;
			targetYLoc = firmPtr->loc_y1;
			targetNationRecno = firmPtr->nation_recno;
		}

		//---- if there is a town on this location -----//

		else if( locPtr->is_town() )
		{
			Town* townPtr = town_array[locPtr->town_recno()];

			targetXLoc = townPtr->loc_x1;
			targetYLoc = townPtr->loc_y1;
			targetNationRecno = townPtr->nation_recno;
		}

		else
			return;
	}
	// ######## patch end Gilbert 5/8 #########//

	//--------- AI debug code ---------//

	//--- AI attacking a nation which its NationRelation::should_attack is 0 ---//

	Unit* attackUnit = unit_array[ selectedUnitArray[0] ];

	if( attackUnit->nation_recno && targetNationRecno )
	{
		if( nation_array[attackUnit->nation_recno]->get_relation(targetNationRecno)->should_attack==0 )
			return;
	}

	//-------- if it's a multiplayer game --------//
	if( !remoteAction && remote.is_enable() )
	{
		short* shortPtr = (short*) remote.new_send_queue_msg(MSG_UNIT_ATTACK, sizeof(short) * (5+selectedCount) );

		shortPtr[0] = targetXLoc;
		shortPtr[1] = targetYLoc;
		shortPtr[2] = targetUnitRecno;
		shortPtr[3] = selectedCount;
		shortPtr[4] = divided;

		memcpy( shortPtr+5, selectedUnitArray, sizeof(short) * selectedCount );
	}
	else
	{
		if(!divided)
		{
			divide_array(targetXLoc, targetYLoc, selectedUnitArray, selectedCount, 1); // 1 for excluding the recno in target location

			Location* locPtr = world.get_loc(targetXLoc, targetYLoc);
			int	targetMobileType = locPtr->has_any_unit();

			if(selected_land_unit_count)
				attack_call(targetXLoc, targetYLoc, UNIT_LAND, targetMobileType, 1, selected_land_unit_array, selected_land_unit_count, targetUnitRecno);
			
			if(selected_sea_unit_count)
				attack_call(targetXLoc, targetYLoc, UNIT_SEA, targetMobileType, 1, selected_sea_unit_array, selected_sea_unit_count, targetUnitRecno);

			if(selected_air_unit_count)
				attack_call(targetXLoc, targetYLoc, UNIT_AIR, targetMobileType, 1, selected_air_unit_array, selected_air_unit_count, targetUnitRecno);

			selected_land_unit_count = selected_sea_unit_count = selected_air_unit_count = 0;
			mem_del(selected_land_unit_array);
			mem_del(selected_sea_unit_array);
			mem_del(selected_air_unit_array);
			return;
		}
		else
			err_here();
	}
}
//----------- End of function UnitArray::attack -----------//


//--------- Begin of function UnitArray::attack_call ---------//
// <int>		targetXLoc			- target x location
// <int>		targetYLoc			- target y location
// <char>	mobileType			- attacker's mobile type
// <char>	targetMobileType	- target mobile type
// <int>		divided				- whether the units are divided
// <short*>	selectedUnitArray	- selected units' recno.
// <int>		selectedCount		- num of selected units
//
void UnitArray::attack_call(int targetXLoc, int targetYLoc, char mobileType, char targetMobileType, int divided, short* selectedUnitArray, int selectedCount, int targetUnitRecno)
{
	//------------- attack now -------------//
	err_when( selectedCount > 10000 );

	Location* locPtr = world.get_loc(targetXLoc, targetYLoc);
	err_when(!locPtr);
	
	// ##### patch begin Gilbert 5/8 ######//
	//if(targetMobileType)
	if( targetUnitRecno && !unit_array.is_deleted(targetUnitRecno) )
	{
		//---------------- attack unit --------------//
		//Unit *targetUnit = unit_array[locPtr->unit_recno(targetMobileType)];
		Unit *targetUnit = unit_array[targetUnitRecno];
		if(!targetUnit->is_visible() || targetUnit->hit_points<=0)
			return;

		// short targetUnitRecno = targetUnit->sprite_recno;
		attack_unit(targetXLoc, targetYLoc, targetUnitRecno, selectedUnitArray, selectedCount);
	}
	// ##### patch end Gilbert 5/8 ######//
	else if(locPtr->is_firm())
	{
		//------------------ attack firm -------------------//
		Firm *firmPtr = firm_array[locPtr->firm_recno()];
		err_when(!firmPtr);
		if(firmPtr->hit_points<=0)
			return;

		attack_firm(targetXLoc, targetYLoc, firmPtr->firm_recno, selectedUnitArray, selectedCount);
	}
	else if(locPtr->is_town())
	{
		//-------------------- attack town -------------------//
		Town *townPtr = town_array[locPtr->town_recno()];
		err_when(!townPtr);
		attack_town(targetXLoc, targetYLoc, townPtr->town_recno, selectedUnitArray, selectedCount);
	}
	else if(locPtr->is_wall())
	{
		//------------------ attack wall ---------------------//
		attack_wall(targetXLoc, targetYLoc, selectedUnitArray, selectedCount);
	}
	else
		return; // no target for attacking
}
//----------- End of function UnitArray::attack_call -----------//


//--------- Begin of function UnitArray::attack_unit ---------//
// <int> targetXLoc, targetYLoc	- the unit upper left location
// <short> targetunitRecno			- the unit recno
//
// Note : this attack function only for attackers with size 1x1.
//
void UnitArray::attack_unit(int targetXLoc, int targetYLoc, short targetUnitRecno, short* selectedUnitArray, int selectedCount)
{
	err_when(selectedCount<=0);
	if(selectedCount==1)
	{
		Unit *unitPtr = (Unit*) get_ptr(selectedUnitArray[0]);
		unitPtr->unit_group_id = unit_array.cur_group_id++;
		unitPtr->attack_unit(targetXLoc, targetYLoc);
		return;
	}
	
	//********************** improve later begin **************************//
	//---------------------------------------------------------------------//
	// codes for differnt territory or different mobile_type attacking should
	// be added in the future.
	//---------------------------------------------------------------------//
	Unit	*firstUnitPtr = unit_array[selectedUnitArray[0]];
	Unit	*targetPtr2 = unit_array[targetUnitRecno];
	if( (world.get_loc(targetXLoc, targetYLoc)->region_id !=
		  world.get_loc(firstUnitPtr->next_x_loc(), firstUnitPtr->next_y_loc())->region_id) ||
		  (targetPtr2->mobile_type!=firstUnitPtr->mobile_type) )
	{
		Unit *unitPtr;
		set_group_id(selectedUnitArray, selectedCount);

		for(int i=0; i<selectedCount; i++)
		{
			unitPtr = (Unit*) get_ptr(selectedUnitArray[i]);
			unitPtr->attack_unit(targetXLoc, targetYLoc);
		}
		return;
	}
	//*********************** improve later end ***************************//
	
	//----------- initialize local parameters ------------//
	Unit	*targetPtr = unit_array[targetUnitRecno];
	int	targetWidth = targetPtr->sprite_info->loc_width;
	int	targetHeight = targetPtr->sprite_info->loc_height;
	int	targetXLoc2 = targetXLoc + targetWidth - 1;
	int	targetYLoc2 = targetYLoc + targetHeight - 1;
	char	surroundLoc = get_target_surround_loc(targetWidth, targetHeight);
	char	*xOffsetPtr = get_target_x_offset(targetWidth, targetHeight, 0);
	char	*yOffsetPtr = get_target_y_offset(targetWidth, targetHeight, 0);

	//---------------------------------------------------------------------//
	// construct data structure
	//---------------------------------------------------------------------//
	int tempVar = sizeof(short)*selectedCount;
	
	memset(dir_array_count, 0, sizeof(dir_array_count));
	int count;
	for(count=0; count<ATTACK_DIR; count++)
	{
		dir_array_ptr[count] = (short*) mem_add(tempVar);
		memset(dir_array_ptr[count], 0, tempVar);
	}

	unit_processed_array = (short *)mem_add(tempVar);
	unit_processed_count = 0;

	//---------------------------------------------------------------------//
	// divide the units into each region
	//---------------------------------------------------------------------//
	Unit		*unitPtr = (Unit*) get_ptr(selectedUnitArray[0]);
	err_when(!unitPtr);
	DWORD groupId = unit_array.cur_group_id++;
	arrange_units_in_group(targetXLoc, targetYLoc, targetXLoc2, targetYLoc2, selectedUnitArray, selectedCount, groupId, 1);

	//---------------------------------------------------------------------//
	// now the attackers are divided into 8 groups to attack the target
	//---------------------------------------------------------------------//
	int	xLoc, yLoc;			// actual target surrounding location to move to
	int	xOffset, yOffset;	// offset location of the target
	int	unprocessed;		// number of units in this group
	int	dist, xDist, yDist, minDist;
	int	destCount;
	short unitPos;				// store the position of the unit with minDist
	short	*curArrayPtr;

	int i;
	for(i=0; i<MAX_UNIT_SURROUND_SIZE; i++)
		memset(unreachable_table[i], 0, sizeof(char)*MAX_UNIT_SURROUND_SIZE);

	//---------------------------------------------------------------------//
	// anaylse the surrounding location of the target
	//---------------------------------------------------------------------//
	int analyseResult = analyse_surround_location(targetXLoc, targetYLoc, targetWidth, targetHeight, targetPtr->mobile_type);
	debug_result_check(analyseResult, targetWidth, targetHeight);
	err_when(analyseResult<0);

	if(!analyseResult) // 0 if all surround location is not accessible
	{
		//------------------------------------------------------------//
		// special handling for this case
		//------------------------------------------------------------//
		handle_attack_target_totally_blocked(targetXLoc, targetYLoc, targetUnitRecno, selectedUnitArray, selectedCount, 1);

		for(count=0; count<ATTACK_DIR; count++)
			mem_del(dir_array_ptr[count]);

		mem_del(unit_processed_array);
		return;
	}

	//---------------------------------------------------------------------//
	// let the units move to the rest accessible location
	//---------------------------------------------------------------------//
	for(count=0; count<ATTACK_DIR; count++) // for each array/group
	{
		//--------- initialize for each group --------//
		unprocessed = dir_array_count[count];	// get the number of units in this region
		if(!unprocessed)
			continue;

		destCount = surroundLoc-1;
		curArrayPtr = dir_array_ptr[count];	// get the recno of units in this region

		xOffsetPtr = get_target_x_offset(targetWidth, targetHeight, count);
		yOffsetPtr = get_target_y_offset(targetWidth, targetHeight, count);
		xOffset = *xOffsetPtr;
		yOffset = *yOffsetPtr;

		//-----------------------------------------------------------------//
		// determine a suitable location for the attacker to move to
		//-----------------------------------------------------------------//
		while(unprocessed)
		{
			//-----------------------------------------------------//
			// find a reachable location, or not searched location
			//-----------------------------------------------------//
			err_when(analyseResult<0);
			if(!analyseResult)
			{
				handle_attack_target_totally_blocked(targetXLoc, targetYLoc, targetUnitRecno, selectedUnitArray, selectedCount, 1);
				
				for(count=0; count<ATTACK_DIR; count++)
					mem_del(dir_array_ptr[count]);

				mem_del(unit_processed_array);
				return;
			}
			else
			{
				for(i=0; i<surroundLoc; i++)
				{
					if((++destCount)>=surroundLoc)
					{
						destCount = 0;
						xOffsetPtr = get_target_x_offset(targetWidth, targetHeight, count);
						yOffsetPtr = get_target_y_offset(targetWidth, targetHeight, count);
						xOffset = *xOffsetPtr;
						yOffset = *yOffsetPtr;
					}
					else
					{
						xOffset = *(++xOffsetPtr);
						yOffset = *(++yOffsetPtr);
					}

					if(!unreachable_table[xOffset+SHIFT_ADJUST][yOffset+SHIFT_ADJUST])
						break;
				}
			}

			err_when(unreachable_table[xOffset+SHIFT_ADJUST][yOffset+SHIFT_ADJUST]);

			//------------------------------------------------------------//
			// find the closest attacker
			//------------------------------------------------------------//
			err_when(unprocessed>2000 || unprocessed <0);
			xLoc = targetXLoc + xOffset;
			yLoc = targetYLoc + yOffset;
			err_when(!world.get_loc(xLoc, yLoc)->can_move(targetPtr->mobile_type));

			for(i=0, minDist=0x7FFFFFF; i<unprocessed; i++)
			{
				unitPtr = (Unit*) get_ptr(curArrayPtr[i]);
				xDist = abs(xLoc-unitPtr->next_x_loc());
				yDist = abs(yLoc-unitPtr->next_y_loc());
				dist = (xDist >= yDist) ? xDist*10+yDist : yDist*10+xDist;

				if(dist < minDist)
				{
					minDist = dist;
					unitPos = i;
				}
			}

			unitPtr = (Unit*) get_ptr(curArrayPtr[unitPos]);
			curArrayPtr[unitPos] = curArrayPtr[--unprocessed]; // move the units in the back to the front

			err_when(unitPtr->action_mode2!=ACTION_ATTACK_WALL &&
						unitPtr->cur_action==SPRITE_ATTACK && unitPtr->action_para==0);
			seek_path.set_status(PATH_WAIT);
			err_when(seek_path.path_status==PATH_NODE_USED_UP);
			unitPtr->attack_unit(targetXLoc, targetYLoc, xOffset, yOffset);

			//------------------------------------------------------------//
			// store the unit sprite_recno in the array
			//------------------------------------------------------------//
			unit_processed_array[unit_processed_count++] = unitPtr->sprite_recno;

			//------------------------------------------------------------//
			// set the flag if unreachable
			//------------------------------------------------------------//
			if(seek_path.path_status==PATH_NODE_USED_UP)
			{
				unreachable_table[xLoc-targetXLoc+SHIFT_ADJUST][yLoc-targetYLoc+SHIFT_ADJUST] = 1;
				analyseResult--;
				debug_result_check(analyseResult, targetWidth, targetHeight);
				err_when(analyseResult<0);

				//------------------------------------------------------------//
				// the nearby location should also be unreachable
				//------------------------------------------------------------//
				check_nearby_location(targetXLoc, targetYLoc, xOffset, yOffset, targetWidth, targetHeight,
											 targetPtr->mobile_type, analyseResult);
			}

			update_unreachable_table(targetXLoc, targetYLoc, targetWidth, targetHeight, unitPtr->mobile_type, analyseResult);

			#ifdef DEBUG
				for(int di=0; di<targetWidth+2; di++)
				{
					for(int dj=0; dj<targetHeight+2; dj++)
					{
						if(di>=1 && di<=targetWidth && dj>=1 && dj<=targetHeight)
							continue;

						int debugXLoc = targetXLoc+di-SHIFT_ADJUST;
						int debugYLoc = targetYLoc+dj-SHIFT_ADJUST;
						if(debugXLoc<0 || debugXLoc>=MAX_WORLD_X_LOC || debugYLoc<0 || debugYLoc>=MAX_WORLD_Y_LOC)
							continue;

						Location *dlPtr = world.get_loc(debugXLoc, debugYLoc);
						
						err_when(!dlPtr->can_move(targetPtr->mobile_type) && unreachable_table[di][dj]==0);
					}
				}
			#endif
		}
	}

	//---------------------------------------------------------------------//
	// set the unreachable flag for each units
	//---------------------------------------------------------------------//
	//-************** codes here ***************-//

	//---------------------------------------------------------------------//
	// destruct data structure
	//---------------------------------------------------------------------//
	for(count=0; count<ATTACK_DIR; count++)
		mem_del(dir_array_ptr[count]);

	mem_del(unit_processed_array);
}
//----------- End of function UnitArray::attack_unit -----------//


//--------- Begin of function UnitArray::attack_firm ---------//
// <int> targetXLoc, targetYLoc	- the firm upper left location
// <short> firmRecno					- the firm recno
//
// try to calculate the best location for each unit to move to the
// surrounding of the firm for attacking
//
void UnitArray::attack_firm(int targetXLoc, int targetYLoc, short firmRecno, short* selectedUnitArray, int selectedCount)
{
	err_when(selectedCount<=0);
	if(selectedCount==1)
	{
		Unit *unitPtr = (Unit*) get_ptr(selectedUnitArray[0]);
		unitPtr->unit_group_id = unit_array.cur_group_id++;
		unitPtr->attack_firm(targetXLoc, targetYLoc);
		return;
	}

	//********************** improve later begin **************************//
	//---------------------------------------------------------------------//
	// codes for differnt territory or different mobile_type attacking should
	// be added in the future.
	//---------------------------------------------------------------------//
	Unit	*firstUnitPtr = unit_array[selectedUnitArray[0]];
	if(world.get_loc(targetXLoc, targetYLoc)->region_id !=
		world.get_loc(firstUnitPtr->next_x_loc(), firstUnitPtr->next_y_loc())->region_id)
	{
		Unit *unitPtr;
		set_group_id(selectedUnitArray, selectedCount);

		for(int i=0; i<selectedCount; i++)
		{
			unitPtr = (Unit*) get_ptr(selectedUnitArray[i]);
			unitPtr->attack_firm(targetXLoc, targetYLoc);
		}
		return;
	}
	//*********************** improve later end ***************************//

	//----------- initialize local parameters ------------//
	Firm		*firmPtr = firm_array[firmRecno];
	FirmInfo *firmInfo = firm_res[firmPtr->firm_id];
	int firmWidth = firmInfo->loc_width;
	int firmHeight = firmInfo->loc_height;
	int targetXLoc2 = targetXLoc + firmWidth - 1;	// the lower right corner of the firm
	int targetYLoc2 = targetYLoc + firmHeight -1;
	char	*xOffsetPtr, *yOffsetPtr;

	//---------------------------------------------------------------------//
	// construct data structure
	//---------------------------------------------------------------------//
	int tempVar = sizeof(short)*selectedCount;
	
	memset(dir_array_count, 0, sizeof(dir_array_count));
	int count;
	for(count=0; count<ATTACK_DIR; count++)
	{
		dir_array_ptr[count] = (short*) mem_add(tempVar);
		memset(dir_array_ptr[count], 0, tempVar);
	}

	unit_processed_array = (short *)mem_add(tempVar);
	unit_processed_count = 0;

	//---------------------------------------------------------------------//
	// divide the units into each region
	//---------------------------------------------------------------------//
	Unit		*unitPtr = (Unit*) get_ptr(selectedUnitArray[0]);
	err_when(!unitPtr);
	DWORD groupId = unit_array.cur_group_id++;
	arrange_units_in_group(targetXLoc, targetYLoc, targetXLoc2, targetYLoc2, selectedUnitArray, selectedCount, groupId, 2);

	//---------------------------------------------------------------------//
	// now the attackers are divided into 8 groups to attack the target
	//---------------------------------------------------------------------//
	int	xLoc, yLoc;			// actual target surrounding location to move to
	int	xOffset, yOffset;	// offset location of the target
	int	unprocessed;		// number of units in this group
	int	destCount;
	int	dist, xDist, yDist, minDist;
	short unitPos;				// store the position of the unit with minDist
	short	*curArrayPtr;
	char	surroundLoc = get_target_surround_loc(firmWidth, firmHeight);

	int i;
	for(i=0; i<MAX_UNIT_SURROUND_SIZE; i++)
		memset(unreachable_table[i], 0, sizeof(char)*MAX_UNIT_SURROUND_SIZE);

	//---------------------------------------------------------------------//
	// analyse the surrounding location of the target
	//---------------------------------------------------------------------//
	int analyseResult = analyse_surround_location(targetXLoc, targetYLoc, firmWidth, firmHeight, unitPtr->mobile_type);
	debug_result_check(analyseResult, firmWidth, firmHeight);
	err_when(analyseResult<0);

	if(!analyseResult) // 0 if all surround location is not accessible
	{
		//------------------------------------------------------------//
		// special handling for this case
		//------------------------------------------------------------//
		handle_attack_target_totally_blocked(targetXLoc, targetYLoc, firmRecno, selectedUnitArray, selectedCount, 2);

		for(count=0; count<ATTACK_DIR; count++)
			mem_del(dir_array_ptr[count]);

		mem_del(unit_processed_array);
		return;
	}

	//---------------------------------------------------------------------//
	// let the units move to the rest accessible location
	//---------------------------------------------------------------------//
	for(count=0; count<ATTACK_DIR; count++) // for each array/group
	{
		//--------- initialize for each group --------//
		unprocessed = dir_array_count[count];	// get the number of units in this region
		curArrayPtr = dir_array_ptr[count];	// get the recno of units in this region
		destCount = surroundLoc - 1;
		xOffsetPtr = get_target_x_offset(firmWidth, firmHeight, count);
		yOffsetPtr = get_target_y_offset(firmWidth, firmHeight, count);
		xOffset = *xOffsetPtr;
		yOffset = *yOffsetPtr;

		//-----------------------------------------------------------------//
		// determine a suitable location for the attacker to move to
		//-----------------------------------------------------------------//
		err_when(unprocessed>2000 || unprocessed <0);
		while(unprocessed)
		{
			//-----------------------------------------------------//
			// find a reachable location, or not searched location
			//-----------------------------------------------------//
			err_when(analyseResult<0);

			if(!analyseResult)
			{
				handle_attack_target_totally_blocked(targetXLoc, targetYLoc, firmRecno, selectedUnitArray, selectedCount, 2);
				
				for(count=0; count<ATTACK_DIR; count++)
					mem_del(dir_array_ptr[count]);

				mem_del(unit_processed_array);
				return;
			}
			else
			{
				for(i=0; i<surroundLoc; i++)
				{
					if((++destCount)>=surroundLoc)
					{
						destCount = 0;
						xOffsetPtr = get_target_x_offset(firmWidth, firmHeight, count);
						yOffsetPtr = get_target_y_offset(firmWidth, firmHeight, count);
						xOffset = *xOffsetPtr;
						yOffset = *yOffsetPtr;
					}
					else
					{
						xOffset = *(++xOffsetPtr);
						yOffset = *(++yOffsetPtr);
					}

					if(!unreachable_table[xOffset+SHIFT_ADJUST][yOffset+SHIFT_ADJUST])
						break;
				}
			}

			err_when(unreachable_table[xOffset+SHIFT_ADJUST][yOffset+SHIFT_ADJUST]);

			//------------------------------------------------------------//
			// find the closest attacker
			//------------------------------------------------------------//
			err_when(unprocessed>2000 || unprocessed <0);
			xLoc = targetXLoc + xOffset;
			yLoc = targetYLoc + yOffset;
			err_when(!world.get_loc(xLoc, yLoc)->can_move(unitPtr->mobile_type));

			for(i=0, minDist=0x7FFFFFF; i<unprocessed; i++)
			{
				unitPtr = (Unit*) get_ptr(curArrayPtr[i]);
				xDist = abs(xLoc-unitPtr->next_x_loc());
				yDist = abs(yLoc-unitPtr->next_y_loc());
				dist = (xDist >= yDist) ? xDist*10+yDist : yDist*10+xDist;

				if(dist < minDist)
				{
					minDist = dist;
					unitPos = i;
				}
			}

			unitPtr = (Unit*) get_ptr(curArrayPtr[unitPos]);
			curArrayPtr[unitPos] = curArrayPtr[--unprocessed]; // move the units in the back to the front

			err_when(unitPtr->action_mode2!=ACTION_ATTACK_WALL &&
						unitPtr->cur_action==SPRITE_ATTACK && unitPtr->action_para==0);
			seek_path.set_status(PATH_WAIT);
			err_when(seek_path.path_status==PATH_NODE_USED_UP);
			unitPtr->attack_firm(targetXLoc, targetYLoc, xOffset, yOffset);

			//------------------------------------------------------------//
			// set the flag if unreachable
			//------------------------------------------------------------//
			if(seek_path.path_status==PATH_NODE_USED_UP)
			{
				unreachable_table[xOffset+SHIFT_ADJUST][yOffset+SHIFT_ADJUST] = 1;
				analyseResult--;
				debug_result_check(analyseResult, firmWidth, firmHeight);
				err_when(analyseResult<0);

				//------------------------------------------------------------//
				// the nearby location should also be unreachable
				//------------------------------------------------------------//
				check_nearby_location(targetXLoc, targetYLoc, xOffset, yOffset, firmWidth, firmHeight,
											 unitPtr->mobile_type, analyseResult);
			}

			update_unreachable_table(targetXLoc, targetYLoc, firmWidth, firmHeight, unitPtr->mobile_type, analyseResult);

			#ifdef DEBUG
				for(int di=0; di<firmWidth+2; di++)
				{
					for(int dj=0; dj<firmHeight+2; dj++)
					{
						if(di>=1 && di<=firmWidth && dj>=1 && dj<=firmHeight)
							continue;

						int debugXLoc = targetXLoc+di-SHIFT_ADJUST;
						int debugYLoc = targetYLoc+dj-SHIFT_ADJUST;
						if(debugXLoc<0 || debugXLoc>=MAX_WORLD_X_LOC || debugYLoc<0 || debugYLoc>=MAX_WORLD_Y_LOC)
							continue;

						Location *dlPtr = world.get_loc(debugXLoc, debugYLoc);
						
						err_when(!dlPtr->can_move(unitPtr->mobile_type) && unreachable_table[di][dj]==0);
					}
				}
			#endif
		}
	}

	//---------------------------------------------------------------------//
	// set the unreachable flag for each units
	//---------------------------------------------------------------------//
	//-************** codes here ***************-//

	//---------------------------------------------------------------------//
	// destruct data structure
	//---------------------------------------------------------------------//
	for(count=0; count<ATTACK_DIR; count++)
		mem_del(dir_array_ptr[count]);

	mem_del(unit_processed_array);
}
//----------- End of function UnitArray::attack_firm -----------//


//--------- Begin of function UnitArray::attack_town ---------//
// <int> targetXLoc, targetYLoc	- the town upper left location
// <short> townRecno					- the town recno
//
// try to calculate the best location for each unit to move to the
// surrounding of the town for attacking
//
void UnitArray::attack_town(int targetXLoc, int targetYLoc, short townRecno, short* selectedUnitArray, int selectedCount)
{
	err_when(selectedCount<=0);
	if(selectedCount==1)
	{
		Unit *unitPtr = (Unit*) get_ptr(selectedUnitArray[0]);
		unitPtr->unit_group_id = unit_array.cur_group_id++;
		unitPtr->attack_town(targetXLoc, targetYLoc);
		return;
	}

	//********************** improve later begin **************************//
	//---------------------------------------------------------------------//
	// codes for differnt territory or different mobile_type attacking should
	// be added in the future.
	//---------------------------------------------------------------------//
	Unit	*firstUnitPtr = unit_array[selectedUnitArray[0]];
	if(world.get_loc(targetXLoc, targetYLoc)->region_id !=
		world.get_loc(firstUnitPtr->next_x_loc(), firstUnitPtr->next_y_loc())->region_id)
	{
		Unit *unitPtr;
		set_group_id(selectedUnitArray, selectedCount);

		for(int i=0; i<selectedCount; i++)
		{
			unitPtr = (Unit*) get_ptr(selectedUnitArray[i]);
			unitPtr->attack_town(targetXLoc, targetYLoc);
		}
		return;
	}
	//*********************** improve later end ***************************//

	//----------- initialize local parameters ------------//
	int targetXLoc2 = targetXLoc + STD_TOWN_LOC_WIDTH - 1;	// the lower right corner of the firm
	int targetYLoc2 = targetYLoc + STD_TOWN_LOC_HEIGHT - 1;
	char	*xOffsetPtr, *yOffsetPtr;

	//---------------------------------------------------------------------//
	// construct data structure
	//---------------------------------------------------------------------//
	int tempVar = sizeof(short)*selectedCount;
	
	memset(dir_array_count, 0, sizeof(dir_array_count));
	int count;
	for(count=0; count<ATTACK_DIR; count++)
	{
		dir_array_ptr[count] = (short*) mem_add(tempVar);
		memset(dir_array_ptr[count], 0, tempVar);
	}

	unit_processed_array = (short *)mem_add(tempVar);
	unit_processed_count = 0;

	//---------------------------------------------------------------------//
	// divide the units into each region
	//---------------------------------------------------------------------//
	Unit		*unitPtr = (Unit*) get_ptr(selectedUnitArray[0]);
	err_when(!unitPtr);
	DWORD groupId = unit_array.cur_group_id++;
	arrange_units_in_group(targetXLoc, targetYLoc, targetXLoc2, targetYLoc2, selectedUnitArray, selectedCount, groupId, 3);

	//---------------------------------------------------------------------//
	// now the attackers are divided into 8 groups to attack the target
	//---------------------------------------------------------------------//
	int	xLoc, yLoc;			// actual target surrounding location to move to
	int	xOffset, yOffset;	// offset location of the target
	int	unprocessed;		// number of units in this group
	int	dist, xDist, yDist, minDist;
	int	destCount;
	short unitPos;				// store the position of the unit with minDist
	short	*curArrayPtr;
	char	surroundLoc = get_target_surround_loc(STD_TOWN_LOC_WIDTH, STD_TOWN_LOC_HEIGHT);

	int i;
 	for(i=0; i<MAX_UNIT_SURROUND_SIZE; i++)
		memset(unreachable_table[i], 0, sizeof(char)*MAX_UNIT_SURROUND_SIZE);

	//---------------------------------------------------------------------//
	// analyse the surrounding location of the target
	//---------------------------------------------------------------------//
	int analyseResult = analyse_surround_location(targetXLoc, targetYLoc, STD_TOWN_LOC_WIDTH, STD_TOWN_LOC_HEIGHT, unitPtr->mobile_type);
	debug_result_check(analyseResult, STD_TOWN_LOC_WIDTH, STD_TOWN_LOC_HEIGHT);
	err_when(analyseResult<0);

	if(!analyseResult) // 0 if all surround location is not accessible
	{
		//------------------------------------------------------------//
		// special handling for this case
		//------------------------------------------------------------//
		handle_attack_target_totally_blocked(targetXLoc, targetYLoc, townRecno, selectedUnitArray, selectedCount, 3);

		for(count=0; count<ATTACK_DIR; count++)
			mem_del(dir_array_ptr[count]);

		mem_del(unit_processed_array);
		return;
	}

	//---------------------------------------------------------------------//
	// let the units move to the rest accessible location
	//---------------------------------------------------------------------//
	for(count=0; count<ATTACK_DIR; count++) // for each array/group
	{
		//--------- initialize for each group --------//
		unprocessed = dir_array_count[count];	// get the number of units in this region
		curArrayPtr = dir_array_ptr[count];	// get the recno of units in this region
		destCount = surroundLoc-1;
		xOffsetPtr = get_target_x_offset(STD_TOWN_LOC_WIDTH, STD_TOWN_LOC_HEIGHT, count);
		yOffsetPtr = get_target_y_offset(STD_TOWN_LOC_WIDTH, STD_TOWN_LOC_HEIGHT, count);
		xOffset = *xOffsetPtr;
		yOffset = *yOffsetPtr;

		//-----------------------------------------------------------------//
		// determine a suitable location for the attacker to move to
		//-----------------------------------------------------------------//
		while(unprocessed)
		{
			//-----------------------------------------------------//
			// find a reachable location, or not searched location
			//-----------------------------------------------------//
			if(!analyseResult)
			{
				handle_attack_target_totally_blocked(targetXLoc, targetYLoc, townRecno, selectedUnitArray, selectedCount, 3);
				
				for(count=0; count<ATTACK_DIR; count++)
					mem_del(dir_array_ptr[count]);

				mem_del(unit_processed_array);
				return;
			}
			else
			{
				for(i=0; i<surroundLoc; i++)
				{
					if((++destCount)>=surroundLoc)
					{
						destCount = 0;
						xOffsetPtr = get_target_x_offset(STD_TOWN_LOC_WIDTH, STD_TOWN_LOC_HEIGHT, count);
						yOffsetPtr = get_target_y_offset(STD_TOWN_LOC_WIDTH, STD_TOWN_LOC_HEIGHT, count);
						xOffset = *xOffsetPtr;
						yOffset = *yOffsetPtr;
					}
					else
					{
						xOffset = *(++xOffsetPtr);
						yOffset = *(++yOffsetPtr);
					}

					if(!unreachable_table[xOffset+SHIFT_ADJUST][yOffset+SHIFT_ADJUST])
						break;
				}
			}

			err_when(unreachable_table[xOffset+SHIFT_ADJUST][yOffset+SHIFT_ADJUST]);

			//------------------------------------------------------------//
			// find the closest attacker
			//------------------------------------------------------------//
			err_when(unprocessed>2000 || unprocessed <0);
			xLoc = targetXLoc + xOffset;
			yLoc = targetYLoc + yOffset;
			err_when(!world.get_loc(xLoc, yLoc)->can_move(unitPtr->mobile_type));

			for(i=0, minDist=0x7FFFFFF; i<unprocessed; i++)
			{
				unitPtr = (Unit*) get_ptr(curArrayPtr[i]);
				xDist = abs(xLoc-unitPtr->next_x_loc());
				yDist = abs(yLoc-unitPtr->next_y_loc());
				dist = (xDist >= yDist) ? xDist*10+yDist : yDist*10+xDist;

				if(dist < minDist)
				{
					minDist = dist;
					unitPos = i;
				}
			}

			unitPtr = (Unit*) get_ptr(curArrayPtr[unitPos]);
			curArrayPtr[unitPos] = curArrayPtr[--unprocessed]; // move the units in the back to the front

			err_when(unitPtr->action_mode2!=ACTION_ATTACK_WALL &&
						unitPtr->cur_action==SPRITE_ATTACK && unitPtr->action_para==0);
			seek_path.set_status(PATH_WAIT);
			err_when(seek_path.path_status==PATH_NODE_USED_UP);
			unitPtr->attack_town(targetXLoc, targetYLoc, xOffset, yOffset);

			//------------------------------------------------------------//
			// set the flag if unreachable
			//------------------------------------------------------------//
			if(seek_path.path_status==PATH_NODE_USED_UP)
			{
				unreachable_table[xOffset+SHIFT_ADJUST][yOffset+SHIFT_ADJUST] = 1;
				analyseResult--;
				debug_result_check(analyseResult, STD_TOWN_LOC_WIDTH, STD_TOWN_LOC_HEIGHT);
				err_when(analyseResult<0);

				//------------------------------------------------------------//
				// the nearby location should also be unreachable
				//------------------------------------------------------------//
				check_nearby_location(targetXLoc, targetYLoc, xOffset, yOffset, STD_TOWN_LOC_WIDTH, STD_TOWN_LOC_HEIGHT,
											 unitPtr->mobile_type, analyseResult);
			}

			update_unreachable_table(targetXLoc, targetYLoc, STD_TOWN_LOC_WIDTH, STD_TOWN_LOC_HEIGHT, unitPtr->mobile_type, analyseResult);

			#ifdef DEBUG
				Location *dlPtr;
				for(int di=0; di<STD_TOWN_LOC_WIDTH+2; di++)
				{
					for(int dj=0; dj<STD_TOWN_LOC_HEIGHT+2; dj++)
					{
						if(di>=1 && di<=STD_TOWN_LOC_WIDTH && dj>=1 && dj<=STD_TOWN_LOC_HEIGHT)
							continue;

						int debugXLoc = targetXLoc+di-SHIFT_ADJUST;
						int debugYLoc = targetYLoc+dj-SHIFT_ADJUST;
						if(debugXLoc<0 || debugXLoc>=MAX_WORLD_X_LOC || debugYLoc<0 || debugYLoc>=MAX_WORLD_Y_LOC)
							continue;

						dlPtr = world.get_loc(debugXLoc, debugYLoc);
						
						err_when(!dlPtr->can_move(unitPtr->mobile_type) && unreachable_table[di][dj]==0);
					}
				}
			#endif
		}
	}

	//---------------------------------------------------------------------//
	// set the unreachable flag for each units
	//---------------------------------------------------------------------//
	//-************** codes here ***************-//

	//---------------------------------------------------------------------//
	// destruct data structure
	//---------------------------------------------------------------------//
	for(count=0; count<ATTACK_DIR; count++)
		mem_del(dir_array_ptr[count]);

	mem_del(unit_processed_array);
}
//----------- End of function UnitArray::attack_town -----------//


//--------- Begin of function UnitArray::attack_wall ---------//
// <int> targetXLoc	- x location of the wall
// <int>	targetYLoc	- y location of the wall
//
void UnitArray::attack_wall(int targetXLoc, int targetYLoc, short* selectedUnitArray, int selectedCount)
{
	err_when(selectedCount<=0);
	if(selectedCount==1)
	{
		Unit *unitPtr = (Unit*) get_ptr(selectedUnitArray[0]);
		unitPtr->unit_group_id = unit_array.cur_group_id++;
		unitPtr->attack_wall(targetXLoc, targetYLoc);
		return;
	}

	//********************** improve later begin **************************//
	//---------------------------------------------------------------------//
	// codes for differnt territory or different mobile_type attacking should
	// be added in the future.
	//---------------------------------------------------------------------//
	Unit	*firstUnitPtr = unit_array[selectedUnitArray[0]];
	if(world.get_loc(targetXLoc, targetYLoc)->region_id !=
		world.get_loc(firstUnitPtr->next_x_loc(), firstUnitPtr->next_y_loc())->region_id)
	{
		Unit *unitPtr;
		set_group_id(selectedUnitArray, selectedCount);

		for(int i=0; i<selectedCount; i++)
		{
			unitPtr = (Unit*) get_ptr(selectedUnitArray[i]);
			unitPtr->attack_wall(targetXLoc, targetYLoc);
		}
		return;
	}
	//*********************** improve later end ***************************//

	//----------- initialize local parameters ------------//
	char	*xOffsetPtr, *yOffsetPtr;

	//---------------------------------------------------------------------//
	// construct data structure
	//---------------------------------------------------------------------//
	int tempVar = sizeof(short)*selectedCount;
	
	memset(dir_array_count, 0, sizeof(dir_array_count));
	int count;
	for(count=0; count<ATTACK_DIR; count++)
	{
		dir_array_ptr[count] = (short*) mem_add(tempVar);
		memset(dir_array_ptr[count], 0, tempVar);
	}

	unit_processed_array = (short *)mem_add(tempVar);
	unit_processed_count = 0;

	//---------------------------------------------------------------------//
	// divide the units into each region
	//---------------------------------------------------------------------//
	Unit		*unitPtr = (Unit*) get_ptr(selectedUnitArray[0]);
	err_when(!unitPtr);
	DWORD groupId = unit_array.cur_group_id++;
	arrange_units_in_group(targetXLoc, targetYLoc, targetXLoc, targetYLoc, selectedUnitArray, selectedCount, groupId, 0);

	//---------------------------------------------------------------------//
	// now the attackers are divided into 8 groups to attack the target
	//---------------------------------------------------------------------//
	int	xLoc, yLoc;			// actual target surrounding location to move to
	int	xOffset, yOffset;	// offset location of the target
	int	unprocessed;		// number of units in this group
	int	dist, xDist, yDist, minDist;
	int	destCount;
	short unitPos;				// store the position of the unit with minDist
	short	*curArrayPtr;
	char	surroundLoc = get_target_surround_loc(1, 1);

	int i;
	for(i=0; i<MAX_UNIT_SURROUND_SIZE; i++)
		memset(unreachable_table[i], 0, sizeof(char)*MAX_UNIT_SURROUND_SIZE);

	//---------------------------------------------------------------------//
	// analyse the surrounding location of the target
	//---------------------------------------------------------------------//
	int analyseResult = analyse_surround_location(targetXLoc, targetYLoc, 1, 1, unitPtr->mobile_type);
	debug_result_check(analyseResult, 1, 1);
	err_when(analyseResult<0);

	if(!analyseResult) // 0 if all surround location is not accessible
	{
		//------------------------------------------------------------//
		// special handling for this case
		//------------------------------------------------------------//
		handle_attack_target_totally_blocked(targetXLoc, targetYLoc, 0, selectedUnitArray, selectedCount, 0);

		for(count=0; count<ATTACK_DIR; count++)
			mem_del(dir_array_ptr[count]);

		mem_del(unit_processed_array);
		return;
	}

	//---------------------------------------------------------------------//
	// let the units move to the rest accessible location
	//---------------------------------------------------------------------//
	for(count=0; count<ATTACK_DIR; count++) // for each array/group
	{
		//--------- initialize for each group --------//
		unprocessed = dir_array_count[count];	// get the number of units in this region
		curArrayPtr = dir_array_ptr[count];	// get the recno of units in this region
		destCount = surroundLoc-1;
		xOffsetPtr = get_target_x_offset(1, 1, count);
		yOffsetPtr = get_target_y_offset(1, 1, count);
		xOffset = *xOffsetPtr;
		yOffset = *yOffsetPtr;

		//-----------------------------------------------------------------//
		// determine a suitable location for the attacker to move to
		//-----------------------------------------------------------------//
		while(unprocessed)
		{
			//-----------------------------------------------------//
			// find a reachable location, or not searched location
			//-----------------------------------------------------//
			if(!analyseResult)
			{
				handle_attack_target_totally_blocked(targetXLoc, targetYLoc, 0, selectedUnitArray, selectedCount, 0);
				
				for(count=0; count<ATTACK_DIR; count++)
					mem_del(dir_array_ptr[count]);

				mem_del(unit_processed_array);
				return;
			}
			else
			{
				for(i=0; i<surroundLoc; i++)
				{
					if((++destCount)>=surroundLoc)
					{
						destCount = 0;
						xOffsetPtr = get_target_x_offset(1, 1, count);
						yOffsetPtr = get_target_y_offset(1, 1, count);
						xOffset = *xOffsetPtr;
						yOffset = *yOffsetPtr;
					}
					else
					{
						xOffset = *(++xOffsetPtr);
						yOffset = *(++yOffsetPtr);
					}

					if(!unreachable_table[xOffset+SHIFT_ADJUST][yOffset+SHIFT_ADJUST])
						break;
				}
			}

			err_when(unreachable_table[xOffset+SHIFT_ADJUST][yOffset+SHIFT_ADJUST]);

			//------------------------------------------------------------//
			// find the closest attacker
			//------------------------------------------------------------//
			err_when(unprocessed>2000 || unprocessed <0);
			xLoc = targetXLoc + xOffset;
			yLoc = targetYLoc + yOffset;
			err_when(!world.get_loc(xLoc, yLoc)->can_move(unitPtr->mobile_type));

			for(i=0, minDist=0x7FFFFFF; i<unprocessed; i++)
			{
				unitPtr = (Unit*) get_ptr(curArrayPtr[i]);
				xDist = abs(xLoc-unitPtr->next_x_loc());
				yDist = abs(yLoc-unitPtr->next_y_loc());
				dist = (xDist >= yDist) ? xDist*10+yDist : yDist*10+xDist;

				if(dist < minDist)
				{
					minDist = dist;
					unitPos = i;
				}
			}

			unitPtr = (Unit*) get_ptr(curArrayPtr[unitPos]);
			curArrayPtr[unitPos] = curArrayPtr[--unprocessed]; // move the units in the back to the front

			err_when(unitPtr->action_mode2!=ACTION_ATTACK_WALL &&
						unitPtr->cur_action==SPRITE_ATTACK && unitPtr->action_para==0);
			seek_path.set_status(PATH_WAIT);
			err_when(seek_path.path_status==PATH_NODE_USED_UP);
			unitPtr->attack_wall(targetXLoc, targetYLoc, xOffset, yOffset);

			//------------------------------------------------------------//
			// set the flag if unreachable
			//------------------------------------------------------------//
			if(seek_path.path_status==PATH_NODE_USED_UP)
			{
				unreachable_table[xOffset+SHIFT_ADJUST][yOffset+SHIFT_ADJUST] = 1;
				analyseResult--;
				debug_result_check(analyseResult, 1, 1);
				err_when(analyseResult<0);
				//------------------------------------------------------------//
				// the nearby location should also be unreachable
				//------------------------------------------------------------//
				check_nearby_location(targetXLoc, targetYLoc, xOffset, yOffset, 1, 1, unitPtr->mobile_type, analyseResult);
			}

			update_unreachable_table(targetXLoc, targetYLoc, 1, 1, unitPtr->mobile_type, analyseResult);

			#ifdef DEBUG
				for(int di=0; di<3; di++)
				{
					for(int dj=0; dj<3; dj++)
					{
						if(di==1 && dj==1)
							continue;

						int debugXLoc = targetXLoc+di-SHIFT_ADJUST;
						int debugYLoc = targetYLoc+dj-SHIFT_ADJUST;
						if(debugXLoc<0 || debugXLoc>=MAX_WORLD_X_LOC || debugYLoc<0 || debugYLoc>=MAX_WORLD_Y_LOC)
							continue;

						Location *dlPtr = world.get_loc(debugXLoc, debugYLoc);
						
						err_when(!dlPtr->can_move(unitPtr->mobile_type) && unreachable_table[di][dj]==0);
					}
				}
			#endif
		}
	}

	//---------------------------------------------------------------------//
	// set the unreachable flag for each units
	//---------------------------------------------------------------------//
	//-************** codes here ***************-//

	//---------------------------------------------------------------------//
	// destruct data structure
	//---------------------------------------------------------------------//
	for(count=0; count<ATTACK_DIR; count++)
		mem_del(dir_array_ptr[count]);

	mem_del(unit_processed_array);
}
//----------- End of function UnitArray::attack_wall -----------//


//--------- Begin of function UnitArray::arrange_units_in_group ---------//
// <int>		xLoc1					- top left x location of the target
// <int>		yLoc1					- top left y location of the target
// <int>		xLoc2					- bottom right x location of the target
// <int>		yLoc2					- bottom right y location of the target
// <short*> selectedUnitArray	- recno. of selected units
// <int>		selectedCount		- count of selected units
// <DWORD>	unitGroupId			- group id for the selected units
//
// <int> targetType	- 0 for wall, 1 for unit, 2 for firm, 3 for town
//
void UnitArray::arrange_units_in_group(int xLoc1, int yLoc1, int xLoc2, int yLoc2, short* selectedUnitArray,
													int selectedCount, DWORD unitGroupId, int targetType)
{
	Unit	*unitPtr;
	int	curXLoc, curYLoc;

	for(int i=0; i<selectedCount; i++)
	{
		unitPtr = (Unit*) get_ptr(selectedUnitArray[i]);
		err_when(!unitPtr);

		unitPtr->unit_group_id = unitGroupId; // set unit_group_id
		err_when(unitPtr->cur_action==SPRITE_IDLE && (unitPtr->cur_x!=unitPtr->next_x || unitPtr->cur_y!=unitPtr->next_y));
		if(unitPtr->cur_action==SPRITE_IDLE)
			unitPtr->set_ready();

		curXLoc = unitPtr->next_x_loc();
		curYLoc = unitPtr->next_y_loc();
		if(curXLoc>=xLoc1-1 && curXLoc<=xLoc2+1 && curYLoc>=yLoc1-1 && curYLoc<=yLoc2+1)
		{
			//------------- already in the target surrounding ----------------//
			switch(targetType)
			{
				case 0:	unitPtr->attack_wall(xLoc1, yLoc1);
							break;

				case 1:	unitPtr->attack_unit(xLoc1, yLoc1);
							break;

				case 2:	unitPtr->attack_firm(xLoc1, yLoc1);
							break;

				case 3:	unitPtr->attack_town(xLoc1, yLoc1);
							break;
			}
			continue;
		}

		//---- the attacker need to call searching to reach the target ----//
		if(curXLoc < xLoc1)
		{
			if(curYLoc < yLoc1)		// 8
				dir_array_ptr[7][dir_array_count[7]++] = selectedUnitArray[i];
			else if(curYLoc > yLoc2)// 2
				dir_array_ptr[1][dir_array_count[1]++] = selectedUnitArray[i];
			else							// 1
				dir_array_ptr[0][dir_array_count[0]++] = selectedUnitArray[i];
		}
		else if(curXLoc > xLoc2)
		{
			if(curYLoc < yLoc1)		// 6
				dir_array_ptr[5][dir_array_count[5]++] = selectedUnitArray[i];
			else if(curYLoc > yLoc2)// 4
				dir_array_ptr[3][dir_array_count[3]++] = selectedUnitArray[i];
			else							// 5
				dir_array_ptr[4][dir_array_count[4]++] = selectedUnitArray[i];
		}
		else // curXLoc==targetXLoc2
		{
			if(curYLoc < yLoc1)		// 7
				dir_array_ptr[6][dir_array_count[6]++] = selectedUnitArray[i];
			else if(curYLoc > yLoc2)// 3
				dir_array_ptr[2][dir_array_count[2]++] = selectedUnitArray[i];
			else // curXLoc==xLoc2 && curYLoc==yLoc2
			{
				// target is one of the selected unit, error
				err_here();
			}
		}
	}
}
//----------- End of function UnitArray::arrange_units_in_group -----------//


//--------- Begin of function UnitArray::analyse_surround_location ---------//
// return the number of accessible surrounding location of the target
//
// <int>		targetXLoc		- target x location
// <int>		targetYLoc		- target y location
// <int>		targetWidth		- target width
// <int>		targetHeight	- target height
// <char>	mobileType		- target mobile type
//
int UnitArray::analyse_surround_location(int targetXLoc, int targetYLoc, int targetWidth, int targetHeight, char mobileType)
{
	static char xIncreTable[4] = {  1,  0, -1,  0};
	static char yIncreTable[4] = {  0,  1,  0, -1};

	err_when(targetWidth<1 || targetWidth>4);
	Location *locPtr;
	int xLoc = targetXLoc-1;
	int yLoc = targetYLoc-1;
	int targetXLoc2 = targetXLoc + targetWidth - 1;
	int targetYLoc2 = targetYLoc + targetHeight - 1;
	int bound = 2*(targetWidth + targetHeight) + 4;  // (x+2)*(y+2) - xy
	int increCount=4, xIncre, yIncre, found=0;

	err_when(targetWidth==3 && targetHeight==3 && bound!=16);

	for(int i=0; i<bound; i++)
	{
		if(xLoc<0 || xLoc>=MAX_WORLD_X_LOC || yLoc<0 || yLoc>=MAX_WORLD_Y_LOC)
			unreachable_table[xLoc-targetXLoc+SHIFT_ADJUST][yLoc-targetYLoc+SHIFT_ADJUST] = 1;
		else
		{
			locPtr = world.get_loc(xLoc, yLoc);
			if(!locPtr->can_move(mobileType))
				unreachable_table[xLoc-targetXLoc+SHIFT_ADJUST][yLoc-targetYLoc+SHIFT_ADJUST] = 1;
			else
				found++;
		}

		if((xLoc==targetXLoc-1 || xLoc==targetXLoc2+1) && (yLoc==targetYLoc-1 || yLoc==targetYLoc2+1)) // at the corner
		{
			if((++increCount)>=4)
				increCount = 0;

			xIncre = xIncreTable[increCount];
			yIncre = yIncreTable[increCount];
		}

		xLoc += xIncre;
		yLoc += yIncre;
	}

	return found;
}
//----------- End of function UnitArray::analyse_surround_location -----------//


//--------- Begin of function UnitArray::check_nearby_location ---------//
// check the target location to find out how many of its surrounding location
// is not blocked
//
// <int>		targetXLoc			- target x location
// <int>		targetYLoc			- target y location
// <char>	xOffset				- x offset from target x location
// <char>	yOffset				- y offset from target y location
// <int>		targetWidth			- target width
// <int>		targetHeight		- target height
// <char>	targetMobileType	- target mobile type
// <int&>	analyseResult		- reference for returning
//
void UnitArray::check_nearby_location(int targetXLoc, int targetYLoc, char xOffset, char yOffset,
												  int targetWidth, int targetHeight, char targetMobileType, int& analyseResult)
{
	#ifdef DEBUG
		int backupAnalyseResult = analyseResult;
		char debugUnreachableTable[MAX_UNIT_SURROUND_SIZE][MAX_UNIT_SURROUND_SIZE];
		memcpy(debugUnreachableTable, unreachable_table, sizeof(char)*MAX_UNIT_SURROUND_SIZE*MAX_UNIT_SURROUND_SIZE);
	#endif
	debug_result_check(analyseResult, targetWidth, targetHeight);

	static char leftXIncreTable[4] = {  1,  0, -1,  0};
	static char leftYIncreTable[4] = {  0,  1,  0, -1};
	static char rightXIncreTable[4] = { -1,  0,  1,  0};
	static char rightYIncreTable[4] = {  0,  1,  0, -1};

	err_when(targetWidth<1 || targetWidth>4);
	Location *locPtr;
	int targetXLoc2 = targetXLoc + targetWidth - 1;
	int targetYLoc2 = targetYLoc + targetHeight - 1;
	int bound = 2*(targetWidth + targetHeight) + 4;  // (x+2)*(y+2) - xy
	
	int leftXLoc ,leftYLoc, leftContinue=1;
	int leftXIncre, leftYIncre, leftIncreCount;
	int rightXLoc, rightYLoc, rightContinue=1;
	int rightXIncre, rightYIncre, rightIncreCount=1;

	bool haveValidSituation = true;

	//-------------------------------------------------------------------------------------//
	// determine the initial situation
	//-------------------------------------------------------------------------------------//
	if((xOffset==-1 || xOffset==targetWidth) && (yOffset==-1 || yOffset==targetHeight)) // at the corner
	{
		if(xOffset==-1)
		{
			if(yOffset==-1) // upper left corner
			{
				leftXIncre = 1;
				leftYIncre = 0;
				leftIncreCount = 0;

				rightXIncre = 0;
				rightYIncre = 1;
				rightIncreCount = 1;
			}
			else	// lower left corner
			{
				leftXIncre = 0;
				leftYIncre = -1;
				leftIncreCount = 3;

				rightXIncre = 1;
				rightYIncre = 0;
				rightIncreCount = 2;
			}
		}
		else
		{
			if(yOffset==-1) // upper right corner
			{
				leftXIncre = 0;
				leftYIncre = 1;
				leftIncreCount = 1;

				rightXIncre = -1;
				rightYIncre = 0;
				rightIncreCount = 0;
			}
			else // lower right corner
			{
				leftXIncre = -1;
				leftYIncre = 0;
				leftIncreCount = 2;

				rightXIncre = 0;
				rightYIncre = -1;
				rightIncreCount = 3;
			}
		}
	}
	else // at the edge
	{
		if(xOffset==-1) // left edge
		{
			leftXIncre = 0;
			leftYIncre = -1;
			leftIncreCount = 3;

			rightXIncre = 0;
			rightYIncre = 1;
			rightIncreCount = 1;
		}
		else if(xOffset==targetWidth) // right edge
		{
			leftXIncre = 0;
			leftYIncre = 1;
			leftIncreCount = 1;

			rightXIncre = 0;
			rightYIncre = -1;
			rightIncreCount = 3;
		}
		else if(yOffset==-1) // upper edge
		{
			leftXIncre = 1;
			leftYIncre = 0;
			leftIncreCount = 0;

			rightXIncre = -1;
			rightYIncre = 0;
			rightIncreCount = 0;
		}
		else if(yOffset==targetHeight) // lower edge
		{
			leftXIncre = -1;
			leftYIncre = 0;
			leftIncreCount = 2;

			rightXIncre = 1;
			rightYIncre = 0;
			rightIncreCount = 2;
		}
		else {
			haveValidSituation = false;
		}
	}

	err_when( !haveValidSituation );

	leftXLoc = rightXLoc = targetXLoc + xOffset;
	leftYLoc = rightYLoc = targetYLoc + yOffset;
	int canReach;
	int outBoundary; // true if out of map boundary

	//-------------------------------------------------------------------------------------//
	// count the reachable location
	//-------------------------------------------------------------------------------------//
	for(int i=1; i<bound; i++) // exclude the starting location
	{
		debug_result_check(analyseResult, targetWidth, targetHeight);
		#ifdef DEBUG
			int debugLeftXLoc = leftXLoc;
			int debugLeftYloc = leftYLoc;
			int debugRightXLoc = rightXLoc;
			int debugRightYLoc = rightYLoc;
			int debugLeftIncreCount = leftIncreCount;
			int debugRightIncreCount = rightIncreCount;
			char debugUnreachableTable2[MAX_UNIT_SURROUND_SIZE][MAX_UNIT_SURROUND_SIZE];
			memcpy(debugUnreachableTable2, unreachable_table, sizeof(char)*MAX_UNIT_SURROUND_SIZE*MAX_UNIT_SURROUND_SIZE);
			for(int k=0; k<MAX_UNIT_SURROUND_SIZE; k++)
			{
				for(int j=0; j<MAX_UNIT_SURROUND_SIZE; j++)
				{
					if(debugUnreachableTable2[k][j] && debugUnreachableTable2[k][j]==1)
						debugUnreachableTable2[k][j] = 2;	// plus 1 to distinguish the original table
				}
			}
		#endif

		//------------------------------------------------------------//
		// process left hand side checking
		//------------------------------------------------------------//
		if(leftContinue)
		{
			canReach = 0;
			outBoundary = 0;

			leftXLoc += leftXIncre;
			leftYLoc += leftYIncre;
			if((leftXLoc==targetXLoc-1 || leftXLoc==targetXLoc2+1) && (leftYLoc==targetYLoc-1 || leftYLoc==targetYLoc2+1))
			{
				if((++leftIncreCount)>=4)
					leftIncreCount = 0;

				leftXIncre = leftXIncreTable[leftIncreCount];
				leftYIncre = leftYIncreTable[leftIncreCount];
			}
			
			if(leftXLoc>=0 && leftXLoc<MAX_WORLD_X_LOC && leftYLoc>=0 && leftYLoc<MAX_WORLD_Y_LOC)
			{
				if(unreachable_table[leftXLoc-targetXLoc+SHIFT_ADJUST][leftYLoc-targetYLoc+SHIFT_ADJUST])
					canReach = 1; // concept incorrect, but it is used to terminate this part of checking
				else
				{
					locPtr = world.get_loc(leftXLoc, leftYLoc);
					if(locPtr->can_move(targetMobileType))
						canReach = 1;
				}
			}
			else
				outBoundary = 1;

			if(canReach)
				leftContinue = 0;
			else if(!outBoundary)
			{
				err_when(unreachable_table[leftXLoc-targetXLoc+SHIFT_ADJUST][leftYLoc-targetYLoc+SHIFT_ADJUST]);
				unreachable_table[leftXLoc-targetXLoc+SHIFT_ADJUST][leftYLoc-targetYLoc+SHIFT_ADJUST] = 1;
				analyseResult--;
				debug_result_check(analyseResult, targetWidth, targetHeight);
				err_when(analyseResult<0);
			}
			#ifdef DEBUG
				else
					err_when(!unreachable_table[leftXLoc-targetXLoc+SHIFT_ADJUST][leftYLoc-targetYLoc+SHIFT_ADJUST]);
			#endif

			i++;
		}

		//------------------------------------------------------------//
		// process right hand side checking
		//------------------------------------------------------------//
		if(rightContinue)
		{
			canReach = 0;
			outBoundary = 0;

			rightXLoc += rightXIncre;
			rightYLoc += rightYIncre;
			if((rightXLoc==targetXLoc-1 || rightXLoc==targetXLoc2+1) && (rightYLoc==targetYLoc-1 || rightYLoc==targetYLoc2+1))
			{
				if((++rightIncreCount)>=4)
					rightIncreCount = 0;

				rightXIncre = rightXIncreTable[rightIncreCount];
				rightYIncre = rightYIncreTable[rightIncreCount];
			}
			
			if(rightXLoc>=0 && rightXLoc<MAX_WORLD_X_LOC && rightYLoc>=0 && rightYLoc<MAX_WORLD_Y_LOC)
			{
				if(unreachable_table[rightXLoc-targetXLoc+SHIFT_ADJUST][rightYLoc-targetYLoc+SHIFT_ADJUST])
					canReach = 1; // concept incorrect, but it is used to terminate this part of checking
				else
				{
					locPtr = world.get_loc(rightXLoc, rightYLoc);
					if(locPtr->can_move(targetMobileType))
						canReach = 1;
				}
			}
			else
				outBoundary = 1;

			if(canReach)
				rightContinue = 0;
			else if(!outBoundary)
			{
				err_when(unreachable_table[rightXLoc-targetXLoc+SHIFT_ADJUST][rightYLoc-targetYLoc+SHIFT_ADJUST]);
				unreachable_table[rightXLoc-targetXLoc+SHIFT_ADJUST][rightYLoc-targetYLoc+SHIFT_ADJUST] = 1;
				analyseResult--;
				debug_result_check(analyseResult, targetWidth, targetHeight);
				err_when(analyseResult<0);
			}
			#ifdef DEBUG
				else
					err_when(!unreachable_table[rightXLoc-targetXLoc+SHIFT_ADJUST][rightYLoc-targetYLoc+SHIFT_ADJUST]);
			#endif
		}

		if(!leftContinue && !rightContinue)
			break;
	}
}
//----------- End of function UnitArray::check_nearby_location -----------//


//--------- Begin of function UnitArray::handle_attack_target_totally_blocked ---------//
// handle attacking while the target is totally blocked
//
// <int>		targetXLoc			- target x loc
// <int>		targetYLoc			- target y loc
// <short>	targetRecno			- target recno
// <short*>	selectedUnitArray	- selected units' recno
// <short>	selectedCount		- num of selected unit
// <int>		targetType			- 0 for wall, 1 for unit, 2 for firm, 3 for town
//
void UnitArray::handle_attack_target_totally_blocked(int targetXLoc, int targetYLoc, short targetRecno,
																	  short *selectedUnitArray, short selectedCount, int targetType)
{
	if(unit_processed_count>0) // some units can reach the target surrounding
	{
		Unit	*processedPtr, *unitPtr;
		int	proCount = unit_processed_count - 1;
		int	unproCount = selectedCount - proCount - 1; // number of unprocessed
		int	sCount = selectedCount-1;
		int	found, i, recno;
		#ifdef DEBUG
			int debugCount;
		#endif

		//------------------------------------------------------------------------------------//
		// use the result of those processed units as a reference of those unprocessed units
		//------------------------------------------------------------------------------------//
		while(unproCount)
		{
			err_when(unit_array.is_deleted(unit_processed_array[proCount]));
			processedPtr = (Unit*) get_ptr(unit_processed_array[proCount]);
			#ifdef DEBUG
				debugCount = 0;
			#endif
			
			err_when(sCount<0);
			do
			{
				#ifdef DEBUG
					debugCount++;
					err_when(debugCount>1000);
				#endif

				found = 0;
				recno = selectedUnitArray[sCount];
				for(i=0; i<unit_processed_count; i++)
				{
					if(unit_processed_array[i]==recno)
					{
						found++;
						break;
					}
				}
				
				err_when(sCount<0 || sCount>selectedCount);
				sCount--;
			}while(found);
			
			unitPtr = (Unit *) get_ptr(recno);
			unitPtr->move_to(processedPtr->move_to_x_loc, processedPtr->move_to_y_loc);

			switch(targetType)
			{
				case 0:	// wall
							unitPtr->action_mode = unitPtr->action_mode2 = ACTION_ATTACK_WALL;
							err_when(targetRecno);
							break;

				case 1:	// unit
							unitPtr->action_mode = unitPtr->action_mode2 = ACTION_ATTACK_UNIT;
							err_when(!targetRecno);
							break;

				case 2:	// firm
							unitPtr->action_mode = unitPtr->action_mode2 = ACTION_ATTACK_FIRM;
							err_when(!targetRecno);
							break;

				case 3:	// town
							unitPtr->action_mode = unitPtr->action_mode2 = ACTION_ATTACK_TOWN;
							err_when(!targetRecno);
							break;
			}
			unitPtr->action_para = unitPtr->action_para2 = targetRecno;
			unitPtr->action_x_loc = unitPtr->action_x_loc2 = targetXLoc;
			unitPtr->action_y_loc = unitPtr->action_y_loc2 = targetYLoc;

			proCount--;
			if(proCount<0)
				proCount = unit_processed_count - 1;
			
			unproCount--;
		}
	}
	else	// none of the units reaches the target surrounding
	{
		//----------------------------------------------------------------//
		// handle the case for 1x1 units now, no 2x2 units
		//----------------------------------------------------------------//
		//-*********** improve later ************-//
		int unprocessed = selectedCount;
		Unit *firstPtr = (Unit *) get_ptr(selectedUnitArray[unprocessed-1]);

		switch(targetType)
		{
			case 0:	firstPtr->attack_wall(targetXLoc, targetYLoc);
						break;

			case 1:	firstPtr->attack_unit(targetXLoc, targetYLoc);
						break;

			case 2:	firstPtr->attack_firm(targetXLoc, targetYLoc);
						break;

			case 3:	firstPtr->attack_town(targetXLoc, targetYLoc);
						break;
		}

		int moveToXLoc = firstPtr->move_to_x_loc;
		int moveToYLoc = firstPtr->move_to_y_loc;

		/*if(seek_path.path_status==PATH_NODE_USED_UP)
		{
			int debug = 0;
		}*/

		Unit *unitPtr;

		while(unprocessed)
		{
			unitPtr = (Unit *) get_ptr(selectedUnitArray[unprocessed-1]);
			unitPtr->move_to(moveToXLoc, moveToYLoc);

			switch(targetType)
			{
				case 0:	// wall
							unitPtr->action_mode = unitPtr->action_mode2 = ACTION_ATTACK_WALL;
							unitPtr->action_para = unitPtr->action_para2 = 0;
							break;

				case 1:	// unit
							unitPtr->action_mode = unitPtr->action_mode2 = ACTION_ATTACK_UNIT;
							unitPtr->action_para = unitPtr->action_para2 = targetRecno;
							break;

				case 2:	// firm
							unitPtr->action_mode = unitPtr->action_mode2 = ACTION_ATTACK_FIRM;
							unitPtr->action_para = unitPtr->action_para2 = targetRecno;
							break;

				case 3:	// town
							unitPtr->action_mode = unitPtr->action_mode2 = ACTION_ATTACK_TOWN;
							unitPtr->action_para = unitPtr->action_para2 = targetRecno;
							break;
			}
			unitPtr->action_x_loc = unitPtr->action_x_loc2 = targetXLoc;
			unitPtr->action_y_loc = unitPtr->action_y_loc2 = targetYLoc;
			
			unprocessed--;
		}
	}
}
//----------- End of function UnitArray::handle_attack_target_totally_blocked -----------//
