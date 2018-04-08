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

//Filename    : OREMOTE2.CPP
//Description : Object Remote - part 2

#define DEBUG_LOG_LOCAL 1
#include <ALL.h>
#include <OVGA.h>
#include <OSTR.h>
#include <OFONT.h>
#include <OSYS.h>
#include <OWORLD.h>
#include <OPOWER.h>
#include <ONATION.h>
#include <OREMOTE.h>
#include <OERRCTRL.h>
#include <OLOG.h>

//---------- Define static functions ----------//

// static int validate_queue(char* queueBuf, int queuedSize);

//-------- Begin of function Remote::new_msg ---------//
//
// Allocate a RemoteMsg with the specified data size.
//
// <short> msgId       - id. of the message.
// <int>   dataSize    - the data size of the RemoteMsg needed (size of RemoteMsg::data_buf)
//
// Data structure:
//
// <short> size of RemoteMsg + <RemoteMsg> RemoteMsg body
// ^                           ^
// | Allocate starts here      | return variable here to the client function
//
RemoteMsg* Remote::new_msg(uint32_t msgId, int dataSize)
{

   int msgSize = sizeof(uint32_t) + dataSize;
	// <uint32_t> is for the message id.

   char* shortPtr;

   //--- use common_msg_buf, if the requested one is not bigger than common_msg_buf ---//
	int reqSize = sizeof(short) + msgSize;
	// <short> for length

   if(reqSize <= COMMON_MSG_BUF_SIZE )
	{
      shortPtr = common_msg_buf;
   }
   else //---- allocate a new RemoteMsg if the requested one is bigger than common_msg_buf ----//
   {
      shortPtr = (char *)mem_add( reqSize );
   }

   //---------- return RemoteMsg now -----------//

   *(short *)shortPtr = msgSize;
	shortPtr += sizeof(short);

	RemoteMsg* remoteMsgPtr = (RemoteMsg*) shortPtr;

	remoteMsgPtr->id = msgId;

	return remoteMsgPtr;
}
//--------- End of function Remote::new_msg ---------//


//-------- Begin of function Remote::free_msg ---------//
//
// Free a RemoteMsg previously allocated by remote_msg().
//
// Data structure:
//
// <short> size of RemoteMsg + <RemoteMsg> RemoteMsg body
// ^                           ^
// | Allocate starts here      | return variable here to the client function
//
void Remote::free_msg(RemoteMsg* remoteMsgPtr)
{
   char* memPtr = (char *)remoteMsgPtr - sizeof(short);
	// include size of message, sizeof MPMSG_FREE_MSG

   if( memPtr != common_msg_buf )      // the real allocation block start 2 bytes before remoteMsgPtr
      mem_del( memPtr );
}
//--------- End of function Remote::free_msg ---------//


//-------- Begin of function Remote::send_msg ---------//
//
// <RemoteMsg*> remoteMsgPtr - the pointer to RemoteMsg to be sent.
// [DPID]       receiverId   - send to whom, 0 for everybody
//                             (default : 0)
//
// <short> size of RemoteMsg + <RemoteMsg> RemoteMsg body
// ^                           ^
// | Allocate starts here      | return variable here to the client function
//
void Remote::send_msg(RemoteMsg* remoteMsgPtr, uint32_t receiverId)
{
	if( handle_vga_lock )
		vga_front.temp_unlock();

	// structure : <short>, <RemoteMsg>
   char* memPtr = (char*)remoteMsgPtr - sizeof(short);   // the preceeding <short> is the size of RemoteMsg
   int msgSize = *(short *)memPtr + sizeof(short);

	// mp_ptr->send( receiverId, memPtr, msgSize );
	ec_remote.send( ec_remote.get_ec_player_id(receiverId), memPtr, msgSize);

   packet_send_count++;

   if( handle_vga_lock )
      vga_front.temp_restore_lock();
}
//--------- End of function Remote::send_msg ---------//


