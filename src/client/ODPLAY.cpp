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

#include <ODPLAY.h>
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


// ------- begin of function MultiPlayerDP::MultiPlayerDP -------//
MultiPlayerDP::MultiPlayerDP() :
	current_sessions(sizeof(DPSessionDesc), 10 ), player_pool(sizeof(DPPlayer), 8 ),
	recv_buffer(new char[MP_RECV_BUFFER_SIZE])
{
	ERR("[MultiPlayerDP::MultiPlayerDP] calling unimplemented method\n");

	/*
	init_flag = 0;
	recv_buffer_size = MP_RECV_BUFFER_SIZE;
	host_flag = 0;
	lobbied_flag = 0;
	*/
}
// ------- end of function MultiPlayerDP::MultiPlayerDP -------//


// ------- begin of function MultiPlayerDP::~MultiPlayerDP -------//
MultiPlayerDP::~MultiPlayerDP()
{
	ERR("[MultiPlayerDP::~MultiPlayerDP] calling unimplemented method\n");
	/*
	deinit();
	delete[] recv_buffer;
	*/
}
// ------- end of function MultiPlayerDP::~MultiPlayerDP -------//

// ------- begin of function MultiPlayerDP::init -------//
void MultiPlayerDP::init(ProtocolType protocol_type)
{
	ERR("[MultiPlayerDP::init()] calling unimplemented method\n");
}
// ------- end of function MultiPlayerDP::init -------//


// ------- begin of function MultiPlayerDP::deinit -------//
void MultiPlayerDP::deinit()
{
	ERR("[MultiPlayerDP::deinit] calling unimplemented method\n");	
}
// ------- end of function MultiPlayerDP::deinit -------//


// ----- begin of function MultiPlayerDP::init_lobbied ------//
void MultiPlayerDP::init_lobbied(int maxPlayers, char *)
{
	ERR("[MultiPlayerDP::init_lobbied] calling unimplemented method\n");
}
// ----- end of function MultiPlayerDP::init_lobbied ------//

// ----- begin of function MultiPlayerDP::is_lobbied -----//
// return 0=not lobbied, 1=auto create, 2=auto join, 4=selectable
int MultiPlayerDP::is_lobbied()
{
	return lobbied_flag;
}
// ----- end of function MultiPlayerDP::is_lobbied -----//

// ----- begin of function MultiPlayerDP::get_lobbied_name -----//
char *MultiPlayerDP::get_lobbied_name()
{
	ERR("[MultiPlayerDP::get_lobbied_name] calling unimplemented method\n");
	return NULL;
}
// ----- end of function MultiPlayerDP::get_lobbied_name -----//

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
// ----- end of function MultiPlayerDP::poll_sessions ------//


// ----- begin of function MultiPlayerDP::get_session ------//
// return a session description
//
// <int> i			i-th session (i start from 1)
// return pointer to a session, NULL if no more
DPSessionDesc *MultiPlayerDP::get_session(int i)
{
	ERR("[MultiPlayerDP::get_session] calling unimplemented method\n");
	return NULL;
	/*
	if( i <= 0 || i > current_sessions.size() )
		return NULL;
	return ((DPSessionDesc *) current_sessions.get(i))->before_use();
	*/
}
// ----- end of function MultiPlayerDP::get_session ------//


// ----- begin of function MultiPlayerDP::create_session ----//
//
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
// ----- end of function MultiPlayerDP::create_session ----//

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
// ------ end of function MultiPlayerDP::join_session ------//

// ------ begin of function MultiPlayerDP::close_session ------//
void MultiPlayerDP::close_session()
{
	ERR("[MultiPlayerDP::close_session] calling unimplemented method\n");
}
// ------ end of function MultiPlayerDP::close_session ------//

// ------ begin of function MultiPlayerDP::disable_join_session ------//
void MultiPlayerDP::disable_join_session()
{
	ERR("[MultiPlayerDP::disable_join_session] calling unimplemented method\n");
}
// ------ end of function MultiPlayerDP::disable_join_session ------//

// ------ begin of function MultiPlayerDP::create_player ------//
// create a local player
//
// <char *> friendlyName          short name of the player, best to be one word only
// [char *] formalName            long name of the player, take friendlyName if NULL (default: NULL)
// [void *] lpData, [DWORD] dataSize    pointer and size of any data sent the remote (default: NULL, 0)
// [DWORD] flags                  not use reserved (default:0)
//
// return TRUE if success
//
int MultiPlayerDP::create_player(char *friendlyName, char *formalName)
{
	ERR("[MultiPlayerDP::create_player] calling unimplemented method\n");
	return FALSE;
}
// ------ end of function MultiPlayerDP::create_player -----//

void MultiPlayerDP::poll_players()
{
	ERR("[MultiPlayerDP::poll_players] calling unimplemented method\n");
}
// -------- end of function MultiPlayerDP::poll_players ------//


// -------- begin of function MultiPlayerDP::get_player -----//
//
// return the i-th player in the player_pool
//
DPPlayer *MultiPlayerDP::get_player(int i)
{
	if( i <= 0 || i > player_pool.size() )
		return NULL;
	return (DPPlayer *)player_pool.get(i);
}
// -------- end of function MultiPlayerDP::get_player -----//


