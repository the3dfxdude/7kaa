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

//Filename    : OUNITAAC.CPP
//Description : Object UnitArray - misc functions

#include <math.h>

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

//--------- Begin of function UnitArray::divide_array ---------//
// divide units into arrays by their mobile_type 
//
// <int> excludeSelectedLocUnit	-	(default 0)
//
void UnitArray::divide_array(int locX, int locY, short* selectedArray, int selectedCount, int excludeSelectedLocUnit)
{
	err_when( selectedCount < 0 );

	selected_land_unit_count = 0;
	selected_sea_unit_count = 0;
	selected_air_unit_count = 0;
	selected_land_unit_array = NULL;
	selected_sea_unit_array = NULL;
	selected_air_unit_array = NULL;

	int unitRecno;
	if(excludeSelectedLocUnit)
	{
		Location *locPtr = world.get_loc(locX, locY);
		short targetMobileType = locPtr->has_any_unit();
		unitRecno = targetMobileType ? locPtr->unit_recno(targetMobileType) : 0;
	}
	
	selected_land_unit_array = (short*) mem_add(sizeof(short)*selectedCount);
	selected_sea_unit_array = (short*) mem_add(sizeof(short)*selectedCount);
	selected_air_unit_array = (short*) mem_add(sizeof(short)*selectedCount);
	memset(selected_land_unit_array, 0, sizeof(short)*selectedCount);
	memset(selected_sea_unit_array, 0, sizeof(short)*selectedCount);
	memset(selected_air_unit_array, 0, sizeof(short)*selectedCount);

	Unit *unitPtr;
	int curRecno;
	for(int i=0; i<selectedCount; i++)
	{
		curRecno = selectedArray[i];
		if(excludeSelectedLocUnit && curRecno==unitRecno)
			continue;

		unitPtr = (Unit*) get_ptr(curRecno);
		switch(unitPtr->mobile_type)
		{
			case UNIT_LAND:
					selected_land_unit_array[selected_land_unit_count++] = curRecno;
					break;

			case UNIT_SEA:
					selected_sea_unit_array[selected_sea_unit_count++] = curRecno;
					break;

			case UNIT_AIR:
					selected_air_unit_array[selected_air_unit_count++] = curRecno;
					break;

			default:	err_here();
						break;
		}
	}
}
//----------- End of function UnitArray::divide_array -----------//	


//--------- Begin of function UnitArray::set_group_id ---------//
// Set group id for the selected units
//
void UnitArray::set_group_id(short* selectedArray, int selectedCount)
{
	//-------------------------------------------------//
	// set unit cur_action and unit_group_id
	//-------------------------------------------------//
	uint32_t curGroupId = unit_array.cur_group_id++;

	for(int j=0; j<selectedCount; j++)
	{
		Unit* unitPtr = (Unit*) get_ptr(selectedArray[j]);
		err_when(!unitPtr);
		err_when(!unitPtr->is_visible());
		err_when(unitPtr->hit_points<=0);

		unitPtr->unit_group_id = curGroupId;
		err_if(unitPtr->cur_action==SPRITE_IDLE && (unitPtr->cur_x!=unitPtr->next_x || unitPtr->cur_y!=unitPtr->next_y))
			err_here();

		if( unitPtr->cur_action == SPRITE_IDLE )				//**maybe need to include SPRITE_ATTACK as well
			unitPtr->set_ready();
	}
}
//----------- End of function UnitArray::set_group_id -----------//	


//--------- Begin of function UnitArray::stop ---------//
//
// <short*> selectedUnitArray  - an array of recno of selected units.
//											(the array should be terminated with 0)
// <int>    selectedCount 		 - no. of selected units.
// [char]   remoteAction 	    - whether this is an action carried out by a remote machine or not.
//											(default: 0)
//
void UnitArray::stop(short* selectedUnitArray, int selectedCount, char remoteAction)
{
	//-------- if it's a multiplayer game --------//
	//
	// Queue the action.
	//
	// Local: this action will be processed when all action messages of all
	//			 remote players are ready from the next frame.
	//
	// Remote: this action will be processed when the remote has received
	//			  frame sync notification from all other players.
	//
	//--------------------------------------------//

	if( !remoteAction && remote.is_enable() )
	{
		short* shortPtr = (short*) remote.new_send_queue_msg(MSG_UNIT_STOP,
								sizeof(short) * (1+selectedCount) );

		shortPtr[0] = selectedCount;

		memcpy( shortPtr+1, selectedUnitArray, sizeof(short) * selectedCount );
	}
	else
	{
		uint32_t curGroupId = unit_array.cur_group_id++;

		//-------------- stop now ---------------//

		err_when( selectedCount > 10000 );		// error

		Unit* unitPtr;

		for( int i=0 ; i<selectedCount ; i++ )
		{
			unitPtr = (Unit*) get_ptr(selectedUnitArray[i]);
			unitPtr->unit_group_id = curGroupId;
			unitPtr->stop2();
		}
	}
}
//----------- End of function UnitArray::stop -----------//