//-------- Begin of function Remote::send_free_msg ---------//
//
// <RemoteMsg*> remoteMsgPtr - the pointer to RemoteMsg to be sent.
// [DPID]       receiverId   - send to whom, 0 for everybody
//                             (default : 0)
//
// <short> size of RemoteMsg + <RemoteMsg> RemoteMsg body
// ^                           ^
// | Allocate starts here      | return variable here to the client function
//
void Remote::send_free_msg(RemoteMsg* remoteMsgPtr, uint32_t receiverId)
{
   if( handle_vga_lock )
		vga_front.temp_unlock();

   char* memPtr = (char*)remoteMsgPtr - sizeof(short);   // the preceeding <short> is the size of RemoteMsg
   int msgSize = *(short *)memPtr + sizeof(short);

	// mp_ptr->send( receiverId, memPtr, msgSize );
	ec_remote.send( ec_remote.get_ec_player_id(receiverId), memPtr, msgSize);

   packet_send_count++;

   if( memPtr != common_msg_buf )      // the real allocation block start 2 bytes before remoteMsgPtr
      mem_del( memPtr );

   if( handle_vga_lock )
      vga_front.temp_restore_lock();
}
//--------- End of function Remote::send_free_msg ---------//



//-------- Begin of function Remote::new_send_queue_msg ---------//
//
// Add the message into the sending queue.
//
// <short> msgId    - id. of the message.
// <int>   dataSize - the data size of the RemoteMsg needed (size of RemoteMsg::data_buf)
//
// Queue data structure:
//
// <1st msg size> + <1st msg body> + <2nd msg size> + ...
//
// return : <char*> the pointer RemoteMsg::data_buf of the allocated message
//
char* Remote::new_send_queue_msg(uint32_t msgId, int dataSize)
{
	char *dataPtr = send_queue[0].reserve(sizeof(short) + sizeof(uint32_t) + dataSize);

	// ----- put the size of message ------//
	*(short *)dataPtr = sizeof(uint32_t) + dataSize;
	dataPtr += sizeof(short);

	// ----- put the message id --------//
	((RemoteMsg *)dataPtr)->id  = msgId;

	return ((RemoteMsg *)dataPtr)->data_buf;
}
//--------- End of function Remote::new_send_queue_msg ---------//


//-------- Begin of function Remote::send_queue_now ---------//
//
// Send out all send_queued message.
//
// [DPID] receiverId - send to whom, 0 for everybody
//                     (default: 0)
//
// return : <int> 1 - sent successfully, the message has been received
//                    by the receivers.
//                0 - not successful.
//
int Remote::send_queue_now(uint32_t receiverId)
{
   if( send_queue[0].length() ==0 )
      return 1;

	if( !send_queue[0].validate_queue() )
		err.run( "Queue Corrupted, Remote::send_queue_now()" );

	if( handle_vga_lock )
		vga_front.temp_unlock();

	//----------------------------------------------//
	int sendFlag = 1;
	//if(!mp_ptr->send( receiverId, send_queue[0].queue_buf, send_queue[0].length()))
	if(ec_remote.send( ec_remote.get_ec_player_id(receiverId), send_queue[0].queue_buf, send_queue[0].length()) <= 0 )
		sendFlag = 0;
	packet_send_count++;

	//---------------------------------------------//

	if( handle_vga_lock )
		vga_front.temp_restore_lock();

	return sendFlag;
}
//--------- End of function Remote::send_queue_now ---------//


//-------- Begin of function Remote::append_send_to_receive ---------//
//
// Append all send queues to the receiving queue.
//
void Remote::append_send_to_receive()
{
   if( send_queue[0].length() ==0 )
		return;

	int n;
	for( n = 0; n < RECEIVE_QUEUE_BACKUP; ++n)
	{
		if( send_frame_count[0] == receive_frame_count[n])
		{
			receive_queue[n].append_queue(send_queue[0]);
			if( !receive_queue[n].validate_queue() )
				err.run( "Queue Corrupted, Remote::append_send_to_receive()-3" );
			break;
		}
	}
	err_when( n >= RECEIVE_QUEUE_BACKUP );			// not found

}
//--------- End of function Remote::append_send_to_receive ---------//



//-------- Begin of function Remote::poll_msg ---------//

