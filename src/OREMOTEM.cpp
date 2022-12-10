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

//Filename    : OREMOTEM.CPP
//Description : Object RemoteMsg

#include <ALL.h>
#include <OFONT.h>
#include <ONEWS.h>
#include <OSYS.h>
#include <ONATION.h>
#include <OUNIT.h>
#include <OGAME.h>
#include <OREMOTE.h>
#include <OFIRM.h>
#include <OFIRMALL.h>
#include <OTOWN.h>
#include <OLOG.h>
#include <OUNITALL.h>
#include <OSPY.h>
#include <OTALKRES.h>
#include <OLONGLOG.h>
#include <OCRC_STO.h>
// ##### patch begin Gilbert 20/1 #######//
#include <OBOX.h>
// ##### patch end Gilbert 20/1 #######//
#include <gettext.h>

//---------------- Define variable type ---------------//

typedef void (RemoteMsg::*MsgProcessFP)();

//----------- Define function pointers array -----------//

static MsgProcessFP msg_process_function_array[] =
{
	&RemoteMsg::queue_header,
	&RemoteMsg::queue_trailer,
	&RemoteMsg::new_nation,
	&RemoteMsg::update_game_setting,
	&RemoteMsg::start_game,
	&RemoteMsg::next_frame,
	&RemoteMsg::request_resend,
	&RemoteMsg::tell_send_time,
	&RemoteMsg::set_speed,
	&RemoteMsg::tell_random_seed,
	&RemoteMsg::request_save_game,
	&RemoteMsg::player_quit,

	&RemoteMsg::unit_stop,
	&RemoteMsg::unit_move,
	&RemoteMsg::unit_set_force_move,
	&RemoteMsg::unit_attack,
	&RemoteMsg::unit_assign,
	&RemoteMsg::unit_change_nation,
	&RemoteMsg::unit_build_firm,
	&RemoteMsg::unit_burn,
	&RemoteMsg::units_settle,
	&RemoteMsg::unit_set_guard,
	&RemoteMsg::unit_set_rank,
	&RemoteMsg::unit_dismount,
	&RemoteMsg::unit_reward,
	&RemoteMsg::units_transform,
	&RemoteMsg::unit_resign,
	&RemoteMsg::units_assign_to_ship,
	&RemoteMsg::units_ship_to_beach,
	&RemoteMsg::unit_succeed_king,
	&RemoteMsg::units_return_camp,
	&RemoteMsg::caravan_change_goods,
	&RemoteMsg::caravan_set_stop,
	&RemoteMsg::caravan_del_stop,
	&RemoteMsg::caravan_selected,
	&RemoteMsg::ship_unload_unit,
	&RemoteMsg::ship_unload_all_units,
	&RemoteMsg::ship_change_goods,
	&RemoteMsg::ship_set_stop,
	&RemoteMsg::ship_del_stop,
	&RemoteMsg::ship_change_mode,
	&RemoteMsg::ship_selected,
	&RemoteMsg::god_cast,
	&RemoteMsg::change_spy_nation,
	&RemoteMsg::notify_cloaked_nation,
	&RemoteMsg::unit_change_aggressive_mode,
	&RemoteMsg::spy_change_notify_flag,

	//#### trevor 15/10 ######//
	&RemoteMsg::spy_assassinate,
	//#### trevor 15/10 ######//

	&RemoteMsg::unit_add_way_point,

	&RemoteMsg::firm_sell,
	&RemoteMsg::firm_cancel,
	&RemoteMsg::firm_destruct,
	&RemoteMsg::firm_set_repair,
	&RemoteMsg::firm_train_level,
	&RemoteMsg::mobilize_worker,
	&RemoteMsg::mobilize_all_workers,
	&RemoteMsg::mobilize_overseer,
	&RemoteMsg::mobilize_builder,
	&RemoteMsg::firm_toggle_link_firm,
	&RemoteMsg::firm_toggle_link_town,
	&RemoteMsg::firm_pull_town_people,
	&RemoteMsg::firm_set_worker_home,
	&RemoteMsg::firm_bribe,
	&RemoteMsg::firm_capture,
	&RemoteMsg::firm_reward,
	&RemoteMsg::camp_patrol,
	&RemoteMsg::toggle_camp_patrol,
	&RemoteMsg::inn_hire,
	&RemoteMsg::market_scrap,
	&RemoteMsg::market_hire_caravan,
	&RemoteMsg::research_start,
	&RemoteMsg::build_weapon,
	&RemoteMsg::cancel_weapon,
	&RemoteMsg::skip_build_weapon,
	&RemoteMsg::build_ship,
	&RemoteMsg::sail_ship,
	&RemoteMsg::skip_build_ship,
	&RemoteMsg::factory_change_product,
	&RemoteMsg::base_mobilize_prayer,
	&RemoteMsg::invoke_god,

	&RemoteMsg::town_recruit,
	&RemoteMsg::town_skip_recruit,
	&RemoteMsg::town_migrate,
	&RemoteMsg::town_collect_tax,
	&RemoteMsg::town_reward,
	&RemoteMsg::town_toggle_link_firm,
	&RemoteMsg::town_toggle_link_town,
	&RemoteMsg::town_auto_tax,
	&RemoteMsg::town_auto_grant,
	&RemoteMsg::town_grant_independent,

	&RemoteMsg::wall_build,
	&RemoteMsg::wall_destruct,

	&RemoteMsg::spy_cycle_action,
	&RemoteMsg::spy_leave_town,
	&RemoteMsg::spy_leave_firm,
	&RemoteMsg::spy_capture_firm,
	&RemoteMsg::spy_drop_identity,
	&RemoteMsg::spy_reward,
	&RemoteMsg::spy_exposed,

	&RemoteMsg::send_talk_msg,
	&RemoteMsg::reply_talk_msg,
	&RemoteMsg::nation_contact,
	&RemoteMsg::nation_set_should_attack,

	&RemoteMsg::chat,

	&RemoteMsg::compare_remote_object,
	&RemoteMsg::compare_remote_object,
	&RemoteMsg::compare_remote_object,
	&RemoteMsg::compare_remote_object,
	&RemoteMsg::compare_remote_object,
	&RemoteMsg::compare_remote_object,
	&RemoteMsg::compare_remote_object,
	&RemoteMsg::compare_remote_object,

	&RemoteMsg::caravan_copy_route,
	&RemoteMsg::compare_remote_crc,
	&RemoteMsg::ship_copy_route,
	&RemoteMsg::firm_request_builder,
	&RemoteMsg::market_switch_restock,
};

//---------- Declare static functions ----------//

static void validate_selected_unit_array(short* selectedUnitArray, short& selectedCount);
static short validate_firm(short firmRecno, unsigned flags = 0);
static short validate_town(short townRecno, unsigned flags = 0);

//------ Begin of function RemoteMsg::process_msg -----//
//
void RemoteMsg::process_msg()
{
	if( id==0 )
		return;

	err_when( id<FIRST_REMOTE_MSG_ID || id>LAST_REMOTE_MSG_ID );

	MsgProcessFP msgProcessFP = msg_process_function_array[id-FIRST_REMOTE_MSG_ID];

	(this->*msgProcessFP)();   // call the corrsponding function to return the news process_msg
}
//------- End of function RemoteMsg::process_msg -----//


//-------- Begin of function RemoteMsg::queue_header ---------//
//
// Nothing here, it shouldn't be called at all.
//
void RemoteMsg::queue_header()
{
}
//--------- End of function RemoteMsg::queue_header ---------//


//-------- Begin of function RemoteMsg::queue_trailer ---------//
//
// Nothing here
//
void RemoteMsg::queue_trailer()
{
#ifdef DEBUG_LONG_LOG
	long_log->printf("Queue trailer of nation %d\n", *(short *)data_buf);
#endif
}
//--------- End of function RemoteMsg::queue_trailer ---------//


//-------- Begin of function RemoteMsg::new_nation ---------//
//
// Create a remote player.
//
// Sent from a client to the host when the client start a new nation.
//
// Client --> MSG_NEW_NATION --> Host
//
// Sent from the host to a newly joined client to update it with all
// existing nations.
//
// Host --> UPDATE_GAME_SETTING --> Client
//
// structure of data_buf:
//
// <Nation> - Nation object of the newly joined nation
//
void RemoteMsg::new_nation()
{
	//-------- create a new nation now --------//

	if( !remote.is_host )		// ignore this message if this is not the host
		return;

	Nation* nationPtr = (Nation*)(this->data_buf);

	int nationRecno = nation_array.new_nation( NATION_REMOTE, nationPtr->race_id, nationPtr->color_scheme_id, nationPtr->player_id );

	game.mp_broadcast_setting();
}
//--------- End of function RemoteMsg::new_nation ---------//


//-------- Begin of function RemoteMsg::update_game_setting ---------//
//
// Sent from the host to clients, update the client with the latest
// game settings.
//
// Host --> UPDATE_GAME_SETTING --> Client
//
// structure of data_buf:
//
// <int32_t>  - random seed
// <short>    - The number of nations joined
// <Nation..> - An array of nation objects
//
void RemoteMsg::update_game_setting()
{
	//------- get parameters -----------//

	char* dataPtr = data_buf;

	//------- set random seed -----------//

	misc.set_random_seed(*(int32_t*)dataPtr);
	dataPtr += sizeof(int32_t);

	//------- update nation_array -----------//

	int nationCount  = *(short*)dataPtr;
	dataPtr         += sizeof(short);

	char	  nationType;
	int	  i, ownCount=0;

	nation_array.deinit();		// deinit() first and then recreate it as follows

	for( i=0 ; i<nationCount ; i++ )
	{
		short nationRecno = *(short *)dataPtr;
		dataPtr += sizeof(short);
		PID_TYPE dpPlayerId = *(PID_TYPE *)dataPtr;
		dataPtr += sizeof(PID_TYPE);
		short colorSchemeId = *(short *)dataPtr;
		dataPtr += sizeof(short);
		short raceId = *(short *)dataPtr;
		dataPtr += sizeof(short);

		// int NationType;
		if( dpPlayerId == remote.self_player_id() )	// if this nation is the player's nation
		{
			nationType = NATION_OWN;
			ownCount++;
		}
		else
			nationType = NATION_REMOTE;

		if(nationRecno != nation_array.new_nation( nationType, raceId, colorSchemeId, dpPlayerId ))
		{
			// nation recno should match across all players
			err.run("Error in transmitting nation data");
		}
	}

	err_when( ownCount>1 );
}
//--------- End of function RemoteMsg::update_game_setting ---------//


//-------- Begin of function RemoteMsg::start_game ---------//
//
// The host sends MSG_START_GAME to the clients to notify them
// to start the game.
//
void RemoteMsg::start_game()
{
	game.started_flag = 1;
}
//--------- End of function RemoteMsg::start_game ---------//


//-------- Begin of function RemoteMsg::next_frame ---------//
//
// Notify the others that we are ready to proceed to the next
// frame.
//
// structure of data_buf:
//
// <short> - nation recno of the message queue
//
void RemoteMsg::next_frame()
{
	short nationRecno = *((short*)data_buf);

	if( !nation_array.is_deleted(nationRecno) )
	{
		nation_array[nationRecno]->next_frame_ready=1;
	}
}
//--------- End of function RemoteMsg::next_frame ---------//


//-------- Begin of function RemoteMsg::request_resend ---------//
//
// Request a specific player to resend its packets.
//
// structure of data_buf:
//
// <DWORD> - player id.
// <DWORD> - frame count of the message queue
//
void RemoteMsg::request_resend()
{
	uint32_t *dwordPtr = (uint32_t*) data_buf;

	err_when( dwordPtr[0] == (~nation_array)->player_id );   // sent to itself

	remote.send_backup_now(dwordPtr[0], dwordPtr[1]);
}
//--------- End of function RemoteMsg::request_resend ---------//


//-------- Begin of function RemoteMsg::tell_send_time ---------//
//
// Display the delivery time of the packet
//
void RemoteMsg::tell_send_time()
{
	String str;

	unsigned long sendTime = *((unsigned long*)data_buf);

	str  = "Packet Delivery Time: ";
	str += misc.get_time() - sendTime;
	str += " ms ";
	str += misc.get_time();

	font_san.disp( ZOOM_X1, 4, str, ZOOM_X1+249);
}
//--------- End of function RemoteMsg::tell_send_time ---------//


//-------- Begin of function RemoteMsg::set_speed ---------//
//
// Order the selected units to stop.
//
// structure of data_buf:
//
// <short>  - the game speed setting.
//
void RemoteMsg::set_speed()
{
	err_when( id != MSG_SET_SPEED);
	short* shortPtr = (short*) data_buf;

	sys.set_speed(shortPtr[0], COMMAND_REMOTE);		// 1-remote call
}
//--------- End of function RemoteMsg::set_speed ---------//


