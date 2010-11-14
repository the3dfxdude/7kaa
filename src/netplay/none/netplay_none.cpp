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

// Filename    : netplay_none.cpp
// Description : MultiPlayerNone, stub multiplayer class
// Onwer       : Gilbert

#include <netplay.h>
#include <ALL.h>
#include <string.h>
#include <OVGALOCK.h>
#include <OBLOB.h>
#include <stdint.h>
#include <dbglog.h>

DBGLOG_DEFAULT_CHANNEL(NetPlay);

NoneSessionDesc::NoneSessionDesc()
{
	MSG("[NoneSessionDesc::NoneSessionDesc] calling unimplemented method\n");
}

NoneSessionDesc::NoneSessionDesc(const NoneSessionDesc &NoneSessionDesc) //: NoneSessionDesc2(NoneSessionDesc)
{
	ERR("[NoneSessionDesc::NoneSessionDesc(const NoneSessionDesc &)] calling unimplemented method\n");
}

NoneSessionDesc& NoneSessionDesc::operator= (const NoneSessionDesc &src)
{
	ERR("[NoneSessionDesc::operator=] calling unimplemented method\n");
	return *this;
}

// to start a multiplayer game, first check if it is called from a
// lobbied (MultiPlayerNone::is_lobbied)

// if it is a lobbied, call init_lobbied before create_player

// if not, call poll_service_provider; display them and let
// user to select, call init and pass the guid of the selected
// service; create_session or poll_sessions+join_session;
// finally create_player.

MultiPlayerNone::MultiPlayerNone() :
	current_sessions(sizeof(NoneSessionDesc), 10 ), player_pool(sizeof(NonePlayer), 8 ),
	recv_buffer(NULL)
{
	MSG("[MultiPlayerNone::MultiPlayerNone] calling unimplemented method\n");
}

MultiPlayerNone::~MultiPlayerNone()
{
	MSG("[MultiPlayerNone::~MultiPlayerNone] calling unimplemented method\n");
}

void MultiPlayerNone::init(ProtocolType protocol_type)
{
	ERR("[MultiPlayerNone::init()] calling unimplemented method\n");
}

void MultiPlayerNone::deinit()
{
	ERR("[MultiPlayerNone::deinit] calling unimplemented method\n");	
}

void MultiPlayerNone::init_lobbied(int maxPlayers, char *)
{
	ERR("[MultiPlayerNone::init_lobbied] calling unimplemented method\n");
}

// return 0=not lobbied, 1=auto create, 2=auto join, 4=selectable
int MultiPlayerNone::is_lobbied()
{
	return lobbied_flag;
}

char *MultiPlayerNone::get_lobbied_name()
{
	ERR("[MultiPlayerNone::get_lobbied_name] calling unimplemented method\n");
	return NULL;
}

void MultiPlayerNone::poll_supported_protocols()
{
	supported_protocols = TCPIP;
}

bool MultiPlayerNone::is_protocol_supported(ProtocolType protocol)
{
	return (protocol & supported_protocols) != 0;
}

int MultiPlayerNone::poll_sessions()
{
	ERR("[MultiPlayerNone::poll_sessions] calling unimplemented method\n");
	return FALSE;
}

// return a session description
//
// <int> i			i-th session (i start from 1)
// return pointer to a session, NULL if no more
NoneSessionDesc *MultiPlayerNone::get_session(int i)
{
	ERR("[MultiPlayerNone::get_session] calling unimplemented method\n");
	return NULL;
}

// create a new session
//
// <char *> sessionName      arbitary name to identify a session, input from user
// <int>    maxPlayers       maximum no. of players in a session
//
// return TRUE if success
int MultiPlayerNone::create_session(char *sessionName, char *playerName, int maxPlayers)
{
	ERR("[MultiPlayerNone::create_session] calling unimplemented method\n");
	return FALSE;
}

// join a session, by passing the index passed into get_session()
// note : do not call poll_sessions between get_session and join_session
//
// <int> currentSessionIndex       the index passed into get_session()
//
// currentSessionIndex start from 1
int MultiPlayerNone::join_session(int currentSessionIndex, char *playerName)
{
	ERR("[MultiPlayerNone::join_session] calling unimplemented method\n");
	return FALSE;
}

void MultiPlayerNone::close_session()
{
	ERR("[MultiPlayerNone::close_session] calling unimplemented method\n");
}

void MultiPlayerNone::disable_join_session()
{
	ERR("[MultiPlayerNone::disable_join_session] calling unimplemented method\n");
}

int MultiPlayerNone::add_player(char *name, uint32_t id)
{
        return 0;
}

void MultiPlayerNone::delete_player(uint32_t id)
{
}

void MultiPlayerNone::poll_players()
{
	ERR("[MultiPlayerNone::poll_players] calling unimplemented method\n");
}

NonePlayer *MultiPlayerNone::get_player(int i)
{
	if( i <= 0 || i > player_pool.size() )
		return NULL;
	return (NonePlayer *)player_pool.get(i);
}

NonePlayer *MultiPlayerNone::search_player(uint32_t playerId)
{
	NonePlayer *player;
	int i = 0;
	while( (player = get_player(++i)) != NULL )
		if( player->player_id == playerId )
			return player;
	return NULL;
}

// determine whether a player is lost
//
// MultiPlayerNone::received must be called (or remote.poll_msg) , 
// so if a player is really lost, the system message from 
// directPlay is received
//
int MultiPlayerNone::is_player_connecting(uint32_t playerId)
{
	for( int p = 1; p <= player_pool.size(); ++p)
	{
		NonePlayer * nonePlayer = (NonePlayer *) player_pool.get(p);
		if( nonePlayer->player_id == playerId )
		{
			return nonePlayer->connecting;
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
int MultiPlayerNone::send(uint32_t toId, void * lpData, uint32_t dataSize)
{
	ERR("[MultiPlayerNone::send] calling unimplemented method\n");
	return FALSE;
}

// send message
//
// pass BROADCAST_PID as toId to all players
//
// return TRUE on success
//
int MultiPlayerNone::send_stream(uint32_t toId, void * lpData, uint32_t dataSize)
{
	ERR("[MultiPlayerNone::send_stream] calling unimplemented method\n");
	return FALSE;
}

// return NULL if fails
// sysMsgCount records how many system messages have been handled
// notice : *sysMsgCount may be != 0, but return NULL
//
char *MultiPlayerNone::receive(uint32_t * from, uint32_t * to, uint32_t * dSize, int *sysMsgCount)
{
	ERR("[MultiPlayerNone::receive] calling unimplemented method\n");
	return NULL;
}

// return NULL if fails
// sysMsgCount records how many system messages have been handled
// notice : *sysMsgCount may be != 0, but return NULL
//
char *MultiPlayerNone::receive_stream(uint32_t * from, uint32_t * to, uint32_t * dSize, int *sysMsgCount)
{
	ERR("[MultiPlayerNone::receive_stream] calling unimplemented method\n");
	return NULL;
}

/*
static int sort_session_id(const void *a, const void *b)
{
	return memcmp( &((NoneSessionDesc *)a)->guidInstance, &((NoneSessionDesc *)b)->guidInstance,
		sizeof(GUID) );
}
*/

static int sort_session_name(const void *a, const void *b)
{
	return strcmp( ((NoneSessionDesc *)a)->name_str(), ((NoneSessionDesc *)b)->name_str() );
}

// sort current_sessions
// <int> sortType, 1=sort by GUID, 2=sort by session name
void MultiPlayerNone::sort_sessions(int sortType )
{
	ERR("[MultiPlayerNone::sort_sessions] calling partially implemented method\n");

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

