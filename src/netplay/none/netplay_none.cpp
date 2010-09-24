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

// Filename    : ODPLAY.CPP
// Description : MultiPlayerDP, multiplayer class using directPlay
// Onwer       : Gilbert

#include <netplay.h>
#include <ALL.h>
#include <string.h>
#include <OVGALOCK.h>
#include <OBLOB.h>
#include <stdint.h>
#include <dbglog.h>

DBGLOG_DEFAULT_CHANNEL(DPlay);

DPSessionDesc::DPSessionDesc()
{
	ERR("[DPSessionDesc::DPSessionDesc] calling unimplemented method\n");
}

DPSessionDesc::DPSessionDesc(const DPSessionDesc &dpSessionDesc) //: DPSESSIONDESC2(dpSessionDesc)
{
	ERR("[DPSessionDesc::DPSessionDesc(const DPSessionDesc &)] calling unimplemented method\n");
}

DPSessionDesc& DPSessionDesc::operator= (const DPSessionDesc &src)
{
	ERR("[DPSessionDesc::operator=] calling unimplemented method\n");
	return *this;
}

// to start a multiplayer game, first check if it is called from a
// lobbied (MultiPlayerDP::is_lobbied)

// if it is a lobbied, call init_lobbied before create_player

// if not, call poll_service_provider; display them and let
// user to select, call init and pass the guid of the selected
// service; create_session or poll_sessions+join_session;
// finally create_player.

MultiPlayerDP::MultiPlayerDP() :
	current_sessions(sizeof(DPSessionDesc), 10 ), player_pool(sizeof(DPPlayer), 8 ),
	recv_buffer(NULL)
{
	ERR("[MultiPlayerDP::MultiPlayerDP] calling unimplemented method\n");
}

MultiPlayerDP::~MultiPlayerDP()
{
	ERR("[MultiPlayerDP::~MultiPlayerDP] calling unimplemented method\n");
}

void MultiPlayerDP::init(ProtocolType protocol_type)
{
	ERR("[MultiPlayerDP::init()] calling unimplemented method\n");
}

void MultiPlayerDP::deinit()
{
	ERR("[MultiPlayerDP::deinit] calling unimplemented method\n");	
}

void MultiPlayerDP::init_lobbied(int maxPlayers, char *)
{
	ERR("[MultiPlayerDP::init_lobbied] calling unimplemented method\n");
}

// return 0=not lobbied, 1=auto create, 2=auto join, 4=selectable
int MultiPlayerDP::is_lobbied()
{
	return lobbied_flag;
}

char *MultiPlayerDP::get_lobbied_name()
{
	ERR("[MultiPlayerDP::get_lobbied_name] calling unimplemented method\n");
	return NULL;
}

void MultiPlayerDP::poll_supported_protocols()
{
	supported_protocols = TCPIP;
}

bool MultiPlayerDP::is_protocol_supported(ProtocolType protocol)
{
	return (protocol & supported_protocols) != 0;
}

int MultiPlayerDP::poll_sessions()
{
	ERR("[MultiPlayerDP::poll_sessions] calling unimplemented method\n");
	return FALSE;
}

// return a session description
//
// <int> i			i-th session (i start from 1)
// return pointer to a session, NULL if no more
DPSessionDesc *MultiPlayerDP::get_session(int i)
{
	ERR("[MultiPlayerDP::get_session] calling unimplemented method\n");
	return NULL;
}

// create a new session
//
// <char *> sessionName      arbitary name to identify a session, input from user
// <int>    maxPlayers       maximum no. of players in a session
//
// return TRUE if success
int MultiPlayerDP::create_session(char *sessionName, int maxPlayers)
{
	ERR("[MultiPlayerDP::create_session] calling unimplemented method\n");
	return FALSE;
}

