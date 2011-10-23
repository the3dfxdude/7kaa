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

// Filename    : multiplayer.cpp
// Description : Multiplayer game support.

#include <multiplayer.h>
#include <ALL.h>
#include <string.h>
#include <OVGALOCK.h>
#include <OBLOB.h>
#include <stdint.h>
#include <dbglog.h>
#include <vga_util.h>
#include <OFONT.h>
#include <OMOUSE.h>

DBGLOG_DEFAULT_CHANNEL(MultiPlayer);

#define MP_UDP_MAX_PACKET_SIZE 800

const uint16_t UDP_GAME_PORT = 19255;


SessionDesc::SessionDesc()
{
	id = 0;
	session_name[0] = '\0';
	password[0] = '\0';
	memset(&address, 0, sizeof(struct inet_address));
}

SessionDesc::SessionDesc(const SessionDesc &src)
{
	id = src.id;
	strcpy(session_name, src.session_name);
	strcpy(password, src.password);
	memcpy(&address, &src.address, sizeof(struct inet_address));
}

SessionDesc& SessionDesc::operator= (const SessionDesc &src)
{
	id = src.id;
	strcpy(session_name, src.session_name);
	strcpy(password, src.password);
	memcpy(&address, &src.address, sizeof(struct inet_address));
	return *this;
}

// to start a multiplayer game, first check if it is called from a
// lobbied (MultiPlayer::is_lobbied)

// if it is a lobbied, call init_lobbied

// if not, call poll_service_provider; display them and let
// user to select, call init and pass the guid of the selected
// service; create_session or poll_sessions+join_session;

MultiPlayer::MultiPlayer() :
	current_sessions(sizeof(SessionDesc), 10 )
{
	init_flag = 0;
	lobbied_flag = 0;
	supported_protocols = TCPIP;
	my_player_id = 0;
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
	host_flag = 0;
	max_players = 0;
	use_remote_session_provider = 0;
	update_available = -1;
	network = new Network();
	game_sock = 0;
	standard_port = 0;
	status = MP_STATUS_IDLE;

	if (!is_protocol_supported(protocol_type)) {
		ERR("[MultiPlayer::init] trying to init unsupported protocol\n");
		return;
	}

	if (!network->init())
	{
		ERR("Could not init the network subsystem.\n");
		return;
	}

	network->resolve_host(&lan_broadcast_address, "255.255.255.255", UDP_GAME_PORT);

	for (int i = 0; i < MAX_NATION; i++) {
		player_pool[i] = NULL;
	}

	recv_buf = new char[MP_RECV_BUFFER_SIZE];

	init_flag = 1;
}

void MultiPlayer::deinit()
{
	int i;

	if (host_flag) {
		// disconnect all clients
		for (i = 0; i < max_players; i++) {
			delete_player(i+1);
		}

		host_flag = 0;
		allowing_connections = 0;
	}
	if (recv_buf) {
		delete [] recv_buf;
		recv_buf = NULL;
	}

	delete network;
	network = NULL;

	current_sessions.zap();
	init_flag = 0;
	lobbied_flag = 0;
	my_player_id = 0;
	status = MP_STATUS_IDLE;
}

