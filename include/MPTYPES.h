/*
 * Seven Kingdoms: Ancient Adversaries
 *
 * Copyright 1997,1998 Enlight Software Ltd.
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

// Filename    : MPTYPES.H
// Description : Multiplayer header, define symbols


#ifndef __MPTYPES_H
#define __MPTYPES_H

#include <stdint.h>

#define PID_TYPE uint32_t
#define BROADCAST_PID 0

#define MP_FRIENDLY_NAME_LEN 64

// A network address, which should be considerd an opaque type (for code other than the chosen network library implementation)
//  to be used only as a token for passing to and from the Multiplayer class and to transfer over the wire.
struct NetworkAddress {
protected:
	uint32_t host;
	uint16_t port;
};

#endif
