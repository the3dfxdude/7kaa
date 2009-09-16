#include <memory.h>
#include <OGRPSEL.h>
#include <OPOWER.h>
#include <OUNIT.h>
#include <ONATION.h>

//--------------- begin of function GroupSelect::GroupSelect ----------------//
GroupSelect::GroupSelect()
{
	//memset(this, 0, sizeof(GroupSelect));
}
//--------------- end of function GroupSelect::GroupSelect ----------------//


//--------------- begin of function GroupSelect::~GroupSelect ----------------//
GroupSelect::~GroupSelect()
{
	deinit();
}
//--------------- end of function GroupSelect::~GroupSelect ----------------//


//--------------- begin of function GroupSelect::init ----------------//
void GroupSelect::init()
{
}
//--------------- end of function GroupSelect::init ----------------//


//--------------- begin of function GroupSelect::deinit ----------------//
void GroupSelect::deinit()
{
}
//--------------- end of function GroupSelect::deinit ----------------//


//--------------- begin of function GroupSelect::group_units ----------------//
// <int> groupNum = 1 - MAX_SELECT_GROUP_NUM
//
void GroupSelect::group_units(int groupNum)
{
	err_when(groupNum<1 || groupNum>MAX_SELECT_GROUP_NUM);

	Unit	*unitPtr;
	int playerNation;

	if(!unit_array.selected_recno)
		return; // invalid call
	else
	{
		playerNation = nation_array.player_recno;
		unitPtr = unit_array[unit_array.selected_recno];
		if(unitPtr->nation_recno!=playerNation)
			return;
	}

	//---------- group selected units ----------//
	int arraySize = unit_array.size();
	for(int i=arraySize; i>0; --i)
	{
		if(unit_array.is_deleted(i))
			continue;

		unitPtr = unit_array[i];
		if(unitPtr->nation_recno!=playerNation)
			continue;


		if(unitPtr->selected_flag)
				unitPtr->group_select_id = groupNum; // new selected unit
		else if(unitPtr->group_select_id==groupNum) // old member in this group
			unitPtr->group_select_id = 0;
	}
}
//--------------- end of function GroupSelect::group_units ----------------//


//--------------- begin of function GroupSelect::select_grouped_units ----------------//
// <int> groupNum = 1 - MAX_SELECT_GROUP_NUM
//
void GroupSelect::select_grouped_units(int groupNum)
{
	err_when(groupNum<1 || groupNum>MAX_SELECT_GROUP_NUM);

	int arraySize = unit_array.size();
	Unit	*unitPtr;
	int	playerNation = nation_array.player_recno;

	//---------- count selected units ----------//
	int groupUnitCount = 0;
	for(int i=arraySize; i>0; --i)
	{
		if(unit_array.is_deleted(i))
			continue;

		unitPtr = unit_array[i];

		if(unitPtr->nation_recno!=playerNation)
			continue;

		if(!unitPtr->is_visible())
			continue;

		if(unitPtr->group_select_id==groupNum)
		{
			groupUnitCount++;
			break;
		}
	}

	if(!groupUnitCount)
		return;
	
	power.reset_selection();

	//---------- select units in this group ----------//
	Unit	*showSelectedUnitPtr = NULL;
	unit_array.selected_count = 0;
	for(i=arraySize; i>0; --i)
	{
		if(unit_array.is_deleted(i))
			continue;

		unitPtr = unit_array[i];

		if(unitPtr->nation_recno!=playerNation)
			continue;

		if(!unitPtr->is_visible())
			continue;

		if(unitPtr->group_select_id==groupNum)
		{
			unitPtr->selected_flag = 1;
			unit_array.selected_count++;

			if(showSelectedUnitPtr==NULL || unitPtr->hit_points>showSelectedUnitPtr->hit_points)
				showSelectedUnitPtr = unitPtr;
		}
	}

	if(showSelectedUnitPtr)
	{
		unit_array.selected_recno = showSelectedUnitPtr->sprite_recno;
		info.disp();
	}
}
//--------------- end of function GroupSelect::select_grouped_units ----------------//