int Remote::poll_msg()
{
   if ( !is_enable() || poll_msg_flag <= 0)
      return 0;

	//--------------------------------------------//

   int        receivedFlag=0;
   uint32_t   msgListSize;
   int        loopCount=0;

   if( handle_vga_lock )
      vga_front.temp_unlock();

	ec_remote.yield();

	char *recvBuf;
	// DPID from;
	char from;
	uint32_t recvLen;

	// while( (recvBuf = mp_ptr->receive(&from, &recvLen)) != NULL)
	while( (recvBuf = ec_remote.receive(&from, &recvLen)) != NULL )
	{
		err_when(++loopCount > 1000 );
		msgListSize = recvLen;

		//-------- received successfully ----------//
/*
		if( !power.enable_flag )		// power.enable_flag determines whether the multiplayer game has started or not
		{
			int validateLen = receive_queue[0].length();
			memcpy( receive_queue[0].reserve(msgListSize), recvBuf, msgListSize );
			if( !receive_queue[0].validate_queue(validateLen) )
				err.run( "Queue Corrupted, Remote::poll_msg()-99" );

			receivedFlag=1;
		}
		else
*/
		err_when( msgListSize < sizeof(short) + sizeof(uint32_t) );
		RemoteMsg *rMsg = (RemoteMsg *)(recvBuf + sizeof(short) );
		int	queueToCopy = -1;

		//--- if message id !=MSG_QUEUE_HEADER, this packet is sent by send_free_msg(), not by send_queue_now() ---//

		if( rMsg->id != MSG_QUEUE_HEADER )
		{
			// DEBUG_LOG("begin MSG_xxxx received");
			// DEBUG_LOG(rMsg->id);
			// DEBUG_LOG(sys.frame_count);
			// DEBUG_LOG("end MSG_xxxx received");
			queueToCopy = 0;
		}
		else
		{

			// ------- find which receive queue to hold the message -----//

			uint32_t senderFrameCount = *(uint32_t *)rMsg->data_buf;
			for(int n = 0; n < RECEIVE_QUEUE_BACKUP; ++n )
			{
				if( senderFrameCount == receive_frame_count[n] )
				{
					queueToCopy = n;
					break;
				}
			}
			// DEBUG_LOG("begin MSG_QUEUE_HEADER received");
			// DEBUG_LOG(senderFrameCount);
			// DEBUG_LOG(sys.frame_count);
			// DEBUG_LOG(n);
			// DEBUG_LOG("end MSG_QUEUE_HEADER received");
		}

		if( queueToCopy >= 0 && queueToCopy < RECEIVE_QUEUE_BACKUP )
		{
			RemoteQueue &rq = receive_queue[queueToCopy];
			int validateLen = rq.length();
			memcpy( rq.reserve(msgListSize), recvBuf, msgListSize );
			if( !rq.validate_queue(validateLen) )
				err.run( "Queue Corrupted, Remote::poll_msg()-2" );
		}
		else
		{
			// discard the frame in non-debug mode
			DEBUG_LOG("message is discard" );
			if( rMsg->id != MSG_QUEUE_HEADER )
			{
				DEBUG_LOG(rMsg->id);
			}
			else
			{
				uint32_t senderFrameCount = *(uint32_t *)rMsg->data_buf;
				DEBUG_LOG("MSG_QUEUE_HEADER message");
				DEBUG_LOG(senderFrameCount);
			}

		}

		ec_remote.de_recv_queue();

		receivedFlag = 1;

		packet_receive_count++;
	}

	if( handle_vga_lock )
		vga_front.temp_restore_lock();

	return receivedFlag;
}

//--------- End of function Remote::poll_msg ---------//


