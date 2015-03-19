/*
 * Seven Kingdoms: Ancient Adversaries
 *
 * Copyright 1997,1998 Enlight Software Ltd.
 * Copyright 2010,2011,2013 Jesse Allen
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
#include <string.h>
#include <stdint.h>
#include <dbglog.h>

DBGLOG_DEFAULT_CHANNEL(MultiPlayer);

#define MP_UDP_MAX_PACKET_SIZE 800

const uint16_t UDP_GAME_PORT = 19255;


SessionDesc::SessionDesc()
{
	id = 0;
	memset(password, 0, sizeof(session_name));
	memset(password, 0, sizeof(password));
	this->address.host = ENET_HOST_ANY;
	this->address.port = UDP_GAME_PORT;
}

SessionDesc::SessionDesc(const char *name, const char *pass, ENetAddress *address)
{
	id = 0;
	strncpy(session_name, name, MP_FRIENDLY_NAME_LEN);
	session_name[MP_FRIENDLY_NAME_LEN] = 0;
	strncpy(password, pass, MP_FRIENDLY_NAME_LEN);
	password[MP_FRIENDLY_NAME_LEN] = 0;
	this->address.host = address->host;
	this->address.port = address->port;
}

SessionDesc::SessionDesc(const SessionDesc &src)
{
	id = src.id;
	strncpy(session_name, src.session_name, MP_FRIENDLY_NAME_LEN);
	session_name[MP_FRIENDLY_NAME_LEN] = 0;
	strncpy(password, src.password, MP_FRIENDLY_NAME_LEN);
	password[MP_FRIENDLY_NAME_LEN] = 0;
	address.host = src.address.host;
	address.port = src.address.port;
}

SessionDesc& SessionDesc::operator= (const SessionDesc &src)
{
	id = src.id;
	strncpy(session_name, src.session_name, MP_FRIENDLY_NAME_LEN);
	session_name[MP_FRIENDLY_NAME_LEN] = 0;
	strncpy(password, src.password, MP_FRIENDLY_NAME_LEN);
	password[MP_FRIENDLY_NAME_LEN] = 0;
	address.host = src.address.host;
	address.port = src.address.port;
	return *this;
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
	host_flag = 0;
	allowing_connections = 0;
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
	host_flag = 0;
	max_players = 0;
	use_remote_session_provider = 0;
	update_available = -1;
	host = NULL;
	packet_mode = ENET_PACKET_FLAG_RELIABLE;

	if (!is_protocol_supported(protocol_type)) {
		ERR("[MultiPlayer::init] trying to init unsupported protocol\n");
		return;
	}

	if (enet_initialize() != 0) {
		ERR("Could not init the enet library.\n");
		return;
	}

	if (enet_address_set_host(&lan_broadcast_address,
		"255.255.255.255") != 0) {
		return;
	}
	lan_broadcast_address.port = UDP_GAME_PORT;

	for (int i = 0; i < MAX_NATION; i++) {
		player_pool[i] = NULL;
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

	close_session();
	current_sessions.zap();
	enet_deinitialize();

	if (recv_buf != NULL) {
		delete [] recv_buf;
		recv_buf = NULL;
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

// Open a multiplayer game port to enable the network service. If successful,
// returns 1. If not successful, returns 0. If fallback is set, then it will
// try a second time using any port.
int MultiPlayer::open_port(uint16_t port, int fallback)
{
	ENetAddress address;

	err_when(host != NULL);

	address.host = ENET_HOST_ANY;
	address.port = port;

	host = enet_host_create(
		&address,
		MAX_NATION,
		2,
		0,
		0
	);
	if (host == NULL) {
		if (!fallback)
			return 0;

		return open_port(0, 0);
	}
	MSG("Opened port %hu\n", host->address.port);

	return 1;
}

int MultiPlayer::set_remote_session_provider(const char *server)
{
	if (enet_address_set_host(
			&remote_session_provider_address,
			server) == 0) {
		remote_session_provider_address.port = UDP_GAME_PORT;
		use_remote_session_provider = 1;
	} else {
		use_remote_session_provider = 0;
	}

	return use_remote_session_provider;
}

int MultiPlayer::poll_sessions()
{
	err_when(!init_flag);

	if (!open_port(UDP_GAME_PORT, 0)) {
		MSG("Cannot open port %d, unable to scan lan.\n", UDP_GAME_PORT);
		return 0;
	}

	current_sessions.zap();

	// Watch for a game beacon broadcast

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
	err_when(!init_flag || maxPlayers <= 0 || maxPlayers > MAX_NATION);

#ifdef HOST_ANY_PORT
	if (!open_port(0, 0)) {
#else
	if (!open_port(UDP_GAME_PORT, 1)) {
#endif
		MSG("Unable to open a port for the session.\n");
		return 0;
	}

	joined_session.id = 0;
	strcpy(joined_session.session_name, sessionName);
	strcpy(joined_session.password, password);

	host_flag = 1;
	max_players = maxPlayers;
	allowing_connections = 1;

	// Add hosts machine's player to the pool now
	err_when(player_pool[0] != NULL);
	my_player = new PlayerDesc(1, playerName);
	set_my_player_id(1);

	return 1;
}

// join a session
// note : do not call poll_sessions between get_session and join_session
//
int MultiPlayer::join_session(SessionDesc *session, char *playerName)
{
	ENetPeer *peer;

	err_when(session == NULL);

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
	if (peer == NULL) {
		return 0;
	}

	max_players = MAX_NATION;
	allowing_connections = 1;

	// register the host now, even though his name is not known yet
	err_when(player_pool[0] != NULL);
	player_pool[0] = new PlayerDesc(1, &session->address);
	peer->data = player_pool[0];

	// create this player locally, we will put it in the right place
	// later
	my_player = new PlayerDesc(playerName);

	joined_session = *session;

	return 1;
}

// Call close_session when leaving any session.
void MultiPlayer::close_session()
{
	if (host_flag) {
		int i;

		// disconnect all clients
		for (i = 0; i < max_players; i++) {
			delete_player(i+1);
		}

		host_flag = 0;
	}
	allowing_connections = 0;

	my_player_id = 0;
	my_player = NULL;

	if (host != NULL) {
		enet_host_destroy(host);
		host = NULL;
	}
}

void MultiPlayer::game_starting()
{
	packet_mode = ENET_PACKET_FLAG_UNSEQUENCED;
}

void MultiPlayer::disable_new_connections()
{
	allowing_connections = 0;
}

// Create a player and add to the pool.
//
// This is only called by the host upon connection from a client. The host
// chooses the player's id.
//
// Returns the player pointer when added to the pool, and NULL if the player
// wasn't added to the pool.
//
PlayerDesc *MultiPlayer::create_player(ENetAddress *address)
{
	int i;

	// search for an empty slot
	for (i = 0; i < max_players; i++)
		if (player_pool[i] == NULL)
			break;
	if (i >= max_players)
		return NULL;

	player_pool[i] = new PlayerDesc(i+1, address);

	return player_pool[i];
}

// Adds a player that is not already connected. Otherwise sets the player's
// name.
//
// Returns 1 when the player's name was set.
// Returns 0 when the player's name could not be set.
int MultiPlayer::add_player(uint32_t id, char *name, ENetAddress *address, char contact)
{
	if (id < 1 || id > max_players) {
		return 0;
	}

	if (player_pool[id-1] == NULL && address->host != ENET_HOST_ANY) {
		ENetPeer *peer;

		peer = NULL;
		if (contact) {
			peer = enet_host_connect(host,
				address,
				1,
				0);
			if (peer == NULL) {
				return 0;
			}

			peer->data = player_pool[id-1];
		}

		player_pool[id-1] = new PlayerDesc(id, address);
	}

	strncpy(player_pool[id-1]->name, name, MP_FRIENDLY_NAME_LEN);
	player_pool[id-1]->name[MP_FRIENDLY_NAME_LEN] = 0;

	MSG("Player '%s' (%d) recognized.\n", player_pool[id-1]->name, id);

	return 1;
}

// Called when a player is identifying himself. The game organizer validates
// the password only, as the password is only sent to the organizer.
//
// Returns 1 when a player is newly authorized.
// Returns 0 when nothing changed.
int MultiPlayer::auth_player(uint32_t id, char *name, char *password)
{
	if (id < 1 || id > max_players) {
		return 0;
	}

	if (player_pool[id-1] == NULL) {
		return 0;
	}

	if (player_pool[id-1]->authorized) {
		return 0;
	}

	if (host_flag && memcmp(password,
			joined_session.password,
			MP_FRIENDLY_NAME_LEN)) {
		MSG("Player '%s' (%d) password is incorrect.\n", player_pool[id-1]->name, id);
		return 0;
	}

	strncpy(player_pool[id-1]->name, name, MP_FRIENDLY_NAME_LEN+1);
        player_pool[id-1]->name[MP_FRIENDLY_NAME_LEN] = 0;
	player_pool[id-1]->authorized = 1;
	MSG("Player '%s' (%d) was authorized.\n", player_pool[id-1]->name, id);

	return 1;
}

// Set my_player_id if not already set.
// Returns 1 if it is newly set.
// Returns 0 if it is cannot be set.
int MultiPlayer::set_my_player_id(uint32_t id)
{
	if (id < 1 || id > max_players)
		return 0;

	if (my_player_id)
		return 0;

	my_player_id = id;
	player_pool[id-1] = my_player;
	my_player->connecting = 1;

	MSG("You have been assigned id=%d\n", id);

	return 1;
}

void MultiPlayer::set_player_name(uint32_t id, char *name)
{
	err_when(!player_pool[id-1]);
	strncpy(player_pool[id-1]->name, name, MP_FRIENDLY_NAME_LEN);
}

// Deletes a player from the pool
void MultiPlayer::delete_player(uint32_t id)
{
	ENetPeer *peer;

	err_when(host == NULL);

	if (id < 1 || id > max_players)
		return;

	peer = get_peer(id);

	if (peer != NULL) {
		enet_peer_disconnect_now(peer, 0);
		peer->data = NULL;
	}

	if (player_pool[id-1] == NULL)
		return;

	MSG("Player '%s' (%d) deleted\n", player_pool[id-1]->name, id);

	delete player_pool[id-1];
	player_pool[id-1] = NULL;
}

void MultiPlayer::poll_players()
{
}

PlayerDesc *MultiPlayer::get_player(int i)
{
	if (i < 1 || i > max_players)
		return NULL;

	return player_pool[i-1];
}

PlayerDesc *MultiPlayer::get_player(ENetAddress *address)
{
	int i;

	for (i = 0; i < max_players; i++) {
		if (player_pool[i] == NULL) {
			continue;
		}
		if (player_pool[i]->address.host == address->host &&
			player_pool[i]->address.port == address->port) {
			break;
		}
	}

	if (i >= max_players)
		return NULL;

	return player_pool[i];
}

PlayerDesc *MultiPlayer::search_player(uint32_t playerId)
{
	if (playerId < 1 || playerId > max_players)
		return NULL;
	return player_pool[playerId-1];
}

ENetPeer *MultiPlayer::get_peer(uint32_t id)
{
	ENetPeer *peer;

	for (peer = host->peers; peer < &host->peers[host->peerCount]; ++peer) {
		if (peer->data != NULL) {
			PlayerDesc *player;

			player = (PlayerDesc *)peer->data;

			if (player->id == id)
				break;
		}
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
	if (playerId < 1 || playerId > max_players || !player_pool[playerId-1])
		return 0;
	return player_pool[playerId-1]->connecting;
}

int MultiPlayer::get_player_count()
{
	int count = 0;
	for (int i = 0; i < max_players; i++)
		if (player_pool[i] && player_pool[i]->connecting)
			count++;
	return count;
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

	err_when(to > max_players);

	if (to == my_player_id) {
		return 0;
	}

	if (msg_size > MP_UDP_MAX_PACKET_SIZE) {
		ERR("Packet message exceeds maximum size.\n");
		return 0;
	}

	if (to != BROADCAST_PID && player_pool[to-1] == NULL) {
		ERR("Player %d does not exist.\n", to);
		return 0;
	}

	packet = enet_packet_create(
		data,
		msg_size,
		packet_mode
	);

	if (to == BROADCAST_PID) {
		enet_host_broadcast(host, 0, packet);
	} else {
		ENetPeer *peer;

		peer = get_peer(to);

		if (peer == NULL)
			return 0;

		enet_peer_send(peer, 0, packet);
	}

	return 1;
}

// receive udp message
//
// return NULL if fails
// sysMsgCount records how many system messages have been handled
// notice : *sysMsgCount may be != 0, but return NULL
//
char *MultiPlayer::receive(uint32_t *from, uint32_t *size, int *sysMsgCount)
{
	int ret;
	ENetEvent event;
	PlayerDesc *player;
	char *got_recv;

	if (sysMsgCount)
		*sysMsgCount = 0;

	ret = enet_host_service(host, &event, 0);
	if (ret < 0) {
		err_now("enet_host_service");
	} else if (ret == 0) {
		return NULL;
	}

	got_recv = NULL;

	player = (PlayerDesc *)event.peer->data;
	if (player == NULL) {
		// The player may actually exist via add_player, sync enet data
		// by performing a lookup.
		player = get_player(&event.peer->address);
		event.peer->data = player;
	}

	if (player != NULL) {
		*from = player->pid();
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
		(*sysMsgCount)++;

		if (player == NULL) {
			if (!allowing_connections) {
				enet_peer_disconnect_now(event.peer, 0);
				break;
			}
			if (host_flag) {
				player = create_player(&event.peer->address);
				if (player == NULL) {
					enet_peer_disconnect_now(event.peer, 0);
					break;
				}
			}
		}

		if (player != NULL) {
			event.peer->data = player;
			MSG("Player '%s' (%d) connected.\n", player->name, player->id);
			MSG("Number of connections: %d\n", host->connectedPeers);
		}

		break;

	case ENET_EVENT_TYPE_DISCONNECT:
		(*sysMsgCount)++;

		if (player != NULL) {
			player->connecting = 0;
			MSG("Player '%s' (%d) disconnected. (fixme)\n", player->name, player->id);
			MSG("Number of connections: %d\n", host->connectedPeers);
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
