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

uint32_t PlayerDesc::pid()
{
	return id;
}

char *PlayerDesc::friendly_name_str()
{
	return name;
}

int PlayerDesc::get_address(struct inet_address *addr)
{
	addr->host = address.host;
	addr->port = address.port;
	return sizeof(struct inet_address);
}