//--------- Begin of function UnitArray::stop_all_war ---------//
// Stop all the units that is in war with the specified nation
// and stop all the units of the specified nation that is in war
// with other nations
//
// <short>	oldNationReno	-	the nation_renco of the specified nation
//
void UnitArray::stop_all_war(short oldNationRecno)
{
	Unit	*unitPtr, *targetUnit;
	Firm	*targetFirm;
	Town	*targetTown;
	Location	*targetLoc;
	char	targetType;
	short targetRecno;
	int	targetXLoc, targetYLoc;

	for(int i=size(); i>0; --i)
	{
		if(is_deleted(i))
			continue;

		unitPtr = operator[](i);

		if(!unitPtr->is_visible())
			continue;

		if(unitPtr->nation_recno==oldNationRecno)
		{
			//------ stop all attacking unit with nation_recno = oldNationRecno -----//
			if(unitPtr->is_action_attack())
				unitPtr->stop2(KEEP_DEFENSE_MODE);
		}
		else if(unitPtr->is_action_attack())
		{
			//---- stop all attacking unit with target's nation_recno = oldNationRecno ----//
			if(unitPtr->action_mode2==ACTION_AUTO_DEFENSE_ATTACK_TARGET)
			{
				targetType = unitPtr->action_mode;
				targetRecno = unitPtr->action_para;
				targetXLoc = unitPtr->action_x_loc;
				targetYLoc = unitPtr->action_y_loc;
			}
			else
			{
				targetType = unitPtr->action_mode2;
				targetRecno = unitPtr->action_para2;
				targetXLoc = unitPtr->action_x_loc2;
				targetYLoc = unitPtr->action_y_loc2;
			}

			switch(targetType)
			{
				case ACTION_ATTACK_UNIT:
				case ACTION_DEFEND_TOWN_ATTACK_TARGET:
				case ACTION_MONSTER_DEFEND_ATTACK_TARGET:
						if(is_deleted(targetRecno))
							continue;

						targetUnit = operator[](targetRecno);
						if(targetUnit->nation_recno==oldNationRecno)
							unitPtr->stop2(KEEP_DEFENSE_MODE);
						break;

				case ACTION_ATTACK_FIRM:
						if(firm_array.is_deleted(targetRecno))
							continue;

						targetFirm = firm_array[targetRecno];
						if(targetFirm->nation_recno==oldNationRecno)
							unitPtr->stop2(KEEP_DEFENSE_MODE);
						break;

				case ACTION_ATTACK_TOWN:
						if(town_array.is_deleted(targetRecno))
							continue;

						targetTown = town_array[targetRecno];
						if(targetTown->nation_recno==oldNationRecno)
							unitPtr->stop2(KEEP_DEFENSE_MODE);
						break;

				case ACTION_ATTACK_WALL:
						targetLoc = world.get_loc(targetXLoc, targetYLoc);
						if(targetLoc->wall_nation_recno()==oldNationRecno)
							unitPtr->stop2(KEEP_DEFENSE_MODE);
						break;
			}
		}
	}
}
//----------- End of function UnitArray::stop_all_war -----------//


//--------- Begin of function UnitArray::stop_war_between ---------//
// stop the war between the two nations specified
//
// <short>	nationReno1	-	the nation_renco of one of the nation
// <short>	nationReno2	-	the nation_renco of the other nation
//
void UnitArray::stop_war_between(short nationRecno1, short nationRecno2)
{
	Unit	*unitPtr, *targetUnit;
	Firm	*targetFirm;
	Town	*targetTown;
	Location	*targetLoc;
	char	targetType;
	short targetRecno;
	int	targetXLoc, targetYLoc;

	for(int i=size(); i>0; --i)
	{
		if(is_deleted(i))
			continue;

		unitPtr = operator[](i);

		if(!unitPtr->is_visible())
			continue;

		if(unitPtr->nation_recno!=nationRecno1 && unitPtr->nation_recno!=nationRecno2)
			continue;

		if(unitPtr->is_action_attack())
		{
			//---- stop all attacking unit with target's nation_recno = oldNationRecno ----//
			if(unitPtr->action_mode2==ACTION_AUTO_DEFENSE_ATTACK_TARGET)
			{
				targetType = unitPtr->action_mode;
				targetRecno = unitPtr->action_para;
				targetXLoc = unitPtr->action_x_loc;
				targetYLoc = unitPtr->action_y_loc;
			}
			else
			{
				targetType = unitPtr->action_mode2;
				targetRecno = unitPtr->action_para2;
				targetXLoc = unitPtr->action_x_loc2;
				targetYLoc = unitPtr->action_y_loc2;
			}

			switch(targetType)
			{
				case ACTION_ATTACK_UNIT:
				case ACTION_DEFEND_TOWN_ATTACK_TARGET:
				case ACTION_MONSTER_DEFEND_ATTACK_TARGET:
						if(is_deleted(targetRecno))
							continue;

						targetUnit = operator[](targetRecno);
						if(targetUnit->nation_recno==nationRecno1 || targetUnit->nation_recno==nationRecno2)
							unitPtr->stop2(KEEP_DEFENSE_MODE);
						break;

				case ACTION_ATTACK_FIRM:
						if(firm_array.is_deleted(targetRecno))
							continue;

						targetFirm = firm_array[targetRecno];
						if(targetFirm->nation_recno==nationRecno1 || targetFirm->nation_recno==nationRecno2)
							unitPtr->stop2(KEEP_DEFENSE_MODE);
						break;

				case ACTION_ATTACK_TOWN:
						if(town_array.is_deleted(targetRecno))
							continue;

						targetTown = town_array[targetRecno];
						if(targetTown->nation_recno==nationRecno1 || targetTown->nation_recno==nationRecno2)
							unitPtr->stop2(KEEP_DEFENSE_MODE);
						break;

				case ACTION_ATTACK_WALL:
						targetLoc = world.get_loc(targetXLoc, targetYLoc);
						if(targetLoc->wall_nation_recno()==nationRecno1 || targetLoc->wall_nation_recno()==nationRecno2)
							unitPtr->stop2(KEEP_DEFENSE_MODE);
						break;
			}
		}
	}
}
//----------- End of function UnitArray::stop_war_between -----------//


