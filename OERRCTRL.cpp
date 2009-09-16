// Filename   : OERRCTRL.H
// Descrition : Error control

#include <OERRCTRL.H>
#include <ODPLAY.H>
#include <OIMMPLAY.H>
#include <CRC.H>
#include <ALL.H>

#define DEBUG_LOG_LOCAL 1
#include <OLOG.H>

// ---------- define constant -----------//
// 0 don't display log, 1 display exception, 2 display more detail, 3 display all
#define DEBUG_LOG_LEVEL 1

const	SHORT_TIME_OUT = 100;
const TIME_OUT = 2000;		// 2 sec
const CONNECT_LOST_TIME = 20000;			// 20 sec
static String debugStr;

void ErrorControl::init(MultiPlayerType *mp, char ecPlayerId )
{
	// ---------- initialize dp_id array ---------- //
	memset(dp_id, 0, sizeof(dp_id) );
	connecting_player_count = 0;
	self_ec_player_id = ecPlayerId;
	mp_ptr = mp;

	// ---------- initialize head and tail of queues ---------- //
	send_head = send_tail = 0;
	recv_head = recv_tail = 0;

}

void ErrorControl::deinit()
{
}

void ErrorControl::set_dp_id(char ecPlayerId, DWORD dpPlayerId )
{
	if( ecPlayerId != self_ec_player_id )
	{
		err_when( ecPlayerId < 1 || ecPlayerId > MAX_PLAYER );
		dp_id[ecPlayerId-1] = dpPlayerId;
		wait_to_receive[ecPlayerId-1] = 0;
		memset(recv_flag[ecPlayerId-1], 0, MAX_QUEUE );
		// next_send[ecPlayerId-1] = 0;
		// next_ack_send[ecPlayerId-1] = 0;
		// retrans_state[ecPlayerId-1] = 0;

		// ------- update connecting_player_count --------//
		connecting_player_count = 0;
		for(int p = 0; p < MAX_PLAYER; ++p)
			if( dp_id[p] )
				connecting_player_count++;
	}
}

// return ec_player_id, 0 for not found (can't found own dpPlayerId)
char ErrorControl::get_ec_player_id( DWORD dpPlayerId )
{
	if( dpPlayerId == BROADCAST_PID || dpPlayerId == 0)
		return 0;
	for( char ecPlayerId = 1; ecPlayerId <= MAX_PLAYER; ++ecPlayerId )
	{
		if( dpPlayerId == dp_id[ecPlayerId-1] )
			return ecPlayerId;
	}
	return 0;
}

// return 1 on success, -1 if queue is_full, 0 for other failure
int ErrorControl::send(char ecPlayerId, void *dataPtr, DWORD dataLen)
{
	if( connecting_player_count == 0)
		return 1;

	if( send_queue_space() < MAX_QUEUE/2)
	{
#if (defined(DEBUG) && DEBUG_LOG_LEVEL >= 1)
		DEBUG_LOG("ec_remote.send() fail, buffer half full");
#endif
		return -1;
	}

	int frameId = en_send_queue();
	if( frameId < 0)
	{
#if (defined(DEBUG) && DEBUG_LOG_LEVEL >= 1)
		DEBUG_LOG("ec_remote.send() fail, buffer full");
#endif
		return frameId;
	}
	else
	{
		// -------- add buffer to queue --------- //
		err_when( frameId >= MAX_QUEUE );
		VLenQueue &sq = send_queue[frameId];
		sq.clear();
		char *ecMsg = sq.reserve( sizeof(EcMsgHeader) + dataLen + CRC_LEN );
		((EcMsgHeader *)ecMsg)->init( FIRST_SEND, self_ec_player_id, frameId );
		memcpy( ecMsg + sizeof(EcMsgHeader), dataPtr, dataLen );
		*((CRC_TYPE *) (ecMsg + sizeof(EcMsgHeader) + dataLen) ) = crc8((unsigned char *)ecMsg, sizeof(EcMsgHeader) + dataLen);

		// ------- clear all ack flags of that frame --------//
		PID_TYPE toDPid = BROADCAST_PID;
		clear_ack( frameId );
		if( ecPlayerId != 0)
		{
			toDPid = dp_id[ecPlayerId];
			err_when(toDPid == BROADCAST_PID);
			for(char p = 1; p <= MAX_PLAYER; ++p)
				if( p != ecPlayerId )
					set_ack(p, frameId);
		}

		// ------- try to send the data for the first time ------- //
		if( mp_ptr->send(toDPid, sq.queue_buf, sq.length()) )
		{
			// mark send time
			mark_send_time(frameId, TIME_OUT );

#if (defined(DEBUG) && DEBUG_LOG_LEVEL >= 3)
			debugStr = "ec_remote.send() successful, frame ";
			debugStr += frameId;
			DEBUG_LOG(debugStr);
#endif

			// mark the func_id of the message RE_SEND
			((EcMsgHeader *)ecMsg)->func_id = RE_SEND;

			// recalculate CRC
			*((CRC_TYPE *) (ecMsg + sizeof(EcMsgHeader) + dataLen) ) = crc8((unsigned char *)ecMsg, sizeof(EcMsgHeader) + dataLen);
		}
		else
		{
			// mark send time, mark a shorter time
			mark_send_time(frameId, SHORT_TIME_OUT);

#if (defined(DEBUG) && DEBUG_LOG_LEVEL >= 2)
			debugStr = "ec_remote.send() fail, frame ";
			debugStr += frameId;
			DEBUG_LOG(debugStr);
#endif
			// still return true to sender, as it will be re-send later
		}
	}

	mp_ptr->after_send();

	return 1;
}

