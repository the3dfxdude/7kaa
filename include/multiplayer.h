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
#include <session_desc.h>
#include <player_desc.h>
#include <ODYNARRB.h>
#include <stdint.h>
#include <enet/enet.h>
#include <OMISC.h>


#define MP_SERVICE_PROVIDER_NAME_LEN 64
#define MP_SESSION_NAME_LEN 64
#define MP_PASSWORD_LEN 32
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

enum
{
	MPMSG_USER_SESSION_STATUS = 0x1f960001,
	MPMSG_REQ_LOGIN_ID,
	MPMSG_LOGIN_ID,
	MPMSG_REQ_SESSION_ID,
	MPMSG_SESSION_ID,
	MPMSG_POLL_SESSIONS,
	MPMSG_SESSION,
	MPMSG_REQ_SESSION_ADDR,
	MPMSG_SESSION_ADDR,
	MPMSG_PING,
	MPMSG_REQ_HOST_NAT_PUNCH,
	MPMSG_HOST_NAT_PUNCH,
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

struct MpMsgUserSessionStatus {
	uint32_t msg_id;
	guuid_t login_id;
	guuid_t session_id;
	uint32_t player_id;
	uint32_t flags;
	char session_name[MP_FRIENDLY_NAME_LEN];
};
struct MpMsgReqLoginId {
	uint32_t msg_id;
	char name[MP_FRIENDLY_NAME_LEN];
};
struct MpMsgLoginId {
	uint32_t msg_id;
	guuid_t login_id;
};
struct MpMsgReqSessionId {
	uint32_t msg_id;
	guuid_t login_id;
	char session_name[MP_FRIENDLY_NAME_LEN];
	char session_password[MP_FRIENDLY_NAME_LEN];
};
struct MpMsgSessionId {
	uint32_t msg_id;
	guuid_t session_id;
};
struct MpMsgPollSessions {
	uint32_t msg_id;
	guuid_t login_id;
};
struct MpMsgSession {
	uint32_t msg_id;
	guuid_t session_id;
	uint32_t flags;
	char session_name[MP_FRIENDLY_NAME_LEN];
};
struct MpMsgReqSessionAddr {
	uint32_t msg_id;
	guuid_t login_id;
	guuid_t session_id;
	char session_password[MP_FRIENDLY_NAME_LEN];
};
struct MpMsgSessionAddr {
	uint32_t msg_id;
	guuid_t session_id;
	uint32_t host;
	uint16_t port;
	uint16_t reserved0;
};
struct MpMsgPing {
	uint32_t msg_id;
};
struct MpMsgReqHostNatPunch {
	uint32_t msg_id;
	guuid_t login_id;
	guuid_t session_id;
	char session_password[MP_FRIENDLY_NAME_LEN];
};
struct MpMsgHostNatPunch {
	uint32_t msg_id;
	uint32_t host;
	uint16_t port;
	uint16_t reserved0;
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
	unsigned          recv_buffer_size;

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
	SessionDesc* get_session(ENetAddress *address);
	SessionDesc* get_session(guuid_t id);
	SessionDesc* get_current_session();

	// -------- functions on player management -------//
	int         add_player(uint32_t playerId, char *name, ENetAddress *address, char contact);
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
	void do_host_nat_punch(MpMsgHostNatPunch *in);

	void update_player_pool();
	uint32_t get_avail_player_id();
	int add_pending_player(PlayerDesc *player);
	PlayerDesc* yank_pending_player(uint32_t playerId);
	PlayerDesc* yank_pending_player(ENetAddress *address);
	ENetPeer *get_peer(uint32_t playerId);
	ENetPeer *get_peer(ENetAddress *address);

	int retrieve_packet(ENetEvent *event, uint32_t *size);
};

extern MultiPlayer mp_obj;

#endif // __MULTIPLAYER_H