//-------- Begin of function RemoteMsg::tell_random_seed ---------//
//
// structure of data_buf:
//
// <short>  - nation recno
// <int32_t> - random seed
//
void RemoteMsg::tell_random_seed()
{
	// ######## patch begin Gilbert 20/1 #########//
	char *p = data_buf;
	short nationRecno = *(short *)p;
	p += sizeof(short);
	int32_t remoteSeed = *(int32_t *)p;

#if defined(DEBUG) && defined(ENABLE_LOG)
	String logLine("remote random seed ");
	logLine += nationRecno;
	logLine += ",";
	logLine += remoteSeed;
	
	LOG_MSG(logLine);
#endif

	// it assume random seed of each nation come in sequence
	// if may fails when connection lost

	static int32_t lastRemoteSeed = -1;
	static short lastNation = 0x7fff;
	if( nationRecno <= lastNation)
	{
		// assume the smallest human nation
		lastRemoteSeed = remoteSeed;
	}
	else
	{
		if( lastRemoteSeed != remoteSeed )
		{
#ifdef DEBUG_LONG_LOG
//			delete long_log;
//			long_log = NULL;
#endif
			LOG_DUMP;
			if( (remote.sync_test_level & 1) && (remote.sync_test_level >= 0) )
			{
				remote.sync_test_level = ~1;	// signal error encountered
				if( sys.debug_session )
					err.run( _("Multiplayer Random Seed Sync Error") );
			}
		}
	}
	lastNation = nationRecno;
	// ######## patch end Gilbert 20/1 #########//
}
//-------- End of function RemoteMsg::tell_random_seed ---------//


//-------- Begin of function RemoteMsg::request_save_game ---------//
void RemoteMsg::request_save_game()
{
	err_when( id != MSG_REQUEST_SAVE );
	// message struct : <DWORD> frame when the game should save
#ifdef DEBUG_LONG_LOG
	long_log->printf("Request save on %d\n", *(DWORD*)data_buf);
#endif
	sys.mp_request_save( *(uint32_t*)data_buf);
}
//-------- End of function RemoteMsg::request_save_game ---------//


//-------- Begin of function RemoteMsg::unit_stop ---------//
//
// Order the selected units to stop.
//
// structure of data_buf:
//
// <short>  - no. of selected unit.
// <char..> - selected unit recno array
//
void RemoteMsg::unit_stop()
{
	err_when( id != MSG_UNIT_STOP);
	short* shortPtr = (short*) data_buf;
	validate_selected_unit_array(shortPtr+1, shortPtr[0]);

	if(shortPtr[0] > 0)
	{
#ifdef DEBUG_LONG_LOG
		long_log->printf("stop units : ");
		for(int i = 0; i < shortPtr[0]; ++i)
		{
			long_log->printf("%d,", shortPtr[1+i]);
			if( i % 20 == 19)
				long_log->printf("\n");
		}
#endif
		unit_array.stop( shortPtr+1, shortPtr[0], COMMAND_REMOTE );   // 1-remote action
	}
}
//--------- End of function RemoteMsg::unit_stop ---------//


//-------- Begin of function RemoteMsg::unit_move ---------//
//
// Order the selected units to move to the specified location.
//
// structure of data_buf:
//
// <short>  - destXLoc
// <short>  - destYLoc
// <short>  - no. of selected unit.
// <char..> - selected unit recno array
//
void RemoteMsg::unit_move()
{
	err_when( id != MSG_UNIT_MOVE);
	short* shortPtr = (short*) data_buf;
	validate_selected_unit_array(shortPtr+4, shortPtr[2]);

	if( shortPtr[2] > 0)
	{
#ifdef DEBUG_LONG_LOG
		long_log->printf("move to (%d,%d), units : ", shortPtr[0], shortPtr[1]);
		for(int i = 0; i < shortPtr[2]; ++i)
		{
			long_log->printf("%d,", shortPtr[4+i]);
			if( i % 20 == 19)
				long_log->printf("\n");
		}
		long_log->printf("\n");
#endif
		unit_array.move_to( shortPtr[0], shortPtr[1], shortPtr[3], shortPtr+4, shortPtr[2], COMMAND_REMOTE );   // 1-remote action
	}
}
//--------- End of function RemoteMsg::unit_move ---------//


//--------- Begin of function RemoteMsg::unit_set_force_move ---------//
void RemoteMsg::unit_set_force_move()
{
	err_when( id != MSG_UNIT_SET_FORCE_MOVE);

	// packet structure : <unit count> <unit recno>...
	short* shortPtr = (short*) data_buf;
	validate_selected_unit_array(shortPtr+1, shortPtr[0]);

	if( shortPtr[0] > 0)
	{
		int i;
#ifdef DEBUG_LONG_LOG
		long_log->printf("set force move to , units : ");
		for(i = 0; i < shortPtr[0]; ++i)
		{
			long_log->printf("%d,", shortPtr[1+i]);
			if( i % 20 == 19)
				long_log->printf("\n");
		}
		long_log->printf("\n");
#endif

		for( i = 0; i < shortPtr[0]; ++i )
		{
			unit_array[shortPtr[1+i]]->force_move_flag = 1;
		}
	}
}
//--------- End of function RemoteMsg::unit_set_force_move ---------//


//-------- Begin of function RemoteMsg::unit_attack ---------//
//
// Order the selected units to attack at the specified location.
//
// structure of data_buf:
//
// <short>  - targetXLoc
// <short>  - targetYLoc
// <short>  - unitRecno
// <short>  - no. of selected unit.
// <short>  - divided;
// <short..> - selected unit recno array
//
void RemoteMsg::unit_attack()
{
	err_when( id != MSG_UNIT_ATTACK);
	short* shortPtr = (short*) data_buf;
	// ###### patch begin Gilbert 5/8 ###########//
	validate_selected_unit_array(shortPtr+5, shortPtr[3]);

	if( shortPtr[3] > 0)
	{
#ifdef DEBUG_LONG_LOG
		long_log->printf("attack (%d,%d), units : ", shortPtr[0], shortPtr[1]);
		for(int i = 0; i < shortPtr[3]; ++i)
		{
			long_log->printf("%d,", shortPtr[5+i]);
			if( i % 20 == 19)
				long_log->printf("\n");
		}
		long_log->printf("\n");
#endif
		int passCount = unit_array.divide_attack_by_nation(remote.nation_processing, shortPtr+5, shortPtr[3]);
		if( passCount > 0 )
			unit_array.attack( shortPtr[0], shortPtr[1], shortPtr[4], shortPtr+5, passCount, COMMAND_REMOTE, shortPtr[2] );   // 1-remote action
	}
	// ###### patch end Gilbert 5/8 ###########//
}
//--------- End of function RemoteMsg::unit_attack ---------//


//-------- Begin of function RemoteMsg::unit_assign ---------//
//
// Order the selected units to move to the specified location.
//
// structure of data_buf:
//
// <short>  - destXLoc
// <short>  - destYLoc
// <short>  - no. of selected unit.
// <char..> - selected unit recno array
//
void RemoteMsg::unit_assign()
{
	err_when( id != MSG_UNIT_ASSIGN);
	short* shortPtr = (short*) data_buf;
	validate_selected_unit_array(shortPtr+4, shortPtr[2]);

	if( shortPtr[2] > 0)
	{
#ifdef DEBUG_LONG_LOG
		long_log->printf("assign to (%d,%d), units : ", shortPtr[0], shortPtr[1]);
		for(int i = 0; i < shortPtr[2]; ++i)
		{
			long_log->printf("%d,", shortPtr[4+i]);
			if( i % 20 == 19)
				long_log->printf("\n");
		}
		long_log->printf("\n");
#endif
		unit_array.assign( shortPtr[0], shortPtr[1], shortPtr[3], COMMAND_REMOTE, shortPtr+4, shortPtr[2]);
	}
}
//--------- End of function RemoteMsg::unit_assign ---------//


//-------- Begin of function RemoteMsg::unit_change_nation ---------//
//
// Order the selected units to move to the specified location.
//
// structure of data_buf:
//
// <short>  - newNationRecno
// <short>  - no. of selected unit.
// <char..> - selected unit recno array
//
void RemoteMsg::unit_change_nation()
{
/*
	short* shortPtr = (short*) data_buf;

	short* selectedUnitArray = shortPtr+2;
	int	 selectedCount		 = shortPtr[1];

	validate_selected_unit_array(selectedUnitArray, selectedCount);

	unit_array.change_nation( shortPtr[0], selectedUnitArray, selectedCount, COMMAND_REMOTE );   // 1-remote action
*/
}
//--------- End of function RemoteMsg::unit_change_nation ---------//


//------ Begin of static function validate_selected_unit_array ------//
//
// Validate all units in selectedUnitArray, remove deleted units from
// selectedUnitArray.
//
static void validate_selected_unit_array(short* selectedUnitArray, short& selectedUnitCount)
{
	for( int i=0 ; i<selectedUnitCount ; i++ )
	{
		Unit* unitPtr;
		if( unit_array.is_deleted(selectedUnitArray[i]) || 
			!(unitPtr = unit_array[selectedUnitArray[i]]) ||
			!unitPtr->is_visible() || !unitPtr->is_nation(remote.nation_processing) )
		{
			memmove( selectedUnitArray+i, selectedUnitArray+i+1, sizeof(short) * (selectedUnitCount-i-1) );
			selectedUnitCount--;
			i--;							// stay with the current recno as the records have been moved. The recno in the current position is actually the next record.
		}
	}
}
//------- End of static function validate_selected_unit_array -------//


//------ Begin of static function validate_firm ------//
//
// return the firmRecno if the firm is controllable by the remote player
//
// bit 0 - skip firm's nation checking
// 
static short validate_firm(short firmRecno, unsigned flags)
{
	err_when( !(flags & 1) && remote.nation_processing == 0);
	Firm* firmPtr;
	if( firmRecno && !firm_array.is_deleted(firmRecno) 
		&& (firmPtr = firm_array[firmRecno]) 
		&& ((flags & 1) || firmPtr->nation_recno == remote.nation_processing) )
		return firmRecno;
	else
		return 0;
}
//------ End of static function validate_firm ------//


//------ Begin of static function validate_town ------//
//
// return the townRecno if the town is controllable by the remote player
//
// bit 0 - skip town's nation checking
// 
static short validate_town(short townRecno, unsigned flags)
{
	Town* townPtr;
	err_when( !(flags & 1) && remote.nation_processing == 0);
	if( townRecno && !town_array.is_deleted(townRecno) 
		&& (townPtr = town_array[townRecno]) 
		&& ((flags & 1) || townPtr->nation_recno == remote.nation_processing) )
		return townRecno;
	else
		return 0;
}
//------ End of static function validate_firm ------//


// ------- Begin of function RemoteMsg::unit_build ---------//
void RemoteMsg::unit_build_firm()
{
	err_when( id != MSG_UNIT_BUILD_FIRM);
	// packet structure : <unit recno> <xLoc> <yLoc> <firmId>
	short *shortPtr = (short *)data_buf;
	short unitCount =1;
	validate_selected_unit_array(shortPtr, unitCount);

	if( unitCount > 0 )
	{
#ifdef DEBUG_LONG_LOG
		long_log->printf("unit %d build firm %d at (%d,%d)\n", shortPtr[0], shortPtr[3], shortPtr[1], shortPtr[2]);
#endif
		unit_array[*shortPtr]->build_firm( shortPtr[1], shortPtr[2],
		shortPtr[3], COMMAND_REMOTE );
	}
}
// ------- End of function RemoteMsg::unit_build_firm ---------//


// ------- Begin of function RemoteMsg::unit_burn ---------//
void RemoteMsg::unit_burn()
{
	err_when( id != MSG_UNIT_BURN);
	// packet structure : <unit recno> <xLoc> <yLoc>
	short *shortPtr = (short *)data_buf;
	short unitCount =1;
	validate_selected_unit_array(shortPtr, unitCount);

	if( unitCount > 0 )
	{
#ifdef DEBUG_LONG_LOG
		long_log->printf("unit %d burn at (%d,%d)\n", shortPtr[0], shortPtr[1], shortPtr[2]);
#endif
		unit_array[*shortPtr]->burn(shortPtr[1], shortPtr[2], COMMAND_REMOTE);
	}
}
// ------- End of function RemoteMsg::unit_burn ---------//


// ------- Begin of function RemoteMsg::units_settle ---------//
void RemoteMsg::units_settle()
{		
	err_when( id != MSG_UNITS_SETTLE);
	// packet structure : <xLoc> <yLoc> <no. of units> <divided> <unit recno ...>
	short *shortPtr = (short *)data_buf;
	validate_selected_unit_array(shortPtr+4, shortPtr[2]);

	if( shortPtr[2] > 0)
	{
#ifdef DEBUG_LONG_LOG
		long_log->printf("settle at (%d,%d), units : ", shortPtr[0], shortPtr[1]);
		for(int i = 0; i < shortPtr[2]; ++i)
		{
			long_log->printf("%d,", shortPtr[4+i]);
			if( i % 20 == 19)
				long_log->printf("\n");
		}
		long_log->printf("\n");
#endif
		unit_array.settle(shortPtr[0], shortPtr[1], shortPtr[3], COMMAND_REMOTE, shortPtr+4, shortPtr[2]);
	}
}
// ------- End of function RemoteMsg::units_settle ---------//


