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

// Filename    : OERRCTRL.H
// Description : Error control for dplay

#ifndef __OERRCTRL_H
#define __OERRCTRL_H

#include <GAMEDEF.h>
#include <MPTYPES.h>
#include <OVQUEUE.h>

struct EcMsgHeader
{
	char	func_id;				// FIRST_SEND / RE_SEND / ACKNOW / NEGACK
	char	sender_id;			// 1 to MAX_NATION (ecPlayerId)
	char	frame_id;

	void	init( char funcId, char senderId, char frameId )
	{
		func_id = funcId;
		sender_id = senderId;
		frame_id = frameId;
	}
};

class MultiPlayer;

class ErrorControl
{
	enum { FIRST_SEND, RE_SEND, ACKNOW, NEGACK };
//	enum { MAX_PLAYER = MAX_NATION, MAX_QUEUE = 12, MAX_RECV_QUEUE = 48 };
	enum { MAX_PLAYER = MAX_NATION, MAX_QUEUE = 18, MAX_RECV_QUEUE = 72 };
	// MAX_QUEUE/2 > Remote::MAX_PROCESS_FRAME_DELAY
	// MAX_RECV_QUEUE > MAX_QUEUE/2 * MAX_PLAYER

private:
	MultiPlayer *mp_ptr;
	int	connecting_player_count;				// no. of peers
	char	self_ec_player_id;

	char	send_head;		// queue_head == queue_tail, empty
	char	send_tail;		// queue_tail == queue_head -1, full
	VLenQueue	send_queue[MAX_QUEUE];			// include EcMsgHeader and 8-bit CRC
	char	ack_flag[MAX_QUEUE][MAX_PLAYER];
	unsigned long	send_time[MAX_QUEUE];
	unsigned long	re_send_after[MAX_QUEUE];

	unsigned long	dp_id[MAX_PLAYER];		// directPlay playerid, 0 if not valid
	char	wait_to_receive[MAX_PLAYER];
	char	recv_flag[MAX_PLAYER][MAX_QUEUE];
	// char	next_send[MAX_PLAYER];
	// char	next_ack_send[MAX_PLAYER];
	// char	retrans_state[MAX_PLAYER];

	char	recv_head;
	char	recv_tail;
	VLenQueue	receive_queue[MAX_RECV_QUEUE];		// include EcMsgHeader and 8-bit CRC

private:
	static inline void inc_frame_id(char &frameId)
		{ if(++frameId >= MAX_QUEUE) frameId = 0; }
	static inline char next_frame_id(char frameId)
		{ return frameId >= MAX_QUEUE-1 ? 0 : frameId+1 ; }
	static inline char prev_frame_id(char frameId)
		{ return frameId <= 0 ? MAX_QUEUE-1 : frameId-1 ; }
	static int	is_between(int low, int t, int high)
		{ return low <= high ? (low <= t && t < high) : (low <= t || t < high); }

	int	is_send_empty();
	int	is_send_full();
	int	send_queue_space();
	int	en_send_queue();									// return frameId
	void	de_send_queue();									// free send_head

	int	is_recv_empty();
	int	is_recv_full();
	void	en_recv_queue(void *dataPtr, unsigned long dataLen);
	int	recv_queue_space();

	// functions on ack_flag
	int	is_waiting_ack(char ecPlayerId, char frameId);
	void	set_ack(char ecPlayerId, char frameId);
	void	clear_ack(char frameId);
	int	are_all_acked(char frameId);
	void	clear_acked_frame();

	// functions on recv_flag
	int	is_waiting_receive(char ecPlayerId, char frameId);
	void	set_recv_flag(char ecPlayerId, char frameId);
	void	clear_recv_flag(char ecPlayerId, char frameId);

	void	mark_send_time(char frameId, unsigned long duration);
	int	need_re_send(char frameId, int promptFactor);

public:
	void	init(MultiPlayer *mp, char ecPlayerId);
	void	deinit();
	void	set_dp_id(char ecPlayerId, unsigned long dpPlayerId );
	char	get_ec_player_id( unsigned long dpPlayerId );

	int	send(char ecPlayerId, void *dataPtr, unsigned long dataLen);
	char *receive(char *sendEcPlayerId, unsigned long *dataLen);
	void	de_recv_queue();			// get the content from recv_queue[recv_head] before de_recv_queue

	int	is_player_valid(char ecPlayerId);
	void	set_player_lost(char ecPlayerId);
	void	yield();
	void	re_transmit(int promptFactor=1);
};

extern ErrorControl ec_remote;

#endif