//-------- Begin of function Remote::process_receive_queue ---------//
//
// Process messages in the receive queue.
//
void Remote::process_receive_queue()
{
	if( !process_queue_flag )			// avoid sys.yield()
		return;

	int processFlag;
	int loopCount=0;

	disable_poll_msg();
	
	RemoteQueue &rq = receive_queue[0];
	RemoteQueueTraverse rqt(rq);

	if( connectivity_mode == MODE_REPLAY )
	{
		if( !replay.read_queue(&rq) )
			connectivity_mode = MODE_REPLAY_END;
	}
	else
		replay.write_queue(&rq);

	if( !rq.validate_queue() )
		err.run( "Queue corrupted, Remote::process_receive_queue()" );

	//---- if not started yet, process the message without handling the message sequence ----//

	if( !power.enable_flag )
	{
		for( rqt.traverse_set_start(); !rqt.traverse_finish(); rqt.traverse_next() )
		{
			err_when( ++loopCount > 1000 );
			RemoteMsg* remoteMsgPtr = rqt.get_remote_msg();
			remoteMsgPtr->process_msg();
		}
	}
	else //--------- process in the order of nation recno --------//
	{
		LOG_BEGIN;
		for( int nationRecno=1 ; nationRecno<=nation_array.size() ; ++nationRecno )
		{
			if( nation_array.is_deleted(nationRecno) || nation_array[nationRecno]->nation_type==NATION_AI )
				continue;

			nation_processing = nationRecno;
			processFlag=0;

			//-------- scan through the message queue --------//

			loopCount = 0;
			for( rqt.traverse_set_start(); !rqt.traverse_finish(); rqt.traverse_next() )
			{
				err_when( ++loopCount > 1000 );
				RemoteMsg* remoteMsgPtr = rqt.get_remote_msg();

				//--- check if this message indicates the start of a new message queue ---//

				if( remoteMsgPtr->id == MSG_QUEUE_HEADER )
				{
					short msgNation = *(short *)(&remoteMsgPtr->data_buf[sizeof(uint32_t)]);

					//--- if this is the message queue of the nation we should process now ----//

					if(msgNation==nationRecno )   // struct of data_buf: <uint32_t> frameCount + <short> nationRecno
					{
						processFlag = 1;
					}
					else  //--- if this is a message queue that should be processed now ---//
					{
						if( processFlag )  //--- if we were previously processing the right queue, now the processing is complete by this point ---//
							break;			// message on next nation is met, stop this nation
					}
				}
				else if( remoteMsgPtr->id == MSG_QUEUE_TRAILER )
				{
					short msgNation = *(short *)remoteMsgPtr->data_buf;

					//--- if this is the message queue of the nation we should process now ----//

					if(msgNation==nationRecno )   // struct of data_buf:<short> nationRecno
						break;			// end of that nation
				}
				else
				{
					//--- if this is the message queue of the nation we should process now ----//

					if( processFlag )
					{
#if defined(DEBUG) && defined(ENABLE_LOG)
						String logStr("begin process remote message id:");
						logStr += (long) remoteMsgPtr->id;
						logStr += " of nation ";
						logStr += nation_processing;
						LOG_MSG(logStr);
#endif                  
						remoteMsgPtr->process_msg();
						LOG_MSG("end process remote message");
						LOG_MSG(misc.get_random_seed());
					}
				}
			}
			nation_processing = 0;
		}
		LOG_END;
	}

	//---------- clear receive_queue[0] and shift from next queue ------//
	rq.clear();

	int n;
	for( n = 1; n < RECEIVE_QUEUE_BACKUP; ++n)
	{
		receive_queue[n-1].swap(receive_queue[n]);
		receive_frame_count[n-1] = receive_frame_count[n];
	}
	receive_frame_count[n-1]++; 

	enable_poll_msg();
}
//--------- End of function Remote::process_receive_queue ---------//


//-------- Begin of function Remote::process_specific_msg ---------//
//
// Pre-process a specific message type.
//
void Remote::process_specific_msg(uint32_t msgId)
{
	if( !process_queue_flag )			// avoid sys.yield()
		return;

	int loopCount=0;

	disable_poll_msg();
	RemoteQueue &rq = receive_queue[0];
	RemoteQueueTraverse rqt(rq);

	if( !rq.validate_queue() )
		err.run( "Queue corrupted, Remote::process_specific_msg()" );

	for( rqt.traverse_set_start(); !rqt.traverse_finish(); rqt.traverse_next() )
	{
		err_when( ++loopCount > 1000 );
		RemoteMsg* remoteMsgPtr = rqt.get_remote_msg();
		if( remoteMsgPtr->id == msgId )
		{
			remoteMsgPtr->process_msg();
			remoteMsgPtr->id = 0;
		}
	}

	enable_poll_msg();
}
//--------- End of function Remote::process_specific_msg ---------//