// ------- Begin of function RemoteMsg::unit_set_guard ---------//
//
// **BUGHERE, this function is no longer needed.
//
void RemoteMsg::unit_set_guard()
{		
/*
	err_when( id != MSG_UNIT_SET_GUARD);
	// packet structure : <unit recno> <new guard mode 0/1>
	short *shortPtr = (short *)data_buf;
	unit_array[*shortPtr]->guard_mode = (char) shortPtr[1];
*/
}
// ------- End of function RemoteMsg::unit_set_guard ---------//


// ------- Begin of function RemoteMsg::unit_set_rank ---------//
void RemoteMsg::unit_set_rank()
{		
	err_when( id != MSG_UNIT_SET_RANK);
	// packet structure : <unit recno> <new rank>
	short *shortPtr = (short *)data_buf;
	short unitCount =1;
	validate_selected_unit_array(shortPtr, unitCount);

	// ignore <new rank> parameter
	//	unit_array[*shortPtr]->set_rank(shortPtr[1]);
	if( unitCount > 0)
	{
#ifdef DEBUG_LONG_LOG
		long_log->printf("unit %d promote/demote to %d\n", shortPtr[0], shortPtr[1]);
#endif
		switch(unit_array[*shortPtr]->rank_id)
		{
		case RANK_SOLDIER:
			unit_array[*shortPtr]->set_rank(RANK_GENERAL);
			break;
		case RANK_GENERAL:
			unit_array[*shortPtr]->set_rank(RANK_SOLDIER);
			break;
		}
	}
}
// ------- End of function RemoteMsg::unit_set_rank ---------//


// ------- Begin of function RemoteMsg::unit_dismount ---------//
void RemoteMsg::unit_dismount()
{		
	err_when( id != MSG_UNIT_DISMOUNT);
	// packet structure : <unit recno>
	short *shortPtr = (short *)data_buf;
	short unitCount =1;
	validate_selected_unit_array(shortPtr, unitCount);

	if( unitCount > 0 )
	{
#ifdef DEBUG_LONG_LOG
		long_log->printf("unit %d dismount\n", shortPtr[0]);
#endif
		UnitVehicle *uv = (UnitVehicle *) unit_array[*shortPtr];
		uv->dismount();
	}
}
// ------- End of function RemoteMsg::unit_dismount ---------//


// ------- Begin of function RemoteMsg::unit_reward ---------//
void RemoteMsg::unit_reward()
{
	//###### begin trevor 9/6 #######//

	err_when( id != MSG_UNIT_REWARD);
	// packet structure : <unit recno> <rewarding nation recno>
	short *shortPtr = (short *)data_buf;
	short unitCount =1;
	validate_selected_unit_array(shortPtr, unitCount);

	if( unitCount > 0 )
	{
#ifdef DEBUG_LONG_LOG
		long_log->printf("nation %d rewards unit %d\n", shortPtr[1], shortPtr[0]);
#endif
		unit_array[*shortPtr]->reward(shortPtr[1]);
	}

	//###### end trevor 9/6 #######//
}
// ------- End of function RemoteMsg::unit_reward ---------//


// ------- Begin of function RemoteMsg::units_transform ------//
void RemoteMsg::units_transform()
{
	err_when( id != MSG_UNITS_TRANSFORM );
	// packet structure <this recno> <no. of units> <unit recno> ...
	short *shortPtr = (short *)data_buf;
	validate_selected_unit_array(shortPtr+2, shortPtr[1]);

	if( unit_array.is_deleted(*shortPtr) )
	{
		// if <this recno> is dead, use the first unit in
		// the unit group
		if( shortPtr[1] >= 1)
		{
			*shortPtr = shortPtr[2];	
			unit_array[*shortPtr]->group_transform(COMMAND_REMOTE,
				shortPtr +2, shortPtr[1]);
		}
	}
	else
	{
		unit_array[*shortPtr]->group_transform(COMMAND_REMOTE,
			shortPtr +2, shortPtr[1]);
	}
}
// ------- End of function RemoteMsg::units_transform ------//


// ------- Begin of function RemoteMsg::unit_resign ---------//
void RemoteMsg::unit_resign()
{		
	err_when( id != MSG_UNIT_RESIGN);
	// packet structure : <unit recno> <nation recno>
	short *shortPtr = (short *)data_buf;
	short unitCount =1;
	validate_selected_unit_array(shortPtr, unitCount);

	if( unitCount > 0 && unit_array[*shortPtr]->is_nation(shortPtr[1])
		&& unit_array[*shortPtr]->can_resign() )
	{
#ifdef DEBUG_LONG_LOG
		long_log->printf("nation %d resigns unit %d\n", shortPtr[1], shortPtr[0]);
#endif
		unit_array[*shortPtr]->resign(COMMAND_REMOTE);
	}
}
// ------- End of function RemoteMsg::unit_resign ---------//


// ------- Begin of function RemoteMsg::units_assign_to_ship ---------//
void RemoteMsg::units_assign_to_ship()
{		
	err_when( id != MSG_UNITS_ASSIGN_TO_SHIP);
	// ##### patch begin Gilbert 5/8 ######//
	// packet structure : <xLoc> <yLoc> <ship recno> <no. of units> <divided> <unit recno ...>
	short *shortPtr = (short *)data_buf;
	validate_selected_unit_array(shortPtr+5, shortPtr[3]);

	if( shortPtr[3] > 0)
	{
#ifdef DEBUG_LONG_LOG
		long_log->printf("assign to ship at (%d,%d), units : ", shortPtr[0], shortPtr[1]);
		for(int i = 0; i < shortPtr[3]; ++i)
		{
			long_log->printf("%d,", shortPtr[5+i]);
			if( i % 20 == 19)
				long_log->printf("\n");
		}
		long_log->printf("\n");
#endif
		unit_array.assign_to_ship(shortPtr[0], shortPtr[1], shortPtr[4],  shortPtr+5, shortPtr[3], COMMAND_REMOTE, shortPtr[2]);
	}
	// ##### patch end Gilbert 5/8 ######//
}
// ------- End of function RemoteMsg::units_assign_to_ship ---------//


// ------- Begin of function RemoteMsg::units_ship_to_beach ---------//
void RemoteMsg::units_ship_to_beach()
{
	err_when( id != MSG_UNITS_SHIP_TO_BEACH);
	// packet structure : <xLoc> <yLoc> <no. of units> <divided> <unit recno ...>
	short *shortPtr = (short *)data_buf;
	validate_selected_unit_array(shortPtr+4, shortPtr[2]);

	if( shortPtr[2] > 0)
	{
#ifdef DEBUG_LONG_LOG
		long_log->printf("move ships to beach at (%d,%d), units : ", shortPtr[0], shortPtr[1]);
		for(int i = 0; i < shortPtr[2]; ++i)
		{
			long_log->printf("%d,", shortPtr[4+i]);
			if( i % 20 == 19)
				long_log->printf("\n");
		}
		long_log->printf("\n");
#endif
		unit_array.ship_to_beach(shortPtr[0], shortPtr[1], shortPtr[3],  shortPtr+4, shortPtr[2], COMMAND_REMOTE);
	}
}
// ------- End of function RemoteMsg::units_assign_to_ship ---------//


// ------- Begin of function RemoteMsg::unit_succeed_king ------//
void RemoteMsg::unit_succeed_king()
{
	err_when( id != MSG_UNIT_SUCCEED_KING);
	// packet structure : <unit recno> <nation recno>
	short *shortPtr = (short *)data_buf;

	short unitCount =1;
	validate_selected_unit_array(shortPtr, unitCount);

	if( unitCount > 0 && !nation_array.is_deleted(shortPtr[1]) && 
		unit_array[*shortPtr]->nation_recno == shortPtr[1] )
	{
#ifdef DEBUG_LONG_LOG
		long_log->printf("unit %d succeed nation %d\n", shortPtr[0], shortPtr[1]);
#endif
		nation_array[shortPtr[1]]->succeed_king(*shortPtr);
		if(unit_array.selected_recno == *shortPtr)
			info.disp();
	}
}
// ------- End of function RemoteMsg::unit_succeed_king ------//


// ------- Begin of function RemoteMsg::units_return_camp ------//
void RemoteMsg::units_return_camp()
{
	err_when( id != MSG_UNITS_RETURN_CAMP );
	// packet structure : <no. of units> <unit recno ...>
	short *shortPtr = (short *)data_buf;
	validate_selected_unit_array(shortPtr+1, *shortPtr);

	if( *shortPtr > 0)
	{
#ifdef DEBUG_LONG_LOG
		long_log->printf("return to camp, units : ");
		for(int i = 0; i < *shortPtr; ++i)
		{
			long_log->printf("%d,", shortPtr[1+i]);
			if( i % 20 == 19)
				long_log->printf("\n");
		}
		long_log->printf("\n");
#endif
		unit_array.return_camp(COMMAND_REMOTE, shortPtr+1, *shortPtr);
	}
}


// ------- Begin of function RemoteMsg::caravan_change_goods ------//
void RemoteMsg::caravan_change_goods()
{
	err_when( id != MSG_U_CARA_CHANGE_GOODS );
	// packet structure <unit recno> <stop id> <new pick_up_type>
	short *shortPtr = (short *)data_buf;
	short unitCount =1;
	validate_selected_unit_array(shortPtr, unitCount);

	if( unitCount > 0)
	{
		Unit *unitPtr = unit_array[*shortPtr];
		UnitCaravan *caravanPtr;
		if( unitPtr->unit_id != UNIT_CARAVAN)
		{
			err_here();
		}
		else
		{
#ifdef DEBUG_LONG_LOG
		long_log->printf("caravan %d change goods row %d, %d\n", shortPtr[0], shortPtr[1], shortPtr[2]);
#endif
			caravanPtr = (UnitCaravan *)unitPtr;
			// caravanPtr->stop_array[shortPtr[1]].pick_up_type = (char) shortPtr[2];
			caravanPtr->set_stop_pick_up(shortPtr[1], shortPtr[2], COMMAND_REMOTE);

			//if( unit_array.selected_recno == *shortPtr )
			//	info.disp();
		}
	}
}
// ------- End of function RemoteMsg::caravan_change_goods ------//


// ------- Begin of function RemoteMsg::caravan_set_stop ------//
void RemoteMsg::caravan_set_stop()
{
	err_when( id != MSG_U_CARA_SET_STOP );
	// packet structure <unit recno> <stop id> <stop x> <stop y>
	short *shortPtr = (short *)data_buf;
	short unitCount =1;
	validate_selected_unit_array(shortPtr, unitCount);

	if( unitCount > 0)
	{
		Unit *unitPtr = unit_array[*shortPtr];
		UnitCaravan *caravanPtr;
		if( unitPtr->unit_id != UNIT_CARAVAN)
		{
			err_here();
		}
		else
		{
#ifdef DEBUG_LONG_LOG
			long_log->printf("caravan %d set stop %d at (%d,%d)\n", shortPtr[0], shortPtr[1], shortPtr[2], shortPtr[3]);
#endif
			caravanPtr = (UnitCaravan *)unitPtr;
			caravanPtr->set_stop(shortPtr[1], shortPtr[2], shortPtr[3], COMMAND_REMOTE);
			// if( unit_array.selected_recno == *shortPtr )
			//	info.disp();
		}
	}
}
// ------- End of function RemoteMsg::caravan_set_stop ------//


// ------- Begin of function RemoteMsg::caravan_del_stop ------//
void RemoteMsg::caravan_del_stop()
{
	err_when( id != MSG_U_CARA_DEL_STOP );
	// packet structure <unit recno> <stop id>
	short *shortPtr = (short *)data_buf;
	short unitCount =1;
	validate_selected_unit_array(shortPtr, unitCount);

	if( unitCount > 0)
	{
		Unit *unitPtr = unit_array[*shortPtr];
		UnitCaravan *caravanPtr;
		if( unitPtr->unit_id != UNIT_CARAVAN)
		{
			err_here();
		}
		else
		{
#ifdef DEBUG_LONG_LOG
			long_log->printf("caravan %d delete stop %d, %d\n", shortPtr[0], shortPtr[1]);
#endif
			caravanPtr = (UnitCaravan *)unitPtr;
			caravanPtr->del_stop(shortPtr[1], COMMAND_REMOTE);
			//if( unit_array.selected_recno == *shortPtr )
			//	info.disp();
		}
	}
}
// ------- End of function RemoteMsg::caravan_del_stop ------//