//--------- Begin of function UnitArray::stop_attack_unit ---------//
// stop all units that is attacking the specified unit
//
// <short>	unitRecno	-	unit_recno of the specified unit
//
void UnitArray::stop_attack_unit(short unitRecno)
{
	if(nation_hand_over_flag)
		return;

	Unit	*unitPtr;

	for(int i=size(); i>0; --i)
	{
		if(is_deleted(i))
			continue;

		unitPtr = operator[](i);

		if(!unitPtr->is_visible())
			continue;

		if((unitPtr->action_para==unitRecno && unitPtr->action_mode==ACTION_ATTACK_UNIT) ||
			(unitPtr->action_para2==unitRecno && unitPtr->action_mode2==ACTION_ATTACK_UNIT))
			unitPtr->stop2(KEEP_DEFENSE_MODE);
	}
}
//----------- End of function UnitArray::stop_attack_unit -----------//


//--------- Begin of function UnitArray::stop_attack_firm ---------//
// stop all units that is attacking the specified firm
//
// <short> firmRecno	-	the firm_recno of the firm
//
void UnitArray::stop_attack_firm(short firmRecno)
{
	if(nation_hand_over_flag)
		return;

	Unit	*unitPtr;

	for(int i=size(); i>0; --i)
	{
		if(is_deleted(i))
			continue;

		unitPtr = operator[](i);

		if(!unitPtr->is_visible())
			continue;

		if((unitPtr->action_para==firmRecno && unitPtr->action_mode==ACTION_ATTACK_FIRM) ||
			(unitPtr->action_para2==firmRecno && unitPtr->action_mode2==ACTION_ATTACK_FIRM))
			unitPtr->stop2(KEEP_DEFENSE_MODE);
	}
}
//----------- End of function UnitArray::stop_attack_firm -----------//


//--------- Begin of function UnitArray::stop_attack_town ---------//
// stop all units that is attacking the specified town
//
// <short> townRecno	-	the town_recno of the specified town
//
void UnitArray::stop_attack_town(short townRecno)
{
	if(nation_hand_over_flag)
		return;

	Unit	*unitPtr;

	for(int i=size(); i>0; --i)
	{
		if(is_deleted(i))
			continue;

		unitPtr = operator[](i);

		if(!unitPtr->is_visible())
			continue;

		if((unitPtr->action_para==townRecno && unitPtr->action_mode==ACTION_ATTACK_TOWN) ||
			(unitPtr->action_para2==townRecno && unitPtr->action_mode2==ACTION_ATTACK_TOWN))
			unitPtr->stop2(KEEP_DEFENSE_MODE);
	}
}
//----------- End of function UnitArray::stop_attack_town -----------//


