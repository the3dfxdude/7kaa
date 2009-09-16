// Filename    : OREMOTEQ.CPP
// Description : remote message queue

#include <ALL.H>
#include <OREMOTEQ.H>
#include <OREMOTE.H>

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

/*
//
// try not to use RemoteQueue::traverse_set_start,
// RemoteQueue::traverse_finish and RemoteQueue::traverse_next
// use RemoteQueueTraverse::traverse_set_start instead
// see RemoteQueue::validate to see how to use RemoteQueueTraverse
//
// ------- begin of function RemoteQueue::traverse_set_start -------//
void RemoteQueue::traverse_set_start(int start)
{
	queue_ptr = queue_buf + start;
}
// ------- end of function RemoteQueue::traverse_set_start -------//


// ------- begin of function RemoteQueue::traverse_finish -------//
int RemoteQueue::traverse_finish()
{
	err_when( (unsigned)queue_ptr < (unsigned)queue_buf ); 
	return queue_ptr - queue_buf >= queued_size;
}
// ------- end of function RemoteQueue::traverse_finish -------//


// ------- begin of function RemoteQueue::traverse_next -------//
// queue_ptr points to the message length
void RemoteQueue::traverse_next()
{
	err_when( queue_ptr - queue_buf >= queued_size );
	err_when( *(unsigned short *)queue_ptr >= 0x8000 );
	queue_ptr += *(unsigned short *)queue_ptr + sizeof(unsigned short);	// skip the message length as well
}
// ------- end of function RemoteQueue::traverse_next -------//


// ------- begin of function RemoteQueue::get_remote_msg -------//
RemoteMsg *RemoteQueue::get_remote_msg(short *msgLen)
{
	if( traverse_finish() )
	{
		if(msgLen)
			msgLen = 0;
		return NULL;
	}
	else
	{
		char *ptr1 = queue_ptr + sizeof(short);
		if(msgLen)
			msgLen = (short *)queue_ptr;
		return (RemoteMsg *)ptr1;
	}
}
// ------- end of function RemoteQueue::get_remote_msg -------//
*/

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
int RemoteQueueTraverse::traverse_finish()
{
	return offset >= remote_queue.queued_size;
}
// ------- end of function RemoteQueueTraverse::traverse_finish -------//


// ------- begin of function RemoteQueueTraverse::traverse_next -------//
void RemoteQueueTraverse::traverse_next()
{
	err_when( traverse_finish() );
	unsigned short msgLen = *(unsigned short *)(remote_queue.queue_buf + offset);
	err_when( msgLen >= 0x8000 );
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
