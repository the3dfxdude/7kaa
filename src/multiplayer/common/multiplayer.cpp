/*
 * Seven Kingdoms: Ancient Adversaries
 *
 * Copyright 1997,1998 Enlight Software Ltd.
 * Copyright 2010,2011,2013,2015,2016 Jesse Allen
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

// Filename    : multiplayer.cpp
// Description : Multiplayer game support.

#include <multiplayer.h>
#include <ALL.h>
#include <version.h>
#include <string.h>
#include <stdint.h>
#include <dbglog.h>

DBGLOG_DEFAULT_CHANNEL(MultiPlayer);

#define MP_UDP_MAX_PACKET_SIZE 800

const uint16_t UDP_GAME_PORT = 19255;
const uint16_t UDP_MONITOR_PORT = 19383;

SessionDesc::SessionDesc()
{
	misc.uuid_generate_random(id);
	flags = SESSION_PREGAME;
	max_players = MAX_NATION;
	player_count = 1;
	memset(password, 0, sizeof(session_name));
	memset(password, 0, sizeof(password));
	this->address.host = ENET_HOST_ANY;
	this->address.port = UDP_GAME_PORT;
}

SessionDesc::SessionDesc(const char *name, const char *pass, ENetAddress *address)
{
	misc.uuid_generate_random(id);
	flags = SESSION_PREGAME;
	max_players = MAX_NATION;
	player_count = 1;
	strncpy(session_name, name, MP_FRIENDLY_NAME_LEN);
	session_name[MP_FRIENDLY_NAME_LEN] = 0;
	strncpy(password, pass, MP_FRIENDLY_NAME_LEN);
	password[MP_FRIENDLY_NAME_LEN] = 0;
	if (pass[0])
		flags |= SESSION_PASSWORD;
	this->address.host = address->host;
	this->address.port = address->port;
}

SessionDesc::SessionDesc(MpMsgUserSessionStatus *m, ENetAddress *address)
{
	misc.uuid_generate_random(id);
	flags = m->flags;
	max_players = MAX_NATION;
	player_count = 1;
	strncpy(this->session_name, m->session_name, MP_FRIENDLY_NAME_LEN);
	session_name[MP_FRIENDLY_NAME_LEN] = 0;
	this->password[0] = 0;
	this->address.host = address->host;
	this->address.port = address->port;
}

SessionDesc::SessionDesc(const SessionDesc &src)
{
	misc.uuid_copy(id, src.id);
	flags = src.flags;
	max_players = src.max_players;
	player_count = src.player_count;
	strncpy(session_name, src.session_name, MP_FRIENDLY_NAME_LEN);
	session_name[MP_FRIENDLY_NAME_LEN] = 0;
	strncpy(password, src.password, MP_FRIENDLY_NAME_LEN);
	password[MP_FRIENDLY_NAME_LEN] = 0;
	address.host = src.address.host;
	address.port = src.address.port;
}

SessionDesc& SessionDesc::operator= (const SessionDesc &src)
{
	misc.uuid_copy(id, src.id);
	flags = src.flags;
	max_players = src.max_players;
	player_count = src.player_count;
	strncpy(session_name, src.session_name, MP_FRIENDLY_NAME_LEN);
	session_name[MP_FRIENDLY_NAME_LEN] = 0;
	strncpy(password, src.password, MP_FRIENDLY_NAME_LEN);
	password[MP_FRIENDLY_NAME_LEN] = 0;
	address.host = src.address.host;
	address.port = src.address.port;
	return *this;
}

inline bool cmp_addr(ENetAddress *a, ENetAddress *b)
{
	return a->host == b->host && a->port == b->port;
}

// to start a multiplayer game, first check if it is called from a
// lobbied (MultiPlayer::is_lobbied)

// if it is a lobbied, call init_lobbied

// if not, call poll_service_provider; display them and let
// user to select, call init and pass the guid of the selected
// service; create_session or poll_sessions+join_session;

MultiPlayer::MultiPlayer() :
	current_sessions(sizeof(SessionDesc), 10)
{
	init_flag = 0;
	lobbied_flag = 0;
	supported_protocols = TCPIP;
	my_player_id = 0;
	my_player = NULL;
	joined_session.flags = 0;
	recv_buf = NULL;
}

MultiPlayer::~MultiPlayer()
{
	deinit();
}

void MultiPlayer::init(ProtocolType protocol_type)
{
	if (init_flag)
		return;

	lobbied_flag = 0;
	my_player_id = 0;
	my_player = NULL;
	use_remote_session_provider = 0;
	update_available = -1;
	host = NULL;
	session_monitor = ENET_SOCKET_NULL;
	packet_mode = ENET_PACKET_FLAG_RELIABLE;

	if (!is_protocol_supported(protocol_type)) {
		ERR("[MultiPlayer::init] trying to init unsupported protocol\n");
		return;
	}

	if (enet_initialize() != 0) {
		ERR("Could not init the enet library.\n");
		return;
	}

	session_monitor = create_socket(UDP_MONITOR_PORT);

	for (int i = 0; i < MAX_NATION; i++) {
		player_pool[i] = NULL;
		pending_pool[i] = NULL;
	}

	recv_buf = new char[MP_RECV_BUFFER_SIZE];

	init_flag = 1;
}

void MultiPlayer::deinit()
{
	if (!init_flag) {
		return;
	}

	init_flag = 0;
	lobbied_flag = 0;

	current_sessions.zap();

	destroy_socket(session_monitor);

	if (host) {
		ENetPeer *peer;
		for (peer = host->peers; peer < &host->peers[host->peerCount]; ++peer) {
			if (peer->state == ENET_PEER_STATE_CONNECTED) {
				enet_peer_disconnect_now(peer, 0);
			}
			if (peer->data) {
				delete (PlayerDesc *)peer->data;
				peer->data = NULL;
			}
		}
	}
	for (int i = 0; i < MAX_NATION; i++) {
		if (pending_pool[i]) {
			delete pending_pool[i];
		}
	}
	delete my_player;

	close_port();
	enet_deinitialize();

	if (recv_buf) {
		delete [] recv_buf;
	}
}

// init_lobbied
// Reads the command line and sets lobby mode if the command line is correct.
// Returns non-zero on success.
int MultiPlayer::init_lobbied(int maxPlayers, char *cmdLine)
{
	MSG("Launching a multiplayer game from command line maxPlayers=%d, cmdLine='%s'\n", maxPlayers, cmdLine);

	if (cmdLine) {
		ENetAddress address;
		SessionDesc *session;

		if (enet_address_set_host(&address, cmdLine) != 0) {
			return 0;
		}
		address.port = UDP_GAME_PORT;

		session = new SessionDesc("Lobbied Game", "1", &address);

		current_sessions.linkin(session);

		lobbied_flag = 2;
	} else {
		// hosting doesn't work yet
		lobbied_flag = 1;
	}

	return 1;
}

// return 0=not lobbied, 1=auto create, 2=auto join, 4=selectable
int MultiPlayer::is_lobbied()
{
	return lobbied_flag;
}

// get_lobbied_name() is used to get the player's name when the game is
// launched to the lobby -- this probably won't ever be needed.
char *MultiPlayer::get_lobbied_name()
{
	return NULL;
}

void MultiPlayer::poll_supported_protocols()
{
}

bool MultiPlayer::is_protocol_supported(ProtocolType protocol)
{
	return (protocol & supported_protocols) != 0;
}

int MultiPlayer::is_update_available()
{
	return update_available;
}

ENetSocket MultiPlayer::create_socket(uint16_t port)
{
	ENetSocket socket;
	ENetAddress address;

	socket = enet_socket_create(ENET_SOCKET_TYPE_DATAGRAM);
	if (socket == ENET_SOCKET_NULL)
		return ENET_SOCKET_NULL;

	address.host = ENET_HOST_ANY;
	address.port = port;

	enet_socket_set_option(socket, ENET_SOCKOPT_REUSEADDR, 1);
	if (enet_socket_bind(socket, &address)<0) {
		enet_socket_destroy(socket);
		return ENET_SOCKET_NULL;
	}
	enet_socket_set_option(socket, ENET_SOCKOPT_NONBLOCK, 1);

	return socket;
}

void MultiPlayer::destroy_socket(ENetSocket socket)
{
	if (socket != ENET_SOCKET_NULL)
		enet_socket_destroy(socket);
}

// Open a multiplayer game port to enable the network service. If successful,
// returns 1. If not successful, returns 0. If fallback is set, then it will
// try a second time using any port.
int MultiPlayer::open_port(uint16_t port, int fallback)
{
	ENetAddress address;

	err_when(!init_flag);
	err_when(host);

	address.host = ENET_HOST_ANY;
	address.port = port;

	host = enet_host_create(
		&address,
		2 * MAX_NATION,
		2,
		0,
		0
	);
	if (host) {
		MSG("Opened port %hu\n", host->address.port);
		return 1;
	}

	if (fallback)
		return open_port(0, 0);

	return 0;
}

void MultiPlayer::close_port()
{
	if (host) {
		enet_host_destroy(host);
		host = NULL;
	}
}

int MultiPlayer::set_remote_session_provider(const char *server)
{
	if (enet_address_set_host(
			&remote_session_provider_address,
			server) == 0) {
		remote_session_provider_address.port = UDP_MONITOR_PORT;
		use_remote_session_provider = 1;
	} else {
		use_remote_session_provider = 0;
	}

	return use_remote_session_provider;
}

int MultiPlayer::poll_sessions()
{
	ENetAddress a;
	ENetBuffer b;

	err_when(!init_flag);

	if (session_monitor == ENET_SOCKET_NULL) {
		return 0;
	}

	b.data = recv_buf;
	b.dataLength = MP_RECV_BUFFER_SIZE;

	current_sessions.zap();

	while (enet_socket_receive(session_monitor, &a, &b, 1)>0) {
		uint32_t id = *(uint32_t *)recv_buf;
		switch (id) {
		case MPMSG_USER_SESSION_STATUS: {
			MpMsgUserSessionStatus *m = (MpMsgUserSessionStatus *)recv_buf;

			if ((m->flags & (SESSION_HOSTING|SESSION_PREGAME)) == 0)
				break;
			if (get_session(&a))
				break;

			SessionDesc desc(m, &a);
			current_sessions.linkin(&desc);

			break;
			}
		}
	}

	if (use_remote_session_provider) {
		// Request a game list
	}

	return 1;
}

// return a session description
//
// <int> i			i-th session (i start from 1)
// return pointer to a session, NULL if no more
SessionDesc *MultiPlayer::get_session(int i)
{
	if( i <= 0 || i > current_sessions.size() )
		return NULL;
	return (SessionDesc *)current_sessions.get(i);
}

SessionDesc *MultiPlayer::get_session(ENetAddress *address)
{
	int i;
	for (i = 1; i <= current_sessions.size(); i++) {
		SessionDesc *p = (SessionDesc *)current_sessions.get(i);
		if (p && cmp_addr(&p->address, address))
			return p;
	}
	return NULL;
}

SessionDesc *MultiPlayer::get_current_session()
{
	return &joined_session;
}

// create a new session
//
// <char *> sessionName      arbitary name to identify a session, input from user
// <char *> playerName       name to identify the local player's name in this session
// <int>    maxPlayers       maximum no. of players in a session
//
// return 1 if success
int MultiPlayer::create_session(char *sessionName, char *password, char *playerName, int maxPlayers)
{
	err_when(!init_flag || maxPlayers <= 0 || maxPlayers > MAX_NATION || host);

#ifdef HOST_ANY_PORT
	if (!open_port(0, 0)) {
#else
	if (!open_port(UDP_GAME_PORT, 1)) {
#endif
		MSG("Unable to open a port for the session.\n");
		return 0;
	}

	strcpy(joined_session.session_name, sessionName);
	strcpy(joined_session.password, password);
	joined_session.max_players = maxPlayers;
	joined_session.player_count = 1;
	joined_session.flags |= SESSION_HOSTING | SESSION_PREGAME;
	if (password[0])
		joined_session.flags |= SESSION_PASSWORD;
	if (joined_session.player_count >= joined_session.max_players)
		joined_session.flags |= SESSION_FULL;

	my_player = new PlayerDesc(playerName);
	my_player->id = 1;
	set_my_player_id(1);
	player_pool[0] = my_player; // can we skip polling players?

	return 1;
}

// join a session
// note : do not call poll_sessions between get_session and join_session
//
int MultiPlayer::join_session(SessionDesc *session, char *playerName)
{
	ENetPeer *peer;
	PlayerDesc *game_host;

	err_when(!session || host);

	if (!open_port(0, 0)) {
		MSG("Unable to open a port for the session.\n");
		return 0;
	}

	peer = enet_host_connect(
		host,
		&session->address,
		1,
		0
	);
	if (!peer) {
		close_port();
		return 0;
	}

	joined_session = *session;
	joined_session.flags &= ~SESSION_HOSTING; // we're not the host

	game_host = new PlayerDesc(&session->address);
	game_host->id = 1;
	game_host->authorized = 1;
	peer->data = game_host;

	my_player = new PlayerDesc(playerName);

	return 1;
}

// Call close_session when leaving any session. Returns the number of
// disconnect procedures started.
int MultiPlayer::close_session()
{
	ENetPeer *peer;
	int count;

	err_when(!host);

	joined_session.flags = 0;

	count = 0;
	for (peer = host->peers; peer < &host->peers[host->peerCount]; ++peer) {
		if (peer->state == ENET_PEER_STATE_CONNECTED) {
			enet_peer_disconnect_later(peer, 0);
			count++;
		}
	}

	return count;
}

void MultiPlayer::game_starting()
{
	packet_mode = ENET_PACKET_FLAG_UNSEQUENCED;
}

void MultiPlayer::disable_new_connections()
{
	ENetPeer *peer;

	joined_session.flags &= ~SESSION_PREGAME;

	for (peer = host->peers; peer < &host->peers[host->peerCount]; ++peer) {
		if (peer->data) {
			PlayerDesc *player = (PlayerDesc *)peer->data;
			if (!player->authorized)
				enet_peer_disconnect(peer, 0);
		}
	}
}

// Returns the next available player id. Used by the game host for adding players.
uint32_t MultiPlayer::get_avail_player_id()
{
	uint32_t playerId;

	playerId = 2;
	while (get_peer(playerId))
		playerId++;

	return playerId;
}

// When a player is not already connected, or when initially connecting
// to a game host, then this will add the player to the pending list.
// Returns 0 if the list is full.
// Returns 1 if the player was added.
int MultiPlayer::add_pending_player(PlayerDesc *player)
{
	unsigned int i;

	for (i = 0; i < MAX_NATION; i++) {
		if (!pending_pool[i])
			break;
	}

	if (i >= MAX_NATION)
		return 0;

	pending_pool[i] = player;

	return 1;
}

// Returns NULL if player is not pending.
// Returns player if found. player is removed from pending pool and
// becomes the responsiblity of the caller.
PlayerDesc *MultiPlayer::yank_pending_player(uint32_t playerId)
{
	unsigned int i;
	PlayerDesc *player;

	for (i = 0; i < MAX_NATION; i++) {
		if (pending_pool[i] && pending_pool[i]->id == playerId) {
			break;
		}
	}

	if (i >= MAX_NATION)
		return NULL;

	player = pending_pool[i];
	pending_pool[i] = NULL;

	return player;
}

// Returns NULL if player is not pending.
// Returns player if found. player is removed from pending pool and
// becomes the responsiblity of the caller.
PlayerDesc *MultiPlayer::yank_pending_player(ENetAddress *address)
{
	unsigned int i;
	PlayerDesc *player;

	for (i = 0; i < MAX_NATION; i++) {
		if (pending_pool[i]) {
			if (cmp_addr(&pending_pool[i]->address, address))
				break;
		}
	}

	if (i >= MAX_NATION)
		return NULL;

	player = pending_pool[i];
	pending_pool[i] = NULL;

	return player;
}

// Adds a player that previously was unknown. Creates a player descriptor if needed, and
// initiates contact if requested. If contact is not requested, then the player is added
// to the pending pool, and waits for contact from that player.
//
// Returns 1 when the player was added succesfully.
// Returns 0 when the player was not added.
int MultiPlayer::add_player(uint32_t playerId, char *name, ENetAddress *address, char contact)
{
	PlayerDesc *player = NULL;
	ENetPeer *peer;

	if (playerId == my_player_id) {
		err_now("add player");
	}
	peer = get_peer(playerId);
	if (!peer && address->host != ENET_HOST_ANY) {
		peer = get_peer(address);
	}
	if (peer) {
		player = (PlayerDesc *)peer->data;
	}
	if (!player) {
		player = new PlayerDesc(address);
	}

	player->id = playerId;
	strncpy(player->name, name, MP_FRIENDLY_NAME_LEN);
	player->name[MP_FRIENDLY_NAME_LEN] = 0;
	player->authorized = 1;

	if (peer) {
		// already connected
		MSG("Player (%d) already connected\n", playerId);
		peer->data = player;
		poll_players();

	} else if (contact) {
		// initiate contact
		MSG("Contacting player (%d)\n", playerId);
		peer = enet_host_connect(host, address, 1, 0);
		if (!peer) {
			delete player;
			return 0;
		}
		peer->data = player;

	} else {
		MSG("Waiting for player (%d)\n", playerId);
		// wait for contact
		if (!add_pending_player(player)) {
			delete player;
			return 0;
		}
	}

	MSG("Player '%s' (%d) recognized.\n", player->name, playerId);

	return 1;
}

// Called when a player is identifying himself. The game organizer validates
// the password only, as the password is only sent to the organizer.
//
// Returns 1 when a player is newly authorized.
// Returns 0 when a player is not authorized.
int MultiPlayer::auth_player(uint32_t playerId, char *name, char *password)
{
	ENetPeer *peer;
	PlayerDesc *player;

	err_when(!host);
	err_when(!(joined_session.flags & SESSION_HOSTING));

	peer = get_peer(playerId);
	if (!peer || !peer->data) {
		return 0;
	}
	player = (PlayerDesc *)peer->data;

	if ((joined_session.flags & SESSION_PASSWORD) && memcmp(password, joined_session.password, MP_FRIENDLY_NAME_LEN)) {
		MSG("Player (%d) password is incorrect.\n", playerId);
		return 0;
	}

	strncpy(player->name, name, MP_FRIENDLY_NAME_LEN+1);
	player->name[MP_FRIENDLY_NAME_LEN] = 0;
	player->authorized = 1;

	MSG("Player '%s' (%d) was authorized.\n", player->name, playerId);
	poll_players();

	return 1;
}

int MultiPlayer::set_my_player_id(uint32_t playerId)
{
	my_player_id = playerId;
	my_player->id = playerId;
	my_player->authorized = 1;

	MSG("You have been assigned id=%d\n", playerId);

	return 1;
}

// Deletes a player by id, disconnecting if necessary.
void MultiPlayer::delete_player(uint32_t playerId)
{
	ENetPeer *peer;
	PlayerDesc *player;

	err_when(!host);

	player = NULL;
	peer = get_peer(playerId);
	if (peer) {
		enet_peer_disconnect(peer, 0);
		player = (PlayerDesc *)peer->data;
		peer->data = NULL;
		MSG("Requesting disconnect from player '%s' (%d).\n", player->name, playerId);
		poll_players();
	} else {
		player = yank_pending_player(playerId);
		if (!player)
			return;
	}

	MSG("Player '%s' (%d) deleted.\n", player->name, playerId);
	delete player;
}

static int sort_players(const void *a, const void *b)
{
	if (((PlayerDesc *)a)->id < ((PlayerDesc *)b)->id)
		return -1;
	if (((PlayerDesc *)a)->id > ((PlayerDesc *)b)->id)
		return 1;
	return 0;
}

void MultiPlayer::poll_players()
{
	ENetPeer *peer;
	unsigned int i;

	player_pool[0] = my_player;
	i = 1;
	for (peer = host->peers; peer < &host->peers[host->peerCount]; ++peer) {
		if (i >= joined_session.max_players)
			break;
		if (peer->data) {
			PlayerDesc *player = (PlayerDesc *)peer->data;

			if (peer->state == ENET_PEER_STATE_CONNECTED && player->authorized) {
				player_pool[i++] = player;
			}
		}
	}

	joined_session.player_count = i;
	if (joined_session.player_count >= joined_session.max_players)
		joined_session.flags |= SESSION_FULL;
	else
		joined_session.flags &= ~SESSION_FULL;

	for ( ; i < MAX_NATION; i++)
		player_pool[i] = NULL;

	qsort(&player_pool, joined_session.player_count, sizeof(player_pool[0]), &sort_players);
}

// retrieve the ith-1 index from the player pool array
PlayerDesc *MultiPlayer::get_player(int i)
{
	if (i < 1 || i > MAX_NATION)
		return NULL;

	return player_pool[i-1];
}

// retrieve the player by ID value from the player pool array
PlayerDesc *MultiPlayer::search_player(uint32_t playerId)
{
	unsigned int i;

	for (i = 0; i < MAX_NATION; i++) {
		if (player_pool[i] && player_pool[i]->id == playerId)
			break;
	}

	if (i >= MAX_NATION)
		return NULL;

	return player_pool[i];
}

// retrieve the player by ID value from enet peer arrary
ENetPeer *MultiPlayer::get_peer(uint32_t playerId)
{
	ENetPeer *peer;

	err_when(!host);

	for (peer = host->peers; peer < &host->peers[host->peerCount]; ++peer) {
		if (peer->data != NULL) {
			PlayerDesc *player;

			player = (PlayerDesc *)peer->data;

			if (player->id == playerId)
				break;
		}
	}

	if (peer < &host->peers[host->peerCount]) {
		return peer;
	}

	return NULL;
}

// retrieve the player by ID value from enet peer arrary
ENetPeer *MultiPlayer::get_peer(ENetAddress *address)
{
	ENetPeer *peer;

	err_when(!host);

	for (peer = host->peers; peer < &host->peers[host->peerCount]; ++peer) {
		if (cmp_addr(&peer->address, address))
			break;
	}

	if (peer < &host->peers[host->peerCount]) {
		return peer;
	}

	return NULL;
}

// determine whether a player is lost
//
// MultiPlayer::received must be called (or remote.poll_msg) , 
// so if a player is really lost, the system message from 
// directPlay is received
//
int MultiPlayer::is_player_connecting(uint32_t playerId)
{
	ENetPeer *peer;

	if (playerId == my_player_id)
		return 1;

	peer = get_peer(playerId);
	if (!peer)
		return 0;

	return peer->state == ENET_PEER_STATE_CONNECTED;
}

int MultiPlayer::get_player_count()
{
	return joined_session.player_count;
}

// send udp message
//
// pass BROADCAST_PID as toId to all players
//
// return 1 on success
//
int MultiPlayer::send(uint32_t to, void *data, uint32_t msg_size)
{
	ENetPacket *packet;

	err_when(!host);

	if (to == my_player_id) {
		return 0;
	}

	if (msg_size > MP_UDP_MAX_PACKET_SIZE) {
		ERR("Packet message exceeds maximum size.\n");
		return 0;
	}

	packet = enet_packet_create(
		data,
		msg_size,
		packet_mode
	);

	if (to == BROADCAST_PID) {
		ENetPeer *peer;
		for (peer = host->peers; peer < &host->peers[host->peerCount]; ++peer) {
			if (peer->state == ENET_PEER_STATE_CONNECTED && peer->data) {
				PlayerDesc *player = (PlayerDesc *)peer->data;
				if (!player->authorized)
					continue;
				enet_peer_send(peer, 0, packet);
			}
		}
	} else {
		ENetPeer *peer;

		peer = get_peer(to);
		if (!peer) {
			return 0;
		}

		enet_peer_send(peer, 0, packet);
	}

	return 1;
}

void MultiPlayer::send_user_session_status(ENetAddress *a)
{
	ENetBuffer b;
	MpMsgUserSessionStatus m;

	if (!host)
		return;

	b.data = &m;
	b.dataLength = sizeof(MpMsgUserSessionStatus);

	m.msg_id = MPMSG_USER_SESSION_STATUS;
	m.game_version[0] = SKVERMAJ;
	m.game_version[1] = SKVERMED;
	m.game_version[2] = SKVERMIN;
	m.reserved0 = 0;
	m.flags = joined_session.flags;
	strncpy(m.session_name, joined_session.session_name, MP_FRIENDLY_NAME_LEN);

	enet_socket_send(host->socket, a, &b, 1);
}

// Returns pointer to the recv_buf when a message is received, with size set.
// Returns NULL when a message was not received.
//
// If a pointer for sysMsgCount is given:
//   The value of sysMsgCount is < 0 when a DISCONNECT event is encountered.
//   The value of sysMsgCount is > 0 when a CONNECT event is encountered.
//   Otherwise sysMsgCount is set to zero.
//
// The value of from is set to the playerId if the player was found.
// If a player was not found from is set to 0.
// Currently, pointers for from and size are always expected.
char *MultiPlayer::receive(uint32_t *from, uint32_t *size, int *sysMsgCount)
{
	int ret;
	ENetEvent event;
	PlayerDesc *player;
	char *got_recv;
	static unsigned long last_broadcast_time;
	unsigned long current_time;

	err_when(!host);

	// periodically broadcast status to the LAN
	current_time = misc.get_time();
	if (current_time > last_broadcast_time + 5000 || current_time < last_broadcast_time) {
		ENetAddress a;
		last_broadcast_time = current_time;
		a.host = ENET_HOST_BROADCAST;
		a.port = UDP_MONITOR_PORT;
		send_user_session_status(&a);
	}

	if (sysMsgCount)
		*sysMsgCount = 0;
	*from = 0;

	ret = enet_host_service(host, &event, 0);
	if (ret < 0) {
		err_now("enet_host_service");
	} else if (ret == 0) {
		return NULL;
	}

	got_recv = NULL;

	player = (PlayerDesc *)event.peer->data;
	if (player) {
		*from = player->id;
	}

	switch (event.type) {
	case ENET_EVENT_TYPE_RECEIVE:
		if (event.packet->dataLength < MP_RECV_BUFFER_SIZE) {
			memcpy(recv_buf, event.packet->data,
				event.packet->dataLength);
			*size = event.packet->dataLength;
			got_recv = recv_buf;
		}
		enet_packet_destroy(event.packet);

		break;

	case ENET_EVENT_TYPE_CONNECT:
		if (sysMsgCount)
			*sysMsgCount = 1;
		MSG("ENET_EVENT_TYPE_CONNECT connectedPeers=%d\n", host->connectedPeers);

		if (!player) {
			// The player may actually exist in the pending_pool.
			player = yank_pending_player(&event.peer->address);
		}

		if (!player) {
			if (!(joined_session.flags & SESSION_PREGAME)) {
				enet_peer_disconnect(event.peer, 0);
				break;
			}
			player = new PlayerDesc(&event.peer->address);
			if (joined_session.flags & SESSION_HOSTING) {
				player->id = get_avail_player_id();
			}
		}

		if (player) {
			MSG("Player '%s' (%d) connected.\n", player->name, player->id);
			*from = player->id;
			event.peer->data = player;
			poll_players();
		}

		break;

	case ENET_EVENT_TYPE_DISCONNECT:
		if (sysMsgCount)
			*sysMsgCount = -1;
		MSG("ENET_EVENT_TYPE_DISCONNECT connectedPeers=%d\n", host->connectedPeers);

		if (player) {
			MSG("Player '%s' (%d) disconnected.\n", player->name, player->id);
			if (joined_session.flags & SESSION_HOSTING) {
				delete player;
			} else {
				// try to save in case of a reconnection or host acknowledgement
				if (!player->authorized || !add_pending_player(player)) {
					delete player;
				}
			}
			event.peer->data = NULL;
			poll_players();
		}

		break;

	default:
		err_now("unhandled enet event");
	}

	return got_recv;
}

/*
static int sort_session_id(const void *a, const void *b)
{
	return memcmp( &((SessionDesc *)a)->guidInstance, &((SessionDesc *)b)->guidInstance,
		sizeof(GUID) );
}
*/

static int sort_session_name(const void *a, const void *b)
{
	return strcmp( ((SessionDesc *)a)->name_str(), ((SessionDesc *)b)->name_str() );
}

// sort current_sessions
// <int> sortType, 1=sort by GUID, 2=sort by session name
void MultiPlayer::sort_sessions(int sortType)
{

	// BUGHERE : quick_sort is a DynArray function but current_sessions is DynArrayB
	switch(sortType)
	{
	case 1:
		ERR("[MultiPlayer::sort_sessions] sorting by GUID is not supported\n");
		//current_sessions.quick_sort(sort_session_id);
		break;
	case 2:
		current_sessions.quick_sort(sort_session_name);
		break;
	default:
		err_here();
	}
}