char *ErrorControl::receive(char *sendEcPlayerId, DWORD *dataLen)
{
	// ----- draw the head of recv_queue ----- //

	if( is_recv_empty() )
	{
		return NULL;
	}
	else
	{
		char *dataPtr = receive_queue[recv_head].queue_buf;
		DWORD len = receive_queue[recv_head].length();
		err_when( len < sizeof(EcMsgHeader) + CRC_LEN);
		if( sendEcPlayerId )
			*sendEcPlayerId = ((EcMsgHeader *)dataPtr)->sender_id;
		if( dataLen )
			*dataLen = len - sizeof(EcMsgHeader) - CRC_LEN;
		return dataPtr + sizeof(EcMsgHeader);
	}
}

int ErrorControl::is_player_valid(char ecPlayerId)
{
	return dp_id[ecPlayerId-1] != 0 || ecPlayerId == self_ec_player_id;
}

void ErrorControl::set_player_lost(char ecPlayerId)
{
	dp_id[ecPlayerId-1] = 0;

	clear_acked_frame();		// some send_queue message may be waiting this player's ack
}

void ErrorControl::yield()
{
	// -------- receive any frame from dplay -----------//
	char *recvPtr;
	DWORD recvLen;
	PID_TYPE from, to;

	static int simError = 1;

	// check any player lost

	// ##### begin Gilbert 2/5 ########//
	int sysMsgCount;
	int p;

	// detect any player lost, detected previous mp_ptr->send
	for( p = 1; p <= MAX_PLAYER; ++p)
	{
		if( dp_id[p-1] && !mp_ptr->is_player_connecting(dp_id[p-1]) )
		{
			set_player_lost(p);
			connecting_player_count--;
		}
	}

	mp_ptr->before_receive();

	while( (recvPtr = mp_ptr->receive(&from, &to, &recvLen, &sysMsgCount)) != NULL
		|| sysMsgCount != 0)
	{
		// -------- detect any player lost ---------//

		if( sysMsgCount )
		{
			for( p = 1; p <= MAX_PLAYER; ++p)
			{
				if( dp_id[p-1] && !mp_ptr->is_player_connecting(dp_id[p-1]) )
				{
					set_player_lost(p);
					connecting_player_count--;
				}
			}
		}

		if( !recvPtr )
			break;	// only received system message from direct play
	// ##### end Gilbert 2/5 ########//

		// -------- receive the message ----------//
#ifdef DEBUG
		// simulate crc error
//		if( ++simError >= 10)
//			simError = 0;
#endif
		if( simError && !crc8((unsigned char *)recvPtr, recvLen))
		{
			// crc correct
			EcMsgHeader ecMsg = *(EcMsgHeader *)recvPtr;
			switch( ecMsg.func_id )
			{
			case FIRST_SEND:
				// accept except frameId is wait_to_receive -1 or recv_flag is set
				if( is_waiting_receive( ecMsg.sender_id, ecMsg.frame_id ) )
				{
					err_when( ecMsg.sender_id <= 0 || ecMsg.sender_id > MAX_PLAYER);
#if ( defined(DEBUG) && DEBUG_LOG_LEVEL >= 3 )
					debugStr = "ec_remote : FIRST_SEND received, from:";
					debugStr += ecMsg.sender_id;
					debugStr += " frame:";
					debugStr += ecMsg.frame_id;
					debugStr += " accepted";
					DEBUG_LOG(debugStr);
#endif
					if( !is_recv_full() )
					{
						if( dp_id[ecMsg.sender_id-1] )
						{
							// mark recv_flag
							set_recv_flag(ecMsg.sender_id, ecMsg.frame_id);

							// send ACK
							char replyMsg[sizeof(EcMsgHeader) + CRC_LEN];
							((EcMsgHeader *)replyMsg)->init(ACKNOW, self_ec_player_id, ecMsg.frame_id);
							*((CRC_TYPE *)(replyMsg + sizeof(EcMsgHeader))) = crc8((unsigned char *)replyMsg, sizeof(EcMsgHeader));
							mp_ptr->send( dp_id[ecMsg.sender_id-1], replyMsg, sizeof(replyMsg) );

							if( ecMsg.frame_id == wait_to_receive[ecMsg.sender_id-1] )
							{
								// clear recv_flag, until it is zero
								char &scanFrame = wait_to_receive[ecMsg.sender_id-1];
								for( ; recv_flag[ecMsg.sender_id-1][scanFrame]; inc_frame_id(scanFrame) )
									clear_recv_flag( ecMsg.sender_id, prev_frame_id(scanFrame) );
							}
						}

						// append the queue to receive queue
						en_recv_queue(recvPtr, recvLen);
					}
					else
					{
						// drop the message if the receive queue is full
#if ( defined(DEBUG) && DEBUG_LOG_LEVEL >= 2 )
						DEBUG_LOG("ec_remote : but receive_queue is full, discard message");
#endif
					}
				}
				else
				{
#if ( defined(DEBUG) && DEBUG_LOG_LEVEL >= 2 )
					debugStr = "ec_remote : FIRST_SEND received, from:";
					debugStr += ecMsg.sender_id;
					debugStr += " frame:";
					debugStr += ecMsg.frame_id;
					debugStr += " discarded";
					DEBUG_LOG(debugStr);
#endif
					// some frame before are missing, wait resend
					// discard the frame, but reply, for the sender not to
					// send the frame again
					if( dp_id[ecMsg.sender_id-1] )
					{
						// send ACK
						char replyMsg[sizeof(EcMsgHeader) + CRC_LEN];
						((EcMsgHeader *)replyMsg)->init(ACKNOW, self_ec_player_id, ecMsg.frame_id);
						*((CRC_TYPE *)(replyMsg + sizeof(EcMsgHeader))) = crc8((unsigned char *)replyMsg, sizeof(EcMsgHeader));
						mp_ptr->send( dp_id[ecMsg.sender_id-1], replyMsg, sizeof(replyMsg) );
					}
				}
				break;

			case RE_SEND:
				// accept except frameId is wait_to_receive -1 or recv_flag is set
				if( is_waiting_receive( ecMsg.sender_id, ecMsg.frame_id ) )
				{
#if ( defined(DEBUG) && DEBUG_LOG_LEVEL >= 2 )
					debugStr = "ec_remote : RE_SEND received, from:";
					debugStr += ecMsg.sender_id;
					debugStr += " frame:";
					debugStr += ecMsg.frame_id;
					debugStr += " accepted";
					DEBUG_LOG(debugStr);
#endif
					err_when( ecMsg.sender_id <= 0 || ecMsg.sender_id > MAX_PLAYER);

					if( !is_recv_full() )
					{
						if( dp_id[ecMsg.sender_id-1] )
						{
							// mark recv_flag
							set_recv_flag(ecMsg.sender_id, ecMsg.frame_id);

							// send ACK
							char replyMsg[sizeof(EcMsgHeader) + CRC_LEN];
							((EcMsgHeader *)replyMsg)->init(ACKNOW, self_ec_player_id, ecMsg.frame_id);
							*((CRC_TYPE *)(replyMsg + sizeof(EcMsgHeader))) = crc8((unsigned char *)replyMsg, sizeof(EcMsgHeader));
							mp_ptr->send( dp_id[ecMsg.sender_id-1], replyMsg, sizeof(replyMsg) );

							if( ecMsg.frame_id == wait_to_receive[ecMsg.sender_id-1] )
							{
								// clear recv_flag, until it is zero
								char &scanFrame = wait_to_receive[ecMsg.sender_id-1];
								for( ; recv_flag[ecMsg.sender_id-1][scanFrame]; inc_frame_id(scanFrame) )
									clear_recv_flag( ecMsg.sender_id, prev_frame_id(scanFrame) );
							}
						}

						// append the queue to receive queue
						en_recv_queue(recvPtr, recvLen);
					}
					else
					{
						// drop the message if the receive queue is full
#if ( defined(DEBUG) && DEBUG_LOG_LEVEL >= 2 )
						DEBUG_LOG("ec_remote : but receive_queue is full, discard message");
#endif
					}
				}
				else
				{
#if ( defined(DEBUG) && DEBUG_LOG_LEVEL >= 2 )
					debugStr = "ec_remote : RE_SEND received, from:";
					debugStr += ecMsg.sender_id;
					debugStr += " frame:";
					debugStr += ecMsg.frame_id;
					debugStr += " discarded";
					DEBUG_LOG(debugStr);
#endif
					// re-dundant frame, discard, but still reply ACK,
					// for the sender not to send the frame again
					if( dp_id[ecMsg.sender_id-1] )
					{
						// send ACK
						char replyMsg[sizeof(EcMsgHeader) + CRC_LEN];
						((EcMsgHeader *)replyMsg)->init(ACKNOW, self_ec_player_id, ecMsg.frame_id);
						*((CRC_TYPE *)(replyMsg + sizeof(EcMsgHeader))) = crc8((unsigned char *)replyMsg, sizeof(EcMsgHeader));
						mp_ptr->send( dp_id[ecMsg.sender_id-1], replyMsg, sizeof(replyMsg) );
					}
				}
				break;
			case ACKNOW:
				// mark the frame ack
				if( is_waiting_ack(ecMsg.sender_id, ecMsg.frame_id) )
				{
#if (defined(DEBUG) && DEBUG_LOG_LEVEL >= 3 )
					debugStr = "ec_remote : ACKNOW received";
					debugStr += ecMsg.sender_id;
					debugStr += " frame:";
					debugStr += ecMsg.frame_id;
					debugStr += " accepted";
					DEBUG_LOG(debugStr);
#endif
					set_ack(ecMsg.sender_id, ecMsg.frame_id);
					clear_acked_frame();
				}
				else
				{
					// discard the frame
#if (defined(DEBUG) && DEBUG_LOG_LEVEL >= 2 )
					debugStr = "ec_remote : ACKNOW received";
					debugStr += ecMsg.sender_id;
					debugStr += " frame:";
					debugStr += ecMsg.frame_id;
					debugStr += " discarded";
					DEBUG_LOG(debugStr);
#endif
				}
				break;
			case NEGACK:
				// re-send only the frameId
				if( is_waiting_ack(ecMsg.sender_id, ecMsg.frame_id) )
				{
#if (defined(DEBUG) && DEBUG_LOG_LEVEL >= 2 )
					debugStr = "ec_remote : NEGACK received";
					debugStr += ecMsg.sender_id;
					debugStr += " frame:";
					debugStr += ecMsg.frame_id;
					debugStr += " accepted";
					DEBUG_LOG(debugStr);
#endif
					char *replyMsg = send_queue[ecMsg.frame_id].queue_buf;
					DWORD replyLen = send_queue[ecMsg.frame_id].length();

					mp_ptr->send( dp_id[ecMsg.sender_id-1], replyMsg, replyLen );
					err_when( replyLen <= sizeof(EcMsgHeader) );

					// don't mark re-send time
					// mark the func_id of the message RE_SEND
					((EcMsgHeader *)replyMsg)->func_id = RE_SEND;

					// recalculate CRC
					*((CRC_TYPE *) (replyMsg + replyLen - CRC_LEN) ) = crc8((unsigned char *)replyMsg, replyLen - CRC_LEN);
					DEBUG_LOG("ec_remote : frame retransmitted");
				}
				else
				{
#if (defined(DEBUG) && DEBUG_LOG_LEVEL >= 2 )
					debugStr = "ec_remote : NEGACK received";
					debugStr += ecMsg.sender_id;
					debugStr += " frame:";
					debugStr += ecMsg.frame_id;
					debugStr += " discarded";
					DEBUG_LOG(debugStr);
#endif
				}
				break;
			default:
				err_here();
			}
		}
		else
		{
			// crc incorrect
			if( recvLen > sizeof(EcMsgHeader) + CRC_LEN )
			{
				char senderId = get_ec_player_id(from);
				if( senderId)
				{
					// send NEGACK frame
					char replyMsg[sizeof(EcMsgHeader) + CRC_LEN];
					((EcMsgHeader *)replyMsg)->init(NEGACK, self_ec_player_id, wait_to_receive[senderId-1]);
					*((CRC_TYPE *)(replyMsg + sizeof(EcMsgHeader))) = crc8((unsigned char *)replyMsg, sizeof(EcMsgHeader));
					mp_ptr->send( dp_id[senderId-1], replyMsg, sizeof(replyMsg) );
				}
#if (defined(DEBUG) && DEBUG_LOG_LEVEL >= 2)
				DEBUG_LOG("ec_remote : long packet corrupted" );
#endif
			}
			else
			{
				// it is probably, ACKNOW/ NEGACK frame, discard it 
#if (defined(DEBUG) && DEBUG_LOG_LEVEL >= 2)
				DEBUG_LOG("ec_remote : short packet corrupted" );
#endif
			}
		}
	}

	// ------ retransmit any un-acked and time-out-ed frame -------//
	clear_acked_frame();
	re_transmit();

	mp_ptr->after_send();	// re_transmit() will call after_send
}