// ------- Begin of function RemoteMsg::caravan_copy_route ------//
void RemoteMsg::caravan_copy_route()
{
	err_when( id != MSG_U_CARA_COPY_ROUTE);
	// packet structure : <unit recno> <copy unit recno>
	short *shortPtr = (short *)data_buf;
	short unitCount= 2;
	validate_selected_unit_array(shortPtr, unitCount);

	if( unitCount > 0)
	{
		Unit *unitPtr = unit_array[*shortPtr];
		UnitCaravan *caravanPtr;
		if( unitPtr->unit_id != UNIT_CARAVAN)
		{
			err_here();
		}
		else
		{
#ifdef DEBUG_LONG_LOG
			long_log->printf("caravan %d copy route from %d\n", shortPtr[0], shortPtr[1]);
#endif
			caravanPtr = (UnitCaravan *)unitPtr;
			caravanPtr->copy_route(shortPtr[1], COMMAND_REMOTE);
		}
	}
}
// ------- End of function RemoteMsg::caravan_copy_route ------//


// ------- Begin of function RemoteMsg::ship_unload_unit ---------//
void RemoteMsg::ship_unload_unit()
{
	err_when( id != MSG_U_SHIP_UNLOAD_UNIT );
	// packet structure <unit recno> <unitSeqId>
	short *shortPtr = (short *)data_buf;
	short unitCount =1;
	validate_selected_unit_array(shortPtr, unitCount);

	if( unitCount > 0)
	{
		Unit *unitPtr = unit_array[*shortPtr];
		if( unitPtr->sprite_info->sprite_type == 'U' 
			&& unitPtr->sprite_info->sprite_sub_type == 'M')
		{
			UnitMarine *shipPtr = (UnitMarine *)unitPtr;

			if( shortPtr[1] <= shipPtr->unit_count )
			{
				// check if the unit is a ship
#ifdef DEBUG_LONG_LOG
				long_log->printf("ship %d unload unit %d\n", shortPtr[0], shortPtr[1]);
#endif
				shipPtr->unload_unit(shortPtr[1], COMMAND_REMOTE);
				if( unit_array.selected_recno == *shortPtr )
					info.disp();
			}
		}
		else
		{
			err_here();
		}
	}
}
// ------- End of function RemoteMsg::ship_unload_unit ---------//


// ----- Begin of function RemoteMsg::ship_unload_all_units ----- //
void RemoteMsg::ship_unload_all_units()
{
	err_when( id != MSG_U_SHIP_UNLOAD_ALL_UNITS );
	// packet structure <unit recno>
	short *shortPtr = (short *)data_buf;
	short unitCount =1;
	validate_selected_unit_array(shortPtr, unitCount);

	if( unitCount > 0)
	{
		Unit *unitPtr = unit_array[*shortPtr];
		if( unitPtr->sprite_info->sprite_type == 'U' 
			&& unitPtr->sprite_info->sprite_sub_type == 'M')
		{
			// check if the unit is a ship
#ifdef DEBUG_LONG_LOG
			long_log->printf("ship %d unload all units\n", shortPtr[0]);
#endif
			UnitMarine *shipPtr = (UnitMarine *)unitPtr;
			shipPtr->unload_all_units(COMMAND_REMOTE);
			if( unit_array.selected_recno == *shortPtr )
				info.disp();
		}
		else
		{
			err_here();
		}
	}
}
// ----- End of function RemoteMsg::ship_unload_all_units ----- //


// ------- Begin of function RemoteMsg::ship_change_goods ------//
void RemoteMsg::ship_change_goods()
{
	err_when( id != MSG_U_SHIP_CHANGE_GOODS );
	// packet structure <unit recno> <stop id> <new pick_up_type>
	short *shortPtr = (short *)data_buf;
	short unitCount =1;
	validate_selected_unit_array(shortPtr, unitCount);

	if( unitCount > 0)
	{
		Unit *unitPtr = unit_array[*shortPtr];
		UnitMarine *shipPtr;
		if( unitPtr->sprite_info->sprite_sub_type != 'M')
		{
			err_here();
		}
		else
		{
#ifdef DEBUG_LONG_LOG
			long_log->printf("ship %d change goods, row %d, %d\n", shortPtr[0], shortPtr[1], shortPtr[2]);
#endif
			shipPtr = (UnitMarine *)unitPtr;
			// shipPtr->stop_array[shortPtr[1]].pick_up_type = (char) shortPtr[2];
			shipPtr->set_stop_pick_up(shortPtr[1], shortPtr[2], COMMAND_REMOTE);
			// if( unit_array.selected_recno == *shortPtr )
			//	info.disp();
		}
	}
}
// ------- End of function RemoteMsg::ship_change_goods ------//


// ------- Begin of function RemoteMsg::ship_set_stop ------//
void RemoteMsg::ship_set_stop()
{
	err_when( id != MSG_U_SHIP_SET_STOP );
	// packet structure <unit recno> <stop id> <stop x> <stop y>
	short *shortPtr = (short *)data_buf;
	short unitCount =1;
	validate_selected_unit_array(shortPtr, unitCount);

	if( unitCount > 0)
	{
		Unit *unitPtr = unit_array[*shortPtr];
		UnitMarine *shipPtr;
		if( unitPtr->sprite_info->sprite_sub_type != 'M')
		{
			err_here();
		}
		else
		{
#ifdef DEBUG_LONG_LOG
			long_log->printf("ship %d set stop %d at (%d,%d)\n", shortPtr[0], shortPtr[1], shortPtr[2], shortPtr[3]);
#endif
			shipPtr = (UnitMarine *)unitPtr;
			shipPtr->set_stop(shortPtr[1], shortPtr[2], shortPtr[3], COMMAND_REMOTE);
		}
	}
}
// ------- End of function RemoteMsg::ship_set_stop ------//


// ------- Begin of function RemoteMsg::ship_del_stop ------//
void RemoteMsg::ship_del_stop()
{
	err_when( id != MSG_U_SHIP_DEL_STOP );
	// packet structure <unit recno> <stop id>
	short *shortPtr = (short *)data_buf;
	short unitCount =1;
	validate_selected_unit_array(shortPtr, unitCount);

	if( unitCount > 0)
	{
		Unit *unitPtr = unit_array[*shortPtr];
		UnitMarine *shipPtr;
		if( unitPtr->sprite_info->sprite_sub_type != 'M')
		{
			err_here();
		}
		else
		{
#ifdef DEBUG_LONG_LOG
			long_log->printf("ship %d delete stop %d\n", shortPtr[0], shortPtr[1]);
#endif
			shipPtr = (UnitMarine *)unitPtr;
			shipPtr->del_stop(shortPtr[1], COMMAND_REMOTE);
		}
	}
}
// ------- End of function RemoteMsg::ship_del_stop ------//


// ------- Begin of function RemoteMsg::ship_change_mode ------//
void RemoteMsg::ship_change_mode()
{
	err_when( id != MSG_U_SHIP_CHANGE_MODE );
	// packet structure <unit recno> <new mode>
	short *shortPtr = (short *)data_buf;
	short unitCount =1;
	validate_selected_unit_array(shortPtr, unitCount);

	if( unitCount > 0)
	{
		Unit *unitPtr = unit_array[*shortPtr];
		UnitMarine *shipPtr;
		if( unitPtr->sprite_info->sprite_sub_type != 'M')
		{
			err_here();
		}
		else
		{
#ifdef DEBUG_LONG_LOG
			long_log->printf("ship %d changes mode %d\n", shortPtr[0], shortPtr[1]);
#endif
			shipPtr = (UnitMarine *)unitPtr;
			shipPtr->auto_mode = (char) shortPtr[1];

			if(*shortPtr==unit_array.selected_recno)
				info.disp();
		}
	}
}
// ------- End of function RemoteMsg::ship_change_mode ------//


// ------- Begin of function RemoteMsg::ship_copy_route ------//
void RemoteMsg::ship_copy_route()
{
	err_when( id != MSG_U_SHIP_COPY_ROUTE);
	// packet structure : <unit recno> <copy unit recno>
	short *shortPtr = (short *)data_buf;
	short unitCount= 2;
	validate_selected_unit_array(shortPtr, unitCount);

	if( unitCount > 0)
	{
		Unit *unitPtr = unit_array[*shortPtr];
		UnitMarine *shipPtr;
		if( unitPtr->unit_id != UNIT_VESSEL )
		{
			err_here();
		}
		else
		{
#ifdef DEBUG_LONG_LOG
			long_log->printf("ship %d copy route from %d\n", shortPtr[0], shortPtr[1]);
#endif
			shipPtr = (UnitMarine *)unitPtr;
			shipPtr->copy_route(shortPtr[1], COMMAND_REMOTE);
		}
	}
}
// ------- End of function RemoteMsg::ship_copy_route ------//


// ------- Begin of function RemoteMsg::change_spy_nation ------//
void RemoteMsg::change_spy_nation()
{
	err_when( id != MSG_UNIT_SPY_NATION );
	// packet structure <unit recno> <new nation recno> <group defect>
	short *shortPtr = (short *)data_buf;
	short unitCount =1;
	validate_selected_unit_array(shortPtr, unitCount);

	if( unitCount > 0)
	{
#ifdef DEBUG_LONG_LOG
		long_log->printf("unit %d changes nation %d (groupDefect=%d)\n", shortPtr[0], shortPtr[1], shortPtr[2]);
#endif
		unit_array[*shortPtr]->spy_change_nation(shortPtr[1], COMMAND_REMOTE, shortPtr[2]);
		if( unit_array.selected_recno == *shortPtr || 
			unit_array[*shortPtr]->selected_flag)
			info.disp();
	}
}
// ------- End of function RemoteMsg::change_spy_nation ------//


// ------- Begin of function RemoteMsg::notify_cloaked_nation ------//
void RemoteMsg::notify_cloaked_nation()
{
	err_when( id != MSG_UNIT_SPY_NOTIFY_CLOAKED_NATION );

	// packet structure <unit recno> <new nation recno>
	short *shortPtr = (short *)data_buf;
	short unitCount =1;
	validate_selected_unit_array(shortPtr, unitCount);

	if( unitCount > 0)
	{
		if( !unit_array[*shortPtr]->spy_recno )
		{
			err_here();
		}
		else
		{
#ifdef DEBUG_LONG_LOG
			long_log->printf("unit %d notify cloaked nation %d\n", shortPtr[0], shortPtr[1]);
#endif
			spy_array[unit_array[*shortPtr]->spy_recno]->notify_cloaked_nation_flag
				= (char) shortPtr[1];
		}
		if( unit_array.selected_recno == *shortPtr || 
			unit_array[*shortPtr]->selected_flag)
			info.disp();
	}
}
// ------- End of function RemoteMsg::notify_cloaked_nation ------//


//------ Begin of function RemoteMsg::unit_change_aggressive_mode -----//
//
void RemoteMsg::unit_change_aggressive_mode()
{
	err_when( id != MSG_UNIT_CHANGE_AGGRESSIVE_MODE);

	// packet structure : <unit recno> <new aggressive mode 0/1>

	short *shortPtr = (short *)data_buf;
	short unitCount =1;
	validate_selected_unit_array(shortPtr, unitCount);

	if( unitCount > 0 )
	{
#ifdef DEBUG_LONG_LOG
		long_log->printf("unit %d %s aggressive mode\n", shortPtr[0], shortPtr[1] ? "set" : "clear");
#endif
		unit_array[*shortPtr]->aggressive_mode = (char) shortPtr[1];
	}
}
//------- End of function RemoteMsg::unit_change_aggressive_mode ------//


//------ Begin of function RemoteMsg::spy_change_notify_flag -----//
//
void RemoteMsg::spy_change_notify_flag()
{
	err_when( id != MSG_SPY_CHANGE_NOTIFY_FLAG) ;

	// packet structure : <spy recno> <new notify flag 0/1>

	short *shortPtr = (short *)data_buf;
	spy_array[*shortPtr]->notify_cloaked_nation_flag = (char) shortPtr[1];
}
//------- End of function RemoteMsg::spy_change_notify_flag ------//


//########## begin trevor 15/10 #############//


//------ Begin of function RemoteMsg::spy_assassinate -----//
//
void RemoteMsg::spy_assassinate()
{
	err_when( id != MSG_SPY_ASSASSINATE );

	// packet structure : <spy recno> <assassinate target unit recno>

	short *shortPtr = (short *)data_buf;
	if( !spy_array.is_deleted(*shortPtr) )
	{
#ifdef DEBUG_LONG_LOG
		long_log->printf("spy %d assassinate unit %d", shortPtr[0], shortPtr[1]);
#endif
		spy_array[*shortPtr]->assassinate( shortPtr[1], COMMAND_REMOTE );
	}
}
//------- End of function RemoteMsg::spy_assassinate ------//


//########## end trevor 15/10 #############//


// ------- Begin of function RemoteMsg::firm_sell ---------//
void RemoteMsg::firm_sell()
{
	err_when( id != MSG_FIRM_SELL);
	// packet structure : <firm recno>
	short *shortPtr = (short *)data_buf;

	if( validate_firm(*shortPtr) )
	{
#ifdef DEBUG_LONG_LOG
		long_log->printf("sell firm %d\n", shortPtr[0]);
#endif
		firm_array[*shortPtr]->sell_firm(COMMAND_REMOTE);
	}
}
// ------- End of function RemoteMsg::firm_sell ---------//


