/*
 * Seven Kingdoms: Ancient Adversaries
 *
 * Copyright 1997,1998 Enlight Software Ltd.
 * Copyright 2010,2011 Jesse Allen
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

// Filename    : netplay_sdlnet.cpp
// Description : MultiPlayerSDL, SDL_net based multiplayer class
// Onwer       : Gilbert

#include <netplay.h>
#include <ALL.h>
#include <string.h>
#include <OVGALOCK.h>
#include <OBLOB.h>
#include <stdint.h>
#include <dbglog.h>

DBGLOG_DEFAULT_CHANNEL(MultiPlayer);

#define MP_UDP_MAX_PACKET_SIZE 500

const Uint16 GAME_PORT = 1234;
const Uint16 UDP_GAME_PORT = 19255;

enum
{
	MPMSG_GAME_BEACON = 0x1f4a0001,
	MPMSG_REQ_GAME_LIST,
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
};
#pragma pack()


SDLSessionDesc::SDLSessionDesc()
{
	id = 0;
	session_name[0] = '\0';
	pass_word[0] = '\0';
	memset(&address, 0, sizeof(IPaddress));
}

SDLSessionDesc::SDLSessionDesc(const SDLSessionDesc &src)
{
	id = src.id;
	strcpy(session_name, src.session_name);
	strcpy(pass_word, src.pass_word);
	memcpy(&address, &src.address, sizeof(IPaddress));
}

SDLSessionDesc& SDLSessionDesc::operator= (const SDLSessionDesc &src)
{
	id = src.id;
	strcpy(session_name, src.session_name);
	strcpy(pass_word, src.pass_word);
	memcpy(&address, &src.address, sizeof(IPaddress));
	return *this;
}

// to start a multiplayer game, first check if it is called from a
// lobbied (MultiPlayerSDL::is_lobbied)

// if it is a lobbied, call init_lobbied

// if not, call poll_service_provider; display them and let
// user to select, call init and pass the guid of the selected
// service; create_session or poll_sessions+join_session;

MultiPlayerSDL::MultiPlayerSDL() :
	current_sessions(sizeof(SDLSessionDesc), 10 )
{
	init_flag = 0;
	lobbied_flag = 0;
	supported_protocols = TCPIP;
	my_player_id = 0;
	host_flag = 0;
	allowing_connections = 0;
	host_sock = NULL;
	listen_sock = NULL;
	peer_sock = NULL;
	game_sock = NULL;
	sock_set = NULL;
	recv_buf = NULL;
}

MultiPlayerSDL::~MultiPlayerSDL()
{
	deinit();
}

void MultiPlayerSDL::init(ProtocolType protocol_type)
{
	init_flag = 0;
	lobbied_flag = 0;
	my_player_id = 0;
	host_flag = 0;
	max_players = 0;

	if (!is_protocol_supported(protocol_type)) {
		ERR("[MultiPlayerSDL::init] trying to init unsupported protocol\n");
		return;
	}

	// TODO: add SDL initialization if required

	if (SDLNet_Init() == -1) {
		ERR("[MultiPlayerSDL::init] unable to init SDL_net: %s\n", SDLNet_GetError());
		return;
	}

	sock_set = SDLNet_AllocSocketSet(MAX_NATION);
	if (!sock_set) {
		ERR("[MultiPlayerSDL::init] SDLNet_AllocSocketSet: %s\n", SDLNet_GetError());
		SDLNet_Quit();
		return;
	}

	SDLNet_ResolveHost(&lan_broadcast_address, "255.255.255.255", UDP_GAME_PORT);

	for (int i = 0; i < MAX_NATION; i++) {
		player_pool[i].id = 0;
		player_pool[i].connecting = 0;
		player_pool[i].socket = NULL;
		memset(&player_pool[i].address, 0, sizeof(player_pool[i].address));
	}

	recv_buf = new char[MP_RECV_BUFFER_SIZE];

	init_flag = 1;
}

void MultiPlayerSDL::deinit()
{
	int i;

	if (host_flag) {
		// disconnect all clients
		for (i = 0; i < max_players; i++) {
			delete_player(i+1);
		}
		if (listen_sock) {
			SDLNet_TCP_Close(listen_sock);
			listen_sock = NULL;
		}

		host_flag = 0;
		allowing_connections = 0;
	} else if (host_sock) {
		SDLNet_TCP_DelSocket(sock_set, host_sock);
		SDLNet_TCP_Close(host_sock);
		host_sock = NULL;
	}
	if (recv_buf) {
		delete [] recv_buf;
		recv_buf = NULL;
	}

	if (peer_sock) {
		SDLNet_UDP_Close(peer_sock);
		peer_sock = NULL;
	}

	if (game_sock) {
		SDLNet_UDP_Close(game_sock);
		game_sock = NULL;
	}

	SDLNet_FreeSocketSet(sock_set);
	SDLNet_Quit();

	current_sessions.zap();
	init_flag = 0;
	lobbied_flag = 0;
	my_player_id = 0;
	sock_set = NULL;
}

// init_lobbied
// Reads the command line and sets lobby mode if the command line is correct.
// Returns non-zero on success.
int MultiPlayerSDL::init_lobbied(int maxPlayers, char *cmdLine)
{
	MSG("[MultiPlayerSDL::init_lobbied] %d, %s\n", maxPlayers, cmdLine);
	if (cmdLine) {
		SDLSessionDesc *session = new SDLSessionDesc();

		strcpy(session->session_name, "Lobbied Game");
		session->pass_word[0] = 0;
		if (SDLNet_ResolveHost(&session->address, cmdLine, GAME_PORT) == -1) {
			MSG("failed to resolve hostname: %s\n", SDLNet_GetError());
			delete session;
			return 0;
		}

		current_sessions.linkin(session);

		lobbied_flag = 2;
	} else {
		// hosting doesn't work yet
		lobbied_flag = 1;
	}
	return 1;
}

// return 0=not lobbied, 1=auto create, 2=auto join, 4=selectable
int MultiPlayerSDL::is_lobbied()
{
	return lobbied_flag;
}

// get_lobbied_name() is used to get the player's name when the game is
// launched to the lobby -- this probably won't ever be needed.
char *MultiPlayerSDL::get_lobbied_name()
{
	return NULL;
}

void MultiPlayerSDL::poll_supported_protocols()
{
}

bool MultiPlayerSDL::is_protocol_supported(ProtocolType protocol)
{
	return (protocol & supported_protocols) != 0;
}

int MultiPlayerSDL::check_duplicates(IPaddress *address)
{
	int i;
	for (i = 0; i < current_sessions.size(); i++)
	{
		SDLSessionDesc *desc;

		desc = (SDLSessionDesc *)current_sessions.get(i+1);
		if (!desc)
			return 0;
		if (desc->address.host == address->host &&
		    desc->address.port == address->port)
			return 1;
	}
	return 0;
}

int MultiPlayerSDL::set_remote_session_provider(const char *server)
{
	use_remote_session_provider = !SDLNet_ResolveHost(&lan_broadcast_address, server, UDP_GAME_PORT);
	return use_remote_session_provider;
}

void MultiPlayerSDL::msg_game_beacon(UDPpacket *p)
{
	MsgGameBeacon *m;
	SDLSessionDesc *desc;

	if (p->len != sizeof(struct MsgGameBeacon))
		return;

	if (check_duplicates(&p->address))
		return;

	m = (MsgGameBeacon *)p->data;
	desc = new SDLSessionDesc();

	strncpy(desc->session_name, m->name, MP_SESSION_NAME_LEN);
	desc->session_name[MP_SESSION_NAME_LEN] = 0;
	desc->pass_word[0] = m->password;
	desc->address.host = p->address.host;
	desc->address.port = p->address.port;
	desc->id = current_sessions.size();
	current_sessions.linkin(desc);

	MSG("[MultiPlayerSDL::poll_sessions] got beacon for game '%s'\n", desc->session_name);
}

int MultiPlayerSDL::poll_sessions()
{
	UDPpacket packet;
	int ret;

	err_when(!init_flag);

	if (!game_sock) {
		game_sock = SDLNet_UDP_Open(UDP_GAME_PORT);
	}
	if (!game_sock) {
		ERR("unable to open session list socket: %s\n", SDLNet_GetError());
		return 0;
	}

	current_sessions.zap();

	packet.data = (Uint8 *)recv_buf;
	packet.maxlen = MP_UDP_MAX_PACKET_SIZE;

	while (1) {
		struct MsgHeader *p;

		p = (struct MsgHeader *)recv_buf;

		ret = SDLNet_UDP_Recv(game_sock, &packet);
		if (ret <= 0)
			break;

		switch (p->msg_id)
		{
		case MPMSG_GAME_BEACON:
			msg_game_beacon(&packet);
			break;
		}

	}

	if (use_remote_session_provider)
	{
		struct MsgRequestGameList m;
		UDPpacket request;

		m.msg_id = MPMSG_REQ_GAME_LIST;

		request.data = (Uint8 *)&m;
		request.maxlen = sizeof(struct MsgRequestGameList);
		request.address.host = remote_session_provider_address.host;
		request.address.port = remote_session_provider_address.port;

		SDLNet_UDP_Send(game_sock, -1, &request);
	}

	return 1;
}

// return a session description
//
// <int> i			i-th session (i start from 1)
// return pointer to a session, NULL if no more
SDLSessionDesc *MultiPlayerSDL::get_session(int i)
{
	if( i <= 0 || i > current_sessions.size() )
		return NULL;
	return (SDLSessionDesc *)current_sessions.get(i);
}

// create a new session
//
// <char *> sessionName      arbitary name to identify a session, input from user
// <char *> playerName       name to identify the local player's name in this session
// <int>    maxPlayers       maximum no. of players in a session
//
// return TRUE if success
int MultiPlayerSDL::create_session(char *sessionName, char *playerName, int maxPlayers)
{
	IPaddress ip_address;

	err_when(!init_flag || maxPlayers <= 0 || maxPlayers > MAX_NATION);

	// open socket for listening

	if (SDLNet_ResolveHost(&ip_address, NULL, GAME_PORT) == -1) {
		ERR("[MultiPlayerSDL::create_session] failed to resolve hostname: %s\n", SDLNet_GetError());
		return FALSE;
	}

	listen_sock = SDLNet_TCP_Open(&ip_address);
	if (!listen_sock) {
		ERR("[MultiPlayerSDL::create_session] failed to start listening: %s\n", SDLNet_GetError());
		return FALSE;
	} else {
		MSG("[MultiPlayerSDL::create_session] waiting for clients on port: %d\n", (int)GAME_PORT);
	}

	peer_sock = SDLNet_UDP_Open(GAME_PORT);
	if (!peer_sock) {
		ERR("[MultiPlayerSDL::create_session] unable to open peer socket: %s\n", SDLNet_GetError());
		SDLNet_TCP_Close(listen_sock);
		listen_sock = NULL;
		return FALSE;
	}

	joined_session.id = 0;
	strcpy(joined_session.session_name, sessionName);
	joined_session.pass_word[0] = 0;

	host_flag = 1;
	max_players = maxPlayers;
	allowing_connections = 1;

	// Add hosts machine's player to the pool now
	if (!add_player(playerName, 1)) {
		return FALSE;
	}
	set_my_player_id(1);

	return TRUE;
}

// join a session, by passing the index passed into get_session()
// note : do not call poll_sessions between get_session and join_session
//
// <int> currentSessionIndex       the index passed into get_session()
//
// currentSessionIndex start from 1
int MultiPlayerSDL::join_session(int i, char *playerName)
{
	SDLSessionDesc *session = (SDLSessionDesc *)current_sessions.get(i);
	if (!session)
		return FALSE;

	// establish connection with server
	host_sock = SDLNet_TCP_Open(&session->address);
	if (!host_sock) {
		MSG("[MultiPlayerSDL::join_session] failed to connect to server: %s\n", SDLNet_GetError());
		return FALSE;
	} else {
		MSG("[MultiPlayerSDL::join_session] successfully connected to server\n");
	}

	int total = SDLNet_TCP_AddSocket(sock_set, host_sock);
	if (total == -1) {
		ERR("[MultiPlayerSDL::join_session] SDLNet_AddSocket: %s\n", SDLNet_GetError());
		err_now("socket error");
	}

	peer_sock = SDLNet_UDP_Open(0);
	if (!peer_sock) {
		ERR("[MultiPlayerSDL::join_session] unable to open peer socket: %s\n", SDLNet_GetError());
		SDLNet_TCP_Close(host_sock);
		host_sock = NULL;
		return FALSE;
	}

	max_players = MAX_NATION;

	// register the host now, even though his name is not known yet
	player_pool[0].address.host = session->address.host;
	player_pool[0].address.port = session->address.port;
	player_pool[0].connecting = 1;

	joined_session = *session;

	return TRUE;
}

void MultiPlayerSDL::close_session()
{
}

void MultiPlayerSDL::disable_join_session()
{
	allowing_connections = 0;
}

void MultiPlayerSDL::accept_connections()
{
	static Uint32 ticks = 0;
	TCPsocket connecting;
	uint32_t player_id;
	Uint32 cur_ticks;

	// accept_connections shouldn't be used by clients
	if (!host_flag) return;

	cur_ticks = SDL_GetTicks();
	if (peer_sock && (cur_ticks > ticks + 3000 || cur_ticks < ticks)) {
		// send the session beacon
		UDPpacket packet;
		struct MsgGameBeacon p;

		ticks = cur_ticks;

		p.msg_id = MPMSG_GAME_BEACON;
		strncpy(p.name, joined_session.session_name, MP_SESSION_NAME_LEN);
#if 0
		if (joined_session.pass_word[0]) {
			p.password = 1;
		else
#endif
			p.password = 0;
		

		packet.channel = -1;
		packet.data = (Uint8 *)&p;
		packet.len = sizeof(struct MsgGameBeacon);
		packet.address.host = lan_broadcast_address.host;
		packet.address.port = lan_broadcast_address.port;

		SDLNet_UDP_Send(peer_sock, packet.channel, &packet);

		if (use_remote_session_provider)
		{
			SDLNet_UDP_Send(peer_sock, -1, &packet);
		}
	}

	connecting = SDLNet_TCP_Accept(listen_sock);
	if (!connecting) {
		return;
	}
	if (!allowing_connections) {
		SDLNet_TCP_Close(connecting);
		return;
	}
	player_id = create_player(connecting);
	if (!player_id) {
		MSG("[MultiPlayerSDL::accept_connections] no room to accept new clients.\n");
		SDLNet_TCP_Close(connecting);
		return;
	}

	MSG("[MultiPlayerSDL::accept_connections] client accepted\n");
}

// Create a player and add to the pool.
//
// This is only called by the host upon connection from a client. The host
// chooses the player's id.
//
// Returns 1 if the player was added to the pool, and 0 if the player
// wasn't added to the pool.
//
int MultiPlayerSDL::create_player(TCPsocket socket)
{
	int i;

	// search for an empty slot
	for (i = 0; i < max_players; i++)
		if (!player_pool[i].id)
			break;
	if (i >= max_players)
		return 0;

	// add to the pool
	player_pool[i].id = i+1;
	strcpy(player_pool[i].name, "?anonymous?");
	player_pool[i].connecting = 1;
	player_pool[i].socket = socket;

	int total = SDLNet_TCP_AddSocket(sock_set, socket);
	if (total == -1) {
		ERR("[MultiPlayerSDL::accept_connections] SDLNet_AddSocket: %s\n", SDLNet_GetError());
		err_now("socket error");
	}

	return 1;
}

// Adds a player already created by the host to the pool
//
// <char *>   name        name of the player
// <uint32_t> id          id provided by the game host
//
// Returns 0 if the player cannot be added, and 1 if the player was added.
//
int MultiPlayerSDL::add_player(char *name, uint32_t id)
{
	if (player_pool[id-1].id)
		// if this happens, we got problems
		return 0;

	// add to the pool
	player_pool[id-1].id = id;
	strncpy(player_pool[id-1].name, name, MP_FRIENDLY_NAME_LEN);
	player_pool[id-1].connecting = 1;
	player_pool[id-1].socket = NULL; // not used by a client

	return 1;
}

void MultiPlayerSDL::set_my_player_id(uint32_t id)
{
	IPaddress *local;

	err_when(!id || id > max_players);

	my_player_id = id;

	local = SDLNet_UDP_GetPeerAddress(peer_sock, -1);
	SDLNet_ResolveHost(&player_pool[my_player_id-1].address, "127.0.0.1", local->port);

	MSG("[MultiPlayerSDL::set_my_player_id] set my_player_id to %d with address %x %x\n", id, player_pool[my_player_id-1].address.host, player_pool[my_player_id-1].address.port);
}

void MultiPlayerSDL::set_player_name(uint32_t id, char *name)
{
	strncpy(player_pool[id-1].name, name, MP_FRIENDLY_NAME_LEN);
}

// Deletes a player from the pool
//
// <uint32_t> id          id provided by the game host
//
void MultiPlayerSDL::delete_player(uint32_t id)
{
	err_when(id < 1 || id > max_players);
	if (player_pool[id-1].id) {
		if (player_pool[id-1].socket) {
			SDLNet_TCP_DelSocket(sock_set, player_pool[id-1].socket);
			SDLNet_TCP_Close(player_pool[id-1].socket);
			player_pool[id-1].socket = NULL;
		}
		player_pool[id-1].id = 0;
		player_pool[id-1].connecting = 0;
		memset(&player_pool[id-1].address, 0, sizeof(player_pool[id-1].address));
	}
}

void MultiPlayerSDL::poll_players()
{
}

SDLPlayer *MultiPlayerSDL::get_player(int i)
{
	if (i < 1 || i > max_players || player_pool[i-1].id != i)
		return NULL;
	return &player_pool[i-1];
}

SDLPlayer *MultiPlayerSDL::search_player(uint32_t playerId)
{
	if (playerId < 1 || playerId > max_players || player_pool[playerId-1].id != playerId)
		return NULL;
	return &player_pool[playerId-1];
}

// determine whether a player is lost
//
// MultiPlayerSDL::received must be called (or remote.poll_msg) , 
// so if a player is really lost, the system message from 
// directPlay is received
//
int MultiPlayerSDL::is_player_connecting(uint32_t playerId)
{
	if (playerId < 1 || playerId > max_players || player_pool[playerId-1].id != playerId)
		return 0;
	return player_pool[playerId-1].connecting;
}

int MultiPlayerSDL::get_player_count()
{
	int count = 0;
	for (int i = 0; i < max_players; i++)
		if (player_pool[i].id == i+1 && player_pool[i].connecting)
			count++;
	return count;
}

// send udp message
//
// pass BROADCAST_PID as toId to all players
//
// return TRUE on success
//
int MultiPlayerSDL::send(uint32_t to, void * data, uint32_t msg_size)
{
	err_when(to > max_players);

	if (!peer_sock)
		return FALSE;
	if (to && to == my_player_id)
		return FALSE;
	if (msg_size > MP_UDP_MAX_PACKET_SIZE) {
		ERR("[MultiPlayerSDL::send] message exceeds maximum size\n");
		return FALSE;
	}

	if (to == BROADCAST_PID) {
		int i;
		for (i = 0; i < max_players; i++)
			if (player_pool[i].connecting && i+1 != my_player_id)
				this->send(i+1, data, msg_size);
		return TRUE;
	}

	if (player_pool[to-1].connecting) {
		UDPpacket packet;

		packet.channel = -1;
		packet.data = (Uint8 *)data;
		packet.len = msg_size;
		packet.address.host = player_pool[to-1].address.host;
		packet.address.port = player_pool[to-1].address.port;

		if (!SDLNet_UDP_Send(peer_sock, packet.channel, &packet)) {
			ERR("[MultiPlayerSDL::send] error while sending data to player %d\n", to);
			return FALSE;
		}
		MSG("[MultiPlayerSDL::send] sent %d bytes to player %d\n", packet.status, to);
		return TRUE;
	}

	return FALSE;
}

// send tcp message
//
// pass BROADCAST_PID as toId to all players
//
// return TRUE on success
//
int MultiPlayerSDL::send_stream(uint32_t to, void * data, uint32_t msg_size)
{
	TCPsocket dest;

	err_when(to > max_players);

	if (to && to == my_player_id)
		return FALSE;

	if (host_flag) {
		if (to == BROADCAST_PID) {
			int i;
			for (i = 0; i < max_players; i++)
				if (player_pool[i].socket && i+1 != my_player_id)
					send_stream(i+1, data, msg_size);
			return TRUE;
		}

		if (!player_pool[to-1].socket) {
			MSG("[MultiPlayerSDL::send_stream] player %d is not connected\n", to);
			return FALSE;
		}
		dest = player_pool[to-1].socket;
	} else {
		// clients forward through the host
		if (!host_sock) {
			MSG("[MultiPlayerSDL::send_stream] not connected to game host\n");
			return FALSE;
		}
		dest = host_sock;
	}

	int bytes_sent = 0;

	// message structure:
	//
	// uint32_t msg_size;  // message _data_ size
	// uint32_t to;        // receiver id
	// byte[]   data;      // byte array of size msg_size

	const int send_buf_size = sizeof(msg_size) + sizeof(to);
	char send_buf[send_buf_size];

	SDLNet_Write32(msg_size, send_buf);
	SDLNet_Write32(to, send_buf + sizeof(msg_size));

	bytes_sent += SDLNet_TCP_Send(dest, send_buf, send_buf_size);
	bytes_sent += SDLNet_TCP_Send(dest, data, msg_size);
	if (bytes_sent != send_buf_size + msg_size) {
		ERR("[MultiPlayerSDL::send_stream] error while sending data to player %d\n", to);
		return FALSE;
	}

	MSG("[MultiPlayerSDL::send_stream] bytes sent: %d to %d\n", (int)bytes_sent, (int)to);
	return TRUE;
}

// receive udp message
//
// return NULL if fails
// sysMsgCount records how many system messages have been handled
// notice : *sysMsgCount may be != 0, but return NULL
//
char *MultiPlayerSDL::receive(uint32_t * from, uint32_t * to, uint32_t * size, int *sysMsgCount)
{
	if (sysMsgCount) *sysMsgCount = 0;
	*from = max_players;

	if (peer_sock) {
		UDPpacket packet;
		int ret;

		packet.data = (Uint8 *)recv_buf;
		packet.maxlen = MP_UDP_MAX_PACKET_SIZE;

		ret = SDLNet_UDP_Recv(peer_sock, &packet);
		if (ret > 0) {
			int i;
			for (i = 0; i < max_players; i++) {
				if (player_pool[i].connecting &&
				    player_pool[i].address.host == packet.address.host &&
				    player_pool[i].address.port == packet.address.port)
					break;
			}
			*from = i < max_players ? i+1 : 0;
			*to = my_player_id;
			*size = packet.len;
			MSG("[MultiPlayerSDL::receive] received %d bytes from player %d\n", *size, *from);
			if (!*from) {
				discovery = *(uint32_t *)packet.data;
				discovery_address.host = packet.address.host;
				discovery_address.port = packet.address.port;
			}
			return *from ? recv_buf : NULL;
		} else if (ret < 0) {
			ERR("[MultiPlayerSDL::receive] could not receive: %s\n", SDLNet_GetError());
		}
	}
	return NULL;
}

// receive tcp message
//
// return NULL if fails
// sysMsgCount records how many system messages have been handled
// notice : *sysMsgCount may be != 0, but return NULL
//
// This function has deficiencies... Only one message from one socket may be
// processed at a time. So a round robin is used to make sure that one client
// high in the list can't hog the connection.
//
// TODO: rename sysMsgCount to playerLost and update the logic
//       (because sysMsgCount is only used to determine playerLost event)
//
// Note: When a disconnection does occur, the socket is closed by a later
// event handler
//
char *MultiPlayerSDL::receive_stream(uint32_t *from, uint32_t *to, uint32_t *size, int *sysMsgCount)
{
	static int round_robin = 0; // used to poll clients fairly
	TCPsocket socket = NULL;
	uint32_t msg_size;
	uint32_t source_id;
	uint32_t target_id;
	int ready;
	int player_index;
	const int header_size = sizeof(msg_size) + sizeof(target_id);

	err_when(!from || !to || !size || !recv_buf);

	// have game host accept connections during game setup
	accept_connections();

	if (sysMsgCount) *sysMsgCount = 0;

	ready = SDLNet_CheckSockets(sock_set, 0);
	if (!ready)
		return NULL;

	// find out who to receive from
	if (host_flag) {
		int count = 0;
		while (count++ < max_players) {
			if (player_pool[round_robin].socket &&
			    SDLNet_SocketReady(player_pool[round_robin].socket)) {
				socket = player_pool[round_robin].socket;
				player_index = round_robin;
				break;
			}
			round_robin++;
			if (round_robin >= max_players)
				round_robin = 0;
		}
	} else if (host_sock) {
		socket = host_sock;
		player_index = 0;
	}
	if (!socket)
		return NULL;

	// read the message header
	ready = SDLNet_TCP_Recv(socket, recv_buf, header_size);
	if (ready <= 0) {
		player_pool[player_index].connecting = 0;
		if (sysMsgCount) *sysMsgCount = 1;
		return NULL;
	} else if (ready < header_size) {
		err_now("unhandled non-blocking operation?");
	}

	msg_size = SDLNet_Read32(recv_buf);
	target_id = SDLNet_Read32(recv_buf + sizeof(msg_size));

	// we will impose a size limitation to avoid an expensive resizing
	// of the buffer
	if (msg_size > MP_RECV_BUFFER_SIZE) {
		MSG("[MultiPlayerSDL::receive_stream] player %d wants to send %d bytes, rejected\n", player_index+1, msg_size);
		player_pool[player_index].connecting = 0;
		if (sysMsgCount) *sysMsgCount = 1;
		return NULL;
	}

	// read in the data
	ready = SDLNet_TCP_Recv(socket, recv_buf, msg_size);
	if (ready <= 0) {
		player_pool[player_index].connecting = 0;
		if (sysMsgCount) *sysMsgCount = 1;
		return NULL;
	} else if (ready < msg_size) {
		err_now("unhandled non-blocking operation?");
	}

	// finish the message
	*to = target_id;
	*size = msg_size;
	*from = player_index+1;

	MSG("[MultiPlayerSDL::receive_stream] received %d bytes from %d\n", msg_size, *from);

	return recv_buf;
}

void MultiPlayerSDL::send_discovery()
{
	// Only send udp discovery packets if we are connected by tcp to the game host
	if (host_sock) {
		this->send(1, &my_player_id, sizeof(my_player_id));
	}
}

// receive_discovery -- Allows a game host to recognize the udp address of a peer.
// Hopefully this will allow NAT transversal too.
//
// returns zero if there are no new connections
// returns len, which is the size of IPaddress, sets the pointer to address, and
// who this is coming from.
int MultiPlayerSDL::receive_discovery(uint32_t *who, void **address)
{
	int ret = 0;

	// Only receive udp discovery packets if we are listening by tcp server
	if (listen_sock) {
		uint32_t from, to, size;
		int sysMsg;
		char *ptr;

		discovery = 0;

		ptr = this->receive(&from, &to, &size, &sysMsg);
		if (!ptr && discovery && discovery <= max_players) {
			MSG("[MultiPlayerSDL::receive_discovery] Received discovery from %d\n", discovery);
			player_pool[discovery-1].address.host = discovery_address.host;
			player_pool[discovery-1].address.port = discovery_address.port;
			ret = sizeof(IPaddress);
			*who = discovery;
			*address = &discovery_address;
		}
		if ((!ptr && discovery) || ptr) {
			char *ack = (char *)"ACK";
			to = ptr ? from : discovery;
			this->send(to, ack, 3);
		}
	}

	return ret;
}

int MultiPlayerSDL::receive_discovery_ack()
{
	// Only receive the discovery ack by a client
	if (host_sock) {
		uint32_t from, to, size;
		int sysMsg;
		char *ptr;

		ptr = this->receive(&from, &to, &size, &sysMsg);
		if (ptr && from == 1 && !memcmp(recv_buf, "ACK", 3)) {
			MSG("[MultiPlayerSDL::receive_discovery_ack] received ack\n");
			return 1;
		}
	}
	return 0;
}

void MultiPlayerSDL::set_peer_address(uint32_t who, void *address)
{
	IPaddress *a = (IPaddress *)address;

	err_when(who < 1 || who > max_players);

	if (who == my_player_id)
		return;

	player_pool[who-1].address.host = a->host;
	player_pool[who-1].address.port = a->port;

	MSG("[MultiPlayerSDL::set_peer_address] set address for %d\n", who);
}

/*
static int sort_session_id(const void *a, const void *b)
{
	return memcmp( &((SDLSessionDesc *)a)->guidInstance, &((SDLSessionDesc *)b)->guidInstance,
		sizeof(GUID) );
}
*/

static int sort_session_name(const void *a, const void *b)
{
	return strcmp( ((SDLSessionDesc *)a)->name_str(), ((SDLSessionDesc *)b)->name_str() );
}

// sort current_sessions
// <int> sortType, 1=sort by GUID, 2=sort by session name
void MultiPlayerSDL::sort_sessions(int sortType)
{

	// BUGHERE : quick_sort is a DynArray function but current_sessions is DynArrayB
	switch(sortType)
	{
	case 1:
		ERR("[MultiPlayerSDL::sort_sessions] sorting by GUID is not supported\n");
		//current_sessions.quick_sort(sort_session_id);
		break;
	case 2:
		current_sessions.quick_sort(sort_session_name);
		break;
	default:
		err_here();
	}
}