//--------- Begin of function UnitArray::assign ---------//
// group assign to firm/town/unit
//
// <int>		destX				-	x location assigned to
// <int>		destY				-	y location assigned to
//	<int>		divided			-	whether the array is divied by divide_array(...) (1 - divided, 0 not)
//	<char>	remoteAction	-
//	[short*]	selectedArray	-	unit_recno of the selected units	(default: NULL)
// [int]		selectedCount	-	number of selected units	(default: 0)
//
void UnitArray::assign(int destX, int destY, int divided, char remoteAction, short* selectedArray, int selectedCount)
{
	err_when(destX<0 || destY<0 || destX>MAX_WORLD_X_LOC-1 || destY>MAX_WORLD_Y_LOC-1);

	//###### begin trevor 25/8 #######//

	//--- set the destination to the top left position of the town/firm ---//

	Location* locPtr = world.get_loc(destX, destY);

	//---- if there is a firm on this location -----//

	if( locPtr->is_firm() )
	{
		Firm* firmPtr = firm_array[locPtr->firm_recno()];

		destX = firmPtr->loc_x1;
		destY = firmPtr->loc_y1;
	}

	//---- if there is a town on this location -----//

	else if( locPtr->is_town() )
	{
		Town* townPtr = town_array[locPtr->town_recno()];

		destX = townPtr->loc_x1;
		destY = townPtr->loc_y1;
	}

	//###### end trevor 25/8 #######//

	//-------------------------------------------//

	Unit*		unitPtr;
	int		freeMemory = 0;

	if( selectedArray == NULL)
	{
		selectedArray = (short*) mem_add(sizeof(short)*size());
		selectedCount = 0;
		freeMemory = 1;

		// find myself
		for(int i=size(); i>0; i--)
		{
			if(is_deleted(i))
				continue;

			unitPtr = operator[](i);
			//err_when(!unitPtr->is_visible() || unitPtr->hit_points<=0);
			if(!unitPtr->is_visible())
				continue;
			
			if( unitPtr->selected_flag && unitPtr->is_own() )
			{
				err_when(unitPtr->hit_points<=0 || unitPtr->cur_action==SPRITE_DIE || unitPtr->action_mode==ACTION_DIE);
				selectedArray[selectedCount++] = i;
			}
		}
	}

	if(!remoteAction && remote.is_enable())
	{
		// packet structure : <xLoc> <yLoc> <no. of units> <unit recno ...>
		short *shortPtr = (short *)remote.new_send_queue_msg(MSG_UNIT_ASSIGN,
			sizeof(short) * (4+selectedCount));
		shortPtr[0] = destX;
		shortPtr[1] = destY;
		shortPtr[2] = selectedCount;
		shortPtr[3] = divided;
		memcpy(shortPtr+4, selectedArray, sizeof(short)*selectedCount);
	}
	else
	{
		if(!divided)
		{
			for(int j=0; j< selectedCount; ++j)
			{
				int i = selectedArray[j];

				if(is_deleted(i))
					continue;

				unitPtr = operator[](i);	//unit_array[i];

				//### begin trevor 7/7#######//
				err_when(!unitPtr->is_visible() );
				err_when( unitPtr->hit_points<=0 );
				//### end trevor 7/7#######//

				unitPtr->stop2();
			}

			//--------- divide the unit by their mobile_type ------------//
			divide_array(destX, destY, selectedArray, selectedCount);
			
			if(selected_land_unit_count)
				assign(destX, destY, 1, remoteAction, selected_land_unit_array, selected_land_unit_count);

			if(selected_sea_unit_count)
			{
				Location *locPtr = world.get_loc(destX, destY);
				if(locPtr->is_firm())
				{
					err_when(firm_array.is_deleted(locPtr->firm_recno()));
					Firm *firmPtr = firm_array[locPtr->firm_recno()];
					if(firmPtr->firm_id==FIRM_HARBOR) // recursive call
						assign(destX, destY, 1, remoteAction, selected_sea_unit_array, selected_sea_unit_count);
					else
						ship_to_beach(destX, destY, 1, selected_sea_unit_array, selected_sea_unit_count, remoteAction);
				}
				//else if(locPtr->is_town())
				else
				{
					err_when(town_array.is_deleted(locPtr->town_recno()));
					ship_to_beach(destX, destY, 1, selected_sea_unit_array, selected_sea_unit_count, remoteAction);
				}
			}

			if(selected_air_unit_count) // no assign for air units
				move_to(destX, destY, 1, selected_air_unit_array, selected_air_unit_count, remoteAction);

			//------------ deinit static variables ------------//
			selected_land_unit_count = selected_sea_unit_count = selected_air_unit_count = 0;
			mem_del(selected_land_unit_array);
			mem_del(selected_sea_unit_array);
			mem_del(selected_air_unit_array);
		}
		else
		{
			//---------- set unit to assign -----------//
			if(selectedCount<2)
			{
				unitPtr = (Unit*) get_ptr(selectedArray[0]);
				err_when(!unitPtr->is_visible() || unitPtr->hit_points<=0);

				if(unitPtr->sprite_info->loc_width<=1)
				{
					unitPtr->unit_group_id = unit_array.cur_group_id++;
					unitPtr->assign(destX, destY);
				}
				else // move to object surrounding
				{
					Location *locPtr = world.get_loc(destX, destY);
					if(locPtr->is_firm())
						unitPtr->move_to_firm_surround(destX, destY, unitPtr->sprite_info->loc_width, unitPtr->sprite_info->loc_height, locPtr->firm_recno());
					else if(locPtr->is_town())
						unitPtr->move_to_town_surround(destX, destY, unitPtr->sprite_info->loc_width, unitPtr->sprite_info->loc_height);
					else if(locPtr->has_unit(UNIT_LAND))
						unitPtr->move_to_unit_surround(destX, destY, unitPtr->sprite_info->loc_width, unitPtr->sprite_info->loc_height, locPtr->unit_recno(UNIT_LAND));
				}
			}
			else // for more than one unit selecting, call group_assign() to take care of it
			{
				set_group_id(selectedArray, selectedCount);
				group_assign(destX, destY, selectedArray, selectedCount);
			}
		}
	}

	if( freeMemory )
		mem_del(selectedArray);
}
//----------- End of function UnitArray::assign -----------//


//--------- Begin of function UnitArray::group_assign ---------//
// <int>		destX				-	x location assigned to
// <int>		destY				-	y location assigned to
// <short*>	selectedArray	-	unit recno of the selected units
// <int>		selectedCount	-	num of selected units
//
void UnitArray::group_assign(int destX, int destY, short* selectedArray, int selectedCount)
{
	enum	{	ASSIGN_TYPE_UNIT = 1,
				ASSIGN_TYPE_FIRM,
				ASSIGN_TYPE_TOWN,
			};

	Location *locPtr = world.get_loc(destX, destY);

	int assignType = 0;	// 1 for unit, 2 for firm, 3 for town
	int miscNo = 0;		// usedd to store the recno of the object
	if(locPtr->has_unit(UNIT_LAND))
	{
		assignType = ASSIGN_TYPE_UNIT;
		miscNo = locPtr->unit_recno(UNIT_LAND);
	}
	else if(locPtr->is_firm())
	{
		assignType = ASSIGN_TYPE_FIRM;
		miscNo = firm_array[locPtr->firm_recno()]->firm_id;
	}
	else if(locPtr->is_town())
	{
		assignType = ASSIGN_TYPE_TOWN;
		miscNo = 0;
	}
	else
		err_here();

	Unit* unitPtr;
	for(int i=0; i<selectedCount; i++)
	{
		unitPtr = operator[](selectedArray[i]);
		err_when(!unitPtr->is_visible() || unitPtr->hit_points<=0);
		if(unitPtr->sprite_info->loc_width<=1)
			unitPtr->assign(destX, destY, i+1); // the thrid parameter is used to generate different result for the searching
		else
		{
			//-----------------------------------------------------------------//
			// for 2x2 unit, unable to assign, so move to the surrounding
			//-----------------------------------------------------------------//
			switch(assignType)
			{
				case ASSIGN_TYPE_UNIT:
							unitPtr->move_to_unit_surround(destX, destY, unitPtr->sprite_info->loc_width,
																	 unitPtr->sprite_info->loc_height, miscNo);
							break;	// is a unit

				case ASSIGN_TYPE_FIRM:
							unitPtr->move_to_firm_surround(destX, destY, unitPtr->sprite_info->loc_width,
																	 unitPtr->sprite_info->loc_height, miscNo);
							break;	// is a firm

				case ASSIGN_TYPE_TOWN:
							unitPtr->move_to_town_surround(destX, destY, unitPtr->sprite_info->loc_width,
																	 unitPtr->sprite_info->loc_height);
							break;	// is a town

				default:	err_here();
							break;
			}
		}
	}
}
//----------- End of function UnitArray::group_assign -----------//