/*
//-------- Begin of function Remote::validate_queue ---------//
//
// Pre-process a specific message type.
//
static int validate_queue(char* queueBuf, int queuedSize)
{
   char*      queuePtr = queueBuf;
   RemoteMsg* remoteMsgPtr;
   int        msgSize, processedSize=0;
   int        loopCount=0;

   while( processedSize < queuedSize )
	{
		msgSize = *((short*)queuePtr);

		remoteMsgPtr = (RemoteMsg*) (queuePtr + sizeof(short));

		if( remoteMsgPtr->id )
		{
			if( remoteMsgPtr->id<FIRST_REMOTE_MSG_ID || remoteMsgPtr->id>LAST_REMOTE_MSG_ID )
				return 0;
		}

		queuePtr       += sizeof(short) + msgSize;
		processedSize  += sizeof(short) + msgSize;

		err_when( loopCount++ > 10000 );    // deadloop
	}

	return 1;
}
//--------- End of function Remote::validate_queue ---------//
*/


//-------- Begin of function Remote::copy_send_to_backup ---------//
//
// Copy the whole send queue to the backup queue.
//
void Remote::copy_send_to_backup()
{
	// ------ shift send_queue ---------//

	send_queue[SEND_QUEUE_BACKUP-1].clear();

	for(int n = SEND_QUEUE_BACKUP-1; n > 1; --n)
	{
		send_queue[n].swap(send_queue[n-1]);
		send_frame_count[n] = send_frame_count[n-1];
	}
	// now send_queue[1] is empty.

	send_queue[1] = send_queue[0];
	send_frame_count[1] = send_frame_count[0];
}
//--------- End of function Remote::copy_send_to_backup ---------//


//-------- Begin of function Remote::send_backup_now ---------//
//
// Send out the backup queue now.
//
// [DPID] receiverId - send to whom, 0 for everybody
//                     (default: 0)
//
// <int> requestFrameCount - need to send the backup buffer of the requested frame count
//
// return : <int> 1 - sent successfully, the message has been received
//                    by the receivers.
//                0 - not successful.
//
int Remote::send_backup_now(uint32_t receiverId, uint32_t requestFrameCount)
{
   //------ determine which backup buffer to send -----//

	font_san.disp( ZOOM_X2-100, 4, "", ZOOM_X2);

	for( int n = 1; n < SEND_QUEUE_BACKUP; ++n)
	{
		if( requestFrameCount == send_frame_count[n] )
		{
			//---------- if nothing to send -------------//

			int retFlag = 1;
			if( send_queue[n].length() > 0 )
			{
				if( handle_vga_lock )
					vga_front.temp_unlock();
				// retFlag = mp_ptr->send(receiverId, send_queue[n].queue_buf, send_queue[n].length());
				retFlag = ec_remote.send(ec_remote.get_ec_player_id(receiverId), send_queue[n].queue_buf, send_queue[n].length()) > 0;
				packet_send_count++;
				if( handle_vga_lock )
					vga_front.temp_restore_lock();
			}
			return retFlag;
		}
	}

	if( requestFrameCount < send_frame_count[SEND_QUEUE_BACKUP-1])
		err.run( "requestFrameCount:%u < backup2_frame_count:%u", requestFrameCount, send_frame_count[SEND_QUEUE_BACKUP-1] );
	return 0;
}
//--------- End of function Remote::send_backup_now ---------//


//-------- Begin of function Remote::init_send_queue ---------//
//
// Initialize the header of the sending queue.
//
// <int>   frameCount  - frame count of this queue
// <short> nationCount - nation recno of the sender
//
void Remote::init_send_queue(uint32_t frameCount, short nationRecno)
{
	send_queue[0].clear();

	// put into the queue : <message length>, MSG_QUEUE_HEADER, <frameCount>, <nationRecno>

	int msgSize = sizeof(uint32_t) + sizeof(uint32_t) + sizeof(short);
	char *sendPtr = send_queue[0].reserve(sizeof(short) + msgSize);

	*(short *)sendPtr = msgSize;
	 sendPtr += sizeof(short);
	*(uint32_t *)sendPtr = MSG_QUEUE_HEADER;
	 sendPtr += sizeof(uint32_t);
	*(uint32_t *)sendPtr = send_frame_count[0] = next_send_frame(nationRecno, frameCount + process_frame_delay);
	 sendPtr += sizeof(uint32_t);
	*(short *)sendPtr = nationRecno;
	 sendPtr += sizeof(short);
}
//--------- End of function Remote::init_send_queue ----------//


