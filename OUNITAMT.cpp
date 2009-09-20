//Filename    : OUNITAMT.CPP
//Description : Object UnitArray, trading code of caravans and ships for multiplayer game
//Owner		  : Alex

//------------------------------------------------------------------------------------------------------//
// Parameters used for updating cargo info. of trade units in muliplayer game:
//
// 1) caravan
// <short>	mp_selected_caravan_recno[MAX_NATION]
// <int>		mp_selected_caravan_count
//	<char>	mp_first_frame_to_select_caravan
//	<short>	mp_pre_selected_caravan_recno
//
// 2) marine that can trade
// <short>	mp_selected_ship_recno[MAX_NATION]
// <int>		mp_selected_ship_count
//	<char>	mp_first_frame_to_select_ship
//	<short>	mp_pre_selected_ship_recno
//
// Functions used in multiplayer game
//
// 1) common
// update_selected_trade_unit_info(...)
//
// 2) caravan
//	mp_mark_selected_caravan(...)
//	mp_get_selected_caravan_count(...)
//	mp_reset_selected_caravan_count(...)
//	mp_add_selected_caravan(...)
//	mp_is_selected_caravan(...)
//
// 3) marine that can trade
//	mp_mark_selected_ship(...)
//	mp_get_selected_ship_count(...)
//	mp_reset_selected_ship_count(...)
//	mp_add_selected_ship(...)
//	mp_is_selected_ship(...)
//
// In mulitplayer game, any order to the units from one player to others player must experience delay
// of one frame.  In order to update information of selected trade units in real time, the design used
// in this part of programming is obviously different from other general multiplayer messages.  The
// documentation is only for caravans, since the method used for marine is absolutely equilvance.
//
// In this version of trading system, the player can only select one trade unit to view the unit detail
// at any time and there are maximum 7 players in the game.  Thus, an array of size MAX_NATION is used
// to store the record number of trade units selected by each players.  A count, mp_selected_caravan_count,
// is used to count the number of selected units in the array, mp_selected_caravan_recno.
//
// The process that a player select a unit can be divided into three part, first frame to select the
// unit, keep selecting the unit and unselect the unit.  If the player selects a caravan, the flag
// mp_first_frame_to_select_caravan set to TRUE, mp_pre_selected_caravan_recno stores the record number
// of this unit.  From now on, the message MSG_U_CARA_SELECTED for the unit keep on sending to other
// players every frame until the unit is unselected.  Since the flag mp_first_frame_to_select_caravan is
// set to TRUE, this caravan information will not be shown in this frame.
//
// In the next frame, the flag mp_first_frame_to_select_caravan is set back to FLASE.  The message
// MSG_U_CARA_SELECTED sent in the previous frame reaches other players' channel.  The message takes effect
// to add a recno in the array mp_selected_caravan_recno.  For each record number in the array, the system
// update the caravan information.  The process continue and the cargo information is updated in real-time
// for the selected trade units.
//
//
// Functions description:
//
// mp_mark_selected_caravan()
// is called every frame to sent message to other players to notice them the recno of the trade unit selected
//
// mp_get_selected_caravan_count()
// for debug only
//
// mp_reset_selected_caravan_count()
// reset the array mp_selected_caravan_recno.
//
// mp_add_selected_caravan()
// handle message sent by other players to add recno in the array mp_selected_caravan_recno
//
// mp_is_selected_caravan()
// check whether the unit is selected, recno in the array mp_selected_caravan_recno
//
//------------------------------------------------------------------------------------------------------//



#include <OUNIT.h>
#include <OREMOTE.h>
#include <OU_MARI.h>
#include <ONATIONA.h>
#include <OCONFIG.h>

//------------ define static variables ------------//
static short	mp_selected_caravan_recno[MAX_NATION] = {0};
static int		mp_selected_caravan_count = 0;
static short	mp_selected_ship_recno[MAX_NATION] = {0};
static int		mp_selected_ship_count = 0;

//--------- Begin of function UnitArray::mp_mark_selected_caravan ---------//
void UnitArray::mp_mark_selected_caravan()
{
	//---------- send remote messages for selected caravan and ship ------------//
	if(remote.is_enable())
	{
		if(selected_recno)
		{
			Unit *selectedUnitPtr = (Unit*)get_ptr(selected_recno);
			//### begin alex 19/9 ###//
			//if(selectedUnitPtr->unit_id==UNIT_CARAVAN && selectedUnitPtr->nation_recno==nation_array.player_recno)
			if(selectedUnitPtr->unit_id==UNIT_CARAVAN && ((nation_array.player_recno &&
				selectedUnitPtr->nation_recno==nation_array.player_recno) || config.show_ai_info))
			//#### end alex 19/9 ####//
			{
				//---------- send message for multiplayer ---------//
				short* shortPtr = (short*) remote.new_send_queue_msg(MSG_U_CARA_SELECTED, sizeof(short));
				shortPtr[0] = selectedUnitPtr->sprite_recno;

				if(selected_recno==mp_pre_selected_caravan_recno)
					mp_first_frame_to_select_caravan = 0;
				else
				{
					mp_pre_selected_caravan_recno = selected_recno;
					mp_first_frame_to_select_caravan = 1;
				}
			}
			else
				mp_pre_selected_caravan_recno = 0;
		}
		else
			mp_pre_selected_caravan_recno = 0;

		mp_reset_selected_caravan_count();
	}
}
//----------- End of function UnitArray::mp_mark_selected_caravan -----------//


