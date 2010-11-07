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

#ifndef __netplay_sdlnet_h__
#define __netplay_sdlnet_h__

#include <ODYNARRB.h>
#include <stdint.h>
#include <SDL/SDL_net.h>

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

struct SDLSessionDesc
{
	char session_name[MP_SESSION_NAME_LEN+1];
	char pass_word[MP_SESSION_NAME_LEN+1];
	uint32_t id;

	SDLSessionDesc();
	SDLSessionDesc(const SDLSessionDesc &);
	SDLSessionDesc& operator= (const SDLSessionDesc &);

	char *name_str() { return session_name; };
	uint32_t session_id() { return id; }
};


struct SDLPlayer
{
	uint32_t player_id;
	char	friendly_name[MP_FRIENDLY_NAME_LEN+1];
	char	formal_name[MP_FORMAL_NAME_LEN+1];
	char	connecting;		// initially set to 1, clear after player disconnected

	uint32_t pid()			{ return player_id; }
	char *friendly_name_str() { return friendly_name; }
	char *formal_name_str() { return formal_name; }
};

class MultiPlayerSDL
{
private:

	int               init_flag;
	int               lobbied_flag;
	ProtocolType      supported_protocols;
	DynArrayB         current_sessions;
	SDLSessionDesc    joined_session;

	uint32_t          my_player_id;
	int               host_flag;
	DynArrayB         player_pool;

	char *            recv_buffer;
	uint32_t          recv_buffer_size;

	IPaddress         ip_address;
	TCPsocket         data_sock;
	TCPsocket         listen_sock; // used by server
	SDLNet_SocketSet  sock_set;

public:

	MultiPlayerSDL();
	~MultiPlayerSDL();

	void   init(ProtocolType);
	void   deinit();
	bool   is_initialized() const { return init_flag != 0; }

	// ------- functions on lobby -------- //
	void   init_lobbied(int maxPlayers, char * cmdLine);
	int    is_lobbied(); // return 0=not lobbied, 1=auto create, 2=auto join, 4=selectable
	char * get_lobbied_name(); // return 0 if not available

	// ------- functions on network protocols ------ //
	void   poll_supported_protocols(); // can be called before init
	bool   is_protocol_supported(ProtocolType);

	// ------- functions on session --------//
	int    poll_sessions();
	void   sort_sessions(int sortType);
	int    create_session(char * sessionName, int maxPlayers);
	int    join_session(int i);
	void   close_session();
	void   disable_join_session();
	void   accept_connections();
	SDLSessionDesc * get_session(int i);

	// -------- functions on player management -------//
	int         create_player(char * friendlyName, char * formalName);
	void        poll_players();
	SDLPlayer * get_player(int i);
	SDLPlayer * search_player(uint32_t playerId);
	int         is_player_connecting(uint32_t playerId);
	int         get_player_count() const { return player_pool.size(); }
	uint32_t    get_my_player_id() const { return my_player_id; }

	// ------- functions on message passing ------//
	int    send(uint32_t toId, void * data, uint32_t size);
	int    send_stream(uint32_t toId, void * data, uint32_t size);
	char * receive(uint32_t * from, uint32_t * to, uint32_t * size, int *sysMsgCount=0);

private:

	char * receive_raw(uint32_t * from, uint32_t * to, uint32_t * size);
	void   process_sys_msg(uint32_t size, char * data);

	// sys message processing
	void send_session_info_request();
	void process_session_info_request();
	void send_session_info_reply();
	void process_session_info_reply(char * data);
};

extern MultiPlayerSDL mp_sdl;

#include <MPTYPES.h>

#endif

