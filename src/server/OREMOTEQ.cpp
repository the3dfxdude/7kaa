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

// Filename    : OREMOTEQ.CPP
// Description : remote message queue

#include <ALL.h>
#include <OREMOTEQ.h>
#include <OREMOTE.h>
#include <stdint.h>


// structure of queue_buf in RemoteQueue :
// <1st message length (short), not including this 2 bytes> <1st message content>
// <2nd message length> <2nd message content> ...

// ------- begin of function RemoteQueue::RemoteQueue -------//
RemoteQueue::RemoteQueue() : VLenQueue()
{
}


RemoteQueue::RemoteQueue(int s) : VLenQueue(s)
{
}

RemoteQueue::RemoteQueue(RemoteQueue &q) : VLenQueue(q)
{
}
// ------- end of function RemoteQueue::RemoteQueue -------//


// ------- begin of function RemoteQueue::~RemoteQueue -------//
RemoteQueue::~RemoteQueue()
{
}
// ------- end of function RemoteQueue::~RemoteQueue -------//

// ------- begin of function RemoteQueue::validate_queue -------//
int RemoteQueue::validate_queue(int start)
{
	int loopCount = 0;
	RemoteQueueTraverse rqt(*this, start);
	for( rqt.traverse_set_start(start); !rqt.traverse_finish(); rqt.traverse_next() )
	{
		RemoteMsg *remoteMsgPtr = rqt.get_remote_msg();
		if( remoteMsgPtr->id )
		{
			if( remoteMsgPtr->id<FIRST_REMOTE_MSG_ID || remoteMsgPtr->id>LAST_REMOTE_MSG_ID )
				return 0;
		}
		err_when( ++loopCount > 200 );
	}

	return 1;
}
// ------- end of function RemoteQueue::validate_queue -------//



// ------- begin of function RemoteQueueTraverse::RemoteQueueTraverse -------//
RemoteQueueTraverse::RemoteQueueTraverse( RemoteQueue &rq, int start ) :
	remote_queue(rq), offset(start)
{
}
// ------- end of function RemoteQueueTraverse::RemoteQueueTraverse -------//


// ------- begin of function RemoteQueueTraverse::traverse_set_start -------//
void RemoteQueueTraverse::traverse_set_start(int start)
{
	offset = start;
}
// ------- end of function RemoteQueueTraverse::traverse_set_start -------//


// ------- begin of function RemoteQueueTraverse::traverse_finish -------//
bool RemoteQueueTraverse::traverse_finish()
{
	return offset >= remote_queue.queued_size;
}
// ------- end of function RemoteQueueTraverse::traverse_finish -------//


// ------- begin of function RemoteQueueTraverse::traverse_next -------//
void RemoteQueueTraverse::traverse_next()
{
	err_when( traverse_finish() );
	uint16_t msgLen = *(uint16_t *)(remote_queue.queue_buf + offset);
	err_when( msgLen >= 0x8000 );
	offset +=  msgLen + sizeof(uint16_t);
}
// ------- end of function RemoteQueueTraverse::traverse_next -------//


// ------- begin of function RemoteQueueTraverse::get_remote_msg -------//
RemoteMsg *RemoteQueueTraverse::get_remote_msg(uint16_t * msgLen)
{
	char *retPtr = remote_queue.queue_buf + offset;
	if (msgLen) *msgLen = *(uint16_t *)retPtr;
	retPtr += sizeof(uint16_t);
	return (RemoteMsg *)retPtr;
}
// ------- end of function RemoteQueueTraverse::get_remote_msg -------//