int ErrorControl::is_send_empty()
{
	return ( send_head == send_tail );
}

int ErrorControl::is_send_full()
{
	return ( send_tail + 1 == send_head || send_tail + 1 == send_head + MAX_QUEUE );
}


int ErrorControl::send_queue_space()
{
	// the queue can hold at most MAX_QUEUE-1 item
	return send_tail >= send_head ? MAX_QUEUE-1 - (send_tail - send_head) : send_head - send_tail -1 ;
}


// return frameId
int ErrorControl::en_send_queue()
{
	if( is_send_full() )
		return -1;
	else
	{
		char f = send_tail;
		err_when( f < 0 || f >= MAX_QUEUE);
		inc_frame_id( send_tail );
		return f;
	}
}

// free queue Id
void ErrorControl::de_send_queue()
{
#if (defined(DEBUG) && DEBUG_LOG_LEVEL >= 3)
	debugStr = "ec_remote.de_send_queue(), frame:";
	debugStr += send_head;
	DEBUG_LOG(debugStr);
#endif
	inc_frame_id(send_head);
}

int ErrorControl::is_recv_empty()
{
	return (recv_head == recv_tail);
}

int ErrorControl::is_recv_full()
{
	return (recv_tail+1 == recv_head || recv_tail+1 == recv_head + MAX_RECV_QUEUE );
}


