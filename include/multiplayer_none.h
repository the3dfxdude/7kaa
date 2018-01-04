/*
 * Seven Kingdoms: Ancient Adversaries
 *
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

// Filename    : multiplayer_none.h
// Description : No multiplayer support.

#ifndef __MULTIPLAYER_NONE_H
#define __MULTIPLAYER_NONE_H

#include <MPTYPES.h>
#include <session_desc.h>
#include <player_desc.h>
#include <stdint.h>


class MultiPlayer
{
public:
	void   init(ProtocolType) {}
	void   deinit() {}
	bool   is_initialized() const { return false; }

	// ------- functions on lobby -------- //
	int    init_lobbied(int maxPlayers, char* cmdLine) { return 0; }
	int    is_lobbied() { return 0; } // return 0=not lobbied, 1=auto create, 2=auto join, 4=selectable
	char * get_lobbied_name() {return nullptr; } // return 0 if not available

	// ------- functions on network protocols ------ //
	bool   is_protocol_supported(ProtocolType) { return false; }
	int    is_update_available() { return false; }
	void   game_starting() {}
	void   disable_new_connections() {}

	// ------- functions on session --------//
	int    set_service_provider(const char *host) { return 0; }
	int    poll_sessions() { return MP_POLL_NO_UPDATE; }
	void   sort_sessions(int sortType) {}
	int    create_session(char *sessionName, char *password, int maxPlayers) { return 0; }
	int    join_session(SessionDesc *session) { return 0; }
	int    close_session() { return 0; }
	SessionDesc* get_session(int i) { return nullptr; }
	SessionDesc* get_session(guuid_t id) { return nullptr; }
	SessionDesc* get_current_session() { return nullptr; }

	// -------- functions on player management -------//
	int         add_player(uint32_t playerId, char *name, const NetworkAddress &address, char contact) { return 0; }
	int         auth_player(uint32_t playerId, char *name, char *password) { return 0; }
	void        create_my_player(char *playerName) {}
	int         set_my_player_id(uint32_t playerId) { return 0; }
	void        delete_player(uint32_t playerId) {}
	int         poll_players() { return MP_POLL_NO_UPDATE; }
	PlayerDesc* get_player(int i) { return nullptr; }
	PlayerDesc* search_player(uint32_t playerId) { return nullptr; }
	int         is_player_connecting(uint32_t playerId) { return 0; }
	uint32_t    get_my_player_id() const { return 0; }

	// ------- functions on message passing ------//
	int    send(uint32_t to, void * data, uint32_t msg_size) { return 0; }
	char  *receive(uint32_t *from, uint32_t *size, int *sysMsgCount=0);
};

#endif // __MULTIPLAYER_ENET_H