// -------- begin of function MultiPlayerDP::search_player -----//
//
// search player by playerID
//
DPPlayer *MultiPlayerDP::search_player(uint32_t playerId)
{
	DPPlayer *player;
	int i = 0;
	while( (player = get_player(++i)) != NULL )
		if( player->player_id == playerId )
			return player;
	return NULL;
}

//
// search player by formal name, case insensitive
//
/*
DPPlayer *MultiPlayerDP::search_player(char *name)
{
	DPPlayer *player;
	int i = 0;
	while( (player = get_player(++i)) != NULL )
		if( strnicmp(player->formal_name, name, MP_FORMAL_NAME_LEN)== 0)
			return player;
	return NULL;
}
*/
// -------- end of function MultiPlayerDP::get_player -----//


// ------- begin of function MultiPlayerDP::is_host --------//
/*
int MultiPlayerDP::is_host(DPID playerID)
{
	err_here();		// not supported
	return 0;
}
*/
// ------- end of function MultiPlayerDP::is_host --------//


// ------- begin of function MultiPlayerDP::am_I_host --------//
/*
int MultiPlayerDP::am_I_host()
{
	return host_flag;
}
*/
// ------- end of function MultiPlayerDP::am_I_host --------//


// ----- begin of function MultiPlayerDP::is_player_connecting ----//
//
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
// ----- end of function MultiPlayerDP::is_player_connecting ----//

// --------- begin of function MultiPlayerDP::send ---------//
// send message
//
// must not call it between IDirectDrawSurface2::Lock and IDirectDrawSurface2::Unlock,
// or between IDirectDrawSurface2::GetDC and IDirectDrawSurface2::ReleaseDC
// pass DPID_ALLPLAYERS as toId to all players
//
// return TRUE on success
//
int MultiPlayerDP::send(uint32_t toId, void * lpData, uint32_t dataSize)
{
	ERR("[MultiPlayerDP::send] calling unimplemented method\n");
	return FALSE;
}
// --------- end of function MultiPlayerDP::send ---------//

// --------- begin of function MultiPlayerDP::send_stream ---------//
// send message
//
// must not call it between IDirectDrawSurface2::Lock and IDirectDrawSurface2::Unlock,
// or between IDirectDrawSurface2::GetDC and IDirectDrawSurface2::ReleaseDC
// pass DPID_ALLPLAYERS as toId to all players
//
// return TRUE on success
//
int MultiPlayerDP::send_stream(uint32_t toId, void * lpData, uint32_t dataSize)
{
	ERR("[MultiPlayerDP::send_stream] calling unimplemented method\n");
	return FALSE;
}
// --------- end of function MultiPlayerDP::send_stream ---------//

// ------- begin of function MultiPlayerDP::receive ------//
// return NULL if fails
// sysMsgCount records how many system messages have been handled
// notice : *sysMsgCount may be != 0, but return NULL
//
char *MultiPlayerDP::receive(uint32_t * from, uint32_t * to, uint32_t * dSize, int *sysMsgCount)
{
	ERR("[MultiPlayerDP::receive] calling unimplemented method\n");
	return NULL;
}
// ------- end of function MultiPlayerDP::receive ------//

// --------- begin of function MultiPlayerDP::send_lobby ---------//
// send message
//
// must not call it between IDirectDrawSurface2::Lock and IDirectDrawSurface2::Unlock,
// or between IDirectDrawSurface2::GetDC and IDirectDrawSurface2::ReleaseDC
//
// return TRUE on success
//
/*
int MultiPlayerDP::send_lobby(LPVOID lpData, DWORD dataSize)
{
	err_when(!init_flag);
	VgaFrontLock vgaLock;
	return !direct_play_lobby->SendLobbyMessage(0, 0, lpData, dataSize);
}
*/
// --------- end of function MultiPlayerDP::send_lobby ---------//

// ------- begin of function MultiPlayerDP::receive_lobby ------//
// return NULL if fails
/*
char *MultiPlayerDP::receive_lobby(LPDWORD dSize)
{
	err_when(!init_flag);
	DWORD dataSize, msgFlag;
	int retryFlag;
	HRESULT hr;

	VgaFrontLock vgaLock;
	do
	{
		retryFlag = 0;
		dataSize = recv_buffer_size;
		hr=direct_play_lobby->ReceiveLobbyMessage(0,0, &msgFlag, recv_buffer, &dataSize);
		switch(hr)
		{
		case 0:
			if(msgFlag == DPLAD_SYSTEM)
			{
				handle_lobby_system_msg(recv_buffer, dataSize);
				retryFlag = 1;
			}
			else
			{
				*dSize = dataSize;
			}
			break;
		case DPERR_BUFFERTOOSMALL:		// assume now dataSize > recv_buffer_size
			delete[] recv_buffer;
			recv_buffer_size = dataSize + 0x400;
			recv_buffer = new char[recv_buffer_size];
			retryFlag = 1;		// direct_play3->receive may not return the same message, so keep retrying
			break;
		default:
			return NULL;
		}
		
	} while (retryFlag);
	return recv_buffer;
}
*/
// ------- end of function MultiPlayerDP::receive_lobby ------//

// ------ Begin of function MultiPlayerDP::sort_sessions -------//
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
// ------ End of function MultiPlayerDP::sort_sessions -------//