int ErrorControl::recv_queue_space()
{
	// the queue can hold at most MAX_RECV_QUEUE-1 item
	return recv_tail >= recv_head ? MAX_RECV_QUEUE-1 - (recv_tail - recv_head) : recv_head - recv_tail -1 ;
}


void ErrorControl::en_recv_queue(void *dataPtr, DWORD dataLen)
{
	if( is_recv_full() )
	{
		err_here();			// receive queue is full
	}
	else
	{
		char f = recv_tail;
		err_when( recv_tail < 0 || recv_tail >= MAX_RECV_QUEUE);
		if( ++recv_tail >= MAX_RECV_QUEUE )
			recv_tail = 0;
		receive_queue[f].clear();
		memcpy( receive_queue[f].reserve(dataLen), dataPtr, dataLen);
	}
}

void ErrorControl::de_recv_queue()
{
	if( ++recv_head >= MAX_RECV_QUEUE )
		recv_head = 0;
}

int ErrorControl::is_waiting_ack(char ecPlayerId, char frameId)
{
	// true if frameId is between send_head (inclusive) and send_tail (non-inclusive)
	return is_between( send_head, frameId, send_tail );
}

void ErrorControl::set_ack(char ecPlayerId, char frameId)
{
	ack_flag[frameId][ecPlayerId-1] = 1;
}

