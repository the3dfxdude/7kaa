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
#include <network_sdlnet.h>

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

	initialized = 1;

	return 1;
}

void NetworkSDLNet::deinit()
{
	SDLNet_Quit();
	initialized = 0;
}