//--------- Begin of function UnitArray::assign_to_camp ---------//
// <int>		destX					-	x location where the camp is
// <int>		destY					-	y location where the camp is
// <char>	remoteAction		-
// [short*]	selectedUnitArray	-	recno of the selected units (default: NULL)
// [int]		selectedCount		-	number of selected units  (default: 0)
//
void UnitArray::assign_to_camp(int destX, int destY, char remoteAction, short* selectedUnitArray, int selectedCount)
{
	divide_array(destX, destY, selectedUnitArray, selectedCount);
	
	if(selected_land_unit_count)
	{
		int assignCount=0, moveCount=0;
		short *assignArray = (short*) mem_add(sizeof(short)*selected_land_unit_count);
		short *moveArray = (short*) mem_add(sizeof(short)*selected_land_unit_count);
		char unitClass;
		Unit *unitPtr;
		memset(assignArray, 0, sizeof(short)*selected_land_unit_count);
		memset(moveArray, 0, sizeof(short)*selected_land_unit_count);

		//----------------------------------------------------------------//
		// Only human and weapon can be assigned to the camp. Others are
		// ordered to move to camp as close as possible.
		//----------------------------------------------------------------//
		for(int i=0; i<selectedCount; i++)
		{
			err_when(unit_array.is_deleted(selectedUnitArray[i]));
			unitPtr = operator[](selectedUnitArray[i]);
			unitClass = unit_res[unitPtr->unit_id]->unit_class;
			if(unitClass==UNIT_CLASS_HUMAN || unitClass==UNIT_CLASS_WEAPON)
				assignArray[assignCount++] = selectedUnitArray[i];
			else
				moveArray[moveCount++] = selectedUnitArray[i];
		}

		if(assignCount)
			assign(destX, destY, 1, remoteAction, assignArray, assignCount);
		if(moveCount)
			move_to(destX, destY, 1, moveArray, moveCount, remoteAction);

		mem_del(assignArray);
		mem_del(moveArray);
	}

	if(selected_sea_unit_count)
		ship_to_beach(destX, destY, 1, selected_sea_unit_array, selected_sea_unit_count, remoteAction);

	if(selected_air_unit_count)
		move_to(destX, destY, 1, selected_air_unit_array, selected_air_unit_count, remoteAction);

	//---------------- deinit static parameters ---------------//
	selected_land_unit_count = selected_sea_unit_count = selected_air_unit_count = 0;
	mem_del(selected_land_unit_array);
	mem_del(selected_sea_unit_array);
	mem_del(selected_air_unit_array);
}
//----------- End of function UnitArray::assign_to_camp -----------//


//--------- Begin of function UnitArray::settle ---------//
//	<int>		destX				-	x location settle to
//	<int>		destY				-	y location settle to
//	<int>		divided			-	whether the selected is divided into subgroup, (1 = divided, 0 = not)
//	<char>	remoteAction	-
//	[short*]	selectedArray	-	the recno of the selected units (default: NULL)
//	[int]		selectedCount	-	num of selected units (default: 0)
//
void UnitArray::settle(int destX, int destY, int divided, char remoteAction, short* selectedArray, int selectedCount)
{
	Unit*		unitPtr;
	int		freeMemory = 0;

	if(selectedArray == NULL)
	{
		selectedArray = (short*) mem_add(sizeof(short)*size());
		selectedCount = 0;
		freeMemory = 1;

		// find myself
		for(int i=size(); i>0; i--)
		{
			if(is_deleted(i))
				continue;

			unitPtr = operator[](i);
			//err_when(!unitPtr->is_visible() || unitPtr->hit_points<=0);
			if(!unitPtr->is_visible())
				continue;
			
			if( unitPtr->selected_flag && unitPtr->is_own() )
			{
				err_when(unitPtr->hit_points<=0 || unitPtr->cur_action==SPRITE_DIE || unitPtr->action_mode==ACTION_DIE);
				selectedArray[selectedCount++] = i;
			}
		}
	}

	if(!remoteAction && remote.is_enable())
	{
		// packet structure : <xLoc> <yLoc> <no. of units> <divided> <unit recno ...>
		short *shortPtr = (short *)remote.new_send_queue_msg(MSG_UNITS_SETTLE, sizeof(short)*(4+selectedCount));
		shortPtr[0] = destX;
		shortPtr[1] = destY;
		shortPtr[2] = selectedCount;
		shortPtr[3] = divided;
		memcpy(shortPtr+4, selectedArray, sizeof(short)*selectedCount);
	}
	else
	{
		if(!divided)
		{
			for(int j=0; j< selectedCount; ++j)
			{
				int i = selectedArray[j];

				if(is_deleted(i))
					continue;

				unitPtr = operator[](i);	//unit_array[i];
				err_when(!unitPtr->is_visible() || unitPtr->hit_points<=0);
				unitPtr->stop2();
			}

			divide_array(destX, destY, selectedArray, selectedCount);

			if(selected_land_unit_count)
				settle(destX, destY, 1, remoteAction, selected_land_unit_array, selected_land_unit_count);
			
			if(selected_sea_unit_count)
				ship_to_beach(destX, destY, 1, selected_sea_unit_array, selected_sea_unit_count, remoteAction);

			if(selected_air_unit_count)
				move_to(destX, destY, 1, selected_air_unit_array, selected_air_unit_count, remoteAction);

			//-------------- deinit static parameters --------------//
			selected_land_unit_count = selected_sea_unit_count = selected_air_unit_count = 0;
			mem_del(selected_land_unit_array);
			mem_del(selected_sea_unit_array);
			mem_del(selected_air_unit_array);
		}
		else
		{
			//---------- set unit to settle -----------//
			if(selectedCount<2)
			{
				unitPtr = (Unit*) get_ptr(selectedArray[0]);
				err_when(!unitPtr->is_visible() || unitPtr->hit_points<=0);
				unitPtr->unit_group_id = unit_array.cur_group_id++;
				unitPtr->settle(destX, destY);
			}
			else
			{
				set_group_id(selectedArray, selectedCount);
				group_settle(destX, destY, selectedArray, selectedCount);
			}
		}
	}

	if(freeMemory)
		mem_del(selectedArray);
}
//----------- End of function UnitArray::settle -----------//