void ErrorControl::clear_ack(char frameId)
{
	memset( ack_flag[frameId], 0, MAX_PLAYER );
}

void ErrorControl::mark_send_time(char frameId, DWORD duration)
{
	send_time[frameId] = m.get_time();
	re_send_after[frameId] = duration;
}

// larger the promptFactor, earlier to re-send
int ErrorControl::need_re_send(char frameId, int promptFactor)
{
	return ((m.get_time() - send_time[frameId]) * promptFactor) >= re_send_after[frameId];
	// do not use m.get_time() >= re_send_after[frameId] + send_time[frameId]
	// assume m.get_time() may count again from zero
}


int ErrorControl::are_all_acked(char frameId)
{
	for( char ecPlayerId = 1; ecPlayerId <= MAX_PLAYER; ++ecPlayerId )
	{
		if( dp_id[ecPlayerId-1] && !ack_flag[frameId][ecPlayerId-1] )
			return 0;
	}
	return 1;
}

void ErrorControl::clear_acked_frame()
{
	err_when( send_head < 0 || send_head >= MAX_QUEUE );
	for( ; !is_send_empty() && are_all_acked(send_head); de_send_queue() );
}

int ErrorControl::is_waiting_receive(char ecPlayerId, char frameId)
{
	err_when( frameId < 0 || frameId >= MAX_QUEUE );
	// the first waiting frame is wait_to_receive, but may receive MAX_QUEUE-2
	// e.g wait_to_receive is 0, then 1 to MAX_QUEUE/2-1 are also acceptable
	return is_between(wait_to_receive[ecPlayerId-1], frameId, 
		(wait_to_receive[ecPlayerId-1] + MAX_QUEUE/2) % MAX_QUEUE) 
		&& !recv_flag[ecPlayerId-1][frameId];
}

