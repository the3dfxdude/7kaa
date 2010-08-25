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
#include <dbglog.h>

DBGLOG_DEFAULT_CHANNEL(Network);

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

		if (++loopCount > 200) ERR("[RemoteQueue::validate_queue] too many messages\n");
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
	if (traverse_finish()) {
		ERR("[RemoteQueueTraverse::traverse_next] traverse out of bounds\n");
		return;
	}

	unsigned short msgLen = *(unsigned short *)(remote_queue.queue_buf + offset);

	if (msgLen >= 0x8000) {
		ERR("[RemoteQueueTraverse::traverse_next] invalid message length\n");
		return;
	}

	offset +=  msgLen + sizeof(unsigned short);
}
// ------- end of function RemoteQueueTraverse::traverse_next -------//


// ------- begin of function RemoteQueueTraverse::get_remote_msg -------//
RemoteMsg *RemoteQueueTraverse::get_remote_msg(short *msgLen)
{
	char *retPtr = remote_queue.queue_buf + offset;
	if( msgLen )
	{
		*msgLen = *(unsigned short *)retPtr;
	}
	retPtr += sizeof(unsigned short);
	return (RemoteMsg *)retPtr;
}
// ------- end of function RemoteQueueTraverse::get_remote_msg -------//