//--------- Begin of function UnitArray::group_settle ---------//
//	<int>		destX				-	x location to settle to
// <int>		destY				-	y location to settle to
// <short*>	selectedArray	-	recno. of selected units
//	<int>		selectedCount	-	num. of selected units
//
void UnitArray::group_settle(int destX, int destY, short* selectedArray, int selectedCount)
{
	#ifdef DEBUG
		uint32_t debugGroupId = (operator[](selectedArray[0]))->unit_group_id;
	#endif

	Unit* unitPtr;

	for(int i=0; i<selectedCount; i++)
	{
		unitPtr = operator[](selectedArray[i]);
		err_when(!unitPtr->is_visible() || unitPtr->hit_points<=0);
		
		#ifdef DEBUG
			err_when(unitPtr->unit_group_id!=debugGroupId);
		#endif

		unitPtr->settle(destX, destY, i+1);
	}
}
//----------- End of function UnitArray::group_settle -----------//


//--------- Begin of function UnitArray::assign_to_ship ---------//
// <int>		shipXLoc			-	x location of the ship
// <int>		shipYLoc			-	y location of the ship
// <int>		divided			-	whether the selected groups is divided (1 - divided, 0 - not)
// <short*>	selectedArray	-	recno of selected units
// <int>		selectedCount	-	num. of selected units
// <char>	remoteAction	-
//
void UnitArray::assign_to_ship(int shipXLoc, int shipYLoc, int divided, short* selectedArray, int selectedCount, char remoteAction, int shipRecno)
{
	err_when(terrain_res[world.get_loc(shipXLoc, shipYLoc)->terrain_id]->average_type!=TERRAIN_OCEAN);
	err_when(!selectedCount || selectedCount>1000);

	if( !remoteAction && remote.is_enable() )
	{
		// packet structure : <xLoc> <yLoc> <ship recno> <no. of units> <divided> <unit recno ...>
		short *shortPtr = (short *)remote.new_send_queue_msg(MSG_UNITS_ASSIGN_TO_SHIP,
			sizeof(short) * (5+selectedCount));
		shortPtr[0] = shipXLoc;
		shortPtr[1] = shipYLoc;
		shortPtr[2] = shipRecno;
		shortPtr[3] = selectedCount;
		shortPtr[4] = divided;
		memcpy(shortPtr+5, selectedArray, sizeof(short)*selectedCount);

		return;
	}

	if(!divided)
	{
		divide_array(shipXLoc, shipYLoc, selectedArray, selectedCount);

		if(selected_sea_unit_count) // Note: the order to call ship unit first
			move_to(shipXLoc, shipYLoc, 1, selected_sea_unit_array, selected_sea_unit_count, remoteAction);

		if(selected_land_unit_count)
			assign_to_ship(shipXLoc, shipYLoc, 1, selected_land_unit_array, selected_land_unit_count, remoteAction, shipRecno);

		if(selected_air_unit_count)
			move_to(shipXLoc, shipYLoc, 1, selected_air_unit_array, selected_air_unit_count, remoteAction);

		//---------------- deinit static parameters -----------------//
		selected_land_unit_count = selected_sea_unit_count = selected_air_unit_count = 0;
		mem_del(selected_land_unit_array);
		mem_del(selected_sea_unit_array);
		mem_del(selected_air_unit_array);
		return;
	}
	else
	{
		// ###### begin Gilbert 5/8 ##########//
		// ----- dont not use shipXLoc, shipYLoc passed, use shipRecno --------//

		UnitMarine *shipPtr = (UnitMarine*) get_ptr(shipRecno);

		if( shipPtr && shipPtr->is_visible() )
		{
			shipXLoc = shipPtr->next_x_loc();
			shipYLoc = shipPtr->next_y_loc();
		}
		else
			return;
		// ##### patch end Gilbert 5/8 #######//

		//------------------------------------------------------------------------------------//
		// find the closest unit to the ship
		//------------------------------------------------------------------------------------//
		int	minDist=0x7FFFFF, dist, distX, distY;
		int	closestUnitRecno = -1;
		Unit	*unitPtr;
		int i;
		for(i=0; i<selectedCount; i++)
		{
			unitPtr = (Unit*) get_ptr(selectedArray[i]);
			distX = abs(shipXLoc - unitPtr->next_x_loc());
			distY = abs(shipYLoc - unitPtr->next_y_loc());
			dist = (distX>distY) ? distX : distY;
			if(dist < minDist)
			{
				minDist = dist;
				closestUnitRecno = i;
			}
		}
		err_when(closestUnitRecno==-1);

		//------------------------------------------------------------------------------------//
		// If the seleceted units are distributed on different territories, select those unit
		// on the same territory as the closet unit for processing, there will be no action for
		// the rest units.
		//------------------------------------------------------------------------------------//
		uint32_t				curGroupId = unit_array.cur_group_id++;
		Unit				*closestUnit = (Unit*) get_ptr(selectedArray[closestUnitRecno]);
		int				closestUnitXLoc = closestUnit->next_x_loc();
		int				closestUnitYLoc = closestUnit->next_y_loc();
		unsigned char	defaultRegionId = world.get_loc(closestUnitXLoc, closestUnitYLoc)->region_id;
		short				*newSelectedArray;
		int				newSelectedCount = 0;

		if(selectedCount>1)
		{
			newSelectedArray = (short*) mem_add(sizeof(short)*selectedCount);
			memset(newSelectedArray, 0, sizeof(short)*selectedCount);

			for(i=0; i<selectedCount; i++)
			{
				unitPtr = (Unit*) get_ptr(selectedArray[i]);
				if(world.get_loc(unitPtr->next_x_loc(), unitPtr->next_y_loc())->region_id == defaultRegionId)
				{
					newSelectedArray[newSelectedCount++] = selectedArray[i]; // on the same territory
					unitPtr->unit_group_id = curGroupId;
				}

				if(unitPtr->cur_action==SPRITE_IDLE)
					unitPtr->set_ready();
			}
			err_when(newSelectedCount > selectedCount);
		}
		else
		{
			err_when(closestUnit->sprite_recno!=selectedArray[0]);
			newSelectedArray = (short*) mem_add(sizeof(short)); // only one unit
			newSelectedArray[0] = selectedArray[0];
			newSelectedCount = 1;
			closestUnit->unit_group_id = curGroupId;
		}
              
		//-------------- ordering the ship move near the coast ----------------//
		int curXLoc = closestUnit->next_x_loc();
		int curYLoc = closestUnit->next_y_loc();
		
		// ##### patch begin Gilbert 5/8 #######//
		// UnitMarine *shipPtr = (UnitMarine*) get_ptr(world.get_unit_recno(shipXLoc, shipYLoc, UNIT_SEA));
		// ##### patch end Gilbert 5/8 #######//

		int landX, landY;
		shipPtr->unit_group_id = curGroupId;
		shipPtr->ship_to_beach(curXLoc, curYLoc, landX, landY);

		if(landX!=-1 && landY!=-1)
		{
			//-------------- ordering the units ---------------//
			#define TRY_SIZE 5
			int countLimit = TRY_SIZE*TRY_SIZE;
			//### begin alex 30/10 ###//
			int j, k, xShift, yShift, checkXLoc, checkYLoc;
			uint8_t regionId = world.get_loc(landX, landY)->region_id;
			Location *locPtr;
			for(i=0, k=0; i<newSelectedCount; i++)
			{
				for(j=1; j<countLimit; j++)
				{
					if(++k>countLimit)
						k = 1;
					misc.cal_move_around_a_point(k, TRY_SIZE, TRY_SIZE, xShift, yShift);
					checkXLoc = landX+xShift;
					checkYLoc = landY+yShift;
					if(checkXLoc<0 || checkXLoc>=MAX_WORLD_X_LOC || checkYLoc<0 || checkYLoc>=MAX_WORLD_Y_LOC)
						continue;

					locPtr = world.get_loc(checkXLoc, checkYLoc);
					if(locPtr->region_id!=regionId || !locPtr->walkable())
						continue;

					unitPtr = (Unit*) get_ptr(newSelectedArray[i]);
					unitPtr->assign_to_ship(landX, landY, shipPtr->sprite_recno, k);
					break;
				}
			}
			//#### end alex 30/10 ####//
		}

		mem_del(newSelectedArray);
	}
}
//----------- End of function UnitArray::assign_to_ship -----------//