void ErrorControl::set_recv_flag(char ecPlayerId, char frameId)
{
	recv_flag[ecPlayerId-1][frameId] = 1;
}


void ErrorControl::clear_recv_flag(char ecPlayerId, char frameId)
{
	recv_flag[ecPlayerId-1][frameId] = 0;
}


void ErrorControl::re_transmit(int promptFactor)
{
	for( char f = send_head; f != send_tail; inc_frame_id(f) )
	{
		if( !are_all_acked(f) && need_re_send(f, promptFactor) )
		{
			// count no. of remote player to re_send
			int resendSuccess = 0;
			int resendFail = 0;
			char *ecMsg = send_queue[f].queue_buf;
			DWORD ecMsgLen = send_queue[f].length();

			for( char ecPlayerId = 1; ecPlayerId <= MAX_PLAYER; ++ecPlayerId )
			{
				// resend to specific remote player
				if( dp_id[ecPlayerId-1] && !ack_flag[f][ecPlayerId-1] )
				{
#if (defined(DEBUG) && DEBUG_LOG_LEVEL >= 2)
					debugStr = "ec.remote : time-out retransmit frame ";
					debugStr += f;
					debugStr += " to ";
					debugStr += ecPlayerId;
					DEBUG_LOG(debugStr);
#endif
					if(  mp_ptr->send(dp_id[ecPlayerId-1], ecMsg, ecMsgLen))
						resendSuccess++;
					else
						resendFail++;
				}

				if( resendSuccess > 0)
				{
					if( resendFail > 0)
					{
						// some resend fail, mark short resend time
						mark_send_time(f, SHORT_TIME_OUT);
					}
					else
					{
						// all resend success, mark longer resend time
						mark_send_time(f, TIME_OUT );
					}

					// mark the func_id of the message RE_SEND
					((EcMsgHeader *)ecMsg)->func_id = RE_SEND;

					// recalculate CRC
					*((CRC_TYPE *) (ecMsg + ecMsgLen - CRC_LEN )) = crc8((unsigned char *)ecMsg, ecMsgLen - CRC_LEN);

				}
				else if( resendFail > 0)
				{
					// all fail, mark a shorter time
					// mark send time, mark a shorter time
					mark_send_time(f, SHORT_TIME_OUT);
				}
			}
		}
	}

	mp_ptr->after_send();
}
