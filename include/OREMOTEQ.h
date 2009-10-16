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

// Filename    : OREMOTEQ.H
// Description : header file of remote message queue


#ifndef __OREMOTEQ_H
#define __OREMOTEQ_H

#include <OVQUEUE.h>

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