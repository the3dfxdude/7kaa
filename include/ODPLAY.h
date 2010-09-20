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

// Filename    : ODPLAY.H
// Description : Header file of MultiPlayerDP (DirectPlay)
// Owner       : Gilbert

#ifndef __ODPLAY_H
#define __ODPLAY_H

#ifndef IMAGICMP

//#include <dplay.h>
//#include <dplobby.h>
#include <ODYNARRB.h>
#include <stdint.h>

//extern GUID GAME_GUID;
//extern HANDLE PLAYER_MESSAGE_HANDLE;

#define MP_SERVICE_PROVIDER_NAME_LEN 64
#define MP_SESSION_NAME_LEN 64
#define MP_PASSWORD_LEN 32
#define MP_FRIENDLY_NAME_LEN 20
#define MP_FORMAL_NAME_LEN 64
#define MP_RECV_BUFFER_SIZE 0x2000

struct DPServiceProvider
{
	GUID guid;
	char description[MP_SERVICE_PROVIDER_NAME_LEN+1];

	char *name_str() { return description; }
	GUID service_id() { return guid; }
};

struct DPSessionDesc
{
	char session_name[MP_SESSION_NAME_LEN+1];
	char pass_word[MP_SESSION_NAME_LEN+1];
	uint32_t id;

	DPSessionDesc();
	DPSessionDesc(const DPSessionDesc &);
	DPSessionDesc& operator= (const DPSessionDesc &);
	void after_copy();
	DPSessionDesc *before_use();

	char *name_str() { return session_name; };
	uint32_t session_id() { return id; }
};


struct DPPlayer
{
	uint32_t player_id;
	char	friendly_name[MP_FRIENDLY_NAME_LEN+1];
	char	formal_name[MP_FORMAL_NAME_LEN+1];
	char	connecting;		// initially set to 1, 
	                     // clear after DPSYS_DESTROYPLAYERORGROUP received from DirectPlay

	uint32_t pid()			{ return player_id; }
	char *friendly_name_str() { return friendly_name; }
	char *formal_name_str() { return formal_name; }
};

class MultiPlayerDP
{
public:
	int						init_flag;
	int						lobbied_flag;
	DynArrayB				service_providers;		// array of DPServiceProvider
	DynArrayB				current_sessions;			// array of DPSessionDesc
	//LPDIRECTPLAY			direct_play1;
	//LPDIRECTPLAY3A		direct_play3;
	//LPDIRECTPLAYLOBBY2A	direct_play_lobby;
	DPSessionDesc			joined_session;
	//DPLCONNECTION *		connection_string;		// only when lobbied

	uint32_t				my_player_id;
	int						host_flag;
	DynArrayB				player_pool;

	char *					recv_buffer;
	uint32_t				recv_buffer_size;

public:
	MultiPlayerDP();
	~MultiPlayerDP();
	void pre_init();
	void init(GUID serviceProviderGuid);
	void deinit();

	// ------- functions on DirectPlayLobby -------- //
	void	init_lobbied(int maxPlayers, char *cmdLine);
	int	is_lobbied();		// return 0=not lobbied, 1=auto create, 2=auto join, 4=selectable
	char *get_lobbied_name();			// return 0 if not available
	int	send_lobby(LPVOID lpData, DWORD dataSize);
	char *receive_lobby(LPDWORD recvLen);

	// ------- functions on service provider ------ //
	void	poll_service_providers();								// can be called before init
	DPServiceProvider *get_service_provider(int i);		// can be called before init

	// ------- functions on session --------//
	int	poll_sessions();
	void	sort_sessions(int sortType);
	DPSessionDesc *get_session(int i);
	int	create_session(char *sessionName, int maxPlayers);
	int	join_session(DPSessionDesc* sessionDesc);
	int	join_session(int currentSessionIndex );
	void	close_session();
	void	disable_join_session();		// so that new player cannot join

	// -------- functions on player management -------//
	int	create_player(char *friendlyName, char *formalName,
		LPVOID lpData=NULL, DWORD dataSize=0, DWORD flags=0);
	void	destroy_player( DPID playerId );
	void	poll_players();
	DPPlayer *get_player(int i);
	DPPlayer *search_player(DPID player_id);
	DPPlayer *search_player(char *name);
	int	is_host(DPID playerId);
	int	am_I_host();
	int	is_player_connecting(DPID playerId);

	// ------- functions on data management ------//
	// remote data (public) : each player has one data to the public
	int	update_public_data(DPID, LPVOID, DWORD );
	int	retrieve_public_data(DPID, LPVOID, LPDWORD);
	// local data (private) : each player keeps a data on each other player
	int	update_private_data(DPID, LPVOID, DWORD);
	int	retrieve_private_data(DPID, LPVOID, LPDWORD);

	// ------- functions on message passing ------//
	int	send(DPID toId, LPVOID lpData, DWORD dataSize);
	void	begin_stream(DPID toID);
	int	send_stream(DPID toId, LPVOID lpData, DWORD dataSize);
	void	end_stream(DPID toID);
	int	get_msg_count();
	char *receive(LPDPID from, LPDPID to, LPDWORD recvLen, int *sysMsgCount=0);

	void	before_receive()		{} // dummy function to compatible with IMMPLAY, call before receive
	void	after_send()			{}	// dummy function to compatible with IMMPLAY, call after send

protected:
	void	handle_system_msg(LPVOID, DWORD );
	void	handle_lobby_system_msg(LPVOID, DWORD);
};

extern MultiPlayerDP mp_dp;

#include <MPTYPES.h>

#endif	// IMAGICMP
#endif	// __ODPLAY_H