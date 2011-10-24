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

// Filename    : network_sdlnet.cpp
// Description : SDLNet implementation for Network

#include <dbglog.h>
#include <network.h>

DBGLOG_DEFAULT_CHANNEL(MultiPlayer);

NetworkSDLNet::NetworkSDLNet()
{
	initialized = 0;
}

NetworkSDLNet::~NetworkSDLNet()
{
	deinit();
}

int NetworkSDLNet::init()
{
	if (initialized)
	{
		return 1;
	}

	if (SDLNet_Init() == -1)
	{
		ERR("Unable to init SDLNet: %s\n", SDLNet_GetError());
		return 0;
	}

	for (int i = 0; i < MAX_UDP_SOCKETS; i++)
	{
		udp_socket_list[i] = NULL;
	}

	initialized = 1;

	return 1;
}

void NetworkSDLNet::deinit()
{
	int i;
	for (i = 0; i < MAX_UDP_SOCKETS; i++)
		if (udp_socket_list[i])
			SDLNet_UDP_Close(udp_socket_list[i]);

	SDLNet_Quit();
	initialized = 0;
}

int NetworkSDLNet::resolve_host(struct inet_address *ip, const char *name, uint16_t port)
{
	IPaddress a;
	int r;

	r = SDLNet_ResolveHost(&a, name, port);
	if (!r)
	{
		ip->host = a.host;
		ip->port = a.port;
	}
	else
	{
		MSG("Couldn't resolve host '%s' port %u: %s\n", name, port, SDLNet_GetError());
	}

	return !r;
}

int NetworkSDLNet::udp_open(uint16_t port)
{
	int i;

	for (i = 0; i < MAX_UDP_SOCKETS; i++)
		if (!udp_socket_list[i])
			break;
	if (i >= MAX_UDP_SOCKETS)
		return 0;

	udp_socket_list[i] = SDLNet_UDP_Open(port);

	if (!udp_socket_list[i])
	{
		MSG("Unable to open udp port %u.\n", port);
		return 0;
	}
	return i+1;
}

void NetworkSDLNet::udp_close(int sock)
{
	int s;

	s = sock - 1;

	if (s > 1 || s >= MAX_UDP_SOCKETS || !udp_socket_list[s])
		return;

	SDLNet_UDP_Close(udp_socket_list[s]);
	udp_socket_list[s] = NULL;
}

int NetworkSDLNet::send(int sock, struct packet_header *p, struct inet_address *to)
{
	int s;
	UDPpacket u;

	s = sock - 1;

	if (s < 0 || s >= MAX_UDP_SOCKETS || !udp_socket_list[s])
		return 0;

	u.channel = -1;
	u.data = (Uint8 *)p;
	u.len = p->size + sizeof(struct packet_header);
	u.address.host = to->host;
	u.address.port = to->port;
	
	if (!SDLNet_UDP_Send(udp_socket_list[s], -1, &u))
	{
		MSG("Couldn't send to %x %u\n", to->host, to->port);
		return 0;
	}
	return 1;
}


int NetworkSDLNet::recv(int sock, struct packet_header *p, struct inet_address *from)
{
	int s;
	int r;
	UDPpacket u;

	s = sock - 1;

	if (s < 0 || s >= MAX_UDP_SOCKETS || !udp_socket_list[s])
		return 0;

	u.channel = -1;
	u.data = (Uint8 *)p;
	u.maxlen = p->size;

	r = SDLNet_UDP_Recv(udp_socket_list[s], &u);
	if (r == -1)
	{
		MSG("Couldn't receive on socket.\n");
		return 0;
	}
	if (!r)
		return 0;
	if (u.len != p->size + sizeof(struct packet_header))
		return 0;

	from->host = u.address.host;
	from->port = u.address.port;

	return u.len;
}
