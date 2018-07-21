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
	service_provider.host = ENET_HOST_ANY;
	misc.uuid_clear(service_login_id);
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
	if (my_player) {
		delete my_player;
		my_player = NULL;
	}

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

		if (enet_address_set_host(&address, cmdLine) != 0) {
			return 0;
		}
		address.port = UDP_GAME_PORT;

		guuid_t sessionId;
		misc.uuid_generate_random(sessionId);
		SessionDesc session("Lobbied Game", sessionId, SessionFlags::Pregame, address);
		strcpy(session.password, "1");
		session.max_players = maxPlayers;

		current_sessions.linkin(&session);

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

int MultiPlayer::set_service_provider(const char *host)
{
	if (host) {
		enet_address_set_host(&service_provider, host);
		service_provider.port = UDP_MONITOR_PORT;
	} else {
		service_provider.host = ENET_HOST_ANY;
	}

	return service_provider.host != ENET_HOST_ANY;
}

int MultiPlayer::poll_sessions()
{
	ENetAddress a;
	ENetBuffer b;
	int ret;
	int login_failed;

	err_when(!init_flag);

	if (session_monitor == ENET_SOCKET_NULL) {
		return MP_POLL_NO_SOCKET;
	}

	b.data = recv_buf;
	b.dataLength = MP_RECV_BUFFER_SIZE;

	ret = MP_POLL_NO_UPDATE;
	login_failed = 0;
	while (enet_socket_receive(session_monitor, &a, &b, 1)>0) {
		uint32_t id = *(uint32_t *)recv_buf;
		ret = MP_POLL_UPDATE;
		switch (id) {
		case MPMSG_USER_SESSION_STATUS: {
			MpMsgUserSessionStatus *m = (MpMsgUserSessionStatus *)recv_buf;

			if ((m->flags & (SessionFlags::Hosting|SessionFlags::Pregame)) == 0)
				break;
			if (get_session(&a))
				break;

			SessionDesc desc(m->session_name, m->session_id, m->flags, a);
			current_sessions.linkin(&desc);

			break;
			}
		case MPMSG_LOGIN_ID: {
			MpMsgLoginId *m = (MpMsgLoginId*)recv_buf;

			if (misc.uuid_is_null(m->login_id)) {
				login_failed = 1;
				break;
			}
			if (a.host == service_provider.host)
				misc.uuid_copy(service_login_id, m->login_id);

			break;
			}
		case MPMSG_SESSION: {
			MpMsgSession *m = (MpMsgSession*)recv_buf;

			if (a.host == service_provider.host) {
				SessionDesc *prev;
				if (prev = get_session(m->session_id)) {
					prev->flags = m->flags;
					break;
				}

				ENetAddress anyAddress;
				anyAddress.host = ENET_HOST_ANY;
				anyAddress.port = ENET_PORT_ANY;
				SessionDesc desc(m->session_name, m->session_id, m->flags, anyAddress);
				current_sessions.linkin(&desc);
			}

			break;
			}
		}
	}

	if (service_provider.host != ENET_HOST_ANY) {
		if (misc.uuid_is_null(service_login_id)) {
			if (login_failed)
				return MP_POLL_LOGIN_FAILED;
			send_req_login_id();
			return MP_POLL_LOGIN_PENDING;
		}
		send_poll_sessions();
	}

	return ret;
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

SessionDesc *MultiPlayer::get_session(guuid_t id)
{
	int i;
	for (i = 1; i <= current_sessions.size(); i++) {
		SessionDesc *p = (SessionDesc *)current_sessions.get(i);
		if (p && !misc.uuid_compare(p->session_id, id))
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
// <int>    maxPlayers       maximum no. of players in a session
//
// return 1 if success
int MultiPlayer::create_session(char *sessionName, char *password, int maxPlayers)
{
	const char *name;

	err_when(!init_flag || maxPlayers <= 0 || maxPlayers > MAX_NATION || host);

#ifdef HOST_ANY_PORT
	if (!open_port(0, 0)) {
#else
	if (!open_port(UDP_GAME_PORT, 1)) {
#endif
		MSG("Unable to open a port for the session.\n");
		return 0;
	}

	if (strlen(sessionName))
		name = sessionName;
	else
		name = "?Anonymous?";
	strcpy(joined_session.session_name, name);
	strcpy(joined_session.password, password);
	joined_session.max_players = maxPlayers;
	joined_session.player_count = 1;
	joined_session.flags |= SessionFlags::Hosting | SessionFlags::Pregame;
	if (password[0])
		joined_session.flags |= SessionFlags::Password;
	if (joined_session.player_count >= joined_session.max_players)
		joined_session.flags |= SessionFlags::Full;
	if (service_provider.host != ENET_HOST_ANY)
		misc.uuid_clear(joined_session.session_id); // generated by service
	else
		misc.uuid_generate_random(joined_session.session_id); // generated for LAN only
	joined_session.address.host = host->address.host;
	joined_session.address.port = host->address.port;

	set_my_player_id(1);
	player_pool[0] = my_player; // can we skip polling players?

	return 1;
}

int MultiPlayer::connect_host()
{
	ENetPeer *peer;
	PlayerDesc *game_host;

	err_when(!host);

	// If host is not known, we will get it from the service later.
	if (joined_session.address.host == ENET_HOST_ANY)
		return 1;

	// The host is known, connect now.
	peer = enet_host_connect(host, &joined_session.address, 1, 0);
	if (!peer)
		return 0;

	game_host = new PlayerDesc(&joined_session.address);
	game_host->id = 1;
	game_host->authorized = 1;
	peer->data = game_host;

	return 1;
}

// join a session
// note : do not call poll_sessions between get_session and join_session
//
int MultiPlayer::join_session(SessionDesc *session)
{
	err_when(!session || host);

	if (!open_port(0, 0)) {
		MSG("Unable to open a port for the session.\n");
		return 0;
	}

	joined_session = *session;
	joined_session.flags &= ~SessionFlags::Hosting; // we're not the host
	connect_host();

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

	joined_session.flags &= ~SessionFlags::Pregame;

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
		update_player_pool();
		return 1;
	}
	if (contact) {
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
		// NAT punch for incoming connection
		send_ping(host->socket, address);
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
	err_when(!(joined_session.flags & SessionFlags::Hosting));

	peer = get_peer(playerId);
	if (!peer || !peer->data) {
		return 0;
	}
	player = (PlayerDesc *)peer->data;

	if ((joined_session.flags & SessionFlags::Password) && memcmp(password, joined_session.password, MP_FRIENDLY_NAME_LEN)) {
		MSG("Player (%d) password is incorrect.\n", playerId);
		return 0;
	}

	strncpy(player->name, name, MP_FRIENDLY_NAME_LEN+1);
	player->name[MP_FRIENDLY_NAME_LEN] = 0;
	player->authorized = 1;

	MSG("Player '%s' (%d) was authorized.\n", player->name, playerId);
	update_player_pool();

	return 1;
}

void MultiPlayer::create_my_player(char *playerName)
{
	const char *name;
	if (strlen(playerName))
		name = playerName;
	else
		name = "?Anonymous?";
	if (my_player)
		strcpy(my_player->name, name);
	else
		my_player = new PlayerDesc(name);
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
		update_player_pool();
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

int MultiPlayer::poll_players()
{
	static unsigned long last_broadcast_time;
	static int poll_time;
	unsigned long current_time;
	int ret;

	// periodically broadcast status
	current_time = misc.get_time();
	ret = MP_POLL_NO_UPDATE;
	if (current_time > last_broadcast_time + poll_time || current_time < last_broadcast_time) {
		ENetAddress a;
		last_broadcast_time = current_time;

		// broadcast status to lan
		a.host = ENET_HOST_BROADCAST;
		a.port = UDP_MONITOR_PORT;
		send_user_session_status(&a);

		poll_time = 5000;
		if (service_provider.host != ENET_HOST_ANY) {
			// communicate with service provider
			ENetBuffer b;

			if (joined_session.flags & SessionFlags::Hosting) {
				if (misc.uuid_is_null(joined_session.session_id)) {
					send_req_session_id();
					poll_time = 300;
					ret = MP_POLL_LOGIN_PENDING;
				} else {
					// try to keep NAT forwarding alive from server to game host
					send_service_ping();
				}
			} else if (joined_session.address.host == ENET_HOST_ANY) {
				send_req_session_addr();
				poll_time = 300;
			}

			b.data = recv_buf;
			b.dataLength = MP_RECV_BUFFER_SIZE;

			while (enet_socket_receive(session_monitor, &a, &b, 1)>0) {
				uint32_t id = *(uint32_t *)recv_buf;
				switch (id) {
				case MPMSG_LOGIN_ID: {
					MpMsgLoginId *m = (MpMsgLoginId*)recv_buf;

					if (misc.uuid_is_null(m->login_id)) {
						ret = MP_POLL_LOGIN_FAILED;
						break;
					}

					if (a.host == service_provider.host)
						misc.uuid_copy(service_login_id, m->login_id);

					break;
					}
				case MPMSG_SESSION_ID: {
					MpMsgSessionId *m = (MpMsgSessionId*)recv_buf;

					if (a.host != service_provider.host)
						break;

					misc.uuid_copy(joined_session.session_id, m->session_id);
					break;
					}
				case MPMSG_SESSION_ADDR: {
					MpMsgSessionAddr *m = (MpMsgSessionAddr*)recv_buf;

					if (joined_session.address.host != ENET_HOST_ANY)
						break;
					if (misc.uuid_is_null(m->session_id)) {
						ret = MP_POLL_NO_SESSION;
						break;
					}
					if (a.host != service_provider.host)
						break;
					if (misc.uuid_compare(joined_session.session_id, m->session_id))
						break;

					joined_session.address.host = m->host;
					joined_session.address.port = m->port;
					connect_host();

					break;
					}
				case MPMSG_HOST_NAT_PUNCH: {
					MpMsgHostNatPunch *m = (MpMsgHostNatPunch*)recv_buf;

					if (a.host != service_provider.host)
						break;
					if (!(joined_session.flags & SessionFlags::Hosting))
						break;

					do_host_nat_punch(m);
					break;
					}
				}
			}

			if (misc.uuid_is_null(service_login_id)) {
				send_req_login_id();
				poll_time = 300;
			} else {
				send_user_session_status(&service_provider);
			}

			if (joined_session.address.host != ENET_HOST_ANY &&
				joined_session.player_count < 2) {
				send_req_host_nat_punch();
				poll_time = 300;
			}
		}
	}

	return ret;
}

void MultiPlayer::update_player_pool()
{
	ENetPeer *peer;
	int i;

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
		joined_session.flags |= SessionFlags::Full;
	else
		joined_session.flags &= ~SessionFlags::Full;

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
	if (misc.uuid_is_null(joined_session.session_id))
		return;

	b.data = &m;
	b.dataLength = sizeof(MpMsgUserSessionStatus);

	m.msg_id = MPMSG_USER_SESSION_STATUS;
	if (a->host == service_provider.host) {
		misc.uuid_copy(m.login_id, service_login_id);
	} else {
		misc.uuid_clear(m.login_id);
	}
	misc.uuid_copy(m.session_id, joined_session.session_id);
	m.player_id = my_player_id;
	m.flags = joined_session.flags;
	strncpy(m.session_name, joined_session.session_name, MP_FRIENDLY_NAME_LEN);

	enet_socket_send(host->socket, a, &b, 1);
}

void MultiPlayer::send_req_login_id()
{
	ENetBuffer b;
	MpMsgReqLoginId m;

	b.data = &m;
	b.dataLength = sizeof(MpMsgReqLoginId);

	m.msg_id = MPMSG_REQ_LOGIN_ID;
	strncpy(m.name, my_player->name, MP_FRIENDLY_NAME_LEN);

	enet_socket_send(session_monitor, &service_provider, &b, 1);
}

void MultiPlayer::send_poll_sessions()
{
	ENetBuffer b;
	MpMsgPollSessions m;

	b.data = &m;
	b.dataLength = sizeof(MpMsgPollSessions);

	m.msg_id = MPMSG_POLL_SESSIONS;
	misc.uuid_copy(m.login_id, service_login_id);

	enet_socket_send(session_monitor, &service_provider, &b, 1);
}

void MultiPlayer::send_req_session_id()
{
	ENetBuffer b;
	MpMsgReqSessionId m;

	if (misc.uuid_is_null(service_login_id))
		return;

	b.data = &m;
	b.dataLength = sizeof(MpMsgReqSessionId);

	m.msg_id = MPMSG_REQ_SESSION_ID;
	misc.uuid_copy(m.login_id, service_login_id);
	memcpy(m.session_name, joined_session.session_name, MP_FRIENDLY_NAME_LEN);
	memcpy(m.session_password, joined_session.password, MP_FRIENDLY_NAME_LEN);

	enet_socket_send(session_monitor, &service_provider, &b, 1);
}

void MultiPlayer::send_req_session_addr()
{
	ENetBuffer b;
	MpMsgReqSessionAddr m;

	if (misc.uuid_is_null(service_login_id))
		return;

	b.data = &m;
	b.dataLength = sizeof(MpMsgReqSessionAddr);

	m.msg_id = MPMSG_REQ_SESSION_ADDR;
	misc.uuid_copy(m.login_id, service_login_id);
	misc.uuid_copy(m.session_id, joined_session.session_id);
	memcpy(m.session_password, joined_session.password, MP_FRIENDLY_NAME_LEN);

	enet_socket_send(session_monitor, &service_provider, &b, 1);
}

void MultiPlayer::send_ping(ENetSocket s, ENetAddress *a)
{
	ENetBuffer b;
	MpMsgPing m;

	if (!host)
		return;
	if (misc.uuid_is_null(joined_session.session_id))
		return;

	b.data = &m;
	b.dataLength = sizeof(MpMsgPing);

	m.msg_id = MPMSG_PING;

	enet_socket_send(s, a, &b, 1);
}

void MultiPlayer::send_req_host_nat_punch()
{
	ENetBuffer b;
	MpMsgReqHostNatPunch m;

	if (!host)
		return;
	if (misc.uuid_is_null(joined_session.session_id))
		return;
	if (misc.uuid_is_null(service_login_id))
		return;

	b.data = &m;
	b.dataLength = sizeof(MpMsgReqHostNatPunch);

	m.msg_id = MPMSG_REQ_HOST_NAT_PUNCH;
	misc.uuid_copy(m.login_id, service_login_id);
	misc.uuid_copy(m.session_id, joined_session.session_id);
	memcpy(m.session_password, joined_session.password, MP_FRIENDLY_NAME_LEN);

	enet_socket_send(host->socket, &service_provider, &b, 1);
}

void MultiPlayer::send_service_ping()
{
	if (session_monitor == ENET_SOCKET_NULL)
		return;

	send_ping(session_monitor, &service_provider);
}

void MultiPlayer::do_host_nat_punch(MpMsgHostNatPunch *in)
{
	ENetAddress a;
	ENetBuffer b;
	MpMsgPing m;

	if (!host)
		return;

	a.host = in->host;
	a.port = in->port;

	send_ping(host->socket, &a);
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
	err_when(!host);

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
			if (!(joined_session.flags & SessionFlags::Pregame)) {
				enet_peer_disconnect(event.peer, 0);
				break;
			}
			player = new PlayerDesc(&event.peer->address);
			if (joined_session.flags & SessionFlags::Hosting) {
				player->id = get_avail_player_id();
			}
		}

		if (player) {
			MSG("Player '%s' (%d) connected.\n", player->name, player->id);
			*from = player->id;
			event.peer->data = player;
			update_player_pool();
		}

		break;

	case ENET_EVENT_TYPE_DISCONNECT:
		if (sysMsgCount)
			*sysMsgCount = -1;
		MSG("ENET_EVENT_TYPE_DISCONNECT connectedPeers=%d\n", host->connectedPeers);

		if (player) {
			MSG("Player '%s' (%d) disconnected.\n", player->name, player->id);
			if (joined_session.flags & SessionFlags::Hosting) {
				delete player;
			} else {
				// try to save in case of a reconnection or host acknowledgement
				if (!player->authorized || !add_pending_player(player)) {
					delete player;
				}
			}
			event.peer->data = NULL;
			update_player_pool();
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
	return strcmp( ((SessionDesc *)a)->session_name, ((SessionDesc *)b)->session_name );
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