// init_lobbied
// Reads the command line and sets lobby mode if the command line is correct.
// Returns non-zero on success.
int MultiPlayer::init_lobbied(int maxPlayers, char *cmdLine)
{
	MSG("[MultiPlayer::init_lobbied] %d, %s\n", maxPlayers, cmdLine);
	if (cmdLine) {
		SessionDesc *session = new SessionDesc();

		strcpy(session->session_name, "Lobbied Game");
		session->password[0] = 1;
		if (!network->resolve_host(&session->address, cmdLine, UDP_GAME_PORT))
		{
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

int MultiPlayer::is_pregame()
{
	return status == MP_STATUS_PREGAME;
}

// open game socket on any port
int MultiPlayer::open_port()
{
	if (game_sock)
	{
		return 1;
	}
	standard_port = 0;
	game_sock = network->udp_open(0);
	return game_sock != 0;
}

// open game socket on the standard port
// Fallback will allow whether you can fallback on random port if the standard
// port is not available. If the standard port is already open, the we don't
// need to do anything. If a non standard port is open, then we close that,
// and open a new socket. The standard port number is defined by UDP_GAME_PORT.
// returns 1 on success, 0 on failure
int MultiPlayer::open_standard_port(int fallback)
{
	if (game_sock)
	{
		if (standard_port)
		{
			return 1;
		}
		network->udp_close(game_sock);
	}
	game_sock = network->udp_open(UDP_GAME_PORT);
	if (!game_sock)
	{
		if (fallback)
		{
			return open_port();
		}
		return 0;
	}
	standard_port = 1;
	return 1;
}

int MultiPlayer::check_duplicates(struct inet_address *address)
{
	int i;
	for (i = 0; i < current_sessions.size(); i++)
	{
		SessionDesc *desc;

		desc = (SessionDesc *)current_sessions.get(i+1);
		if (!desc)
			return 0;
		if (desc->address.host == address->host &&
		    desc->address.port == address->port)
			return 1;
	}
	return 0;
}

int MultiPlayer::set_remote_session_provider(const char *server)
{
	use_remote_session_provider = network->resolve_host(&remote_session_provider_address, server, UDP_GAME_PORT);
	return use_remote_session_provider;
}

void MultiPlayer::msg_game_beacon(MsgGameBeacon *m, struct inet_address *addr)
{
	SessionDesc *desc;

	if (check_duplicates(addr))
		return;

	desc = new SessionDesc();

	strncpy(desc->session_name, m->name, MP_SESSION_NAME_LEN);
	desc->session_name[MP_SESSION_NAME_LEN] = 0;
	desc->password[0] = m->password;
	desc->address.host = addr->host;
	desc->address.port = addr->port;
	desc->id = current_sessions.size();
	current_sessions.linkin(desc);

	MSG("[MultiPlayer::poll_sessions] got beacon for game '%s'\n", desc->session_name);
}

// returns the next ack
int MultiPlayer::msg_game_list(MsgGameList *m, int last_ack, struct inet_address *addr)
{
	SessionDesc *desc;
	int i;

	// only allow this message from a trusted provider
	if (addr->host != remote_session_provider_address.host ||
		addr->port != remote_session_provider_address.port) {
		return last_ack;
	}

	for (i = 0; i < MP_GAME_LIST_SIZE; i++) {
		struct inet_address addy;

		if (!m->list[i].host) {
			continue;
		}

		addy.host = m->list[i].host;
		addy.port = m->list[i].port;
		if (check_duplicates(&addy)) {
			continue;
		}

		desc = new SessionDesc();

		strncpy(desc->session_name, m->list[i].name, MP_SESSION_NAME_LEN);
		desc->session_name[MP_SESSION_NAME_LEN] = 0;
		desc->password[0] = m->list[i].password;
		desc->address.host = addy.host;
		desc->address.port = addy.port;
		desc->id = current_sessions.size();
		current_sessions.linkin(desc);

		MSG("[MultiPlayer::poll_sessions] got beacon for game '%s'\n", desc->session_name);
	}

	if (m->total_pages >= last_ack)
		return 1;

	return last_ack++;
}

void MultiPlayer::msg_version_nak(MsgVersionNak *m, struct inet_address *addr)
{
	if (update_available > -1)
		return;

	// only allow this message from a trusted provider
	if (addr->host != remote_session_provider_address.host ||
		addr->port != remote_session_provider_address.port)
		return;

	if (m->major > SKVERMAJ)
	{
		update_available = 1;
		return;
	}
	if (m->medium > SKVERMED)
	{
		update_available = 1;
		return;
	}
	if (m->minor > SKVERMIN)
	{
		update_available = 1;
		return;
	}

	update_available = 0;
}

int MultiPlayer::poll_sessions()
{
	static int ack_num = 1;
	static int attempts = 0;
	struct packet_header *h;
	int ret;

	err_when(!init_flag);

	if (!open_standard_port(0))
	{
		MSG("Cannot open port %d, unable to scan lan.\n", UDP_GAME_PORT);
		return 0;
	}

	current_sessions.zap();

	h = (struct packet_header *)recv_buf;
	h->size = MP_UDP_MAX_PACKET_SIZE;

	while (1) {
		struct inet_address addr;
		struct MsgHeader *p;
		int ack;

		p = (struct MsgHeader *)(recv_buf+sizeof(struct packet_header));

		ret = network->recv(game_sock, h, &addr);
		if (ret <= 0)
			break;

		switch (p->msg_id)
		{
		case MPMSG_GAME_BEACON:
			msg_game_beacon((MsgGameBeacon *)p, &addr);
			break;
		case MPMSG_GAME_LIST:
			ack = msg_game_list((MsgGameList *)p, ack_num, &addr);
			if (ack != ack_num) {
				attempts = 0;
				ack_num = ack;
			}
			break;
		case MPMSG_VERSION_NAK:
			msg_version_nak((MsgVersionNak *)p, &addr);
			break;
		default:
			MSG("received unhandled message %u\n", p->msg_id);
		}

	}

	if (use_remote_session_provider)
	{
		struct MsgRequestGameList m;

 		if (attempts > 10)
			ack_num = 1;
		attempts++;

		m.msg_id = MPMSG_REQ_GAME_LIST;
		m.ack = ack_num;

		send_nonseq_msg(game_sock, (char *)&m, sizeof(struct MsgRequestGameList), &remote_session_provider_address);

		if (update_available < 0)
		{
			struct MsgVersionAck n;

			n.msg_id = MPMSG_VERSION_ACK;

			send_nonseq_msg(game_sock, (char *)&n, sizeof(struct MsgVersionAck), &remote_session_provider_address);
		}
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

	// open socket for listening
	if (!open_standard_port(1))
	{
		MSG("Unable to get the game socket.\n");
		return 0;
	}

	joined_session.id = 0;
	strcpy(joined_session.session_name, sessionName);
	strcpy(joined_session.password, password);

	host_flag = 1;
	max_players = maxPlayers;
	allowing_connections = 1;

	// Add hosts machine's player to the pool now
	if (!add_player(playerName, 1)) {
		return 0;
	}
	set_my_player_id(1);

	status = MP_STATUS_PREGAME;

	return 1;
}

// join a session
// note : do not call poll_sessions between get_session and join_session
//
// <int> i -- the index from get_session()
// <char *> playerName -- the name the player will be known by
// <char *> password -- allows entering password for the session
//
int MultiPlayer::join_session(int i, char *password, char *playerName)
{
	SessionDesc *session = (SessionDesc *)current_sessions.get(i);
	if (!session)
		return 0;

	if (!open_port())
	{
		MSG("Unable to get the game socket.\n");
		return 0;
	}

	max_players = MAX_NATION;

	// register the host now, even though his name is not known yet
	player_pool[0] = new PlayerDesc();
	player_pool[0]->id = 0;
	player_pool[0]->address.host = session->address.host;
	player_pool[0]->address.port = session->address.port;
	player_pool[0]->connecting = 1;

	joined_session = *session;
	strncpy(joined_session.password, password, MP_SESSION_NAME_LEN);

	status = MP_STATUS_CONNECTING;

	return 1;
}

void MultiPlayer::close_session()
{
}

void MultiPlayer::disable_join_session()
{
	allowing_connections = 0;
}

void MultiPlayer::send_game_beacon()
{
	static uint32_t ticks = 0;
	uint32_t player_id;
	uint32_t cur_ticks;

	cur_ticks = m.get_time();
	if (game_sock && (cur_ticks > ticks + 3000 || cur_ticks < ticks)) {
		// send the session beacon
		struct MsgGameBeacon p;

		ticks = cur_ticks;

		p.msg_id = MPMSG_GAME_BEACON;
		strncpy(p.name, joined_session.session_name, MP_SESSION_NAME_LEN);
		if (joined_session.password[0])
			p.password = 1;
		else
			p.password = 0;
		

		send_nonseq_msg(game_sock, (char *)&p, sizeof(struct MsgGameBeacon), &lan_broadcast_address);

		if (use_remote_session_provider)
		{
			send_nonseq_msg(game_sock, (char *)&p, sizeof(struct MsgGameBeacon), &remote_session_provider_address);
		}
	}
}

// Create a player and add to the pool.
//
// This is only called by the host upon connection from a client. The host
// chooses the player's id.
//
// Returns id if the player was added to the pool, and 0 if the player
// wasn't added to the pool.
//
int MultiPlayer::create_player(char *name, struct inet_address *address)
{
	int i;

	// search for an empty slot
	for (i = 0; i < max_players; i++)
		if (!player_pool[i])
			break;
	if (i >= max_players)
		return 0;

	// add to the pool
	player_pool[i] = new PlayerDesc();
	player_pool[i]->id = i+1;
	strncpy(player_pool[i]->name, name, MP_FRIENDLY_NAME_LEN);
	player_pool[i]->name[MP_FRIENDLY_NAME_LEN] = 0;
	player_pool[i]->connecting = 1;
	player_pool[i]->address.host = address->host;
	player_pool[i]->address.port = address->port;

	return player_pool[i]->id;
}

// Adds a player already created by the host to the pool
//
// <char *>   name        name of the player
// <uint32_t> id          id provided by the game host
//
// Returns 0 if the player cannot be added, and 1 if the player was added.
//
int MultiPlayer::add_player(char *name, uint32_t id)
{
	if (!player_pool[id-1]) {
		player_pool[id-1] = new PlayerDesc();
	}

	// add to the pool
	player_pool[id-1]->id = id;
	strncpy(player_pool[id-1]->name, name, MP_FRIENDLY_NAME_LEN);
	player_pool[id-1]->connecting = 1;

	return 1;
}

void MultiPlayer::set_my_player_id(uint32_t id)
{
	err_when(!id || id > max_players || !player_pool[my_player_id-1]);

	my_player_id = id;
}

void MultiPlayer::set_player_name(uint32_t id, char *name)
{
	err_when(!player_pool[id-1]);
	strncpy(player_pool[id-1]->name, name, MP_FRIENDLY_NAME_LEN);
}

// Deletes a player from the pool
//
// <uint32_t> id          id provided by the game host
//
void MultiPlayer::delete_player(uint32_t id)
{
	err_when(id < 1 || id > max_players);
	if (player_pool[id-1]) {
		delete player_pool[id-1];
		player_pool[id-1] = NULL;
	}
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

PlayerDesc *MultiPlayer::search_player(uint32_t playerId)
{
	if (playerId < 1 || playerId > max_players)
		return NULL;
	return player_pool[playerId-1];
}

int MultiPlayer::get_player_id(struct inet_address *address)
{
	int i;
	for (i = 0; i < max_players; i++)
	{
		if (player_pool[i] &&
			player_pool[i]->address.host == address->host &&
			player_pool[i]->address.port == address->port)
		{
			break;
		}
	}
	if (i >= max_players)
	{
		return 0;
	}
	return i+1;
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

int MultiPlayer::send_nonseq_msg(int sock, char *msg, int msg_size, struct inet_address *to)
{
	char send_buf[MP_UDP_MAX_PACKET_SIZE];
	struct packet_header *h;
	char *msg_buf;
	int total;

	total = msg_size + sizeof(struct packet_header);
	if (total > MP_UDP_MAX_PACKET_SIZE)
	{
		ERR("message exceeds maximum size\n");
		return 0;
	}

	h = (struct packet_header *)send_buf;
	msg_buf = send_buf + sizeof(struct packet_header);

	h->type = PACKET_NONSEQ;
	h->size = total;
	h->window = 0;
	h->sequence = 0;
	h->window_ack = 0;
	h->sequence_ack = 0;

	memcpy(msg_buf, msg, msg_size);

	if (!network->send(sock, h, to))
	{
		return 0;
	}

	return 1;
}

int MultiPlayer::send_system_msg(int sock, char *msg, int msg_size, struct inet_address *to)
{
	char send_buf[MP_UDP_MAX_PACKET_SIZE];
	struct packet_header *h;
	char *msg_buf;
	int total;

	total = msg_size + sizeof(struct packet_header);
	if (total > MP_UDP_MAX_PACKET_SIZE)
	{
		ERR("message exceeds maximum size\n");
		return 0;
	}

	h = (struct packet_header *)send_buf;
	msg_buf = send_buf + sizeof(struct packet_header);

	h->type = PACKET_SYSTEM;
	h->size = total;
	h->window = 0;
	h->sequence = 0;
	h->window_ack = 0;
	h->sequence_ack = 0;

	memcpy(msg_buf, msg, msg_size);

	if (!network->send(sock, h, to))
	{
		return 0;
	}

	return 1;
}

// send udp message
//
// pass BROADCAST_PID as toId to all players
//
// return 1 on success
//
int MultiPlayer::send(uint32_t to, void * data, uint32_t msg_size)
{
	err_when(to > max_players);

	if (!game_sock)
		return 0;
	if (to && to == my_player_id)
		return 0;
	if (msg_size > MP_UDP_MAX_PACKET_SIZE) {
		ERR("[MultiPlayer::send] message exceeds maximum size\n");
		return 0;
	}

	if (to == BROADCAST_PID) {
		int i;
		for (i = 0; i < max_players; i++)
			if (player_pool[i] &&
				player_pool[i]->connecting &&
				i+1 != my_player_id)
				this->send(i+1, data, msg_size);
		return 1;
	}

	if (player_pool[to-1] && player_pool[to-1]->connecting)
	{
		if (!send_nonseq_msg(game_sock, (char *)data, msg_size, &player_pool[to-1]->address))
		{
			ERR("[MultiPlayer::send] error while sending data to player %d\n", to);
			return 0;
		}
		MSG("[MultiPlayer::send] sent %d bytes to player %d\n", msg_size, to);
		return 1;
	}

	return 0;
}

// send tcp message
//
// pass BROADCAST_PID as toId to all players
//
// return 1 on success
//
int MultiPlayer::send_stream(uint32_t to, void * data, uint32_t msg_size)
{
	err_when(to > max_players);

	if (to && to == my_player_id)
		return 0;

	return 1;
}

// receive udp message
//
// return NULL if fails
// sysMsgCount records how many system messages have been handled
// notice : *sysMsgCount may be != 0, but return NULL
//
char *MultiPlayer::receive(uint32_t * from, uint32_t * to, uint32_t * size, int *sysMsgCount)
{
	if (sysMsgCount) *sysMsgCount = 0;
	*from = max_players;

	if (game_sock) {
		struct inet_address addr;
		struct packet_header *h;
		int ret;

		h = (struct packet_header *)recv_buf;
		h->size = MP_UDP_MAX_PACKET_SIZE;

		ret = network->recv(game_sock, h, &addr);
		if (ret > 0) {
			*from = get_player_id(&addr);
			*to = my_player_id;
			*size = h->size - sizeof(struct packet_header);
			MSG("[MultiPlayer::receive] received %d bytes from player %d\n", *size, *from);
			return *from ? recv_buf + sizeof(struct packet_header) : NULL;
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
char *MultiPlayer::receive_stream(uint32_t *from, uint32_t *to, uint32_t *size, int *sysMsgCount)
{
	err_when(!from || !to || !size || !recv_buf);

	if (sysMsgCount) *sysMsgCount = 0;

	return NULL;
}

void MultiPlayer::udp_accept_connections(struct packet_header *h, struct inet_address *address)
{
	struct MsgConnect *m;
	struct MsgConnectAck a;
	char password[MP_SESSION_NAME_LEN+1];
	int id;
	MsgNewPeerAddress msg;

	m = (struct MsgConnect *)((char *)h + sizeof(struct packet_header));

	// check if this is really a connect message
	if (h->size != sizeof(struct MsgConnect) + sizeof(struct packet_header) ||
			m->msg_id != MPMSG_CONNECT)
		return;

	// check the password
	strncpy(password, m->password, MP_SESSION_NAME_LEN);
	password[MP_SESSION_NAME_LEN] = 0;
	if (strcmp(joined_session.password, password) != 0)
		return;

	// allow connection if we can create the player
	id = create_player("?anonymous?", address);
	if (!id)
	{
		return;
	}
	MSG("Player %d connected.\n", id);

	// respond to the player
	a.msg_id = MPMSG_CONNECT_ACK;

	// send the connection ack message
	send_system_msg(game_sock, (char *)&a, sizeof(struct MsgConnectAck), address);

	// tell all peers
	msg.msg_id = MPMSG_NEW_PEER_ADDRESS;
	msg.player_id = id;
	msg.host = address->host;
	msg.port = address->port;
	send_stream(BROADCAST_PID, &msg, sizeof(msg));
}

void MultiPlayer::set_peer_address(uint32_t who, struct inet_address *address)
{
	err_when(who < 1 || who > max_players);

	if (who == my_player_id)
		return;
	if (!player_pool[who-1])
		return;

	player_pool[who-1]->address.host = address->host;
	player_pool[who-1]->address.port = address->port;

	MSG("[MultiPlayer::set_peer_address] set address for %d\n", who);
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

int MultiPlayer::show_leader_board()
{
	struct MsgRequestLadder m;
	struct MsgLadder *a;
	struct inet_address addr;
	struct packet_header *h;
	int ret, i, x, y;

	if (!open_port())
	{
		MSG("Unable to get the game socket.\n");
		return -1;
	}

	m.msg_id = MPMSG_REQ_LADDER;

	send_nonseq_msg(game_sock, (char *)&m, sizeof(struct MsgRequestLadder), &remote_session_provider_address);

	h = (struct packet_header *)recv_buf;
	a = (struct MsgLadder *)(recv_buf + sizeof(struct packet_header));
	h->size = MP_UDP_MAX_PACKET_SIZE;

	ret = network->recv(game_sock, h, &addr);
	if (ret <= 0)
		return 0;

	if (h->size != sizeof(struct MsgLadder) + sizeof(struct packet_header) ||
		addr.host != remote_session_provider_address.host ||
		addr.port != remote_session_provider_address.port ||
		a->msg_id != MPMSG_LADDER)
			return 0;

	vga_util.disp_image_file("HALLFAME");

	y = 116;
	for (i = 0; i < MP_LADDER_LIST_SIZE; i++, y += 76)
	{
		String str;
		char name[MP_PLAYER_NAME_LEN+1];
		int pos, y2;

		strncpy(name, a->list[i].name, MP_PLAYER_NAME_LEN);
		name[MP_PLAYER_NAME_LEN] = 0;

		if (!name[0])
			continue;

		x = 120;
		y2 = y + 17;
		pos = i + 1;
		str = pos;
		str += ".";
		font_std.put(x, y, str);

		x += 16;

		font_std.put(x, y, name);

		str = "Wins : ";
		str += a->list[i].wins;

		font_std.put(x, y2, str);

		str = "Losses : ";
		str += a->list[i].losses;

		font_std.put(x + 110, y2, str);

		str = "Score : ";
		str += a->list[i].score;

		font_std.put(x + 260, y2, str);
	}

	mouse.wait_press(60);

	vga_util.finish_disp_image_file();

	return 1;
}

void MultiPlayer::yield_recv()
{
	uint32_t ticks;
	uint32_t cur_ticks;
	struct inet_address sender;
	struct packet_header *h;
	int ret;

	h = (struct packet_header *)recv_buf;

	ticks = m.get_time();
	cur_ticks = ticks;

	while (cur_ticks < ticks + 50) {
		int player_id;

		h->size = MP_UDP_MAX_PACKET_SIZE;

		ret = network->recv(game_sock, h, &sender);
		if (ret <= 0)
			return;

		player_id = get_player_id(&sender);
		if (!player_id && status == MP_STATUS_PREGAME) {
			if (host_flag)
				udp_accept_connections(h, &sender);
			continue;
		}

		switch (h->type) {
		case PACKET_NONSEQ:
			break;
		case PACKET_STREAM:
			break;
		case PACKET_SYSTEM:
			break;
		}
	}
}

void MultiPlayer::yield_connecting()
{
	struct inet_address joining;
	struct packet_header *h;
	struct MsgConnect m;
	struct MsgConnectAck *a;
	struct inet_address *addr;
	int ret;

	addr = &player_pool[0]->address;
	m.msg_id = MPMSG_CONNECT;
	m.player_id = my_player_id;
	strncpy(m.password, joined_session.password, MP_SESSION_NAME_LEN);

	// send the connection message
	send_system_msg(game_sock, (char *)&m, sizeof(struct MsgConnect), addr);


	h = (struct packet_header *)recv_buf;
	a = (struct MsgConnectAck *)(recv_buf + sizeof(struct packet_header));

	h->size = MP_UDP_MAX_PACKET_SIZE;

	// check for ack
	ret = network->recv(game_sock, h, &joining);
	if (ret <= 0)
		return;

	// check if this really is an ack
	if (joining.host != addr->host ||
			joining.port != addr->port ||
			h->size != sizeof(struct MsgConnectAck) + sizeof(struct packet_header) ||
			a->msg_id != MPMSG_CONNECT_ACK)
		return;

	status = MP_STATUS_PREGAME;

	MSG("[MultiPlayer::udp_join_session] udp connection established\n");
}

void MultiPlayer::yield_pregame()
{
	yield_recv();
	if (host_flag)
	{
		send_game_beacon();
	}
}

void MultiPlayer::yield()
{
	switch (status)
	{
	case MP_STATUS_CONNECTING:
		yield_connecting();
		break;
	case MP_STATUS_PREGAME:
		yield_pregame();
		break;
	}
}