// ------- Begin of function RemoteMsg::firm_cancel ---------//
void RemoteMsg::firm_cancel()
{
	err_when( id != MSG_FIRM_CANCEL);
	short *shortPtr = (short *)data_buf;

	if( validate_firm(*shortPtr) )
	{
#ifdef DEBUG_LONG_LOG
		long_log->printf("firm %d cancel construction\n", shortPtr[0]);
#endif
		firm_array[*shortPtr]->cancel_construction(COMMAND_REMOTE);
	}
}
// ------- End of function RemoteMsg::firm_cancel ---------//


// ------- Begin of function RemoteMsg::firm_destruct ---------//
void RemoteMsg::firm_destruct()
{
	err_when( id != MSG_FIRM_DESTRUCT);
	// packet structure : <firm recno>
	short *shortPtr = (short *)data_buf;

	if( validate_firm(*shortPtr) )
	{
#ifdef DEBUG_LONG_LOG
		long_log->printf("destruct firm %d\n", shortPtr[0]);
#endif
		firm_array[*shortPtr]->destruct_firm(COMMAND_REMOTE);
	}
}
// ------- End of function RemoteMsg::firm_destruct ---------//


// ------- Begin of function RemoteMsg::firm_set_repair ---------//
void RemoteMsg::firm_set_repair()
{
	//##### begin trevor 19/6 ######//
/*
	err_when( id != MSG_FIRM_SET_REPAIR);
	// packet structure : <firm recno> <new setting>
	short *shortPtr = (short *)data_buf;

	if( validate_firm(*shortPtr) )
		firm_array[*shortPtr]->is_repairing = (char)shortPtr[1];
*/
	//##### end trevor 19/6 ######//
}
// ------- End of function RemoteMsg::firm_set_repair ---------//


// ------- Begin of function RemoteMsg::firm_train_level ---------//
void RemoteMsg::firm_train_level()
{
/*		//**BUGHERE, no more training in game 
	err_when( id != MSG_FIRM_TRAIN_LEVEL);
	// packet structure : <firm recno> <new train level>
	short *shortPtr = (short *)data_buf;

	if( !firm_array.is_deleted(*shortPtr) )
		firm_array[*shortPtr]->train_level = (char)shortPtr[1];
*/
}
// ------- End of function RemoteMsg::firm_train_level ---------//


// ------- Begin of function RemoteMsg::mobilize_worker ---------//
void RemoteMsg::mobilize_worker()
{
	err_when( id != MSG_FIRM_MOBL_WORKER);
	// packet structure : <firm recno> <workerId>
	short *shortPtr = (short *)data_buf;

	if( validate_firm(*shortPtr) && shortPtr[1] <= firm_array[*shortPtr]->worker_count)
	{
#ifdef DEBUG_LONG_LOG
		long_log->printf("firm %d mobilize worker %d\n", shortPtr[0], shortPtr[1]);
#endif
		firm_array[*shortPtr]->mobilize_worker(shortPtr[1],COMMAND_REMOTE);
	}
}
// ------- End of function RemoteMsg::mobilize_worker ---------//


// ------- Begin of function RemoteMsg::mobilize_all_workers ---------//
void RemoteMsg::mobilize_all_workers()
{
	err_when( id != MSG_FIRM_MOBL_ALL_WORKERS );
	// packet structure : <firm recno>
	short *shortPtr = (short *)data_buf;

	if( validate_firm(*shortPtr) )
	{
#ifdef DEBUG_LONG_LOG
		long_log->printf("firm %d mobilize all workers\n", shortPtr[0]);
#endif
		firm_array[*shortPtr]->mobilize_all_workers(COMMAND_REMOTE);
	}
}
// ------- End of function RemoteMsg::mobilize_all_workers ---------//


// ------- Begin of function RemoteMsg::mobilize_overseer ---------//
void RemoteMsg::mobilize_overseer()
{
	err_when( id != MSG_FIRM_MOBL_OVERSEER);
	// packet structure : <firm recno>
	short *shortPtr = (short *)data_buf;

	if( validate_firm(*shortPtr) && firm_array[*shortPtr]->overseer_recno )
	{
#ifdef DEBUG_LONG_LOG
		long_log->printf("firm %d mobilize overseer\n", shortPtr[0]);
#endif
		firm_array[*shortPtr]->assign_overseer(0);
	}
}
// ------- End of function RemoteMsg::mobilize_overseer ---------//


// ------- Begin of function RemoteMsg::mobilize_builder ---------//
void RemoteMsg::mobilize_builder()
{
	err_when( id != MSG_FIRM_MOBL_BUILDER);
	// packet structure : <firm recno>
	short *shortPtr = (short *)data_buf;

	if( validate_firm(*shortPtr) && firm_array[*shortPtr]->builder_recno )
	{
#ifdef DEBUG_LONG_LOG
		long_log->printf("firm %d mobilize builder\n", shortPtr[0]);
#endif
		firm_array[*shortPtr]->set_builder(0);
	}
}
// ------- End of function RemoteMsg::mobilize_builder ---------//


// ------ Begin of function RemoteMsg::firm_toggle_link_firm ----//
void RemoteMsg::firm_toggle_link_firm()
{
	err_when( id != MSG_FIRM_TOGGLE_LINK_FIRM);
	// packet structure : <firm recno> <link Id> <toggle Flag>
	short *shortPtr = (short *)data_buf;

	if( validate_firm(*shortPtr) )
	{
		// ###### begin Gilbert 10/11 #######//
		Firm *firmPtr = firm_array[*shortPtr];
		short linkedFirmRecno = 0;
		if( shortPtr[1] <= firmPtr->linked_firm_count
			&& (linkedFirmRecno = firmPtr->linked_firm_array[shortPtr[1]-1])
			&& validate_firm(linkedFirmRecno, 1) )
		{
#ifdef DEBUG_LONG_LOG
			long_log->printf("firm %d %s firm link %d\n", shortPtr[0], 
				shortPtr[2] ? "set":"clear", shortPtr[1]);
#endif
			firm_array[*shortPtr]->toggle_firm_link(shortPtr[1], shortPtr[2], COMMAND_REMOTE);
		}
		// ###### end Gilbert 10/11 #######//
	}
}
// ------ End of function RemoteMsg::firm_toggle_link_firm ----//


// ------ Begin of function RemoteMsg::firm_toggle_link_town ----//
void RemoteMsg::firm_toggle_link_town()
{
	err_when( id != MSG_FIRM_TOGGLE_LINK_TOWN);
	// packet structure : <firm recno> <link Id> <toggle Flag>
	short *shortPtr = (short *)data_buf;

	if( validate_firm(*shortPtr) )
	{
		// ###### begin Gilbert 10/11 ########//
		Firm *firmPtr = firm_array[*shortPtr];
		short linkedTownRecno = 0;
		if( shortPtr[1] <= firmPtr->linked_town_count
			&& (linkedTownRecno = firmPtr->linked_town_array[shortPtr[1]-1])
			&& validate_town(linkedTownRecno, 1) )
		{
#ifdef DEBUG_LONG_LOG
			long_log->printf("firm %d %s town link %d\n", shortPtr[0], 
				shortPtr[2] ? "set":"clear", shortPtr[1]);
#endif
			firmPtr->toggle_town_link(shortPtr[1], shortPtr[2], COMMAND_REMOTE);

			// update town loyalty if the firm is FIRM_CAMP
			if( firmPtr->firm_id == FIRM_CAMP )
			{
				Town *townPtr = town_array[linkedTownRecno];
				if( townPtr->nation_recno )
					townPtr->update_target_loyalty();
				else
					townPtr->update_target_resistance();
				townPtr->update_camp_link();
			}
		}
		// ###### end Gilbert 10/11 ########//
	}
}
// ------ End of function RemoteMsg::firm_toggle_link_town ----//


// ------ Begin of function RemoteMsg::firm_pull_town_people ----//
void RemoteMsg::firm_pull_town_people()
{
	err_when( id != MSG_FIRM_PULL_TOWN_PEOPLE);
	// packet structure : <firm recno> <town recno> <race Id or 0> <force Pull>
	short *shortPtr = (short *)data_buf;

	if( validate_firm(*shortPtr) && validate_town(shortPtr[1]) )
	{
#ifdef DEBUG_LONG_LOG
		long_log->printf("firm %d %s pull race %d from town %d\n", shortPtr[0], 
			shortPtr[3] ? "forcely" : "", shortPtr[2], shortPtr[1]);
#endif
		firm_array[*shortPtr]->pull_town_people(shortPtr[1], COMMAND_REMOTE, shortPtr[2], shortPtr[3]);
	}
}
// ------ End of function RemoteMsg::firm_pull_town_people ----//


// ------ Begin of function RemoteMsg::firm_set_worker_home ----//
void RemoteMsg::firm_set_worker_home()
{
	err_when( id != MSG_FIRM_SET_WORKER_HOME);
	// packet structure : <firm recno> <town recno> <workerId>
	short *shortPtr = (short *)data_buf;

	if( validate_firm(*shortPtr) && validate_town(shortPtr[1], 1) )
	{
#ifdef DEBUG_LONG_LOG
		long_log->printf("firm %d workder %d migrate to town %d\n", shortPtr[0], shortPtr[2], shortPtr[1]);
#endif
		firm_array[*shortPtr]->set_worker_home_town(shortPtr[1], COMMAND_REMOTE, shortPtr[2]);
	}
}
// ------ End of function RemoteMsg::firm_set_worker_home ----//


// ------ Begin of function RemoteMsg::firm_bribe ----//
void RemoteMsg::firm_bribe()
{
	err_when( id != MSG_FIRM_BRIBE);
	// packet structure <firm recno> <spy recno> <bribe target : worker (0=overseer)> <amount>
	short *shortPtr = (short *)data_buf;

	// ###### begin Gilbert 10/11 #######//
	Firm *firmPtr;
	if( validate_firm(*shortPtr, 1) && !spy_array.is_deleted(shortPtr[1])
		&& (firmPtr = firm_array[*shortPtr])
		&& (shortPtr[2] == 0 && firmPtr->overseer_recno ||
			shortPtr[2] >= 1 && shortPtr[2] <= firm_array[*shortPtr]->worker_count) )
	// ###### end Gilbert 10/11 #######//
	{
#ifdef DEBUG_LONG_LOG
		long_log->printf("firm %d, spy %d briber worker %d\n", shortPtr[0], shortPtr[1], shortPtr[2]);
#endif
		firm_array[*shortPtr]->spy_bribe(shortPtr[3], shortPtr[1], shortPtr[2]);
	}
}
// ------ End of function RemoteMsg::firm_bribe ----//


// ------ Begin of function RemoteMsg::firm_capture ----//
void RemoteMsg::firm_capture()
{
	err_when( id != MSG_FIRM_CAPTURE);
	// packet structure <firm recno> <nation recno>
	short *shortPtr = (short *)data_buf;

	if( validate_firm(*shortPtr, 1) )
	{
#ifdef DEBUG_LONG_LOG
		long_log->printf("firm %d, capture by nation %d\n", shortPtr[0], shortPtr[1]);
#endif
		firm_array[*shortPtr]->capture_firm(shortPtr[1]);
	}
}
// ------ End of function RemoteMsg::firm_capture ----//


// ------- Begin of function RemoteMsg::camp_patrol ---------//
void RemoteMsg::camp_patrol()
{
	err_when( id != MSG_F_CAMP_PATROL);
	if( validate_firm(*(short *)data_buf) )
	{
		FirmCamp *camp = firm_array[*(short *)data_buf]->cast_to_FirmCamp();
		if(camp)
		{
			if(camp->overseer_recno || camp->worker_count > 0)
			{
#ifdef DEBUG_LONG_LOG
				long_log->printf("camp %d patrols\n", *(short *)data_buf);
#endif
				camp->patrol();
			}
		}
		else
		{
			err_here();
		}
	}
}
// ------- End of function RemoteMsg::camp_patrol ---------//


// ------- Begin of function RemoteMsg::toggle_camp_patrol ---------//
void RemoteMsg::toggle_camp_patrol()
{
	err_when( id != MSG_F_CAMP_TOGGLE_PATROL);
	// packet structure <firm recno> <defense_flag>
	short *shortPtr = (short *)data_buf;
	if( validate_firm(*shortPtr) )
	{
		FirmCamp *camp = firm_array[*shortPtr]->cast_to_FirmCamp();
		if(camp)
		{
#ifdef DEBUG_LONG_LOG
			long_log->printf("camp %d %s patrol flag\n", shortPtr[0],
				shortPtr[1] ? "set":"clear");
#endif
			camp->defense_flag = char(shortPtr[1]);
			if( firm_array.selected_recno == *shortPtr)
				info.disp();
		}
		else
		{
			err_here();
		}
	}
}
// ------- End of function RemoteMsg::toggle_camp_patrol ---------//


