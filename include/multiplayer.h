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

#ifndef __MULTIPLAYER_H
#define __MULTIPLAYER_H

enum ProtocolType
{
	None = 0,
	IPX = 1,
	TCPIP = 2,
	Modem = 4,
	Serial = 8
};

enum
{
	MP_POLL_NO_UPDATE,
	MP_POLL_UPDATE,
	MP_POLL_LOGIN_PENDING,
	MP_POLL_LOGIN_FAILED,
	MP_POLL_NO_SOCKET,
	MP_POLL_NO_SESSION,
};


#if defined(USE_ENET)
#include <multiplayer_enet.h>
#elif defined(USE_NOMULTIPLAYER)
#include <multiplayer_none.h>
#else
#error "A multiplayer backend must be specified."
#endif

extern MultiPlayer mp_obj;

#endif