// join a session, by passing the index passed into get_session()
// note : do not call poll_sessions between get_session and join_session
//
// <int> currentSessionIndex       the index passed into get_session()
//
// currentSessionIndex start from 1
int MultiPlayerDP::join_session(int currentSessionIndex)
{
	ERR("[MultiPlayerDP::join_session] calling unimplemented method\n");
	return FALSE;
}

void MultiPlayerDP::close_session()
{
	ERR("[MultiPlayerDP::close_session] calling unimplemented method\n");
}

void MultiPlayerDP::disable_join_session()
{
	ERR("[MultiPlayerDP::disable_join_session] calling unimplemented method\n");
}

// create a local player
//
// <char *> friendlyName          short name of the player, best to be one word only
// [char *] formalName            long name of the player, take friendlyName if NULL (default: NULL)
// return TRUE if success
//
int MultiPlayerDP::create_player(char *friendlyName, char *formalName)
{
	ERR("[MultiPlayerDP::create_player] calling unimplemented method\n");
	return FALSE;
}

void MultiPlayerDP::poll_players()
{
	ERR("[MultiPlayerDP::poll_players] calling unimplemented method\n");
}

DPPlayer *MultiPlayerDP::get_player(int i)
{
	if( i <= 0 || i > player_pool.size() )
		return NULL;
	return (DPPlayer *)player_pool.get(i);
}

DPPlayer *MultiPlayerDP::search_player(uint32_t playerId)
{
	DPPlayer *player;
	int i = 0;
	while( (player = get_player(++i)) != NULL )
		if( player->player_id == playerId )
			return player;
	return NULL;
}

// determine whether a player is lost
//
// MultiPlayerDP::received must be called (or remote.poll_msg) , 
// so if a player is really lost, the system message from 
// directPlay is received
//
int MultiPlayerDP::is_player_connecting(uint32_t playerId)
{
	for( int p = 1; p <= player_pool.size(); ++p)
	{
		DPPlayer *dpPlayer = (DPPlayer *) player_pool.get(p);
		if( dpPlayer->player_id == playerId )
		{
			return dpPlayer->connecting;
		}
	}
	return 0;
}

// send message
//
// pass BROADCAST_PID as toId to all players
//
// return TRUE on success
//
int MultiPlayerDP::send(uint32_t toId, void * lpData, uint32_t dataSize)
{
	ERR("[MultiPlayerDP::send] calling unimplemented method\n");
	return FALSE;
}

// send message
//
// pass BROADCAST_PID as toId to all players
//
// return TRUE on success
//
int MultiPlayerDP::send_stream(uint32_t toId, void * lpData, uint32_t dataSize)
{
	ERR("[MultiPlayerDP::send_stream] calling unimplemented method\n");
	return FALSE;
}

// return NULL if fails
// sysMsgCount records how many system messages have been handled
// notice : *sysMsgCount may be != 0, but return NULL
//
char *MultiPlayerDP::receive(uint32_t * from, uint32_t * to, uint32_t * dSize, int *sysMsgCount)
{
	ERR("[MultiPlayerDP::receive] calling unimplemented method\n");
	return NULL;
}

/*
static int sort_session_id(const void *a, const void *b)
{
	return memcmp( &((DPSessionDesc *)a)->guidInstance, &((DPSessionDesc *)b)->guidInstance,
		sizeof(GUID) );
}
*/

static int sort_session_name(const void *a, const void *b)
{
	return strcmp( ((DPSessionDesc *)a)->name_str(), ((DPSessionDesc *)b)->name_str() );
}

// sort current_sessions
// <int> sortType, 1=sort by GUID, 2=sort by session name
void MultiPlayerDP::sort_sessions(int sortType )
{
	ERR("[MultiPlayerDP::sort_sessions] calling partially implemented method\n");

	// BUGHERE : quick_sort is a DynArray function but current_sessions is DynArrayB
	switch(sortType)
	{
	case 1:
		//current_sessions.quick_sort(sort_session_id);
		break;
	case 2:
		current_sessions.quick_sort(sort_session_name);
		break;
	default:
		err_here();
	}
}

