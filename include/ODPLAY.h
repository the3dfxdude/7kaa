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

#include <ODYNARRB.h>
#include <stdint.h>

#define MP_SERVICE_PROVIDER_NAME_LEN 64
#define MP_SESSION_NAME_LEN 64
#define MP_PASSWORD_LEN 32
#define MP_FRIENDLY_NAME_LEN 20
#define MP_FORMAL_NAME_LEN 64
#define MP_RECV_BUFFER_SIZE 0x2000

enum ProtocolType
{
	None = 0,
	IPX = 1,
	TCPIP = 2,
	Modem = 4,
	Serial = 8
};

struct DPSessionDesc
{
	char session_name[MP_SESSION_NAME_LEN+1];
	char pass_word[MP_SESSION_NAME_LEN+1];
	uint32_t id;

	DPSessionDesc();
	DPSessionDesc(const DPSessionDesc &);
	DPSessionDesc& operator= (const DPSessionDesc &);

	char *name_str() { return session_name; };
	uint32_t session_id() { return id; }
};


struct DPPlayer
{
	uint32_t player_id;
	char	friendly_name[MP_FRIENDLY_NAME_LEN+1];
	char	formal_name[MP_FORMAL_NAME_LEN+1];
	char	connecting;		// initially set to 1, clear after player disconnected

	uint32_t pid()			{ return player_id; }
	char *friendly_name_str() { return friendly_name; }
	char *formal_name_str() { return formal_name; }
};

class MultiPlayerDP
{
private:
	int						init_flag;
	int						lobbied_flag;
	ProtocolType			supported_protocols;
	DynArrayB				current_sessions;
	DPSessionDesc			joined_session;

	uint32_t				my_player_id;
	int						host_flag;
	DynArrayB				player_pool;

	char *					recv_buffer;
	uint32_t				recv_buffer_size;

public:
	MultiPlayerDP();
	~MultiPlayerDP();

	void init(ProtocolType);
	void deinit();
	bool is_initialized() const { return init_flag != 0; }

	// ------- functions on DirectPlayLobby -------- //
	void	init_lobbied(int maxPlayers, char *cmdLine);
	int	is_lobbied();		// return 0=not lobbied, 1=auto create, 2=auto join, 4=selectable
	char *get_lobbied_name();			// return 0 if not available

	// ------- functions on service provider ------ //
	void   poll_supported_protocols(); // can be called before init
	bool   is_protocol_supported(ProtocolType);

	// ------- functions on session --------//
	int	poll_sessions();
	void	sort_sessions(int sortType);
	DPSessionDesc *get_session(int i);
	int	create_session(char *sessionName, int maxPlayers);
	int	join_session(int currentSessionIndex );
	void	close_session();
	void	disable_join_session();		// so that new player cannot join

	// -------- functions on player management -------//
	int	create_player(char *friendlyName, char *formalName);
	void	poll_players();
	DPPlayer *get_player(int i);
	DPPlayer *search_player(uint32_t player_id);
	int	is_player_connecting(uint32_t playerId);
	int       get_player_count() const { return player_pool.size(); }
	uint32_t  get_my_player_id() const { return my_player_id; }

	// ------- functions on message passing ------//
	int	send(uint32_t toId, void * lpData, uint32_t dataSize);
	int	send_stream(uint32_t toId, void * lpData, uint32_t dataSize);
	char *receive(uint32_t * from, uint32_t * to, uint32_t * recvLen, int *sysMsgCount=0);
};

extern MultiPlayerDP mp_dp;

#include <MPTYPES.h>

#endif	// __ODPLAY_H

