/*
 * Seven Kingdoms: Ancient Adversaries
 *
 * Copyright 1997,1998 Enlight Software Ltd.
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

#ifdef IMAGICMP
	#define MultiPlayerType MultiPlayerIM
	#define mp_obj mp_im
	#define BROADCAST_PID IMMBROADCAST
	#define PID_TYPE IMMPID
	#define ServiceProviderDesc IMServiceProvider
	#define ServiceIdType DWORD
	#define SessionDesc IMSessionDesc
	#define SessionIdType DWORD
	#define PlayerDesc IMPlayer
#else
	#define MultiPlayerType MultiPlayerDP
	#define mp_obj mp_dp
	#define BROADCAST_PID DPID_ALLPLAYERS
	#define PID_TYPE DPID
	#define ServiceProviderDesc DPServiceProvider
	#define ServiceIdType GUID
	#define SessionDesc DPSessionDesc
	#define SessionIdType GUID
	#define PlayerDesc DPPlayer
#endif

#endif