//--------- Begin of function UnitArray::ship_to_beach ---------//
// <int>		destX				-	x location the ship move to
//	<int>		destY				-	y location the ship move to
//	<int>		divided			-	whether the units are divided (1 - divided, 0 - not)
//	<short*>	selectedArray	-	the recno. of the selected units
//	<int>		selectedCount	-	num. of selected units
//	<char>	remoteAction	-
//
void UnitArray::ship_to_beach(int destX, int destY, int divided, short* selectedArray, int selectedCount, char remoteAction)
{
	Unit *unitPtr;

	if( !remoteAction && remote.is_enable() )
	{
		// packet structure : <xLoc> <yLoc> <no. of units> <divided> <unit recno ...>
		short *shortPtr = (short *)remote.new_send_queue_msg(MSG_UNITS_SHIP_TO_BEACH,
			sizeof(short) * (4+selectedCount));
		shortPtr[0] = destX;
		shortPtr[1] = destY;
		shortPtr[2] = selectedCount;
		shortPtr[3] = divided;
		memcpy(shortPtr+4, selectedArray, sizeof(short)*selectedCount);

		return;
	}

	set_group_id(selectedArray, selectedCount);

	//--------------------------------------------------------------------//
	//--------------------------------------------------------------------//
	#define CHECK_SEA_DIMENSION	50
	#define CHECK_SEA_SIZE			CHECK_SEA_DIMENSION*CHECK_SEA_DIMENSION
	Location *locPtr = world.get_loc(destX, destY);
	uint8_t regionId = locPtr->region_id;
	int xShift, yShift, checkXLoc, checkYLoc;
	int landX, landY, seaX, seaY, tempX, tempY;

	int j=1, k, found;
	
	//--------------------------------------------------------------------//
	// find a unit that can carrying units.  Let it to do the first searching.
	// Use the returned reference parameters (landX, landY) for the other
	// ships to calculate their final location to move to
	//--------------------------------------------------------------------//
	int i;
	for(i=0; i<selectedCount; i++) // for first unit
	{
		unitPtr = (Unit*) get_ptr(selectedArray[i]);
		if(unit_res[unitPtr->unit_id]->carry_unit_capacity>0)
		{
			unitPtr->ship_to_beach(destX, destY, landX, landY); // landX=landY=-1 if calling move_to() instead
			i++;
			break;
		}
		else
			unitPtr->move_to(destX, destY, 1);
	}

	int totalCheck = 0;
	for(; i<selectedCount; i++) // for the rest units
	{
		unitPtr = (Unit*) get_ptr(selectedArray[i]);
		if(unit_res[unitPtr->unit_id]->carry_unit_capacity>0 && landX!=-1 && landY!=-1)
		{
			for(found=0; j<=CHECK_SEA_SIZE; j++, totalCheck++)
			{
				misc.cal_move_around_a_point(j, CHECK_SEA_DIMENSION, CHECK_SEA_DIMENSION, xShift, yShift);

				if(j>=CHECK_SEA_SIZE)
					j = 1;

				if(totalCheck == CHECK_SEA_SIZE)
				{
					//--------------------------------------------------------------------//
					// can't handle this case
					//--------------------------------------------------------------------//
					unitPtr->ship_to_beach(landX, landY, tempX, tempY);
					totalCheck = 0;
					break;
				}

				seaX = landX+xShift;
				seaY = landY+yShift;
				if(seaX<0 || seaX>=MAX_WORLD_X_LOC || seaY<0 || seaY>=MAX_WORLD_Y_LOC)
					continue;

				locPtr = world.get_loc(seaX, seaY);
				if(terrain_res[locPtr->terrain_id]->average_type!=TERRAIN_OCEAN)
					continue;

				//--------------------------------------------------------------------//
				// if it is able to find a location around the surrounding location with
				// same region id we prefer, order the unit to move there.
				//--------------------------------------------------------------------//
				for(k=2; k<=9; k++)
				{
					misc.cal_move_around_a_point(k, 3, 3, xShift, yShift);
					checkXLoc = seaX+xShift;
					checkYLoc = seaY+yShift;
					if(checkXLoc<0 || checkXLoc>=MAX_WORLD_X_LOC || checkYLoc<0 || checkYLoc>=MAX_WORLD_Y_LOC)
						continue;
					
					locPtr = world.get_loc(checkXLoc, checkYLoc);
					if(locPtr->region_id!=regionId)
						continue;

					unitPtr->ship_to_beach(checkXLoc, checkYLoc, tempX, tempY);
					found++;
					err_when(unitPtr->result_node_array && unitPtr->result_node_count==unitPtr->result_node_recno &&
								(unitPtr->result_node_array[unitPtr->result_node_count-1].node_x!=unitPtr->go_x>>ZOOM_X_SHIFT_COUNT ||
								unitPtr->result_node_array[unitPtr->result_node_count-1].node_y!=unitPtr->go_y>>ZOOM_Y_SHIFT_COUNT));
					break;
				}

				if(found)
				{
					totalCheck = 0;
					break;
				}
			}
		}
		else // cannot carry units
			unitPtr->move_to(destX, destY, 1);
	}
}
//----------- End of function UnitArray::ship_to_beach -----------//


