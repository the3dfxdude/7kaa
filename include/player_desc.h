/*
 * Seven Kingdoms: Ancient Adversaries
 *
 * Copyright 1997,1998 Enlight Software Ltd.
 * Copyright 2010,2011,2015 Jesse Allen
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

// Filename    : player_desc.h
// Description : MultiPlayer's PlayerDesc class

#ifndef __PLAYER_DESC_H
#define __PLAYER_DESC_H

#ifndef EMSCRIPTEN
#include <MPTYPES.h>
#include <enet/enet.h>
#else
#include <session_desc.h>
#endif

class PlayerDesc
{
#ifndef EMSCRIPTEN
public:
	uint32_t id;
	char name[MP_FRIENDLY_NAME_LEN+1];
	char authorized;
	ENetAddress address;

	PlayerDesc(const char* name, const ENetAddress& address);
#endif
};

#endif // __PLAYER_DESC_H