// ------- Begin of function RemoteMsg::firm_reward ---------//
void RemoteMsg::firm_reward()
{
	err_when( id != MSG_FIRM_REWARD);
	// packet structure : <firm recno> <worker id>
	short *shortPtr = (short *)data_buf;

	if( validate_firm(*shortPtr) )
	{
		Firm *firmPtr = firm_array[*shortPtr];
		// ##### begin Gilbert 10/11 ########//
		if( shortPtr[1] == 0 && firmPtr->overseer_recno ||
			shortPtr[1] >= 1 && shortPtr[1] <= firmPtr->worker_count )
		{
#ifdef DEBUG_LONG_LOG
			long_log->printf("firm %d reward worker %d\n", shortPtr[0], shortPtr[1]);
#endif
			firmPtr->reward(shortPtr[1], COMMAND_REMOTE);
		}
		// ##### end Gilbert 10/11 ########//
	}
}
// ------- End of function RemoteMsg::firm_reward ---------//


// ------- Begin of function RemoteMsg::inn_hire ---------//
void RemoteMsg::inn_hire()
{
	err_when( id != MSG_F_INN_HIRE);
	// packet structure : <firm recno> <unit id> <combat level> <skill id> <skill_level> <hire cost> <spy recno> <nation no>
	short *shortPtr = (short *)data_buf;
	if( validate_firm(*shortPtr) )
	{
		FirmInn *inn = firm_array[*shortPtr]->cast_to_FirmInn();
		if(inn)
		{
#ifdef DEBUG_LONG_LOG
			long_log->printf("inn %d hire %d, by nation %d\n", shortPtr[0], shortPtr[1], shortPtr[7]);
#endif
			inn->hire_remote(shortPtr[1], shortPtr[2], shortPtr[3], shortPtr[4], shortPtr[5], shortPtr[6]);
			if( shortPtr[7] == nation_array.player_recno)
			{
				inn->put_info(INFO_REPAINT);
			}
		}
		else
		{
			err_here();
		}
	}
}
// ------- End of function RemoteMsg::inn_hire ---------//


// ------- Begin of function RemoteMsg::market_scrap ---------//
void RemoteMsg::market_scrap()
{
	err_when( id != MSG_F_MARKET_SCRAP );
	// packet structure : <firm recno> <cell no 0-3>
	short *shortPtr = (short *)data_buf;
	if( validate_firm(*shortPtr) )
	{

		FirmMarket *firmMarket = firm_array[*shortPtr]->cast_to_FirmMarket();

		if(!firmMarket)
		{
			err_here();
		}
		else
		{
#ifdef DEBUG_LONG_LOG
			long_log->printf("market %d scrap good row %d\n", shortPtr[0], shortPtr[1]);
#endif
			MarketGoods* marketGoods = firmMarket->market_goods_array + shortPtr[1];

			err_when(marketGoods->raw_id && marketGoods->product_raw_id);
			if(marketGoods->raw_id)
			{
				firmMarket->market_raw_array[marketGoods->raw_id-1] = NULL;
				marketGoods->raw_id = 0;
			}
			else if(marketGoods->product_raw_id)
			{
				firmMarket->market_product_array[marketGoods->product_raw_id-1] = NULL;
				marketGoods->product_raw_id = 0;
			}
			marketGoods->stock_qty = (float) 0;

			if( firm_array.selected_recno == *shortPtr )
				info.disp();
		}
	}
}
// ------- End of function RemoteMsg::market_scrap ---------//


// ------- Begin of function RemoteMsg::market_hire_caravan ---------//
void RemoteMsg::market_hire_caravan()
{
	err_when( id != MSG_F_MARKET_HIRE_CARA );
	// packet structure : <town recno>
	short *shortPtr = (short *)data_buf;
	if( validate_firm(*shortPtr) )
	{
		FirmMarket *market = firm_array[*shortPtr]->cast_to_FirmMarket();
		if(!market)
		{
			err_here();
		}
		else
		{
#ifdef DEBUG_LONG_LOG
			long_log->printf("market %d hire caravan\n", shortPtr[0]);
#endif
			market->hire_caravan(COMMAND_REMOTE);
		}
	}
}
// ------- End of function RemoteMsg::market_hire_caravan ---------//


// ------- Begin of function RemoteMsg::research_start ---------//
void RemoteMsg::research_start()
{
	err_when( id != MSG_F_RESEARCH_START );
	// packet structure : <firm recno> <tech Id>
	short *shortPtr = (short *)data_buf;
	if( validate_firm(*shortPtr) )
	{
		FirmResearch *research = firm_array[*shortPtr]->cast_to_FirmResearch();
		if(!research)
		{
			err_here();
		}
		else
		{
#ifdef DEBUG_LONG_LOG
			long_log->printf("tower of science %d start research tech %d\n", shortPtr[0], shortPtr[1]);
#endif
			research->start_research(shortPtr[1], COMMAND_REMOTE);
		}
	}
}
// ------- End of function RemoteMsg::research_start ---------//


// ------- Begin of function RemoteMsg::build_weapon ---------//
void RemoteMsg::build_weapon()
{
	err_when( id != MSG_F_WAR_BUILD_WEAPON );
	// packet structure : <firm recno> <unit Id> <amount>
	short *shortPtr = (short *)data_buf;
	if( validate_firm(*shortPtr) )
	{

		FirmWar *warFactory = firm_array[*shortPtr]->cast_to_FirmWar();
		if(!warFactory)
		{
			err_here();
		}
		else
		{
#ifdef DEBUG_LONG_LOG
			long_log->printf("war factory %d start building unit id %d\n", shortPtr[0], shortPtr[1]);
#endif
			warFactory->add_queue(shortPtr[1], shortPtr[2]);
		}
	}
}
// ------- End of function RemoteMsg::build_weapon ---------//


// ------- Begin of function RemoteMsg::cancel_weapon ---------//
void RemoteMsg::cancel_weapon()
{
	err_when( id != MSG_F_WAR_CANCEL_WEAPON );
	// packet structure : <firm recno> <unit Id> <amount>
	short *shortPtr = (short *)data_buf;
	if( validate_firm(*shortPtr) )
	{

		FirmWar *warFactory = firm_array[*shortPtr]->cast_to_FirmWar();
		if(!warFactory)
		{
			err_here();
		}
		else
		{
#ifdef DEBUG_LONG_LOG
			long_log->printf("war factory %d cancel building unit id %d\n", shortPtr[0], shortPtr[1]);
#endif
			warFactory->remove_queue(shortPtr[1], shortPtr[2]);
		}
	}
}
// ------- End of function RemoteMsg::cancel_weapon ---------//


// ------- Begin of function RemoteMsg::skip_build_weapon ---------//
void RemoteMsg::skip_build_weapon()
{
	err_when( id != MSG_F_WAR_SKIP_WEAPON );
	// packet structure : <firm recno>
	short *shortPtr = (short *)data_buf;
	if( validate_firm(*shortPtr) )
	{

		FirmWar *warFactory = firm_array[*shortPtr]->cast_to_FirmWar();
		if(!warFactory)
		{
			err_here();
		}
		else
		{
#ifdef DEBUG_LONG_LOG
			long_log->printf("war factory %d skip weapon building\n", shortPtr[0]);
#endif
			warFactory->cancel_build_unit();
		}
	}
}
// ------- End of function RemoteMsg::skip_build_weapon ---------//


// ------- Begin of function RemoteMsg::build_ship ---------//
void RemoteMsg::build_ship()
{
	err_when( id != MSG_F_HARBOR_BUILD_SHIP );
	// packet structure : <firm recno> <unit Id> <amount>
	short *shortPtr = (short *)data_buf;
	if( validate_firm(*shortPtr) )
	{
		FirmHarbor *harbor = firm_array[*shortPtr]->cast_to_FirmHarbor();
		if(!harbor)
		{
			err_here();
		}
		else
		{
#ifdef DEBUG_LONG_LOG
			long_log->printf("harbor %d start building unit id %d\n", shortPtr[0], shortPtr[1]);
#endif
			// harbor->build_ship(shortPtr[1], COMMAND_REMOTE);
			if( shortPtr[1] > 0)
				harbor->add_queue(shortPtr[1], shortPtr[2]);
			else if( shortPtr[1] < 0)
				harbor->remove_queue(-shortPtr[1], shortPtr[2]);
			else
			{
				err_here();
			}
		}
	}
}
// ------- End of function RemoteMsg::build_ship ---------//


// ------- Begin of function RemoteMsg::sail_ship ---------//
void RemoteMsg::sail_ship()
{
	err_when( id != MSG_F_HARBOR_SAIL_SHIP );
	// packet structure : <firm recno> <browse Recno>
	short *shortPtr = (short *)data_buf;
	if( validate_firm(*shortPtr) )
	{

		FirmHarbor *harbor = firm_array[*shortPtr]->cast_to_FirmHarbor();
		if(!harbor)
		{
			err_here();
		}
		else
		{
#ifdef DEBUG_LONG_LOG
			long_log->printf("ship %d depart from harbor %d\n", shortPtr[1], shortPtr[0]);
#endif
			harbor->sail_ship(shortPtr[1], COMMAND_REMOTE);
		}
	}
}
// ------- End of function RemoteMsg::sail_ship --------//


// ------- Begin of function RemoteMsg::skip_build_ship ---------//
void RemoteMsg::skip_build_ship()
{
	err_when( id != MSG_F_HARBOR_SKIP_SHIP );
	// packet structure : <firm recno>
	short *shortPtr = (short *)data_buf;
	if( validate_firm(*shortPtr) )
	{

		FirmHarbor *harbor = firm_array[*shortPtr]->cast_to_FirmHarbor();
		if(!harbor)
		{
			err_here();
		}
		else
		{
#ifdef DEBUG_LONG_LOG
			long_log->printf("harbr %d skip ship building\n", shortPtr[0]);
#endif
			harbor->cancel_build_unit();
		}
	}
}
// ------- End of function RemoteMsg::skip_build_ship ---------//


// ------- Begin of function RemoteMsg::factory_change_product ---------//
void RemoteMsg::factory_change_product()
{
#define DEFAULT_FACTORY_MAX_STOCK_QTY 	500
#define DEFAULT_FACTORY_MAX_RAW_STOCK_QTY 500

	err_when( id != MSG_F_FACTORY_CHG_PROD );
	// packet structure : <firm recno> <product id>
	short *shortPtr = (short *)data_buf;
	if( validate_firm(*shortPtr) )
	{
		FirmFactory *factory = firm_array[*shortPtr]->cast_to_FirmFactory();
		if(!factory)
		{
			err_here();
		}
		else
		{
#ifdef DEBUG_LONG_LOG
			long_log->printf("factory %d change product to %d\n", shortPtr[0], shortPtr[1]);
#endif
			factory->product_raw_id = shortPtr[1];
			factory->stock_qty = (float) 0;
			factory->max_stock_qty = (float) DEFAULT_FACTORY_MAX_STOCK_QTY;
			factory->raw_stock_qty = (float) 0;
			factory->max_raw_stock_qty = (float) DEFAULT_FACTORY_MAX_RAW_STOCK_QTY;
		}
	}
}
// ------- End of function RemoteMsg::factory_change_product --------//


void	RemoteMsg::base_mobilize_prayer()
{
	err_when( id != MSG_F_BASE_MOBL_PRAYER);


	err_here();
	/*
	// packet structure : <firm recno>
	short *shortPtr = (short *)data_buf;
	if( validate_firm(*shortPtr) )
	{
		FirmBase *base = firm_array[*shortPtr]->cast_to_FirmBase();
		if( !base )
		{
			err_here();
		}
		else
		{
#ifdef DEBUG_LONG_LOG
			long_log->printf("seat of power %d mobilize prayer\n", shortPtr[0]);
#endif
			base->resign_prayer();
		}
	}
	*/
}

void RemoteMsg::invoke_god()
{
	err_when( id != MSG_F_BASE_INVOKE_GOD);
	// packet structure : <firm recno>
	short *shortPtr = (short *)data_buf;
	if( validate_firm(*shortPtr) )
	{
		FirmBase *base = firm_array[*shortPtr]->cast_to_FirmBase();
		if( !base )
		{
			err_here();
		}
		else
		{
			// ##### begin Gilbert 10/11 ########//
			if( base->can_invoke() )
			{
#ifdef DEBUG_LONG_LOG
				long_log->printf("seat of power %d invoke god\n", shortPtr[0]);
#endif
				base->invoke_god();
			}
			// ##### end Gilbert 10/11 ########//
		}
	}
}


// ------- Begin of function RemoteMsg::town_recruit ---------//
void RemoteMsg::town_recruit()
{
	err_when( id != MSG_TOWN_RECRUIT);
	// packet structure : <town recno> <skill id> <race id> <amount>
	short *shortPtr = (short *)data_buf;
	if( validate_town(*shortPtr) )
	{
#ifdef DEBUG_LONG_LOG
		long_log->printf("town %d train skill %d of race %d\n", shortPtr[0], shortPtr[1], shortPtr[2]);
#endif
		if( shortPtr[2] > 0 )
		{
			if( shortPtr[1] == -1 )		// recruit unskilled unit
				town_array[*shortPtr]->recruit(shortPtr[1], shortPtr[2], COMMAND_REMOTE);
			else								// add train worker skill 
				town_array[*shortPtr]->add_queue((char) shortPtr[1], (char) shortPtr[2], shortPtr[3]);
		}
		else if( shortPtr[2] == -1)
		{
			// remove train worker skill
			town_array[*shortPtr]->remove_queue((char) shortPtr[1], shortPtr[3]);
		}
		else
		{
			err_here();
		}
		if( town_array.selected_recno == *shortPtr )
			info.update();
	}
}
// ------- End of function RemoteMsg::town_recruit ---------//


