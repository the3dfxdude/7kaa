// Filename    : OVQUEUE.CPP
// Description : variable length queue


#include <OVQUEUE.H>
#include <ALL.H>

#define QUEUE_SIZE_INC 0x1000

// -------- begin of function VLenQueue::VLenQueue ----------//
VLenQueue::VLenQueue() : queue_buf(NULL), queue_buf_size(0), queued_size(0), queue_ptr(NULL)
{
}


VLenQueue::VLenQueue(int s) : queue_buf(mem_add(s)), queue_buf_size(s)
{
	queued_size = 0;
	queue_ptr = NULL;
}


VLenQueue::VLenQueue(VLenQueue &q) : queue_buf(mem_add(q.queue_buf_size)),
	queue_buf_size(q.queue_buf_size)
{
	memcpy( queue_buf, q.queue_buf, q.queued_size);
	queued_size = q.queued_size;
	queue_ptr = queue_buf + (q.queue_ptr - q.queue_buf);
}
// -------- end of function VLenQueue::VLenQueue ----------//

// -------- begin of function VLenQueue::~VLenQueue ----------//
VLenQueue::~VLenQueue()
{
	if( queue_buf )
		mem_del(queue_buf);
}
// -------- end of function VLenQueue::~VLenQueue ----------//


// -------- begin of function VLenQueue::operator= ----------//
VLenQueue& VLenQueue::operator= (VLenQueue &q)
{
	expand( q.queued_size );
	memcpy( queue_buf, q.queue_buf, queued_size = q.queued_size);
	queue_ptr = queue_buf + (q.queue_ptr - q.queue_buf);
	return *this;
}
// -------- end of function VLenQueue::operator= ----------//


// -------- begin of function VLenQueue::clear ----------//
void VLenQueue::clear()
{
	queued_size = 0;
	queue_ptr = queue_buf;
}
// -------- end of function VLenQueue::clear ----------//


// -------- begin of function VLenQueue::reserve ----------//
char* VLenQueue::reserve( int s )
{
	expand( queued_size + s);
	char *ret= queue_buf + queued_size;
	queued_size += s;
	return ret;
}
// -------- end of function VLenQueue::reserve ----------//


// -------- begin of function VLenQueue::append_queue ----------//
void VLenQueue::append_queue( VLenQueue &q )
{
	memcpy( reserve(q.queued_size), q.queue_buf, q.queued_size) ;
}
// -------- end of function VLenQueue::append_queue ----------//


// -------- begin of function VLenQueue::swap ----------//
void VLenQueue::swap( VLenQueue &q)
{
	char *tempPtr;
	int	tempInt;

	tempPtr = queue_buf;
	queue_buf = q.queue_buf;
	q.queue_buf = tempPtr;

	tempInt = queue_buf_size;
	queue_buf_size = q.queue_buf_size;
	q.queue_buf_size = tempInt;

	tempInt = queued_size;
	queued_size = q.queued_size;
	q.queued_size = tempInt;

	tempPtr = queue_ptr;
	queue_ptr = q.queue_ptr;
	q.queue_ptr = tempPtr;
}
// -------- end of function VLenQueue::swap ----------//


// -------- begin of function VLenQueue::length ----------//
int VLenQueue::length()
{
	return queued_size;
}
// -------- end of function VLenQueue::length ----------//


// -------- begin of function VLenQueue::expand ----------//
void VLenQueue::expand( int newSize)
{
	if( newSize > queued_size )
	{
		char *oldBuf = queue_buf;
		queue_buf_size = newSize + (QUEUE_SIZE_INC - newSize % QUEUE_SIZE_INC);
		queue_buf = mem_resize( queue_buf, queue_buf_size);
		queue_ptr = queue_buf + (queue_ptr - oldBuf);
	}
}
// -------- end of function VLenQueue::expand ----------//
