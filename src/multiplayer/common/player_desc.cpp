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

// Filename    : player_desc.cpp
// Description : PlayerDesc tracks the connection of human opponents during
//               multiplayer games.

#include <player_desc.h>
#include <string.h>

PlayerDesc::PlayerDesc()
{
	id = 0;
	strcpy(name, "?Anonymous?");
	address.host = ENET_HOST_ANY;
	address.port = 0;
	connecting = 0;
	authorized = 0;
}

PlayerDesc::PlayerDesc(ENetAddress *address)
{
	id = 0;
	strcpy(name, "?Anonymous?");
	this->address.host = address->host;
	this->address.port = address->port;
	connecting = 0;
	authorized = 0;
}

PlayerDesc::PlayerDesc(uint32_t id, ENetAddress *address)
{
	this->id = id;
	strcpy(name, "?Anonymous?");
	this->address.host = address->host;
	this->address.port = address->port;
	connecting = 0;
	authorized = 0;
}

PlayerDesc::PlayerDesc(uint32_t id, char *name)
{
	this->id = id;
	strcpy(this->name, name);
	address.host = ENET_HOST_ANY;
	address.port = 0;
	connecting = 0;
	authorized = 0;
}

PlayerDesc::PlayerDesc(char *name)
{
	id = 0;
	strcpy(this->name, name);
	address.host = ENET_HOST_ANY;
	address.port = 0;
	connecting = 0;
	authorized = 0;
}

uint32_t PlayerDesc::pid()
{
	return id;
}

char *PlayerDesc::friendly_name_str()
{
	return name;
}

ENetAddress *PlayerDesc::get_address()
{
	if (address.host == ENET_HOST_ANY)
		return NULL;

	return &this->address;
}
