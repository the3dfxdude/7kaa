// Filename    : OREMOTEQ.H
// Description : header file of remote message queue


#ifndef __OREMOTEQ_H
#define __OREMOTEQ_H

#include <OVQUEUE.H>

struct RemoteMsg;

class RemoteQueue : public VLenQueue
{
public:
	RemoteQueue();
	RemoteQueue(int);
	RemoteQueue(RemoteQueue &);
	~RemoteQueue();

	// try to create RemoteQueueTraverse for traversal
	// instead of using these three functions
	void	traverse_set_start(int=0);
	int	traverse_finish();
	void	traverse_next();
	RemoteMsg *get_remote_msg(short *msgLen=NULL);

	int	validate_queue(int start=0);
};

class RemoteQueueTraverse
{
public:
	RemoteQueue &remote_queue;
	int	offset;

public:
	RemoteQueueTraverse( RemoteQueue &rq, int start=0 );

	void	traverse_set_start(int=0);
	int	traverse_finish();
	void	traverse_next();
	RemoteMsg *get_remote_msg(short *msgLen=NULL);
};

#endif