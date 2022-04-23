/*
* Seven Kingdoms: Ancient Adversaries
*
* Copyright 2015 Jesse Allen
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

#ifndef __SESSIONDESC_H
#define __SESSIONDESC_H

#ifndef EMSCRIPTEN
#include <MPTYPES.h>
#include <misc_uuid.h>
#include <enet/enet.h>
#else
typedef void * ENetAddress;
typedef void ENetEvent;
typedef void * ENetSocket;
typedef void ENetPeer;
typedef void ENetPacket;
#endif

struct SessionFlags {
	enum {
		Hosting         = 1,
		Full            = 2,
		Password        = 4,
		Pregame         = 8
	};
};

struct SessionDesc
{
#ifndef EMSCRIPTEN
	char session_name[MP_FRIENDLY_NAME_LEN+1];
	char password[MP_FRIENDLY_NAME_LEN+1];
	guuid_t session_id;
	uint32_t flags;
	int max_players;
	int player_count;
	ENetAddress address;

	SessionDesc() = default;
	SessionDesc(const char* session_name, const guuid_t& session_id, uint32_t flags, const ENetAddress& address);
#endif
};

#endif
