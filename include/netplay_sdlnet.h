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
#include <SDL_net.h>
#include <network.h>

#define MP_SERVICE_PROVIDER_NAME_LEN 64
#define MP_SESSION_NAME_LEN 64
#define MP_PASSWORD_LEN 32
#define MP_FRIENDLY_NAME_LEN 64
#define MP_RECV_BUFFER_SIZE 0x2000
#define MP_GAME_LIST_SIZE 10
#define MP_LADDER_LIST_SIZE 6
// NOTE: MP_PLAYER_NAME_LEN must match PLAYER_NAME_LEN in OCONFIG.h
#define MP_PLAYER_NAME_LEN 20

enum
{
	MPMSG_GAME_BEACON = 0x1f4a0001,
	MPMSG_REQ_GAME_LIST,
	MPMSG_GAME_LIST,
	MPMSG_VERSION_ACK,
	MPMSG_VERSION_NAK,
	MPMSG_CONNECT,
	MPMSG_CONNECT_ACK,
	MPMSG_REQ_LADDER,
	MPMSG_LADDER,
};

#pragma pack(1)
struct MsgHeader
{
	uint32_t msg_id;
};

struct MsgGameBeacon
{
	uint32_t msg_id;
	char name[MP_SESSION_NAME_LEN];
	char password;
};

struct MsgRequestGameList
{
	uint32_t msg_id;
	uint32_t ack;
};

struct remote_game
{
	char name[MP_SESSION_NAME_LEN];
	char password;
	uint32_t host;
	uint16_t port;
};

struct MsgGameList
{
	uint32_t msg_id;
	uint32_t page;
	uint32_t total_pages;
	struct remote_game list[MP_GAME_LIST_SIZE];
};

struct MsgVersionAck
{
	uint32_t msg_id;
};

struct MsgVersionNak
{
	uint32_t msg_id;
	uint32_t major;
	uint32_t medium;
	uint32_t minor;
};

struct MsgConnect
{
	uint32_t msg_id;
	uint32_t player_id;
	char password[MP_SESSION_NAME_LEN];
};

struct MsgConnectAck
{
	uint32_t msg_id;
};

struct MsgRequestLadder
{
	uint32_t msg_id;
};

struct ladder_entry
{
	char name[MP_PLAYER_NAME_LEN];
	uint16_t wins;
	uint16_t losses;
	int32_t score;
};

struct MsgLadder
{
	uint32_t msg_id;
	struct ladder_entry list[MP_LADDER_LIST_SIZE];
};
#pragma pack()

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
	char password[MP_SESSION_NAME_LEN+1];
	uint32_t id;
	struct inet_address address;

	SDLSessionDesc();
	SDLSessionDesc(const SDLSessionDesc &);
	SDLSessionDesc& operator= (const SDLSessionDesc &);

	char *name_str() { return session_name; };
	uint32_t session_id() { return id; }
};


struct SDLPlayer
{
	uint32_t id;
	char     name[MP_FRIENDLY_NAME_LEN+1];
	char	connecting;		// initially set to 1, clear after player disconnected
	TCPsocket socket;
	struct inet_address address;

	uint32_t pid() { return id; }
	char     *friendly_name_str() { return name; }
	int get_address(struct inet_address *addr) { addr->host = address.host; addr->port = address.port; return sizeof(struct inet_address); }
};

class MultiPlayerSDL
{
private:

	int               init_flag;
	int               lobbied_flag;
	ProtocolType      supported_protocols;
	DynArrayB         current_sessions;
	SDLSessionDesc    joined_session;
	NetworkSDLNet     *network;

	uint32_t          my_player_id;
	char              my_name[MP_FRIENDLY_NAME_LEN+1];

	int               host_flag;
	int               allowing_connections;
	int               max_players;

	SDLPlayer         player_pool[MAX_NATION];

	char *            recv_buf;

	TCPsocket         host_sock; // used by client to talk to game host
	TCPsocket         listen_sock; // used by server
	int               peer_sock; // peer-to-peer communication
	int               game_sock;
	SDLNet_SocketSet  sock_set;

	struct inet_address lan_broadcast_address;
	struct inet_address remote_session_provider_address;

	int use_remote_session_provider;
	int update_available;

public:

	MultiPlayerSDL();
	~MultiPlayerSDL();

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

	// ------- functions on session --------//
	int    set_remote_session_provider(const char *server);
	int    poll_sessions();
	void   sort_sessions(int sortType);
	int    create_session(char *sessionName, char *password, char *playerName, int maxPlayers);
	int    join_session(int i, char *playerName);
	int    udp_join_session(char *password);
	void   close_session();
	void   disable_join_session();
	void   accept_connections();
	int    udp_accept_connections(uint32_t *who, struct inet_address *address);
	SDLSessionDesc * get_session(int i);

	// -------- functions on player management -------//
	int         add_player(char *name, uint32_t id);
	void        set_my_player_id(uint32_t id);
	void        set_player_name(uint32_t id, char *name);
	void        delete_player(uint32_t id);
	void        poll_players();
	SDLPlayer * get_player(int i);
	SDLPlayer * search_player(uint32_t playerId);
	int         is_player_connecting(uint32_t playerId);
	int         get_player_count();
	uint32_t    get_my_player_id() const { return my_player_id; }
	void        set_peer_address(uint32_t who, struct inet_address *address);

	// ------- functions on message passing ------//
	int    send(uint32_t to, void * data, uint32_t msg_size);
	int    send_stream(uint32_t to, void * data, uint32_t msg_size);
	char * receive(uint32_t * from, uint32_t * to, uint32_t * size, int *sysMsgCount=0);
	char * receive_stream(uint32_t * from, uint32_t * to, uint32_t * size, int *sysMsgCount=0);

	int show_leader_board();

private:
	int send_nonseq_msg(int sock, char *msg, int msg_size, struct inet_address *to);
	int send_system_msg(int sock, char *msg, int msg_size, struct inet_address *to);

	int create_player(TCPsocket socket);
	int check_duplicates(struct inet_address *address);
	void msg_game_beacon(MsgGameBeacon *m, struct inet_address *addr);
	int msg_game_list(MsgGameList *m, int last_ack, struct inet_address *addr);
	void msg_version_nak(MsgVersionNak *p, struct inet_address *addr);
};

extern MultiPlayerSDL mp_sdl;

#include <MPTYPES.h>

#endif