// ------- Begin of function RemoteMsg::town_skip_recruit ---------//
void RemoteMsg::town_skip_recruit()
{
	err_when( id != MSG_TOWN_SKIP_RECRUIT);
	// packet structure : <town recno>
	short *shortPtr = (short *)data_buf;
	if( validate_town(*shortPtr) )
	{
#ifdef DEBUG_LONG_LOG
		long_log->printf("town %d skip unit training\n", shortPtr[0]);
#endif
		town_array[*shortPtr]->cancel_train_unit();

		if( town_array.selected_recno == *shortPtr )
			info.disp();
	}
}
// ------- End of function RemoteMsg::town_skip_recruit ---------//


// ------- Begin of function RemoteMsg::town_migrate ---------//
void RemoteMsg::town_migrate()
{
	err_when( id != MSG_TOWN_MIGRATE);
	// packet structure : <town recno> <dest town recno> <race id> <count>
	short *shortPtr = (short *)data_buf;

	if( validate_town(*shortPtr) && validate_town(shortPtr[1]) )
	{
#ifdef DEBUG_LONG_LOG
		long_log->printf("town %d race %d migrate to town %d\n", shortPtr[0], shortPtr[2], shortPtr[1]);
#endif
		town_array[*shortPtr]->migrate_to(shortPtr[1], COMMAND_REMOTE, shortPtr[2], shortPtr[3]);
	}
}
// ------- End of function RemoteMsg::town_migrate ---------//


// ------- Begin of function RemoteMsg::town_collect_tax ---------//
void RemoteMsg::town_collect_tax()
{
	//### begin trevor 6/8 ####//
	err_when( id != MSG_TOWN_COLLECT_TAX );
	// packet structure : <town recno>
	short *shortPtr = (short *)data_buf;

	if( validate_town(*shortPtr) )
	{
#ifdef DEBUG_LONG_LOG
		long_log->printf("town %d collect tax\n", shortPtr[0]);
#endif
		town_array[*shortPtr]->collect_tax(COMMAND_REMOTE);
	}
	//### end trevor 6/8 ####//
}
// ------- End of function RemoteMsg::town_collect_tax ---------//


// ------- Begin of function RemoteMsg::town_reward ---------//
void RemoteMsg::town_reward()
{
	//### begin trevor 6/8 ####//
	err_when( id != MSG_TOWN_REWARD );
	// packet structure : <town recno>
	short *shortPtr = (short *)data_buf;

	if( validate_town(*shortPtr) )
	{
#ifdef DEBUG_LONG_LOG
		long_log->printf("town %d reward\n", shortPtr[0]);
#endif
		town_array[*shortPtr]->reward(COMMAND_REMOTE);
	}
	//### end trevor 6/8 ####//
}
// ------- End of function RemoteMsg::town_reward ---------//


// ------ Begin of function RemoteMsg::town_toggle_link_firm ----//
void RemoteMsg::town_toggle_link_firm()
{
	err_when( id != MSG_TOWN_TOGGLE_LINK_FIRM);
	// packet structure : <town recno> <link Id> <toggle Flag>
	short *shortPtr = (short *)data_buf;

	if( validate_town(*shortPtr) )
	{
#ifdef DEBUG_LONG_LOG
		long_log->printf("town %d %s firm link %d\n", shortPtr[0], 
			shortPtr[2] ? "set" : "clear", shortPtr[1]);
#endif
		Town *townPtr = town_array[*shortPtr];
		// ####### begin Gilbert 10/11 #######//
		short linkedFirmRecno = 0;
		if( shortPtr[1] <= townPtr->linked_firm_count
			&& (linkedFirmRecno = townPtr->linked_firm_array[shortPtr[1]-1])
			&& validate_firm(linkedFirmRecno, 1) )
		{
			townPtr->toggle_firm_link(shortPtr[1], shortPtr[2], COMMAND_REMOTE);

			// update loyalty if the linked firm is FIRM_BASE
			if( firm_array[linkedFirmRecno]->firm_id == FIRM_CAMP )
			{
				if(townPtr->nation_recno)
					townPtr->update_target_loyalty();
				else
					townPtr->update_target_resistance();
				townPtr->update_camp_link();
			}
		}
		// ####### end Gilbert 10/11 #######//
	}
}
// ------ End of function RemoteMsg::town_toggle_link_firm ----//


// ------ Begin of function RemoteMsg::town_toggle_link_town ----//
void RemoteMsg::town_toggle_link_town()
{
	err_when( id != MSG_TOWN_TOGGLE_LINK_TOWN);
	// packet structure : <town recno> <link Id> <toggle Flag>
	short *shortPtr = (short *)data_buf;

	if( validate_town(*shortPtr) )
	{
		// ###### begin Gilbert 10/11 #######//
		Town *townPtr = town_array[*shortPtr];
		short linkedTownRecno = 0;
		if( shortPtr[1] <= townPtr->linked_town_count
			&& (linkedTownRecno = townPtr->linked_town_array[shortPtr[1]-1])
			&& validate_town(linkedTownRecno, 1) )
		{
#ifdef DEBUG_LONG_LOG
			long_log->printf("town %d %s town link %d\n", shortPtr[0], 
				shortPtr[2] ? "set" : "clear", shortPtr[1]);
#endif
			town_array[*shortPtr]->toggle_town_link(shortPtr[1], shortPtr[2], COMMAND_REMOTE);
		}
		// ###### end Gilbert 10/11 #######//
	}
}
// ------ End of function RemoteMsg::town_toggle_link_town ----//


// ------ Begin of function RemoteMsg::town_auto_tax -------//
void RemoteMsg::town_auto_tax()
{
	err_when( id != MSG_TOWN_AUTO_TAX );
	// packet structure : <town recno> <loyalty level>
	// or <-nation recno> <loyalty level>
	short *shortPtr = (short *)data_buf;

	if( *shortPtr > 0)
	{
		if( validate_town(*shortPtr) )
		{
#ifdef DEBUG_LONG_LOG
			long_log->printf("town %d auto collect tax at loyal %d\n", shortPtr[0], shortPtr[1]);
#endif
			town_array[*shortPtr]->set_auto_collect_tax_loyalty(shortPtr[1]);
			if( town_array.selected_recno == *shortPtr )
				info.disp();
		}
	}
	else
	{
		short nationRecno = -*shortPtr;
		err_when( !nationRecno );
#ifdef DEBUG_LONG_LOG
			long_log->printf("nation %d auto collect tax at loyal %d\n", nationRecno, shortPtr[1]);
#endif
		if( !nation_array.is_deleted(nationRecno) )
		{
			nation_array[nationRecno]->set_auto_collect_tax_loyalty(shortPtr[1]);

			for( int townRecno=town_array.size() ; townRecno>0 ; townRecno-- )
			{
				Town *townPtr;
				if( !town_array.is_deleted(townRecno) && (townPtr = town_array[townRecno]) &&
					townPtr->nation_recno == nationRecno )
				{
					townPtr->set_auto_collect_tax_loyalty(shortPtr[1]);
					if( town_array.selected_recno == townRecno )
						info.disp();
				}
			}
		}
	}
}
// ------ End of function RemoteMsg::town_auto_tax -------//


// ------ Begin of function RemoteMsg::town_auto_grant -------//
void RemoteMsg::town_auto_grant()
{
	err_when( id != MSG_TOWN_AUTO_GRANT );
	// packet structure : <town recno> <loyalty level>
	// or <-nation recno> <loyalty level>
	short *shortPtr = (short *)data_buf;

	if( *shortPtr > 0 )
	{
		if( validate_town(*shortPtr) )
		{
#ifdef DEBUG_LONG_LOG
			long_log->printf("town %d auto grant at loyal %d\n", shortPtr[0], shortPtr[1]);
#endif
			town_array[*shortPtr]->set_auto_grant_loyalty(shortPtr[1]);
			if( town_array.selected_recno == *shortPtr )
				info.disp();
		}
	}
	else
	{
		short nationRecno = -*shortPtr;
		err_when( !nationRecno );
#ifdef DEBUG_LONG_LOG
			long_log->printf("nation %d auto grant at loyal %d\n", nationRecno, shortPtr[1]);
#endif
		if( !nation_array.is_deleted(nationRecno) )
		{
			nation_array[nationRecno]->set_auto_grant_loyalty(shortPtr[1]);

			for( int townRecno=town_array.size() ; townRecno>0 ; townRecno-- )
			{
				Town *townPtr;
				if( !town_array.is_deleted(townRecno) && (townPtr = town_array[townRecno]) &&
					townPtr->nation_recno == nationRecno )
				{
					townPtr->set_auto_grant_loyalty(shortPtr[1]);
					if( town_array.selected_recno == townRecno )
						info.disp();
				}
			}
		}
	}
}
// ------ End of function RemoteMsg::town_auto_grant -------//


// ------ Begin of function RemoteMsg::town_grant_independent -------//
void RemoteMsg::town_grant_independent()
{
	err_when( id != MSG_TOWN_GRANT_INDEPENDENT );
	// packet structure : <town recno> <nation recno>
	short *shortPtr = (short *)data_buf;

	if( validate_town(*shortPtr, 1) && !nation_array.is_deleted(shortPtr[1]) )
	{
		town_array[*shortPtr]->grant_to_non_own_town(shortPtr[1], COMMAND_REMOTE);
	}
}
// ------ Begin of function RemoteMsg::town_grant_independent -------//


// ------- Begin of function RemoteMsg::wall_build---------//
void RemoteMsg::wall_build()
{
	err_when( id != MSG_WALL_BUILD);
	// packet structure : <nation recno> <xLoc> <yLoc>
	short *shortPtr = (short *)data_buf;

	if( !nation_array.is_deleted(*shortPtr) )
	{
#ifdef DEBUG_LONG_LOG
		long_log->printf("nation %d build wall at (%d,%d)\n", shortPtr[0], shortPtr[1], shortPtr[2]);
#endif
		world.build_wall_tile( shortPtr[1], shortPtr[2], shortPtr[0], COMMAND_REMOTE);
	}
}
// ------- End of function RemoteMsg::wall_build---------//


// ------- Begin of function RemoteMsg::wall_destruct ---------//
void RemoteMsg::wall_destruct()
{
	err_when( id != MSG_WALL_DESTRUCT);
	// packet structure : <nation recno> <xLoc> <yLoc>
	short *shortPtr = (short *)data_buf;

	if( !nation_array.is_deleted(*shortPtr) )
	{
#ifdef DEBUG_LONG_LOG
		long_log->printf("nation %d destruct wall at (%d,%d)\n", shortPtr[0], shortPtr[1], shortPtr[2]);
#endif
		world.destruct_wall_tile( shortPtr[1], shortPtr[2], shortPtr[0], COMMAND_REMOTE);
	}
}
// ------- End of function RemoteMsg::wall_build---------//


// ------- Begin of function RemoteMsg::spy_cycle_action -------//
void RemoteMsg::spy_cycle_action()
{
	err_when( id != MSG_SPY_CYCLE_ACTION);
	// packet structure : <spy recno>
	short *shortPtr = (short *)data_buf;

	if( !spy_array.is_deleted(*shortPtr) )
	{
#ifdef DEBUG_LONG_LOG
		long_log->printf("spy %d change action\n", shortPtr[0]);
#endif
		spy_array[*shortPtr]->set_next_action_mode();
	}
}
// ------- End of function RemoteMsg::spy_cycle_action -------//

// ------- Begin of function RemoteMsg::spy_leave_town -------//
void RemoteMsg::spy_leave_town()
{
	err_when( id != MSG_SPY_LEAVE_TOWN);
	// packet structure : <spy recno>
	short *shortPtr = (short *)data_buf;

	if( !spy_array.is_deleted(*shortPtr) )
	{
#ifdef DEBUG_LONG_LOG
		long_log->printf("spy %d leave town\n", shortPtr[0]);
#endif
		//##### trevor 10/10 #####//

		if( spy_array[*shortPtr]->spy_place == SPY_TOWN )
		{
			spy_array[*shortPtr]->mobilize_town_spy();
			spy_array[*shortPtr]->notify_cloaked_nation_flag = 0;
		}

		//##### trevor 10/10 #####//
	}
}
// ------- End of function RemoteMsg::spy_leave_town -------//


