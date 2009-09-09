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