//--------- Begin of function UnitArray::mp_get_selected_caravan_count ---------//
// function only for debug
//
int UnitArray::mp_get_selected_caravan_count()
{
	//return selected_caravan_count;
	if(mp_selected_caravan_count)
		return mp_selected_caravan_recno[0];
	else
		return 0;
}
//----------- End of function UnitArray::mp_get_selected_caravan_count -----------//


//--------- Begin of function UnitArray::mp_reset_selected_caravan_count ---------//
void UnitArray::mp_reset_selected_caravan_count()
{
	mp_selected_caravan_count = 0;
}
//----------- End of function Unit::mp_reset_selected_caravan_count -----------//


//--------- Begin of function UnitArray::mp_add_selected_caravan ---------//
// the specified recno is one of the caravans selected in the previous frame
// by one of the players
//
void UnitArray::mp_add_selected_caravan(short unitRecno)
{
	err_when(mp_selected_caravan_count>=MAX_NATION);
	mp_selected_caravan_recno[mp_selected_caravan_count++] = unitRecno;
}
//----------- End of function UnitArray::mp_add_selected_caravan -----------//


//--------- Begin of function UnitArray::mp_is_selected_caravan ---------//
// return 1 if the caravan is selected in the previous frame by one of the players
// return 0 otherwise
//
int UnitArray::mp_is_selected_caravan(short unitRecno)
{
	err_when(mp_selected_caravan_count>=MAX_NATION);
	for(int i=mp_selected_caravan_count-1; i>=0; --i)
	{
		if(mp_selected_caravan_recno[i] == unitRecno)
			return 1;
	}

	return 0;
}
//----------- End of function UnitArray::mp_is_selected_caravan -----------//


//--------- Begin of function UnitArray::mp_mark_selected_ship ---------//
void UnitArray::mp_mark_selected_ship()
{
	//---------- send remote messages for selected ship ------------//
	if(remote.is_enable())
	{
		if(selected_recno)
		{
			Unit *selectedUnitPtr = (Unit*)get_ptr(selected_recno);
			//### begin alex 19/9 ###//
			//if(unit_res[selectedUnitPtr->unit_id]->unit_class == UNIT_CLASS_SHIP &&
			//	selectedUnitPtr->unit_id != UNIT_TRANSPORT && selectedUnitPtr->nation_recno == nation_array.player_recno)
			if(unit_res[selectedUnitPtr->unit_id]->unit_class == UNIT_CLASS_SHIP &&
				selectedUnitPtr->unit_id != UNIT_TRANSPORT && ((nation_array.player_recno &&
				selectedUnitPtr->nation_recno == nation_array.player_recno) || config.show_ai_info))
			//#### end alex 19/9 ####//
			{
				if(((UnitMarine*)selectedUnitPtr)->auto_mode)
				{
					//---------- send message for multiplayer ---------//
					short* shortPtr = (short*) remote.new_send_queue_msg(MSG_U_SHIP_SELECTED, sizeof(short));
					shortPtr[0] = selectedUnitPtr->sprite_recno;

					if(selected_recno==mp_pre_selected_ship_recno)
						mp_first_frame_to_select_ship = 0;
					else
					{
						mp_pre_selected_ship_recno = selected_recno;
						mp_first_frame_to_select_ship = 1;
					}
				}
				else
					mp_pre_selected_ship_recno = 0;
			}
			else
				mp_pre_selected_ship_recno = 0;
		}
		else
			mp_pre_selected_ship_recno = 0;

		mp_reset_selected_ship_count();
	}
}
//----------- End of function UnitArray::mp_mark_selected_ship -----------//


//--------- Begin of function UnitArray::mp_get_selected_ship_count ---------//
// function only for debug
//
int UnitArray::mp_get_selected_ship_count()
{
	//return selected_ship_count;
	if(mp_selected_ship_count)
		return mp_selected_ship_recno[0];
	else
		return 0;
}
//----------- End of function UnitArray::mp_get_selected_ship_count -----------//

  
//--------- Begin of function UnitArray::mp_reset_selected_ship_count ---------//
void UnitArray::mp_reset_selected_ship_count()
{
	mp_selected_ship_count = 0;
}
//----------- End of function Unit::mp_reset_selected_ship_count -----------//


//--------- Begin of function UnitArray::mp_add_selected_ship ---------//
// the specified recno is one of the ships selected in the previous frame
// by one of the players
//
void UnitArray::mp_add_selected_ship(short unitRecno)
{
	err_when(mp_selected_ship_count>=MAX_NATION);
	mp_selected_ship_recno[mp_selected_ship_count++] = unitRecno;
}
//----------- End of function UnitArray::mp_add_selected_ship -----------//


//--------- Begin of function UnitArray::mp_is_selected_ship ---------//
// return 1 if the ship is selected in the previous frame by one of the players
// return 0 otherwise
//
int UnitArray::mp_is_selected_ship(short unitRecno)
{
	err_when(mp_selected_ship_count>=MAX_NATION);
	for(int i=mp_selected_ship_count-1; i>=0; --i)
	{
		if(mp_selected_ship_recno[i] == unitRecno)
			return 1;
	}

	return 0;
}
//----------- End of function UnitArray::mp_is_selected_ship -----------//