// ------- Begin of function RemoteMsg::spy_leave_firm -------//
void RemoteMsg::spy_leave_firm()
{
	err_when( id != MSG_SPY_LEAVE_FIRM);
	// packet structure : <spy recno>
	short *shortPtr = (short *)data_buf;

	if( !spy_array.is_deleted(*shortPtr) )
	{
#ifdef DEBUG_LONG_LOG
		long_log->printf("spy %d leave firm\n", shortPtr[0]);
#endif
		//##### trevor 10/10 #####//

		if( spy_array[*shortPtr]->spy_place == SPY_FIRM )
		{
			spy_array[*shortPtr]->mobilize_firm_spy();
			spy_array[*shortPtr]->notify_cloaked_nation_flag = 0;
		}

		//##### trevor 10/10 #####//
	}
}
// ------- End of function RemoteMsg::spy_leave_firm -------//


// ------- Begin of function RemoteMsg::spy_capture_firm -------//
void RemoteMsg::spy_capture_firm()
{
	err_when( id != MSG_SPY_CAPTURE_FIRM);
	// packet structure : <spy recno>
	short *shortPtr = (short *)data_buf;

	if( !spy_array.is_deleted(*shortPtr) )
	{
#ifdef DEBUG_LONG_LOG
		long_log->printf("spy %d capture firm\n", shortPtr[0]);
#endif
		spy_array[*shortPtr]->capture_firm();
	}
}
// ------- End of function RemoteMsg::capture_firm -------//


// ------- Begin of function RemoteMsg::spy_drop_identity ------//
void RemoteMsg::spy_drop_identity()
{
	err_when( id != MSG_SPY_DROP_IDENTITY );

	// packet structure : <spy recno>
	short *shortPtr = (short *)data_buf;

	if( !spy_array.is_deleted(*shortPtr) )
	{
#ifdef DEBUG_LONG_LOG
		long_log->printf("spy %d drop identity\n", shortPtr[0]);
#endif
		Spy* spyPtr = spy_array[*shortPtr];
		if( spyPtr->spy_place != SPY_MOBILE ) // message can only be for mobile spy
			return;
		short sprite_recno = spyPtr->spy_place_para; // mobile spy
		spyPtr->drop_spy_identity();
		if( sprite_recno && sprite_recno == unit_array.selected_recno )
			info.disp();
	}
}
// ------- End of function RemoteMsg::spy_drop_identity ------//


// ------- Begin of function RemoteMsg::spy_reward ------//
void RemoteMsg::spy_reward()
{
	err_when( id != MSG_SPY_REWARD );

	// packet structure : <spy recno>
	short *shortPtr = (short *)data_buf;

	if( !spy_array.is_deleted(*shortPtr) )
	{
#ifdef DEBUG_LONG_LOG
		long_log->printf("spy %d reward\n", shortPtr[0]);
#endif
		spy_array[*shortPtr]->reward(COMMAND_REMOTE);
	}
}
// ------- End of function RemoteMsg::spy_reward ------//


// ------- Begin of function RemoteMsg::spy_set_exposed ------//
void RemoteMsg::spy_exposed()
{
	err_when( id != MSG_SPY_EXPOSED );

	// packet structure : <spy recno>
	short *shortPtr = (short *)data_buf;

	if( !spy_array.is_deleted(*shortPtr) )
	{
#ifdef DEBUG_LONG_LOG
		long_log->printf("spy %d set exposed\n", shortPtr[0]);
#endif
		spy_array[*shortPtr]->set_exposed(COMMAND_REMOTE);
	}
}
// ------- End of function RemoteMsg::spy_set_exposed ------//


// ------- Begin of function RemoteMsg::send_talk_msg -------//
void RemoteMsg::send_talk_msg()
{
	err_when( id != MSG_SEND_TALK_MSG);
	// packet structure : <talkMsg>

#ifdef DEBUG_LONG_LOG
	TalkMsg *talkMsg = (TalkMsg *)data_buf;
	long_log->printf("talk message from %d to %d, id %d, para1=%d, para2=%d\n",
		talkMsg->from_nation_recno, talkMsg->to_nation_recno,
		talkMsg->talk_id, talkMsg->talk_para1, talkMsg->talk_para2);
#endif
	talk_res.send_talk_msg( (TalkMsg *)data_buf, COMMAND_REMOTE);
}
// ------- End of function RemoteMsg::send_talk_msg -------//


// ------- Begin of function RemoteMsg::reply_talk_msg -------//
void RemoteMsg::reply_talk_msg()
{
	err_when( id != MSG_REPLY_TALK_MSG);
	// packet structure : <talkRecno:int> <reply type:char> <padding:char>

	//####### begin trevor 28/8 ########//
	
	int talkMsgRecno = *(int*)data_buf;

	if( !talk_res.is_talk_msg_deleted(talkMsgRecno) )
	{
#ifdef DEBUG_LONG_LOG
		long_log->printf("reply talk message %d, %d\n", talkMsgRecno, data_buf[sizeof(int)]);
#endif
		talk_res.reply_talk_msg( talkMsgRecno, data_buf[sizeof(int)], COMMAND_REMOTE);
	}

	//####### end trevor 28/8 ########//
}
// ------- End of function RemoteMsg::reply_talk_msg -------//


// ------- Begin of function RemoteMsg::nation_contact -------//
void RemoteMsg::nation_contact()
{
	err_when( id != MSG_NATION_CONTACT);
	// packet structure : <player nation> <explored nation>
	short *shortPtr = (short *)data_buf;

	err_when( *shortPtr != remote.nation_processing );
	if( !nation_array.is_deleted(*shortPtr) && !nation_array.is_deleted(shortPtr[1]) )
	{
		//####### begin trevor 30/8 #######//
#ifdef DEBUG_LONG_LOG
		long_log->printf("nation %d discover nation %d\n", shortPtr[0], shortPtr[1]);
#endif
		nation_array[shortPtr[0]]->establish_contact(shortPtr[1]);
		//####### end trevor 30/8 #######//
	}
}
// ------- End of function RemoteMsg::nation_contact -------//


// ------- Begin of function RemoteMsg::nation_set_should_attack -------//
void RemoteMsg::nation_set_should_attack()
{
	err_when( id != MSG_NATION_SET_SHOULD_ATTACK );
	// packet structure : <player nation> <target nation> <new value>
	short *shortPtr = (short *)data_buf;

	err_when( *shortPtr != remote.nation_processing );
	if( !nation_array.is_deleted(*shortPtr) && !nation_array.is_deleted(shortPtr[1]) )
	{
#ifdef DEBUG_LONG_LOG
		long_log->printf("nation %d %s instruct attack nation %d\n", shortPtr[0],
			shortPtr[2] ? "set":"clear", shortPtr[1]);
#endif
		nation_array[shortPtr[0]]->set_relation_should_attack(shortPtr[1], (char)shortPtr[2], COMMAND_REMOTE);
	}
}
// ------- End of function RemoteMsg::nation_set_should_attack -------//


// ------- Begin of function RemoteMsg::caravan_selected -------//
void RemoteMsg::caravan_selected()
{
	err_when( id != MSG_U_CARA_SELECTED );
	// packet structure : <sprite_recno>
	short *shortPtr = (short *)data_buf;
}
// ------- End of function RemoteMsg::caravan_selected -------//


// ------- Begin of function RemoteMsg::ship_selected -------//
void RemoteMsg::ship_selected()
{
	err_when( id != MSG_U_SHIP_SELECTED );
	// packet structure : <sprite_recno>
	short *shortPtr = (short *)data_buf;
}
// ------- End of function RemoteMsg::ship_selected -------//

//##### begin trevor 30/9 #######//

//------- Begin of function RemoteMsg::chat -------//
//
// Packet structure : <to nation recno> <from nation recno> <char[CHAT_STR_LEN+1]>
//
void RemoteMsg::chat()
{
	short *shortPtr = (short *)data_buf;

	int toNationRecno   = shortPtr[0];
   int fromNationRecno = shortPtr[1];

#ifdef DEBUG_LONG_LOG
	long_log->printf("nation %d send chat message to %d\n", 
		shortPtr[1],	shortPtr[0]);
#endif

	if( toNationRecno == nation_array.player_recno ||
		(toNationRecno == 0 && fromNationRecno != nation_array.player_recno) ||
		(toNationRecno == -1 && !nation_array.is_deleted(fromNationRecno) &&
			nation_array[fromNationRecno]->is_allied_with_player) )
	{
		news_array.chat_msg( fromNationRecno, (char*)(shortPtr+2) );
	}
}
//------- End of function RemoteMsg::chat -------//

//##### end trevor 30/9 #######//


//------- Begin of function RemoteMsg::compare_remote_object -------//
void	RemoteMsg::compare_remote_object()
{
	err_when( id < MSG_COMPARE_NATION || id > MSG_COMPARE_TALK );

	// ###### patch begin Gilbert 20/1 #######//
	if( crc_store.compare_remote(id, data_buf) )
	{
		if( sys.debug_session )
			err.run( _("Multiplayer Object Sync Error") );
	}
	// ###### patch end Gilbert 20/1 #######//
}
//------- End of function RemoteMsg::compare_remote_object -------//


//------- Begin of function RemoteMsg::compare_remote_crc -------//
void RemoteMsg::compare_remote_crc()
{
	err_when( id != MSG_COMPARE_CRC );

	if( (remote.sync_test_level & 2) && (remote.sync_test_level >= 0)
		&& crc_store.compare_frame(data_buf) )
	{
		remote.sync_test_level = ~2;	// signal error encountered
	}
}
//------- End of function RemoteMsg::compare_remote_crc -------//


//------- Begin of function RemoteMsg::unit_add_way_point -------//
void RemoteMsg::unit_add_way_point()
{
	//### begin alex 16/10 ###//
	err_when( id != MSG_UNIT_ADD_WAY_POINT);
	short* shortPtr = (short*) data_buf;
	validate_selected_unit_array(shortPtr+3, shortPtr[2]);

	if( shortPtr[2] > 0)
	{
#ifdef DEBUG_LONG_LOG
		long_log->printf("add way point at (%d,%d)\n", shortPtr[0], shortPtr[1]);
		for(int i = 0; i < shortPtr[2]; ++i)
		{
			long_log->printf("%d,", shortPtr[3+i]);
			if( i % 20 == 19)
				long_log->printf("\n");
		}
		long_log->printf("\n");
#endif
		unit_array.add_way_point(shortPtr[0], shortPtr[1], shortPtr+3, shortPtr[2], COMMAND_REMOTE); // 1-remote action
	}
	//#### end alex 16/10 ####//
}
//------- End of function RemoteMsg::unit_add_way_point -------//


// ------- Begin of function RemoteMsg::god_cast -------//
void RemoteMsg::god_cast()
{
	err_when( id != MSG_U_GOD_CAST );
	// packet structure : <unit recno> <loc x> <loc y> <power type>
	short *shortPtr = (short *)data_buf;

	short unitCount =1;
	validate_selected_unit_array(shortPtr, unitCount);

	if(unitCount > 0)
	{
		Unit *unitPtr = unit_array[*shortPtr];
		err_when(unit_res[unitPtr->unit_id]->unit_class != UNIT_CLASS_GOD);
#ifdef DEBUG_LONG_LOG
		long_log->printf("god %d cast power %d at(%d,%d)\n", shortPtr[0], shortPtr[3],
			shortPtr[1], shortPtr[2]);
#endif
		unitPtr->go_cast_power(shortPtr[1], shortPtr[2], (char) shortPtr[3], COMMAND_REMOTE);
	}
}
// ------- End of function RemoteMsg::god_cast -------//


// ------- Begin of function RemoteMsg::player_quit ---------//
void RemoteMsg::player_quit()
{
	// to let other player know a player quit voluntarily, not by error condition
	err_when( id != MSG_PLAYER_QUIT );
	// packet structure : <nation recno> <retire flag>

	short *shortPtr = (short *)data_buf;
	if( shortPtr[1])
		news_array.multi_retire(*shortPtr);
	else
		news_array.multi_quit_game(*shortPtr);
}
// ------- End of function RemoteMsg::player_quit ---------//


// ------- Begin of function RemoteMsg::firm_request_builder ---------//
void RemoteMsg::firm_request_builder()
{
	err_when( id != MSG_FIRM_REQ_BUILDER);
	// packet structure : <firm recno>
	short *shortPtr = (short *)data_buf;
	if( validate_firm(*shortPtr) )
	{
		firm_array[*shortPtr]->send_idle_builder_here(COMMAND_REMOTE);
	}
}
// ------- End of function RemoteMsg::firm_request_builder ---------//


// ------- Begin of function RemoteMsg::switch_restock ---------//
void RemoteMsg::market_switch_restock()
{
	err_when(id != MSG_F_MARKET_RESTOCK);
	// packet structure : <firm recno>
	short *shortPtr = (short *)data_buf;
	if( validate_firm(*shortPtr) )
	{
		FirmMarket *firmMarket = firm_array[*shortPtr]->cast_to_FirmMarket();

		if(!firmMarket)
			err_here();
		else
			firmMarket->switch_restock();
	}
}
// ------- End of function RemoteMsg::switch_restock ---------//
