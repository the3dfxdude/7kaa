/*
 * Seven Kingdoms: Ancient Adversaries
 *
 * Copyright 2011 Jesse Allen
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

// Filename    : network_none.cpp
// Description : Dummy implementation for Network

#include <dbglog.h>
#include <network.h>

DBGLOG_DEFAULT_CHANNEL(MultiPlayer);

NetworkNone::NetworkNone()
{
}

NetworkNone::~NetworkNone()
{
	deinit();
}

int NetworkNone::init()
{
	return 0;
}

void NetworkNone::deinit()
{
}

int NetworkNone::resolve_host(struct inet_address *ip, const char *name, uint16_t port)
{
	return 0;
}

int NetworkNone::udp_open(uint16_t port)
{
	return 0;
}

void NetworkNone::udp_close(int sock)
{
}

int NetworkNone::send(int sock, struct packet_header *p, struct inet_address *to)
{
	return 0;
}


int NetworkNone::recv(int sock, struct packet_header *p, struct inet_address *from)
{
	return 0;
}
