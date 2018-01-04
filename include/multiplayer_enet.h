/*
 * Seven Kingdoms: Ancient Adversaries
 *
 * Copyright 1997,1998 Enlight Software Ltd.
 * Copyright 2010,2011,2013,2015 Jesse Allen
 * Copyright 2018 Richard Dijk <microvirus.multiplying@gmail.com>
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

// Filename    : multiplayer_enet.h
// Description : Multiplayer game support.

#ifndef __MULTIPLAYER_ENET_H
#define __MULTIPLAYER_ENET_H

#include <MPTYPES.h>
#include <session_desc.h>
#include <player_desc.h>
#include <ODYNARRB.h>
#include <stdint.h>
#include <enet/enet.h>


enum ProtocolType
{
	None = 0,
	IPX = 1,
	TCPIP = 2,
	Modem = 4,
	Serial = 8
};

enum
{
	MP_POLL_NO_UPDATE,
	MP_POLL_UPDATE,
	MP_POLL_LOGIN_PENDING,
	MP_POLL_LOGIN_FAILED,
	MP_POLL_NO_SOCKET,
	MP_POLL_NO_SESSION,
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
	PlayerDesc        my_player;

	PlayerDesc        *player_pool[MAX_NATION];
	PlayerDesc        *pending_pool[MAX_NATION];

	char *            recv_buf;

	ENetHost          *host;
	uint32_t          packet_mode;

	ENetSocket        session_monitor;
	ENetAddress       service_provider;
	guuid_t            service_login_id;

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
	int    set_service_provider(const char *host);
	int    poll_sessions();
	void   sort_sessions(int sortType);
	int    create_session(char *sessionName, char *password, int maxPlayers);
	int    join_session(SessionDesc *session);
	int    close_session();
	SessionDesc* get_session(int i);
	SessionDesc* get_session(guuid_t id);
	SessionDesc* get_current_session();

	// -------- functions on player management -------//
	int         add_player(uint32_t playerId, char *name, const NetworkAddress &address, char contact);
	int         auth_player(uint32_t playerId, char *name, char *password);
	void        create_my_player(char *playerName);
	int         set_my_player_id(uint32_t playerId);
	void        delete_player(uint32_t playerId);
	int         poll_players();
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
	int connect_host();

	ENetSocket create_socket(uint16_t port);
	void destroy_socket(ENetSocket socket);

	void send_user_session_status(ENetAddress *a);
	void send_req_login_id();
	void send_poll_sessions();
	void send_req_session_id();
	void send_req_session_addr();
	void send_ping(ENetSocket s, ENetAddress *a);
	void send_req_host_nat_punch();
	void send_service_ping();
	void do_host_nat_punch(ENetAddress *address);

	SessionDesc* get_session_from_address(const ENetAddress &address);

	void update_player_pool();
	uint32_t get_avail_player_id();
	int add_pending_player(PlayerDesc *player);
	PlayerDesc* yank_pending_player(uint32_t playerId);
	PlayerDesc* yank_pending_player(const ENetAddress &address);
	ENetPeer *get_peer(uint32_t playerId);
	ENetPeer *get_peer(ENetAddress *address);
};

extern MultiPlayer mp_obj;

#endif // __MULTIPLAYER_ENET_H