//-------- Begin of function Remote::init_receive_queue ---------//
//
// Initialize the header of the receiving queue,
// call this only when power is just enabled
//
// <int>   frameCount  - frame count of this queue
// <short> nationCount - nation recno of the receiveer
//
void Remote::init_receive_queue(uint32_t frameCount)
{
	int n;

	for(n = 0; n < RECEIVE_QUEUE_BACKUP; ++n)
	{
		receive_queue[n].clear();
		receive_frame_count[n] = n+frameCount;
	}

	for(n = 0; n < process_frame_delay; ++n)
	{
		// nations are not created yet, put MSG_QUEUE_HEADER/MSG_NEXT_FRAME and MSG_QUEUE_TRAILER
		// even though the nation will not exist
		for(short nationRecno = 1; nationRecno <= MAX_NATION; ++nationRecno)
		{
			//if( nation_array.is_deleted(nationRecno) || 
			//	nation_array[nationRecno]->nation_type==NATION_AI )
			//	continue;

			char *receivePtr;
			int msgSize;

			// put into the queue : <message length>, MSG_QUEUE_HEADER, <frameCount>, <nationRecno>

			msgSize = sizeof(uint32_t) + sizeof(uint32_t) + sizeof(short);
			receivePtr = receive_queue[n].reserve(sizeof(short) + msgSize);
			*(short *)receivePtr = msgSize;
			 receivePtr += sizeof(short);
			*(uint32_t *)receivePtr = MSG_QUEUE_HEADER;
			 receivePtr += sizeof(uint32_t);
			*(uint32_t *)receivePtr = frameCount + n;
			 receivePtr += sizeof(uint32_t);
			*(short *)receivePtr = nationRecno;
			 receivePtr += sizeof(short);

			// put into the queue : <message length>, MSG_NEXT_FRAME, <nationRecno>

			msgSize = sizeof(uint32_t) + sizeof(short);
			receivePtr = receive_queue[n].reserve(sizeof(short) + msgSize);

			*(short *)receivePtr = msgSize;
			 receivePtr += sizeof(short);
			*(uint32_t *)receivePtr = MSG_NEXT_FRAME;
			 receivePtr += sizeof(uint32_t);
			*(short *)receivePtr = nationRecno;
			 receivePtr += sizeof(short);

			// put into the queue : <message length>, MSG_QUEUE_TRAILER, <nationRecno>

			msgSize = sizeof(uint32_t) + sizeof(short);
			receivePtr = receive_queue[n].reserve(sizeof(short) + msgSize);

			*(short *)receivePtr = msgSize;
			 receivePtr += sizeof(short);
			*(uint32_t *)receivePtr = MSG_QUEUE_TRAILER;
			 receivePtr += sizeof(uint32_t);
			*(short *)receivePtr = nationRecno;
			 receivePtr += sizeof(short);

		}
	}
}
//--------- End of function Remote::init_receive_queue ----------//


//--------- Begin of function Remote::init_start_mp ----------//
void Remote::init_start_mp()
{
	int n;
	for( n = 0; n < SEND_QUEUE_BACKUP; ++n)
	{
		send_queue[n].clear();
		send_frame_count[n] = 0;
	}

	for( n = 0; n < RECEIVE_QUEUE_BACKUP; ++n)
	{
		receive_queue[n].clear();
		receive_frame_count[n] = 0;
	}
}
//--------- End of function Remote::init_start_mp ----------//


//--------- Begin of function Remote::enable_process_queue --------//
void Remote::enable_process_queue()
{
	process_queue_flag = 1;
}
//--------- End of function Remote::enable_process_queue --------//


//--------- Begin of function Remote::disable_process_queue --------//
void Remote::disable_process_queue()
{
	process_queue_flag = 0;
}
//--------- End of function Remote::disable_process_queue --------//


//-------- Begin of function Remote::enable_poll_msg ---------//
void Remote::enable_poll_msg()
{
	poll_msg_flag = 1;
}
//-------- End of function Remote::enable_poll_msg ---------//


//-------- Begin of function Remote::disable_poll_msg ---------//
void Remote::disable_poll_msg()
{
	poll_msg_flag = 0;
}
//-------- End of function Remote::disable_poll_msg ---------//