//----------- Begin of function UnitArray::divide_attack_by_nation -----------//
//
// units which can be controlled by nationRecno to attack are moved
// to the front of the selectedArray, return is number of units passed
//
int UnitArray::divide_attack_by_nation(short nationRecno, short *selectedArray, int selectedCount)
{
	// elements before i are pass, elements on or after passCount are not pass
	int loopCount = selectedCount+2;
	int passCount = selectedCount;
	for( int i = 0; i < passCount;  )
	{
		err_when(--loopCount == 0);
		short unitRecno = selectedArray[i];

		// a clocked spy cannot be commanded by original nation to attack
		if( !is_deleted(unitRecno) && operator[](unitRecno)->nation_recno == nationRecno )
		{
			// pass
			++i;
		}
		else
		{
			// fail, swap [i] with [passCount-1]
			--passCount;

			selectedArray[i] = selectedArray[passCount];
			selectedArray[passCount] = unitRecno;
		}
	}
	return passCount;
}
//----------- End of function UnitArray::divide_attack_by_nation -----------//


//----------- Begin of function UnitArray::add_way_point -----------//
// <int>		pointX			-	x location of the point to be added
//	<int>		pointY			-	y location of the point to be added
// <short*>	selectedArray	-	recno. of selected units
//	<int>		selectedCount	-	num. of selected units
//	<char>	remoteAction	-
//
void UnitArray::add_way_point(int pointX, int pointY, short* selectedArray, int selectedCount, char remoteAction)
{
	if(!remoteAction && remote.is_enable())
	{
		// packet structure : <xLoc> <yLoc> <no. of units> <unit recno ...>
		short *shortPtr = (short *)remote.new_send_queue_msg(MSG_UNIT_ADD_WAY_POINT,
			sizeof(short) * (3+selectedCount));
		shortPtr[0] = pointX;
		shortPtr[1] = pointY;
		shortPtr[2] = selectedCount;
		memcpy(shortPtr+3, selectedArray, sizeof(short)*selectedCount);
	}
	else
	{
		uint32_t groupId = unit_array.cur_group_id;
		Unit *unitPtr;

		int i;
		for(i=0; i<selectedCount; ++i)
		{
			unitPtr = unit_array[selectedArray[i]];
			unitPtr->unit_group_id = groupId;
		}

		unit_array.cur_group_id++;
	
		for(i=0; i<selectedCount; ++i)
		{
			unitPtr = unit_array[selectedArray[i]];
			unitPtr->add_way_point(pointX, pointY);
		}
	}
}
//----------- End of function UnitArray::add_way_point -----------//
