/*
 * Seven Kingdoms: Ancient Adversaries
 *
 * Copyright 1997,1998 Enlight Software Ltd.
 * Copyright 2010,2011,2013,2015 Jesse Allen
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

// Filename    : multiplayer.h
// Description : Multiplayer game support.

#ifndef __MULTIPLAYER_H
#define __MULTIPLAYER_H

#include <MPTYPES.h>
#include <player_desc.h>
#include <ODYNARRB.h>
#include <stdint.h>
#include <enet/enet.h>

#define MP_SERVICE_PROVIDER_NAME_LEN 64
#define MP_SESSION_NAME_LEN 64
#define MP_PASSWORD_LEN 32
#define MP_RECV_BUFFER_SIZE 0x2000
#define MP_GAME_LIST_SIZE 10
#define MP_LADDER_LIST_SIZE 6

enum ProtocolType
{
	None = 0,
	IPX = 1,
	TCPIP = 2,
	Modem = 4,
	Serial = 8
};

struct SessionDesc
{
	char session_name[MP_FRIENDLY_NAME_LEN+1];
	char password[MP_FRIENDLY_NAME_LEN+1];
	uint32_t id;
	ENetAddress address;

	SessionDesc();
	SessionDesc(const SessionDesc &);
	SessionDesc& operator= (const SessionDesc &);
	SessionDesc(const char *name, const char *pass, ENetAddress *address);

	char *name_str() { return session_name; };
	uint32_t session_id() { return id; }
};


class MultiPlayer
{
private:

	int               init_flag;
	int               lobbied_flag;

	ProtocolType      supported_protocols;
	DynArrayB         current_sessions;
	SessionDesc       joined_session;

	uint32_t          my_player_id;
	PlayerDesc        *my_player;

	int               host_flag;
	int               allowing_connections;
	uint32_t          packet_mode;
	int               max_players;

	PlayerDesc        *player_pool[MAX_NATION];
	PlayerDesc        *pending_pool[MAX_NATION];

	char *            recv_buf;

	ENetHost          *host;

	ENetAddress       lan_broadcast_address;
	ENetAddress       remote_session_provider_address;

	int use_remote_session_provider;
	int update_available;

public:

	MultiPlayer();
	~MultiPlayer();

	void   init(ProtocolType);
	void   deinit();
	bool   is_initialized() const { return init_flag != 0; }

	// ------- functions on lobby -------- //
	int    init_lobbied(int maxPlayers, char * cmdLine);
	int    is_lobbied(); // return 0=not lobbied, 1=auto create, 2=auto join, 4=selectable
	char * get_lobbied_name(); // return 0 if not available

	// ------- functions on network protocols ------ //
	void   poll_supported_protocols(); // can be called before init
	bool   is_protocol_supported(ProtocolType);
	int    is_update_available();
	void   game_starting();
	void   disable_new_connections();

	// ------- functions on session --------//
	int    set_remote_session_provider(const char *server);
	int    poll_sessions();
	void   sort_sessions(int sortType);
	int    create_session(char *sessionName, char *password, char *playerName, int maxPlayers);
	int    join_session(SessionDesc *session, char *playerName);
	int    close_session();
	SessionDesc* get_session(int i);
	SessionDesc *get_current_session();

	// -------- functions on player management -------//
	int         add_player(uint32_t playerId, char *name, ENetAddress *address, char contact);
	int         auth_player(uint32_t playerId, char *name, char *password);
	int         set_my_player_id(uint32_t playerId);
	void        delete_player(uint32_t playerId);
	void        poll_players();
	PlayerDesc* get_player(int i);
	PlayerDesc* search_player(uint32_t playerId);
	int         is_player_connecting(uint32_t playerId);
	int         get_player_count();
	uint32_t    get_my_player_id() const { return my_player_id; }

	// ------- functions on message passing ------//
	int    send(uint32_t to, void * data, uint32_t msg_size);
	char  *receive(uint32_t *from, uint32_t *size, int *sysMsgCount=0);

private:
	int open_port(uint16_t port, int fallback);
	void close_port();

	uint32_t get_avail_player_id();
	int add_pending_player(PlayerDesc *player);
	PlayerDesc* get_pending_player(uint32_t playerId);
	PlayerDesc* get_pending_player(ENetAddress *address);
	ENetPeer *get_peer(uint32_t playerId);
	ENetPeer *get_peer(ENetAddress *address);
};

extern MultiPlayer mp_obj;

#endif // __MULTIPLAYER_H

